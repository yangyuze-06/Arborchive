# P12 Allocation & Lifetime Research

## Ownership and alignment

`ExprProcessor` owns allocation semantics, relationship rows, and main
expression graph edges. `FunctionProcessor` exposes only a stable function
reference resolver. `ASTVisitor` injects that dependency and dispatches
`CXXNewExpr`/`CXXDeleteExpr` directly.

All four tables are CodeQL-aligned and already approved in
`docs/datatable-list.txt`: `expr_allocator` (162), `expr_deallocator` (163),
`new_allocated_type` (180), and `new_array_allocated_type` (181). Their columns
match `docs/semmlecode.cpp.dbscheme`.

## Type rule

- Plain new persists `CXXNewExpr::getAllocatedType()`.
- Array new wraps that type in exactly one outer constant array when the outer
  extent is an integer constant; otherwise it uses an incomplete array.
- Because the allocated type is retained as the element type of the new outer
  array, fixed nested dimensions such as `new int[n][4]` remain intact.

## Function forms

- Allocator form is 1 when Clang says alignment is implicitly passed or the
  selected function contains an `std::align_val_t` parameter; otherwise 0.
- Deallocator form is a bitset: canonical `size_t` = 1,
  `std::align_val_t` = 2, destroying operator delete = 4.
- A relation is emitted only after the function declaration resolves to a
  non-negative persisted function ID.
- Scalar delete expressions with a virtual destroyed-type destructor omit the
  relation only when runtime final-overrider selection can change the target.
  Explicit global delete, array delete, and final record/destructor cases retain
  Clang's statically selected deallocator.
- Type/value/instantiation-dependent new/delete expressions are deferred before
  expression or allocated-type materialization.

## Lifetime feasibility closure

This phase deliberately does not implement `synthetic_destructor_call`,
`expr_reuse`, or synthetic destructor/deallocator call children for delete.
`CXXDeleteExpr` does not directly expose a persistable destructor-call `Expr`,
while current expression identity depends on stable source ranges. Synthesizing
such nodes would break identity and main-graph semantics.

### Re-entry condition

Re-enter lifetime call materialization only after the project establishes both:

1. a specification for compiler-generated expression identity; and
2. a specification for routine-call materialization.

## Migration

P12 adds four schema tables and begins emitting expression kinds 87, 88, 128,
and 129. Existing databases must be rebuilt.
