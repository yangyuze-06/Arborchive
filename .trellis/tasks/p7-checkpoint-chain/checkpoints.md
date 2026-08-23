# P7 Checkpoint Chain Report

This report records checkpoint outcomes for the P7 continuation chain. It is
task-specific context only and does not change roadmap status, schema, model,
table initialization, or `docs/datatable-list.txt`.

## Checkpoint 3: `attribute_arg_type` Design Gate

Status: BLOCKED. No `attribute_arg_type` candidate is SAFE NOW.

### Scope Audited

Repository context audited:

- `.trellis/tasks/p7-checkpoint-chain/goal.md`
- `docs/analysis/p7_full_done_criteria.md`
- `docs/analysis/p7f_attribute_expr_design.md`
- `docs/analysis/p7_remaining_attribute_scope_audit.md`
- `docs/analysis/p7_attribute_owner_links.md`
- `docs/roadmap.md`
- `include/core/processor/type_processor.h`
- `src/core/processor/type_processor.cc`
- `include/core/processor/attribute_processor.h`
- `src/core/processor/attribute_processor.cc`
- `include/core/processor/expr_processor.h`
- `src/core/processor/expr_processor.cc`
- `tests/unit-tests/p7/`

Local Clang 19 generated attribute APIs were also inspected through
`/opt/homebrew/opt/llvm@19/include/clang/AST/Attrs.inc`.

### Attr Type Payload Inventory

Observed TypeSourceInfo-backed or QualType-backed candidate APIs include:

- `AlignedAttr::getAlignmentType()` for type-form alignment attributes.
- `VecTypeHintAttr::getTypeHint()` / `getTypeHintLoc()`.
- `TypeTagForDatatypeAttr::getMatchingCType()` / `getMatchingCTypeLoc()`.
- `IBOutletCollectionAttr::getInterface()` / `getInterfaceLoc()`.
- `OwnerAttr::getDerefType()` / `getDerefTypeLoc()`.
- `PointerAttr::getDerefType()` / `getDerefTypeLoc()`.
- `PreferredNameAttr::getTypedefType()` / `getTypedefTypeLoc()`.
- `PreferredTypeAttr::getType()` / `getTypeLoc()`.

These APIs prove that type-bearing attributes exist, but they do not prove that
Arborchive can safely populate `attribute_arg_type` today.

### TypeProcessor Audit

Current `TypeProcessor::processType(const Type *T)` canonicalizes first:

```cpp
const QualType qualType = T->getCanonicalTypeInternal();
const clang::Type *type = qualType.getTypePtr();
```

That means a TypeSourceInfo-backed attribute argument would currently be routed
through canonical type identity, not through an explicit source spelling policy.
For attribute arguments, this leaves alias/typedef spelling behavior
undocumented.

`TypeProcessor::processDependentType(QualType QT)` is also not safe for P7 type
arguments. It may create template-parameter ids for some dependent types and,
for broader dependent types, falls back to:

```cpp
std::string typeName = QT.getAsString(pp_);
```

The P7 checkpoint hard rules forbid using `getAsString()` as an
`attribute_arg_type` payload and forbid processing dependent types for this
checkpoint.

Other relevant observations:

- Existing type owner links use stable ids for records, enums, typedefs, and
  aliases, but owner links are not the same semantic surface as
  `AttributeArgument.getValueType()`.
- `attribute_arg_type.type_id` is `@type ref` in the dbscheme, while existing
  owner-link evidence often references `usertypes` ids. The `@type` vs
  `@usertype` policy for attribute type payloads is still not documented
  enough for implementation.
- Existing derived-type processing uses string names for derived type rows.
  That may be acceptable for ordinary type-table extraction, but it is not a
  proven payload policy for attribute type arguments.

### Candidate Verdicts

