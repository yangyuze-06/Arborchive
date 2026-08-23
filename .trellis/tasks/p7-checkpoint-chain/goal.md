# P7 Checkpoint Chain Goal

## Purpose

Run the remaining P7 Attribute System work as a checkpointed completion chain.

This is a large implementation experiment, but it must remain semantically safe. The goal is to continue from the current P7 state, attempt the remaining high-risk payload families, and decide whether P7 can honestly move closer to full `DONE`.

This file is intended to be referenced by a short Codex `/goal` command instead of pasted directly into the goal objective.

## Current P7 State

P7 remains `PARTIAL`.

Already implemented safe subsets:

- explicit supported attribute presence rows;
- `funcattributes` owner links;
- `typeattributes` owner links for stable record / enum / alias type ids;
- `varattributes` owner links for field / global / local / param variable ids;
- canonical `ParmVarDecl` variable entity id reuse;
- conservative `stmtattributes` owner links for supported inner stmt ids;
- `attribute_arg_value` for stable string payloads:
  - `DeprecatedAttr` message / replacement;
  - `AnnotateAttr` annotation string;
  - `SectionAttr` section name;
- `attribute_arg_constant` for direct and shallow-wrapped `AlignedAttr` integer literal constants;
- `ExprProcessor::getOrProcessExprId(const clang::Expr *)`;
- minimal `attribute_arg_expr` support for non-literal, non-dependent `AlignedAttr` expressions such as `aligned(8 + 8)`.

Currently deferred or partial:

- broader `attribute_arg_expr` attrs:
  - `EnableIfAttr`;
  - `DiagnoseIfAttr`;
  - `AnnotateAttr` expression args;
  - `AssumeAlignedAttr`;
- `attribute_arg_type`;
- `attribute_arg_name`;
- generalized constant / `APValue` handling;
- broader constant/value families:
  - `NonNullAttr`;
  - `FormatAttr`;
  - `AllocSizeAttr`;
  - `AllocAlignAttr`;
  - `AvailabilityAttr`;
  - `AliasAttr`;
- `stmtattributes` wrapper-vs-inner parity;
- final P7 full-DONE decision.

Reference documents:

- `docs/analysis/p7_full_done_criteria.md`
- `docs/analysis/p7f_attribute_expr_design.md`
- `docs/analysis/p7_remaining_attribute_scope_audit.md`
- `docs/analysis/p7_attribute_owner_links.md`
- `docs/roadmap.md`
- `docs/semmlecode.cpp.dbscheme`

## Global Hard Constraints

Do not violate these constraints:

- Do not modify schema/model/table initialization unless a documented CodeQL alignment bug is proven.
- Do not stringify arbitrary attribute arguments.
- Do not map expression/type/name payloads into `attribute_arg_value`.
- Do not use `Type::getAsString()` as a type payload.
- Do not evaluate arbitrary expressions.
- Do not process dependent/template attribute args unless a safe policy is proven.
- Do not move attribute argument logic into `ASTVisitor`.
- `ASTVisitor` must remain thin.
- `AttributeProcessor` owns attribute argument classification and typed payload insertion.
- `ExprProcessor` and `TypeProcessor` may provide stable ids.
- Do not mark `docs/datatable-list.txt` as `DONE` until final parity review proves full-DONE criteria.
- Do not hide uncertainty.
- If unsure, defer and document.

## CodeQL-Shaped Payload Policy

P7 must preserve typed payload separation:

- `attribute_arg_value` is for stable text/string/token payload semantics only.
- `attribute_arg_constant` is for stable constant/integer semantics only.
- `attribute_arg_expr` is for stable `@expr` ids only.
- `attribute_arg_type` is for stable `@type` ids only.
- `attribute_arg_name` is only for true CodeQL named argument semantics, with Microsoft named arguments as the primary target.

Never collapse typed payloads into string fallbacks.

## Checkpoint Rule

After every checkpoint:

