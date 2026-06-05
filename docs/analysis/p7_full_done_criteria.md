# P7 Full DONE Criteria And CodeQL Parity Spec

This document defines the review standard for marking P7 Attribute System
tables full `DONE`. It is a criteria and parity document only. It does not
implement behavior, change schema/model/table initialization, modify tests, or
mark P7 as `DONE`.

## 1. Verdict

P7 is not full `DONE` yet.

The current implementation is a strong CodeQL-shaped safe subset:

- explicit supported attribute presence rows;
- owner links for functions, selected type declarations, variables, and
  conservative attributed statements;
- stable string payloads for selected Clang attributes;
- direct and shallow-wrapped integer literal constants for selected
  `AlignedAttr` arguments.
- minimal non-literal expression payloads for selected `AlignedAttr` and
  `AssumeAlignedAttr` arguments.

Full `DONE` requires CodeQL parity criteria and validation for the whole P7
table family, especially the typed `AttributeArgument` payload families:

- `attribute_arg_value`;
- `attribute_arg_constant`;
- `attribute_arg_type`;
- `attribute_arg_expr`;
- `attribute_arg_name`.

No P7 table should be treated as full `DONE` merely because it has rows. Full
`DONE` means the rows implement semantically correct CodeQL-shaped behavior,
have positive and negative tests, have SQLite join evidence, and have
documented unsupported/deferred policy.

## 2. CodeQL Parity Anchors

The P7 full-DONE target is shaped by the public CodeQL C/C++ `Attribute` and
`AttributeArgument` APIs:

- `AttributeArgument.getAttribute()`;
- `AttributeArgument.getIndex()`;
- `AttributeArgument.getName()`;
- `AttributeArgument.getValueConstant()`;
- `AttributeArgument.getValueExpr()`;
- `AttributeArgument.getValueInt()`;
- `AttributeArgument.getValueText()`;
- `AttributeArgument.getValueType()`;
- `Attribute.getArgument()`;
- `Attribute.getAnArgument()`;
- `Attribute.getName()`.

Important semantic anchors:

- `Attribute.getName()` excludes the attribute namespace.
- `AttributeArgument.getValueText()` is text for the value if the value is a
  constant or token. It is not an arbitrary source spelling dump.
- `AttributeArgument.getName()` represents named arguments. CodeQL documents
  named arguments as a Microsoft feature, so only Microsoft-style attributes
  should be treated as the primary candidate for `attribute_arg_name`.
- Clang `Attr::isImplicit()` must remain excluded from explicit source
  attribute rows unless a future policy explicitly says otherwise.
- `AnnotateAttr` can expose a stable annotation string and may also expose
  expression arguments. String payloads and expression payloads must remain
  distinct.

## 3. Current Implemented Safe Subset

Current P7 implementation status, based on the local roadmap, P7 audits,
schema/model/table definitions, processor code, and P7 fixtures:

- `attributes` rows are recorded for explicit non-implicit attrs with supported
  Clang syntax kinds: GNU, standard, declspec, Microsoft, and alignas.
- `funcattributes` is populated from function declarations after function id
  resolution.
- `typeattributes` is populated for stable record, enum, typedef, and type
  alias type ids.
- `varattributes` is populated for field, global, local, and parameter variable
  entity ids.
- `stmtattributes` is populated for conservative `AttributedStmt` owner links
  using supported inner statement ids.
- `attribute_args` rows are created only for currently supported payloads.
- `attribute_arg_value` currently records stable strings for:
  - `DeprecatedAttr` message and replacement;
  - `AnnotateAttr` annotation;
  - `SectionAttr` section name;
  - `WarnUnusedResultAttr` / `[[nodiscard("...")]]` message.
- `attribute_arg_constant` currently records direct and shallow-wrapped
  integer-literal `AlignedAttr` payloads through stable `@expr` literal ids.
- `attribute_arg_expr` currently records minimal non-literal `AlignedAttr` and
  `AssumeAlignedAttr` expression subsets through stable `@expr` ids.
- P7a/P7b behavior is preserved by the focused P7 fixtures.
- The canonical `ParmVarDecl` fix reuses the canonical parameter variable
  entity id for parameter attributes and avoids duplicate parameter owner ids.

Current deferred areas:

- `attribute_arg_type`;
- broader `attribute_arg_expr` families beyond the selected non-literal
  `AlignedAttr` and `AssumeAlignedAttr` safe subsets;