| Candidate | API | Verdict | Blocker |
| --- | --- | --- | --- |
| `AlignedAttr` type form | `TypeSourceInfo *getAlignmentType()` | DEFER | Closest candidate, but current type path canonicalizes and does not prove alias/spelling behavior for `alignas(T)` or GNU type-form alignment. |
| `VecTypeHintAttr` | `TypeSourceInfo *getTypeHintLoc()` | DEFER | OpenCL-specific semantics and fixture/toolchain support need separate proof; still blocked by TypeProcessor type payload policy. |
| `TypeTagForDatatypeAttr` | `TypeSourceInfo *getMatchingCTypeLoc()` plus identifier/bool fields | DEFER | Mixed payload family; implementing only the type field would require argument-index and non-type field policy first. |
| `IBOutletCollectionAttr` | `TypeSourceInfo *getInterfaceLoc()` | DEFER | Objective-C-specific semantics and validation fixture support are not proven for this phase. |
| `OwnerAttr` / `PointerAttr` | `TypeSourceInfo *getDerefTypeLoc()` | DEFER | Specialized ownership/type-state attributes; CodeQL argument semantics are not pinned down. |
| `PreferredNameAttr` / `PreferredTypeAttr` | `TypeSourceInfo *...Loc()` | DEFER | Semantics are not clearly CodeQL `getValueType()` arguments for the P7 safe subset. |
| Broad `TypeAttr` sweep | generated type attributes | UNSAFE | Would mix unrelated attribute semantics and likely confuse owner/type spelling with type-valued arguments. |

### Blocker

Checkpoint 3 is blocked because there is no current TypeProcessor API that:

- accepts a TypeSourceInfo-backed attribute argument as such;
- skips all dependent, type-dependent, instantiation-dependent, and unresolved
  type payloads;
- returns a stable positive `@type` id without any `getAsString()` fallback;
- documents whether the id preserves source spelling or canonical identity;
- documents whether `@usertype` ids satisfy the dbscheme `@type ref` contract
  for attribute type payloads;
- has a positive fixture proving the chosen policy and a negative fixture
  proving dependent types remain skipped.

### Decision

Do not implement `attribute_arg_type` in this checkpoint.

`attribute_arg_type` remains deferred. P7 remains PARTIAL.

### Validation

Commands run before this checkpoint report:

```bash
git status --short
git diff --check
git diff --stat
git diff --name-only
```

Results:

- `git diff --check`: PASS.
- No behavior, schema, model, table definition, or fixture changes were made in
  this checkpoint.
- No SQLite evidence was generated for Checkpoint 3 because no
  `attribute_arg_type` implementation was attempted.

### Self-Review

- Did this checkpoint use `getAsString()` as an attribute type payload? No.
- Did this checkpoint invent type ids? No.
- Did this checkpoint flatten type args into `attribute_arg_value`? No.
- Did this checkpoint process dependent types? No.
- Did this checkpoint implement `attribute_arg_name`? No.
- Did this checkpoint implement generalized APValue or `CONSTANT_EXPR`? No.
- Did this checkpoint mark P7 DONE or modify `docs/datatable-list.txt`? No.
- Did this checkpoint expand `ASTVisitor`? No.

Verdict: BLOCKED, safely deferred.

### Continue Decision

It is safe to continue only to checkpoints that are independent of
`attribute_arg_type`. Checkpoint 4 (`attribute_arg_name`) is independent at the
schema table level, but it should start as a separate design gate and must not
reuse the blocked type-payload assumptions.

## Checkpoint 4: `attribute_arg_name` Microsoft Named Args

Status: BLOCKED. No `attribute_arg_name` candidate is SAFE NOW.

### Scope Audited

Repository context audited:

- `.trellis/tasks/p7-checkpoint-chain/goal.md`
- `docs/analysis/p7_full_done_criteria.md`
- `docs/analysis/p7_remaining_attribute_scope_audit.md`
- `docs/analysis/p7f_attribute_expr_design.md`
- `docs/roadmap.md`
- `include/model/db/attribute.h`
- `include/db/table_defs/attribute.h`
- `src/core/processor/attribute_processor.cc`
- `include/core/ast_visitor.h`
- `src/core/ast_visitor.cc`

Local Clang 19 generated APIs were inspected through:

- `/opt/homebrew/opt/llvm@19/include/clang/AST/Attrs.inc`
- `/opt/homebrew/opt/llvm@19/include/clang/Basic/AttrList.inc`
- `/opt/homebrew/opt/llvm@19/include/clang/AST/DeclCXX.h`

### Name Semantics Target

The P7 docs require `attribute_arg_name` to represent true CodeQL
`AttributeArgument.getName()` semantics. The checkpoint specifically treats
Microsoft named arguments as the primary target and forbids:

