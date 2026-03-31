# Testing Guide

This page documents the ORCA test harness, how to write and run tests, and common gotchas that contributors encounter.

---

## Running Tests

### Headless layout tests (no display needed)

```bash
make test-properties    # C unit tests for the property VM
make test-headless      # Lua layout tests (orca -test=tests/test_layout.lua)
```

These run without a display or OpenGL context.

### Display-required tests

```bash
xvfb-run make test      # Full suite including test1.lua and test.xml
```

These require either a real display or a virtual framebuffer (`xvfb-run` on Linux, `Xvfb` on macOS CI). The engine creates a real window, initializes the renderer, and runs the test Lua/XML file.

### Running a single XML test

```bash
./build/bin/orca -test=tests/test.xml
```

The `-test=<file>` flag wraps the file in a short bootstrap script that `require`s all modules and then calls `doxmlfile(file)` (for `.xml`) or `luaL_dofile` (for `.lua`).

---

## Writing Layout Tests

### Lua test files

Layout tests are plain Lua scripts that use the `orca.UIKit` module. The engine bootstraps `orca`, `orca.UIKit`, and all plugins before running the file.

```lua
local orca = require "orca"
orca.init()
local ui = require "orca.UIKit"

-- IMPORTANT: Always set ResizeMode = "NoResize" for test screens
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

screen:msgSend("UpdateLayout", screen.Width, screen.Height)

assert(screen.Width == 1000, "Width should be 1000")
```

### XML test files

XML test files are loaded by `doxmlfile()`. Any `<script>` element inside the root element is executed after the node tree is constructed.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Screen Name="test" Width="1000" Height="1000" ResizeMode="NoResize">
    <Node2D Name="box" Width="100" Height="50" />
    <script>
        updateLayout(_ENV, Width, Height)
        local box = _ENV:findChild("box", true)
        assert(box.ActualWidth == 100)
        assert(box.ActualHeight == 50)
    </script>
</Screen>
```

---

## Common Gotchas

### Screen dimensions are overridden by the window size

`Screen.MeasureOverride` calls `WI_GetSize()` and overrides the screen's `Width`/`Height` with the actual window dimensions **when `ResizeMode == kResizeModeCanResize`**, which is the default.

On CI virtual displays (`xvfb-run`), the window is typically 640×480 — so a screen created without `ResizeMode = "NoResize"` silently becomes 640×480 instead of your intended size, causing layout assertions like `assert(screen.Width == 1000)` to fail.

**Always set `ResizeMode = "NoResize"` for test screens:**

```lua
-- Lua
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }
```

```xml
<!-- XML -->
<Screen Width="1000" Height="1000" ResizeMode="NoResize">
```

### `test-properties` vs display tests

`make test-properties` compiles and runs `tests/test_properties.c` as a standalone C binary with `TEST_MEMORY=1`. This binary intercepts `malloc`/`free` to track leaks — it does **not** link against the main orca library and has no Lua or renderer dependencies. It runs entirely headless.

`make test` runs `test-properties` first, then `test-headless`, then the display-required tests (`test1.lua`, `test.xml`). If `test-properties` fails, the display tests are never reached.

### The `TEST_MEMORY` allocator

The `tests/test_properties.c` test binary defines custom `malloc`/`calloc`/`realloc`/`free` wrappers that track allocation counts:

- On first call, `init_alloc_funcs()` resolves the real libc symbols via `dlsym(RTLD_NEXT, ...)`. A 1 KB bootstrap buffer handles the one `calloc` that `dlsym` makes internally.
- A recursion guard (`s_init_in_progress`) prevents re-entrancy during `dlsym`.
- Tracking is only active after `mem_init()` is called; allocations made during module initialization (GnuTLS, libEGL) are not counted.

> **macOS note:** The `__libc_malloc` / `__libc_free` symbols are glibc-internal and do not exist on Apple platforms. Always use `dlsym(RTLD_NEXT, "malloc")` for portable interception.

---

## Property VM Bindings

### Binding expressions

Property bindings in XML use a formula language evaluated by the property VM (`source/core/property/property_runtime.c`):

```xml
<!-- Simple property reference -->
<property name="Visible">
    <Binding>{./IsActive}</Binding>
</property>

<!-- Arithmetic -->
<property name="Width">
    <Binding>MUL({./Scale}, 100)</Binding>
</property>

<!-- Conditional (IF operator) -->
<property name="Text">
    <Binding>IF(EQUAL({./IsActive}, 1), "Active", "Inactive")</Binding>
</property>
```

### Supported operators

| Operator | Description |
|---|---|
| `{./PropertyPath}` | Property reference — reads from the binding target |
| `ADD(a, b)` | Addition |
| `SUB(a, b)` | Subtraction |
| `MUL(a, b)` | Multiplication |
| `DIV(a, b)` | Division |
| `EQUAL(a, b)` | Returns 1 if equal, 0 otherwise |
| `IF(cond, true-val, false-val)` | Conditional — returns `true-val` if `cond != 0` (or non-empty string), else `false-val` |
| `"string literal"` | String constant |
| `123`, `1.5` | Numeric constants |

### IF operator semantics

- Condition is truthy when: numeric `cond != 0`, or string `cond` is non-empty.
- The result type follows the selected branch (`true-val` or `false-val`).
- Both branches can be any expression, including nested operators.

### Binding modes

The `Mode` attribute on a `<Binding>` element controls when the binding is re-evaluated:

| Mode | Description |
|---|---|
| `Expression` (default) | Re-evaluated whenever the source property changes |
| `OneTime` | Evaluated once when the node is created |
| `OneWay` | Same as `Expression` |

```xml
<property name="Opacity">
    <Binding Mode="Expression" Attribute="WholeProperty">{./Alpha}</Binding>
</property>
```

---

## XML Attribute Parsing

### Namespace-aware attribute iteration

When iterating XML attributes in C (`source/parsers/p_xml.c`), use `xmlGetProp`/`xmlGetNsProp` rather than `xmlNodeListGetString`:

```c
/* Correct — works with XML_PARSE_COMPACT */
xmlChar* value = attr->ns
    ? xmlGetNsProp(attr->parent, attr->name, attr->ns->href)
    : xmlGetProp(attr->parent, attr->name);
lua_pushstring(L, (const char*)value);
xmlFree(value);
```

`xmlNodeListGetString(doc, attr->children, 1)` can return `NULL` when `XML_PARSE_COMPACT` alters how small text nodes are stored internally. The canonical `xmlGetProp` API is immune to this.
