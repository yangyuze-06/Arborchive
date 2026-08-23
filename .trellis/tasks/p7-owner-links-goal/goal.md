# P7 Attribute Owner-Link Reinforcement Goal

## 1. Overview

This is a goal-mode brief for a future Codex `/goal` task. The future goal
should reference this file instead of embedding a long objective directly in
the prompt.

The purpose of the goal is to reinforce the P7 Attribute System from the
current P7a/P7b minimal safe subset into a function, type, variable, and
statement owner-link safe subset.

Current state:

- P7 is currently PARTIAL.
- Full P7 schema/model/table initialization already exists for:
  - `attributes`
  - `funcattributes`
  - `typeattributes`
  - `varattributes`
  - `stmtattributes`
  - `attribute_args`
  - `attribute_arg_value`
  - `attribute_arg_constant`
  - `attribute_arg_type`
  - `attribute_arg_expr`
  - `attribute_arg_name`
- P7a/P7b minimal safe subset is already implemented:
  - function attribute presence
  - `funcattributes` owner links
  - `DeprecatedAttr` string payload to `attribute_arg_value`
  - direct integer literal `AlignedAttr` payload to `attribute_arg_constant`
- `typeattributes`, `varattributes`, and `stmtattributes` are currently
  schema-only.
- `attribute_arg_type`, `attribute_arg_expr`, and `attribute_arg_name` remain
  deferred.
- Generalized `CONSTANT_EXPR`, dependent/template argument forms, and named
  argument forms remain deferred.

Goal outcome:

- Implement and validate P7c: `typeattributes` owner links.
- Implement and validate P7d: `varattributes` owner links.
- Implement and validate P7e: `stmtattributes` owner links.
- Explicitly exclude P7f complex argument payload tables from this goal.

P7 overall may remain PARTIAL after this goal because complex argument tables
remain deferred.

## 2. Non-goals / Forbidden Changes

The future goal must not:

- Implement `attribute_arg_type`.
- Implement `attribute_arg_expr`.
- Implement `attribute_arg_name`.
- Implement generalized `CONSTANT_EXPR`.
- Support dependent/template attribute arguments.
- Stringify arbitrary attribute arguments.
- Create Arbor-specific owner ids.
- Add schema unless a CodeQL alignment bug is proven and documented.
- Move attribute logic into `ASTVisitor`.
- Mark full P7 DONE.
- Do broad unrelated cleanup.

## 3. Architecture Rules

Preserve CodeQL-style layering:

```text
Attribute -> owner relation table -> AttributeArgument -> typed argument payload table
```

`AttributeProcessor` owns attribute-specific logic:

- attribute presence extraction
- kind/name/namespace mapping
- insertion into `attributes`
- insertion into `funcattributes`, `typeattributes`, `varattributes`, and
  `stmtattributes`
- reuse of existing safe argument extraction

Other processors may delegate to `AttributeProcessor` only after a stable owner
id exists.

`ASTVisitor` must remain thin. It must not contain argument mapping,
serialization, constant interpretation, schema assembly, or model insertion
logic.

## 4. Phase 0: Audit First

The future goal must first inspect:

- `include/db/table_defs/attribute.h`
- `include/model/db/attribute.h`
- `include/db/table_init.h`
- `include/core/processor/attribute_processor.h`
- `src/core/processor/attribute_processor.cc`
- `src/core/ast_visitor.cc`
- relevant type processors
- relevant variable/declaration processors
- relevant statement processors
- `src/core/processor/expr_processor.cc`
- existing P7a/P7b tests and fixtures
- `docs/roadmap.md`
- `docs/datatable-list.txt`
- `.trellis/tasks/p7b-attribute-arguments-xhigh/`

Audit questions:

- Where are stable type ids created?
- Where are stable variable/field/parameter ids created?
- Where are stable statement ids created?
- Which processors own those ids?
- Where can attribute processing be delegated after owner id creation?
- Are any owner families not stable enough for safe population?

Stop condition:

If an owner family lacks a stable CodeQL-aligned owner id, do not invent one.
Skip that subtask and document the blocker.

## 5. Phase 1: P7c Typeattributes

Implement conservative type attribute owner links only where stable type ids
already exist.

Candidate declarations to audit:

- `RecordDecl` / `CXXRecordDecl`
- `EnumDecl`
- `TypedefNameDecl` / `TypeAliasDecl`

Use only stable candidates. Add focused fixture coverage. Validate
`attributes` plus `typeattributes` rows with SQLite joins.

Do not implement `attribute_arg_type` or complex type argument extraction.

## 6. Phase 2: P7d Varattributes

