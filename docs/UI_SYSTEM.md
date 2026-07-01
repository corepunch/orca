# UI System

Use this file when changing UIKit controls, layout, XML UI, popups, or styling behavior.

## Main paths

- `plugins/UIKit/UIKit.cgen`
- `plugins/UIKit/*.c`
- `docs/styling.md`
- `docs/app/xml/triggers-actions-messages.md`
- `tests/test_layout.lua`
- `tests/test_styles_lua.lua`

## Core rules

- UIKit is built on the same object/component system as the rest of ORCA.
- If you add a control, property, or message, follow `.cgen -> make modules -> C handlers -> tests`.
- Use messages and properties for cross-system behavior; do not make core depend on UIKit headers.

## Layout and test rule

For tests with explicit dimensions, always set `ResizeMode="NoResize"` in XML or `ResizeMode = "NoResize"` in Lua.
Otherwise the window size can override the intended test dimensions.

## XML message shorthand

Common patterns:

- `{Screen.ShowModal Path=...}`
- `{Popup.ClosePopup}`
- `{Node.RightButtonUp}`

Use the shorthand when it is clearer than a full trigger/action block.

## Popup rules

- Prefer `Popup` roots for modal content.
- `Screen.ShowModal` should point to the popup asset path.
- Use `Popup.ClosePopup` to dismiss and optionally return data.

## Data-driven layout pattern

Separate visual data from layout structure by driving layout properties through data bindings rather than CSS classes or hardcoded attributes.

### Anti-pattern: layout encodes data

Before — each indent level required a distinct CSS class:

```xml
<!-- Screen layout — six individual placeholders, each with a CSS class for indent -->
<LayerPrefabPlaceholder Name="Line1" class="model-indent-root" .../>
<LayerPrefabPlaceholder Name="Line2" class="model-indent-child" .../>
<LayerPrefabPlaceholder Name="Line3" class="model-indent-grandchild" .../>
```
```css
.model-indent-root   { padding-left: 0; }
.model-indent-child  { padding-left: 16; }
.model-indent-grandchild { padding-left: 32; }
```

### Pattern: data drives layout

After — `GridBox` + `ItemsSource` + `ItemTemplate`, indent lives in data:

```xml
<!-- Screen layout — single GridBox, no CSS indent classes -->
<GridBox Columns="auto"
  ItemsSource="Example/Data/ApplicationData:XmlModel"
  ItemTemplate="Example/Prefabs/XmlModelNode"/>
```

```xml
<!-- Prefab binds layout property to data -->
<StackView Name="XmlModelNode" Node.PaddingLeft="{Binding DataContext/Indent}">
  <ImageView Source="{Binding DataContext/Icon}"/>
  <TextBlock Text="{Binding DataContext/Title}"/>
</StackView>
```

```xml
<!-- Data file — each item declares its own indent -->
<DataObject Name="XmlModel">
  <DataObject Name="Line1">
    <DataObjectString Name="Indent" Value="0"/>
    ...
  </DataObject>
  <DataObject Name="Line2">
    <DataObjectString Name="Indent" Value="16"/>
    ...
  </DataObject>
</DataObject>
```

### When to use

- **Lists of items** where each item varies in layout properties (padding, color, visibility).
- **Tree or nested structures** where depth is a property of data, not of layout.
- **Data that may change** — binding keeps the layout static and lets data drive appearance.

### When to keep explicit layout

- **Unique, non-repeating sections** (hero, header, footer) — the layout *is* the structure, not a template.
- **Fixed-position overlays** where placement is absolute and unrelated to data.

## Good follow-up reads

- `docs/styling.md`
- `docs/architecture/message-actions.md`
- `docs/app/xml/triggers-actions-messages.md`
