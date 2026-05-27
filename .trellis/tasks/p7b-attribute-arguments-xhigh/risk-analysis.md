# P7b Risk Analysis

## High-Risk Areas

`Expr` and `APValue` handling is the highest risk. Arbitrary expression or
constant serialization can become compiler-version-dependent and hard to
review.

Attribute APIs are heterogeneous. Many Clang attribute classes expose custom
fields rather than a uniform argument list.

AST traversal may not visit attribute arguments the same way it visits normal
source expressions, so P7b must not rely on accidental visitor behavior.

## Mitigations

Use typed Clang APIs for selected attributes only:

- `DeprecatedAttr::getMessage()` for stable string text;
- `AlignedAttr::getAlignmentExpr()` only when it contains a direct integer
  literal through a small, reviewed unwrap path.

Reuse expression tables for integer constants instead of inventing an
attribute-specific scalar blob.

Keep semantic branching in `AttributeProcessor`; visitor remains thin.

## Deferred Semantics

The following are intentionally deferred:

- arbitrary expression arguments;
- APValue dumps;
- raw AST dumps;
- type arguments;
- named arguments;
- constant expressions that are not direct integer literals;
- broad attribute-family coverage.

## Review Risks

Reviewers should check:

- no `attribute_arg_value` catch-all behavior was introduced;
- `attribute_args.kind` matches the typed payload table used;
- schema tables are all listed in `docs/datatable-list.txt`;
- `ASTVisitor` did not gain attribute argument mapping logic;
- SQLite evidence includes both positive argument rows and unchanged P7a
  presence rows.

## Rollback Strategy

The change is additive. A rollback should remove:

- P7b task docs;
- P7b model/table/init additions;
- AttributeProcessor argument helpers;
- the P7b fixture/test entry;
- regenerated storage instantiations.

No existing P7a schema should be deleted or renamed.
