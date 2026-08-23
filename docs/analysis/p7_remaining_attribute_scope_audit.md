# P7 Remaining Attribute Scope Audit

## 1. Verdict

P7 overall remains PARTIAL. The current implementation is a useful CodeQL-shaped
safe subset, but it is not a complete Attribute System.

Implemented enough to keep:

- Function attribute presence and `funcattributes`.
- Stable whitelisted strings as `attribute_arg_value`:
  - `DeprecatedAttr` message/replacement strings.
  - `AnnotateAttr` annotation strings.
  - `SectionAttr` section names.
  - `WarnUnusedResultAttr` / `[[nodiscard("...")]]` messages.
- Direct and shallow wrapped integer-literal `AlignedAttr` expression arguments
  as `attribute_arg_constant`.
- Minimal non-literal `AlignedAttr` expression arguments such as `8 + 8` as
  `attribute_arg_expr`, using stable `ExprProcessor` id reuse.
- Minimal non-literal `AssumeAlignedAttr` alignment and offset expression
  arguments as `attribute_arg_expr`, with literal and constant handling still
  deferred.
- Conservative owner links for stable type, variable, and statement owners.
- Canonical `ParmVarDecl` variable entity id reuse for parameter declarations,
  parameter attributes, and `DeclRefExpr` references.

Not implemented and not safe to mark DONE:

- `attribute_arg_type`.
- Broad `attribute_arg_expr` families beyond the selected non-literal
  `AlignedAttr` and `AssumeAlignedAttr` safe subsets.
- `attribute_arg_name`.
- Generalized `CONSTANT_EXPR` beyond the direct integer-literal safe subset.
- Dependent/template attribute arguments.
- Availability named fields and versions.
- Alias strings on the current Darwin validation target.
- Broad Clang `Attr` argument modeling.

Recommended next implementation task: continue the checkpoint chain with
`stmtattributes` wrapper-vs-inner parity review. The type and name argument
design gates are blocked for now, and `EnableIfAttr`, `DiagnoseIfAttr`,
`AnnotateAttr` expression payloads, generalized constants, and scalar-index
constant families remain deferred.

## 2. Current P7 Status Summary

Schema/model/table initialization is complete for the P7 table family listed in
`docs/datatable-list.txt` and present in `docs/semmlecode.cpp.dbscheme`.
Behavior population is intentionally partial.

Current ownership shape:

- `ASTVisitor` obtains owner ids from domain processors and delegates to
  `AttributeProcessor`.
- `AttributeProcessor` owns attribute row creation, owner-link rows, supported
  argument classification, and typed payload insertion.
- `ExprProcessor` is used for direct or shallow wrapped integer literal
  expression rows used by `attribute_arg_constant`, and for the selected
  non-literal `AlignedAttr` and `AssumeAlignedAttr` expression subsets used by
  `attribute_arg_expr`.
- `TypeProcessor`, `VariableProcessor`, and `StmtProcessor` provide stable owner
  ids for the implemented owner-link safe subset.

This shape is architecturally acceptable as a transitional safe subset. Future
P7f work should not add argument-specific branching to `ASTVisitor`.

## 3. P7 Table Status Matrix

