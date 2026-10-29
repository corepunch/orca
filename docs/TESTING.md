# Testing

Use this file as the default map for adding or fixing tests.

## Where tests live

- C tests: `tests/test_*.c`
- Lua tests: `tests/test_*.lua`
- Lua spec-style tests: `tests/*_spec.lua`
- Scene-format tests: `tests/lisp/test_*.lua` and `tests/xml/test_*.lua`

Tests in `tests/` run in both scene-format builds. A test that needs one
format's loader or fixtures (`loadObjectFromLispString`, a `.lisp` file) goes in
`tests/<format>/`, and only that build runs it. The Makefile picks these files
up by wildcard: `tests/lisp/test_metro_lisp.lua` becomes `make test-metro-lisp`
in the Lisp build. When a test file mixes both kinds of case, keep the
format-independent cases in `tests/` and move the others to
`tests/<format>/test_<name>_<format>.lua`.

```bash
make test-headless                                              # XML build
make CPPFLAGS=-DORCA_USE_LISP BUILDDIR=build-lisp test-headless # Lisp build
```

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

## Fast Chat iteration

Appwrite validation is **unit-only by default**. Use
`make test-chat-appwrite` for the Lua adapter (standalone Lua; no engine build
or startup), or `make test-chat-appwrite-backend` for Function logic with
injected dependencies. Select affected backend cases with
`node --test --test-name-pattern='PATTERN' apps/chat/backend/appwrite/test/backend.test.js`.
Do not run Appwrite checks for unrelated UI changes or repeat passing tests
without a relevant code change. Integration, end-to-end and live-suite emulator
runs require an explicit user request, even when they are offline.

For app-only Chat presentation changes, `make test-chat-ui` runs the offline
timeline, photo-loader, and footer checks using the existing binary. It does
not rebuild the engine or contact Appwrite. Run only the affected test file
when possible, for example `build/bin/orca -test=tests/test_chat_timeline.lua` or
`build/bin/orca -test=tests/test_chat_photo_viewer.lua`.

Rebuild with `make unite` when native code/API changes or the binary is
missing. Some individual engine test targets depend on `unite`, so invoking
the existing binary directly avoids those rebuilds during app-only work.
Use `make test-chat` for broader offline Chat changes; do not repeat passing
suites without a relevant change or unresolved concern.

Live Appwrite setup, cleanup, Node/native suites, and two-account device checks
run only when the user explicitly requests them. Never start them as an
automatic follow-up to an edit, build, deployment, or offline test. See the
[live test guide](../apps/chat/backend/appwrite/live/README.md).

## String property ownership

Run the focused allocation and binding suites with:

```bash
make test-string-memory
```

`tests/test_string_properties.c` checks ownership of literals, stack/heap
buffers, interior aliases, empty and large Unicode strings, independent
copies, inheritance, state overrides, binding expressions and tokens. Its
stress cases exercise chained and conditional bindings, fan-out, temporary
storage overflow within and across nested evaluations, recovery after failed
evaluation, and destruction in different orders. Each case must return to its
exact live-allocation baseline using `mem_tracker.h`.

`tests/test_string_bindings.lua` exercises the public Lua and XML APIs,
repeated binding replacement, inherited DataContext replacement, snapshots,
reparenting and garbage collection. Lua-created graph tests also check native
object counts after collection; XML tests check expression and string lifetime.
Both suites run in `make test-headless`.

On Linux with GCC or Clang sanitizer support:

```bash
make test-string-memory-sanitized
```

This rebuilds the engine, plugins and tests in `build/string-sanitized`, runs
both suites with AddressSanitizer, UndefinedBehaviorSanitizer and leak
detection, and disables the test allocator interposition so the sanitizers
can intercept allocations themselves. The ordinary build remains separate.
For a focused C reproduction, pass a case-name substring to
`build/bin/test_string_properties`, for example `interior_alias`.

The Lua suite currently exposes additional loader lifetime leaks under
LeakSanitizer: XML loaders retain their initial root reference, and
`loadObjectFromXmlStringWithSchema` leaves its inline schema registered without
returning an owner that can free it. The sanitizer target reports these as
failures. The allocation-tracked C ownership suite is independently leak-clean.

## Event arguments and controller serialization

```bash
make test-message-registry test-message-payload test-event-args test-event-serialization test-controllers
```

