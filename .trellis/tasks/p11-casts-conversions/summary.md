# P11 Casts & Conversion System Summary

## status

DONE on `codex/p11-casts-conversions`.

## implementation

- Registered five CodeQL-aligned conversion/type/load/generated facts.
- Replaced visitor-side implicit-cast orchestration with thin cast and paren
  dispatch; `ExprProcessor` owns all P11 semantics.
- Preserved non-conversion expression keys and made same-location conversion
  wrappers distinct.
- Recorded one expression type/value category per supported expression.
- Modeled lvalue loads without wrappers, array decay as kind 8, other implicit
  casts as kind 214, parentheses as kind 12, and explicit casts as 210–214.
- Classified conversion semantics as kinds 0–7 and kept implicit/explicit
  `compgenerated` behavior separate.
- Preserved the P10 main-expression graph boundary.

## commits

- `5808032 feat(db): add P11 conversion fact tables`
- `701fca1 feat(expr): extract P11 cast conversion semantics`
- `c8af902 test(p11): verify cast conversion closure`

## verification

- ORM generation: 167 instantiations.
- LLVM 19 debug build: PASS.
- P11 targeted SQLite assertions: PASS, 0 warnings.
- Full `scripts/test_all.sh`: PASS.
- P11 database: 167 tables, 44 expressions, 24 conversion edges, 44 type
  rows, 15 loads, 10 generated nodes, and 21 conversion-kind rows.
- Rollback on a copy restored the original hash and behavior; the transaction
  modified artifact remains changed.

## deferred / risk

- Deferred: dependent casts, reference wrappers, temporary initialization,
  C11 generic, Objective-C/address-space casts, and `BuiltinBitCastExpr`.
- Conversion kind 7 is implemented but is not forced through a temporary
  construction fixture because `temp_init` remains explicitly deferred.
- Existing third-party compiler warnings remain unchanged.
