# App styling

ORCA uses a CSS-like stylesheet system mapped to object properties. XML and
Lua construct the same UIKit objects and use the same styling pipeline.
This page separates supported behavior from recommended app conventions.
Check the [CSS compatibility reference](../css-compatibility.md) before using
a browser declaration, unit, selector, or shorthand.

## Theme and style ownership

**Supported:** package `ThemeLibrary` entries define named theme values.
`apps/Example/package.lua` uses semantic roles such as `surface`,
`card-background`, `text-primary`, `text-secondary`, and `accent`:

```lua
ThemeLibrary = {
  { Key = "surface", Value = "#0B0B12" },
  { Key = "text-primary", Value = "#F1F5F9" },
  { Key = "accent", Value = "#8B5CF6" },
}
```

The Example stylesheet uses `var(--accent)` to resolve these theme keys;
ORCA's `$accent` notation is also used by property values. A `ThemeLibrary`
key is not a browser custom-property declaration or a full CSS variables API.

**Convention:** define colors by their role, including surfaces, text,
borders, primary actions, and feedback. Keep palette literals in the theme
rather than repeating hex values throughout views. Add a role when its
meaning differs, even when it currently shares another role's color.

Keep presentation in the stylesheet owned by the relevant component or page.
Keep object structure in XML/Lua and state transitions in controllers or app
logic. Avoid giving one property competing owners in CSS, bindings, and Lua.
Use explicit dimensions in code when they express actual structure or data;
do not move every visual constant into runtime construction code.

## Stylesheet organization

**Supported:** load a stylesheet through `StyleSheet` on the consuming screen
or node, and assign classes through XML/Lua `class` or `StyleClass`. Imports
are relative to their stylesheet. `apps/Example/Styles/app.css` is a small
manifest importing base, layout, typography, component, and page styles.

```css
@import "components/buttons.css";

.primary-button {
  @apply: .button;
  background-color: var(--accent);
  color: var(--accent-foreground);
}
```

Here `.button` and the referenced tokens must exist in the app. `@apply:` is
ORCA's composition syntax. Do not substitute browser framework syntax.

**Convention:** start with one sheet for a small app; split by responsibility
when it becomes hard to navigate. Prefer shallow, semantic classes such as
`.toolbar`, `.message-bubble`, and `.primary-button`. Keep a selector's
declarations together. Use descendant, ID, or compound selectors only when
their extra scope is useful; avoid coupling styles to incidental tree depth.
Shared component appearance belongs with the component; page-specific
composition belongs with the page. XML and Lua need no separate CSS dialects.

## Layout and sizing

**Supported:** ordinary numeric CSS values use bare numbers, such as
`height: 44; gap: 12; padding: 10 16;`. Browser `px`, `rem`, `vh`, and general
percentages are not converted. `width: 100%` is a special stretch shorthand,
not general percentage sizing. CSS edge shorthands use top/right/bottom/left
ordering; `padding: 10 16` sets vertical padding to 10 and horizontal to 16.

`width: auto` uses ORCA's automatic width and stretches when finite layout
space permits it, unless auto margins request content-sized alignment.
`margin-left: auto; margin-right: auto;` centers horizontally;
`margin-left: auto;` aligns to the trailing edge. The corresponding vertical
auto margins center or align vertically. These are layout rules, not
instructions to estimate text width or position children manually.

Use `StackView` for a linear row or column. Its `Direction`, `Spacing`,
`AlignItems`, and `JustifyContent` describe that arrangement. Use `Grid` when
the screen needs explicit tracks, such as `Rows="auto 1fr auto"` for a header,
the remaining content, and a composer. Give remaining-space tracks a finite parent
extent. `Rows` and `Columns` have their own track grammar; `1fr` there does
not make browser flex declarations valid in CSS.