These offline suites cover reflected payload snapshots, string and object
ownership, component fields, unsigned values, rejected cyclic/deep graphs,
queue cancellation/coalescing/disposal, and abandoned Lua events. Controller
tests assert callback behavior across named declarations, conventions, reload,
cloning, explicit clearing, and handler replacement. They also round-trip long
Unicode message strings and native lowercase field names.

## MVVM form bindings

```bash
make test-two-way-bindings test-mvvm-form
ORCA_MVVM_NATIVE=1 xvfb-run -a build/bin/orca -test=tests/test_mvvm_form_sample.lua
```

The binding suite covers immediate draft edits, inherited and replaced data
contexts, reparenting, XML shorthand, serialization, binding chains, Unicode,
null values, invalid reverse expressions, and object reclamation. The sample
suite exercises XML-defined fields and real Save, Discard, and checkbox events.
Text fields bind to string draft properties; parse numeric input during Save
validation. Computed expressions remain forward-only.

## C test shape

C tests in this repo are standalone binaries wired in `Makefile` through `C_TEST_RULE`.
There is no single shared `test_utils.h` pattern across all C tests.

When adding a new C test:

1. Copy the closest existing file in `tests/`.
2. Keep the harness local to the file unless an existing helper already fits.
3. Add a dedicated target to `Makefile` if the file is a new standalone binary.
4. Prefer small arrange / act / assert blocks and explicit cleanup.

Include policy:

- Include public aggregate headers such as `<include/orca.h>`, `<core/core.h>`,
  `<filesystem/filesystem.h>`, and plugin/module headers.
- Do not include private implementation headers from `source/`, such as
  `source/core/core_local.h`, `source/core/object/object_internal.h`, or
  `source/filesystem/fs_xml_inline.h`.
- If a C test must exercise an exported-but-private symbol, declare the narrow
  `extern` locally in that test and add a short comment explaining the seam.

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

## Test macros (test_local.h)

All macros are defined in `tests/test_local.h`. Include it in any test file that
uses them.

### Core test harness

| Macro | Description |
|-------|-------------|
| `RUN(test_name)` | Top-level test runner. Creates a named test function with pass/fail tracking. |
| `END_RUN` | Closes a `RUN` block. Prints "OK" or "FAIL" with failure count. |
| `WITH(type, var, init, cleanup)` | Scoped resource management with automatic cleanup via `break` on failure. |
| `END_WITH` | Closes a `WITH` block, calls the cleanup function. |
| `EXPECT(expr)` | Asserts a condition. On failure, prints file/line and breaks out of `WITH`. |
| `EXPECT_OK(hr)` | Asserts an `HRESULT` indicates success (`SUCCEEDED`). |
| `EXPECT_NULL(ptr)` | Asserts a pointer is `NULL`. |

### Property helpers

| Macro | Description |
|-------|-------------|
| `FIND_SHORT_PROPERTY(obj, name, prop_ptr)` | Finds a property by fnv1a32 hash and assigns to pointer. |
| `PROP_STR(s)` | Wraps a string literal as `char**` for `PROP_SetValue`. |
| `PROP_TEST(obj, name, dtype, setval, cmpval)` | Shorthand for find→type→null→set→not-null→value pattern. |
| `PROP_GET(obj, name, type)` | Gets a property value and casts to a specific type. |
| `PROP_SET(obj, name, value)` | Sets a property value. |
| `PROP_GET_CAST(obj, name, type)` | Gets a property value with explicit cast. |
| `RUN_PROG(obj, expr, reg)` | Shorthand for create-token → run-program → release-token. |

### Object creation and manipulation

| Macro | Description |
|-------|-------------|
| `CREATE_OBJECT(className)` | Creates an object of a given class with no parameters. |
| `CREATE_OBJECT_WITH_PARAMS(className, params)` | Creates an object with a parameter list. |
| `GET_PROPERTY(obj, name)` | Shorthand for find→get value pattern. |
| `SET_PROPERTY(obj, name, value)` | Shorthand for find→set value pattern. |
| `GET_CLASS_NAME(obj)` | Gets the class name of an object. |
| `IS_TYPE(obj, typeName)` | Checks if an object is of a given type (fnv1a32 class ID comparison). |
| `AS_TYPE(obj, typeName)` | Casts an object to a specific type (no explicit cast). |
| `AS_TYPE_CAST(obj, typeName, castType)` | Casts an object with explicit type cast. |