| Table | Schema/model/init | Behavior population | Safe subset coverage | Test coverage | Docs status | Remaining work | Status |
|---|---|---|---|---|---|---|---|
| `attributes` | Present and aligned with dbscheme columns `id`, `kind`, `name`, `name_space`, `location`. | Populated for explicit non-implicit attrs with supported syntax kind. | GNU, standard, declspec, MS, alignas kind mapping via Clang APIs. | P7a/P7b/P7 owner-link fixtures. | Roadmap and owner-link note accurate. | Broader semantic completeness depends on more owner entrypoints and Attr classes. | SAFE SUBSET DONE |
| `funcattributes` | Present. | Populated from `VisitFunctionDecl` after function id resolution. | Function declarations with explicit attrs. | `attribute_presence_case.cc`, `attribute_arguments_case.cc`. | Accurate. | Audit constructors/destructors/conversion/deduction guide coverage before claiming all functions. | SAFE SUBSET DONE |
| `typeattributes` | Present. | Populated for records, enums, typedefs, and type aliases after type id resolution. | Stable `@type`/`@usertype` owner ids for record/enum/alias declarations. | `attribute_owner_links_case.cc`. | Accurate. | Unsupported type-like attrs and attributed type spellings remain unmodeled. | SAFE SUBSET DONE |
| `varattributes` | Present. | Populated for fields, globals, locals, and parameters using unified variable entity ids. | `@membervariable`, `@globalvariable`, `@localvariable`, `@parameter`. | `attribute_owner_links_case.cc`; SQLite duplicate-param query. | Accurate and documents ParmVarDecl fix. | Add explicit regression tests for canonical parameter id reuse and no duplicate `params` rows. | SAFE SUBSET DONE |
| `stmtattributes` | Present. | Populated for conservative `AttributedStmt` inner owners returned by `StmtProcessor`. | `[[fallthrough]]` to inner `NullStmt`; `[[likely]]` to inner `ReturnStmt`; more inner stmt kinds supported by helper but not all tested. | `attribute_owner_links_case.cc`. | Accurate. | Add unsupported-wrapper skip regression; decide whether wrapper-vs-inner semantics match CodeQL extractor expectations. | CONSERVATIVE PARTIAL |
| `attribute_args` | Present. | Populated only for supported P7b/P7f safe-subset args. | Stable string payloads, direct/shallow wrapped literal `AlignedAttr`, and selected non-literal `AlignedAttr`/`AssumeAlignedAttr` expr args. | `attribute_arguments_case.cc`, `attribute_string_args_case.cc`, `attribute_constant_args_case.cc`, `attribute_expr_args_case.cc`, `attribute_assume_aligned_expr_args_case.cc`. | Accurate. | More typed argument families and indexes. | SAFE SUBSET DONE |
| `attribute_arg_value` | Present. | Populated for whitelisted stable strings only. | `DeprecatedAttr` message/replacement, `AnnotateAttr` annotation, `SectionAttr` name, `WarnUnusedResultAttr` / `[[nodiscard("...")]]` message. | `attribute_arguments_case.cc`, `attribute_string_args_case.cc`, `attribute_extra_value_args_case.cc`. | Updated. | More string payloads only when target-supported and index semantics are clear. | SAFE SUBSET DONE |
| `attribute_arg_constant` | Present. | Populated for `AlignedAttr` when the alignment expr unwraps to an `IntegerLiteral`. | Direct integer literal and shallow `ConstantExpr`/paren/implicit-cast wrappers around a literal; non-literal `8 + 8` is now classified as expr, not folded. | `attribute_arguments_case.cc`, `attribute_constant_args_case.cc`, `attribute_expr_args_case.cc`. | Updated. | Generalized constants, APValue, ParamIdx, scalar fields, and `CONSTANT_EXPR` kind remain deferred. | SAFE SUBSET DONE |
| `attribute_arg_type` | Present. | Not populated. | None. | Empty in P7 fixtures. | Roadmap says deferred. | Define type argument semantics and owner id policy. | DEFERRED |
| `attribute_arg_expr` | Present. | Populated for selected non-literal expression subsets. | `aligned(8 + 8)`, shallow wrapped non-literal arithmetic expressions, and `AssumeAlignedAttr` non-literal alignment/offset expressions use stable `@expr` ids; dependent template expressions are skipped. | `attribute_expr_args_case.cc`, `attribute_assume_aligned_expr_args_case.cc`. | Updated. | Broader expr-backed attrs, dependent policy evidence, unevaluated/constant-expr classification, literal policy for `AssumeAlignedAttr`, and broad `Expr **args_` families. | SAFE SUBSET PARTIAL |
| `attribute_arg_name` | Present. | Not populated. | None. | Empty in P7 fixtures. | Roadmap says deferred. | Define what CodeQL means by name args before implementation. | DEFERRED |

Status note: `SAFE SUBSET DONE` in this audit means only that the currently
implemented safe subset is reviewable, documented, and covered enough to keep.
It is not equivalent to full CodeQL semantic completion and must not be copied
into `docs/datatable-list.txt` as `DONE`.