- `attribute_arg_name`;
- generalized `CONSTANT_EXPR` and APValue handling;
- dependent/template attribute arguments;
- target-unsupported alias strings;
- broad Clang `Attr` argument modeling;
- final CodeQL wrapper-vs-inner semantics for statement attributes.

## 4. Full DONE Criteria By Table

The table below defines the full-DONE bar. `Current state meets full DONE` is
intentionally strict: safe-subset coverage is not full CodeQL parity.

| Table | Current status | Full DONE requirement | Positive fixture requirement | Negative/deferred fixture requirement | SQLite evidence requirement | Current state meets full DONE |
| --- | --- | --- | --- | --- | --- | --- |
| `attributes` | Populated for explicit non-implicit attrs with supported syntax kind and split name/namespace. | Match CodeQL `Attribute` identity, kind, name-without-namespace, namespace, and location semantics for all P7-supported attribute forms. Preserve implicit-attr policy. | GNU, standard, alignas, declspec, Microsoft, namespaced standard attrs, source locations, and duplicate spellings where CodeQL distinguishes them. | Implicit attrs skipped; unsupported syntax skipped or documented; macro/location edge cases covered or deferred. | Counts by `kind`; join to `locations_default`; rows proving `name` excludes namespace and `name_space` carries scope. | No. Safe subset only. |
| `funcattributes` | Populated from `VisitFunctionDecl` after function id resolution. | Link every CodeQL-relevant function-like attribute owner to the stable `@function` id expected by Arborchive/CodeQL parity. | Free functions, methods, constructors, destructors, conversion operators, deduction guides if CodeQL expects them, and redeclarations. | Non-function attrs skipped; implicit attrs skipped; unsupported function-like entities documented. | Join `funcattributes.spec_id -> attributes.id` and `funcattributes.func_id -> functions.id`; count by function-like owner kind. | No. Constructors/destructors/conversions/deduction guide coverage is not proven. |
| `typeattributes` | Populated for records, enums, typedefs, and type aliases. | Link every CodeQL-relevant type owner to the correct stable `@type` entity without confusing owner attributes with type-valued arguments. | Record, enum, typedef, using alias, and attributed type spelling cases if supported. | Dependent, incomplete, and unsupported type-like attrs skipped or documented. | Join `typeattributes.spec_id -> attributes.id`; verify referenced ids exist in Arborchive type tables; alias/canonical evidence where relevant. | No. Safe subset only. |
| `varattributes` | Populated for fields, globals, locals, and parameters using variable entity ids. | Link every CodeQL-relevant accessible variable owner to the stable `@accessible` family id, including canonical parameter behavior. | Field, global, local, parameter, redeclared variables where relevant, and joins to `var_decls` plus owner-specific tables. | Duplicate parameter rows rejected; unsupported/implicit variable attrs skipped; macro owner cases covered or deferred. | Join to `membervariables`, `globalvariables`, `localvariables`, and `params`; duplicate `ParmVarDecl` query must return zero rows. | No. Safe subset only, with manual duplicate-param evidence needing dedicated regression. |
| `stmtattributes` | Populated for conservative `AttributedStmt` inner owners such as `fallthrough` and `likely`. | Match CodeQL statement owner semantics for attributed statements, including whether CodeQL owns the wrapper or the inner statement. | `fallthrough`, `likely`, and each supported inner statement kind with joins to `stmts`. | Unsupported attributed statements remain skipped; wrapper-vs-inner ambiguity explicitly tested; duplicate rows rejected. | Join `stmtattributes.stmt_id -> stmts.id`; rows proving chosen wrapper or inner statement id; unsupported-wrapper fixture keeps count unchanged. | No. Conservative partial. |
| `attribute_args` | Populated only for supported safe-subset args. | Represent CodeQL `AttributeArgument` identity, attribute back-reference, index, kind, and location for all supported typed payloads. | Multiple args, correct zero-based indexes, empty/token/constant/type/constant-expr/expr kind mapping, and `getAttribute()` joins. | Unsupported/dependent args skipped; no orphan payload rows; no duplicate indexes for the same semantic arg. | Join `attribute_args.attribute -> attributes.id`; per-Attr ordered index checks; kind counts matching payload tables. | No. Safe subset only. |
| `attribute_arg_value` | Populated for whitelisted stable strings. | Populate only CodeQL value-text/token payloads from stable string/text/token semantics, not arbitrary spelling dumps. | Deprecated message/replacement, Annotate annotation, Section name, WarnUnusedResult/nodiscard message, and future target-supported stable strings with index checks. | Strings that are names, decl refs, types, or exprs must not land here; target-unsupported attrs skipped. | Join to `attribute_args`; verify `kind` is token/value-compatible; value rows match stable expected text. | No. Safe subset only. |
| `attribute_arg_constant` | Populated for direct/shallow-wrapped `AlignedAttr` integer literals via stable `@expr` ids. | Populate CodeQL constant payloads through stable constant/integer semantics and expression/value facts, with clear `CONSTANT` vs `CONSTANT_EXPR` policy. | Direct integer literal, shallow wrappers, and future scalar/index constants only after `@expr` or value-int parity policy exists. | Non-literal `aligned(8 + 8)` must not be folded into this table; APValue, dependent constants, and raw scalar fields skipped until policy; no fake `@expr` refs. | Join `attribute_arg_constant.constant -> exprs.id`; verify literal value/text rows; non-literal fixture has no constant row for the same argument. | No. Safe subset only. |
| `attribute_arg_type` | Table exists, not populated. | Map CodeQL `getValueType()` to stable `@type` ids without `getAsString()` fallback and with documented spelling/canonical semantics. | At least one TypeSourceInfo-backed Attr, alias vs canonical fixture, and joins to referenced type rows. | Dependent types skipped; unsupported TypeAttr families skipped; no string fallback. | Join `attribute_arg_type.type_id` to referenced type table; fixture proving spelling/canonical decision. | No. Deferred. |
| `attribute_arg_expr` | Minimal non-literal `AlignedAttr` subset populated. | Map CodeQL `getValueExpr()` to stable `@expr` ids with no duplicate expression rows and clear dependent/unevaluated/constant policy. | At least one expression-backed Attr, non-literal expression, joins to `exprs` and payload rows, and no duplicate rows. | Dependent expressions skipped; broad `Expr **args_` skipped until safe; `aligned(8 + 8)` strategy tested. | Join `attribute_arg_expr.expr -> exprs.id`; duplicate-expression query; rows proving constant-vs-expr classification. | No. Minimal safe subset only. |
| `attribute_arg_name` | Table exists, not populated. | Match CodeQL `getName()` semantics for named arguments, primarily Microsoft named arguments, and never use it for arbitrary identifiers. | Microsoft named argument fixture if available, with correct name, index, and owning attribute. | Mode/Format/Availability identifiers, strings, decl refs, and exprs not modeled as names unless parity is proven. | Join `attribute_arg_name.arg -> attribute_args.id`; verify attribute kind is Microsoft-supported and unrelated identifier-like payloads stay empty. | No. Deferred. |

