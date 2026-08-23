#!/usr/bin/env python3
"""Executable SQLite assertions for Arborchive regression fixtures."""

from __future__ import annotations

import argparse
import sqlite3
import sys
from pathlib import Path


class Audit:
    def __init__(self, case: str, database: Path) -> None:
        self.case = case
        self.database = database
        self.connection = sqlite3.connect(database)
        self.errors = 0
        self.warnings = 0

    def close(self) -> None:
        self.connection.close()

    def scalar(self, sql: str):
        row = self.connection.execute(sql).fetchone()
        return None if row is None else row[0]

    def check(self, label: str, sql: str, expected, *, warning: bool = False) -> None:
        try:
            actual = self.scalar(sql)
        except sqlite3.Error as error:
            self._report(label, f"SQL error: {error}", warning)
            return
        if actual != expected:
            self._report(label, f"expected={expected!r}, actual={actual!r}", warning)

    def check_columns(
        self, table: str, expected: list[str], *, warning: bool = False
    ) -> None:
        try:
            actual = [
                row[1]
                for row in self.connection.execute(f'pragma table_info("{table}")')
            ]
        except sqlite3.Error as error:
            self._report(f"{table} columns", f"SQL error: {error}", warning)
            return
        if actual != expected:
            self._report(
                f"{table} columns",
                f"expected={expected!r}, actual={actual!r}",
                warning,
            )

    def warn_if_positive(self, label: str, sql: str) -> None:
        try:
            actual = self.scalar(sql)
        except sqlite3.Error as error:
            self._report(label, f"SQL error: {error}", True)
            return
        if actual and actual > 0:
            self._report(label, f"actual={actual}", True)

    def _report(self, label: str, detail: str, warning: bool) -> None:
        if warning:
            self.warnings += 1
            level = "WARN"
        else:
            self.errors += 1
            level = "ERROR"
        print(f"[{level}] {self.case}: {label}: {detail}", file=sys.stderr)

    def finish(self) -> int:
        if self.errors:
            print(
                f"[assert_db] {self.case}: FAIL "
                f"({self.errors} errors, {self.warnings} warnings)",
                file=sys.stderr,
            )
            return 1
        print(f"[assert_db] {self.case}: PASS ({self.warnings} warnings)")
        return 0


def check_count(audit: Audit, table: str, expected: int) -> None:
    audit.check(f"{table} count", f'select count(*) from "{table}"', expected)


def check_common(audit: Audit) -> None:
    audit.check("database integrity", "pragma integrity_check", "ok")
    audit.check("compilation row", "select count(*) from compilations", 1)
    audit.check("finished compilation", "select count(*) from compilation_finished", 1)
    audit.check("source file rows", "select count(*) >= 1 from files", 1)


def check_namespace(audit: Audit) -> None:
    expected_counts = {
        "namespaces": 14,
        "namespace_decls": 18,
        "namespace_inline": 1,
        "usings": 10,
        "using_container": 8,
    }
    for table, expected in expected_counts.items():
        check_count(audit, table, expected)

    audit.check(
        "reopened namespace identity",
        """
        select count(*) from (
          select n.name
          from namespaces n
          join namespace_decls d on d.namespace_id=n.id
          where n.name in ('multi_block','p4_decl_reopened','p4_reopened','using_reopened')
          group by n.name
          having count(distinct n.id)=1 and count(d.id)=2
        )
        """,
        4,
    )
    audit.check(
        "using kind distribution",
        """
        select count(*) from (
          select kind, count(*) as n from usings group by kind
          having (kind=1 and n=7) or (kind=2 and n=3)
        )
        """,
        2,
    )
    audit.check(
        "using owner orphans",
        """
        select count(*)
        from using_container c
        left join usings u on u.id=c.child
        left join namespaces n on n.id=c.parent
        left join files f on f.id=c.parent
        where u.id is null or (n.id is null and f.id is null)
        """,
        0,
    )
    audit.warn_if_positive(
        "known P4 namespacembrs unresolved references",
        "select count(*) from namespacembrs where memberid < 0",
    )
    audit.check_columns("namespacembrs", ["parentid", "memberid"])
    audit.check(
        "CodeQL memberid unique constraint",
        """
        select count(*) from pragma_index_list('namespacembrs') il
        where il."unique"=1
          and (select count(*) from pragma_index_info(il.name))=1
          and exists (
            select 1 from pragma_index_info(il.name) ii where ii.name='memberid'
          )
        """,
        1,
        warning=True,
    )


