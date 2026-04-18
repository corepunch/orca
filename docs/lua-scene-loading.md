# Lua Scene Loading: `body()`, `rebuild()`, and Component Composition

This guide covers how to build and populate scene trees from Lua — the same
mechanism used by the Banking sample and every MoonScript component in the
codebase.

---

## Core Concepts

### `rebuild()` — the scene-population entry point

Every Behaviour-based object has a `rebuild()` method.  Calling `rebuild()`
runs the object's `body` function, which populates the object's children.

```lua
local ui = require "orca.UIKit"
local container = screen + ui.Node2D {}

container:rebuild(function(self)
  self:addChild(ui.Node2D { Name = "Header" })
  self:addChild(ui.Node2D { Name = "Content" })
end)
```

`rebuild()` is called **automatically** at the end of every object's
constructor (`Behaviour.__init` calls `self:rebuild()`), so the `body`
method defined in a class is run once at creation without any extra call.

### The `body` field

`rebuild(fn)` stores `fn` as the object's `body` field and then runs it.
A class can define `body` as a method, in which case `rebuild()` in the
constructor picks it up automatically:

```lua
local MyWidget = ui.Node2D:extend {
  body = function(self)
    self:addChild(ui.TextBlock { Text = "Hello" })
  end
}

local w = screen + MyWidget {}   -- body() is called during construction
```

---

## Construction Syntax

### Table constructor `{}`

The standard Lua way to set initial properties:

```lua
local node = ui.Node2D { Name = "Box", Width = 100, Height = 50 }
```

All key/value pairs in the table are assigned as properties on the native
C object (`__newindex` → `OBJ_SetProperty`).

### Class shorthand string `".class-name"`

Pass a CSS-class string as the second argument to apply a style class at
construction time:

```lua
local node = ui.Node2D ".rounded.shadow"
local btn  = ui.Button "#my-id"   -- # sets Name, . applies a class
```

- A string starting with `"."` applies one or more dot-separated style
  classes (same as `obj:addClass("rounded"); obj:addClass("shadow")`).
- A string starting with `"#"` sets the object's `Name` to the part after
  `#`, then applies any additional `.class` segments.

You can combine both a string and a table:

```lua
local node = ui.Node2D(".container", { Width = 300 })
```

### Function body at construction time

Pass a function as the second argument to provide a one-shot body function:

```lua
local container = screen + ui.Node2D(function(self)
  self:addChild(ui.TextBlock { Text = "dynamic content" })
end)
```

When a plain function is passed this way, the `OF_CLEARBODY` flag is set:
after the body runs once, the `body` field is cleared so subsequent
`rebuild()` calls do not repeat the same children without explicit input.
This is the correct pattern when a body is data-driven and you want full
control over when it runs again.

---

## Extending UIKit Classes

### The `extend {}` pattern

Any UIKit class can be subclassed in Lua using the `extend {}` method
inherited from `Behaviour`:

```lua
local MyCard = ui.Node2D:extend {
  apply = function(self)
    return "flex-col rounded shadow p-4"   -- Tailwind-style classes
  end,

  body = function(self)
    self:addChild(ui.TextBlock { Text = self.Title })
    self:addChild(ui.TextBlock { Text = self.Subtitle })
  end
}

local card = screen + MyCard { Title = "Balance", Subtitle = "$1,234.56" }
```

The `extend {}` call:
1. Creates a new Lua class that inherits from the UIKit class.
2. The `body` function is called by `rebuild()` during construction.
3. The `apply` function (if present) returns a string of style classes that
   are applied automatically by the engine.

### Banking-sample pattern (MoonScript equivalent)

The Banking sample uses MoonScript class syntax, which compiles to the same
pattern.  The Lua equivalent is identical:

**MoonScript:**
```moonscript
class HomePage extends ui.StackView
    apply: => "flex-col w-full gap-2"
    body: =>
        HeroSection ".my-2"
        Transactions limit: 5
```

