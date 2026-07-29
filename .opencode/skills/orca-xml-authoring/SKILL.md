---
name: orca-xml-authoring
description: Create or edit ORCA XML screens, pages, popups, prefabs, package declarations, object trees, properties, bindings, triggers, actions, and messages. Use for declarative UI markup and XML serialization work; use the asset-and-scene skill for resource libraries and scene content.
---

# ORCA XML Authoring

## Gather the contract

1. Read `docs/UI_SYSTEM.md`.
2. Read `docs/app/xml/triggers-actions-messages.md` when handling events,
   actions, popups, or navigation.
3. Read the relevant element pages under
   `.opencode/generated/xml/<module>/`. Generate them with `make modules` when
   absent; if generation is unavailable, inspect only the owning `.cgen`.
4. Inspect at most one nearby screen, page, popup, or prefab using the same
   pattern.

Treat generated XML documentation as the authority for element names,
properties, value types, inheritance, arrays, and messages. Do not infer XAML
or browser behavior.

## Author the view

1. Choose the semantic root:
   - Use `Screen` for a top-level window view.
   - Use `Page` for content hosted by page or navigation controls.
   - Use `Popup` for modal content.
   - Use the appropriate reusable node root for a prefab.
2. Keep hierarchy, static properties, bindings, and simple reactions in XML.
3. Apply `Name` only when lookup, object paths, tests, or tooling need it.
4. Apply reusable presentation with `class`/`StyleClass`; use `$orca-css` to
   create or change stylesheets.
5. Bind repeated layout to data with `ItemsSource`, `ItemTemplate`, and
   `{Binding DataContext/...}` instead of duplicating nodes.
6. Use exact generated property names. Qualify inherited properties when it
   removes ambiguity, such as `Node.PaddingLeft`.

## Wire behavior

Use inline shorthand for one clear action:

```xml
<Button Text="Open"
  LeftButtonUp="{Screen.ShowModal Path=App/Screens/SettingsPopup}"/>
```

Use `Node.Triggers` when a trigger has multiple actions, watches another
property, or needs explicit dispatch configuration:

```xml
<Button Text="Hide">
  <Node.Triggers>
    <EventTrigger RoutedEvent="Node.LeftButtonUp">
      <HideAction Path="../Panel"/>
    </EventTrigger>
  </Node.Triggers>
</Button>
```

- Map action attributes exactly to generated message payload fields.
- Omit `Target` to route a generated message through ancestors; set it to
  dispatch to a resolved object path.
- Use `Mode="Post"` when synchronous dispatch would mutate the tree during the
  current event.
- Open modal content with `Screen.ShowModal` and close a `Popup` with
  `Popup.ClosePopup`.
- Use `$orca-controller-binding` when behavior needs computation, runtime
  state, or data mutation.

## Validate

Load the exact XML asset or run the narrowest XML/headless test. For a fixed
test viewport, set `ResizeMode="NoResize"`. Verify the root type, resource
paths, object paths, bindings, and at least one event path before broad tests.

Use `$orca-xml-assets-scenes` for images, textures, shaders, font families,
SceneKit, or SpriteKit declarations.
