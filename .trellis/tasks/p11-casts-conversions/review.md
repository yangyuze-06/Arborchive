# P11 Review Follow-up

## confirmed findings

1. `CK_LValueToRValue` cached the main/load expression ID and caused
   `ParenExpr` conversion chains to fork.
2. Function-to-pointer decay was dropped because function `DeclRefExpr` had no
   conversion-source identity.
3. `CK_UncheckedDerivedToBase` fell through to conversion kind 0.
4. The `expr_types` assertion detected missing rows but not duplicates.
5. `this` initially collided with its same-location `MemberExpr` when added as
   a conversion source.

## corrections

- Load markers still target the main expression, while the lvalue-to-rvalue
  cache returns the direct converted ID for outer conversion chaining.
- Routine references use kind 97 plus `funbind`; `this` uses kind 85 with a
  new-source-only key suffix; `nullptr` uses literal kind 123.
- Both checked and unchecked derived-to-base CastKinds map to kind 2.
- SQLite assertions require exactly one `expr_types` row and verify the exact
  parenthesized chain, function decay, `nullptr`, and unchecked inheritance.

## residual boundary

Class functional casts whose direct source is `CXXConstructExpr` require the
deferred `temp_init` subsystem and remain outside P11. Scalar C-style and
functional syntax is implemented and tested. No ASTVisitor orchestration was
added during the review fix.
