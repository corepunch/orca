---
name: orca-c
description: >
  C engine development reference for ORCA. Covers the object/component model,
  macros, codegen workflow, .cgen format, and coding conventions. Use when
  working on the ORCA engine, adding components, writing C handlers, or
  modifying core subsystems.
---

# ORCA C Engine Development

A framework for building UI applications using an object/component architecture.
Components are declared in `.cgen` XML and implemented in C.

## Principles

1. **`.cgen` is source of truth.** Public API shape, properties, messages — all declared in `.cgen`.
2. **Generated files are outputs.** Never edit `*_export.c`, `*_properties.h`, or `*_module.h` by hand.
3. **Prefer components over fields.** New behavior should be a component, not a new embedded field on `Object`.
4. **Messages over coupling.** Cross-subsystem calls should prefer messages over tight direct coupling.
5. **Core stays plugin-free.** Core must remain buildable without plugin header dependencies.

## Critical Rules

### Header Map → [rules/header-map.md](./rules/header-map.md)

- **Include paths:** `include/orca.h` for everything public, `include/api.h` for Lua bindings.
- **Plugin code:** `#include <include/api.h>` + generated headers for your plugin.

### Architecture → [rules/architecture.md](./rules/architecture.md)

- **Runtime model:** ORCA is an object/component engine. `Object` is the universal scene node.
- **Main layers:** Core runtime, filesystem/parsers, renderer, plugins, Lua surface, codegen.
- **New behavior = component.** Don't add fields to `Object`.

### Adding Classes → [rules/adding-classes.md](./rules/adding-classes.md)

- **Mandatory workflow:** Edit `.cgen` → `make modules` → implement C handlers → confirm registration → add tests.
- **Minimum `.cgen` content:** `<class>`, `<summary>`, `<handles>`, `<properties>`, `<messages>`.
- **Validation:** `make modules`, then narrowest relevant test target.

### Coding Style → [rules/coding-style.md](./rules/coding-style.md)

- **Compact code.** Single-statement functions on one line. Omit braces for single-statement bodies.
- **Match surrounding file.** Use the nearest file in the same subsystem as style reference.
- **Don't edit generated files.** Generated code is output, not edit point.

### Object/Component Model → [rules/object-component-system.md](./rules/object-component-system.md)

- **Object struct:** `Name`, `parent`, `children`, `components`, `properties`.
- **Component = data + handler.** Each component type has a `ClassDesc` and an `ObjProc` handler.
- **Message dispatch:** Engine walks component list, calls each handler until one returns non-zero.

### Macros → [rules/macros-reference.md](./rules/macros-reference.md)

- **`REGISTER_CLASS(NAME, parent_ids...)`** — Defines `ClassDesc` for standalone components.
- **`DECL(SHORT, CLASS, NAME, FIELD, TYPE, ...)`** — Declares one property row.
- **`HANDLER(NAME, message)`** — Declares a message handler in the `ObjProc` switch.
- **`OBJ_SendMessageW(obj, msg, w, l)`** — Sends a message to an object.

### Codegen Workflow → [rules/code-generation.md](./rules/code-generation.md)

- **Run `make modules`** after editing `.cgen`.
- **Generated outputs:** `*_export.c`, `*_properties.h`, `*_module.h`.
- **Handwritten files:** `*.c` implementation files are not generated.

### .cgen Format → [rules/cgen-format.md](./rules/cgen-format.md)

- **Module root:** `<module name="..." prefix="">` with enums, structs, interfaces, classes.
- **Class declaration:** `<class name="..." parent="...">` with handles, properties, messages.
- **Property types:** `float`, `int`, `uint`, `bool`, `string`, `enum`, `struct`, `component`.

## Quick Reference

```bash
# Build codegen and generate modules
make modules

# Run tests
make test

# Build everything
make
```

## Key Patterns

```c
// Component handler pattern
static LRESULT MyComponentProc(struct Object *obj, void *cmp, uint32_t msg, wParam_t w, lParam_t l) {
    struct MyComponent *self = (struct MyComponent *)cmp;
    switch (msg) {
        case MSG_LOAD:     return MyComponent_Load(obj, self);
        case MSG_FREE:     return MyComponent_Free(obj, self);
        case MSG_UPDATE:   return MyComponent_Update(obj, self, w, l);
        case MSG_GETPROP:  return MyComponent_GetProperty(obj, self, w, l);
        case MSG_SETPROP:  return MyComponent_SetProperty(obj, self, w, l);
    }
    return 0;
}

// Property access
float speed = GetSpeed(obj);        // generated accessor macro
SetSpeed(obj, 3.14f);               // generated accessor macro

// Message sending
OBJ_SendMessageW(obj, MSG_UPDATE, 0, 0);
OBJ_SendMessageW(obj, MSG_SET_DIRTY, 0, 0);
```

## Detailed References

- [rules/architecture.md](./rules/architecture.md) — Runtime model, layers, architectural rules
- [rules/adding-classes.md](./rules/adding-classes.md) — Step-by-step workflow for new components
- [rules/coding-style.md](./rules/coding-style.md) — C style, architectural style, documentation style
- [rules/object-component-system.md](./rules/object-component-system.md) — Object struct, components, messages
- [rules/macros-reference.md](./rules/macros-reference.md) — Complete macro vocabulary
- [rules/code-generation.md](./rules/code-generation.md) — Build workflow, generated outputs
- [rules/cgen-format.md](./rules/cgen-format.md) — .cgen XML format reference
- [rules/header-map.md](./rules/header-map.md) — Include paths for each subsystem
- [rules/property-system.md](./rules/property-system.md) — Finding, reading, setting, iterating properties
- [rules/bindings.md](./rules/bindings.md) — Attaching and reading property bindings
- [rules/lifecycle.md](./rules/lifecycle.md) — Object create, init, hierarchy, reference counting
- [rules/component-accessors.md](./rules/component-accessors.md) — Get<ClassName>() pattern
- [rules/iteration-macros.md](./rules/iteration-macros.md) — FOR_LOOP, FOR_EACH_*, FOR_EACH_CHILD
- [rules/memory-and-helpers.md](./rules/memory-and-helpers.md) — ZeroAlloc, SafeFree, RAII, linked lists, logging
- [rules/generated-file-locations.md](./rules/generated-file-locations.md) — Where generated files live
- [rules/common-pitfalls.md](./rules/common-pitfalls.md) — Anti-patterns and their fixes
- [rules/available-classes.md](./rules/available-classes.md) — Quick index of all registered classes
