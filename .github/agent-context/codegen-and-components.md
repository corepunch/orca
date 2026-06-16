# Codegen and Components

Read this when changing `.cgen` files, adding component classes, changing
messages/properties/methods, editing codegen tools, or touching generated API
shape.

## Start Here

- `docs/schemas/module.dtd`
- `source/core/core.cgen`
- `plugins/UIKit/UIKit.cgen`
- `docs/MODULE_XML_GUIDE.md`
- `tools/codegen/src/model.c`
- `tools/codegen/plugins/header.c`
- `tools/codegen/plugins/properties.c`
- `tools/codegen/plugins/export.c`

## Mandatory Workflow

1. Edit the module `.cgen` first.
2. Run `make modules`.
3. Implement C handlers using `HANDLER(CLASS, NS, EVENT)`.
4. Register new handwritten `.c` files in the Makefile path and Xcode project.
5. Add focused tests.
6. Build/test with the narrowest useful command.

Never edit generated files by hand. Generated headers, property hashes, and
exports under `generated/` are ignored and overwritten.

## Module Structure

Every C module lives in `source/<module>/` or `plugins/<name>/` and exposes its
API through a co-located `.cgen` file. Codegen produces:

- `<module>.h`
- `<module>_properties.h`
- `<module>_export.c`

The root `Makefile` and `tools/Makefile` discover module files with
`find source plugins -name '*.cgen'`.

## XML Containers

Use grouped containers:

- `<class>`: `<handles>`, `<properties>`, `<fields>`, `<methods>`, `<messages>`
- `<struct>`: `<fields>`, `<methods>`
- `<interface>`: `<methods>`, `<messages>`
- `<message>`: `<fields>`
- `<module>`: `<includes>`, `<externals>`

`<topic title="...">` is allowed only inside an interface `<methods>` container.
It is a documentation separator, not a codegen method.

## Component Class Rules

- Every handled message needs a matching `<handle message="NS.Event"/>`.
- A class with no `<handles>` generates an empty Proc switch and silently does
  nothing.
- `Object.Create` handlers run after component allocation and before property
  values are applied. Set initialization defaults directly there.
- Use generated registration macros and call `OBJ_RegisterClass(&_ClassName)` in
  the module registration callback when required.

## Xcode Project Wiring

New `.c` files under `source/core/components/` or `plugins/*/` must be added to
`orca.xcodeproj/project.pbxproj` in four places:

1. `PBXBuildFile`
2. `PBXFileReference`
3. owning `PBXGroup`
4. `PBXSourcesBuildPhase`

Skipping this means Makefile builds can pass while macOS/Xcode builds omit the
file.

## Common Failures

- Writing C handlers before declaring the class in `.cgen`.
- Declaring a class but forgetting `<handle>` entries.
- Migrating a subsystem to a component but leaving the old global/singleton path
  active.
- Including plugin headers from core.
- Skipping tests for attach, property set/get, and message roundtrips.
