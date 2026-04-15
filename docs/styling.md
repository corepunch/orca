# Style System

ORCA's style system implements a **CSS-like approach** for setting component properties on visual objects.
Style _classes_ are attached to objects at load time; _stylesheet rules_ map selectors to property values and are resolved when `Object.ThemeChanged` is dispatched.

---

## Overview

```
Object
  └─ StyleController (attach-only component)
       ├─ classes:    style_class* ─── linked list of parsed class tokens
       └─ stylesheet: style_rule* ─── linked list of CSS-like rules
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
| `stylesheet`| `style_rule*`    | Linked list of per-object stylesheet rules (owned by this component) |

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

The `class` XML attribute is parsed by `StyleController.AddClasses()` at load time.
Tokens are space-separated; each token is parsed into a `style_class` node.

### Setting classes from Lua

```lua
-- At object creation (via the property table):
local btn = UIKit.Button { class = "primary:hover" }

-- At runtime (assign a space-separated class string):
btn.class = "selected"
```

### C API

```c
// Parse a full space-separated class attribute string (called during XML load)
OBJ_ParseClassAttribute(lpObject_t obj, const char* classAttr);
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

`addStyleRule()` is a Lua method available on any `Node2D` (or any object with `StyleController`).
It registers rules on the calling object's `StyleController.stylesheet`.

```lua
-- Attach a stylesheet to a specific object
btn:addStyleRule(".button", {
    Background = "#3c6",
    Foreground = "white",
    Width      = 120,
    Height     = 40,
})

-- Pseudo-state rules
btn:addStyleRule(".button:hover", {
    Background = "#5e8",
})

btn:addStyleRule(".button:active", {
    Background = "#2a4",
})
```

Rules for the same selector can be split across multiple `addStyleRule()` calls; they are appended to the list and applied in order.

### `@apply` directive

The `@apply` key inside a rule table includes the rules from another selector (similar to Tailwind/PostCSS `@apply`):

```lua
btn:addStyleRule(".special-button", {
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
-- addStyleRule is exposed on every Node2D via core_export.c
obj:addStyleRule(".selector[:pseudo-state...]", { property = value, ... })
```

---

## Applying styles

### `Object.ThemeChanged` — the style trigger

Styles are applied by sending `Object.ThemeChanged` or `StyleController.ThemeChanged` to an object.
`StyleController` handles both messages and recalculates all matching rules.
Objects without a `StyleController` silently ignore the message.

Both messages have a single field:

| Field | Type | Description |
|-------|------|-------------|
| `recursive` | `bool_t` | When `TRUE`, the message is forwarded to every child object |

```c
// C — send the typed message directly:
_SendMessage(object, Object, ThemeChanged, .recursive = TRUE);
```

```lua
-- Lua — raises StyleController.ThemeChanged on this object (applies styles)
node:ThemeChanged()

-- With recursive descent into children:
node:ThemeChanged(StyleController_ThemeChangedEventArgs{ recursive = true })
```

