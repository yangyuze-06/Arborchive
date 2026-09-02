# P12 Allocation & Lifetime Plan

## Goal

Implement the CodeQL-aligned allocation expression and allocator/deallocator
facts for P12 without adding synthetic call expressions or moving semantic
coordination into `ASTVisitor`.

## Scope

- `CXXNewExpr` kinds 87/129 and `CXXDeleteExpr` kinds 88/128.
- `new_allocated_type`, `new_array_allocated_type`, `expr_allocator`, and
  `expr_deallocator`.
- Stable function ID resolution through a narrow `FunctionProcessor` API.
- Main-tree edges for initializer (1), non-constant extent (2), and the
  trivially-destructible delete operand (3).
- One isolated C++20 fixture and SQLite assertions.

## Non-goals

- `synthetic_destructor_call`.
- `expr_reuse`.
- Synthetic destructor, allocator, or deallocator call expression nodes.
- Any P13 metadata work.

## Allowed paths

- `include/core/processor/`, `src/core/processor/`
- `include/model/db/`, `include/db/table_defs/`, `include/db/table_init.h`
- `include/core/ast_visitor.h`, `src/core/ast_visitor.cc`
- `src/db/storage_facade_instantiations.inc`
- `tests/unit-tests/p12/`, `scripts/test_all.sh`, `scripts/assert_test_db.py`
- `.trellis/tasks/p12-allocation-lifetime/`, `docs/roadmap.md`

## Forbidden paths

- Unrelated processor and schema subsystems.
- Tables absent from `docs/datatable-list.txt`.
- Semantic/cache/dependency/database orchestration inside `Visit*` methods.

## Required specs

- `AGENTS.md`, `docs/agent_workflow.md`, `.trellis/workflow.md`
- `.trellis/spec/arborchive/{architecture,ast-visitor-boundary,processor-boundary,schema-codeql-alignment,testing-and-verification}.md`
- `docs/datatable-list.txt`, `docs/semmlecode.cpp.dbscheme`, P12 roadmap.

## Validation

1. Baseline `scripts/test_all.sh`.
2. `python3 scripts/generate_instantiations.py`.
3. `make debug -j 8`.
4. Targeted P12 extraction and `scripts/assert_test_db.py`.
5. `scripts/test_all.sh`.
6. SQLite schema, row, reference, kind, form, graph, and virtual-guard checks.

## Risks

- Clang overload selection differs by ABI/standard-library declaration set.
- A virtual destructor makes the delete target runtime-dependent.
- Array-new `getAllocatedType()` excludes only the outer runtime/constant
  extent, so outer array reconstruction must preserve nested array dimensions.
