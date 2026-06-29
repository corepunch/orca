# Instructions Writing Guide

Use this as a starter when creating or refactoring agent instructions for a new
project. The goal is fast local-agent startup and useful codebase guidance.

## Core Idea

Keep always-on instructions tiny. Anything injected into every request should be
limited to hard rules, routing, and a few high-value paths. Put details in
lazy-loaded files that the agent reads only when the task needs them.

Recommended layout:

```text
.github/
  copilot-instructions.md
  agent-context/
    README.md
    frontend.md
    backend.md
    data-model.md
    build-and-tests.md
    style.md
    reference.md
```

`copilot-instructions.md` should usually be under 1-2 KB. Area files should be
small enough to read whole, usually 1-4 KB. Larger references are fine if they
are clearly marked as archives and not read by default.

## Hot-Path Template

Copy this into `.github/copilot-instructions.md` and adjust the routing table.

```md
# PROJECT Agent Rules

Keep this file tiny: agents may inject it into every model request. Put details
in `.github/agent-context/` and read them only when needed.

## Always

- State the 3-5 project rules that must apply to every task.
- Search first; then read the smallest relevant line range.
- Scope code-symbol searches to code directories so instruction files do not
  pollute grep results.
- Add a "plan-only" rule: for planning requests, read one area file plus a few
  exact symbol ranges, then write the plan. Do not let the agent do full
  implementation discovery before creating a plan.
- Ban web fetches for local implementation work unless external docs are
  explicitly requested.
- Do not preload large files or attach broad `@file` context unless requested.
- Do not edit generated files; edit sources and rerun generation.

## Before Editing

Read exactly the smallest matching area file; read two only for cross-area work.

| Task | Read |
|---|---|
| Frontend UI, styling, routes | `.github/agent-context/frontend.md` |
| Backend APIs, services, jobs | `.github/agent-context/backend.md` |
| Data model, migrations, queries | `.github/agent-context/data-model.md` |
| Build rules, tests, CI, docs | `.github/agent-context/build-and-tests.md` |
| Formatting/style only | `.github/agent-context/style.md` |

Full archived guide: `.github/agent-context/reference.md`. Do not read it by
default.

## Quick Paths

- Main app: `src/`
- Tests: `tests/`
- Config: `package.json`, `Makefile`, `.github/workflows/`

## Local-Agent Hygiene

- For casual questions, use a no-tool/minimal profile.
- Avoid repeating the same request text and file mention in one prompt.
- Prefer prompts like: "Find X, then read only relevant lines."
```

## Area File Template

Use one file per work area. Put concrete paths, workflows, and gotchas there.

```md
# AREA NAME

Read this when changing SHORT DESCRIPTION.

## Start Here

- `path/to/primary/source`
- `path/to/reference/test`
- `path/to/docs`

## Rules

- Rules specific to this area.
- Include ownership boundaries and dependency direction.
- Mention generated files and source-of-truth files.

## Workflow

1. Edit the source-of-truth file.
2. Regenerate or migrate if needed.
3. Add/update focused tests.
4. Run the narrowest useful validation.

## Gotchas

- Real failure modes observed in this project.
- Naming traps, path traps, init-order traps, or environment assumptions.

## Verify

- `command for focused tests`
- `broader command if shared behavior changed`
```

## What Belongs In Always-On Instructions

- Non-negotiable architecture constraints.
- Generated-file warnings.
- Read/search discipline.
- A routing table for lazy-loaded area files.
- A few path hints that prevent bad first searches.

## What Belongs In Area Files

- Component workflows.
- Build/test commands.
- Code style examples.
- Framework-specific conventions.
- Historical gotchas.
- Links to deeper docs.

## What Belongs In Archived Reference

- Long architecture overview.
- Full API tables.
- Big examples.
- Rare migration notes.
- Anything useful but not needed before the first tool call.

## Anti-Patterns

- Putting the whole architecture guide in `copilot-instructions.md`.
- Listing every directory and command in the hot path.
- Copying long generated schemas into always-on context.
- Telling the agent to read every area file before editing.
- Using vague area names that force the agent to open multiple files.
- Keeping stale paths in instructions; one bad path can create many wasted tool
  calls.

## Refactoring Existing Instructions

1. Measure the current file with `wc -l -w -c`.
2. Move the original to `agent-context/reference.md`.
3. Extract 4-8 area files by ownership or workflow.
4. Replace the hot-path file with the routing template.
5. Add quick paths for common search traps.
6. Test with a fresh local-agent thread and inspect logs for repeated bloat.
7. Trim anything that appears in every request but is not needed every time.