- modeling string payloads as names;
- modeling declaration references as names;
- guessing based on source spelling;
- using identifier-like tokens such as `ModeAttr`, `FormatAttr`,
  `AvailabilityAttr`, or `CleanupAttr` without CodeQL parity proof.

### Local Microsoft Attribute Inventory

The local Clang 19 generated Attr inventory includes Microsoft or declspec
attribute classes such as:

- `MSABIAttr`
- `MSAllocatorAttr`
- `MSConstexprAttr`
- `MSInheritanceAttr`
- `MSNoVTableAttr`
- `MSStructAttr`
- `MSVtorDispAttr`
- `DLLExportAttr`
- `DLLImportAttr`
- `ThreadAttr`
- `UuidAttr`
- `NakedAttr`
- `NoAliasAttr`
- `RestrictAttr`
- `SelectAnyAttr`
- calling-convention attrs such as `StdCallAttr`, `FastCallAttr`,
  `ThisCallAttr`, and `VectorCallAttr`

These classes are not true named-argument payload carriers for P7:

- many are marker attrs with no payload;
- some carry enum/scalar payloads;
- `UuidAttr` carries a GUID string and optional `MSGuidDecl`, not a named
  argument;
- none of the audited generated `Attr` classes expose a stable
  source-level name/value argument pair suitable for `attribute_arg_name`.

### `__declspec(property(get=..., put=...))` Probe

The closest local syntax to Microsoft named arguments is:

```cpp
struct P7NameProbe {
  int getX();
  void putX(int);
  __declspec(property(get=getX, put=putX)) int x;
};
```

The local Clang 19 AST accepts this under `-fms-extensions`, but it creates an
`MSPropertyDecl`, not an `Attr` subclass:

```text
MSPropertyDecl ... x
```

`DeclCXX.h` exposes:

```cpp
IdentifierInfo *GetterId, *SetterId;
IdentifierInfo *getGetterId() const;
IdentifierInfo *getSetterId() const;
```

This is not currently owned by P7 `AttributeProcessor`:

- `AttributeProcessor` records source attributes by iterating `decl->attrs()`
  and `AttributedStmt::getAttrs()`.
- There is no current `VisitMSPropertyDecl` or processor path that maps an
  `MSPropertyDecl` to `attributes` / `attribute_args`.
- Implementing this now would require new declaration ownership and a decision
  about whether `__declspec(property)` should be represented as a normal
  attribute row, a property declaration fact, or both.

That is outside the safe `attribute_arg_name` checkpoint.

### Candidate Verdicts

| Candidate | API / syntax | Verdict | Blocker |
| --- | --- | --- | --- |
| `__declspec(property(get=..., put=...))` | `MSPropertyDecl::getGetterId()` / `getSetterId()` | DEFER | Local toolchain parses it, but it is a declaration kind, not a Clang `Attr`; current P7 AttributeProcessor ownership cannot safely create AttributeArgument rows for it. |
| `UuidAttr` | `getGuid()`, `getGuidDecl()` | DEFER | GUID is string/decl payload, not a named argument. |
| `MSInheritanceAttr` / `MSVtorDispAttr` | enum/scalar payloads | DEFER | These are option/scalar semantics, not named args. |
| declspec marker attrs | no payload | UNSAFE | No argument exists to name. |
| `ModeAttr`, `FormatAttr`, `AvailabilityAttr` | `IdentifierInfo *` fields | UNSAFE for this checkpoint | Identifier-like fields are format/platform/mode semantics, not proven Microsoft named arguments. |
| `CleanupAttr` | `FunctionDecl *getFunctionDecl()` | UNSAFE for this checkpoint | Declaration reference, not a named argument. |

### Blocker

Checkpoint 4 is blocked because there is no current candidate that satisfies
all required conditions:

- true Microsoft named-argument semantics;
- local Clang syntax accepted by the validation toolchain;
- represented as a Clang `Attr` reachable through current `AttributeProcessor`
  ownership;
- stable argument indexes and names;
- no misclassification of strings, declaration references, expression payloads,
  type payloads, or identifier-like option tokens.

