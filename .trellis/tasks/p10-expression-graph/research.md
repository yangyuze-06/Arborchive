# P10 Expression Graph Core Research

## task type

- CodeQL-aligned schema;
- expression/statement/initialiser processor behavior;
- SQLite semantic verification.

## context read

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
- expression, statement, initialiser model/table/processor paths

## CodeQL counterpart

`exprparents(expr_id, child_index, parent_id)` is a CodeQL-aligned main-tree
edge. `expr_id` is the child expression, `parent_id` is an expression,
statement, or initialiser identity, and `child_index` follows the parent kind's
CodeQL ordering. The table deliberately has no additional primary key; the
processor owns relation deduplication.

CodeQL keeps conversion wrappers outside the main expression tree. P10
therefore normalizes children with `IgnoreParenCasts()` while preserving the
existing implicit-cast rows for later P11 `exprconv` work.

## ownership and boundaries

- `ExprProcessor` owns normalization, supported-expression filtering, relation
  insertion, and deduplication.
- `StmtProcessor` and `InitializationProcessor` use an explicit nullable
  `ExprProcessor*` dependency.
- `ASTVisitor` only constructs processors and injects that dependency; its
  `Visit*` methods remain unchanged.
- P10 excludes range-for, case expressions, allocation, cast/conversion
  semantics, attribute arguments, `expr_ancestor`, and new expression kinds.

## migration and risk

- Existing columns and CLI behavior do not change.
- Old databases must be regenerated to gain `exprparents`.
- Expression identity remains source-location based, so the fixture checks
  duplicate children and wrapper normalization explicitly.
