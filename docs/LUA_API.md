# Lua API

Use this file when changing Lua-visible behavior.

## Where the Lua API comes from

- Generated bindings from module `.cgen` files
- Handwritten object/Lua bridge files under `source/core/object/`
- Runtime Lua code under `share/orca/`

## Module loading rules

- Built-in modules are registered in `package.preload` and execute on first `require`.
- `orca.<name>` auto-requires the matching module through the `orca` table.
- Native builds load plugin modules from shared libraries.
- WebGL builds preload plugin modules into the binary.

## Change workflow

1. If the API shape changes, edit the module `.cgen` first.
2. Run `make modules`.
3. Implement the runtime behavior in handwritten C or Lua.
4. Add a focused Lua test.

## Important boundaries

- Object/Lua bridge files live in `source/core/object/`, not `source/object/`.
- If module init order matters, call `require` explicitly.
- Prefer messages and properties over tight plugin-to-plugin coupling.

## Good follow-up reads

- `docs/lua-scripting.md`
- `docs/app/moonscript/scenes-and-rebuild.md`
- `docs/app/xml/triggers-actions-messages.md`