The only promising syntax, `__declspec(property(get=..., put=...))`, is an
`MSPropertyDecl` path and needs a separate design decision before P7 can safely
model it.

### Decision

Do not implement `attribute_arg_name` in this checkpoint.

`attribute_arg_name` remains deferred. P7 remains PARTIAL.

### Validation

Commands run during this checkpoint:

```bash
git status --short
git diff --check
git diff --stat
git diff --name-only
rg -n "attribute_arg_name|AttributeArgument|getName|Microsoft|named argument|named args|MS" docs .trellis include src tests
rg -n "Microsoft_|Declspec_|MS[A-Za-z]|MSProperty|Uuid|Property|Inheritance|NoVTable|__declspec|property\\(" /opt/homebrew/opt/llvm@19/include/clang/AST/Attrs.inc /opt/homebrew/opt/llvm@19/include/clang/Basic/AttrList.inc
/opt/homebrew/opt/llvm@19/bin/clang++ -x c++ -std=c++17 -fms-extensions -Xclang -ast-dump -fsyntax-only -
rg -n "MSProperty|VisitMSProperty|MSPropertyDecl|process.*Property|property" include src tests docs
```

Results:

- `git diff --check`: PASS after this report update.
- Schema/datatable guard diff is empty for `docs/datatable-list.txt`,
  `include/model/db`, `include/db/table_defs`, `include/db/table_init.h`,
  `src/db`, and `include/db/storage_facade.h`.
- Local Clang parses `__declspec(property(get=..., put=...))` and emits
  `MSPropertyDecl`.
- Repository search found no current `MSPropertyDecl` processing path in
  Arborchive.
- No behavior, schema, model, table definition, or fixture changes were made in
  this checkpoint.
- No SQLite evidence was generated for Checkpoint 4 because no
  `attribute_arg_name` implementation was attempted.

### Self-Review

- Did this checkpoint guess that an identifier-like token is a name? No.
- Did this checkpoint model string payloads as names? No.
- Did this checkpoint model declaration references as names? No.
- Did this checkpoint implement `attribute_arg_type`? No.
- Did this checkpoint implement `attribute_arg_name`? No.
- Did this checkpoint modify `ASTVisitor`? No.
- Did this checkpoint mark P7 DONE or modify `docs/datatable-list.txt`? No.

Verdict: BLOCKED, safely deferred.

### Continue Decision

It is safe to continue to Checkpoint 5 because constant/value family expansion
is independent of `attribute_arg_name`. Checkpoint 5 must remain limited to
stable value/constant payloads and must not infer names from identifiers.

## Checkpoint 5: Constant/Value Family Expansion

Status: PARTIAL PASS. One additional value-payload candidate is SAFE NOW and
implemented; scalar constant families remain deferred.

### Scope Audited

Repository context audited:

- `.trellis/tasks/p7-checkpoint-chain/goal.md`
- `docs/analysis/p7_full_done_criteria.md`
- `docs/analysis/p7_remaining_attribute_scope_audit.md`
- `docs/analysis/p7f_attribute_expr_design.md`
- `docs/roadmap.md`
- `src/core/processor/attribute_processor.cc`
- `scripts/test_all.sh`
- `tests/unit-tests/p7/`

Local Clang 19 generated APIs were inspected through
`/opt/homebrew/opt/llvm@19/include/clang/AST/Attrs.inc`.

### Candidate Verdicts

| Candidate | API / syntax | Verdict | Reason |
| --- | --- | --- | --- |
| `WarnUnusedResultAttr` / `[[nodiscard("...")]]` | `getMessage()` / `getMessageLength()` | SAFE NOW, implemented | Stable string payload, deterministic index `0`, local C++20 fixture support, and empty `[[nodiscard]]` provides a no-argument negative case. |
| `AliasAttr` | `getAliasee()` | DEFER | Getter is stable, but local Darwin validation rejects `alias` attributes. |
| `WeakRefAttr` | `getAliasee()` | DEFER | Same target/compiler validation risk as alias-like payloads; not in this checkpoint's safe set. |
| `AvailabilityAttr` | `getMessage()` / `getReplacement()` | DEFER | String getters are stable, but argument indexes are entangled with platform, version, unavailable/strict flags, environment, and replacement semantics. |
| `EnableIfAttr` / `DiagnoseIfAttr` message | `getMessage()` | DEFER | Mixed expression/string/enum payload families; expression arguments remain deferred, so string-only modeling would hide incomplete semantics. |
| `NonNullAttr` | `ParamIdx` list | DEFER | Scalar indexes are not source `@expr` ids; `attribute_arg_constant.constant` is `@expr ref`. |
| `FormatAttr` / `FormatArgAttr` | `IdentifierInfo *`, `int`, `ParamIdx` | DEFER | Mixes identifier-like format kind and scalar indexes; target table and `@expr` policy remain unclear. |
| `AllocSizeAttr` / `AllocAlignAttr` | `ParamIdx` fields | DEFER | Scalar indexes are not source `@expr` ids. |

