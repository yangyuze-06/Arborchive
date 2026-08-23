# P7f Attribute Expr Design Audit

This document audits whether Arborchive can safely populate
`attribute_arg_expr` for Clang attribute expression arguments and records the
implemented safe checkpoints. This document does not change schema/model/table
initialization, change `docs/datatable-list.txt`, or mark P7 `DONE`.

## 1. Verdict

Only selected non-literal `AlignedAttr` and `AssumeAlignedAttr` expression
subsets should be implemented at this stage. Broader `attribute_arg_expr`
support should not be implemented yet.

The schema target is clear: `attribute_arg_expr.arg` identifies an
`@attribute_arg`, and `attribute_arg_expr.expr` identifies an `@expr`. That
matches the CodeQL-shaped `AttributeArgument.getValueExpr()` concept for
arguments whose value is an expression.

The current implementation is still not safe for arbitrary attribute
expression payloads because:

- `ExprProcessor::getOrProcessExprId()` is proven only for existing expression
  extraction paths and the selected `AlignedAttr` / `AssumeAlignedAttr`
  candidates.
- Several expression-backed attributes have mixed string/expr/enum payloads and
  need explicit argument-index policy before insertion.
- Clang `RecursiveASTVisitor` traverses declaration attributes and expression
  children, so each new candidate still needs duplicate-row SQL evidence.
- Dependent, unevaluated, constant-evaluated, and broad `Expr **args_` families
  need per-attribute policy.
- `attribute_arg_expr` must not be confused with `attribute_arg_constant`,
  `attribute_arg_value`, or `attribute_arg_type`.

Safe next step: keep the remaining expression-backed families deferred and
move to the separate type-argument design gate. Do not sweep all
expression-backed attributes at once.

## 2. Current P7 Expr-Arg Status

Current P7 state:

- P7 overall remains `PARTIAL`.
- `attribute_arg_expr` exists in schema/model/table definitions and is populated
  only for selected non-literal `AlignedAttr` and `AssumeAlignedAttr`
  expression subsets.
- `attribute_arg_type` and `attribute_arg_name` also remain deferred.
- Current string payload support is limited to stable whitelisted getters such
  as `DeprecatedAttr`, `AnnotateAttr`, and `SectionAttr`.
- Current constant payload support is limited to direct and shallow-wrapped
  integer-literal `AlignedAttr` arguments.
- `AlignedAttr` non-literal expressions such as `aligned(8 + 8)` and
  `AssumeAlignedAttr` non-literal alignment/offset expressions are recorded as
  `attribute_arg_expr` when `ExprProcessor` can provide a stable id.
- Generalized `CONSTANT_EXPR` and APValue handling remain deferred.
- Dependent/template attribute arguments remain deferred.

Current implementation path:

- `AttributeProcessor::recordAttribute()` skips null and implicit Clang attrs.
- `AttributeProcessor::recordAttributeArguments()` handles only selected
  string attrs and `AlignedAttr` literal constants.
- `recordIntegerConstantArgument()` unwraps a small set of wrappers and then
  calls `ExprProcessor::processAttributeIntegerLiteral()`.
- `AttributeProcessor` inserts `DbModel::AttributeArgExpr` only for selected
  non-literal `AlignedAttr` and `AssumeAlignedAttr` subsets.

The current safe subset should be preserved. `attribute_arg_expr` work must not
regress P7a/P7b, owner links, string payloads, or literal constants.

## 3. Schema And CodeQL Semantic Target

Local schema shape from `docs/semmlecode.cpp.dbscheme`:

```text
attribute_args(
    unique int id: @attribute_arg,
    int kind: int ref,
    int attribute: @attribute ref,
    int index: int ref,
    int location: @location_default ref
);

case @attribute_arg.kind of
  0 = @attribute_arg_empty
| 1 = @attribute_arg_token
| 2 = @attribute_arg_constant
| 3 = @attribute_arg_type
| 4 = @attribute_arg_constant_expr
| 5 = @attribute_arg_expr
;

attribute_arg_expr(
    unique int arg: @attribute_arg ref,
    int expr: @expr ref
)
```