1. run focused validation;
2. run self-review;
3. report `PASS`, `PARTIAL PASS`, `BLOCKED`, or `NEEDS CHANGES`;
4. continue only if the checkpoint is semantically safe;
5. if unsafe, revert or isolate the unsafe checkpoint and document the blocker.

Before moving to the next checkpoint, report:

- changed files;
- implementation summary;
- validation commands and results;
- SQLite evidence;
- self-review verdict;
- whether it is safe to continue.

## Checkpoint 0: Baseline Verification

Run:

```bash
git status --short
git diff --check
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
./scripts/test_all.sh
```

Inspect current focused P7 DBs with `./build/demo` if needed.

Confirm:

- current P7 fixtures pass;
- `attribute_arg_expr` rows exist only for the implemented `AlignedAttr` expression safe subset;
- `attribute_arg_type` is still `0`;
- `attribute_arg_name` is still `0`;
- duplicate `ParmVarDecl` query returns no rows;
- duplicate expression smoke query returns no rows;
- P7 remains `PARTIAL` in docs.

Duplicate parameter query:

```sql
select function, index, type_id, count(*), group_concat(id)
from params
group by function, index, type_id
having count(*) > 1;
```

## Checkpoint 1: Preserve Existing Minimal Expr Subset

The current minimal `attribute_arg_expr` subset must remain valid:

- `aligned(16)` goes to `attribute_arg_constant`;
- `aligned((32))` goes to `attribute_arg_constant`;
- `aligned(64u)` goes to `attribute_arg_constant`;
- `aligned(8 + 8)` goes to `attribute_arg_expr`;
- dependent aligned expressions are skipped;
- no argument appears in both `attribute_arg_constant` and `attribute_arg_expr`;
- `attribute_arg_type` and `attribute_arg_name` remain unaffected.

Do not expand behavior in this checkpoint. Only verify and preserve the existing state.

Validation:

```sql
select 'attribute_arg_expr', count(*) from attribute_arg_expr
union all select 'attribute_arg_constant', count(*) from attribute_arg_constant
union all select 'attribute_arg_type', count(*) from attribute_arg_type
union all select 'attribute_arg_name', count(*) from attribute_arg_name;
```

Overlap query:

```sql
select aa.id
from attribute_args aa
join attribute_arg_constant c on c.arg = aa.id
join attribute_arg_expr e on e.arg = aa.id;
```

Self-review:

- Did any expression get stringified?
- Did any dependent expr get processed?
- Did constant behavior remain intact?
- Is P7 still `PARTIAL`?

## Checkpoint 2: Broader `attribute_arg_expr` Design Gate

Audit these candidates:

- `EnableIfAttr`;
- `DiagnoseIfAttr`;
- `AnnotateAttr` expression args;
- `AssumeAlignedAttr` expression forms.

For each candidate, answer:

1. Does the local Clang API expose a stable `Expr *`?
2. Does `ExprProcessor::getOrProcessExprId` support that expression kind without broad new expression semantics?
3. Would explicit processing duplicate rows already visited by RAV?
4. Can dependent/type-dependent/value-dependent/instantiation-dependent/unexpanded-pack expressions be skipped reliably?
5. Is the argument index deterministic and CodeQL-shaped?
6. Is a stable positive fixture possible under this repository’s compiler flags?
7. Is a negative/dependent fixture possible?
8. Should the candidate be `SAFE NOW`, `SAFE AFTER MORE ExprProcessor SUPPORT`, `DEFER`, or `UNSAFE`?

Implementation rule:

- If no candidate is clearly `SAFE NOW`, do not implement.
- If exactly one candidate is clearly `SAFE NOW`, implement only that one with a focused fixture and SQLite evidence.
- Do not implement multiple new expression attrs in this checkpoint.

Forbidden in this checkpoint:

- `attribute_arg_type`;
- `attribute_arg_name`;
- generalized `APValue` / `CONSTANT_EXPR`;
- arbitrary expression attrs;
- string fallback;
- source text fallback;
- `docs/datatable-list.txt` `DONE`.

