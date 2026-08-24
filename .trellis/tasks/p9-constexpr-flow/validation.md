# P9 Validation

## Required commands

```bash
python3 scripts/generate_instantiations.py
git diff --check
make debug -j 8
scripts/test_all.sh
python3 scripts/db_summary.py tests/output/unit-tests-p9-constexpr_flow_case.db
sqlite3 tests/output/unit-tests-p9-constexpr_flow_case.db \
  "select kind, count(*) from stmts where kind in (1,2,35,38,39) group by kind;"
```

## Acceptance evidence

- The P9 fixture is parsed as C++23.
- Statement kinds have the distribution `1:2`, `2:2`, `35:3`, `38:1`,
  `39:1`.
- Ordinary-if relations have counts initialization `2`, then `2`, else `2`.
- Constexpr relations have counts initialization `2`, then `3`, else `3`.
- Consteval relations have counts then `2`, else `2`, with one kind-38 and one
  kind-39 owner in each table.
- Ordinary and constexpr initialization tables each contain one declaration
  statement and one expression statement target.
- Every expression statement target reuses the corresponding `exprs.id`.
- Every owner/child ID resolves to `stmts`; no relationship stores a negative ID.
- All six audited initialization/P9 tables expose the exact expected columns
  and a primary-key owner.
- `if_initialization` exists and `if_initalization` does not.