`docs/datatable-list.txt` should remain unmarked for P7 tables for now. There is
no current convention that distinguishes full DONE from safe-subset done inside
that list, and marking these tables DONE would overstate P7 completion.

## 4. Owner-Link Audit Results

### Typeattributes

The implemented type owner links are stable enough for the current safe subset.
`ASTVisitor` calls `TypeProcessor` first, then passes the returned type id to
`AttributeProcessor`. The current fixture validates:

- record: `deprecated|243|P7cDeprecatedRecord|1`
- enum: `deprecated|265|P7cDeprecatedEnum|4`
- alias: `deprecated|278|P7cDeprecatedAlias|14`

No Arbor-specific type owner ids were invented. The row shape uses the existing
`@type`-compatible user type ids. Unsupported type-like attribute cases, such as
attributed type spelling and Clang `TypeAttr` payloads, are not forced into
`typeattributes`; those belong to future P7f type-argument work.

Residual risk: `typeattributes.type_id` is dbscheme `@type ref`, while many
validated rows are `usertypes` ids. This appears consistent with current
Arborchive type modeling, but final CodeQL parity should be rechecked when
`attribute_arg_type` is implemented.

### Varattributes

The canonical `ParmVarDecl` blocker appears fixed. `VariableProcessor` caches
the variable entity id for parameters; `VisitParmVarDecl` resolves the canonical
cached id before recording attributes. The duplicate parameter query currently
returns no rows:

```sql
select function, "index", type_id, count(), group_concat(id)
from params
group by function, "index", type_id
having count() > 1;
```

Focused owner evidence:

```text
no_unique_address|252|member|field
maybe_unused|285|global|p7d_global
maybe_unused|307|param|param
maybe_unused|320|local|local
```

`varattributes.var_id` uses the same variable entity families already used by
`var_decls`: member, global, parameter, and local variable ids. That matches the
dbscheme intent of `@accessible ref` better than field/param/local-specific ad
hoc ids would.

Residual risk: add a regression test that explicitly joins `varattributes`,
`params`, `var_decls`, and `DeclRefExpr`/variable reference output for the same
parameter id. The current evidence exists, but it is not a dedicated assertion.

### Stmtattributes

`StmtProcessor::getSupportedAttributedStmtOwnerId` is conservative. It unwraps
`AttributedStmt` and records attributes on stable inner statement ids for known
statement families. Current evidence:

```text
fallthrough|361|26
likely|379|6
```

This avoids inventing a wrapper statement id and avoids duplicate wrapper/inner
owner rows for the tested cases. Unsupported wrappers return `-1`, causing
`AttributeProcessor` to skip insertion.

Residual risk: current tests cover only `NullStmt` and `ReturnStmt` inner
owners. The helper supports more inner stmt classes than the fixture proves.
Keep `stmtattributes` as CONSERVATIVE PARTIAL until unsupported-wrapper skip
behavior, additional supported inner statement cases, and wrapper-vs-inner owner
semantics have CodeQL parity confirmation.

## 5. P7a/P7b Compatibility Results

Existing P7a/P7b behavior remains intact in current output:

- `attribute_presence_case.cc` covers function attribute presence and
  `funcattributes`.
- `attribute_arguments_case.cc` covers `DeprecatedAttr` message string and
  direct integer literal `AlignedAttr`.
- `AttributeProcessor` also has a `DeprecatedAttr` replacement-string branch,
  but the current fixture does not cover it.
- Current P7b counts:
  - `attributes|2`
  - `funcattributes|2`
  - `attribute_args|2`
  - `attribute_arg_value|1`
  - `attribute_arg_constant|1`
  - `attribute_arg_type|0`
  - `attribute_arg_expr|0`
  - `attribute_arg_name|0`

Current P7b row evidence:

```text
253|deprecated|1|0|prefer p7b_plain_function
281|aligned|2|0|16
```