**Track sizes** follow WPF row and column definitions. A pixel track (`48px`,
or a bare `48`) keeps its size. An `auto` track fits its largest child,
margins included. A star track (`1fr`, `*`, `2*`) shares by weight whatever
space the pixel and `auto` tracks and `Spacing` leave, and never becomes
negative. In a 600-point grid, `Rows="auto 1fr auto"` with a 50-point header
and a 70-point footer gives 50, 480 and 70. When the space is unbounded, as for
a grid in a vertical `StackView`, star tracks request their content and then
share the size they are arranged at. Tracks that `Grid` generates when `Rows`
or `Columns` is omitted are `auto`, and a single generated row fills the
grid. Use `1fr` for equal columns; `auto auto auto` sizes each column to its
content.

**Box model:** `Width` and `Height` size the padding box; a border is drawn
outside it and layout counts it as margin. A control with `Height="48"` and
`BorderWidth="2"` therefore occupies 52 points, while the same control without
a border occupies 48. To keep bordered and borderless controls on one grid,
subtract the border from the bordered control's height. This is CSS
content-box behaviour, not XAML's, where `Height` includes the border; it is
kept for compatibility and may change. Children with no explicit height take
the track size of a `Grid`, border included.

**Thickness order:** in XML attributes and Lua properties, `Margin`,
`Padding` and `BorderWidth` use XAML order: `"12"`, `"horizontal vertical"`,
or `"left top right bottom"`. `Padding="12 0"` is 12 on the left and right.
CSS declarations keep CSS order, so `padding: 12 0` is 12 at the top and
bottom.

**Convention:** let content determine intrinsic sizes and use a small spacing
scale consistently. Reserve fixed sizing for intentional constraints such as
icons and control heights. Prefer container layout over coordinate offsets.
Test narrow and resized windows before depending on one screenshot's bounds.

For repeating items, bind layout values that represent data rather than
creating `.indent-1`, `.indent-2`, and similar classes:

```xml
<StackView Node.PaddingLeft="{Binding DataContext/Indent}">
  <TextBlock Text="{Binding DataContext/Title}"/>
</StackView>
```

This uses each item's data context. Keep shared spacing, typography, and
colors in CSS. See [data-driven layout](../UI_SYSTEM.md#data-driven-layout-pattern).

## Icons, typography, and interaction

Use raw Lucide SVGs with `stroke="currentColor"`. `VectorView` rasterizes an
SVG as a template alpha mask; inherited `Foreground`, including CSS `color`,
tints it. Set its view size explicitly and preserve the source proportions.
`ForegroundHint` has `None`, `Translucent`, and `Opaque` values; it does not
select template rendering. Do not add an unsupported `-orca-tint-mode` CSS
declaration to make a `VectorView` tint.

**Convention:** use a compact type hierarchy with distinct body, title, and
secondary text roles. Select font families actually registered or supported
by the app. Preserve readable contrast on every surface and state, including
muted text and placeholders. Color alone should not identify errors or unread
items. Use the supported `hover`, `focus`, and `active` states where relevant,
and make keyboard focus visible. For touch-oriented screens, aim for roughly
44-unit control targets; icon artwork can be smaller inside them. This is
design guidance, not a universal engine minimum for every desktop control.

## State and native verification

Give loading, empty, and error states meaningful content: explain what is
happening, distinguish no results from no data, and offer an applicable retry
or next step. Preserve useful content while refreshing when practical.
Every visible action should perform its stated behavior. Label demo content
and unfinished capabilities honestly; do not invent successful operations,
messages, online status, or counters to make a screen appear complete.

Verify in the native ORCA renderer at the intended window size. Inspect text
wrapping, clipping, alignment, icon tint, focus, and actual control behavior,
including loading and failure paths. For deterministic fixed-size tests use
`ResizeMode="NoResize"`; refresh styles recursively when the test requires it.
Property assertions prove matching, parsing, and mapping, but do not prove a
draw path renders correctly. Pair focused style tests with native visual
inspection. See [UI rules](../UI_SYSTEM.md) and [testing](../TESTING.md).