def check_p5(audit: Audit) -> None:
    counts = {
        "unit-tests/p5/hierarchy_case": (6, 7, 5, 1, 8),
        "unit-tests/p5/layout_case": (3, 4, 2, 1, 9),
        "unit-tests/p5/semantic_gaps_case": (9, 11, 5, 3, 21),
    }
    derivations, specifiers, direct, virtual, fields = counts[audit.case]
    for table, expected in (
        ("derivations", derivations),
        ("derspecifiers", specifiers),
        ("direct_base_offsets", direct),
        ("virtual_base_offsets", virtual),
        ("fieldoffsets", fields),
    ):
        check_count(audit, table, expected)

    audit.check(
        "derivation reference orphans",
        """
        select count(*) from derivations d
        left join usertypes sub on sub.id=d.sub
        left join usertypes super on super.id=d.super
        left join locations_default l on l.id=d.location
        where sub.id is null or super.id is null or l.id is null
        """,
        0,
    )
    audit.check(
        "derivation specifier orphans",
        """
        select count(*) from derspecifiers ds
        left join derivations d on d.derivation=ds.der_id
        left join specifiers s on s.id=ds.spec_id
        where d.derivation is null or s.id is null
        """,
        0,
    )
    audit.check(
        "direct offset orphans",
        """
        select count(*) from direct_base_offsets o
        left join derivations d on d.derivation=o.der_id
        where d.derivation is null
        """,
        0,
    )
    audit.check(
        "virtual offset orphans",
        """
        select count(*) from virtual_base_offsets o
        left join usertypes sub on sub.id=o.sub
        left join usertypes super on super.id=o.super
        where sub.id is null or super.id is null
        """,
        0,
    )
    audit.check(
        "field offset orphans",
        """
        select count(*) from fieldoffsets f
        left join membervariables m on m.id=f.id
        where m.id is null
        """,
        0,
    )
    audit.check_columns(
        "derivations",
        ["derivation", "sub", "index", "super", "location"],
        warning=True,
    )
    audit.warn_if_positive(
        "known P5 membervariable type placeholders",
        """
        select count(*) from fieldoffsets f
        join membervariables m on m.id=f.id
        where m.type_id < 0
        """,
    )


def check_p6(audit: Audit) -> None:
    check_count(audit, "lambdas", 14)
    check_count(audit, "lambda_capture", 14)
    audit.check(
        "lambda reference orphans",
        """
        select count(*) from lambdas l
        left join exprs e on e.id=l.expr
        where e.id is null
        """,
        0,
    )
    audit.check(
        "lambda capture reference orphans",
        """
        select count(*) from lambda_capture c
        left join lambdas l on l.expr=c.lambda
        left join membervariables m on m.id=c.field
        left join locations_default loc on loc.id=c.location
        where l.expr is null or m.id is null or loc.id is null
        """,
        0,
    )
    audit.check(
        "duplicate lambda capture indexes",
        """
        select count(*) from (
          select lambda, "index", count(*) n from lambda_capture
          group by lambda, "index" having n > 1
        )
        """,
        0,
    )
    audit.warn_if_positive(
        "known P6 capture field type placeholders",
        """
        select count(*) from lambda_capture c
        join membervariables m on m.id=c.field
        where m.type_id < 0
        """,
    )


P7_COUNTS: dict[str, dict[str, int]] = {
    "unit-tests/p7/attribute_presence_case": {
        "attributes": 1,
        "funcattributes": 1,
    },
    "unit-tests/p7/attribute_arguments_case": {
        "attributes": 2,
        "funcattributes": 2,
        "attribute_args": 2,
        "attribute_arg_value": 1,
        "attribute_arg_constant": 1,
    },
    "unit-tests/p7/attribute_owner_links_case": {
        "attributes": 9,
        "typeattributes": 3,
        "varattributes": 4,
        "stmtattributes": 2,
    },
    "unit-tests/p7/attribute_string_args_case": {
        "attributes": 3,
        "attribute_args": 4,
        "attribute_arg_value": 4,
    },
    "unit-tests/p7/attribute_constant_args_case": {
        "attributes": 3,
        "attribute_args": 3,
        "attribute_arg_constant": 3,
    },
    "unit-tests/p7/attribute_expr_args_case": {
        "attributes": 4,
        "attribute_args": 3,
        "attribute_arg_constant": 1,
        "attribute_arg_expr": 2,
    },
    "unit-tests/p7/attribute_assume_aligned_expr_args_case": {
        "attributes": 3,
        "attribute_args": 2,
        "attribute_arg_expr": 2,
    },
    "unit-tests/p7/attribute_extra_value_args_case": {
        "attributes": 2,
        "attribute_args": 1,
        "attribute_arg_value": 1,
    },
}