## 5. AttributeArgument Typed Payload Semantics

### Value/Text Payloads

`attribute_arg_value` may be populated only from stable value/text semantics:

- stable Clang string getters such as selected message, replacement,
  annotation, or section-name APIs;
- token/text payloads that CodeQL would expose through value-text semantics;
- source-stable payloads with documented argument index.

`AttributeArgument.getValueText()` is text for a constant or token value. It is
not permission to dump arbitrary source spelling, `QualType::getAsString()`,
`Expr::printPretty()`, declaration names, or target-specific debug text.

It must not be used as a catch-all for:

- expressions;
- types;
- declaration references;
- arbitrary `getAsString()` dumps;
- platform/name identifiers unless CodeQL treats them as text values rather
  than names.

### Constant Payloads

`attribute_arg_constant.constant` is an `@expr` reference in the local
dbscheme. It therefore needs a stable expression row, not a raw integer column.

Current safe support is intentionally narrow:

- direct integer literal;
- parenthesized or implicit-cast literal;
- shallow `ConstantExpr` wrapping an integer literal.

Future expansion must define:

- whether APValue-backed constants are represented;
- how `CONSTANT_EXPR` kind differs from `CONSTANT`;
- whether scalar Clang fields such as `ParamIdx` can be mapped to source
  `@expr` rows;
- whether folded expressions create constant payloads, expression payloads, or
  both.

Constants must use stable constant/integer semantics. They must not be
implemented by stringifying expressions or by inventing expression ids for
scalar fields that do not have source expression identity.

### Expression Payloads

`attribute_arg_expr` is for CodeQL `getValueExpr()` semantics. Expressions must
map to stable `@expr` ids. They must not be serialized as strings.