### Implemented Scope

Implemented only the `WarnUnusedResultAttr` non-empty message value subset:

- `[[nodiscard("p7f_use_result")]]` records one `attribute_args` row with
  `AttributeArgKind::TOKEN`.
- The corresponding `attribute_arg_value` row stores `p7f_use_result`.
- Empty `[[nodiscard]]` records the attribute presence row but no argument row.
- No constant, expr, type, or name payload rows are created by this fixture.

No generalized APValue, `CONSTANT_EXPR`, scalar-index, type, name, declaration
reference, or expression payload support was added.

### SQLite Evidence

Focused fixture:

```text
tests/unit-tests/p7/attribute_extra_value_args_case.cc
```

Focused database:

```text
tests/output/unit-tests-p7-attribute_extra_value_args_case.db
```

Value row evidence:

```text
nodiscard|1|0|p7f_use_result
```

Typed payload counts:

```text
attribute_arg_value|1
attribute_arg_constant|0
attribute_arg_expr|0
attribute_arg_type|0
attribute_arg_name|0
```

Empty-message negative evidence:

```text
nodiscard|1
nodiscard|0
```

The first `nodiscard` row has one argument, and the empty `[[nodiscard]]` row
has zero argument rows.

### Validation

Commands run during this checkpoint:

```bash
git diff --check
git diff -- docs/datatable-list.txt include/model/db include/db/table_defs include/db/table_init.h src/db include/db/storage_facade.h
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
./build/demo -c config.example.toml \
  -s tests/unit-tests/p7/attribute_extra_value_args_case.cc \
  -o tests/output/unit-tests-p7-attribute_extra_value_args_case.db
python3 scripts/db_summary.py \
  tests/output/unit-tests-p7-attribute_extra_value_args_case.db
sqlite3 tests/output/unit-tests-p7-attribute_extra_value_args_case.db \
  'select a.name, aa.kind, aa."index", av.value from attribute_arg_value av join attribute_args aa on aa.id = av.arg join attributes a on a.id = aa.attribute order by a.id, aa."index";'
sqlite3 tests/output/unit-tests-p7-attribute_extra_value_args_case.db \
  'select "attribute_arg_value", count(*) from attribute_arg_value union all select "attribute_arg_constant", count(*) from attribute_arg_constant union all select "attribute_arg_expr", count(*) from attribute_arg_expr union all select "attribute_arg_type", count(*) from attribute_arg_type union all select "attribute_arg_name", count(*) from attribute_arg_name;'
sqlite3 tests/output/unit-tests-p7-attribute_extra_value_args_case.db \
  'select a.name, count(aa.id) from attributes a left join attribute_args aa on aa.attribute = a.id group by a.id, a.name order by a.id;'
./scripts/test_all.sh
```

Results:

- `git diff --check`: PASS.
- Schema/datatable guard diff is empty for `docs/datatable-list.txt`,
  `include/model/db`, `include/db/table_defs`, `include/db/table_init.h`,
  `src/db`, and `include/db/storage_facade.h`.
- Debug build: PASS with existing LLVM/header warnings.
- Focused DB generation: PASS.
- Focused SQLite evidence: PASS, as listed above.
- `./scripts/test_all.sh`: PASS.

### Self-Review

- Are all value/constant handlers explicit whitelist handlers? Yes.
- Did this checkpoint evaluate arbitrary expressions? No.
- Did this checkpoint insert raw scalar indexes into `attribute_arg_constant`?
  No.
