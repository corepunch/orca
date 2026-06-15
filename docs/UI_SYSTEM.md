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

## Good follow-up reads

- `docs/styling.md`
- `docs/architecture/message-actions.md`
- `docs/app/xml/triggers-actions-messages.md`
