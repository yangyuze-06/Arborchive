# P7 Lightweight Pilot Validation Plan

This file records validation protocols for future P7 work. The current pilot is
docs-only and should use only the docs-only checks.

## Docs-only task

Use when the patch changes only Markdown task context or documentation files
and does not touch code, schema, tests, scripts, build files, generated files,
or runtime state.

```bash
git diff --check
git status --short
git diff --name-only
```

`scripts/test_all.sh` is not required for a docs-only task when:

- no C++ behavior changed;
- no schema / ORM / storage files changed;
- no tests or fixtures changed;
- no scripts or build files changed;
- the diff is limited to allowed documentation paths.

The final report should explicitly state that behavior tests were not run
because the patch is docs-only.

## Processor behavior change

Use when future P7 work changes attribute extraction behavior in a processor or
helper.

```bash
scripts/test_all.sh
```

Also run a relevant fixture that exercises the changed behavior. After the
fixture produces a database, inspect summary and representative rows:

```bash
python3 scripts/db_summary.py tests/output/<case>.db
sqlite3 tests/output/<case>.db "select * from <table> limit 10;"
```

The implementation summary should name the fixture, the expected tables, and
the row evidence used for review.

## Schema / ORM change

Use when future P7 work changes DB models, table definitions, table
initialization, storage facade paths, generated ORM instantiations, or schema
semantics.

```bash
python3 scripts/generate_instantiations.py
make debug -j 8
scripts/test_all.sh
sqlite3 tests/output/<case>.db ".tables"
sqlite3 tests/output/<case>.db "select * from <table> limit 10;"
```

Schema / ORM validation must also report:

- changed model, table definition, initialization, and storage files;
- generated instantiation status;
- CodeQL counterpart or Arbor extension reason;
- table existence;
- representative row evidence;
- migration impact and affected fixtures.

## ASTVisitor delegation change

Use when future P7 work adds or changes `Visit*` delegation.

Review checks:

- Check that each touched `Visit*` remains thin.
- Check that traversal code performs only lightweight guards and delegation.
- Check that semantic logic is delegated to a processor or helper.
- Check that visitor code does not assemble schema rows.
- Check that visitor code does not directly coordinate storage, cache, key, or
  dependency policy.

Validation commands:

```bash
scripts/test_all.sh
python3 scripts/db_summary.py tests/output/<case>.db
sqlite3 tests/output/<case>.db "select * from <table> limit 10;"
```

The DB checks are required when delegation changes semantic output. If a
delegation-only compatibility patch does not change output, the summary should
explain why DB row inspection was not required and what residual risk remains.

## CodeQL alignment review

Use when future P7 work touches attribute-related schema semantics, table
mapping, or any CodeQL-aligned behavior.

Every schema or semantic mapping review must include:

- CodeQL counterpart;
- Arbor-specific reason;
- table category: CodeQL-aligned, Arbor extension, or experimental-deferred;
- validation query;
- alignment risk;
- migration impact;
- affected tests or fixtures.

Reviewers should be able to answer:

- Is the table approved by `docs/datatable-list.txt` or explicitly documented
  as an Arbor extension?
- Does the table or field meaning align with
  `docs/semmlecode.cpp.dbscheme` when intended to be CodeQL-compatible?
- Does any Arbor extension have a clear naming or documentation boundary?
- Does SQLite output prove the intended facts were extracted?

## Failure reporting

If a validation command fails, report:

- the exact failed command;
- the failure reason or most relevant error output;
- whether the failure blocks completion;
- the residual risk.

If a command cannot run, report:

- the exact command not run;
- why it could not run;
- what coverage is missing;
- what risk remains.

Do not claim the implementation task is complete when required validation fails
or cannot run.