Before implementation, Arborchive must prove:

- `ExprProcessor` can create or reuse stable ids for attribute expressions;
- explicit attribute-expression processing does not duplicate normal traversal
  output;
- dependent expressions are skipped;
- unevaluated and constant-evaluated contexts are classified;
- negative cases remain empty.

### Type Payloads

`attribute_arg_type` is for CodeQL `getValueType()` semantics. Types must map
to stable `@type` ids. They must not use `QualType::getAsString()` as a fallback
payload.

Before implementation, Arborchive must document:

- whether the source API is `TypeSourceInfo`, `QualType`, or `Type *`;
- whether spelling or canonical identity is required;
- whether current `@usertype` ids satisfy the dbscheme `@type` reference in
  this context;
- how aliases, typedefs, attributed types, and dependent types are handled.

Dependent/template type payloads should be skipped unless a future policy
proves they are CodeQL-compatible and reviewable.

### Name Payloads

`attribute_arg_name` is for CodeQL named argument semantics. It must not be
used for arbitrary identifier-like payloads merely because the table stores a
string.

Policy:

- Microsoft named arguments are the primary supported target.
- String payloads remain `attribute_arg_value`.
- Declaration references remain deferred until CodeQL parity is proven.
- `ModeAttr`, `FormatAttr`, and `AvailabilityAttr` identifiers must not be
  modeled as names unless CodeQL parity confirms they are named arguments.
- Dependent/template names are skipped until a future policy proves safety.

## 6. Candidate Attr Classification Matrix

| Attr family | Likely payload kind | Current implementation status | Safe next step | Full DONE blocker | Target table(s) | Risk |
| --- | --- | --- | --- | --- | --- | --- |
| `DeprecatedAttr` | Message string, replacement string. | Implemented as value safe subset. | Keep regression coverage for both indexes and owner joins. | Broader spelling/index parity and owner coverage. | `attribute_args`, `attribute_arg_value`. | Low. |
| `AnnotateAttr` | Annotation string plus possible expression args. | Annotation string implemented; expression args deferred. | Keep annotation string distinct from expr args. | Expr arg policy and delayed/dependent args. | `attribute_arg_value`; future `attribute_arg_expr`. | Medium. |
| `SectionAttr` | Section name string. | Implemented as value safe subset. | Keep target-valid fixture coverage. | Target/macro spelling and index parity. | `attribute_args`, `attribute_arg_value`. | Low. |
| `WarnUnusedResultAttr` | Message string. | Implemented as value safe subset for non-empty `[[nodiscard("...")]]` messages. | Keep empty `[[nodiscard]]` as a no-argument negative case. | Broader spelling parity for GNU/Clang warn_unused_result forms. | `attribute_args`, `attribute_arg_value`. | Low. |
| `AliasAttr` | Aliasee string. | Not implemented. | Audit on a target where Clang accepts alias attrs. | Current Darwin validation target can reject aliases. | Candidate `attribute_arg_value`. | Medium. |
| `AvailabilityAttr` | Platform/environment identifiers, versions, message/replacement strings. | Not implemented. | Design argument index map before implementing message/replacement. | Mixed names, versions, booleans, strings, and possible named fields. | Candidate `attribute_arg_value`; possible future constant/name only after parity. | High. |
| `AlignedAttr` | Alignment expression or type form. | Integer literal constant safe subset and non-literal expr subset implemented. | Keep direct literals in constants and non-literal source expressions in expr rows. | Broader constant-expr/type semantics. | `attribute_arg_constant`, `attribute_arg_expr`; future `attribute_arg_type`. | High. |
| `NonNullAttr` | Parameter index list. | Not implemented. | Audit CodeQL value-int/constant semantics for parameter indexes. | `attribute_arg_constant` expects `@expr`; raw `ParamIdx` is unsafe. | Candidate constant family only after policy. | Medium-High. |
| `FormatAttr` | Format type identifier, format index, first arg index. | Not implemented. | Design indexes and CodeQL meaning of format type identifier. | Mix of identifier-like and scalar integer payloads. | Candidate constant family; name/value only after parity. | High. |
| `AllocSizeAttr` | Element/number parameter indexes. | Not implemented. | Audit scalar-index representation. | `ParamIdx` is not necessarily a source `@expr`. | Candidate constant family only after policy. | Medium-High. |
| `AllocAlignAttr` | Parameter index. | Not implemented. | Audit scalar-index representation. | Same `ParamIdx`/`@expr` issue. | Candidate constant family only after policy. | Medium-High. |
| `AssumeAlignedAttr` | Alignment expr, offset expr. | Non-literal alignment/offset expr subset implemented; literal and constant policy deferred. | Keep non-literal expr rows separate from literals. | Broader constant classification and full literal/offset policy. | `attribute_arg_expr`; maybe future `attribute_arg_constant` for literals. | High. |
| `EnableIfAttr` | Condition expr, message string. | Not implemented. | Design condition expr first or explicitly defer it while modeling message. | Partial string-only modeling may hide missing expr semantics. | Candidate `attribute_arg_expr` and `attribute_arg_value`. | High. |
| `DiagnoseIfAttr` | Condition expr, message string, diagnostic kind. | Not implemented. | Same split as `EnableIfAttr`; decide diagnostic kind policy. | Mixed expr/string/enum semantics. | Candidate `attribute_arg_expr` and `attribute_arg_value`; name only with proof. | High. |
| `CleanupAttr` | Function declaration reference. | Not implemented. | Design CodeQL mapping for decl refs. | Decl ref is not text/name/expr/type by default. | Deferred; no target table without parity. | High. |
| `ModeAttr` | Identifier-like mode. | Not implemented. | Design-only audit against CodeQL `getName()` and value-text behavior. | Identifier may be option text, not named arg. | Deferred; possible `attribute_arg_name` only with proof. | High. |
| `AnnotateTypeAttr` | Annotation string plus possible expression args. | Not implemented. | Audit owner/type placement and stable string getter. | Type-owner vs type-argument boundary; expr args. | Candidate `attribute_arg_value`; future `attribute_arg_expr` or owner/type path after design. | High. |
| Microsoft attributes with named args | Named arguments, possibly typed values. | Not implemented. | Inventory Clang Microsoft Attr APIs and create one fixture if available. | Need CodeQL `getName()` parity and compiler support. | Primary candidate `attribute_arg_name`; value/constant/expr/type table depends on value kind. | High. |