Local table/model mapping:

- `include/model/db/attribute.h` defines `AttributeArgKind::EXPR = 5`.
- `DbModel::AttributeArgExpr` has `arg` and `expr`.
- `include/db/table_defs/attribute.h` maps `attribute_arg_expr.arg` as the
  primary key and `attribute_arg_expr.expr` as the expression id.

CodeQL-shaped interpretation:

- `attribute_args.attribute` corresponds to `AttributeArgument.getAttribute()`.
- `attribute_args.index` corresponds to `AttributeArgument.getIndex()`.
- `attribute_arg_expr.expr` corresponds to `AttributeArgument.getValueExpr()`.
- `attribute_arg_expr` must be populated only for expression-valued arguments.
- `attribute_arg_expr` must not be used for stable strings, type payloads,
  Microsoft named-argument names, or raw scalar integer fields.

Coexistence with constants:

- `attribute_arg_constant.constant` also points to `@expr`, but it represents
  CodeQL constant payload semantics, not arbitrary expression payloads.
- Direct integer literals currently belong to `attribute_arg_constant`, not
  `attribute_arg_expr`.
- Non-literal source expressions such as `aligned(8 + 8)` must not be folded
  into the same representation as direct `aligned(16)`.
- The `CONSTANT_EXPR` kind exists in the dbscheme, but Arborchive does not yet
  have a documented policy for it.

Recommended target split:

- Direct and shallow literal constants: keep using `attribute_arg_constant`.
- Non-literal source expressions: future candidate for `attribute_arg_expr`
  only after `ExprProcessor` safety work and CodeQL parity confirmation.
- Folded/APValue-only constants: defer to a separate generalized constant /
  `CONSTANT_EXPR` design.
- Do not populate both `attribute_arg_constant` and `attribute_arg_expr` for the
  same argument unless a future CodeQL parity review proves that both APIs
  should expose rows for the same source argument.

## 4. ExprProcessor Capability Audit

Current public expression APIs:

- `processDeclRef(DeclRefExpr *)` records variable/non-type-template parameter
  access, but returns no id.
- `processUnaryOperator`, `processBinaryOperator`, and
  `processConditionalOperator` record expression rows, but return no id.
- Literal processors record literal rows; only
  `processAttributeIntegerLiteral(const IntegerLiteral *)` returns an id and
  checks the expression cache first.
- `processCallExpr`, `processImplicitCastExpr`, `processArraySubscriptExpr`,
  `processInitListExpr`, and `processUnaryExprOrTypeTraitExpr` return no id.
- `processConceptSpecializationExpr` and `processNonTypeTemplateParmDecl`
  return ids for specialized non-attribute paths.

Current cache behavior:

- `KeyGen::Expr_::makeKey()` creates an expression key from spelling location
  and selected expression-specific data such as opcode or decl id.
- `SEARCH_EXPR_CACHE` and `INSERT_EXPR_CACHE` exist.
- `processAttributeIntegerLiteral()` checks `SEARCH_EXPR_CACHE` and reuses an
  existing id.
- `processConceptSpecializationExpr()` and
  `processNonTypeTemplateParmDecl()` also check the cache.
- `processBaseExpr()` does not check the cache. It always creates a new
  `exprs` row, inserts the cache key, and returns the new id.
- Many ordinary expression processors call `processBaseExpr()` directly without
  a pre-check.

Capability verdict:

- `ExprProcessor` can produce stable ids for the current integer-literal
  attribute constant subset.
- `ExprProcessor` cannot yet safely produce or return stable ids for arbitrary
  attribute expressions.
- `ExprProcessor` has no dedicated dependent-expression skip API.
- `ExprProcessor` has no documented distinction for unevaluated,
  constant-evaluated, folded, or APValue-only attribute expressions.
