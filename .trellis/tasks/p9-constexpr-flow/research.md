# P9 Constexpr / Consteval Flow Audit

## Semantic mapping

LLVM 19 represents every supported form as `clang::IfStmt` and exposes the
form through `IfStmt::getStatementKind()`:

| Clang `IfStatementKind` | Arborchive `StmtKind` | CodeQL value |
| --- | --- | --- |
| `Ordinary` | `IF` | 2 |
| `Constexpr` | `CONSTEXPR_IF` | 35 |
| `ConstevalNonNegated` | `CONSTEVAL_IF` | 38 |
| `ConstevalNegated` | `NOT_CONSTEVAL_IF` | 39 |

`getInit()`, `getThen()`, and `getElse()` return the same child statement
objects visited later by `RecursiveASTVisitor`. P9 therefore uses the existing
statement key/cache and deferred dependency replacement path rather than
creating relationship-specific child IDs.

An expression used directly as an init-statement or branch is visited through
the expression pipeline and would not otherwise enter the statement cache.
P9 materializes that same AST node in `stmts` with kind `EXPR` (1), reusing
the corresponding `exprs.id`, before writing the relationship. This supplies
the CodeQL `@stmt_expr` identity without introducing P10 expression-parent
relationships.

## CodeQL schema alignment

The five P9 tables and their unique owner fields are:

- `constexpr_if_initialization(constexpr_if_stmt, init_id)`
- `constexpr_if_then(constexpr_if_stmt, then_id)`
- `constexpr_if_else(constexpr_if_stmt, else_id)`
- `consteval_if_then(constexpr_if_stmt, then_id)`
- `consteval_if_else(constexpr_if_stmt, else_id)`

The owner field is implemented as the SQLite primary key. The consteval tables
retain CodeQL's `constexpr_if_stmt` field spelling even though the accepted
owner kinds are both consteval variants.

## Migration and boundaries

- The historical `if_initalization` table is renamed to the CodeQL-compatible
  `if_initialization` spelling without a compatibility alias. Existing output
  databases must be regenerated.
- `ASTVisitor::VisitIfStmt()` remains dispatch-only and unchanged.
- P10 expression parents, casts, allocation, attribute arguments, generic
  statement parents, and unrelated schema cleanup remain out of scope.
- The existing `if_else` SQL column spelling mismatch is recorded but deferred
  to a separate compatibility change.
