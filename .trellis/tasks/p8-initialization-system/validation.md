# P8 Initialization System Validation

## required commands

```bash
git diff --check
python3 scripts/generate_instantiations.py
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
scripts/test_all.sh
```

## P8 database target

```text
tests/output/unit-tests-p8-initialization_case.db
```

## SQL evidence queries

Row counts:

```bash
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select count(*) from initialisers;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select count(*) from braced_initialisers;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select count(*) from aggregate_field_init;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select count(*) from aggregate_array_init;"
```

Representative rows:

```bash
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select init, var, expr, location from initialisers order by init;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select init from braced_initialisers order by init;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select aggregate, initializer, field, position from aggregate_field_init order by aggregate, position;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select aggregate, initializer, element_index, position from aggregate_array_init order by aggregate, position;"
```

Join checks:

```bash
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select i.init, i.var, v.name, i.expr from initialisers i join var_decls v on v.variable = i.var order by i.init;"
sqlite3 tests/output/unit-tests-p8-initialization_case.db "select b.init, i.expr from braced_initialisers b join initialisers i on i.init = b.init order by b.init;"
```

## expected evidence

- `initialisers` has rows for the global scalar, braced scalar, struct
  aggregate, array aggregate, local scalar, local braced scalar, local struct
  aggregate, and local array aggregate fixture variables.
- `braced_initialisers` has rows for the braced scalar and aggregate
  initializers, and its `init` values match `initialisers.init`, not `exprs.id`.
- `aggregate_field_init` has representative rows for struct aggregate
  initialization.
- `aggregate_array_init` has representative rows for array aggregate
  initialization.
- `initialisers.var` joins through `var_decls.variable`, proving it stores the
  direct variable entity id rather than `var_decls.id`.

## command results

Final validation results:

```text
git diff --check
PASS

python3 scripts/generate_instantiations.py
PASS: Successfully generated 156 instantiations.

make debug -j 8
FAIL: default LLVM_CONFIG resolved to LLVM 22.1.4.
Reason: Arborchive requires LLVM 19. This is a toolchain selection issue, not a
P8 code/test failure.

make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
PASS
Notes: existing LLVM/sqlite_orm/header warnings remain; no P8 compile error.

scripts/test_all.sh
PASS
```

The first P8 SQL pass showed `aggregate_array_init = 0` and
`aggregate_field_init.initializer = -1` rows. That exposed an existing
`InitListExpr` child-expression traversal-order issue in the already-DONE
aggregate path. A narrow fix was applied:

- preserve existing aggregate extraction ownership in `ExprProcessor`;
- add CodeQL `#keyset[aggregate, position]` composite primary keys to the two
  aggregate tables;
- write placeholder aggregate rows and dependency-resolve child initializer
  expression ids after traversal.

Final P8 row counts:

```text
initialisers|8
braced_initialisers|6
aggregate_field_init|8
aggregate_array_init|10
```

Final representative `initialisers` rows:

```text
init  var  expr  location
95    89   98    93
107   101  110   105
125   119  129   123
150   143  158   148
194   188  200   192
210   204  213   208
231   225  235   229
267   260  275   265
```

Final `initialisers` to variable-name evidence:

```text
init  var  name              expr
95    89   p8_global_scalar  98
107   101  p8_braced_scalar  110
125   119  p8_global_point   129
150   143  p8_global_array   158
194   188  p8_local_scalar   200
210   204  p8_local_braced   213
231   225  p8_local_point    235
267   260  p8_local_array    275
```

Final `braced_initialisers` join evidence:

```text
init  expr
107   110
125   129
150   158
210   213
231   235
267   275
```

This proves `braced_initialisers.init` references `initialisers.init`, not the
`InitListExpr` / `@expr` id.

Final `aggregate_field_init` rows:

```text
aggregate  initializer  field  position
129        135          76     0
129        140          77     1
132        135          76     0
132        140          77     1
235        241          76     0
235        244          77     1
238        247          76     0
238        253          77     1
```

Final `aggregate_array_init` rows:

```text
aggregate  initializer  element_index  position
158        164          0              0
158        169          1              1
158        174          2              2
161        164          0              0
161        169          1              1
161        174          2              2
275        287          0              0
275        293          1              1
278        287          0              0
278        293          1              1
```

No unresolved initializer expression placeholders remain:

```text
initialisers_expr_minus1            0
aggregate_field_initializer_minus1  0
aggregate_array_initializer_minus1  0
```