## 7. Expr Argument DONE Criteria

`attribute_arg_expr` can be implemented only after these conditions are true:

- `ExprProcessor` exposes a safe path to create or reuse stable `@expr` ids for
  attribute expressions.
- Attribute expression processing does not create duplicate `exprs`, value
  rows, or parent/child rows when the same expression is also visited normally.
- Dependent expressions are skipped.
- Unevaluated, constant-evaluated, and ordinary expression contexts are
  classified correctly.
- The project has a documented rule for `aligned(8 + 8)`: represent as
  `attribute_arg_expr`, represent as `attribute_arg_constant_expr`, represent as
  both if CodeQL does, or defer.
- Direct literal constants remain distinct from non-literal expressions.
- `AnnotateAttr` expression args are not collapsed into annotation strings.
- Positive fixtures cover at least one supported expression-backed Attr.
- Negative fixtures cover dependent expressions, unsupported `Expr **args_`
  families, and skipped non-literal cases where policy says defer.
- SQLite evidence joins `attribute_args`, `attribute_arg_expr`, `exprs`, and
  relevant value/relationship tables.

The minimal non-literal `AlignedAttr` subset now satisfies these conditions for
one candidate only. Broader `attribute_arg_expr` remains deferred until the same
evidence exists for each additional attribute family.

## 8. Type Argument DONE Criteria

`attribute_arg_type` can be implemented only after these conditions are true:

- The source API for each supported Attr is pinned down:
  `TypeSourceInfo`, `QualType`, or `Type *`.
- `TypeProcessor` returns a stable `@type` id for the chosen source without a
  string fallback.
- The `@type` vs `@usertype` policy is documented for attribute type payloads.
- Canonical vs spelling type policy is documented, including aliases and
  typedefs.
- Dependent types are skipped unless a future policy proves they are safe and
  CodeQL-compatible.
- Attributed type spelling is not confused with owner links in
  `typeattributes`.
- Positive fixtures include at least one TypeSourceInfo-backed Attr and an
  alias/canonical distinction case.
- Negative fixtures prove dependent/unsupported types remain skipped.
- SQLite evidence joins `attribute_args`, `attribute_arg_type`, and referenced
  type rows.

Until those are true, `attribute_arg_type` remains deferred.

## 9. Name Argument DONE Criteria

