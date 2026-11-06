# Theme Values

ORCA components resolve many visual defaults through named theme values.
That gives you one place to tune the look of a whole app without touching every screen.

Theme values are just key/value pairs in your project package, usually defined in `package.lua`:

```lua
ThemeLibrary = {
  { Key = "accent",             Value = "#8B5CF6" },
  { Key = "accent-background",  Value = "#2A2145" },
  { Key = "accent-foreground",  Value = "#FFFFFF" },
  { Key = "control-background", Value = "#141420" },
  { Key = "control-foreground", Value = "#F1F5F9" },
  { Key = "control-border",     Value = "#2B3142" },
  { Key = "control-muted",      Value = "#64748B" },
}
```

Once a key exists, you can use it from XML with the `$key-name` syntax:

```xml
<StackView BackgroundColor="$control-background">
  <TextBlock Text="Hello" ForegroundColor="$control-foreground"/>
</StackView>
```

If a component asks for a theme key that does not exist, ORCA now prints a warning to stderr.
That makes missing theme entries much easier to spot during development.

## How Theme Lookup Works

Theme keys are resolved by `FS_GetThemeValue()` during XML load and by CSS
`var(--key)` while a stylesheet is parsed. `FS_FindThemeValue()` performs the
same lookup without reporting a missing key.

The practical rule is simple:

1. Define the key in your project `ThemeLibrary`.
2. Reference it from XML as `$your-key`.
3. Reference it from CSS as `var(--your-key, fallback)`.

## Default Stylesheet

UIKit controls have no colors, borders or padding built into C. Their look comes
from `share/orca/UIKit/default.css`, the engine's equivalent of a browser's
user-agent stylesheet. Normal startup registers it (through
`share/plugins/default-styles.lua`) before any application sheet, so an
application rule for the same property always wins, exactly like loading
several stylesheets one after another:

```css
/* Your app sheet: only what differs from the default */
.danger { background-color: #c0392b; }
Button { border-radius: 0; }
```

The default sheet covers `Button`, `Input`, `TabBar`/`Tab`, `CheckBox`,
`RadioButton` and `ToggleSwitch`, including `:hover`, `:active`,
`:selected`, `:checked`, `:focus`, `:focus-visible` and `:disabled`. It follows
the system appearance (`:dark`) unless the project sets its palette.

### Palette keys

Each color in the default sheet reads an optional `ui-*` theme key and falls
back to a neutral light or dark value. Define the keys in `package.lua` to
retheme every control at once:

| Key | Used for |
|---|---|
| `ui-accent`, `ui-accent-foreground` | Checked boxes, radios, switches, focus rings, input focus |
| `ui-control`, `ui-control-hover`, `ui-control-active` | Button fill and its states |
| `ui-control-foreground`, `ui-control-muted` | Control text |
| `ui-input`, `ui-input-border`, `ui-input-border-hover` | Input and indicator surfaces |
| `ui-separator` | Separator lines |
| `ui-radius`, `ui-radius-small` | Corner radii (buttons/inputs, checkboxes) |

Tabs and the tab bar underline fall back to the project `accent` key before
the sheet's own blue. `apps/Editor/package.lua` is a complete dark palette.

`var(--key, fallback)` resolves when the sheet is parsed and accepts nested
fallbacks. With a fallback, a missing key is not reported.

## Example XML

```xml
<StackView Direction="Vertical" Spacing="12" BackgroundColor="$surface" Padding="20">
  <TabView Name="EditorTabs" SelectedValue="xml">
    <Tab Name="XmlTab" Value="xml">XML</Tab>
    <Tab Name="LuaTab" Value="lua">Lua</Tab>
  </TabView>

  <Button Text="Build Preview"/>
</StackView>
```

This stays readable because the colors live in the theme, not in the layout itself.

## Example Application Theme

The Example app in this repository defines its own theme keys in
[apps/Example/package.lua](../apps/Example/package.lua),
including:

- `panel-background`
- `surface`
- `surface-deep`
- `card-background`
- `text-primary`
- `text-secondary`
- `accent`
- `accent-green`
- `accent-blue`
- `accent-amber`

Those are used throughout the sample screens and can be customized the same way in your own project.