Validation if implementation happens:

- `git diff --check`;
- build;
- focused `./build/demo` DB generation;
- `db_summary.py`;
- SQLite evidence:
  - expected `attribute_arg_expr` rows;
  - no constant/expr overlap;
  - duplicate expr groups = `0`;
  - `attribute_arg_type = 0`;
  - `attribute_arg_name = 0`;
- `./scripts/test_all.sh`.

Self-review:

- Did any expr get stringified?
- Did any dependent expr get processed?
- Did any candidate require broad `ExprProcessor` expansion?
- Did P7 remain `PARTIAL`?

## Checkpoint 3: `attribute_arg_type` Design Gate

Before implementation, audit:

- Which `Attr` APIs expose `TypeSourceInfo`, `QualType`, or stable type payloads?
- Does `TypeProcessor` already produce stable `@type` ids for these types?
- What is the `@type` / `@usertype` policy?
- Should canonical type or spelling type be used?
- Can dependent types be skipped reliably?
- Is there a safe positive fixture?

Candidate families may include:

- `AnnotateTypeAttr`, only if local API and CodeQL semantics are clear;
- other type-bearing attrs only if clearly stable.

Implementation rule:

- If no candidate is clearly `SAFE NOW`, document blocker and leave `attribute_arg_type` deferred.
- If one candidate is clearly `SAFE NOW`, implement only that one.

Forbidden:

- `getAsString()` fallback;
- invented type ids;
- dependent types;
- flattening type args into `attribute_arg_value`;
- broad type-system refactor.

Validation if implementation happens:

- focused fixture;
- `attribute_arg_type` rows join to stable type ids;
- no string fallback;
- dependent cases skipped;
- existing expr/value/constant behavior unaffected;
- `./scripts/test_all.sh`.

Self-review:

- Are type ids truly stable?
- Is `@type/@usertype` policy documented?
- Was spelling/canonical behavior chosen deliberately?
- Is P7 still `PARTIAL` unless all criteria are met?

## Checkpoint 4: `attribute_arg_name` Microsoft Named Args

Before implementation, audit:

- CodeQL named arguments are Microsoft feature.
- Determine if local Clang and fixture syntax can produce Microsoft named arguments.
- Do not use `ModeAttr`, `FormatAttr`, `AvailabilityAttr`, or `CleanupAttr` as name unless CodeQL parity is proven.

Implementation rule:

- Implement only if true Microsoft named argument semantics are available and testable.
- Otherwise document blocker and keep `attribute_arg_name` deferred.

Forbidden:

- modeling string payloads as names;
- modeling declaration references as names;
- guessing based on spelling;
- using identifier-like tokens as names without CodeQL parity.

Validation if implementation happens:

- `attribute_arg_name` rows only for true named args;
- string payloads are not misclassified;
- decl refs are not misclassified;
- expr/type behavior unaffected;
- `./scripts/test_all.sh`.

Self-review:

- Did any identifier-like token get guessed as name?
- Is this aligned with CodeQL named argument semantics?

## Checkpoint 5: Constant/Value Family Expansion

Goal: expand constant/value payloads only for stable Clang APIs.

Candidates:

- `NonNullAttr` integer indexes;
- `FormatAttr` integer indexes and stable text/name-like kind only if target table is clear;
- `AllocSizeAttr` integer positions;
- `AllocAlignAttr` integer positions;
- `AvailabilityAttr` message/replacement strings only if indexing semantics are clear;
- `AliasAttr` only if target/compiler validation is stable.

Rules:

- Do not implement generalized `APValue` unless separately designed and tested.
- Do not evaluate arbitrary expressions.
- Do not turn expr payloads into constants unless CodeQL parity says so.
- Keep argument indexes deterministic.
- Do not model string/name/expr/type ambiguously.

Add focused fixtures if implemented:

