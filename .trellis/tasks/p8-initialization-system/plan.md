# P8 Initialization System Plan

## goal

Implement the minimal P8a/P8b slice for variable initializers:

- add CodeQL-aligned schema support for `initialisers`;
- add CodeQL-aligned schema support for `braced_initialisers`;
- extract one `initialisers` row for each supported `VarDecl::getInit()`;
- extract one `braced_initialisers` row when the initializer expression is an
  `InitListExpr`.

## scope

- variable initializers from `VarDecl::getInit()`;
- global scalar initialization;
- local scalar initialization;
- braced scalar initialization;
- struct aggregate initialization;
- array aggregate initialization;
- SQLite evidence for `initialisers`, `braced_initialisers`,
  `aggregate_field_init`, and `aggregate_array_init`.

## non-goals

- constructor initializer tables;
- CXX member initializer extraction;
- default member initializer extraction;
- constexpr/consteval flow from P9;
- expression parent/ancestor graph from P10;
- broad expression modeling for initializer wrapper nodes;
- rewriting existing aggregate initialization behavior.

## implementation shape

1. Add `DbModel::Initialiser` and `DbModel::BracedInitialiser`.
2. Add table definitions:
   - `initialisers(init, var, expr, location)`;
   - `braced_initialisers(init)`.
3. Register the tables in `include/db/table_init.h`.
4. Add `InitializationProcessor`.
   - Resolve `initialisers.var` through `SEARCH_VARIABLE_CACHE`.
   - Generate a distinct `@initialiser` id.
   - Record initializer expression location with `SrcLocRecorder::processExpr`.
   - Use `DependencyManager` when the initializer expression id is not cached
     yet.
   - Insert `braced_initialisers` with the generated initializer id, not the
     `@expr` id.
5. Wire the processor into `ASTVisitor`.
   - Keep the visitor as delegation only.
   - Call initialization extraction after `VariableProcessor` has processed the
     declaration.
6. Regenerate storage facade instantiations.
7. Add `tests/unit-tests/p8/initialization_case.cc`.
8. Add the P8 fixture to `scripts/test_all.sh`.

## allowed files and directories

- `.trellis/tasks/p8-initialization-system/research.md`
- `.trellis/tasks/p8-initialization-system/plan.md`
- `.trellis/tasks/p8-initialization-system/validation.md`
- `.trellis/tasks/p8-initialization-system/summary.md`
- `include/model/db/initialization.h`
- `include/db/table_defs/initialization.h`
- `include/db/table_init.h`
- `include/core/processor/initialization_processor.h`
- `src/core/processor/initialization_processor.cc`
- `include/core/ast_visitor.h`
- `src/core/ast_visitor.cc`
- `src/db/storage_facade_instantiations.inc`
- `tests/unit-tests/p8/initialization_case.cc`
- `scripts/test_all.sh`

## forbidden files and directories

- Trellis runtime or scheduler code;
- `.trellis/spec/` stable governance docs;
- unrelated `docs/` roadmap changes;
- constructor/member initializer subsystem files unless compilation requires a
  narrow include adjustment;
- P9/P10 tables or processors;
- existing aggregate initialization code except for a validation-proven narrow
  fix.

## validation plan

Required commands:

```bash
git diff --check
python3 scripts/generate_instantiations.py
make debug -j 8
scripts/test_all.sh
```

P8 database evidence:

```bash
python3 scripts/db_summary.py tests/output/unit-tests-p8-initialization_case.db
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select count(*) from initialisers;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select count(*) from braced_initialisers;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select count(*) from aggregate_field_init;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select count(*) from aggregate_array_init;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select init, var, expr, location from initialisers order by init;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select init from braced_initialisers order by init;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select aggregate, initializer, field, position from aggregate_field_init order by aggregate, position;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select aggregate, initializer, element_index, position from aggregate_array_init order by aggregate, position;"
```

## expected risks

- The existing expression subsystem may not materialize every possible
  initializer expression form. This slice validates stable scalar literal and
  `InitListExpr` cases.
- `initialisers.expr` may be dependency-resolved after traversal because the
  declaration is visited before its initializer expression.
- Existing variable type IDs may have transitional issues unrelated to P8; P8
  should use direct variable entity IDs from the existing cache semantics.
