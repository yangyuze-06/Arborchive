# P7 Attribute Owner-Link Safe Subset

This note records the P7 owner-link reinforcement result after the P7a/P7b
minimal attribute subset.

P7 remains `PARTIAL`. The safe subsets below are implemented and validated, but
complex attribute argument payload tables remain deferred.

## Implemented Scope

Implemented safe subsets:

- P7a: function attribute presence through `attributes` and `funcattributes`.
- P7b: safe argument payload subset:
  - `DeprecatedAttr` stable string payloads in `attribute_arg_value`.
  - direct integer literal `AlignedAttr` payloads in
    `attribute_arg_constant`.
- P7c: type owner links in `typeattributes` for stable `@usertype` ids:
  - records
  - enums
  - type aliases / typedef names
- P7d: variable owner links in `varattributes` for CodeQL-aligned variable
  entity ids:
  - fields / `@membervariable`
  - global variables / `@globalvariable`
  - parameters / `@parameter`
  - local variables / `@localvariable`
- P7e: statement owner links in `stmtattributes` for conservative
  `AttributedStmt` cases:
  - `[[fallthrough]]` linked to the inner `NullStmt` / `StmtKind::EMPTY`
  - `[[likely]]` linked to the inner `ReturnStmt` / `StmtKind::RETURN`

Deferred scope:

- P7f remains deferred.
- `attribute_arg_type` is not populated.
- `attribute_arg_expr` is not populated.
- `attribute_arg_name` is not populated.
- Generalized `CONSTANT_EXPR` is not implemented.
- Dependent/template attribute arguments are not implemented.
- Arbitrary attribute argument string dumping is not implemented.

## Architecture Boundary

`ASTVisitor` remains a thin delegation layer for this subset. It obtains stable
owner ids from the owning processors and delegates attribute row creation to
`AttributeProcessor`.

`ASTVisitor` does not:

- map attribute kinds;
- serialize attribute arguments;
- interpret constants;
- assemble schema rows;
- insert attribute models directly.

`AttributeProcessor` owns:

- attribute presence extraction;
- kind/name/name_space mapping;
- insertion into `attributes`;
- insertion into `funcattributes`, `typeattributes`, `varattributes`, and
  `stmtattributes`;
- reuse of the existing safe P7b argument extraction path.

## Canonical ParmVarDecl Owner Id Fix

During review, the initial owner-link patch showed that the same parameter
could be processed through both `VisitVarDecl` and `VisitParmVarDecl`. The
first path created the canonical `@parameter` id and stored it in the variable
cache. The second path created a duplicate `params` row and linked
`varattributes` to that duplicate id.

Before the fix, focused SQLite evidence showed:

```text
duplicate_param_signature|304|0|264|2|307,311
param_var_decls|307,311
param_varattribute|311
```

The fix made parameter processing idempotent by reusing the variable cache for
`ParmVarDecl` and by linking parameter attributes to the canonical cached
variable entity id.

After the fix, focused SQLite evidence shows:

```text
params|307
param_var_decls|307
param_varattribute|307
```

The `DeclRefExpr` path resolves the same canonical id:

```text
Found variable ID: 307
```

## SQLite Evidence

Focused fixture:

```text
tests/unit-tests/p7/attribute_owner_links_case.cc
```

Focused database:

```text
tests/output/unit-tests-p7-attribute_owner_links_case.db
```

Core P7 row counts after `scripts/test_all.sh`:

```text
attributes|9
typeattributes|3
varattributes|4
stmtattributes|2
attribute_arg_type|0
attribute_arg_expr|0
attribute_arg_name|0
```

`varattributes` owner evidence:

```text
no_unique_address|252|member|field
maybe_unused|285|global|p7d_global
maybe_unused|307|param|param
maybe_unused|320|local|local
```

Focused DB evidence also validated:

```text
deprecated|243|P7cDeprecatedRecord|1
deprecated|265|P7cDeprecatedEnum|4
deprecated|278|P7cDeprecatedAlias|14
fallthrough|361|26
likely|379|6
```

P7a/P7b compatibility evidence:

```text
250|nodiscard||245
253|deprecated|prefer p7b_plain_function
281|aligned|16
```

## Validation Commands

Commands run for the owner-link reinforcement and ParmVarDecl canonical-id
fix:

```bash
git diff --check
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
./scripts/test_all.sh
./build/demo -c config.example.toml \
  -s tests/unit-tests/p7/attribute_owner_links_case.cc \
  -o tests/output/unit-tests-p7-attribute_owner_links_case.db
python3 scripts/db_summary.py \
  tests/output/unit-tests-p7-attribute_owner_links_case.db
```

`scripts/generate_database.sh` was referenced by the original goal brief, but
that script does not exist in this checkout. The available generation path is
`build/demo` with `-c`, `-s`, and `-o`.

## Status

P7 should remain `PARTIAL` until the complex argument payload family is
implemented and validated. Do not mark the full P7 attribute system `DONE`
while P7f remains deferred.
