# P12 Allocation & Lifetime Summary

## Implemented

- Added CodeQL-aligned allocation tables: `expr_allocator`,
  `expr_deallocator`, `new_allocated_type`, and
  `new_array_allocated_type`.
- Added new/delete expression kinds 87/129 and 88/128 in `ExprProcessor`.
- Reconstructed array-new allocated types with a constant or incomplete outer
  extent while preserving nested fixed arrays.
- Added allocator/deallocator form classification and a narrow
  `FunctionProcessor::resolveFunctionReference()` materialization API.
- Added safe main-tree edges for initializer (1), dynamic extent (2), and
  trivially-destructible delete operand (3).
- Guarded only runtime-selected virtual-destructor deletes from misleading
  static deallocator rows; global, array, and final cases retain static facts.
- Deferred dependent new/delete patterns before P12 expression and allocated
  type materialization.
- Kept synthetic lifetime call nodes and `expr_reuse` deferred.

## CodeQL/schema review

- Category: CodeQL-aligned tables already listed in
  `docs/datatable-list.txt` and defined by `docs/semmlecode.cpp.dbscheme`.
- Arbor-specific extension: none.
- Alignment risk: Clang overload selection is persisted directly; function IDs
  are materialized before relation insertion, and negative IDs are rejected.
- Migration: old databases must be rebuilt because four tables and four emitted
  expression kinds are new.
- Database evidence: `evidence/sql-verification.log` records schema shape,
  representative counts/forms, array shapes, graph edges, reference integrity,
  runtime virtual guarding, and static global/array/final relationships.

## Verification

- Baseline `scripts/test_all.sh`: PASS, exit 0.
- ORM generation: 171 model instantiations.
- LLVM 19 debug build: PASS.
- Modified full `scripts/test_all.sh`, including P12: PASS, exit 0.
- P12 SQLite assertions: PASS, 0 warnings.
- `git diff --check`: PASS.

## Known limitations

- `synthetic_destructor_call`, `expr_reuse`, and compiler-generated call
  materialization remain deferred until stable generated-expression identity
  and routine-call materialization specifications exist.
- The extractor deliberately records only the requested safe main-expression
  child subset; placement arguments and synthetic calls are not graph children.