Owner-link additions do not appear to break P7a/P7b. However, tests may still be
fragile if they depend on exact generated ids or insertion order. Future tests
should assert semantic joins and counts rather than hard-coded ids.

## 6. P7f Remaining Scope

CodeQL dbscheme separates argument identity from typed payload:

- `attribute_args.id` is `@attribute_arg`.
- `attribute_args.kind` distinguishes empty, token, constant, type,
  constant-expr, and expr.
- `attribute_arg_value` stores string/token payload.
- `attribute_arg_type` points to `@type`.
- `attribute_arg_constant` points to `@expr`.
- `attribute_arg_expr` points to `@expr`.
- `attribute_arg_name` stores a string name.

The remaining work is not "complete P7f"; it is several independent design and
validation tasks.

### Attribute Arg Value

Safe expansion area, with currently implemented payloads:

- `DeprecatedAttr::getMessage` and `getReplacement`.
- `AnnotateAttr::getAnnotation`.
- `SectionAttr::getName`.
- `WarnUnusedResultAttr::getMessage` for non-empty `[[nodiscard("...")]]`
  messages.

Candidate stable string APIs still not implemented:

- `AliasAttr::getAliasee`.
- `WeakRefAttr::getAliasee` if included in scope.
- `AnnotateTypeAttr::getAnnotation` if a stable owner path is defined.
- `EnableIfAttr::getMessage` and `DiagnoseIfAttr::getMessage`, but only if
  their expression args remain explicitly deferred.
- `AvailabilityAttr::getMessage` and `getReplacement`, but platform/version
  fields need separate design.
Do not use `attribute_arg_value` as a catch-all dump for names, expressions,
types, APValues, or raw AST text.

Current blockers:

- `AliasAttr` could not be validated in the current Darwin fixture because
  Clang reports aliases as unsupported on this target.
- `AvailabilityAttr` message/replacement strings are stable getters, but their
  argument indexes are entangled with platform/version/named fields, so they
  remain deferred.

### Attribute Arg Constant

Current support is narrower than CodeQL's full constant model. It now validates
direct integer literals, parenthesized integer literals, and implicit-cast
integer literals in `AlignedAttr`; non-literal `aligned(8 + 8)` is classified
through `attribute_arg_expr` instead of being folded into a constant.

Future expansion could include Clang API fields that are already scalar
integers, such as
`FormatAttr::getFormatIdx`, `FormatAttr::getFirstArg`, `AllocSizeAttr`
`ParamIdx`, `AllocAlignAttr::getParamIndex`, and `NonNullAttr` index lists.

Design question: these are integer-like parameters, but they are not necessarily
source `@expr` nodes. Since dbscheme says `attribute_arg_constant.constant` is
`@expr ref`, Arborchive should not insert raw scalar integers into this table
without an expression-row policy. Prefer deferral or a dedicated audit before
modeling `ParamIdx`/plain int fields.

### Attribute Arg Expr

Candidates with `Expr *` APIs:

- `AlignedAttr::getAlignmentExpr`.
- `AssumeAlignedAttr::getAlignment` and `getOffset`.
- `EnableIfAttr::getCond`.
- `DiagnoseIfAttr::getCond`.
- `AnnotateAttr::args`.
- `AnnotateTypeAttr::args`.

This should wait for a minimal expression-argument design. The main danger is
duplicating rows or processing expressions that normal traversal does not visit.

### Attribute Arg Type

Candidates with type APIs:

- `AlignedAttr::getAlignmentType` for type alignment form.
- `VecTypeHintAttr::getTypeHint`.
- `TypeTagForDatatypeAttr::getMatchingCType`.
- Other Clang `TypeAttr` subclasses only after an explicit inventory.

This should wait until alias spelling vs canonical type id semantics are
decided. `attribute_arg_type.type_id` points to `@type`, not `@usertype` in the
dbscheme text, so P7f type work must choose how Arborchive maps type payloads
without losing important spelling.

### Attribute Arg Name

This is the least clear table. Potential name-like APIs include
`IdentifierInfo *` fields such as `ModeAttr::getMode`,
`FormatAttr::getType`, `AvailabilityAttr::getPlatform`, and
`AvailabilityAttr::getEnvironment`.

