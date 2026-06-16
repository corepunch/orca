# ORCA Agent Guide

## Context policy

Minimize prompt size and avoid reading source files unnecessarily.

Do not reconstruct architecture by reading many source files. If the relevant documentation, API, or target file is not obvious, ask the user which component or document to use before exploring broadly.

Use the following sources in order:

1. Read the relevant document in `docs/` for high-level architecture, conventions, and workflows.
2. Read `.cgen` files to understand public APIs, classes, structs, methods, properties, messages, and Lua bindings.
3. Use `ctags` to locate symbols, determine ownership of functions/types/macros, and identify relevant implementation files.
4. Use `grep` or regex searches only for textual searches such as string literals, macro expansions, or patterns that are not represented as symbols.
5. Read source files only when their implementation details are required.

Avoid scanning entire directories or reading many files verbatim.

## Clarification policy

For ambiguous tasks, ask questions before exploring.

Ask for:
- the relevant subsystem or module
- the relevant document
- the expected behavior
- the reference implementation, if one exists

A short question is preferred over reading dozens of files.

## Task-specific read order

* **Build or test work**: read `docs/BUILD.md`, then `docs/TESTING.md`.
* **Architecture lookup**: read `docs/PROJECT_MAP.md`, then `docs/ARCHITECTURE.md`.
* **Add or change a class/component**: read the relevant `.cgen` file, then the related `docs/` files, then the implementation.
* **Lua-facing API work**: read `.cgen` first, then `docs/LUA_API.md`.
* **UI, layout, XML, popup, or style work**: read `docs/UI_SYSTEM.md`.
* **C formatting or local conventions**: read `docs/CODING_STYLE.md`.

## Narrow-context rules

1. Prefer `docs/` and `.cgen` files before opening C source files.
2. Use `ctags` for symbol lookup before repository-wide searches.
3. Read the file you intend to change.
4. Read at most one similar implementation or test unless compilation, tests, or diagnostics indicate broader investigation is required.
5. Do not scan the entire repository unless explicitly required by the task.
6. If more than 3-5 files must be read to understand a task, stop and ask the user for guidance or additional documentation.
7. Expand the investigation only when compilation errors, failing tests, or runtime diagnostics require it.

## Test editing rules

When editing tests:

1. Read `docs/TESTING.md`.
2. Read the specific test file being changed.
3. Read `tests/test_local.h` if shared test helpers or macros are involved.
4. Read at most one neighboring test file for style/reference.
5. Do not scan all `tests/*.c` unless the task is explicitly to refactor all tests.
6. Do not launch exploration subagents for ordinary test edits.

## Working with APIs, classes, and generated code

* `.cgen` files are the authoritative API specification.
* They describe public classes, structs, methods, fields, properties, messages, documentation, and Lua exports.
* Prefer reading `.cgen` files instead of generated headers or bindings when understanding an API.
* Never edit files under `generated/` by hand. Modify `.cgen` or source files and rerun `make modules`.
* Use `ctags` to find implementations and usages of APIs defined by `.cgen`.

## Anti-exploration rules

Do not run broad repository exploration unless explicitly requested.

Avoid:

- `Glob "**/*"` or repeated broad `Glob` calls.
- Reading every file in a directory.
- Reading all tests to find examples.
- Re-running grep/glob for facts already discovered.
- Spawning subagents for small local edits.
- Using "Explore" or repository analysis workflows unless explicitly requested.
- Inferring project conventions from many examples; consult `docs/CODING_STYLE.md`.
- Searching for "how similar code is written" by reading many files.

For local edits:

1. Read the target file.
2. Read the nearest shared helper/header if needed.
3. Use `ctags` or a narrow grep for a specific symbol/pattern.
4. Edit.
5. Build/test.