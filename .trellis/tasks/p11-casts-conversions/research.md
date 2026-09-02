# P11 Casts & Conversion System Research

## task classification

- CodeQL-aligned schema and ORM registration;
- expression processor semantic extraction;
- AST visitor boundary reduction;
- fixture, SQLite, roadmap, and migration verification.

## context read

- `AGENTS.md`, `docs/agent_workflow.md`, `.trellis/workflow.md`;
- `.trellis/spec/README.md` and all relevant Arborchive architecture,
  visitor, processor, schema, testing, and commit specs;
- `docs/roadmap.md`, `docs/datatable-list.txt`, and the relevant declarations
  in `docs/semmlecode.cpp.dbscheme`;
- current P10 expression graph notes and expression model/table/key/processor
  paths.

## CodeQL alignment

All five facts are named in the checked-in CodeQL dbscheme. `exprconv` points
from the converted expression to its conversion wrapper. `expr_types` stores
the expression type and CodeQL value category (prvalue 1, xvalue 2, lvalue 3).
`expr_isload` marks lvalue-to-rvalue loads without materializing a wrapper.
`compgenerated` marks compiler-generated implicit conversion nodes only.
`conversionkinds` classifies cast semantics independently of cast syntax.

These are CodeQL-aligned tables, not Arbor extensions. The migration changes
supported implicit cast rows from legacy expression kind 217 to kind 214 and
therefore requires rebuilding old databases.

## ownership

- `ExprProcessor` owns all P11 semantic and cross-table writes.
- `TypeProcessor` remains the type identity owner and is injected into
  `ExprProcessor`; qualifier recording remains delegated to
  `SpecifierProcessor` through the same processor boundary.
- `ASTVisitor` only dispatches `CastExpr` and `ParenExpr`.
- `exprparents` continues to normalize with `IgnoreParenCasts()`, so conversion
  wrappers cannot enter the main expression graph.
- Existing field reads are materialized on demand as CodeQL call-like member
  reads only when an implicit conversion needs a concrete converted endpoint;
  this preserves P8 aggregate initializer references without opening a general
  member-expression roadmap phase.
- Review follow-up adds narrow conversion-source identities for routine
  references, `this`, and `nullptr`. The new `this` key suffix applies only to
  a previously unsupported expression and prevents collision with its
  same-location `MemberExpr`; established non-conversion keys are unchanged.

## deferred

Dependent casts, reference wrappers, temporary initialization, C11 generic,
Objective-C/address-space casts, and `BuiltinBitCastExpr` stay outside P11.
Constructor-backed class functional casts remain part of the deferred
`temp_init` boundary; scalar functional casts are covered by P11.