def check_p7(audit: Audit) -> None:
    for table, expected in P7_COUNTS[audit.case].items():
        check_count(audit, table, expected)
    check_count(audit, "attribute_arg_type", 0)
    check_count(audit, "attribute_arg_name", 0)
    audit.check(
        "attribute location orphans",
        """
        select count(*) from attributes a
        left join locations_default l on l.id=a.location
        where l.id is null or a.id < 0 or a.location < 0
        """,
        0,
    )
    audit.check(
        "function attribute owner orphans",
        """
        select count(*) from funcattributes o
        left join functions f on f.id=o.func_id
        left join attributes a on a.id=o.spec_id
        where f.id is null or a.id is null
        """,
        0,
    )
    audit.check(
        "type attribute owner orphans",
        """
        with type_ids(id) as (
          select id from types union select id from builtintypes
          union select id from derivedtypes union select id from usertypes
          union select id from routinetypes
        )
        select count(*) from typeattributes o
        left join type_ids t on t.id=o.type_id
        left join attributes a on a.id=o.spec_id
        where t.id is null or a.id is null
        """,
        0,
    )
    audit.check(
        "variable attribute owner orphans",
        """
        with variable_ids(id) as (
          select id from globalvariables union select id from localvariables
          union select id from membervariables union select id from params
        )
        select count(*) from varattributes o
        left join variable_ids v on v.id=o.var_id
        left join attributes a on a.id=o.spec_id
        where v.id is null or a.id is null
        """,
        0,
    )
    audit.check(
        "statement attribute owner orphans",
        """
        select count(*) from stmtattributes o
        left join stmts s on s.id=o.stmt_id
        left join attributes a on a.id=o.spec_id
        where s.id is null or a.id is null
        """,
        0,
    )
    audit.check(
        "attribute argument owner orphans",
        """
        select count(*) from attribute_args aa
        left join attributes a on a.id=aa.attribute
        where a.id is null or aa.id < 0 or aa.attribute < 0 or aa.location < 0
        """,
        0,
    )
    for table, column in (
        ("attribute_arg_value", "arg"),
        ("attribute_arg_constant", "arg"),
        ("attribute_arg_expr", "arg"),
        ("attribute_arg_type", "arg"),
        ("attribute_arg_name", "arg"),
    ):
        audit.check(
            f"{table} argument orphans",
            f"""
            select count(*) from {table} p
            left join attribute_args aa on aa.id=p.{column}
            where aa.id is null or p.{column} < 0
            """,
            0,
        )
    audit.check(
        "constant payload expression orphans",
        """
        select count(*) from attribute_arg_constant p
        left join exprs e on e.id=p.constant
        where e.id is null
        """,
        0,
    )
    audit.check(
        "expression payload orphans",
        """
        select count(*) from attribute_arg_expr p
        left join exprs e on e.id=p.expr
        where e.id is null
        """,
        0,
    )
    audit.check(
        "attribute payload overlap",
        """
        select count(*) from (
          select arg from (
            select arg from attribute_arg_value
            union all select arg from attribute_arg_constant
            union all select arg from attribute_arg_expr
            union all select arg from attribute_arg_type
            union all select arg from attribute_arg_name
          ) payloads
          group by arg having count(*) > 1
        ) overlaps
        """,
        0,
    )


def check_p8(audit: Audit) -> None:
    for table, expected in (
        ("initialisers", 8),
        ("braced_initialisers", 6),
        ("aggregate_field_init", 4),
        ("aggregate_array_init", 5),
    ):
        check_count(audit, table, expected)
    audit.check(
        "initialiser reference orphans",
        """
        select count(*) from initialisers i
        left join var_decls v on v.variable=i.var
        left join exprs e on e.id=i.expr
        left join locations_expr l on l.id=i.location
        where v.variable is null or e.id is null or l.id is null
           or i.var < 0 or i.expr < 0 or i.location < 0
        """,
        0,
    )
    audit.check(
        "braced initialiser orphans",
        """
        select count(*) from braced_initialisers b
        left join initialisers i on i.init=b.init
        where i.init is null
        """,
        0,
    )
    audit.check(
        "aggregate field reference orphans",
        """
        select count(*) from aggregate_field_init a
        left join exprs aggregate_expr on aggregate_expr.id=a.aggregate
        left join exprs initializer_expr on initializer_expr.id=a.initializer
        left join membervariables m on m.id=a.field
        where aggregate_expr.id is null or initializer_expr.id is null or m.id is null
        """,
        0,
    )
    audit.check(
        "CodeQL initialiser expr unique constraint",
        """
        select count(*) from pragma_index_list('initialisers') il
        join pragma_index_info(il.name) ii
        where il."unique"=1 and ii.name='expr'
        """,
        1,
        warning=True,
    )
    audit.check(
        "aggregate array reference orphans",
        """
        select count(*) from aggregate_array_init a
        left join exprs aggregate_expr on aggregate_expr.id=a.aggregate
        left join exprs initializer_expr on initializer_expr.id=a.initializer
        where aggregate_expr.id is null or initializer_expr.id is null
           or a.element_index <> a.position
        """,
        0,
    )


def run(case: str, database: Path) -> int:
    audit = Audit(case, database)
    try:
        check_common(audit)
        if case == "unit-tests/namespace":
            check_namespace(audit)
        elif case.startswith("unit-tests/p5/"):
            check_p5(audit)
        elif case == "unit-tests/p6/lambda_case":
            check_p6(audit)
        elif case.startswith("unit-tests/p7/"):
            check_p7(audit)
        elif case == "unit-tests/p8/initialization_case":
            check_p8(audit)
        else:
            audit.warn_if_positive(
                "known non-P8 initialiser expression placeholders",
                "select count(*) from initialisers where expr < 0",
            )
        return audit.finish()
    finally:
        audit.close()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("case")
    parser.add_argument("database", type=Path)
    args = parser.parse_args()
    if not args.database.is_file():
        parser.error(f"database does not exist: {args.database}")
    return run(args.case, args.database)


if __name__ == "__main__":
    raise SystemExit(main())
