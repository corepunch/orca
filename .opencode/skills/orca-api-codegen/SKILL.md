---
name: orca-api-codegen
description: Add or change ORCA generated API shape, including classes, properties, fields, methods, messages, interfaces, structs, enums, functions, and Lua export metadata. Use for new classes or properties, `.cgen` edits, generated accessors and IDs, code-generator changes, or C/Lua/XML API-reference regeneration.
---

# ORCA API Codegen

Use the owning module `.cgen` as the source of truth. Treat generated code and
generated documentation as disposable output.

## Workflow

1. Locate the owning subsystem with `docs/PROJECT_MAP.md`.
2. Read the relevant reference under
   `.opencode/generated/{c,lua,xml}/<module>/`. Fall back to the matching
   `.cgen` only when the generated reference is missing or stale.
3. Read `docs/MODULE_XML_GUIDE.md` for declaration syntax and at most one
   nearby declaration for local ordering and naming.
4. Edit the relevant `.cgen`. Declare every message handled by a class under
   its `<handles>` section.
5. Run `make modules`.
6. Inspect the generated header, property definitions, registration, or
   `*_export.c` table needed to confirm the declaration. Do not edit it.
7. Implement the required behavior in handwritten C. For a new source file,
   add the same build and Xcode project wiring used by its nearest peer.
8. Add a focused test with `$orca-testing`; run the narrowest test target and
   broaden validation only for shared API changes.

Use `$orca-lua-exposure` for Lua-visible methods, properties, messages, or
functions.

## Guardrails

- Never edit `generated/`, `.opencode/generated/`, or `docs/generated/`.
- Preserve PascalCase declaration names. Expect generated Lua method names to
  lowercase the first letter unless `export` explicitly overrides the name.
- Add behavior as a component instead of embedding unrelated fields in
  `Object`.
- Keep core independent of plugin headers. Cross subsystem boundaries through
  properties, messages, or generic interfaces.
- Use `OBJ_FindLongProperty` with generated full-path `ID_*` values; use
  `OBJ_FindShortProperty` only with a short-name hash.

## Verification

Run commands in this order and stop at the narrowest sufficient level:

```bash
make modules
make unite
make test-properties   # property/codegen work
make test-headless     # shared C, Lua, or UI behavior
```