- `tests/unit-tests/p7/attribute_extra_constant_args_case.cc`
- `tests/unit-tests/p7/attribute_extra_value_args_case.cc`

or one combined fixture if cleaner.

Validation:

- expected value/constant rows;
- no accidental expr/type/name rows;
- positive and negative cases;
- `./scripts/test_all.sh`.

Self-review:

- Are all value/constant handlers explicit whitelist handlers?
- Did any expression get evaluated arbitrarily?
- Are argument indexes deterministic?
- Is unsupported ambiguity documented?

## Checkpoint 6: `stmtattributes` Wrapper-vs-Inner Parity Review

Goal: decide whether current inner-stmt ownership is enough for full `DONE`.

Tasks:

- inspect local statement schema and CodeQL dbscheme;
- inspect `StmtProcessor` `AttributedStmt` handling;
- determine whether wrapper stmt ids should exist;
- if wrapper support is needed and safe, propose or implement a minimal stable wrapper strategy;
- if not safe, document why `stmtattributes` remains `PARTIAL`.

Do not force wrapper ids if it destabilizes existing statement extraction.

Validation:

- existing `fallthrough` / `likely` rows remain correct;
- unsupported wrappers are skipped or modeled deliberately;
- no duplicate wrapper/inner owner rows;
- `./scripts/test_all.sh`.

Self-review:

- Is wrapper-vs-inner semantics CodeQL-shaped?
- Are unsupported wrappers skipped safely?
- Can `stmtattributes` be full `DONE`, or must it remain `CONSERVATIVE PARTIAL`?

## Checkpoint 7: Documentation and Final Parity Decision

Update docs conservatively:

- `docs/roadmap.md`;
- `docs/analysis/p7_full_done_criteria.md`;
- `docs/analysis/p7f_attribute_expr_design.md`;
- `docs/analysis/p7_remaining_attribute_scope_audit.md`;
- optional `docs/analysis/p7_checkpoint_chain_report.md`.

Decide:

- Can P7 be honestly marked full `DONE`?
- If yes, list exact evidence for every P7 table.
- If no, keep P7 `PARTIAL` and list remaining blockers.

Only modify `docs/datatable-list.txt` if all of these are true:

- all P7 full-DONE criteria are met;
- expr/type/name/constant/value/owner semantics have positive and negative tests;
- unsupported/dependent cases have documented policy;
- SQLite evidence supports every table;
- no partial owner-link blocker remains.

## Final Validation

Run:

```bash
git diff --check
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
./scripts/test_all.sh
```

For every new P7 fixture:

```bash
./build/demo <fixture> <output-db>
python3 scripts/db_summary.py <output-db>
```

Run SQLite evidence for every P7 table:

- `attributes`;
- `funcattributes`;
- `typeattributes`;
- `varattributes`;
- `stmtattributes`;
- `attribute_args`;
- `attribute_arg_value`;
- `attribute_arg_constant`;
- `attribute_arg_type`;
- `attribute_arg_expr`;
- `attribute_arg_name`.

Also verify:

- duplicate `ParmVarDecl` query remains empty;
- duplicate expr smoke query remains empty;
- P7a/P7b compatibility queries pass;
- owner-link fixture queries pass.

## Final Report Format

Report:

1. verdict: `PASS`, `PARTIAL PASS`, `BLOCKED`, or `NEEDS CHANGES`;
2. whether P7 can honestly be marked full `DONE`;
3. changed files grouped by code/tests/docs;
4. implemented payload families;
5. deferred payload families and blockers;
6. architecture boundary review;
7. SQLite evidence;
8. validation results;
9. risk of schema pollution;
10. recommended commit message.

## Commit Message Guidance

If P7 truly passes all full-DONE criteria:

```text
feat: complete P7 attribute system
```

If several safe subsets are added but P7 remains partial:

```text
feat: expand P7 attribute argument coverage
```

If mainly blockers/docs are produced:

```text
docs: document P7 full completion blockers
```