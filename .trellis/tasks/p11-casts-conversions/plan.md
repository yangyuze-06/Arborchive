# P11 Casts & Conversion System Plan

## scope

1. Add the CodeQL-aligned `exprconv`, `expr_types`, `expr_isload`,
   `compgenerated`, and `conversionkinds` facts.
2. Make `ExprProcessor` own expression type/value-category facts, cast type
   processing, conversion-chain construction, and conversion-kind mapping.
3. Replace visitor-side implicit-cast orchestration with thin `CastExpr` and
   `ParenExpr` dispatch.
4. Extend only cast/parenthesis expression keys so same-location wrappers stay
   distinct while all non-conversion expression keys remain unchanged.
5. Add the isolated P11 fixture, SQLite assertions, roadmap/table status, and
   migration/deferred evidence.

## non-goals

- dependent casts;
- `reference_to`, `ref_indirect`, and `temp_init`;
- C11 generic selection;
- Objective-C/address-space-specific conversions;
- `BuiltinBitCastExpr`;
- allocation, lifetime, CFG, or attribute expression work.

## allowed paths

- `include/model/db/expr.h`, `include/db/table_defs/expr.h`,
  `include/db/table_init.h`, generated storage instantiations;
- expression key generator, `ExprProcessor`, and the narrow `TypeProcessor`
  routing needed for expression-owned builtin/record type identities;
- thin cast/paren declarations and definitions in `ASTVisitor`;
- `tests/unit-tests/p11/`, `scripts/assert_test_db.py`,
  `scripts/test_all.sh`;
- `docs/roadmap.md`, `docs/datatable-list.txt`;
- `.trellis/tasks/p11-casts-conversions/`.

## forbidden paths

- unrelated processors and schema tables;
- `.trellis/spec/` governance;
- P12+ semantic subsystems.

## validation

```bash
python3 scripts/generate_instantiations.py
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
scripts/test_all.sh
python3 scripts/db_summary.py tests/output/unit-tests-p11-casts_conversion_case.db
sqlite3 tests/output/unit-tests-p11-casts_conversion_case.db '<P11 checks>'
```

## risks

- Recursive AST traversal can revisit wrappers, so all relation tables need
  stable identity and idempotent insertion.
- Clang may synthesize more nested casts than source syntax suggests; tests
  assert invariants and representative categories rather than brittle totals
  except where an exact per-cast rule is required.
- Existing source-location keys collide for nested wrappers; only conversion
  keys may change to avoid regressing established expression identity.