However, many apparent names are actually:

- string payloads (`AliasAttr`, `SectionAttr`);
- declaration references (`CleanupAttr::getFunctionDecl`);
- expression payloads (`EnableIfAttr`, `AnnotateAttr` extra args);
- enum-like fields better represented as stable text or constants.

Recommendation: defer `attribute_arg_name` until CodeQL extractor conventions
for name arguments are confirmed.

## 7. Candidate Clang Attr Matrix

| Attr class | Payload APIs observed | Classification | Recommended modeling |
|---|---|---|---|
| `DeprecatedAttr` | message, replacement strings | SAFE NOW | Message and replacement branches are implemented and covered. |
| `AlignedAttr` direct/shallow integer literal | `getAlignmentExpr` | SAFE NOW | Implemented for direct, parenthesized, implicit-cast, and shallow `ConstantExpr`-wrapped integer literals. |
| `AlignedAttr` expression such as `8 + 8` | `getAlignmentExpr` | SAFE SUBSET DONE | Implemented as `attribute_arg_expr` for non-literal, non-dependent expressions; do not fold silently. |
| `AlignedAttr` type form | `getAlignmentType` | SAFE ONLY AFTER TypeProcessor support | Candidate for `attribute_arg_type`; preserve type spelling policy first. |
| `AnnotateAttr` | annotation string, `Expr **args_`, delayed args | PARTIAL SAFE | Annotation string implemented; expr args defer. |
| `AnnotateTypeAttr` | annotation string, `Expr **args_`, delayed args | PARTIAL SAFE | Annotation string safe; owner/type placement and expr args defer. |
| `CleanupAttr` | `FunctionDecl *getFunctionDecl` | DEFER | Decl reference is not clearly value/name/expr; needs CodeQL mapping. |
| `ModeAttr` | `IdentifierInfo *getMode` | DEFER | Likely `attribute_arg_name`, but semantics unclear. |
| `NonNullAttr` | `ParamIdx` list | DEFER | Scalar index list lacks obvious `@expr` owner. |
| `FormatAttr` | `IdentifierInfo *type`, `int formatIdx`, `int firstArg` | DEFER | Mixes name-like and integer-like payloads; design indexes first. |
| `AvailabilityAttr` | platform/environment identifiers, versions, booleans, message/replacement strings | PARTIAL SAFE | Message/replacement strings safe later; platform/version/name fields defer. |
| `AllocSizeAttr` | `ParamIdx` elem/num params | DEFER | Integer-like but not necessarily `@expr`. |
| `AllocAlignAttr` | `ParamIdx` param index | DEFER | Same scalar-index issue. |
| `AssumeAlignedAttr` | alignment/offset `Expr *` | SAFE SUBSET DONE | Non-literal alignment/offset expressions are modeled as `attribute_arg_expr`; direct literal and broader constant policy remain deferred. |
| `EnableIfAttr` | condition `Expr *`, message string | PARTIAL SAFE | Message string safe only if condition remains deferred or separately modeled. |
| `DiagnoseIfAttr` | condition `Expr *`, message string, diagnostic enum | PARTIAL SAFE | Message string safe; condition and enum/name policy deferred. |
| `SectionAttr` | name string | SAFE NOW | Implemented for target-valid section names. |
| `WarnUnusedResultAttr` | message string | SAFE NOW | Implemented for non-empty `[[nodiscard("...")]]` messages; empty `[[nodiscard]]` remains a no-argument negative case. |
| `AliasAttr` | aliasee string | DEFER | Getter is stable, but current Darwin validation target rejects alias attributes; needs target-supported fixture. |
| Other visible string attrs | generated `StringRef` getters | AUDIT FIRST | Add one family at a time with tests and docs. |
| Other visible `Expr **args_` attrs | expression ranges | UNSAFE / DO NOT MODEL YET | Too broad for current ExprProcessor guarantees. |

## 8. Expr, Type, And Name Argument Risk Analysis

### Expression Arguments

