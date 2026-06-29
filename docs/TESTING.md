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
