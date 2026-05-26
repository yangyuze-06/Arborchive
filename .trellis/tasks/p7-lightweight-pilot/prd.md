# P7 Lightweight Pilot PRD

## Task goal

Create a non-personal task context for the first lightweight Trellis-style
Arborchive workflow experiment around P7. This task validates whether the
`.trellis/` context layer and `.agents/skills/arborchive-*` skills can guide an
agent through bounded planning, research, validation design, and summary before
any implementation work begins.

This pilot does not implement P7 business code.

## Background

Arborchive now has a lightweight Trellis-style governance layer:

- `.trellis/workflow.md`
- `.trellis/spec/arborchive/*`
- `.trellis/tasks/examples/README.md`
- `.agents/skills/arborchive-*`

The repository roadmap identifies P7 as the Attribute System phase. The current
roadmap separates attribute presence extraction from attribute argument
extraction, but this pilot is only a task-context and research exercise.

## Scope

- Create `.trellis/tasks/p7-lightweight-pilot/` as a reusable, non-personal
  task context directory.
- Add `prd.md`, `research.md`, `validation.md`, and `summary.md`.
- Record the allowed boundaries for a future P7 implementation task.
- Record validation protocols that future P7 implementation work can reuse.
- Keep the work docs-only and reviewable.

## Non-goals

- Do not implement P7 extraction logic.
- Do not modify C++ business code.
- Do not modify schema, ORM, table initialization, storage, or build files.
- Do not modify tests or fixtures.
- Do not run Trellis runtime initialization.
- Do not create personal onboarding, session, journal, or runtime task state.
- Do not validate the official Trellis runtime.

## Allowed files

- `.trellis/tasks/p7-lightweight-pilot/prd.md`
- `.trellis/tasks/p7-lightweight-pilot/research.md`
- `.trellis/tasks/p7-lightweight-pilot/validation.md`
- `.trellis/tasks/p7-lightweight-pilot/summary.md`

## Forbidden files

- `src/`
- `include/`
- `tests/`
- `scripts/`
- `CMakeLists.txt`
- schema / ORM / table initialization files
- `.codex/`
- `.trellis/scripts/`
- `.trellis/.template-hashes.json`
- `.trellis/tasks/00-join-*`
- personal onboarding / session / journal files
- `.agents/skills/trellis-*`

## Required specs

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

## Expected output

The task directory should contain:

- a PRD that defines the experiment boundary;
- research notes that identify P7 architecture risks without inventing missing
  implementation details;
- a validation protocol for docs-only, processor behavior, schema / ORM,
  ASTVisitor delegation, and CodeQL alignment review tasks;
- a summary that explains what this pilot validates and what it does not
  validate.

## Success criteria

- The agent reads governance, spec, and skill files before writing task files.
- The task is decomposed into PRD, research, validation, and summary artifacts.
- The task recognizes Arborchive architecture boundaries:
  - `ASTVisitor` remains traversal / dispatch oriented.
  - semantic extraction belongs in processors or helpers.
  - schema changes require a CodeQL counterpart or explicit Arbor extension
    reason.
  - behavior, schema, and processor changes require validation evidence.
- The patch remains docs-only.
- No business code, schema, tests, scripts, build files, personal state, or
  Trellis runtime files are modified.
- Future P7 implementation work can reuse `validation.md` as a starting
  verification protocol.
