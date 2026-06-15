# Testing

Use this file as the default map for adding or fixing tests.

## Where tests live

- C tests: `tests/test_*.c`
- Lua tests: `tests/test_*.lua`
- Lua spec-style tests: `tests/*_spec.lua`

## Authoritative commands

```bash
make test-properties
make test-headless
xvfb-run make test
```

## Which command to use

- Property VM or C-only work: start with `make test-properties` or the matching C target in `Makefile`.
- Headless Lua/UI/layout work: use `make test-headless`.
- Display-backed behavior: use `xvfb-run make test`.

## C test shape

C tests in this repo are standalone binaries wired in `Makefile` through `C_TEST_RULE`.
There is no single shared `test_utils.h` pattern across all C tests.

When adding a new C test:

1. Copy the closest existing file in `tests/`.
2. Keep the harness local to the file unless an existing helper already fits.
3. Add a dedicated target to `Makefile` if the file is a new standalone binary.
4. Prefer small arrange / act / assert blocks and explicit cleanup.

Good references:

- `tests/test_styles.c`
- `tests/test_filesystem.c`
- `tests/test_message_registry.c`

## Lua test shape

Most Lua tests:

1. `require` the modules they need (`orca.core`, `orca.UIKit`, etc.)
2. Create a small object tree.
3. Trigger behavior.
4. Use `assert(...)` or `require "orca.test"` helpers.
5. Call `core.flushQueue()` before asserting async or rebuild-driven behavior.

Good references:

- `tests/test_state_manager.lua`
- `tests/test_layout.lua`
- `tests/test_body.lua`

## High-value rules

- For explicit test screen sizes, always set `ResizeMode = "NoResize"` in Lua or `ResizeMode="NoResize"` in XML.
- Call `core.flushQueue()` after `rebuild()` and before asserting queued behavior.
- Do not call `removeFromParent()` on root objects created by `loadObjectFromLispString()` or `loadObjectFromXmlString()`.
- Prefer one focused new test over broad rewrites of existing tests.

## For local models

1. Read this file first.
2. Read the file under test.
3. Read at most one similar existing test.
4. Do not scan all of `tests/` unless the first attempt fails.