`attribute_arg_name` can be implemented only after these conditions are true:

- The CodeQL meaning of `AttributeArgument.getName()` is pinned down in project
  docs.
- Microsoft named arguments are confirmed as the primary supported target.
- A Clang API inventory identifies which Microsoft attrs expose named
  arguments and values in a source-stable way.
- String payloads are not misclassified as names.
- Declaration references are not misclassified as names.
- Expression-backed identifiers are not misclassified as names.
- `ModeAttr`, `FormatAttr`, and `AvailabilityAttr` are not modeled as names
  unless CodeQL parity proves they are named arguments.
- Positive fixtures cover one Microsoft named-argument case if available on the
  validation toolchain.
- Negative fixtures prove ordinary string, identifier, decl-ref, and expr
  payloads do not populate `attribute_arg_name`.
- SQLite evidence joins `attribute_args` and `attribute_arg_name` and confirms
  the expected index.

Until those are true, `attribute_arg_name` remains deferred.

## 10. Generalized Constant/APValue DONE Criteria

Generalized constants and APValue-backed arguments can be implemented only
after these conditions are true:

- The project documents the CodeQL distinction between
  `attribute_arg_constant` and `attribute_arg_constant_expr`.
- Direct integer literals, shallow literal wrappers, folded constant
  expressions, and APValue-only constants have separate policies.
- Constant payloads use stable constant/integer semantics, not arbitrary source
  text or pretty-printed expressions.
- Any payload stored in `attribute_arg_constant.constant` has a real stable
  `@expr` id, because the dbscheme column is `@expr ref`.
- Scalar Clang fields such as `ParamIdx`, `FormatAttr` indexes,
  `AllocSizeAttr` indexes, and `AllocAlignAttr` indexes are not inserted as
  fake expression refs.
- `AttributeArgument.getValueInt()` parity is audited before scalar index
  families are modeled.
- Dependent/template constants remain skipped unless a future policy proves
  they are safe and CodeQL-compatible.
- Positive fixtures cover each constant family separately.
- Negative fixtures prove non-literal expressions such as `aligned(8 + 8)`
  follow the documented expr, constant, both, or deferred strategy.
- SQLite evidence joins `attribute_args`, `attribute_arg_constant`, `exprs`,
  and literal/value tables and proves no orphan or fake expr ids exist.

Until those are true, generalized `CONSTANT_EXPR`, APValue, and scalar index
families remain deferred.

## 11. Owner-Link DONE Criteria

Owner links are a strong safe subset today, but full `DONE` needs the remaining
parity questions closed:

- `stmtattributes` must decide wrapper-vs-inner CodeQL parity for
  `AttributedStmt`.
- Each supported inner statement family in
  `StmtProcessor::getSupportedAttributedStmtOwnerId` must have positive
  coverage, or unsupported families must be explicitly removed/deferred.
- Unsupported attributed statements should remain skipped unless a future
  CodeQL parity review proves their owner semantics and stable statement ids.
- Broader function-like declaration coverage must be audited if CodeQL expects
  attributes on constructors, destructors, conversion operators, deduction
  guides, methods, or other function entities beyond the current fixture
  coverage.
- Type owner coverage must decide whether enum constants, attributed type
  spellings, namespace-scoped type forms, and other attributed entities are in
  P7 owner scope or deferred.
- Variable owner coverage must preserve the canonical parameter id behavior and
  prove no duplicate `params` rows.
- If CodeQL expects attributes on enum constants, labels, namespaces, using
  declarations, template parameters, or other declarations, P7 must either
  implement them through the owning subsystem or document them as deferred.
- Macro expansion and location policy must be documented.
- Implicit attributes must remain excluded from explicit source attrs unless a
  future explicit policy changes this.
- `ASTVisitor` must remain dispatch-oriented; new owner coordination belongs in
  processors/helpers, not visitor schema orchestration.

## 12. Required Validation Matrix For Full DONE

Future full-DONE validation must include:

- focused fixtures for every typed payload table:
  `attribute_arg_value`, `attribute_arg_constant`, `attribute_arg_type`,
  `attribute_arg_expr`, and `attribute_arg_name`;
- focused fixtures for generalized constants/APValue only after that policy is
  approved;
- positive and negative cases for each supported Attr family;
- SQLite counts for P7 tables;
- SQLite joins proving each payload row has a valid `attribute_args` row and a
  valid referenced `@attribute`, `@expr`, or `@type` row;
