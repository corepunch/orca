`check.svg` reuses Chat's heavy Lucide check (stroke width 3.5), licensed under
the ISC license. Its provenance and license are recorded in
[`apps/chat/assets/icons/README.md`](../../../apps/chat/assets/icons/README.md).

`selection-controls.css` provides the default selection-control skin. Normal
startup registers it globally before application styles through
`share/plugins/selection-controls.lua`. Applications can replace the SVG with
`-orca-source: url("MyApp/Icons/check.svg")` on the `control-check` part.

See [Selection controls](../../../docs/selection-controls.md) for choosing
between checkboxes, radio groups and switches, XML/Lua examples, binding and
event semantics, CSS parts, sizing and compatibility. The native
[Controls gallery](../../../apps/Controls/README.md) shows the shared skin in
dark and light settings.