- `ExprProcessor` has no public "resolve or create once" API suitable for
  `AttributeProcessor`.

Required API before implementation:

```text
resolveAttributeExprId(const clang::Expr *expr, AttributeExprPolicy policy)
```

The exact name can differ, but the behavior must be:

- return an existing cached id if present;
- skip null, invalid-location, dependent, type-dependent, value-dependent, and
  instantiation-dependent expressions unless explicitly allowed later;
- process only a small allowlist of expression classes;
- insert at most one `exprs` row for the expression;
- return `-1` for unsupported expressions;
- expose enough status for `AttributeProcessor` to avoid writing orphan
  `attribute_arg_expr` rows;
- avoid widening `ASTVisitor`.

## 5. Traversal And Duplication Risk

Clang traversal facts in this checkout:

- `ASTVisitor` inherits `clang::RecursiveASTVisitor<ASTVisitor>`.
- The Clang `RecursiveASTVisitor` `TraverseDecl` path visits `D->attrs()`.
- Generated `Traverse*Attr` methods traverse expression children for relevant
  attrs:
  - `TraverseAlignedAttr` traverses `getAlignmentExpr()` when the attribute is
    expression-backed.
  - `TraverseAnnotateAttr` traverses `args()` and `delayedArgs()`.
  - `TraverseAnnotateTypeAttr` traverses `args()` and `delayedArgs()`.
  - `TraverseAssumeAlignedAttr` traverses `getAlignment()` and `getOffset()`.
  - `TraverseDiagnoseIfAttr` traverses `getCond()`.
  - `TraverseEnableIfAttr` traverses `getCond()`.

Ordering risk:

- `VisitFunctionDecl`, `VisitVarDecl`, `VisitFieldDecl`, and type visitors call
  `AttributeProcessor` while visiting declarations.
- After declaration visitor logic, RecursiveASTVisitor traverses attached attrs.
- If `AttributeProcessor` explicitly processes an expression and the generated
  attr traversal later visits the same expression, the expression may be
  processed twice.

Why current literal constants are safer:

- `recordIntegerConstantArgument()` processes only an `IntegerLiteral`.
- `processAttributeIntegerLiteral()` checks the expression cache first.
- When RecursiveASTVisitor later visits the same integer literal,
  `VisitIntegerLiteral()` calls the same cache-guarded method.

Why arbitrary expressions are unsafe now:

- A future explicit call for a `BinaryOperator` like `8 + 8` could create one
  row.
- Later `VisitBinaryOperator()` can call `processBinaryOperator()`, which calls
  `processBaseExpr()` without a cache check and can create a duplicate row.
- Similar duplication risk exists for unary, conditional, call, cast,
  subscript, init-list, and trait expressions.

Future invariant:

- For any attribute expression argument, there must be exactly one stable
  `exprs` row for the source expression key.
- `attribute_arg_expr.expr` must point to that one row.
- The implementation must either make all relevant expression processors
  cache-guarded or make the attribute expr resolver reuse rows created by normal
  traversal without causing normal traversal to insert a duplicate later.

Suggested duplicate detection:

```sql
select kind, location, count(*), group_concat(id)
from exprs
group by kind, location
having count(*) > 1;
```

For targeted fixtures, also check the specific attribute expression text/value
or surrounding attribute rows:

```sql
select a.name, aa."index", aa.kind, ae.expr, e.kind, e.location
from attribute_arg_expr ae
join attribute_args aa on aa.id = ae.arg
join attributes a on a.id = aa.attribute
join exprs e on e.id = ae.expr;
```

The duplicate query is a smoke test, not a complete proof. The stronger proof
is a processor invariant: every expression-writing path used by attribute args
must resolve cache before insertion.

## 6. Candidate Attr Matrix