- P7a/P7b regression coverage;
- owner-link regression coverage for functions, types, variables, and
  statements;
- duplicate `ParmVarDecl` query coverage;
- unsupported/dependent cases proving deferred tables remain empty when they
  should;
- macro/location checks when location semantics are in scope;
- no orphan rows in payload tables;
- `scripts/test_all.sh` coverage for behavior changes;
- ORM regeneration and debug build if schema/model/table initialization ever
  changes.

Representative full-DONE SQL evidence should include:

```sql
select count(*) from attributes;
select count(*) from attribute_args;
select count(*) from attribute_arg_value;
select count(*) from attribute_arg_constant;
select count(*) from attribute_arg_type;
select count(*) from attribute_arg_expr;
select count(*) from attribute_arg_name;
```

```sql
select aa.id, aa.attribute, aa."index", aa.kind, av.value
from attribute_args aa
join attribute_arg_value av on av.arg = aa.id;
```

```sql
select aa.id, aa.attribute, aa."index", aa.kind, ac.constant
from attribute_args aa
join attribute_arg_constant ac on ac.arg = aa.id
join exprs e on e.id = ac.constant;
```

```sql
select aa.id, aa.attribute, aa."index", aa.kind, at.type_id
from attribute_args aa
join attribute_arg_type at on at.arg = aa.id;
```

```sql
select aa.id, aa.attribute, aa."index", aa.kind, ae.expr
from attribute_args aa
join attribute_arg_expr ae on ae.arg = aa.id
join exprs e on e.id = ae.expr;
```

```sql
select aa.id, aa.attribute, aa."index", aa.kind, an.name
from attribute_args aa
join attribute_arg_name an on an.arg = aa.id;
```

```sql
select function, "index", type_id, count(), group_concat(id)
from params
group by function, "index", type_id
having count() > 1;
```

The duplicate parameter query must return zero rows.

## 13. Recommended Implementation Sequence

Recommended sequence for closing P7 without mixing roadmap phases:

1. P7-full-criteria docs closeout.
2. P7f-expr design audit.
3. P7f-expr minimal safe subset.
4. P7f-type design audit.
5. P7f-type minimal safe subset.
6. P7f-name Microsoft named args audit.
7. P7f-name minimal safe subset only if semantics are clear.
8. P7 constant family expansion for `NonNullAttr`, `FormatAttr`, `AllocSizeAttr`,
   `AllocAlignAttr`, and related scalar/index families only after an `@expr`
   or value-int parity policy exists.
9. P7 final parity review across all owner links and typed payload tables.
10. `docs/datatable-list.txt` DONE decision.

This order intentionally keeps expression, type, name, constant expansion, and
final status decisions separate.

## 14. Final Recommendation

P7 remains `PARTIAL`. The current safe subset is strong and reviewable, but it
is not full `DONE`.

Safe-subset complete now:

- explicit supported attribute presence;
- `funcattributes` for current function coverage;
- type owner links for stable record/enum/alias ids;
- variable owner links for field/global/local/parameter ids;
- conservative statement owner links for currently validated attributed
  statements;
- stable string payloads for `DeprecatedAttr`, `AnnotateAttr`, and
  `SectionAttr`;
- direct and shallow-wrapped integer-literal `AlignedAttr` constants;
- minimal non-literal `AlignedAttr` and `AssumeAlignedAttr` expressions in
  `attribute_arg_expr`;
- preserved P7a/P7b behavior;
- canonical `ParmVarDecl` owner-id fix.

Still deferred:

- broader `attribute_arg_expr` families beyond those safe subsets;
- `attribute_arg_type`;
- `attribute_arg_name`;
- generalized `CONSTANT_EXPR` and APValue;
- dependent/template attribute arguments;
- broad Clang Attr argument modeling;
- Microsoft named-argument support;
- target-sensitive alias payload validation;
- broader owner-link parity.

Full `DONE` is blocked by typed argument semantics, owner-link parity gaps,
negative/deferred tests, SQLite join evidence, and final docs/status policy.

`docs/datatable-list.txt` should remain unchanged for now. Marking P7 tables
`DONE` before these criteria are met would overstate the implementation.

The next implementation task should be the P7f expr broader design gate. Do not
add EnableIf/DiagnoseIf/Annotate/AssumeAligned expression payloads until their
expression-id, duplicate-row, dependent-expression, and argument-index policies
are written and reviewable.
