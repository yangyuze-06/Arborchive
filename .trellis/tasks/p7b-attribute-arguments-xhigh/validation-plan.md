# P7b Validation Plan

## Required Commands

Run the full schema/behavior validation sequence:

```bash
git diff --check
python3 scripts/generate_instantiations.py
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
scripts/test_all.sh
```

## Database Evidence Target

Use the P7b fixture database:

```text
tests/output/unit-tests-p7-attribute_arguments_case.db
```

Run:

```bash
python3 scripts/db_summary.py tests/output/unit-tests-p7-attribute_arguments_case.db
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db ".tables"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select * from attributes order by id;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select * from funcattributes order by func_id, spec_id;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select * from attribute_args order by id;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select * from attribute_arg_value order by arg;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select * from attribute_arg_constant order by arg;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select aa.id, a.name, aa.kind, aa.\"index\" from attribute_args aa join attributes a on aa.attribute = a.id order by aa.id;"
sqlite3 tests/output/unit-tests-p7-attribute_arguments_case.db "select v.str from attribute_arg_constant aac join valuebind vb on vb.expr = aac.constant join \"values\" v on v.id = vb.val order by aac.arg;"
```

## Evidence Requirements

The final summary must prove:

- `attribute_args` rows exist;
- each argument row links to an `attributes` owner;
- string and integer arguments have distinct kinds;
- deprecated string text is present in `attribute_arg_value`;
- aligned integer value is present through `attribute_arg_constant` ->
  `valuebind` -> `values`;
- P7a `attributes` and `funcattributes` rows still exist;
- P7b tables appear in `.tables`.

## Failure Reporting

If any required command fails, report:

- exact command;
- key failure output;
- whether implementation is blocked;
- residual risk.

Do not claim full completion if required build/test or SQLite evidence is
missing.