Current `ExprProcessor` can create stable literal rows for integer literals via
`processAttributeIntegerLiteral`, and can create or reuse stable ids for the
selected non-literal `AlignedAttr` and `AssumeAlignedAttr` expression subsets.
It is not yet proven safe for arbitrary attribute expressions.

Risks:

- Normal traversal may not visit attribute expressions consistently.
- Explicit processing may duplicate rows if the expression is also visited.
- Attribute expressions can be unevaluated, constant-evaluated, dependent, or
  semantic-only depending on the Attr class.
- `aligned(8 + 8)` should not be silently converted into the same representation
  as direct `aligned(16)`. It likely needs either `attribute_arg_expr` or a
  carefully defined `CONSTANT_EXPR` path, possibly both if CodeQL does that.
- Dependent expressions should be skipped until expression dependency modeling
  is ready.

Recommendation: keep the implemented `AlignedAttr` and `AssumeAlignedAttr`
subsets narrow. Do not add `EnableIfAttr`, `DiagnoseIfAttr`, or `AnnotateAttr`
expression payloads until a separate design gate proves argument indexes,
mixed payload policy, and duplicate-expression behavior.

### Type Arguments

`TypeProcessor` can produce stable ids for many `QualType` and `TypeDecl`
inputs, including dependent-type fallback paths. That does not automatically
settle attribute type argument semantics.

Risks:

- Clang Attr APIs may expose `TypeSourceInfo`, `QualType`, or `Type *`; these
  have different spelling/canonicalization implications.
- Alias spelling may matter for CodeQL-style attribute arguments, but current
  type processing often canonicalizes or maps to user type ids.
- Dependent types can be represented, but P7f should not silently create
  broad dependent payload facts without tests.
- dbscheme says `attribute_arg_type.type_id` is `@type ref`; future work must
  verify whether Arborchive's `usertypes` rows satisfy that contract in this
  context.

Recommendation: P7f-3 should start with one TypeSourceInfo-backed Attr and
record whether the chosen id preserves spelling or canonical identity. Do not
mix this with owner-link work.

### Name Arguments

`attribute_arg_name` has unclear semantic boundaries. It stores a string, but
that does not mean every identifier-like or string-like payload belongs there.

Risks:

- `IdentifierInfo` fields may be true names, enum-like options, platform names,
  or format kinds.
- String payloads are already better represented by `attribute_arg_value`.
- Declaration references such as `CleanupAttr::getFunctionDecl` are not names
  unless CodeQL explicitly models them that way.
- Expression-backed names must remain expression payloads.

Recommendation: P7f-4 should be a design-only or one-Attr implementation task
after CodeQL semantics are confirmed. Good candidates to study are `ModeAttr`,
`FormatAttr`, and `AvailabilityAttr`; do not implement all three together.

## 9. Docs And Test Coverage Audit

Docs are mostly accurate after owner-link reinforcement:

- `docs/roadmap.md` clearly says P7 is PARTIAL and defers P7f.
- `docs/analysis/p7_attribute_owner_links.md` documents the owner-link safe
  subset and the canonical `ParmVarDecl` fix.
- `.trellis/tasks/p7-owner-links-goal/goal.md` is stale as a goal brief because
  it describes owner links as future work; keep it as historical task context,
  not current status.
- `.trellis/tasks/p7b-attribute-arguments-xhigh/` remains useful for the P7b
  rationale and risks.
- `docs/datatable-list.txt` should remain unchanged until a safe-subset DONE
  convention exists.

Stale command risk:

- `docs/analysis/p7_attribute_owner_links.md` correctly notes that
  `scripts/generate_database.sh` does not exist and that the current manual DB
  path is `build/demo -c config.example.toml -s ... -o ...`.
- Future docs should avoid referencing `generate_database.sh` unless the script
  is added.

Implemented behaviors covered by tests:

- Function attribute presence.
- Function owner links.
- Deprecated message string payload.
- Deprecated replacement string payload.
- Annotate annotation string payload.
- Section name string payload.
- Direct and shallow wrapped integer literal aligned payloads.
- `AlignedAttr` non-literal expression skip behavior for `8 + 8`.
- Type owner links for record/enum/alias.
- Var owner links for field/global/param/local.
- Stmt owner links for `fallthrough` and `likely`.