- Did this checkpoint model strings, declaration references, expressions,
  types, or identifier-like tokens as names? No.
- Did this checkpoint implement `attribute_arg_type` or `attribute_arg_name`?
  No.
- Did this checkpoint implement generalized APValue or `CONSTANT_EXPR`? No.
- Did this checkpoint modify `ASTVisitor`? No.
- Did this checkpoint mark P7 DONE or modify `docs/datatable-list.txt`? No.

Verdict: PARTIAL PASS. The `WarnUnusedResultAttr` value subset is safe to keep;
constant scalar-index families, alias-like strings, availability strings, and
mixed expression/string families remain deferred.

### Continue Decision

It is safe to continue to Checkpoint 6. The next checkpoint is independent of
the newly added value payload and should focus only on `stmtattributes`
wrapper-vs-inner parity.

## Checkpoint 6: `stmtattributes` Wrapper-vs-Inner Parity Review

Status: BLOCKED for full DONE. Current inner-stmt ownership remains a
CONSERVATIVE PARTIAL safe subset.

### Scope Audited

Repository context audited:

- `.trellis/tasks/p7-checkpoint-chain/goal.md`
- `docs/analysis/p7_full_done_criteria.md`
- `docs/analysis/p7_remaining_attribute_scope_audit.md`
- `docs/analysis/p7_attribute_owner_links.md`
- `docs/roadmap.md`
- `docs/semmlecode.cpp.dbscheme`
- `include/model/db/stmt.h`
- `include/db/table_defs/stmt.h`
- `include/db/table_defs/attribute.h`
- `include/core/processor/stmt_processor.h`
- `src/core/processor/stmt_processor.cc`
- `include/core/ast_visitor.h`
- `src/core/ast_visitor.cc`
- `tests/unit-tests/p7/attribute_owner_links_case.cc`

Local Clang 19 AST behavior was probed with `-Xclang -ast-dump` for attributed
`return`, `fallthrough`, and `break` statements.

### Current Behavior

`ASTVisitor::VisitAttributedStmt` currently delegates to `StmtProcessor` for an
owner id and then delegates attribute insertion to `AttributeProcessor`:

```cpp
int stmt_id =
    stmt_processor_->getSupportedAttributedStmtOwnerId(attributedStmt);
attribute_processor_->processStatementAttributes(stmt_id, attributedStmt);
```

`StmtProcessor::getSupportedAttributedStmtOwnerId` unwraps the `AttributedStmt`
and records a supported inner statement id for:

- `ReturnStmt`
- `NullStmt`
- `CompoundStmt`
- `IfStmt`
- `ForStmt`
- `CXXForRangeStmt`
- `WhileStmt`
- `DoStmt`
- `SwitchStmt`
- `DeclStmt`

Unsupported inner statements return `-1`, causing
`AttributeProcessor::processStatementAttributes` to skip insertion.

### Schema Audit

Local `stmts` schema:

```text
stmts(
    unique int id: @stmt,
    int kind: int ref,
    int location: @location_stmt ref
)
```

Local `stmtattributes` schema:

```text
stmtattributes(
    int stmt_id: @stmt ref,
    int spec_id: @attribute ref
)
```

Neither `docs/semmlecode.cpp.dbscheme` nor `include/model/db/stmt.h` defines a
distinct `@stmt_attributed` / attributed-wrapper statement kind. The current
`StmtKind` enum has concrete statement kinds such as `RETURN`, `BLOCK`,
`DECL`, and `EMPTY`, but no wrapper kind for `clang::AttributedStmt`.

### Wrapper-vs-Inner Decision

Do not implement wrapper statement ids in this checkpoint.

Reasons:

- There is no local dbscheme/model statement kind for an attributed wrapper.
- Reusing the inner statement kind with wrapper locations would create
  ambiguous statement identities.
- Adding a new statement kind would be a schema/model/table-init change and
  would require CodeQL alignment proof, which this checkpoint does not have.
- Current inner-stmt ownership is already stable for the tested safe subset and
  avoids duplicate wrapper/inner owner rows.

The current inner-stmt strategy is safe to keep as a conservative subset, but
not enough to mark `stmtattributes` full DONE.

### SQLite Evidence

Focused fixture:

