---
name: orca-controller-binding
description: Connect ORCA XML views to Lua controllers and data using `Controller`, named event handlers, `DataContext`, property bindings, relative object paths, and binding modes. Use for XML code-behind, XML-to-Lua events, dynamic view data, and controller or binding diagnostics.
---

# ORCA Controller Binding

## Establish the contract

1. Read `docs/LUA_API.md`, `docs/UI_SYSTEM.md`, and
   `docs/app/xml/triggers-actions-messages.md`.
2. Read the relevant class, event, and property pages under
   `.opencode/generated/{xml,lua}/<module>/`.
3. Inspect at most one nearby controller-backed view; use
   `samples/Adventure/Pages/Play.xml` and
   `samples/Adventure/Scripts/Play.lua` for the basic pattern.
4. Generate missing API pages with `make modules`; if generation is
   unavailable, inspect only the owning `.cgen`.

## Bind a controller

Set `Controller` on the XML root to the project module path:

```xml
<Page Name="PlayPage" Controller="Adventure/Scripts/Play">
  <Input Name="CommandInput" Submit="HandleSubmit"/>
</Page>
```

Return a table from that Lua file:

```lua
local Controller = {
  HandleSubmit = function(self, args, sender)
    local output = self.view:findChild("Output", true)
    -- Handle the event with generated APIs.
  end,
}

return Controller
```

- Treat `self` as the controller table and `self.view` as the XML root.
- Use the handler signature established by the event bridge:
  `function(self, args, sender)`.
- Reference a controller function by name from an event attribute, such as
  `Submit="HandleSubmit"`.
- Name a controller function `<ObjectName>_<EventName>` to auto-wire it to
  that named descendant's event. A function named only `<EventName>` targets
  the root event.
- Resolve descendants with the generated Lua method name and cache them only
  for a lifecycle that cannot outlive the view.

## Bind data and properties

- Use `{Binding DataContext/Field}` to resolve data from the nearest
  `DataContext` and walk the requested path.
- Use relative object paths for peer properties, such as
  `{Binding ../Footer/SelectedValue}`.
- Use the exact generated destination property. Qualify it when needed, such
  as `Node.PaddingLeft`.
- Use `OneWay` for display, `TwoWay` for synchronized editing, and
  `OneWayToSource` only when the control should write without reading.
- Use an explicit `BindingExpression` or `attachPropertyProgram` only when
  attribute shorthand cannot express the required program.
- Keep simple show/hide, message dispatch, and navigation reactions in XML
  actions. Use the controller for computation, validation, runtime state, or
  data mutation.

## Diagnose in layers

Verify these independently:

1. The controller path resolves and the chunk returns a table.
2. The event name is a generated event property and the function is found.
3. `self.view` and any named descendant resolve.
4. The binding source path resolves through the expected `DataContext` or
   object hierarchy.
5. The destination property accepts the produced type and the selected mode
   propagates in the intended direction.

Test view loading, controller invocation, one event payload, and one binding
update with the narrowest headless test.
