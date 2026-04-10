# Style System

ORCA's style system implements a **CSS-like approach** for setting component properties on visual objects.
Style _classes_ are attached to objects at load time; _stylesheet rules_ map selectors to property values and are resolved when `Object.ThemeChanged` is dispatched.

---

## Overview

```
Object
  └─ StyleController (attach-only component)
       ├─ classes:    style_class* ─── linked list of parsed class tokens
       └─ stylesheet: style_sheet* ─── linked list of CSS-like rules
```

When styles are applied, each class token on the object is looked up in the stylesheet chain.
Matching rules write their property values directly on the object.
Pseudo-state rules (`:hover`, `:focus`, `:active`, `:dark`) are applied only when the matching condition is active.

---

## StyleController component

`StyleController` is an **attach-only component** defined in `source/core/core.xml` and implemented in `source/core/components/StyleController.c`.
It is automatically attached to every `Node2D` (and any class that lists `StyleController` as a parent) because `Node2D` declares `parent="Node,StyleController"` in `UIKit.xml`.

```c
// Retrieve the component pointer; returns NULL if not attached
struct StyleController* sc = GetStyleController(object);
```

### Fields

| Field       | Type              | Description |
|-------------|-------------------|-------------|
| `classes`   | `style_class*`    | Linked list of parsed style class tokens (owned by this component) |
| `stylesheet`| `style_sheet*`    | Linked list of per-object stylesheet rules (owned by this component) |

A second **global stylesheet** (`static_sheet` in `StyleController.c`) holds rules that are visible to all objects and checked before the per-object chain.

---

## Style classes

A **style class** identifies an object's logical role (e.g., `"button"`, `"primary"`) and optional pseudo-states.

### Syntax

```
ClassName[:pseudo-state...][ / opacity]
```

| Part | Example | Description |
|------|---------|-------------|
| Base name | `button` | Lookup key matched against stylesheet selectors |
| Pseudo-state(s) | `:hover`, `:focus`, `:active`, `:dark` | Gate the class — the rule fires only when this state is active |
| Opacity | `/50` | 0–100 integer applied to the alpha channel of color properties |

Multiple pseudo-states can be chained: `button:hover:focus` requires both hover and focus to be active.

### Setting classes from XML

```xml
<Button class="button:hover primary/80" />
```

The `class` XML attribute is parsed by `OBJ_ParseClassAttribute()` at load time.
Tokens are space-separated; each token is parsed into a `style_class` node.

### Setting classes from Lua

```lua
-- At object creation (via the property table):
local btn = UIKit.Button { class = "primary:hover" }

-- At runtime:
core.addClassToObject(btn, "selected")
```

### C API

```c
// Parse a full space-separated class attribute string (called during XML load)
OBJ_ParseClassAttribute(lpObject_t obj, const char* classAttr);

// Add a single class token at runtime
OBJ_AddClass(lpObject_t obj, const char* token);  // e.g., "primary:hover"
```

---

## Stylesheet rules

A **stylesheet rule** maps a selector and a property name to a string value.
The selector is a class name (with or without a leading `.`); the optional pseudo-states on the selector gate when the rule fires.

### Rule structure

```
.selector[:pseudo-state...]  { propertyName: "value" }
```

### Loading stylesheets from Lua

`addStyleSheet()` is a Lua method available on any `Node2D` (or any object with `StyleController`).
It registers rules on the calling object's `StyleController.stylesheet`.

```lua
-- Attach a stylesheet to a specific object
btn:addStyleSheet(".button", {
    Background = "#3c6",
    Foreground = "white",
    Width      = 120,
    Height     = 40,
})

-- Pseudo-state rules
btn:addStyleSheet(".button:hover", {
    Background = "#5e8",
})

btn:addStyleSheet(".button:active", {
    Background = "#2a4",
})
```

Rules for the same selector can be split across multiple `addStyleSheet()` calls; they are appended to the list and applied in order.

### `@apply` directive

The `@apply` key inside a rule table includes the rules from another selector (similar to Tailwind/PostCSS `@apply`):

```lua
btn:addStyleSheet(".special-button", {
    ["@apply"] = "button",   -- inherit all ".button" rules
    Background = "#a0f",     -- override background
})
```

### C API

```c
// Register a single rule on an object (selector may include a leading '.')
OBJ_AddStyleClass(
    lpObject_t  obj,
    const char* name,      // e.g., "button" or ".button"
    const char* property,  // e.g., "Background"
    const char* value,     // e.g., "#3c6"
    uint32_t    flags      // STYLE_HOVER | STYLE_FOCUS etc., or 0
);
```

### Lua API

```lua
-- addStyleSheet is exposed on every Node2D via core_export.c
obj:addStyleSheet(".selector[:pseudo-state...]", { property = value, ... })
```

---

## Applying styles

### `Object.ThemeChanged` — the style trigger

Styles are applied by sending `Object.ThemeChanged` to an object.
`StyleController` handles this message and recalculates all matching rules.
Objects without a `StyleController` silently ignore the message.

The message has a single field:

| Field | Type | Description |
|-------|------|-------------|
| `recursive` | `bool_t` | When `TRUE`, the message is forwarded to every child object |

