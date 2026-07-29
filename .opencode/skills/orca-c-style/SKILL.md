---
name: orca-c-style
description: Apply ORCA conventions to handwritten C implementation and test code. Use when adding, editing, or reviewing `.c` and handwritten `.h` files, choosing compact control-flow idioms, accessing generated properties, matching `.cgen` Lua signatures, or checking component and dependency boundaries.
---

# ORCA C Style

Match the nearest handwritten file in the same subsystem. Keep code compact
without hiding complex control flow.

## Write compact control flow

- Return early instead of nesting a failure branch.
- Omit braces around one readable statement.
- Keep a single-statement function on one line when the surrounding file does.
- Use a declaration initializer or ternary when it expresses one choice
  clearly.
- Use short-circuit assignment only for a simple pointer/truthiness guard and
  one expression.
- Use a plain `if` for complex conditions, multiple statements, or long calls.
- Use the comma operator for a short side effect plus return only when it stays
  obvious.
- Keep declarations close to first use and reuse existing helpers and macros.

```c
if (!ic->ItemsSource) return FALSE;
return Con_Printf("error"), FALSE;
struct Object *o = tpl ? OBJ_Instantiate(tpl) : OBJ_Create(ID_TextBlock);
n && (n->DataContext = OBJ_GetComponent(data, ID_DataObject));
FOR_EACH_OBJECT(c, hObject) { first = c; break; }
```

## Preserve architecture

- Add reusable behavior as a component instead of embedding unrelated fields
  in `Object`.
- Keep core free of plugin headers.
- Cross subsystem boundaries with messages, properties, or generic interfaces.
- Never edit generated C or generated documentation.

## Access properties correctly

Use generated full-path `ID_*` values with `OBJ_FindLongProperty`:

```c
struct Property *p = OBJ_FindLongProperty(obj, ID_Node_Controller);
```

Pass only a short-name hash such as `fnv1a32("Controller")` to
`OBJ_FindShortProperty`; never pass an `ID_*` value to it.

## Match generated Lua signatures

For a `.cgen` method or function declared with `lua="true"`, accept
`lua_State *L` as the first C parameter:

```xml
<method name="Rebuild" lua="true">
```

```c
void OBJ_Rebuild(lua_State *L, struct Object *self);
```

Use `lua="true"` only when the implementation touches the Lua stack or calls
Lua behavior. Verify the signature against generated declarations instead of
guessing it.

## Review the edit

1. Compare formatting with the nearest implementation.
2. Remove comments that only narrate the code; retain short comments that
   explain intent, ownership, or an unusual seam.
3. Check generated/public headers before introducing local declarations.
4. Build and run the focused tests selected by `$orca-testing`.
