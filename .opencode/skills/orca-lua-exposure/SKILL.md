---
name: orca-lua-exposure
description: Expose existing or new ORCA engine behavior to Lua through generated exports or handwritten object-bridge code. Use for `.cgen` Lua flags and export names, Lua-visible methods, properties, messages, or functions, `object_lua_*` changes, module registration, conversion logic, and Lua-facing API tests.
---

# ORCA Lua Exposure

Reuse the existing C API whenever possible. Add a handwritten Lua bridge only
for behavior or conversions that codegen cannot express.

## Workflow

1. Read `.opencode/generated/lua/<module>/`, then `docs/LUA_API.md`. Fall
   back to the relevant `.cgen` only when the generated reference is missing.
2. Confirm whether the C method, property, message, or function already
   exists and identify its owning module.
3. Use `$orca-api-codegen` for any API or export-metadata change. Edit the
   owning `.cgen`, then run `make modules`.
4. Use `lua="true"` when the C implementation must receive `lua_State *L`;
   make it the first implementation parameter exactly as generated.
5. Inspect the generated `*_export.c` registration table to verify the actual
   Lua name and wrapper. Never patch the table.
6. Add handwritten conversion or dispatch code only when generated bindings
   cannot represent the operation. Keep object bridge work under
   `source/core/object/`.
7. Require modules explicitly when initialization order matters.
8. Add a focused Lua test and run it through the narrowest existing headless
   target.

## Guardrails

- Preserve PascalCase in `.cgen`; expect `FindChild` to register as
  `findChild` unless `export` overrides it.
- Do not invent a second C implementation for Lua.
- Preserve module boundaries by dispatching messages or using properties
  instead of calling plugin runtime helpers.
- Distinguish generated camelCase method names from explicitly named direct
  message handlers.

## Test the Lua contract

Cover:

- valid argument and object conversion;
- returned value or pushed Lua value;
- invalid input or failure behavior;
- one call through the registered Lua name;
- queued behavior after `core.flushQueue()` when applicable.

Use `$orca-testing` for harness and command selection.