```c
// C — convenience wrapper (sends Object.ThemeChanged internally)
OBJ_ApplyStyles(lpObject_t object, bool_t recursive);

// Alternatively, send the typed message directly:
_SendMessage(object, Object, ThemeChanged, .recursive = TRUE);
```

```lua
-- Lua — method call (wraps OBJ_ApplyStyles)
node:applyStyles(recursive --[[bool]])
```

### Resolution order

When `StyleController` receives `Object.ThemeChanged` for an object:

1. `PROP_ClearSpecialized` resets any previously applied specialised values.
2. **Body rules** — if the object is a root node (no parent), rules with the selector `"body"` in its own stylesheet are applied first.
3. **Per-class rules** — for each `style_class` attached to the object:
   - If the class has pseudo-state flags (`STYLE_HOVER`, etc.), the current object state is tested; the class is skipped when the state does not match.
   - `OBJ_EnumStyleClasses()` walks the global stylesheet first, then each ancestor's stylesheet bottom-up, and invokes `_ApplyRule` for every matching rule.
4. **Specialised-flag guard** — for each property hit by a state-gated rule, `PF_SPECIALIZED` is set.
   Any subsequent un-gated (default) rule for the same property is skipped when `PF_SPECIALIZED` is already set, ensuring the state-specific value wins.
5. **Recursive descent** — if `recursive=TRUE`, `Object.ThemeChanged` with `recursive=TRUE` is sent to every direct child.

### When styles are applied automatically

- On `Object.ThemeChanged` (non-recursive) — sent automatically by the engine when hover state changes (`CORE_UpdateHover`) or focus changes (`OBJ_SetFocus`). `StyleController` handles this message directly; no intermediate Node handler is needed.
- On `Object.PropertyChanged` for the `class` property — when Lua sets `node.class = "…"`.
- Explicitly by application code after state changes.

---

## Pseudo-states

| Token   | Flag           | Condition checked by `OBJ_GetStyleFlags()` |
|---------|----------------|--------------------------------------------|
| `hover` | `STYLE_HOVER`  | `core_GetHover() == object` |
| `focus` | `STYLE_FOCUS`  | `core_GetFocus() == object` |
| `active`| `STYLE_SELECT` | `OBJ_GetFlags(object) & OF_SELECTED` |
| `dark`  | `STYLE_DARK`   | `WI_IsDarkTheme()` (system-wide) |

### Example: dark-mode background

```lua
screen:addStyleSheet(".card", {
    Background = "white",
})

screen:addStyleSheet(".card:dark", {
    Background = "#1e1e2e",
})
```

```xml
<Node2D class="card" />
```

When the user switches to dark mode, `Object.ThemeChanged` is broadcast, `StyleController` re-runs style application, and the `:dark` rule takes precedence.

---

## Opacity modifier

Color properties can be dimmed using the `/N` opacity syntax on the class token:

```xml
<!-- "primary" class at 70% opacity -->
<Node2D class="primary/70" />
```

After the color value is resolved, the alpha channel is overwritten with `N / 100.0`.

---

## Data structures

### `style_class` (internal)

```c
struct style_class {
    struct style_class* next;
    shortStr_t value;   // base class name (e.g., "button", not "button:hover")
    byte_t flags;       // STYLE_HOVER | STYLE_FOCUS | STYLE_DARK | STYLE_SELECT
    byte_t opacity;     // 0–100 (default 100)
};
```

### `style_sheet` (internal rule record)

```c
struct style_sheet {
    struct style_sheet* next;
    uint32_t class_id;    // FNV1a32 of base selector (without leading '.')
    uint32_t prop_id;     // FNV1a32 of property name
    uint32_t flags;       // pseudo-state gate mask (same bit layout as style_class.flags)
    shortStr_t classname; // selector string (e.g., ".button")
    shortStr_t name;      // property name (e.g., "Background")
    shortStr_t value;     // property value string (e.g., "#3c6")
};
```

`class_id` and `prop_id` are pre-computed at rule-insertion time for O(1) matching during resolution.

---

## Object.Release and memory

`StyleController` handles `Object.Release` and calls `OBJ_ClearStyleClasses()`, which frees both the `classes` and `stylesheet` linked lists.
The global `static_sheet` is never freed (it lives for the duration of the process).

---

## Complete Lua example

```lua
local core = require "orca.core"
local ui   = require "orca.UIKit"

local screen = ui.Screen { Width = 800, Height = 600, ResizeMode = "NoResize" }

-- Define a global stylesheet on the root screen
screen:addStyleSheet(".btn", {
    Background = "#4a90d9",
    Foreground = "white",
    CornerRadius = 6,
    Padding = 8,
})

screen:addStyleSheet(".btn:hover", {
    Background = "#3a80c9",
})

screen:addStyleSheet(".btn:active", {
    Background = "#2a70b9",
})

-- Create a button and assign the "btn" class
local myBtn = screen + ui.Button {
    class = "btn",
    Text  = "Click me",
    Width = 120,
    Height = 36,
}

-- Apply styles initially
myBtn:applyStyles(false)
```
