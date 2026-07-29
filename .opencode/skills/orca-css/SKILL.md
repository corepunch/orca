---
name: orca-css
description: Create, edit, or review ORCA CSS stylesheets, selectors, classes, pseudo-states, theme values, imports, property mappings, and reusable component or page styles. Use for `.css` files and CSS-driven styling of ORCA XML, Lua, or MoonScript views.
---

# ORCA CSS Authoring

## Confirm support

1. Read `docs/UI_SYSTEM.md` and `docs/css-compatibility.md`.
2. Read the relevant target properties under
   `.opencode/generated/xml/<module>/`, usually `UIKit`, `core`, or
   `renderer`.
3. Inspect at most one nearby stylesheet and its consuming view.
4. Generate missing API pages with `make modules`; if generation is
   unavailable, inspect only the owning `.cgen`.

Treat ORCA CSS as a CSS-like subset mapped to ORCA properties. Do not assume a
browser feature exists. Confirm the parser or an existing test before using a
selector, unit, shorthand, or at-rule that is not already established.

## Troubleshoot missing styles

Separate the styling pipeline into selector matching, value parsing, property
mapping, and rendering:

1. Confirm the consuming view loads the stylesheet and applies the expected
   class.
2. Check `docs/css-compatibility.md` for the declaration name, value grammar,
   and supported units. Replace browser-only units and shorthand syntax before
   investigating rendering.
3. Prove parsing and mapping by reading the target property after a recursive
   theme refresh or by extending the nearest focused style test.
4. If the property is set but no pixels change, inspect the owning control's
   draw handler and one renderer call site. A mapping in `Css.c` proves only
   that the value reaches the object; it does not prove the visual path exists.
5. Keep parser/property coverage separate from display-backed rendering
   coverage so a parse-only test cannot mask an unimplemented draw path.

## Author maintainable styles

1. Use semantic classes such as `.card`, `.toolbar`, and `.primary-button`.
2. Keep declarations for a selector together. Group selectors that share the
   same declarations.
3. Prefer shallow class selectors; use type, ID, descendant, direct-parent,
   compound, and pseudo-state selectors only when their extra specificity is
   needed.
4. Compose shared declarations with the established `@apply` syntax rather
   than cloning blocks.
5. Split larger apps into a small manifest with relative `@import` entries,
   then coarse `base`, `layout`, `typography`, `components`, and `pages`
   files.
6. Apply styles with XML/Lua `class` or `StyleClass`. Use `$orca-xml-authoring`
   for view structure and `$orca-lua-authoring` for runtime class changes.

```css
@import "components/buttons.css";

.primary-button {
  @apply: .button;
  background-color: $accent;
  color: $text-primary;
}

.primary-button:hover {
  opacity: 0.8;
}
```

Use existing project theme-value syntax; do not introduce browser custom
properties unless the local parser and tests explicitly support them.

## Respect ORCA semantics

- Preserve selector and pseudo-state case as required by the existing
  stylesheet.
- Use only supported pseudo-states, commonly `hover`, `focus`, `active`, and
  `dark`.
- Map declaration names to actual ORCA properties and value types. Unsupported
  declarations may be ignored rather than reported.
- Prefer data bindings over CSS classes when a layout value represents data,
  such as per-item indentation or visibility.
- Use raw Lucide SVG with `stroke="currentColor"` and
  `-orca-tint-mode: template` for tintable icons.

## Validate

Load the smallest view using the stylesheet and trigger a recursive theme
refresh when the test or runtime path requires it. Verify base, hover/focus,
inheritance/composition, imports, and one representative property mapping.
Run the narrowest style or Lua style test before broader UI tests.