| Candidate | Likely source API | Classification | Expected target table(s) | Should populate expr, constant, both, or neither | Risk | Fixture idea |
| --- | --- | --- | --- | --- | --- | --- |
| `AlignedAttr` direct literal | `getAlignmentExpr()` unwrapped to `IntegerLiteral` | SAFE NOW | `attribute_args`, `attribute_arg_constant` | Constant only. Already implemented. | Low | Existing `attribute_constant_args_case.cc` direct/paren/unsigned literals. |
| `AlignedAttr` `aligned(8 + 8)` | `getAlignmentExpr()` returns non-literal `Expr *` such as `BinaryOperator` or wrapper | SAFE SUBSET DONE | `attribute_arg_expr` for non-literal, non-dependent source expressions | Expr only for this subset; do not also populate `attribute_arg_constant`. | High | `attribute_expr_args_case.cc` with `[[gnu::aligned(8 + 8)]]`. |
| `AlignedAttr` type form | `getAlignmentType()` | DEFER | `attribute_arg_type` | Neither for expr work. | High | Separate P7f-type fixture. |
| `AssumeAlignedAttr` | `getAlignment()`, `getOffset()` | SAFE SUBSET DONE | `attribute_arg_expr`; literal subcases may later use constant if policy allows | Expr for non-literal, non-dependent alignment/offset only. Literal constants are skipped in this checkpoint. | High | `attribute_assume_aligned_expr_args_case.cc` with non-literal, literal, and dependent forms. |
| `EnableIfAttr` | `getCond()`, `getMessage()` | SAFE ONLY AFTER ExprProcessor API/caching support | `attribute_arg_expr` for cond; `attribute_arg_value` for message only after split policy | Neither now. Future expr for condition, value for message. | High | Overload guarded by `enable_if(param > 0, "msg")`; dependent template variant as negative. |
| `DiagnoseIfAttr` | `getCond()`, `getMessage()`, diagnostic type | SAFE ONLY AFTER ExprProcessor API/caching support | `attribute_arg_expr` for cond; `attribute_arg_value` for message; diagnostic kind deferred | Neither now. Future expr for condition, value for message. | High | Function with `diagnose_if(param == 0, "msg", "warning")`; dependent template negative. |
| `AnnotateAttr` annotation string | `getAnnotation()` | SAFE NOW for string subset | `attribute_arg_value` | Value only. Already implemented for annotation string. | Low | Existing `attribute_string_args_case.cc`. |
| `AnnotateAttr` expression args | `args()`, `delayedArgs()` | DEFER | Future `attribute_arg_expr` | Neither now. Do not collapse into string. | High | `annotate("tag", 1 + 2)` if accepted by Clang; delayed args negative if unstable. |
| `AnnotateTypeAttr` expression args | `args()`, `delayedArgs()` | DEFER | Future `attribute_arg_expr`, but owner/type placement also unresolved | Neither now. | High | Type spelling with `[[clang::annotate_type("tag", expr)]]` if supported. |
| Other attrs with generated `Expr **args_` | generated `args()` ranges | UNSAFE | Unknown | Neither. | High | No broad fixture until API and candidate-specific parity exist. |
| APValue/folded-only constants | Attr-specific semantic value APIs, not necessarily source `Expr *` | DEFER | Future `attribute_arg_constant` or `attribute_arg_constant_expr` | Neither now. | High | Separate generalized constant fixture after policy. |

Classification definitions:

- `SAFE NOW`: already implemented or safe under existing cache-guarded literal
  path.
- `SAFE ONLY AFTER ExprProcessor API/caching support`: candidate has a clear
  expression API, but current processor/traversal invariants are insufficient.
- `DEFER`: candidate needs another subsystem policy first, such as type,
  annotation delayed args, or CodeQL constant-expr parity.
- `UNSAFE`: too broad or ambiguous for this phase.

No candidate should populate `attribute_arg_name` or `attribute_arg_type` as
part of P7f-expr work.

## 7. aligned(8 + 8) Policy

