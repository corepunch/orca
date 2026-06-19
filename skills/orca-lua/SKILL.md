---
name: orca-lua
description: >
  Lua API reference for ORCA. Covers module system, scene building,
  MoonScript patterns, and generated Lua bindings. Use when writing
  Lua scripts, building UIs from Lua, or working with ORCA's Lua API.
---

# ORCA Lua API

A framework for building UI applications with Lua scripting.
Components are created from Lua using generated bindings and composed into scene trees.

## Getting Started

### Create a new project

```sh
orca new MyProject
```

Creates:
```
MyProject/
├── package.xml        # Project config with startup screen
├── Screens/
│   └── Main.xml       # Hello World screen
└── Images/
```

### Run it

```sh
make run PROJECT=MyProject/package.xml
```

### Add Lua scripts

Create `Scripts/` in your project and add a `ProjectReference`:

```xml
<ProjectReferenceLibrary>
  <ProjectReferenceItem Name="Scripts">Scripts</ProjectReferenceItem>
</ProjectReferenceLibrary>
```

Then in Lua:

```lua
local ui = require "orca.UIKit"
local MyComponent = ui.Node2D:extend {
  body = function(self)
    self:addChild(ui.TextBlock { Text = "Hello from Lua!" })
  end
}
return MyComponent
```

## Principles

1. **Scene tree = component tree.** UIs are built by composing objects into a parent/child hierarchy.
2. **`rebuild()` populates children.** Use `rebuild(function(self) ... end)` to define an object's children.
3. **Properties at construction.** Pass properties as a table: `ui.Button { Name = "OK", Width = 100 }`.
4. **Style classes for theming.** Apply CSS-like classes: `ui.Button ".primary"`.
5. **Messages for communication.** Objects communicate via messages, not direct method calls.

## Critical Rules

### Module System → [rules/lua-scripting.md](./rules/lua-scripting.md)

- **Built-in modules:** `require "orca.core"`, `require "orca.UIKit"`, etc.
- **Auto-require shorthand:** `orca.geometry` auto-requires on first access.
- **Plugin loading:** Native `.so`/`.dll` in `LIBDIR`, Lua plugins in `SHAREDIR/plugins/`.
- **Global paths:** `LIBDIR`, `SHAREDIR`, `PLUGDIR`, `PROJECTDIR`.

### Scene Building → [rules/scenes-and-rebuild.md](./rules/scenes-and-rebuild.md)

- **`rebuild(fn)`** — Stores `fn` as the object's `body` and runs it.
- **Table constructor:** `ui.Node2D { Name = "Box", Width = 100 }`.
- **Class shorthand:** `ui.Node2D ".rounded.shadow"` applies style classes.
- **`+` operator:** `parent + child` adds child to parent.
- **`body` method:** Define `body = function(self)` in a class for automatic construction.

### Link Navigation → [rules/link-navigation.md](./rules/link-navigation.md)

- **`Link` component** wraps navigation with XML-defined routes.
- **Router pattern:** Define routes in XML, handle in Lua.

## Key Patterns

```lua
local ui = require "orca.UIKit"

-- Basic object creation
local btn = ui.Button {
    Name = "OK",
    Width = 100,
    Height = 32,
    Text = "Click me"
}

-- With style classes
local btn = ui.Button(".primary", { Text = "Submit" })

-- Scene tree construction
local screen = ui.Node2D {}
screen:rebuild(function(self)
    self:addChild(ui.Node2D { Name = "Header" })
    self:addChild(ui.Node2D { Name = "Content" })
end)

-- Class definition with body
local MyWidget = ui.Node2D:extend {
    body = function(self)
        self:addChild(ui.TextBlock { Text = "Hello" })
        self:addChild(ui.Button { Text = "OK" })
    end
}

-- Event handling
btn.OnClick = function(self)
    print("Button clicked!")
end

-- Property access
local w = btn.Width
btn.Width = 200
```

## Component Selection

| Need | Use |
|------|-----|
| Basic container | `Node2D` |
| Text display | `TextBlock` |
| Button | `Button` |
| Input | `TextField` |
| Image | `Image` |
| List | `ListView` |
| Popup | `Popup` via `Screen.ShowModal` |
| Layout | `StackPanel`, `Grid` |

## Quick Reference

```lua
-- Require modules
local core = require "orca.core"
local ui = require "orca.UIKit"
local geo = require "orca.geometry"

-- Object creation
local obj = ui.ClassName { Prop = value }
local obj = ui.ClassName(".class", { Prop = value })

-- Hierarchy
parent:addChild(child)
child:removeFromParent()

-- Properties
obj.Prop = value
local v = obj.Prop

-- Messages
obj:sendMessage(MSG_NAME, w, l)

-- Style classes
obj:addClass("class-name")
obj:removeClass("class-name")
obj:toggleClass("class-name")
```

## Detailed References

- [rules/lua-scripting.md](./rules/lua-scripting.md) — Module system, globals, plugin loading
- [rules/scenes-and-rebuild.md](./rules/scenes-and-rebuild.md) — Scene tree construction, rebuild pattern
- [rules/link-navigation.md](./rules/link-navigation.md) — Link component and navigation
