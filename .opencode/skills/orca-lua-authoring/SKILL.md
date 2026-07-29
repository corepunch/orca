---
name: orca-lua-authoring
description: Write or refactor ORCA application code in Lua or MoonScript, including modules, object construction, reusable components, scene composition, lifecycle, routing, messages, and project script loading. Use for application behavior implemented above the C engine API.
---

# ORCA Lua Authoring

## Gather the API

1. Read `docs/LUA_API.md`.
2. Read the relevant pages under `.opencode/generated/lua/<module>/`.
3. Read `docs/UI_SYSTEM.md` for UI, layout, popup, or navigation behavior.
4. Inspect at most one nearby Lua or MoonScript module using the same pattern.

Generate missing API pages with `make modules`. If generation is unavailable,
inspect only the owning `.cgen`. Use `$orca-lua-exposure` when a required API
is not exported instead of guessing a binding.

## Load modules

- Require built-in modules explicitly when initialization order matters, such
  as `require "orca.core"` or `require "orca.UIKit"`.
- Add project script roots through the package's `ProjectReferences`; require
  application modules by the path convention used by that project.
- Keep project/package declarations declarative. Put runtime behavior in Lua
  or MoonScript modules.
- Preserve the surrounding language and module convention.

## Build objects and scenes

Use generated class names, properties, and camelCase method exports:

```lua
local ui = require "orca.UIKit"

local Card = ui.StackView:extend {
  body = function(self)
    self:addChild(ui.TextBlock { Text = self.Title })
  end,
}

return Card
```

- Construct objects with property tables.
- Compose children with `addChild`. Use `local child = parent + object` only
  when the returned child is consumed; bare `parent + child` is invalid Lua.
- Define reusable components with the local `extend`/class pattern and a
  `body` method.
- Use `rebuild` for deferred reconstruction. Flush the event queue in
  headless tests before asserting rebuilt children.
- Prefer messages, properties, and bindings for communication across objects
  or subsystems.
- Preserve object ownership and lifecycle; do not retain stale scene objects
  across rebuild or removal.

## Keep boundaries clear

- Use `$orca-controller-binding` when an XML view owns structure and Lua
  supplies event behavior.
- Use `$orca-xml-authoring` for declarative screens and prefabs.
- Use `$orca-xml-assets-scenes` for packaged images, textures, shaders, font
  families, SceneKit, or SpriteKit assets.
- Use generated names rather than PascalCase `.cgen` method names; codegen
  normally exports `FindChild` as `findChild`.

## Validate

Add or update the narrowest Lua/headless test. Exercise module loading, object
construction, one relevant property or message, and any deferred rebuild or
queue behavior. For fixed-size UI tests, set `ResizeMode = "NoResize"`.
