# P10 Expression Graph Core Validation

## acceptance checks

- exact columns: `expr_id, child_index, parent_id`;
- every child resolves to `exprs`;
- every parent resolves to `exprs`, `stmts`, or `initialisers`;
- no negative entity IDs, duplicate relations, duplicate main parents, or
  expression-parent self loops;
- representative CodeQL indices for expressions, calls, statements, and
  initialisers;
- parenthesized expression statements normalize to the underlying main-tree
  expression before materializing `StmtKind::EXPR`;
- conversion kinds do not enter the main graph;
- full regression suite passes without new warning classes.

## database target

```text
tests/output/unit-tests-p10-expression_graph_case.db
```

## evidence commands

```bash
python3 scripts/db_summary.py tests/output/unit-tests-p10-expression_graph_case.db
sqlite3 tests/output/unit-tests-p10-expression_graph_case.db \
  'select expr_id, child_index, parent_id from exprparents order by parent_id, child_index;'
python3 scripts/assert_test_db.py unit-tests/p10/expression_graph_case \
  tests/output/unit-tests-p10-expression_graph_case.db
```

## final results

- LLVM 19 debug build: PASS.
- P10 targeted SQLite assertions: PASS, 69 `exprparents` rows.
- Full `scripts/test_all.sh`: PASS.
- Exact command evidence is also recorded in `summary.md` and
  `transaction/VERIFICATION.txt`.
