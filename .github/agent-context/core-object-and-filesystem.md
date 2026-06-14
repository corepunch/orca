# Core Object and Filesystem

Read this when changing the core object/component runtime, object lifecycle,
message dispatch, filesystem/package loading, XML parsers, localization, or
network/editor core services.

## Start Here

- `source/core/`
- `source/core/object/object_internal.h`
- `source/core/component.c`
- `include/orca.h`
- `include/orcadef.h`
- `source/filesystem/`
- `source/parsers/`
- `source/localization/`
- `source/network/`
- `source/editor/server.c`
- `docs/architecture/object-component-system.md`
- `docs/way-of-working.md`

## Object and Component Model

- `Object` is the universal scene node: named, hierarchical, and extended by
  attached components.
- Components are typed data blocks plus an object Proc. The `cmp` argument is
  the component data block.
- Use `HANDLER(CLASS, NS, EVENT)` for message handlers.
- Switch cases for generated message IDs must mask with `MSG_DATA_MASK`.
- Generated accessors like `GetAnimationPlayer(obj)` return `NULL` when the
  component is absent.

## Architecture Boundaries

- Core must stay runnable without Lua/MoonScript-only assumptions.
- Core must not include plugin headers such as `<UIKit/UIKit.h>`.
- Plugins depend on core; core talks to plugins through messages or generic
  interfaces.
- When adding new behavior, prefer a component over new embedded fields on
  `struct Object`.

## Object Struct Refactoring Direction

The long-term goal is zero subsystem fields in `Object`. Subsystems should move
to components. Done examples include animation players/clips and property
animations. Planned/migrating areas include timers, state managers,
stylesheets/classes, and aliases.

Migration checklist:

1. Implement a component with all needed `<handle>` entries.
2. Register it through generated class registration.
3. Remove the legacy `struct Object` field and accessor macro.
4. Remove manual release code from `OBJ_Release`.
5. Remove old enum bookkeeping.
6. Add tests for attach, properties, and at least one message roundtrip.

## Parser and Filesystem Gotchas

- XML attributes should be read with `xmlGetProp` or `xmlGetNsProp`, then freed
  with `xmlFree`. Avoid `xmlNodeListGetString` for attributes; compact-mode
  libxml2 storage can return `NULL` for non-empty values.
- Package data and sample `package.lua` files should stay declarative when they
  are loaded as project metadata.

## System-Level Message Handlers

- `SV_RegisterMessageProc` registers a raw platform event handler separate from
  per-object `ObjProc` dispatch.
- Signature: `LRESULT handler(lua_State *L, struct AXmessage *msg);`
- Return `TRUE` to consume; `FALSE` to let later handlers run.
- Handlers run in reverse registration order.
- Register from `on-luaopen` callbacks. Examples:
  `CORE_ProcessMessage`, `ui_handle_event`, and `filesystem_handle_event`.
