# P8 Initialization System Research

## task type

- schema or CodeQL alignment;
- processor or semantic extraction;
- test or verification.

## specs and context read

- `AGENTS.md`
- `docs/agent_workflow.md`
- `.trellis/workflow.md`
- `.trellis/spec/README.md`
- `.trellis/spec/arborchive/architecture.md`
- `.trellis/spec/arborchive/ast-visitor-boundary.md`
- `.trellis/spec/arborchive/processor-boundary.md`
- `.trellis/spec/arborchive/schema-codeql-alignment.md`
- `.trellis/spec/arborchive/testing-and-verification.md`
- `docs/roadmap.md`
- `docs/datatable-list.txt`
- `docs/semmlecode.cpp.dbscheme`
- `include/db/table_init.h`
- `include/db/storage_facade.h`
- `src/db/storage_facade.cc`
- `scripts/generate_instantiations.py`
- `include/core/ast_visitor.h`
- `src/core/ast_visitor.cc`
- existing expression and variable model/table/processor files

## P8 roadmap scope

`docs/roadmap.md` defines P8 as the Initialization System:

- `initialisers`
- `braced_initialisers`
- `aggregate_field_init`
- `aggregate_array_init`

`docs/datatable-list.txt` shows:

- `initialisers` and `braced_initialisers` are not marked DONE;
- `aggregate_field_init` and `aggregate_array_init` are already marked DONE.

The current code already has `ExprProcessor::processInitListExpr` and
`ASTVisitor::VisitInitListExpr` delegates to it. That existing aggregate
initialization path should be preserved unless validation proves a narrow fix is
needed.

## CodeQL schema counterpart

`docs/semmlecode.cpp.dbscheme` defines:

```text
initialisers(
    unique int init: @initialiser,
    int var: @accessible ref,
    unique int expr: @expr ref,
    int location: @location_expr ref
);

braced_initialisers(
    int init: @initialiser ref
);
```

`@initialiser` is an element-like entity in the CodeQL schema, separate from
`@expr`. Therefore `initialisers.init` is a generated initializer identity, not
the initializer expression id.

`braced_initialisers.init` must reference the generated `@initialiser` identity.
It must not reference the `InitListExpr` / `@expr` id directly.

`initialisers.expr` references the initializer expression `@expr`. For braced
forms, the expression will normally be the `InitListExpr` row that existing
`ExprProcessor::processInitListExpr` records.

`initialisers.location` references `@location_expr`, so the location should be
recorded through the expression location path for the initializer expression.

## accessible ID semantics

The CodeQL schema defines:

```text
@addressable = @function | @variable ;
@accessible = @addressable | @enumconstant ;
@variable = @localscopevariable | @globalvariable | @membervariable ;
@localscopevariable = @localvariable | @parameter;
```

Current Arborchive variable handling has a transitional/deprecated
`variable` table, but the actual processors have already moved to direct
variable entity IDs:

- `VariableProcessor::processLocalVar` returns `localvariables.id`;
- `VariableProcessor::processParam` returns `params.id`;
- `VariableProcessor::processGlobalVar` returns `globalvariables.id`;
- `VariableProcessor::processMemberVar` / `resolveMemberVarId` return
  `membervariables.id`;
- `VariableProcessor::processVarDecl` inserts the direct variable entity id into
  `SEARCH_VARIABLE_CACHE` / `INSERT_VARIABLE_CACHE`;
- `ExprProcessor::processDeclRef` uses that cached direct variable id for
  `varbind.var`.

Therefore P8 should store `initialisers.var` as the cached direct variable
entity id, not `var_decls.id`.

This is important because `VariableProcessor::processVarDecl` returns a
declaration row id (`var_decls.id`) for visitor-side callers, while the cache is
the CodeQL-style `@accessible`-compatible entity id.

## traversal and dependency implications

`RecursiveASTVisitor` visits a `VarDecl` before traversing the initializer
expression children. That means `VarDecl::getInit()` may not yet have a cached
`@expr` id when initializer extraction runs.

The safe implementation is:

- generate the `@initialiser` id immediately;
- resolve `var` from the variable cache after `VariableProcessor` has processed
  the declaration;
- insert an `initialisers` row with the known `init`, `var`, `location`, and a
  placeholder `expr = -1` when the initializer expression is not cached yet;
- add an `EXPR` dependency keyed by the initializer expression;
- replace the same `initialisers.init` row once the expression id is resolved.

`braced_initialisers` can be inserted immediately because it only depends on the
generated `@initialiser` id.

## ownership decision

Initialization semantics should be owned by a new `InitializationProcessor`.

`ASTVisitor` should only provide narrow delegation after variable processing has
made the `@accessible` variable id available in cache. It must not decide schema
row fields, braced classification, dependency policy, or ID semantics.

## allowed paths

- `.trellis/tasks/p8-initialization-system/`
- `include/model/db/`
- `include/db/table_defs/`
- `include/db/table_init.h`
- `include/core/processor/`
- `src/core/processor/`
- `include/core/ast_visitor.h`
- `src/core/ast_visitor.cc`
- `src/db/storage_facade_instantiations.inc`
- `tests/unit-tests/p8/`
- `scripts/test_all.sh`

## forbidden scope

- no Trellis runtime, dependency, scheduler, execution framework, or generated
  command layer;
- no new Arbor-prefixed tables;
- no table renames;
- no constructor initializer, member initializer, P9 constexpr flow, or P10
  expression graph work;
- no semantic row assembly in `ASTVisitor`;
- no rewrite of existing aggregate initialization logic unless validation
  proves a narrow P8-related fix is required.

## risks

- Existing expression coverage is incomplete, so some initializer expression
  forms may remain unresolved until future expression phases.
- Top-level initializer expressions may contain implicit wrappers; P8 should
  use a stable initializer expression key but avoid broad expression modeling.
- Existing aggregate init tables already use `InitListExpr` ids as the aggregate
  expression identity. P8 should validate them but avoid changing that behavior
  in the first slice.
