`check.svg` reuses Chat's heavy Lucide check (stroke width 3.5), licensed under
the ISC license. Its provenance and license are recorded in
[`apps/chat/assets/icons/README.md`](../../../apps/chat/assets/icons/README.md).

`default.css` is the engine's user-agent stylesheet: the default look of
buttons, inputs, tabs and selection controls. Normal startup registers it
globally before application styles through `share/plugins/default-styles.lua`;
application rules override it property by property. Applications can replace the SVG with
`-orca-source: url("MyApp/Icons/check.svg")` on the `control-check` part.

See [Selection controls](../../../docs/selection-controls.md) for choosing
between checkboxes, radio groups and switches, XML/Lua examples, binding and
event semantics, CSS parts, sizing and compatibility. The native
[Controls gallery](../../../apps/Controls/README.md) shows the shared skin in
dark and light settings.

`pivot-back-ring.svg` (30 points) is the 2-point outline around the `Pivot`
back glyph, drawn as a `VectorView` rasterized with antialiasing at device
scale and tinted by the inherited `Foreground`.
`share/plugins/metro-rings.lua` registers it as a global default; replace it
with `-orca-source` on the `pivot-back` part. `ApplicationBar` buttons draw
no circle: each is a plain 28-point glyph.