Implement conservative variable attribute owner links only where stable
variable-like owner ids already exist.

Audit possible owner families:

- `VarDecl` global variables
- `VarDecl` local variables
- `FieldDecl` fields
- `ParmVarDecl` parameters

Do not invent a unified variable id. Use only existing CodeQL-aligned owner
ids.

Add focused fixture coverage. Validate `attributes` plus `varattributes` rows
with SQLite joins. Document exactly which variable-like declarations are
covered.

## 7. Phase 3: P7e Stmtattributes

Implement conservative statement attribute owner links only where stable
statement ids already exist.

Audit `StmtProcessor` carefully:

- How are statement ids created?
- Are wrapper statements represented?
- Are `AttributedStmt` nodes already processed?
- Does the inner statement get a separate id?
- Which id should own the attribute?
- Would processing create duplicate wrapper/inner ownership?

Candidate syntax to evaluate:

- `[[fallthrough]]`
- `[[likely]]`
- `[[unlikely]]`
- GNU statement attributes if stable

If statement attribute ownership is ambiguous, document the blocker and leave
`stmtattributes` deferred.

Add focused fixture coverage only for safe cases. Validate `attributes` plus
`stmtattributes` rows with SQLite joins.

## 8. Phase 4: Preserve P7a/P7b Behavior

Existing function attribute behavior must remain compatible.

Required preservation checks:

- `DeprecatedAttr` string payload should still populate `attribute_arg_value`.
- Direct integer literal `AlignedAttr` should still populate
  `attribute_arg_constant`.
- Unsupported complex args must not be string-dumped.
- `attribute_arg_type`, `attribute_arg_expr`, and `attribute_arg_name` should
  remain unpopulated unless a later P7f goal implements them.

## 9. Tests and Fixtures

Suggested fixture names:

- `tests/unit-tests/p7/attribute_owner_links_case.cc`

Or separate files if project convention prefers:

- `tests/unit-tests/p7/type_attribute_case.cc`
- `tests/unit-tests/p7/var_attribute_case.cc`
- `tests/unit-tests/p7/stmt_attribute_case.cc`

Requirements:

- Each implemented owner family should have a positive case.
- Fixtures should be minimal and compiler-stable.
- Avoid cases requiring generalized complex argument extraction.
- Update `scripts/test_all.sh` only if that is project convention for new unit
  fixtures.

## 10. Docs Update Rules

`docs/roadmap.md` may be updated after implementation to say the P7 owner-link
safe subset has been reinforced.

`docs/datatable-list.txt` must not mark all P7 tables DONE. Only mark
individual owner-link tables DONE if repository convention supports safe-subset
DONE. Otherwise keep P7 overall PARTIAL and document scope in the roadmap or an
analysis doc.

Optional analysis doc:

- `docs/analysis/p7_attribute_owner_links.md`

## 11. Validation Commands

Use these commands as examples and adapt paths if needed:

```bash
git diff --check

./scripts/generate_database.sh tests/unit-tests/p7/attribute_owner_links_case.cc tests/output/unit-tests-p7-attribute_owner_links_case.db

python3 scripts/db_summary.py tests/output/unit-tests-p7-attribute_owner_links_case.db

sqlite3 tests/output/unit-tests-p7-attribute_owner_links_case.db "
select 'attributes', count(*) from attributes
union all select 'funcattributes', count(*) from funcattributes
union all select 'typeattributes', count(*) from typeattributes
union all select 'varattributes', count(*) from varattributes
union all select 'stmtattributes', count(*) from stmtattributes
union all select 'attribute_args', count(*) from attribute_args
union all select 'attribute_arg_value', count(*) from attribute_arg_value
union all select 'attribute_arg_constant', count(*) from attribute_arg_constant
union all select 'attribute_arg_type', count(*) from attribute_arg_type
union all select 'attribute_arg_expr', count(*) from attribute_arg_expr
union all select 'attribute_arg_name', count(*) from attribute_arg_name;
"

sqlite3 tests/output/unit-tests-p7-attribute_owner_links_case.db "
select a.id, a.name, a.name_space, ta.type
from typeattributes ta
join attributes a on a.id = ta.attribute
order by a.id;
"

sqlite3 tests/output/unit-tests-p7-attribute_owner_links_case.db "
select a.id, a.name, a.name_space, va.variable
from varattributes va
join attributes a on a.id = va.attribute
order by a.id;
"

sqlite3 tests/output/unit-tests-p7-attribute_owner_links_case.db "
select a.id, a.name, a.name_space, sa.stmt
from stmtattributes sa
join attributes a on a.id = sa.attribute
order by a.id;
"
```
