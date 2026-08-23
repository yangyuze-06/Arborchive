# P7 Lightweight Pilot Summary

## What was created

Created a non-personal P7 pilot task context under
`.trellis/tasks/p7-lightweight-pilot/`:

- `prd.md`
- `research.md`
- `validation.md`
- `summary.md`

## Which governance/spec/skill files were read

- `AGENTS.md`
- `docs/agent_workflow.md`
- `.trellis/workflow.md`
- `.trellis/spec/README.md`
- `.trellis/spec/arborchive/architecture.md`
- `.trellis/spec/arborchive/ast-visitor-boundary.md`
- `.trellis/spec/arborchive/processor-boundary.md`
- `.trellis/spec/arborchive/schema-codeql-alignment.md`
- `.trellis/spec/arborchive/testing-and-verification.md`
- `.trellis/spec/arborchive/commit-and-pr-style.md`
- `.trellis/tasks/examples/README.md`
- `.agents/skills/arborchive-orient/SKILL.md`
- `.agents/skills/arborchive-plan/SKILL.md`
- `.agents/skills/arborchive-verify/SKILL.md`
- `.agents/skills/arborchive-schema-review/SKILL.md`
- `.agents/skills/arborchive-finish/SKILL.md`
- `docs/roadmap.md`

## Why no business code changed

This pilot is a docs-only workflow experiment. Its purpose is to validate that
an agent can read governance first, plan within explicit boundaries, research a
future P7 task conservatively, and prepare validation protocols without
implementing extraction behavior.

No C++ code, schema, ORM, tests, scripts, build files, or generated files were
needed for that goal.

## What this experiment validates

- Lightweight governance files can steer agent orientation before edits.
- The task can be decomposed into PRD, research, validation, and summary.
- Arborchive architecture boundaries can be captured in task context:
  - `ASTVisitor` stays traversal / dispatch oriented.
  - processors own semantic extraction.
  - schema work requires CodeQL counterpart or Arbor extension rationale.
  - SQLite output is reviewable evidence for semantic extraction.
- Docs-only tasks can remain constrained to task context files.
- Future P7 implementation work has a reusable validation starting point.

## What this experiment does not validate

- It does not validate official Trellis runtime behavior.
- It does not validate `.codex/hooks`.
- It does not validate `.trellis/scripts`.
- It does not validate template hashes or `.trellis/.template-hashes.json`.
- It does not validate personal task state, onboarding state, or session
  journals.
- It does not validate P7 implementation correctness.
- It does not validate generated SQLite output for attributes.

## Risks / limitations

- The roadmap identifies P7 as Attribute System, but a concrete implementation
  subtask still needs to be selected before coding.
- This pilot does not inspect current attribute-related processor or schema
  implementation details.
- Future P7 work may discover ownership or schema constraints that require a
  narrower implementation plan.
- Validation commands in `validation.md` are protocols for future
  implementation; only docs-only checks apply to this pilot.

## Recommended next step

Open a separate P7 implementation task when the target is clear. Reuse this
task's `validation.md`, then load the current AST dispatch flow, related
processor files, schema/model/table definitions, roadmap notes, and fixtures
before editing code.

## Suggested commit message

```text
docs: add P7 lightweight pilot task context
```
