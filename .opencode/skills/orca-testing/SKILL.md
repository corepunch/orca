---
name: orca-testing
description: Select, write, and run focused ORCA tests for C components, generated properties and messages, Lua APIs, XML loading, UI layout, styles, controllers, renderer behavior, and headless execution. Use whenever implementation changes require verification or an existing ORCA test fails.
---

# ORCA Testing

Read `docs/TESTING.md` before editing tests.

## Workflow

1. Read the implementation contract from the relevant generated reference
   under `.opencode/generated/{c,lua,xml}/<module>/`. Fall back to the owning
   `.cgen` only when the reference is unavailable.
2. Read the target test and at most one neighboring test with the same fixture
   style.
3. Read `tests/test_local.h` only when using its shared macros.
4. Add the smallest arrange/act/assert case that proves the behavior and its
   important failure path.
5. Use public aggregate headers in C tests. Declare a narrow local `extern`
   with a short explanation only when an exported private seam is unavoidable.
6. Wire a new standalone C test through the existing `C_TEST_RULE` pattern in
   `Makefile`.
7. Run the narrowest relevant target first; broaden only for shared behavior.

## Coverage expectations

- For a new component, cover creation or attachment, property access, and one
  handled message.
- For Lua exposure, cover call shape, conversion, and invalid input.
- For XML or controllers, cover loading and the resulting event, binding, or
  action.
- For layout or style, set deterministic dimensions and
  `ResizeMode="NoResize"`.
- For queued Lua, rebuild, or binding behavior, call `core.flushQueue()` before
  asserting.
- For renderer behavior, tolerate an uninitialized renderer in headless tests.

Do not call `removeFromParent()` on roots returned by
`loadObjectFromLispString()` or `loadObjectFromXmlString()`. Do not weaken
unrelated assertions to make a new test pass.

## Command selection

```bash
make test-properties   # property VM or focused C property work
make test-headless     # C binaries and headless Lua/UI/layout suite
xvfb-run make test     # display-backed behavior
```

Start narrow. Run `make test-headless` after shared engine, Lua, XML, or UI
changes. Use the display-backed suite only when the behavior requires it.
