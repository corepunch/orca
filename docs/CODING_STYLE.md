# C Coding Style

See also: compact idioms in `AGENTS.md`.

## Property Lookup

Generated `ID_*` macros (e.g. `ID_Node_Controller`) contain the full-path FNV-1a32 hash (`fnv1a32("Node.Controller")`). Use `OBJ_FindLongProperty` with them:

```c
struct Property *p = OBJ_FindLongProperty(obj, ID_Node_Controller);
```

`OBJ_FindShortProperty` expects the short-name hash (`fnv1a32("Controller")`). Do not pass `ID_*` macros to it.

## cgen `lua="true"`

When a `<method>` or `<function>` in `.cgen` has `lua="true"`, the code generator inserts `lua_State *L` as the first parameter of the C function. The implementation must match:

```xml
<method name="Rebuild" lua="true">
```

```c
// generated wrapper calls:  OBJ_Rebuild(L, self, ...);
void OBJ_Rebuild(lua_State *L, struct Object *self);
```

Use `lua="true"` whenever the implementation needs to push/pop values, call Lua functions, or otherwise touch the Lua stack.