Recommendation: keep `aligned(8 + 8)` deferred for now.

Do not model it as `attribute_arg_constant` by folding the expression to `16`.
That would erase source-shape information and make it indistinguishable from
direct `aligned(16)`, which is not a safe CodeQL-shaped assumption.

Do not model it as both `attribute_arg_constant` and `attribute_arg_expr`
without explicit CodeQL parity evidence. The dbscheme has both constant and
expression families, and double-populating them for the same argument would be
a semantic claim that the project has not proven.

Future preferred path after API/caching support:

- If CodeQL parity confirms `getValueExpr()` exposes the source expression,
  model `aligned(8 + 8)` as `attribute_arg_expr` with
  `AttributeArgKind::EXPR`.
- If CodeQL parity shows this belongs to `getValueConstant()` or
  `CONSTANT_EXPR`, design that path separately.
- Keep direct/shallow integer literals in `attribute_arg_constant`.

Architecture reason:

- `AttributeProcessor` should classify the attribute argument and delegate
  expression id creation to an expression-owned API.
- `ASTVisitor` must not grow special-case expression orchestration.
- Folding `8 + 8` inside `AttributeProcessor` would mix attribute, expression,
  and constant-evaluation ownership.

## 8. Dependent/Template Expression Policy

Dependent/template attribute expressions should be skipped for now.

Detection policy for future implementation:

- Skip if `expr->isValueDependent()`.
- Skip if `expr->isTypeDependent()`.
- Skip if `expr->isInstantiationDependent()`.
- Skip if `expr->containsUnexpandedParameterPack()`.
- Skip null expressions and invalid source locations.
- Skip delayed `AnnotateAttr` args until their instantiation timing and
  CodeQL semantics are clear.

Negative test requirements:

- Template function with an attribute expression depending on a template
  parameter.
- Attribute expression with a dependent `sizeof(T)` or equivalent dependent
  expression if accepted by Clang.
- `AnnotateAttr` or `DiagnoseIfAttr` dependent condition if accepted by the
  validation toolchain.
- Ensure `attribute_arg_expr`, `attribute_arg_constant`, `attribute_arg_type`,
  and `attribute_arg_name` remain empty for skipped dependent cases.

Future behavior should prefer an explicit skip over a partial or stringified
row. No string fallback is allowed.

## 9. Future Implementation Criteria

Mandatory conditions before implementing `attribute_arg_expr`:

- `ExprProcessor` exposes a stable API that accepts a supported `Expr *` and
  returns an existing or newly created `@expr` id.
- The API checks expression cache before insertion.
- The API returns `-1` for unsupported or dependent expressions.
- All expression-writing paths used by the first candidate are duplicate-safe.
- The implementation has a clear traversal strategy: either normal RAV
  traversal creates the row before `attribute_arg_expr` is recorded, or the
  explicit attribute path creates it once and later traversal reuses it.
- No `Expr::printPretty()`, source text, or other string fallback is used.
- No APValue/generalized constant evaluation is introduced in the expr patch.
- `attribute_arg_type` and `attribute_arg_name` remain unaffected.
- Positive fixtures cover the first candidate.
- Negative fixtures cover dependent/template expressions and unsupported attr
  expr families.
- SQLite evidence proves:
  - `attribute_args.kind` is `EXPR` for expression rows;
  - `attribute_arg_expr.arg` joins to `attribute_args.id`;
  - `attribute_arg_expr.expr` joins to `exprs.id`;
  - duplicate expression queries return zero relevant duplicates;
  - `attribute_arg_constant` remains reserved for literal constants;
  - `attribute_arg_type` and `attribute_arg_name` remain empty for expr-only
    fixtures.
- P7a/P7b regression stays green.
- Owner-link regression stays green.
- `scripts/test_all.sh` passes for behavior changes.

## 10. P7f-expr Precondition Implementation

This precondition is now implemented in `ExprProcessor`.

Added API:

