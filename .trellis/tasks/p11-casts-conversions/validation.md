# P11 Casts & Conversion System Validation

## schema review

- Category: CodeQL-aligned tables.
- Counterparts: `exprconv`, `expr_types`, `expr_isload`, `compgenerated`, and
  `conversionkinds` match `docs/semmlecode.cpp.dbscheme` names, column order,
  direction, and unique-side semantics.
- Arbor-specific extension: none.
- Alignment risk: conversion kind 7 depends on Clang exposing a supported
  same-class prvalue wrapper; the mapping is implemented, while temporary
  construction remains deferred rather than broadening P11 into `temp_init`.
- Migration: supported implicit casts formerly stored as kind 217 now use kind
  214; the five new tables and changed kind require rebuilding old databases.
- Fixture: `tests/unit-tests/p11/casts_conversion_case.cc`.

## validation commands and results

```text
BASELINE: scripts/test_all.sh
RESULT: [test_all] All checks passed.
EXIT: 0

python3 scripts/generate_instantiations.py
RESULT: Successfully generated 167 instantiations
EXIT: 0

make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
RESULT: linked build/demo; existing LLVM/sqlite_orm warnings only
EXIT: 0

TARGETED: python3 scripts/assert_test_db.py unit-tests/p11/casts_conversion_case tests/output/unit-tests-p11-casts_conversion_case.db
RESULT: [assert_db] unit-tests/p11/casts_conversion_case: PASS (0 warnings)
EXIT: 0

MODIFIED: scripts/test_all.sh
RESULT: [test_all] All checks passed.
EXIT: 0
```

An intermediate modified run exposed two P8 aggregate initializer orphans.
The cause was an implicit lvalue-to-rvalue conversion whose direct source was
an already-valid field read that lacked an on-demand expression identity. The
processor now materializes that field read only when needed as a conversion
source; P8 targeted assertions and the complete suite then passed.

## database evidence

Database:
`tests/output/unit-tests-p11-casts_conversion_case.db`

```text
exprs            44
exprconv         24
expr_types       44
expr_isload      15
compgenerated    10
conversionkinds  21
```

Value categories: prvalue 23, xvalue 1, lvalue 20. Conversion-kind evidence
covers 0–6, including bool, both inheritance directions, both member-pointer
directions, and non-inheritance class glvalue adjustment. Assertions also prove
one type row per expression, one conversion-kind row per kind 210–214 cast,
nested conversion edges, generated/explicit separation, absence of kind 217,
and exclusion of conversion nodes from `exprparents`.

## rollback evidence

The executable transaction rollback was run against a copy. It restored SHA-256
`7dcf6cbcb926c7dd3244f8d91db4a0fd4496ec3236f2a7c16ee6c2f3b38ec1d6`
and the legacy `processImplicitCastExpr` behavior while the recorded
`MODIFIED_FILE` retained SHA-256
`4607a14249a67b63693fce7362278f79a07cf51b7935318c2b627d83df64eccd`.
