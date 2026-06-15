# ORCA Agent Guide

## Context policy

Before reading source files, check whether a relevant operational document exists in `docs/`.
Use those quick docs as the default map, then read only the file you need to change.

### Task-specific read order

- **Build or test work**: read `docs/BUILD.md`, then `docs/TESTING.md`.
- **Architecture lookup**: read `docs/PROJECT_MAP.md`, then `docs/ARCHITECTURE.md`.
- **Add or change a class/component**: read `docs/ADDING_CLASSES.md`, then the relevant module `.cgen`.
- **Lua-facing API work**: read `docs/LUA_API.md`.
- **UI, layout, XML, popup, or style work**: read `docs/UI_SYSTEM.md`.
- **C formatting or local conventions**: read `docs/CODING_STYLE.md`.

### Narrow-context rules

1. Prefer the matching file in `docs/` before scanning directories.
2. Read the file you are changing.
3. Read at most one similar implementation or test unless compilation fails.
4. Do not scan the whole repository unless the task explicitly requires exploration.

### Testing tasks

1. Read `docs/TESTING.md`.
2. Read only the file under test.
3. Read at most one similar existing test.
4. Do not scan all tests unless explicitly asked.

### Codegen tasks

- Treat `.cgen` files as the source of truth for public API shape and cheap context.
- Prefer reading `.cgen` summaries, details, topics, properties, and messages before reading generated files.
- Never edit `generated/` by hand; edit `.cgen` or source and rerun `make modules`.