```cpp
int ExprProcessor::getOrProcessExprId(const clang::Expr *expr);
```

Behavior:

- Returns the existing cached `@expr` id when the expression key has already
  been processed.
- Creates and caches one `exprs` row only for expression classes already owned
  by current `ExprProcessor` extraction paths.
- Returns `-1` for null expressions, invalid source locations, dependent,
  type-dependent, value-dependent, instantiation-dependent, or unexpanded-pack
  expressions.
- Returns `-1` for unsupported expression classes instead of stringifying or
  partially modeling them.
- Does not evaluate expressions, fold APValues, implement `CONSTANT_EXPR`, or
  populate `attribute_arg_expr`.

Cache/reuse policy:

- `processBaseExpr()` now checks the expression cache before inserting an
  `exprs` row.
- Public expression processors with companion rows, such as literal values,
  variable binds, call binds, init-list aggregate binds, and sizeof/alignof
  binds, return early when the expression id is already cached.
- This makes the future ordering safe for the first P7f-expr candidate: an
  explicit attribute path may create the expression row once, and later
  RecursiveASTVisitor traversal will reuse the cached row instead of inserting
  another `exprs` row.

Current support remains conservative:

- Supported through existing processors: decl refs, unary operators, binary
  operators, conditional operators, string/integer/floating/character/bool
  literals, calls, implicit casts, array subscripts, init lists, and
  sizeof/alignof expressions.
- Unsupported wrappers such as `ConstantExpr` still require the future caller
  to choose an explicit unwrapping/classification policy.
- Direct/shallow integer-literal `AlignedAttr` constants remain in
  `attribute_arg_constant`.

Why broader `attribute_arg_expr` remains deferred:

- The precondition task established stable expression id reuse.
- Checkpoint 1 chose and implemented only the first attribute argument
  classifier: non-literal `AlignedAttr`.
- Checkpoint 2 added only one more classifier: non-literal
  `AssumeAlignedAttr` alignment and offset expressions.
- Each remaining candidate still needs argument index policy,
  constant-vs-expr policy, mixed-payload policy, and focused positive/negative
  fixtures.
- `attribute_arg_type`, `attribute_arg_name`, generalized APValue constants,
  and broad expression-backed attribute families remain out of scope.

Validation evidence to collect for this precondition:

```sql
select kind, location, count(*), group_concat(id)
from exprs
group by kind, location
having count(*) > 1;

select count(*) from attribute_arg_expr;
select count(*) from attribute_arg_type;
select count(*) from attribute_arg_name;
```

For the precondition patch, validation focused on build/test compatibility and
no duplicate ordinary expression rows. Checkpoint 1 then wired the API into
`AttributeProcessor` only for non-literal `AlignedAttr` expressions and added
`tests/unit-tests/p7/attribute_expr_args_case.cc`.

## 11. Checkpoint 1: Minimal AlignedAttr Expr Subset

Implemented safe subset:

- `AlignedAttr::getAlignmentExpr()` is classified in `AttributeProcessor`.
- Direct/shallow integer literals remain `attribute_arg_constant`.
- Non-literal, non-dependent source expressions such as `8 + 8` and
  shallow-wrapped `4 * 4` become `attribute_arg_expr`.
- `ExprProcessor::getOrProcessExprId()` supplies the stable `@expr` id.
- Dependent template aligned expressions are skipped because the expression id
  resolver rejects dependent expressions.

Validation evidence from `tests/unit-tests/p7/attribute_expr_args_case.cc`:

```text
attribute_arg_constant|1
attribute_arg_expr|2
attribute_arg_type|0
attribute_arg_name|0
p7f_expr_aligned_add|aligned|5|0|expr-kind 25
p7f_expr_aligned_wrapped_mul|aligned|5|0|expr-kind 27
p7f_expr_aligned_literal|aligned|2|0|16
dependent template aligned attribute has no payload row
duplicate expr smoke query returned zero rows
constant/expr overlap query returned zero rows
```