```text
tests/unit-tests/p7/attribute_owner_links_case.cc
```

Focused database:

```text
tests/output/unit-tests-p7-attribute_owner_links_case.db
```

Statement attribute owner evidence:

```text
fallthrough|361|26
likely|376|6
```

Here `26` is `StmtKind::EMPTY` / inner `NullStmt`, and `6` is
`StmtKind::RETURN` / inner `ReturnStmt`.

Duplicate owner row query:

```sql
select sa.stmt_id, sa.spec_id, count(*)
from stmtattributes sa
group by sa.stmt_id, sa.spec_id
having count(*) > 1;
```

Result: no rows.

P7 table count smoke evidence:

```text
stmtattributes|2
attributes|9
attribute_arg_type|0
attribute_arg_name|0
```

### Unsupported Wrapper Probe

Local Clang 19 AST dump shows that unsupported cases can occur:

```cpp
while (x) { [[likely]] break; }
```

AST evidence:

```text
AttributedStmt
|-LikelyAttr
`-BreakStmt
```

`BreakStmt` is not currently in
`StmtProcessor::getSupportedAttributedStmtOwnerId`, so this remains skipped.
That is safer than inventing an unsupported owner id, but it is another reason
`stmtattributes` cannot be full DONE yet.

### Validation

Commands run during this checkpoint:

```bash
git status --short
git diff --check
git diff --stat
git diff --name-only
./build/demo -c config.example.toml \
  -s tests/unit-tests/p7/attribute_owner_links_case.cc \
  -o tests/output/unit-tests-p7-attribute_owner_links_case.db
sqlite3 tests/output/unit-tests-p7-attribute_owner_links_case.db \
  'select a.name, sa.stmt_id, s.kind from stmtattributes sa join attributes a on a.id = sa.spec_id join stmts s on s.id = sa.stmt_id order by a.name;'
sqlite3 tests/output/unit-tests-p7-attribute_owner_links_case.db \
  'select sa.stmt_id, sa.spec_id, count(*) from stmtattributes sa group by sa.stmt_id, sa.spec_id having count(*) > 1;'
sqlite3 tests/output/unit-tests-p7-attribute_owner_links_case.db \
  'select "stmtattributes", count(*) from stmtattributes union all select "attributes", count(*) from attributes union all select "attribute_arg_type", count(*) from attribute_arg_type union all select "attribute_arg_name", count(*) from attribute_arg_name;'
/opt/homebrew/opt/llvm@19/bin/clang++ -x c++ -std=c++20 \
  -Xclang -ast-dump -fsyntax-only -
