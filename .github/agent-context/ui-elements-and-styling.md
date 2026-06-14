# UI Elements and Styling

Read this when changing UIKit controls, XML screens, layout behavior, styles,
Tailwind-style classes, modals, popups, or UI tests.

## Start Here

- `plugins/UIKit/UIKit.cgen`
- `plugins/UIKit/Button.c`
- `plugins/UIKit/UIKit_message.c`
- `docs/styling.md`
- `samples/Example/Screens/GetStartedPopup.xml`
- `tests/test_styles_lua.lua`

## Rules

- UI clipping uses `Overflow`, `overflow-x`, and `overflow-y` as the source of
  truth. Do not introduce or rely on `ClipChildren`.
- UIKit components are still object/components. If you add a control or message,
  follow the `.cgen -> make modules -> HANDLER -> tests` workflow.
- Use XML message shorthand when it keeps markup clear:
  `{Node.RightButtonUp}`, `{Popup.ClosePopup}`, `{Screen.ShowModal Path=...}`.
- Core code must not include UIKit headers. Shared behavior should flow through
  object/component messages or generic core interfaces.
- For test screens with explicit dimensions, set `ResizeMode="NoResize"` in XML
  or `ResizeMode = "NoResize"` in Lua. The default resizable mode can collapse
  tests to the virtual framebuffer size.

## Popups

- Modal popups should be `Popup` objects, not `Screen` placeholders.
- `{Screen.ShowModal Path=...}` points to the popup template asset, not a scene
  tree path.
- Popup roots usually should not set `Width` or `Height`; let the full-screen
  overlay/background provide the bounds unless custom sizing is intentional.
- Use `Popup.ClosePopup` to dismiss and return a result.

## Styling and Theme Notes

- `tailwind.lua` defines utility classes, but color values come from
  `ThemeLibrary` / `orca.theme.colors`.
- Do not add or restore `config/tailwind.lua`; the Tailwind plugin does not load
  per-project Tailwind config.
- Missing `$...` theme keys should be fixed by adding static `ThemeLibrary`
  entries in `package.lua`.
- Keep `samples/*/package.lua` declarative. Do not call `require` or runtime
  APIs from package files; they run in a restricted project table environment.

## Verify

- Prefer focused Lua tests for layout/style behavior.
- Use `make test-styles-lua` or `make test-headless` when available and relevant.