Self-review:

- No expression was stringified.
- No arbitrary expression was evaluated or folded.
- Direct literal constants stayed in `attribute_arg_constant`.
- Non-literal expressions did not also populate `attribute_arg_constant`.
- `attribute_arg_type` and `attribute_arg_name` remained empty.
- `ASTVisitor` stayed unchanged for this checkpoint.

## 12. Checkpoint 2: Broader Expr Design Gate

Candidate audit:

| Candidate | API stability | ExprProcessor support | Duplicate risk | Dependent skip | Index policy | Fixture viability | Verdict |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `AssumeAlignedAttr` | Local Clang 19 exposes `getAlignment()` and `getOffset()` as `Expr *`; generated traversal visits both. | Supported for current non-literal arithmetic expressions through `getOrProcessExprId`. | Cache checks in `getOrProcessExprId` and expression processors make explicit processing and later RAV traversal reuse one row. | `getOrProcessExprId` rejects dependent/type-dependent/value-dependent/instantiation-dependent/unexpanded-pack expressions. | Source-order indexes: alignment `0`, offset `1`; null offset is skipped. | Positive, literal-skip, and dependent-skip fixture compiles under `config.example.toml` flags. | `SAFE NOW` for non-literal expr subset only. |
| `EnableIfAttr` | Local Clang 19 exposes `getCond()` and `getMessage()`. | Condition expressions can be supported, but the attr mixes expr and stable message text. | Generated traversal visits the condition, so cache reuse is required. | Dependent template forms compile and can be skipped. | Cond/message index policy needs a CodeQL parity decision before insertion. | Positive and dependent fixtures compile. | `DEFER`. |
| `DiagnoseIfAttr` | Local Clang 19 exposes `getCond()`, `getMessage()`, and diagnostic type. | Condition expressions can be supported, but the attr mixes expr, string, and enum semantics. | Generated traversal visits the condition, so cache reuse is required. | Dependent template forms compile and can be skipped. | Cond/message/diagnostic-kind indexes need a CodeQL parity decision before insertion. | Positive and dependent fixtures compile. | `DEFER`. |
| `AnnotateAttr` expression args | Local Clang 19 exposes `args()` and `delayedArgs()`. | Individual args can be supported only after variadic/delayed-arg policy. | Generated traversal visits both arg ranges. | Dependent examples compile, but delayed instantiation timing is unresolved. | Annotation string is already index `0`; expression args need stable variadic index policy. | Positive and dependent fixtures compile. | `DEFER`. |

Implemented Checkpoint 2 safe subset:

- `AssumeAlignedAttr` non-literal alignment and offset expressions are recorded
  as `attribute_arg_expr`.
- Direct/shallow integer-literal `AssumeAlignedAttr` arguments are skipped for
  now instead of being forced into `attribute_arg_constant`.
- Dependent/template `AssumeAlignedAttr` expressions are skipped by
  `ExprProcessor::getOrProcessExprId`.
- No `attribute_arg_type`, `attribute_arg_name`, APValue, `CONSTANT_EXPR`,
  string fallback, source text fallback, or `ASTVisitor` change is introduced.

Fixture:

- `tests/unit-tests/p7/attribute_assume_aligned_expr_args_case.cc`

Validation evidence from
`tests/unit-tests/p7/attribute_assume_aligned_expr_args_case.cc`:

```text
attributes|3
funcattributes|3
attribute_args|2
attribute_arg_expr|2
attribute_arg_constant|0
attribute_arg_type|0
attribute_arg_name|0
assume_aligned|gnu|0|5|expr-kind 25
assume_aligned|gnu|1|5|expr-kind 25
literal assume_aligned attribute has no payload row
dependent template assume_aligned attribute has no payload row
duplicate expr smoke query returned zero rows
constant/expr overlap query returned zero rows
```

Self-review:

