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

## DataContext and datasource API

- `DataContext` is the base class for data-providing objects. It handles the `DataContext.GetData` message.
- `DataSource` (parent: DataContext) owns a root `DataObject` tree loaded by a concrete subclass.
- `XmlDataSource` (parent: DataSource) loads data from XML, optionally backed by a `Schema.xml`.
- `DataObject` (parent: DataContext) is the runtime representation of a schema entity instance.
- Schema files (`DS_ParseSchema`, `DS_ParseSchemaFromString` in `source/data/data_schema.c`) register `ClassDesc` entries with typed column properties.
- Binding engine walks the `DataContext` chain: DataSource returns its root DataObject, DataObject returns itself.

## Good follow-up reads

- `docs/lua-scripting.md`
- `docs/app/moonscript/scenes-and-rebuild.md`
- `docs/app/xml/triggers-actions-messages.md`
