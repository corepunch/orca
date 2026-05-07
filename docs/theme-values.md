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

Theme keys are resolved by `FS_GetThemeValue()` during XML load and by helpers like
`FS_GetThemeColor()` in component startup code.

The practical rule is simple:

1. Define the key in your project `ThemeLibrary`.
2. Reference it from XML as `$your-key`.
3. Let the component fall back to its built-in default if the key is absent.

## Common UIKit Theme Keys

These are the default keys used by UIKit controls and the easiest ones to override in a theme:

| Key | Used by | Purpose |
|---|---|---|
| `accent` | Tabs, selected states, emphasis text | Main accent color |
| `accent-background` | Tabs, buttons, tab bar underline | Filled accent surfaces |
| `accent-foreground` | Buttons and accent surfaces | Text/icon color on accent backgrounds |
| `control-background` | Buttons, tabs, general controls | Default control fill |
| `control-foreground` | Buttons and control labels | Default foreground text/icon color |
| `control-border` | Controls that draw borders | Subtle border color |
| `control-muted` | Muted control states | Disabled or secondary control color |
| `card-background` | Card-style containers | Neutral card background |

## Default Colors In Tabs And Buttons

If you only want to override a few colors, start here:

### Tabs

Tabs use these theme keys by default:

- Selected tab background: `accent-background`, then `accent`
- Unselected tab background: `control-background`, then `card-background`
- Tab text foreground: whatever the `Tab` component style resolves for text, with selected state using the accent palette
- Tab bar underline: `accent-background`, then `accent`

### Buttons

Buttons use these theme keys by default:

- Button fill: `control-background`, then `accent-background`, then `accent`
- Button text foreground: `control-foreground`, then `accent-foreground`
- Button border and muted states: `control-border` and `control-muted`

That means you can make buttons and tabs feel consistent across the whole app by changing just a few theme entries.

## Example Theme Override

Here is a small theme block that gives you a darker control surface and a brighter accent:

```lua
ThemeLibrary = {
  { Key = "accent",             Value = "#7C3AED" },
  { Key = "accent-background",  Value = "#2C1D4A" },
  { Key = "accent-foreground",  Value = "#FFFFFF" },
  { Key = "control-background", Value = "#181824" },
  { Key = "control-foreground", Value = "#F8FAFC" },
  { Key = "control-border",     Value = "#30344A" },
  { Key = "control-muted",      Value = "#677089" },
}
```

With those values in place, tabs, buttons, and other UIKit controls will immediately pick up the new palette.

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
[samples/Example/package.lua](../samples/Example/package.lua),
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
