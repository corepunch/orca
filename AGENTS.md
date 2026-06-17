# ORCA Agent Guide

## Source priority

Use sources in this order — stop as soon as the task is clear:

1. `docs/` — architecture, conventions, workflows
2. `docs/generated/*.md` — API documentation: classes, structs, methods, properties, messages, Lua exports
3. `.cgen` — authoritative source to access only when adding new classes, not for reference
4. `ctags` — symbol lookup, ownership, relevant implementation files
4. `grep` — string literals, macro expansions, patterns not represented as symbols
5. Source files — only when implementation details are required

## Task read order

| Task | Read |
|------|------|
| Build / test | `docs/BUILD.md`, `docs/TESTING.md` |
| Architecture | `docs/PROJECT_MAP.md`, `docs/ARCHITECTURE.md` |
| Class / component | `docs/generated/` for the relevant module, then related `docs/`, then implementation |
| Lua API | `docs/generated/` for the relevant module, then `docs/LUA_API.md` |
| UI / layout / XML / style | `docs/UI_SYSTEM.md` |
| C style / conventions | `docs/CODING_STYLE.md` |

## Clarification policy

For ambiguous tasks, ask before exploring. Ask for the relevant subsystem, document, expected behavior, or reference implementation. A short question beats reading dozens of files.

## Narrow-context rules

- Read at most one similar implementation or test for reference.
- If more than 3–5 files are needed to understand a task, stop and ask.
- Do not scan entire directories or the whole repository unless explicitly required.
- Expand investigation only when compilation errors, failing tests, or diagnostics require it.

## Test edits

Read `docs/TESTING.md`, the target test file, and `tests/test_local.h` if shared macros are involved. At most one neighboring test for reference. Do not scan all `tests/*.c` unless explicitly refactoring all tests.

## Generated code

Never edit `generated/` by hand — modify `.cgen` or source files and rerun `make modules`.

## Anti-exploration

Avoid `Glob "**/*"`, reading every file in a directory, re-running searches for known facts, spawning subagents for small edits, or inferring conventions from many examples.

For local edits: read target → nearest helper if needed → `ctags`/narrow grep → edit → build/test.

## Planning mode

- Ask clarifying questions before proposing a plan.
- Never assume design, tech stack, or features.
- Use sub-agents for research and to review the plan before presenting it.

## Change / edit mode

- Coordinate sub-agents rather than implementing directly.
- Parallelize independent changes across sub-agents.
- Use premium models for complex tasks, mid-tier for simpler ones.
- After each feature, run lint, type check, and build.