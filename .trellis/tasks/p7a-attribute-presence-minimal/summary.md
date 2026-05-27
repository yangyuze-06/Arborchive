# P7a Attribute Presence Minimal Summary

## changed files

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

## what changed

Implemented a minimal P7a Attribute Presence Graph slice for function
attributes:

- added CodeQL-aligned P7a presence models and table definitions;
- initialized `attributes`, `typeattributes`, `funcattributes`,
  `varattributes`, and `stmtattributes`;
- added `AttributeProcessor`;
- delegated function attribute extraction from `VisitFunctionDecl`;
- added a focused P7 fixture with `[[nodiscard]]`;
- added the fixture to `scripts/test_all.sh`;
- regenerated storage facade instantiations.

## selected minimal target

The selected target is a C++11-style function attribute:

```cpp
[[nodiscard]] int p7a_nodiscard_function() { return 7; }
```

The expected database evidence is:

- an `attributes` row with kind `1` (`@stdattribute`) and name `nodiscard`;
- a `funcattributes` row linking the function entity to that attribute row.

## CodeQL alignment statement

This task uses CodeQL counterpart tables from `docs/semmlecode.cpp.dbscheme`:

- `attributes(id, kind, name, name_space, location)`
- `funcattributes(func_id, spec_id)`

No Arbor extension table was added. P7b argument tables were intentionally not
implemented or initialized.

## ASTVisitor boundary result

`ASTVisitor` was modified only for thin delegation:

- owns no attribute row assembly;
- owns no attribute kind mapping;
- owns no storage/cache/key/dependency policy for attributes;
- calls `attribute_processor_->processFunctionAttributes(func_id, decl)` after
  the function entity ID is available.

## processor/helper ownership

`AttributeProcessor` owns the minimal attribute presence extraction:

- iterates explicit Clang attributes on supported declarations;
- maps supported syntax to the CodeQL attribute kind enum;
- records source location through `SrcLocRecorder`;
- inserts `DbModel::Attribute`;
- inserts `DbModel::FuncAttribute`.

Unsupported syntax and implicit attributes are skipped in this minimal slice.

## validation commands

```bash
python3 scripts/generate_instantiations.py
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
scripts/test_all.sh
git diff --check
python3 scripts/db_summary.py tests/output/unit-tests-p7-attribute_presence_case.db
sqlite3 tests/output/unit-tests-p7-attribute_presence_case.db ".tables"
sqlite3 tests/output/unit-tests-p7-attribute_presence_case.db "select * from attributes limit 10;"
sqlite3 tests/output/unit-tests-p7-attribute_presence_case.db "select * from funcattributes limit 10;"
sqlite3 tests/output/unit-tests-p7-attribute_presence_case.db ".tables" | grep attribute_arg
```

## command results

- `python3 scripts/generate_instantiations.py`: passed; generated 148
  instantiations.
- `make debug -j 8`: first attempt failed because default `llvm-config`
  reported LLVM 22. Re-run with LLVM 19 passed.
- `scripts/test_all.sh`: passed; all configured fixtures completed.
- `git diff --check`: passed.
- `python3 scripts/db_summary.py ...`: passed; P7 DB has 148 tables and 3
  function rows.
- `.tables`: passed; P7a presence tables are present and no `attribute_arg*`
  table is listed.

## DB files inspected

- `tests/output/unit-tests-p7-attribute_presence_case.db`

## SQLite queries

```sql
select * from attributes limit 10;
```

Result:

```text
78|1|nodiscard||76
```

```sql
select * from funcattributes limit 10;
```

Result:

```text
73|78
```

`grep attribute_arg` over `.tables` produced no output, confirming P7b
argument tables were not added in this minimal task.

## risks / limitations

- Only function attributes are populated.
- Only explicit attributes with supported Clang syntax are recorded.
- Type, variable, and statement attribute relationships are schema-ready but
  not behavior-populated in this task.
- No attribute arguments are recorded.
- Existing `VisitFunctionDecl` already contains legacy multi-processor
  orchestration; this patch adds one thin delegation call but does not address
  the broader visitor slimming debt.
- Existing test output includes pre-existing unresolved dependency warnings for
  some type keys; they did not fail `scripts/test_all.sh`.

## follow-ups

- Add variable/type/statement presence extraction in separate P7a tasks.
- Add GNU and MS syntax fixtures in separate small patches.
- Start P7b only after P7a presence ownership and fixtures are stable.
- Consider a later visitor-slimming task for existing `VisitFunctionDecl`
  orchestration.

## suggested commit message

```text
p7a: add minimal function attribute presence extraction
```
