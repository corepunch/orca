`check.svg` reuses Chat's heavy Lucide check (stroke width 3.5), licensed under
the ISC license. Its provenance and license are recorded in
[`apps/chat/assets/icons/README.md`](../../../../apps/chat/assets/icons/README.md).

`selection-controls.css` provides the default selection-control skin. Normal
startup registers it globally before application styles through
`share/plugins/selection-controls.lua`. Applications can replace the SVG with
`-orca-source: url("MyApp/Icons/check.svg")` on the `control-check` part.
