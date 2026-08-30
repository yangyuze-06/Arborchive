# P10 Expression Graph Core Summary

## status

DONE on `codex/p10-expression-graph`.

## implemented behavior

- Added CodeQL-aligned `exprparents(expr_id, child_index, parent_id)`.
- Kept conversion wrappers outside the main graph through
  `IgnoreParenCasts()`.
- Added CodeQL child indices for supported expression, call, statement, and
  initialiser parents.
- Kept `ASTVisitor` dispatch-oriented; graph semantics remain processor-owned.
- Added the P10 regression fixture and executable SQLite assertions.

## validation results

- `python3 scripts/generate_instantiations.py`: PASS, 162 instantiations.
- `git diff --check`: PASS.
- `make debug -j 8`: exit 2 because the default toolchain is LLVM 22.1.4.
- `make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8`:
  PASS; existing third-party/LLVM warnings remain.
- `scripts/test_all.sh`: PASS, including P10 SQLite assertions.
- P10 database: 162 tables, 69 `exprparents` rows, assertion result
  `PASS (0 warnings)`.
- Review fixes:
  - parenthesized ExprStmt roots now reuse the normalized main-expression ID;
  - conditional-index assertions no longer depend on SQLite aggregation order.
- Rollback was executed on a copy and restored the original SHA-256; the
  transaction `MODIFIED_FILE` remains changed.

## remaining risks

- P10 intentionally covers only already-supported expression kinds.
- Source-location expression keys remain the identity mechanism.
- Range-for, case expressions, conversions/casts, allocation, attribute
  arguments, and `expr_ancestor` remain deferred to their planned phases.
