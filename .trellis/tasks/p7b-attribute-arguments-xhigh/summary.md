# P7b Attribute Argument System Summary

## changed files

- `.trellis/tasks/p7b-attribute-arguments-xhigh/architecture.md`
- `.trellis/tasks/p7b-attribute-arguments-xhigh/schema-plan.md`
- `.trellis/tasks/p7b-attribute-arguments-xhigh/validation-plan.md`
- `.trellis/tasks/p7b-attribute-arguments-xhigh/risk-analysis.md`
- `.trellis/tasks/p7b-attribute-arguments-xhigh/summary.md`
- `include/model/db/attribute.h`
- `include/db/table_defs/attribute.h`
- `include/db/table_init.h`
- `include/core/processor/attribute_processor.h`
- `src/core/processor/attribute_processor.cc`
- `include/core/processor/expr_processor.h`
- `src/core/processor/expr_processor.cc`
- `src/core/ast_visitor.cc`
- `src/db/storage_facade_instantiations.inc`
- `tests/unit-tests/p7/attribute_arguments_case.cc`
- `scripts/test_all.sh`

## architecture decisions

P7b remains owned by `AttributeProcessor`. It records argument rows only after
the owning `attributes` row exists. The visitor does not inspect or classify
arguments.

`ExprProcessor` provides a small cache-aware helper for integer literal
expression rows used by `attribute_arg_constant`. This keeps expression row
creation in the expression domain while leaving attribute argument semantics in
the attribute processor.

Presence and arguments are adjacent but not collapsed: `recordAttribute`
records the attribute presence row, while `recordAttributeArguments` and typed
helpers record argument rows.

## schema decisions

Added the CodeQL-approved P7b table family:

- `attribute_args`
- `attribute_arg_value`
- `attribute_arg_type`
- `attribute_arg_constant`
- `attribute_arg_expr`
- `attribute_arg_name`

No Arbor extension table was added. P7a presence tables remain in place.

`attribute_args.kind` follows `docs/semmlecode.cpp.dbscheme`:

- `1` means token/text argument;
- `2` means constant argument.

The first implementation populates only those two safe kinds.

## CodeQL alignment statement

The implementation preserves the CodeQL-style split:

- `Attribute` -> `attributes`
- `AttributeArgument` -> `attribute_args`
- text/token payload -> `attribute_arg_value`
- constant payload -> `attribute_arg_constant` referencing existing `@expr`

Integer values are proven through existing `exprs`, `valuebind`, and `values`
rows. Arbitrary arguments are not collapsed into a text blob.

## supported semantics

Supported in this first slice:

- `[[deprecated("text")]]` message string as `kind = 1` plus
  `attribute_arg_value`;
- `DeprecatedAttr` replacement string when Clang exposes one;
- `[[gnu::aligned(16)]]` direct integer literal as `kind = 2` plus
  `attribute_arg_constant`.

The P7b fixture is `tests/unit-tests/p7/attribute_arguments_case.cc`.

## unsupported semantics

Deferred intentionally:

- arbitrary `Expr *` argument persistence;
- APValue stringification;
- raw AST dumps;
- non-literal constant expressions such as `8 + 8`;
- type arguments;
- named arguments;
- broad variadic attribute families;
- empty-string argument presence where Clang does not expose a stable
  present-vs-absent distinction in this path.

## ASTVisitor boundary result

`src/core/ast_visitor.cc` only changed processor initialization so
`AttributeProcessor` receives the existing `ExprProcessor` dependency. No
visitor code maps attribute argument kinds, interprets constants, serializes
values, or assembles schema rows.

## processor ownership

`AttributeProcessor` owns:

- concrete supported attribute family dispatch;
- argument kind mapping;
- argument owner relation;
- stable string extraction;
- stable integer-literal extraction policy;
- `attribute_args` and typed payload row assembly.

`ExprProcessor` owns the literal `@expr` row and value binding used by
`attribute_arg_constant`.

## validation commands

```bash
git diff --check
python3 scripts/generate_instantiations.py
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
scripts/test_all.sh
python3 scripts/db_summary.py tests/output/unit-tests-p7-attribute_arguments_case.db
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db ".tables"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select * from attributes order by id;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select * from funcattributes order by func_id, spec_id;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select id, kind, attribute, \"index\", location from attribute_args order by id;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select * from attribute_arg_value order by arg;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select * from attribute_arg_constant order by arg;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select aa.id, a.id, a.name, a.name_space, a.kind, aa.kind, aa.\"index\" from attribute_args aa join attributes a on aa.attribute = a.id order by aa.id;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select aa.id, a.name, v.str from attribute_args aa join attributes a on aa.attribute = a.id join attribute_arg_constant aac on aac.arg = aa.id join valuebind vb on vb.expr = aac.constant join \"values\" v on v.id = vb.val order by aa.id;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select aa.id, a.name, av.value from attribute_args aa join attributes a on aa.attribute = a.id join attribute_arg_value av on av.arg = aa.id order by aa.id;"
sqlite3 tests/output/unit-tests-p7-attribute_presence_case.db "select * from attributes order by id; select * from funcattributes order by func_id, spec_id;"
```

One exploratory SQLite query initially used unquoted `values` and failed
because `values` is a SQL keyword. The corrected quoted-table query above
passed.

## validation results

- `git diff --check`: passed.
- `python3 scripts/generate_instantiations.py`: passed; generated 154
  instantiations.
- `make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8`:
  passed. LLVM/third-party warnings and macOS SDK link warnings remain
  non-fatal.
- `scripts/test_all.sh`: passed.
- `db_summary.py` for P7b fixture: passed; database has 154 tables.

## SQLite evidence

P7b attribute rows:

```text
78|1|deprecated||76
101|1|aligned|gnu|99
```

P7b function-owner rows:

```text
73|78
96|101
```

P7b argument rows:

```text
81|1|78|0|79
109|2|101|0|107
```

String argument payload:

```text
81|deprecated|prefer p7b_plain_function
```

Integer constant payload and resolved value:

```text
109|104
109|aligned|16
```

Owner/kind distinction:

```text
81|78|deprecated||1|1|0
109|101|aligned|gnu|1|2|0
```

P7a presence rows after the P7b change:

```text
78|1|nodiscard||76
73|78
```

Deferred typed tables are present and empty in the P7b fixture:

```text
attribute_arg_type: 0
attribute_arg_expr: 0
attribute_arg_name: 0
```

## limitations

Only function attributes are exercised because P7a currently delegates function
attributes. Type, variable, and statement attribute argument extraction remain
future work.

The integer constant path supports direct integer literals after a narrow
`ConstantExpr` unwrap. It does not evaluate arbitrary constant expressions.

The string path records stable strings exposed by typed Clang attribute APIs; it
does not parse raw tokens.

## follow-ups

- Add variable/type/statement attribute argument extraction after the
  corresponding P7a presence owner links are populated.
- Add type argument support through `attribute_arg_type` once a stable
  attribute family and fixture are selected.
- Add named argument support through `attribute_arg_name` with a dedicated
  CodeQL-aligned fixture.
- Consider broader expression argument support only after the expression graph
  phase can provide stable `@expr` identities.

## risks

- Attribute API coverage is intentionally narrow.
- Existing `VisitFunctionDecl` still has legacy multi-processor orchestration;
  this patch does not expand semantic argument logic there.
- Existing tests still emit known non-fatal unresolved dependency warnings for
  some type keys.
- Empty deprecated messages are skipped because this slice avoids guessing
  whether an empty string was explicit or absent.

## suggested commit message

```text
p7b: add minimal attribute argument extraction
```
