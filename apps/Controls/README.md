# Selection controls

A deterministic native XML/CSS gallery at 780 × 844 points, with two 390-point
panels. It presents the same checkbox, radio-button, and switch states on black
and white surfaces. Labels and indicator geometry belong to the shared controls;
the app owns its typography, spacing, and palette.

Run `build/bin/orca apps/Controls` after `make unite`. The screen is
`Controls/Screens/Selection`; `ResizeMode="NoResize"` keeps the comparison stable
for native screenshots. Its shared control stylesheet is loaded by normal
startup, so the gallery stylesheet only supplies the app-specific appearance.

Tap the labels or indicators to toggle the enabled controls. Radio buttons are
exclusive within each panel. Disabled rows should retain their displayed state.
Use Tab and Space to inspect keyboard focus and activation. A native frame is
needed to verify the SVG mark, borders, and thumb; headless layout alone does not
verify their appearance.