Behaviors currently more manual than asserted:

- Parameter canonical id reuse across parameter declaration, parameter
  attributes, and expression references.
- No duplicate `params` rows.
- Unsupported `AttributedStmt` wrappers skipped.
- P7f tables remain empty in owner-link fixture.
- Additional supported inner stmt kinds in `getSupportedAttributedStmtOwnerId`.
- Alias string payload on a target that supports `AliasAttr`.
- Availability string payload indexes.

Future P7f tests should be separate from owner-link tests. Owner-link tests
should prove stable ownership; argument tests should prove row kind, argument
index, typed payload, and deferred table emptiness where applicable.

## 10. Recommended Remaining P7 Roadmap

| Phase | Goal | Safe candidate attrs | Forbidden cases | Validation strategy | Expected populated tables | Expected empty tables | Risk |
|---|---|---|---|---|---|---|---|
| P7f-audit closeout | Land this audit and align docs. | None. | Behavior/schema/test changes. | `git diff --check`, doc diff, keyword scan. | None new. | No DB change. | Low |
| P7f-1 value expansion | Add stable string payloads only. | DONE for `DeprecatedAttr` message/replacement, `AnnotateAttr` annotation, `SectionAttr` name, and `WarnUnusedResultAttr` / `[[nodiscard("...")]]` message; next candidates require blockers to clear. | Expr args, type args, name args, raw dumps, target-unsupported `AliasAttr`. | Focused fixture plus SQLite joins for `attribute_args` and `attribute_arg_value`; full test suite. | `attribute_args`, `attribute_arg_value`. | `attribute_arg_type`, `attribute_arg_expr`, `attribute_arg_name`. | Low-Medium |
| P7f-2 constant safe subset | Validate direct/shallow literal constants. | DONE for direct, parenthesized, unsigned/implicit-cast `AlignedAttr`; non-literal `aligned(8 + 8)` is classified as expr. | APValue, arbitrary expression evaluation, ParamIdx as raw scalar. | Focused fixture plus SQLite joins for `attribute_arg_constant`; full test suite. | `attribute_args`, `attribute_arg_constant`. | `attribute_arg_type`, `attribute_arg_name`; no non-literal expr folded into constant. | Medium |
| P7f-3 expr minimal subset | Define and implement expression-backed safe subsets. | DONE for non-literal, non-dependent `AlignedAttr` and `AssumeAlignedAttr` expressions using `ExprProcessor::getOrProcessExprId`. | Broad `Expr **args_`, APValue dumps, EnableIf/DiagnoseIf/Annotate until design gate; literal `AssumeAlignedAttr` constants until constant policy. | Prove no duplicate expr rows; inspect `exprs`, value tables, `attribute_arg_expr`. | `attribute_args`, `attribute_arg_expr`. | `attribute_arg_type`, `attribute_arg_name`; dependent expr payloads. | High |
| P7f-4 type minimal subset | Define and implement one TypeSourceInfo-backed Attr. | No safe implementation in this checkpoint: `@type`/`@usertype` and spelling/canonical semantics remain open. | Dependent types, broad TypeAttr sweep, canonical/spelling ambiguity. | SQL for `attribute_arg_type` and referenced type rows; alias/canonical fixture. | None yet. | `attribute_arg_type`. | High |
| P7f-5 name design | Confirm CodeQL name-argument semantics. | Study `ModeAttr`, `FormatAttr`, `AvailabilityAttr`. | Treating strings/decl refs/exprs as names without proof. | Prefer design doc first; if implemented, one Attr and one fixture. | None yet. | `attribute_arg_name`. | High |
| P7f-5 generalized constants | Audit `CONSTANT_EXPR`, APValue, ParamIdx, scalar int fields. | Maybe `FormatAttr` indexes after expression-row policy. | Raw scalar integer insertion into `@expr ref` table. | Dedicated SQL evidence for expression/value rows and kind mapping. | `attribute_arg_constant` or future `CONSTANT_EXPR` mapping. | Type/name unless in scope. | High |
| P7 final docs decision | Decide datatable-list convention and final P7 status. | All P7 tables only after semantics and coverage are complete. | Marking DONE while P7f remains deferred. | Full suite plus P7 DB matrix checks. | All intended P7 tables. | None unless explicitly out of scope. | Medium |