### Object iteration

| Macro | Description |
|-------|-------------|
| `FOR_EACH_CHILD(parent, child)` | Iterates over child objects. |
| `FOR_EACH_CHILD_WITH_CONTEXT(parent, child, ctx)` | Iterates over children with a context pointer. |

### Window helpers

| Macro | Description |
|-------|-------------|
| `OPEN_WINDOW(name)` | Opens a test window via `WND_Create`. |
| `CLOSE_WINDOW(window)` | Closes a test window via `WND_Destroy`. |

### Message helpers

| Macro | Description |
|-------|-------------|
| `SEND_MSG(obj, msg, wp, lp)` | Sends a message to an object and returns the result. |
| `POST_MSG(obj, msg, wp, lp)` | Posts a message to an object. |
| `MSG_RESULT(result)` | Returns the result of a message handler. |
| `MSG_RESULT_EQ(result, expected)` | Checks that a message result equals a value. |
| `REGISTER_MESSAGE(msg)` | Registers a message in the registry. |
| `FIND_MESSAGE(msg)` | Finds a message in the registry. |
| `SUBSCRIBE(msg, handler)` | Subscribes a handler to a message. |

### Comparison helpers

| Macro | Description |
|-------|-------------|
| `FLOAT_EQ(a, b)` | Checks that two floats are approximately equal (within 0.001). |
| `UINT_EQ(a, b)` | Checks that two unsigned ints are equal. |
| `SIZE_EQ(a, b)` | Checks that two sizes/lengths are equal. |
| `STRING_EQ(a, b)` | Checks that two strings are equal (`strcmp`). |
| `STR_EQ(a, b)` | Alias for `STRING_EQ`. |
| `STRNEQ(a, b)` | Checks that two strings are NOT equal. |

### Memory helpers

| Macro | Description |
|-------|-------------|
| `ASSERT_NOT_NULL(ptr)` | Asserts a pointer is not `NULL`. |
| `CHECK_ALLOC(ptr, msg)` | Asserts a pointer is not `NULL` (with message). |
| `FREE_IF_NULL(ptr)` | Frees a pointer and sets it to `NULL` if non-NULL. |

### File helpers

| Macro | Description |
|-------|-------------|
| `READ_FILE(path, buf, len)` | Reads a file into a buffer (returns `HRESULT`). |
| `WRITE_FILE(path, buf, len)` | Writes a buffer to a file (returns `HRESULT`). |
| `FILE_EXISTS(path)` | Checks if a file exists. |

### Lua helpers

| Macro | Description |
|-------|-------------|
| `LUA_STATE(L)` | Creates a Lua state (`lua_open()`). |
| `LUA_RELEASE(L)` | Closes a Lua state (`lua_close`). |
| `LUA_DOFILE(L, path)` | Executes a Lua file. |
| `LUA_DOSTRING(L, str)` | Executes a Lua string. |

### Trigger helpers

| Macro | Description |
|-------|-------------|
| `TRIGGER_ACTION(obj, action)` | Triggers a trigger action and returns the result. |
| `OBSERVED_COUNT()` | Gets the number of observed trigger actions. |
| `RESET_OBSERVED()` | Resets the observed trigger action count. |

### XML helpers (placeholders)

| Macro | Description |
|-------|-------------|
| `XML_PARSE(xml)` | Placeholder for parsing an XML string. |
| `XML_FREE(parser)` | Placeholder for freeing an XML parser. |
| `XML_FIND(parser, name)` | Placeholder for finding an XML element. |
| `XML_GET_ATTR(element, attr)` | Placeholder for getting an XML attribute. |

### Editor helpers (placeholders)

| Macro | Description |
|-------|-------------|
| `EDITOR_NEW()` | Placeholder for creating a new editor. |
| `EDITOR_FREE(editor)` | Placeholder for freeing an editor. |
| `EDITOR_SET(editor, text)` | Placeholder for setting editor content. |
| `EDITOR_GET(editor)` | Placeholder for getting editor content. |

## For local models

1. Read this file first.
2. Read the file under test.
3. Read at most one similar existing test.
4. Do not scan all of `tests/` unless the first attempt fails.
