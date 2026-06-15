# Lua Bindings and Programs

Read this when changing Lua-facing APIs, `object_lua_*` files, property
bindings/formulas, application scripts, MoonScript/Lapis-inspired framework
code, or generated Lua exports.

## Start Here

- `source/core/object/object_lua_*.c`
- `source/core/property/`
- `share/orca/`
- `docs/lapis/lapis-user-guide.md`
- `docs/lapis/lapis-internals.md`
- `docs/skills/Orca-App-MoonScript-Lapis-Builder.skill.md`
- `docs/skills/Orca-Lua-Lapis-Internals-Architect.skill.md`

## Lua Module Loading

- Built-in modules are registered in `package.preload` by `luaopen_orca` before
  project code runs, but execute lazily on first `require`.
- The `orca` table auto-requires `orca.<key>` through `__index`, so
  `orca.geometry` and `require "orca.geometry"` are equivalent.
- On WebGL, plugin modules are compiled into WASM and pre-registered; `cpath` is
  not extended for `.so` loading.
- If init order matters, call `require` explicitly. Do not rely on registration
  order for module side effects.

## Message and Object API Notes

- Object/Lua bridge code lives under `source/core/object/`; do not look for
  `source/object/`.
- Lua may call object message handlers directly with capitalized method names,
  for example `self:MessageName(...)` or `screen:SetModalObject(modal)`.
- If a Lua API needs a new C method, declare it in `.cgen`, run `make modules`,
  then implement the handler/body. Do not edit generated exports by hand.
- Component coupling should use messages. Avoid plugin-to-plugin runtime helper
  symbol calls when a message API would preserve module boundaries.

## Property VM and Bindings

- Property bindings live under `source/core/property/`.
- `IF(cond, true_val, false_val)` follows spreadsheet-style semantics:
  numeric nonzero is true, non-empty string is true, and the result type follows
  the selected branch.
- `Token_Release` must recurse into `args[]` as well as `next`; function-call
  tokens own their argument subtrees.

## App Framework Direction

- For Orca application development, prefer MoonScript with Lapis-inspired route,
  view, widget, and layout structure.
- Use Tailwind utilities and the local Orca UI dialect where practical.
- When the primitive set is missing something, propose a small Lapis-inspired
  framework improvement instead of forcing awkward app code.

## Verify

- Add Lua tests for new Lua-facing behavior.
- Run `make modules` after `.cgen` API changes.
- Use the narrowest relevant Lua/headless test first, then broader tests if the
  API surface is shared.