## 11. Validation Strategy For Future Implementation

Minimum for docs-only P7 audit work:

```bash
git diff --check
git diff -- docs/analysis/p7_remaining_attribute_scope_audit.md
rg "P7 overall|PARTIAL|attribute_arg_type|attribute_arg_expr|attribute_arg_name|ParmVarDecl|generate_database.sh|build/demo" docs/analysis/p7_remaining_attribute_scope_audit.md
```

Minimum for future behavior changes:

```bash
python3 scripts/generate_instantiations.py
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
scripts/test_all.sh
```

Focused DB checks for owner links:

```sql
select "attributes", count(*) from attributes
union all select "typeattributes", count(*) from typeattributes
union all select "varattributes", count(*) from varattributes
union all select "stmtattributes", count(*) from stmtattributes
union all select "attribute_arg_type", count(*) from attribute_arg_type
union all select "attribute_arg_expr", count(*) from attribute_arg_expr
union all select "attribute_arg_name", count(*) from attribute_arg_name;
```

Duplicate parameter regression:

```sql
select function, "index", type_id, count(), group_concat(id)
from params
group by function, "index", type_id
having count() > 1;
```

Use `build/demo -c config.example.toml -s <fixture> -o <db>` for focused DB
generation in this checkout. Do not refer to `generate_database.sh` unless that
script exists.

Recent P7f SQLite evidence:

```text
attribute_string_args_case:
annotate|1|0|p7f_annotation
deprecated|1|0|use p7f_replacement_target
deprecated|1|1|p7f_replacement_target
section|1|0|__DATA,p7f_section
complex|0|0|0|0

attribute_constant_args_case:
p7f_aligned_direct|aligned|2|0|16
p7f_aligned_paren|aligned|2|0|32
p7f_aligned_unsigned|aligned|2|0|64
attributes|3
attribute_args|3
attribute_arg_constant|3
attribute_arg_type|0
attribute_arg_expr|0
attribute_arg_name|0

attribute_expr_args_case:
p7f_expr_aligned_add|aligned|5|0|expr-kind 25
p7f_expr_aligned_wrapped_mul|aligned|5|0|expr-kind 27
p7f_expr_aligned_literal|aligned|2|0|16
dependent template aligned attribute has no payload row
attribute_arg_constant|1
attribute_arg_expr|2
attribute_arg_type|0
attribute_arg_name|0

attribute_assume_aligned_expr_args_case:
attributes|3
funcattributes|3
attribute_args|2
attribute_arg_expr|2
attribute_arg_constant|0
attribute_arg_type|0
attribute_arg_name|0
assume_aligned|gnu|0|5|expr-kind 25
assume_aligned|gnu|1|5|expr-kind 25
literal and dependent assume_aligned expressions have no payload row
```

## 12. Final Recommendation

Keep P7 marked PARTIAL. Treat P7a/P7b, P7c/P7d owner links, the narrow P7f
string/constant payload subsets, and the minimal non-literal `AlignedAttr` and
`AssumeAlignedAttr` expression subsets as safe-subset done. Treat
`stmtattributes` as conservative partial safe subset, and keep complex
`attribute_arg_type`, broader `attribute_arg_expr`, and `attribute_arg_name`
work deferred.

The checkpoint-chain decision is to keep P7 PARTIAL and leave
`docs/datatable-list.txt` unchanged. The safest next implementation task is a
fresh, narrow design gate for one remaining deferred family, such as broader
expression payloads or type payloads, with its own positive/negative fixture
and SQLite evidence. Remaining `EnableIfAttr`, `DiagnoseIfAttr`, and
`AnnotateAttr` expression payloads should not be implemented until their
semantic risks above are resolved.