!!! tip "Lua event syntax"
    `self:ThemeChanged()` works because `ThemeChanged` is an **event property** on `StyleController`.
    Accessing any event property as `self.EventName` returns a callable closure that sends the message.
    This pattern applies to all component events — for example, `player:Play()` sends `AnimationPlayer.Play`.
    See [Lua events and properties](#lua-events-and-properties) below.

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
| `dark`  | `STYLE_DARK`   | `axIsDarkTheme()` (system-wide) |

### Example: dark-mode background

```lua
screen:addStyleRule(".card", {
    Background = "white",
})

screen:addStyleRule(".card:dark", {
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

### `style_rule` (internal rule record)

```c
struct style_rule {
    struct style_rule* next;
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

## WPF XAML stylesheet format

In addition to the Lua `addStyleRule` API and CSS file loader, ORCA supports
a subset of the **WPF (Windows Presentation Foundation) XAML** stylesheet
format.  This is useful for teams migrating from or familiar with WPF/XAML
tooling, and it maps cleanly onto the same `addStyleRule` infrastructure.

### Supported elements

| WPF element / attribute | ORCA equivalent |
|---|---|
| `<Style x:Key="name">` | Selector `.name` |
| `<Style TargetType="Button">` | Selector `.Button` (see note below) |
| `<Style BasedOn="base">` | `@apply = "base"` (inherits base rules) |
| `<Setter Property="P" Value="V"/>` | Property `P` with value `V` |
| `<Style.Triggers>/<Trigger Property="IsMouseOver" Value="True">` | `:hover` pseudo-state rule |
| `<Style.Triggers>/<Trigger Property="IsFocused" Value="True">` | `:focus` pseudo-state rule |
| `<Style.Triggers>/<Trigger Property="IsKeyboardFocused" Value="True">` | `:focus` pseudo-state rule |
| `<Style.Triggers>/<Trigger Property="IsSelected" Value="True">` | `:active` pseudo-state rule |
| `<Style.Triggers>/<Trigger Property="IsChecked" Value="True">` | `:active` pseudo-state rule |
| `<Style.Triggers>/<Trigger Property="IsPressed" Value="True">` | `:active` pseudo-state rule |

`BasedOn` accepts either a plain name (`"button"`) or a `{StaticResource key}`
reference — both are resolved to the key name.

!!! note "`TargetType` and class matching"
    ORCA's style system matches rules by explicit class name, not by element
    type.  A `TargetType="Button"` style registers its rules under the selector
    `.Button`.  Objects receive these rules only if they have `class="Button"`
    (set explicitly or via the `class=` XML attribute).

### Standalone `.xaml` files

The `file-wpf.lua` plugin (loaded automatically from `share/plugins/`) adds a
`package.searcher` for `.xaml` files.  Requiring a `.xaml` path returns the
parsed style table — same format as `file-css.lua`:

```lua
local styles = require "themes/dark"  -- loads themes/dark.xaml
for k, v in pairs(styles) do
    screen:addStyleRule(k, v)
end
-- Or register globally so all screens receive the rules:
for k, v in pairs(require "themes/dark") do
    orca.styles[k] = v
end
```

### Inline `<StyleSheet>` in XML layouts

A `<StyleSheet>path/to/styles.xml</StyleSheet>` element inside an XML layout
file loads the referenced XML stylesheet and applies it directly to the
enclosing node.  The referenced file must have a `<Styles>` or
`<ResourceDictionary>` root element.

### Example

**`themes/dark.xaml`**

```xml
<ResourceDictionary
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml">

  <!-- Named class (equivalent to .btn { ... } in CSS) -->
  <Style x:Key="btn">
    <Setter Property="BackgroundColor" Value="#3b82f6"/>
    <Setter Property="ForegroundColor" Value="white"/>
    <Setter Property="CornerRadius"    Value="6"/>
    <Setter Property="Padding"         Value="8"/>
    <Style.Triggers>
      <Trigger Property="IsMouseOver" Value="True">
        <Setter Property="BackgroundColor" Value="#2563eb"/>
      </Trigger>
      <Trigger Property="IsSelected" Value="True">
        <Setter Property="BackgroundColor" Value="#1d4ed8"/>
      </Trigger>
    </Style.Triggers>
  </Style>

  <!-- Style inheritance via BasedOn (equivalent to @apply) -->
  <Style x:Key="btn-danger" BasedOn="btn">
    <Setter Property="BackgroundColor" Value="#ef4444"/>
  </Style>

  <!-- Dark-mode override using :dark pseudo-state -->
  <Style x:Key="card">
    <Setter Property="BackgroundColor" Value="white"/>
  </Style>

</ResourceDictionary>
```

**Lua usage:**

```lua
local styles = require "themes/dark"
for k, v in pairs(styles) do screen:addStyleRule(k, v) end

local btn = screen + ui.Button {
    class  = "btn",
    Text   = "Save",
    Width  = 120,
    Height = 36,
}
btn:ThemeChanged()
```

### CSS to WPF conversion reference

| CSS | WPF XAML equivalent |
|-----|---------------------|
| `.btn { background-color: #3b82f6; }` | `<Style x:Key="btn"><Setter Property="BackgroundColor" Value="#3b82f6"/></Style>` |
| `.btn:hover { background-color: #2563eb; }` | `<Style.Triggers><Trigger Property="IsMouseOver" Value="True"><Setter Property="BackgroundColor" Value="#2563eb"/></Trigger></Style.Triggers>` |
| `.btn:focus { border-color: yellow; }` | `<Trigger Property="IsFocused" Value="True"><Setter Property="BorderColor" Value="yellow"/></Trigger>` |
| `.btn:active { background-color: #1d4ed8; }` | `<Trigger Property="IsSelected" Value="True"><Setter Property="BackgroundColor" Value="#1d4ed8"/></Trigger>` |
| `@apply btn;` | `<Style x:Key="special" BasedOn="btn">` |

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
screen:addStyleRule(".btn", {
    Background = "#4a90d9",
    Foreground = "white",
    CornerRadius = 6,
    Padding = 8,
})

screen:addStyleRule(".btn:hover", {
    Background = "#3a80c9",
})

screen:addStyleRule(".btn:active", {
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
myBtn:ThemeChanged()
```

---

## Lua events and properties

ORCA objects expose a uniform Lua API for **reading and writing properties** and for **sending messages** through the component property system.

### Property access

Any property declared in a component's XML definition can be read and written directly on the Lua object:

```lua
-- Read a property
local w = node.Width          -- returns the current Width value

-- Write a property
node.Width = 120              -- sets Width; may trigger layout/render
node.Text  = "Hello"
node.Color = "#4a90d9"
```

Setting a property that is bound to layout or rendering automatically marks the object dirty and triggers the next paint cycle.

### Events (self:EventName)

Every **message** declared in a component's `<messages>` block becomes an **event property** accessible via `self.EventName`.  Reading the property returns a callable closure that sends the message synchronously:

```lua
-- AnimationPlayer events
player:Play()                       -- sends AnimationPlayer.Play
player:Stop()                       -- sends AnimationPlayer.Stop
player:Pause()                      -- sends AnimationPlayer.Pause

-- StyleController.ThemeChanged — apply styles
node:ThemeChanged()                  -- non-recursive (default)

-- Pass an EventArgs table for messages that carry data:
node:ThemeChanged(StyleController_ThemeChangedEventArgs{ recursive = true })
```

The pattern works for any component that defines messages:

| Call | Component | Message sent |
|------|-----------|--------------|
| `player:Play()` | `AnimationPlayer` | `AnimationPlayer.Play` |
| `player:Stop()` | `AnimationPlayer` | `AnimationPlayer.Stop` |
| `node:ThemeChanged()` | `StyleController` | `StyleController.ThemeChanged` |

### Event callbacks (self.EventName = function)

Assigning a Lua function to an event property registers it as a **callback** that fires when the corresponding message is dispatched to the object:

```lua
-- Called when the animation finishes
function player:Completed(event, sender)
    print("animation done")
end

-- Equivalent to:
player.Completed = function(self, event, sender)
    print("animation done")
end
```

The callback receives `(self, event, sender)` where `event` is the EventArgs struct pushed as a userdata (or nil for messages with no fields) and `sender` is the originating object.
