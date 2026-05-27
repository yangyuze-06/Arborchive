# P7b Schema Plan

## Schema Scope

Add the CodeQL-approved P7b attribute argument tables that are already listed in
`docs/datatable-list.txt`:

- `attribute_args`;
- `attribute_arg_value`;
- `attribute_arg_type`;
- `attribute_arg_constant`;
- `attribute_arg_expr`;
- `attribute_arg_name`.

No Arbor extension table is planned.

## CodeQL Counterpart

The table family directly corresponds to the CodeQL C/C++ attribute argument
schema in `docs/semmlecode.cpp.dbscheme`:

- `attribute_args.id` is the `@attribute_arg` identity;
- `attribute_args.kind` preserves the CodeQL argument kind enum;
- `attribute_args.attribute` links back to `@attribute`;
- `attribute_args.index` stores argument order;
- `attribute_args.location` links to `@location_default`;
- typed tables preserve text, type, constant, expression, and name relations.

## Argument Kind Enum

The implementation will document the enum in `include/model/db/attribute.h`:

- `0`: empty;
- `1`: token/text;
- `2`: constant;
- `3`: type;
- `4`: constant expression;
- `5`: expression.

The first implementation populates only `1` and `2`.

## Field Semantics

`attribute_args.attribute` always references a row in `attributes`.

`attribute_args.index` is zero-based argument order within the owning attribute.

`attribute_args.location` uses the best stable source location available. For
string arguments exposed only as a semantic string by Clang, the attribute
location is used. For integer literal arguments, the literal expression
location is used.

`attribute_arg_value.value` is for stable text/token values, not a catch-all
serializer.

`attribute_arg_constant.constant` references an existing expression row. The
value itself is proven through `valuebind` and `values`.

## Migration Impact

This is an additive schema change. P7a presence tables remain unchanged.

Required synchronized files:

- `include/model/db/attribute.h`;
- `include/db/table_defs/attribute.h`;
- `include/db/table_init.h`;
- generated `src/db/storage_facade_instantiations.inc`.

## Validation Queries

Representative checks:

```sql
select id, kind, attribute, "index", location from attribute_args order by id;
select arg, value from attribute_arg_value order by arg;
select arg, constant from attribute_arg_constant order by arg;
select aa.id, a.name, aa.kind, aa."index"
from attribute_args aa join attributes a on aa.attribute = a.id
order by aa.id;
select v.str
from attribute_arg_constant aac
join valuebind vb on vb.expr = aac.constant
join values v on v.id = vb.val
order by aac.arg;
```

These queries prove row existence, owner relation, kind distinction, string
extraction, integer extraction, and relationship to existing expression value
tables.

## Alignment Risk

Main risks:

- treating arbitrary expressions as constants without stable expression
  modeling;
- using `attribute_arg_value` as a universal blob;
- recording an argument row without a typed payload.

Mitigation: only support reviewed string and integer literal categories; defer
complex `Expr` and APValue forms.