```

Results:

- `git diff --check`: PASS after this report update.
- Schema/datatable guard diff is empty for `docs/datatable-list.txt`,
  `include/model/db`, `include/db/table_defs`, `include/db/table_init.h`,
  `src/db`, and `include/db/storage_facade.h`.
- Focused DB generation: PASS.
- Focused SQLite evidence: PASS, as listed above.
- Clang AST probe: PASS; unsupported attributed `BreakStmt` shape confirmed.
- No behavior, schema, model, table definition, or fixture changes were made in
  this checkpoint.

### Self-Review

- Is wrapper-vs-inner semantics proven enough for full DONE? No.
- Are unsupported wrappers skipped safely? Yes, by returning `-1`.
- Did this checkpoint force wrapper ids? No.
- Did this checkpoint add a schema/model/table-init change? No.
- Did this checkpoint modify `ASTVisitor` or `StmtProcessor`? No.
- Did this checkpoint mark P7 DONE or modify `docs/datatable-list.txt`? No.

Verdict: BLOCKED for full DONE, safe to keep as CONSERVATIVE PARTIAL.

### Continue Decision

It is safe to continue to Checkpoint 7. Final documentation must keep P7
PARTIAL unless all remaining full-DONE criteria are proven, which they are not.

## Checkpoint 7: Final Documentation And P7 Parity Decision

Status: PASS for documentation closeout. P7 remains PARTIAL.

### Scope Audited

Repository context audited:

- `.trellis/tasks/p7-checkpoint-chain/checkpoints.md`
- `docs/analysis/p7_full_done_criteria.md`
- `docs/analysis/p7_remaining_attribute_scope_audit.md`
- `docs/analysis/p7_attribute_owner_links.md`
- `docs/roadmap.md`
- `docs/datatable-list.txt` guard diff
- schema/model/table-init guard paths:
  - `include/model/db`
  - `include/db/table_defs`
  - `include/db/table_init.h`
  - `src/db`
  - `include/db/storage_facade.h`

### Final Parity Decision

Do not mark P7 full DONE.

The current implementation is a reviewable safe subset:

- explicit supported attribute presence;
- function, type, variable, and conservative statement owner links;
- whitelisted stable string payloads;
- direct and shallow wrapped integer-literal `AlignedAttr` constants;
- minimal non-literal `AlignedAttr` and `AssumeAlignedAttr` expression payloads.

Full DONE is still blocked by:

- no safe `attribute_arg_type` implementation;
- no safe `attribute_arg_name` implementation;
- broader expression-backed attributes still deferred;
- generalized constants, APValue, `CONSTANT_EXPR`, and scalar-index families
  still deferred;
- statement wrapper-vs-inner CodeQL parity not proven for full
  `stmtattributes`;
- broader owner-link coverage and negative/deferred fixture coverage gaps.

Therefore `docs/datatable-list.txt` must remain unchanged.

### Documentation Updates

Updated `docs/analysis/p7_attribute_owner_links.md` so the original owner-link
deferred P7f note is clearly historical and points readers to the current
remaining-scope audit for the narrow `attribute_arg_expr` safe subset.

Updated `docs/analysis/p7_remaining_attribute_scope_audit.md` final
recommendation so the checkpoint-chain closeout says:

- P7 remains PARTIAL;
- `docs/datatable-list.txt` remains unchanged;
- future implementation should use a fresh narrow design gate for one deferred
  family at a time.

No behavior, fixture, schema, model, table definition, table initialization, or
storage code was changed in this checkpoint.

### SQLite Evidence

No new DB generation was required for this docs-only checkpoint.

Checkpoint 6 focused SQLite evidence remains the latest statement parity
evidence:

```text
fallthrough|361|26
likely|376|6
stmtattributes|2
attributes|9
attribute_arg_type|0
attribute_arg_name|0
```

The duplicate `stmtattributes` query returned no rows:

```sql
select sa.stmt_id, sa.spec_id, count(*)
from stmtattributes sa
group by sa.stmt_id, sa.spec_id
having count(*) > 1;
```

### Validation

Commands run for this checkpoint:

```bash
git diff --check
git diff -- docs/datatable-list.txt include/model/db include/db/table_defs include/db/table_init.h src/db include/db/storage_facade.h
rg -n "attribute_arg_expr is not populated|P7f remains deferred|attribute_arg_type|attribute_arg_name|DONE|PARTIAL|datatable-list|Checkpoint 7|Final" docs/analysis/p7_attribute_owner_links.md docs/analysis/p7_remaining_attribute_scope_audit.md docs/analysis/p7_full_done_criteria.md docs/roadmap.md .trellis/tasks/p7-checkpoint-chain/checkpoints.md
```

Results:

- `git diff --check`: PASS after the Checkpoint 7 docs patch.
- Schema/datatable guard diff is empty for `docs/datatable-list.txt`,
  `include/model/db`, `include/db/table_defs`, `include/db/table_init.h`,
  `src/db`, and `include/db/storage_facade.h`.
- Keyword audit found the expected PARTIAL/deferred/final-decision language and
  the owner-link historical wording that this checkpoint corrected.
- Behavior tests were not rerun for this docs-only checkpoint because no
  runtime behavior, fixture, build, schema, or table initialization files were
  changed in Checkpoint 7.

### Self-Review

- Did this checkpoint mark P7 DONE? No.
- Did this checkpoint modify `docs/datatable-list.txt`? No.
- Did this checkpoint modify schema/model/table initialization? No.
- Did this checkpoint implement `attribute_arg_type` or `attribute_arg_name`?
  No.
- Did this checkpoint add behavior to `ASTVisitor` or processors? No.
- Are remaining blockers documented? Yes.

Verdict: PASS for Checkpoint 7 documentation closeout. The P7 checkpoint chain
has a conservative final decision: keep P7 PARTIAL and continue future work only
through independent narrow design gates.
