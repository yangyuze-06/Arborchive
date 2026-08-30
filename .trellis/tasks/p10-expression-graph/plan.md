# P10 Expression Graph Core Plan

## scope

1. Add and register `ExprParent` / `exprparents` without a schema primary key.
2. Add `ExprProcessor::recordExprParent(...)` and ID-based support for the
   already-supported expression kinds.
3. Record CodeQL indices for unary, binary, conditional, calls, array
   subscript, init-list, and expression-form `sizeof`/`alignof`.
4. Connect supported statement and P8 initialiser roots through explicit
   processor dependencies.
5. Reuse expression IDs when materializing `StmtKind::EXPR` roots.
6. Regenerate storage instantiations, add the P10 fixture and SQLite assertions,
   then update the roadmap/task evidence.

## allowed paths

- expression model/table/processor files;
- statement and initialization processor headers/implementations;
- processor construction in `src/core/ast_visitor.cc`;
- `include/db/table_init.h` and generated storage instantiations;
- `tests/unit-tests/p10/`, `scripts/assert_test_db.py`, `scripts/test_all.sh`;
- `docs/datatable-list.txt`, `docs/roadmap.md`;
- `.trellis/tasks/p10-expression-graph/`.

## forbidden paths and non-goals

- `.trellis/spec/`;
- unrelated `ASTVisitor::Visit*` logic;
- P11 casts/conversions, P12 allocation, attribute arguments;
- `expr_ancestor`, range-for, case-expression, or new expression-kind work;
- CLI/config/database-field compatibility changes.

## validation plan

```bash
python3 scripts/generate_instantiations.py
git diff --check
make debug -j 8
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
scripts/test_all.sh
python3 scripts/db_summary.py tests/output/unit-tests-p10-expression_graph_case.db
sqlite3 tests/output/unit-tests-p10-expression_graph_case.db \
  'select expr_id, child_index, parent_id from exprparents order by parent_id, child_index;'
```