**Lua equivalent:**
```lua
local HomePage = ui.StackView:extend {
  apply = function(self)
    return "flex-col w-full gap-2"
  end,
  body = function(self)
    self:addChild(HeroSection ".my-2")
    self:addChild(Transactions { limit = 5 })
  end
}
```

In MoonScript, bare function calls inside `body =>` (`HeroSection ".my-2"`)
are implicitly added as children because the `__add` metamethod (`+`) is
defined — but in Lua you **must call `self:addChild(...)` explicitly** (see
[Gotcha: `+` cannot be a bare statement](#gotcha-the--operator-cannot-be-a-bare-statement)).

---

## The `+` Operator

The `+` operator on any Behaviour object is a shorthand for `addChild` that
returns the child:

```lua
local child = parent + ui.Node2D {}
-- equivalent to:
local child = ui.Node2D {}
parent:addChild(child)
```

This is convenient for chaining:

```lua
local inner = outer + ui.Node2D {}
inner:addChild(ui.TextBlock { Text = "nested" })
```

### Gotcha: the `+` operator cannot be a bare statement

In Lua, `parent + child` is an **expression**, not a statement.  The result
**must** be assigned or used:

```lua
-- WRONG — Lua syntax error:
self + ui.Node2D { Name = "Child" }

-- CORRECT — assign the result:
self:addChild(ui.Node2D { Name = "Child" })
-- or:
local child = self + ui.Node2D { Name = "Child" }
```

MoonScript allows bare `+` because it compiles to an assignment.  In Lua,
always use `self:addChild(...)` inside `body` functions.

---

## Async Execution: Why `rebuild()` Needs a Flush in Tests

### How `rebuild()` actually runs

`rebuild()` does **not** run the body function synchronously.  Instead, it:

1. Creates a new Lua coroutine that calls `f_rebuild`.
2. Posts a `kEventResumeCoroutine` message to the platform event queue.
3. Returns immediately.

The coroutine is resumed the next time the event loop processes the queue
(during the next `axPollEvent` + `SV_DispatchMessage` cycle in the main
loop).

This is why, in a running application, `body()` always appears to run
"right away" — but it's actually deferred to the next event cycle.

### Why headless tests need `core.flushQueue()`

In a headless test (run via `orca -test=file.lua`), there is no running
event loop.  The test script runs synchronously from top to bottom.
After calling `rebuild()`, the coroutine is queued but not yet executed.

**You must call `core.flushQueue()` to drain the queue** before making
assertions about children created by `body()`:

```lua
local core = require "orca.core"
local ui   = require "orca.UIKit"

local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
local node   = screen + ui.Node2D {}

node:rebuild(function(self)
  self:addChild(ui.Node2D { Name = "Child" })
end)

core.flushQueue()   -- ← REQUIRED before any child assertions

local n = 0
for _ in node.children do n = n + 1 end
assert(n == 1, "expected 1 child")
```

### Gotcha: variable becomes a number after `flushQueue()` without sentinel

This bug manifests as a Lua variable that was a table suddenly becoming
an integer after `core.flushQueue()`.  The root cause is a stack
imbalance in the `kEventResumeCoroutine` handler inside `ui_handle_event`:
the handler calls `lua_pop(L, 1)` expecting an Event userdata on `L` (put
there by the normal `f_dispatch_message` path), but direct calls from C
do not put that value there.

`core.flushQueue()` is implemented to push a `nil` sentinel before each
`SV_DispatchMessage` call so the pop is always balanced:

```c
static int f_flush_queue(lua_State* L) {
  struct AXmessage msg;
  int top = lua_gettop(L);
  while (axPollEvent(&msg)) {
    lua_pushnil(L);                   // sentinel for lua_pop in handler
    SV_DispatchMessage(L, &msg);
    lua_settop(L, top);               // restore stack unconditionally
  }
  return 0;
}
```

If you ever write a different flush loop without this sentinel, be aware
that any Lua local variable referring to a registry value may silently
receive the wrong value after the pop.

---

## `body()` Lifecycle Summary

| Phase | What happens |
|---|---|
| `MyClass = ui.Node2D:extend { body = fn }` | A new Lua class is created.  `body = fn` stored in the class table. |
| `obj = screen + MyClass {}` | `Behaviour.__init` runs: sets metamethods, calls `orca.core.Object.new`, assigns properties, calls `self:rebuild()`. |
| `self:rebuild()` | Wraps `f_rebuild` in a coroutine, posts `kEventResumeCoroutine`. |
| Next event loop tick | `f_rebuild` runs in the coroutine: calls `OBJ_Clear` (removes existing children), calls `body(self)`. |
| `body(self)` | User code: creates and adds children. |
| After `body` returns | `ID_Node_ViewDidLoad` is posted; if `OF_CLEARBODY` was set, `body` is cleared. |

---

## Property Access Inside `body()`

The body function receives `self` — the same object table.  All properties
set before `rebuild()` are visible inside `body()`:

```lua
local container = screen + ui.Node2D { Width = 200 }

container:rebuild(function(self)
  -- self.Width == 200 here
  for i = 1, math.floor(self.Width / 40) do
    self:addChild(ui.Node2D { Width = 40, Height = 40 })
  end
end)
```

---

## Text Content

When `rebuild(str)` is called with a string instead of a function, the
object's text content is set directly (no coroutine is involved):

```lua
local label = screen + ui.TextBlock {}
label:rebuild("Hello, world!")
-- No flushQueue() needed for string bodies — it's synchronous.

assert(label:getTextContent() == "Hello, world!")
```

**Note:** `label.Text` reads the UIKit `Text` property (a displayed copy
managed by the TextBlock component), while `label:getTextContent()` reads
the raw `TextContent` field set on the native C object.  They may differ
immediately after a `rebuild(str)` call until a layout pass is triggered.

---

## Complete Working Example

```lua
local core = require "orca.core"
local ui   = require "orca.UIKit"

-- ---------------------------------------------------------------------------
-- Define a reusable component
-- ---------------------------------------------------------------------------
local AccountCard = ui.Node2D:extend {
  apply = function(self)
    return "flex-col p-4 rounded"
  end,

  body = function(self)
    self:addChild(ui.TextBlock { Text = self.AccountName or "Account" })
    self:addChild(ui.TextBlock { Text = "$" .. tostring(self.Balance or 0) })
  end
}

-- ---------------------------------------------------------------------------
-- Build the scene
-- ---------------------------------------------------------------------------
local screen = ui.Screen { Width = 800, Height = 600, ResizeMode = "NoResize" }

local accounts = screen + ui.Node2D {}
accounts:rebuild(function(self)
  self:addChild(AccountCard { AccountName = "Checking", Balance = 1200 })
  self:addChild(AccountCard { AccountName = "Savings",  Balance = 8450 })
end)

-- In a test: flush the queue before asserting
core.flushQueue()

local n = 0
for _ in accounts.children do n = n + 1 end
assert(n == 2, "expected 2 account cards")

print("Scene loaded successfully.")
```

---

## Quick Reference

| Pattern | Notes |
|---|---|
| `ui.Foo {}` | Construct with properties |
| `ui.Foo ".cls"` | Construct and apply style class |
| `ui.Foo "#id.cls"` | Construct, set Name, apply style class |
| `ui.Foo(fn)` | Construct with one-shot body function (cleared after first run) |
| `parent + child` | `addChild(child)` shorthand; **must** assign result in Lua |
| `obj:addChild(child)` | Explicit child addition (preferred inside `body`) |
| `ui.Foo:extend { body = fn }` | Subclass with a `body` method that runs on every `rebuild()` |
| `obj:rebuild(fn)` | Set body and schedule async re-population |
| `obj:rebuild("str")` | Set text content synchronously |
| `core.flushQueue()` | In headless tests: drain event queue to execute pending `body()` calls |
| `obj:getTextContent()` | Read raw text content set by `rebuild("str")` |