- No expression was stringified.
- No arbitrary expression was evaluated or folded.
- Literal `AssumeAlignedAttr` arguments were not forced into
  `attribute_arg_constant`.
- Dependent/template expressions did not produce payload rows.
- `attribute_arg_type` and `attribute_arg_name` remained empty.
- `ASTVisitor` stayed unchanged for this checkpoint.

## 13. Recommended Next Task

Broader `attribute_arg_expr` should remain unimplemented outside the selected
`AlignedAttr` and `AssumeAlignedAttr` subsets.

Recommended next task:

```text
P7f-type design gate
```

Scope:

- Audit `attribute_arg_type` candidates independently from expression payloads.
- Keep remaining `EnableIfAttr`, `DiagnoseIfAttr`, and `AnnotateAttr`
  expression payloads deferred until their mixed payload and argument-index
  policies are clear.
- Keep `ASTVisitor` dispatch-only and keep constants/types/names separate.

Current implemented candidate:

- `AlignedAttr::getAlignmentExpr()` when the unwrapped source expression is
  not a direct/shallow integer literal and is not dependent.
- `AssumeAlignedAttr::getAlignment()` and `getOffset()` when the unwrapped
  source expression is not a direct/shallow integer literal and is not
  dependent.

Non-goals:

- Do not implement `AnnotateAttr` expression args.
- Do not implement `EnableIfAttr` or `DiagnoseIfAttr`.
- Do not implement `attribute_arg_type`.
- Do not implement `attribute_arg_name`.
- Do not implement APValue or generalized `CONSTANT_EXPR`.
- Do not fold `aligned(8 + 8)` into an integer constant.

Fixture names:

- `tests/unit-tests/p7/attribute_expr_args_case.cc`
- `tests/unit-tests/p7/attribute_assume_aligned_expr_args_case.cc`
- Optional later negative fixture:
  `tests/unit-tests/p7/attribute_expr_dependent_case.cc`

Target SQL for a future implementation:

```sql
select a.name, aa."index", aa.kind, ae.expr, e.kind
from attributes a
join attribute_args aa on aa.attribute = a.id
join attribute_arg_expr ae on ae.arg = aa.id
join exprs e on e.id = ae.expr
where a.name in ('aligned', 'assume_aligned');
```

```sql
select kind, location, count(*), group_concat(id)
from exprs
group by kind, location
having count(*) > 1;
```

```sql
select count(*) from attribute_arg_constant;
select count(*) from attribute_arg_type;
select count(*) from attribute_arg_name;
```

Validation commands for future behavior work:

```bash
python3 scripts/generate_instantiations.py
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
scripts/test_all.sh
./build/demo -c config.example.toml \
  -s tests/unit-tests/p7/attribute_expr_args_case.cc \
  -o tests/output/unit-tests-p7-attribute_expr_args_case.db
python3 scripts/db_summary.py \
  tests/output/unit-tests-p7-attribute_expr_args_case.db
```

For behavior checkpoints, rerun the focused fixture and full test suite before
continuing.

## 14. Validation Strategy

Design-document validation:

```bash
git diff --check
git diff -- docs/analysis/p7f_attribute_expr_design.md
rg "attribute_arg_expr|ExprProcessor|getValueExpr|aligned\\(8 \\+ 8\\)|dependent|duplicate|AnnotateAttr|EnableIfAttr|DiagnoseIfAttr" \
  docs/analysis/p7f_attribute_expr_design.md
```

Behavior validation for expression checkpoints must additionally include:

- focused P7 expr fixtures;
- SQLite joins for `attribute_args`, `attribute_arg_expr`, and `exprs`;
- duplicate-expression queries;
- negative dependent/template fixtures;
- P7 string/constant regression checks;
- owner-link regression checks;
- full `scripts/test_all.sh`.

Keep P7 marked `PARTIAL` until all P7 full-DONE criteria are met. Keep
`docs/datatable-list.txt` unchanged for this audit and for any future
safe-subset-only implementation.
