# P7a Attribute Presence Minimal Plan

## selected minimal target

Implement the smallest verifiable P7a presence subset:

- source construct: one function declaration with a C++11 `[[nodiscard]]`
  attribute;
- DB evidence: one or more rows in `attributes`;
- relationship evidence: a row in `funcattributes` linking the function entity
  to the recorded attribute.

This target avoids P7b argument serialization and starts with a stable
declaration attribute.

## files expected to change

- `.trellis/tasks/p7a-attribute-presence-minimal/plan.md`
- `.trellis/tasks/p7a-attribute-presence-minimal/summary.md`
- `include/model/db/attribute.h`
- `include/db/table_defs/attribute.h`
- `include/db/table_init.h`
- `include/core/processor/attribute_processor.h`
- `src/core/processor/attribute_processor.cc`
- `include/core/ast_visitor.h`
- `src/core/ast_visitor.cc`
- `src/db/storage_facade_instantiations.inc`
- `tests/unit-tests/p7/attribute_presence_case.cc`
- `scripts/test_all.sh`

## files forbidden to change

- P7b attribute argument implementation beyond schema-adjacent model/table
  definitions required for CodeQL-aligned approved tables;
- `.codex/`
- `.trellis/scripts/`
- `.trellis/.template-hashes.json`
- `.trellis/tasks/00-join-*`
- personal onboarding / session / journal task files;
- `.agents/skills/trellis-*`

## schema status

`docs/datatable-list.txt` already lists the P7 attribute tables:

- `attributes`
- `attribute_args`
- `attribute_arg_value`
- `attribute_arg_type`
- `attribute_arg_constant`
- `attribute_arg_expr`
- `attribute_arg_name`
- `typeattributes`
- `funcattributes`
- `varattributes`
- `stmtattributes`

`docs/semmlecode.cpp.dbscheme` defines their CodeQL-aligned column semantics.
The current code does not yet have attribute DB models, table definitions,
table initialization, or storage instantiations. A safe minimal implementation
requires adding only the P7a presence schema plumbing and regenerating storage
instantiations.

The behavior implemented in this task should only populate `attributes` and
`funcattributes`. The P7b argument tables must not be added or populated by
this task.

## CodeQL counterpart / alignment statement

This task uses CodeQL-aligned tables from `docs/semmlecode.cpp.dbscheme`:

- `attributes(id, kind, name, name_space, location)`
- `funcattributes(func_id, spec_id)`

`attributes.kind` follows the CodeQL enum where `1` is `@stdattribute`, matching
C++11 attribute syntax such as `[[nodiscard]]`. `funcattributes.func_id` links
to the function entity and `funcattributes.spec_id` links to the attribute row.

No Arbor extension table is introduced.

## ASTVisitor boundary plan

`ASTVisitor` must not assemble attribute rows or own schema semantics.

The only visitor change should be thin delegation after a function entity ID is
available:

- keep existing function processing behavior;
- if `FunctionProcessor::routerProcess` returns a valid function ID, call
  `AttributeProcessor::processFunctionAttributes(func_id, decl)`;
- keep all attribute row construction inside `AttributeProcessor`.

## processor/helper ownership plan

Create `AttributeProcessor` as the owner of attribute presence extraction.

Responsibilities:

- inspect Clang `Attr` objects attached to supported declarations;
- map supported syntax to the CodeQL `attributes.kind` enum;
- normalize attribute name and namespace from Clang attribute metadata;
- record source location through `SrcLocRecorder`;
- insert `DbModel::Attribute`;
- insert the supported relationship row, initially `DbModel::FuncAttribute`;
- explicitly skip implicit attributes and unsupported syntax in this minimal
  task.

## validation plan

Run:

```bash
git diff --check
python3 scripts/generate_instantiations.py
make debug -j 8
scripts/test_all.sh
python3 scripts/db_summary.py tests/output/unit-tests-p7-attribute_presence_case.db
sqlite3 tests/output/unit-tests-p7-attribute_presence_case.db ".tables"
sqlite3 tests/output/unit-tests-p7-attribute_presence_case.db "select * from attributes limit 10;"
sqlite3 tests/output/unit-tests-p7-attribute_presence_case.db "select * from funcattributes limit 10;"
```

Also inspect that P7b argument tables were not added in this minimal task:

```bash
sqlite3 tests/output/unit-tests-p7-attribute_presence_case.db ".tables" | grep attribute_arg
```

## explicit non-goals

- Do not implement P7b attribute argument extraction.
- Do not serialize `Expr` or `APValue`.
- Do not populate `attribute_args` or `attribute_arg_*` rows.
- Do not implement type, variable, or statement attribute relationships yet.
- Do not place semantic extraction or row assembly in `ASTVisitor`.
- Do not introduce undocumented tables.
- Do not run `trellis init`.
- Do not commit, push, open a PR, or run `git add .`.
