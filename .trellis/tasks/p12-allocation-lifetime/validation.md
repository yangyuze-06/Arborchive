# P12 Validation Contract

- Schema columns exactly match the CodeQL counterparts.
- New/delete kind distribution includes plain and array forms.
- Every allocator/deallocator function reference is positive and resolves to
  `functions.id`.
- Allocator forms cover plain and aligned calls.
- Deallocator forms cover size, alignment, and destroying-delete bits.
- Plain new types use allocated types; array-new types are outer array types;
  nested fixed dimensions survive.
- Initializer edge 1, dynamic extent edge 2, and trivial delete edge 3 exist.
- Non-trivial/virtual delete operands do not enter the safe child-3 subset.
- Runtime-selected scalar virtual-destructor deletes do not receive a static
  `expr_deallocator` row; explicit global, array, and final virtual-type deletes
  retain their statically selected relation.
- Dependent new/delete patterns do not produce partial P12 expressions or array
  types with unresolved element IDs.
- No synthetic expression kind or deferred lifetime table is added.

Commands:

```bash
python3 scripts/generate_instantiations.py
make debug -j 8
scripts/test_all.sh
python3 scripts/db_summary.py tests/output/unit-tests-p12-allocation_lifetime_case.db
sqlite3 tests/output/unit-tests-p12-allocation_lifetime_case.db ".tables"
```
