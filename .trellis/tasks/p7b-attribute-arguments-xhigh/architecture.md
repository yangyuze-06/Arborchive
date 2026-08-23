# P7b Attribute Argument Architecture

## Task Goal

Implement the first reviewable P7b Attribute Argument System slice while
preserving CodeQL semantic alignment and the Arborchive processor boundary.

The first slice intentionally supports only stable scalar argument categories:

- string token/value arguments exposed by selected Clang attribute APIs;
- integer constant arguments backed by stable literal expression rows.

## Ownership Boundaries

`AttributeProcessor` owns the P7b semantic pipeline:

- attribute argument traversal;
- argument category classification;
- argument kind mapping to CodeQL-compatible integer kinds;
- stable scalar extraction;
- relationship from argument rows to owning attribute rows;
- DB model assembly for `attribute_args` and typed `attribute_arg_*` rows.

`ExprProcessor` may be used only as the expression-domain owner for creating or
reusing literal `@expr` rows needed by `attribute_arg_constant`. It does not
decide whether an attribute argument is supported.

`ASTVisitor` remains traversal/delegation only. It must not inspect attribute
arguments, map argument kinds, interpret constants, serialize values, or insert
attribute argument rows.

## Visitor Responsibilities

The visitor may:

- initialize `AttributeProcessor` with helper processor dependencies;
- call attribute processing after a target entity ID is known.

The visitor must not:

- branch on concrete `clang::Attr` subclasses;
- build `DbModel::AttributeArg` or related rows;
- choose serialization or deferral policy;
- coordinate CodeQL argument tables directly.

For this first P7b slice, no new visitor entrypoint is expected beyond the
existing function-attribute delegation added by P7a.

## Processor Responsibilities

`AttributeProcessor` records an attribute presence row first, then records
supported arguments for that same attribute ID. This keeps presence and
argument ownership adjacent but not collapsed:

- `recordAttribute` remains responsible for the `attributes` row;
- `recordAttributeArguments` owns argument rows for an existing attribute ID;
- specific helpers record string token arguments and integer constant
  arguments.

Unsupported argument forms are skipped explicitly. They are not serialized into
opaque text.

## Schema Layering

The schema uses the CodeQL-approved P7b table family from
`docs/datatable-list.txt` and `docs/semmlecode.cpp.dbscheme`:

- `attribute_args(id, kind, attribute, index, location)`;
- `attribute_arg_value(arg, value)`;
- `attribute_arg_type(arg, type_id)`;
- `attribute_arg_constant(arg, constant)`;
- `attribute_arg_expr(arg, expr)`;
- `attribute_arg_name(arg, name)`.

`attribute_args` is the common identity and owner layer. The typed tables carry
category-specific payloads. This avoids a catch-all value column and preserves
kind distinction for CodeQL-style APIs such as `getValueText`,
`getValueConstant`, `getValueInt`, `getValueExpr`, `getValueType`, and named
arguments.

## Argument Semantic Model

The first slice maps:

- string token/value argument: `kind = 1` (`@attribute_arg_token`) plus
  `attribute_arg_value`;
- integer constant argument: `kind = 2` (`@attribute_arg_constant`) plus
  `attribute_arg_constant`, where the target is a stable literal `@expr`.

The integer literal expression is also represented through the existing
`exprs`, `values`, `valuetext`, and `valuebind` tables, so SQLite evidence can
prove the constant value without storing an attribute-specific blob.

## CodeQL Alignment Reasoning

CodeQL separates the attribute entity from its arguments and separates argument
identity from typed value relations. Arborchive follows the same layering:

- `Attribute` -> `attributes`;
- `AttributeArgument` -> `attribute_args`;
- text/token value -> `attribute_arg_value`;
- constant value -> `attribute_arg_constant` linked to existing `@expr` value
  tables;
- type/expr/name relations remain schema-ready but deferred unless supported.

The implementation does not copy CodeQL internals. It preserves the observable
semantic distinctions that CodeQL users expect.

## Supported Argument Categories

Initial supported categories:

- `DeprecatedAttr` message string, for `[[deprecated("reason")]]`;
- `DeprecatedAttr` replacement string when present;
- `AlignedAttr` integer expression argument when it reduces to a stable
  integer literal, for `[[gnu::aligned(16)]]`.

These categories are selected because LLVM19 exposes stable typed accessors and
they can be validated with small source fixtures.

## Unsupported Argument Categories

Deferred in this slice:

- arbitrary `Expr *` argument persistence;
- unstable APValue or AST dump stringification;
- non-literal constant expressions such as `8 + 8`;
- type arguments in attributes;
- named arguments;
- variadic attribute argument families not explicitly modeled here;
- raw token streams.

Unsupported arguments are skipped rather than collapsed into text.

## Serialization Policy

Allowed serialization:

- stable string values returned by typed Clang APIs such as
  `DeprecatedAttr::getMessage()`;
- stable scalar integer values from direct literal arguments and existing
  expression value tables.

Forbidden serialization:

- raw AST dumps;
- compiler-version-dependent APValue dumps;
- opaque JSON/text blobs;
- one-field collapse of every argument kind.

## Extensibility Strategy

Future P7b slices should add one argument family at a time. Each new family must
document:

- the Clang API used;
- the CodeQL table/kind mapping;
- unsupported/deferred cases;
- a fixture and SQLite query proving row shape and value semantics.

Type, expr, and named arguments should use the existing `attribute_arg_type`,
`attribute_arg_expr`, and `attribute_arg_name` tables when their semantics are
stable enough to review.
