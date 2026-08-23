# P7 Lightweight Pilot Research

## Roadmap signal

The current roadmap identifies P7 as the Attribute System phase:

- P7a: Attribute Presence Graph
  - Tables: `attributes`, `typeattributes`, `funcattributes`,
    `varattributes`, `stmtattributes`
  - Focus: GNU attributes, C++11 attributes, MS attributes
  - Complexity: Medium
- P7b: Attribute Argument System
  - Tables: `attribute_args`, `attribute_arg_value`, `attribute_arg_type`,
    `attribute_arg_constant`, `attribute_arg_expr`, `attribute_arg_name`
  - Focus: constant args, expression args, type args
  - Complexity: High
  - Risk: `Expr` / `APValue` serialization is complex; future work should use
    a safe subset and record deferred cases.

These roadmap details are enough to frame a bounded research and validation
task. They are not enough to implement P7 without a concrete subtask and
confirmed extraction owner.

## Do not infer unavailable roadmap details

If current files do not explicitly define a P7 subtask, do not invent one.

This pilot should stay limited to context, research, and validation preparation.
It should not decide the exact AST coverage, schema shape, fixtures, or
processor APIs for a future implementation.

Before real P7 implementation begins, the user or roadmap should clarify
whether the first implementation target is:

- P7a attribute presence graph;
- P7b attribute argument system;
- a smaller fixture-backed subset of one of those phases;
- a feasibility note rather than code.

## Candidate engineering boundaries

Potential P7 work may involve:

- processor or helper ownership for attribute extraction;
- schema/model/table/storage paths for attribute-related tables;
- documentation updates explaining safe subsets and deferred serialization
  cases;
- tests or fixtures for GNU attributes, C++11 attributes, MS attributes,
  declaration attributes, type attributes, statement attributes, and argument
  forms.

This pilot does not inspect or modify concrete processor, schema, or fixture
files. Future implementation must load the current AST dispatch flow, related
processor ownership, and related model/table definitions before editing.

## Architecture guardrails

- `ASTVisitor` must remain traversal-oriented.
- Processor or helper code owns semantic extraction.
- Schema work must preserve CodeQL alignment.
- SQLite output is the reviewable evidence.

For future P7 implementation, `ASTVisitor` should only identify relevant Clang
AST entrypoints and delegate to the attribute owner. It must not assemble
attribute rows, coordinate multiple attribute tables, own key/cache/dependency
policy, or make CodeQL schema decisions.

## Processor ownership

Attribute semantics should enter a processor/helper boundary dedicated to the
attribute subsystem or a clearly documented dominant owner if no dedicated
processor exists yet.

Processor-owned logic may include:

- deciding which Clang attribute objects are supported;
- normalizing attribute spelling, placement, and target entity information;
- mapping supported attribute presence facts to DB-facing models;
- limiting argument extraction to a safe subset;
- documenting unsupported or deferred argument forms;
- producing stable IDs or status only when callers need them.

Cross-domain writes should be documented. For example, if an attribute fact
links to a function, variable, type, or statement table, the future patch should
explain why the attribute subsystem is the correct owner and which validation
query proves the relation.

## Schema and CodeQL alignment

Future P7 schema work must start from existing approved table names and field
semantics. It should not add tables outside the documented schema list.

Each schema change must state:

- CodeQL counterpart;
- Arbor-specific reason;
- table category: CodeQL-aligned, Arbor extension, or experimental-deferred;
- validation query;
- alignment risk;
- migration impact;
- affected tests or fixtures.

The likely P7 presence tables look CodeQL-aligned by name and must preserve the
intended CodeQL semantics. Any Arbor-only reinforcement or additional table
must be explicitly documented as an Arbor extension, preferably with an
`arbor_*` naming boundary or equally clear rationale.

## Arbor extension requirements

An Arbor extension must explain:

- why there is no direct CodeQL counterpart;
- why Arborchive needs the fact anyway;
- how the extension avoids polluting CodeQL-aligned semantics;
- how reviewers can distinguish extension facts from CodeQL counterpart facts;
- which SQL query or DB summary proves the extension behaves as intended.

This pilot does not propose any Arbor extension.

## Alignment risks

Potential P7 alignment risks include:

- treating all Clang attribute spellings as one normalized concept without
  preserving CodeQL-compatible meaning;
- mixing declaration, type, statement, and argument semantics in one patch;
- adding convenience-only tables or fields;
- serializing expressions or APValues in a way that appears stable but is not;
- silently recording unsupported attributes as complete facts;
- expanding visitor orchestration because attributes appear on many AST node
  categories.

## Testing risks

Potential testing risks include:

- relying only on aggregate cases instead of isolated attribute fixtures;
- checking that tables exist without checking representative rows;
- failing to verify GNU, C++11, and MS attribute variants separately when they
  are in scope;
- omitting negative or deferred cases for unsupported argument forms;
- treating a successful build as proof of semantic extraction correctness.

## Suggested P7 decomposition

A conservative future implementation path should split P7 into small tasks:

1. Confirm P7a target tables, processor owner, and minimal fixture.
2. Implement attribute presence for one narrow declaration category.
3. Add SQL evidence for representative rows and owner links.
4. Expand presence coverage by semantic target, not by random table fill-in.
5. Start P7b only after presence extraction is stable.
6. For P7b, begin with a safe argument subset and explicitly defer unstable
   `Expr` / `APValue` serialization cases.

Each implementation task should have its own PRD or checklist, targeted
research, validation evidence, and summary.
