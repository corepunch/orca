# Testing Guide

This document describes the C and Lua testing infrastructure used in the Orca project.

## Table of Contents
- [C Tests](#c-tests)
- [Lua Tests](#lua-tests)
- [Helpers & Macros](#helpers--macros)
- [Naming Conventions](#naming-conventions)
- [How to Add a New Test](#how-to-add-a-new-test)

---

## C Tests

### File Location

C tests live in `tests/` with filenames matching the pattern `test_<component>.c`.

### Structure

Each C test file follows this structure:

1. **Helper functions** (optional) — static functions that set up test fixtures or
   create test-specific objects. These are prefixed with `_` and documented with
   block comments.

2. **Test functions** — individual `static void test_<name>(void)` functions, each
   testing a single behavior or edge case.

3. **Test registry** — a static array of `{ name, function }` pairs using the
   `DECL_TEST` macro.

4. **`main()`** — iterates over the registry, running each test and tracking
   memory snapshots for leak detection.

### Example

```c
/* Test: parsing a :hover pseudo-class sets OF_HOVERABLE */
static void test_parse_class_hover_flag(void) {
    WITH(struct Object, obj, make_styled_object(), destroy_object) {
        EXPECT(!(OBJ_GetFlags(obj) & OF_HOVERABLE));
        _SendMessage(obj, StyleController, AddClasses, "btn:hover");
        EXPECT(OBJ_GetFlags(obj) & OF_HOVERABLE);
    }
}
```

Key points:
- Tests are `static` — they are not visible outside the test file.
- The `WITH` macro handles setup/teardown (resource management).
- `EXPECT` asserts conditions; failures increment `s_tests_failed`.
- `MEM_SNAPSHOT()` / `MEM_CHECK_LEAK()` detect memory leaks per test.

---

## Lua Tests

### File Location

Lua tests live in `tests/` with filenames matching the pattern `test_<component>.lua`.

### Structure

Each Lua test file follows this structure:

1. **Setup** — `require('test_utils')` to import helpers, then initialize the
   framework (e.g., `mem_init()`).

2. **Test blocks** — `it()` calls that group related assertions. Each `it()` takes
   a string description and a function body.

3. **Assertions** — `expect()` calls with chainable matchers (`.to_be()`,
   `.to_equal()`, `.to_have_key()`, etc.).

4. **Spies/Mocks** — `spy()` wraps functions to track calls, arguments, and return
   values.

5. **Test runner** — a call to the test framework's runner (e.g., `run_tests()`)
   that executes all registered `it()` blocks and reports results.

### Example

```lua
local tu = require('test_utils')
local it = tu.it
local expect = tu.expect
local spy = tu.spy

it('creates an object with the given class name', function()
    local obj = OBJ_Create(ID_TestObj)
    expect(OBJ_GetClassName(obj)).to_be('test')
    OBJ_Release(obj)
end)

it('tracks calls to a spied function', function()
    local add = spy.new(function(a, b) return a + b end)
    expect(add(2, 3)).to_equal(5)
    expect(add.calls).to_have_key(1)
    expect(add.calls[1].args).to_equal({2, 3})
end)
```

Key points:
- `it()` blocks are registered at parse time and executed by the runner.
- `expect()` returns a matcher object with fluent methods like `.to_be()`,
  `.to_equal()`, `.to_be_nil()`, `.to_have_key()`.
- `spy.new()` wraps any function, recording all calls in `.calls`.

---

## Helpers & Macros

### C Helpers

| Helper | Description |
|--------|-------------|
| `WITH(type, name, init_expr, cleanup_fn)` | RAII-style resource management. `init_expr` creates/allocates; `cleanup_fn` frees on scope exit. |
| `EXPECT(expr)` | Asserts `expr` is non-zero (true). Increments `s_tests_failed` on failure. |
| `EXPECT_OK(result)` | Asserts `SUCCEEDED(result)`. For COM-style HRESULT checks. |
| `MEM_SNAPSHOT()` | Captures current allocation state for leak detection. |
| `MEM_CHECK_LEAK(snap, test_name)` | Compares current allocations against `snap`; reports leaks. |
| `_SendMessage(obj, Component, Message, ...)` | Sends a message to a component, bypassing the public API for test access. |
| `make_<fixture>()` | Test helper that creates a configured object for testing. |

### Lua Helpers

| Helper | Description |
|--------|-------------|
| `it(description, fn)` | Registers a test case. Executed by the test runner. |
| `expect(value)` | Returns a matcher object for fluent assertions. |
| `spy.new(fn)` | Wraps `fn` to record all calls, arguments, and return values. |
| `tu.it`, `tu.expect`, `tu.spy` | Accessed via `require('test_utils')`. |

### Lua Matchers

| Matcher | Checks |
|---------|--------|
| `.to_be(expected)` | Strict equality (`==`). |
| `.to_equal(expected)` | Deep/equivalent equality. |
| `.to_be_nil()` | Value is `nil`. |
| `.to_be_truthy()` / `.to_be_falsy()` | Boolean truthiness. |
| `.to_have_key(key)` | Table contains the given key. |
| `.to_have_length(n)` | Table/string has length `n`. |

---

## Naming Conventions

### C Tests

- **Test function**: `test_<component>_<behavior>`, e.g. `test_parse_class_hover_flag`.
- **Helper functions**: `_` prefix, e.g. `_test_add_float_rule()`, `make_styled_object()`.
- **Test registry array**: `tests[]` — static array of `{ name, function }` pairs.
- **Global counters**: `s_tests_run`, `s_tests_failed`, `s_current_test`.

### Lua Tests

- **Test block description**: human-readable string, e.g. `'creates an object with the given class name'`.
- **Test file**: `test_<component>.lua`, e.g. `test_object.lua`, `test_message.lua`.
- **Test variables**: `local` scope, descriptive names, e.g. `obj`, `add`, `spy_obj`.

---

## How to Add a New Test

### C Test

1. **Create or open** `tests/test_<component>.c`.
2. **Add helper functions** (if needed) with `_` prefix and block comments.
3. **Write a test function**:
   ```c
   static void test_<component>_<behavior>(void) {
       WITH(struct Object, obj, make_<fixture>(), destroy_<fixture>) {
           /* setup */
           EXPECT(condition);
           /* action */
           EXPECT(result);
       }
   }
   ```
4. **Register the test** in the `tests[]` array using `DECL_TEST`:
   ```c
   DECL_TEST(test_<component>_<behavior>),
   ```
5. **Build and run** the test binary. Verify it passes and reports no memory leaks.

### Lua Test

1. **Create or open** `tests/test_<component>.lua`.
2. **Require helpers**:
   ```lua
   local tu = require('test_utils')
   local it = tu.it
   local expect = tu.expect
   ```
3. **Write test blocks**:
   ```lua
   it('describes the expected behavior', function()
       -- setup
       expect(value):to_be(expected)
       -- action
       expect(result):to_equal(expected)
   end)
   ```
4. **Run the test** with the Lua test runner.

---

## C Example: Complete New Test

```c
/*
 * test_trigger_actions.c
 */

/* Helper: creates a trigger-enabled object for testing */
static struct Object* make_trigger_obj(void) {
    struct Object *obj = OBJ_Create(ID_TriggerTest);
    /* configure object for trigger testing */
    return obj;
}

static void destroy_trigger_obj(struct Object *obj) {
    OBJ_Release(obj);
}

/* Test: pressing a trigger button invokes the bound action */
static void test_trigger_button_press(void) {
    WITH(struct Object, obj, make_trigger_obj(), destroy_trigger_obj) {
        /* Set up the trigger action */
        _SendMessage(obj, Trigger, BindAction, "press", on_press_callback);

        /* Simulate button press */
        _SendMessage(obj, Trigger, Press, .button = BTN_A);

        /* Verify the callback was invoked */
        EXPECT(g_press_called == TRUE);
    }
}

/* Register in tests[]: */
DECL_TEST(test_trigger_button_press),
```

---

## Lua Example: Complete New Test

```lua
-- test_filesystem.lua

local tu = require('test_utils')
local it = tu.it
local expect = tu.expect

-- Initialize framework
mem_init()

it('reads a file and returns its contents', function()
    local path = 'tests/fixtures/sample.txt'
    local contents = FS_ReadFile(path)
    expect(contents).to_be_truthy()
    expect(contents):to_have_length(> 0)
end)

it('returns nil for nonexistent files', function()
    local contents = FS_ReadFile('tests/fixtures/does_not_exist.txt')
    expect(contents).to_be_nil()
end)

it('tracks file system calls via spy', function()
    local read_file = spy.new(FS_ReadFile)
    local result = read_file('tests/fixtures/sample.txt')
    expect(read_file.calls).to_have_key(1)
    expect(read_file.calls[1].args[1]):to_equal('tests/fixtures/sample.txt')
end)

-- Run tests
run_tests()
```
