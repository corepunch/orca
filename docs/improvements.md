# Code Improvements & Gotchas

This document records design issues, gotchas, misleading patterns, and known improvement areas found during development. It is intended for contributors who need to understand why certain decisions were made and what traps to avoid.

---

## Animation System

### `animation2.c` is entirely commented out
`source/core/components/animation2.c` contains only commented-out code — the full property animation tween API that was replaced by the `PropertyAnimation` attach-only component. The file was kept to preserve context but adds confusion. It should be removed once the new component-based tween system is confirmed stable.

### `animation.c` has two disabled interpolation algorithms
`animation_evaluate()` in `source/core/components/animation.c` is compiled with `#define USE_CUBIC_BEZIER`. The Hermite and linear variants are inside `#elif` / `#else` blocks that are never compiled. Only one algorithm should exist; the dead variants should be removed.

### `AnimationCurve` was initially `attach-only` (wrong design)
The first version of the component system made `AnimationCurve` an attach-only component bolted onto the `AnimationClip` object's component list. This was wrong: `AnimationCurve` is a data object that belongs as a **child** of `AnimationClip` in the object hierarchy, not as an attached component. Components are for behaviour; child objects are for data. The rule is: if something has identity (a name, can be referenced, carries data that outlives a single message) it should be a child object, not an attach-only component.

### `AnimationPlayer` iterates clip's children, not components
Because `AnimationCurve` is now a child object of `AnimationClip`, `AnimationPlayer.Animate` uses `FOR_EACH_OBJECT` to walk the clip's children and calls `GetAnimationCurve(child)` on each. If the relationship between clip and curves ever changes (e.g. grandchildren), this traversal will break silently because `FOR_EACH_OBJECT` only walks direct children.

### `TANGENT_MODE_BEZIER` macro must be defined before first use
In `AnimationPlayer.c`, `TANGENT_MODE_BEZIER` must appear before `keyframe_evaluate()`. If it is accidentally moved after the function definition, the build will fail with an undefined identifier — not a compile error about the macro itself, which makes it harder to diagnose.

---

## Component / Object System

### `OBJ_SendMessageW` does NOT mask routing bits
The routing bits (lower 2 bits of every message ID) are **not** stripped by `OBJ_SendMessageW` before calling each component's `ObjProc`. Generated `*Proc` functions mask in their `switch` statement: `case ID_Object_Start & MSG_DATA_MASK:`. Direct callers of `OBJ_SendMessageW` that compare raw `msg` values inside their proc will silently match the wrong case unless they also mask. Always use `& MSG_DATA_MASK` when comparing message IDs inside an `ObjProc`.

### `IsAttachOnly` is only enforced at the Lua bridge
The `IsAttachOnly` flag in `ClassDesc` is checked by `OBJ_AddComponentByName` (the Lua bridge, which raises a `luaL_error`). Direct C callers of `OBJ_AddComponent(obj, classID)` are **not** restricted. The flag is architectural documentation, not a hard constraint in C code. If you call `OBJ_AddComponent` directly with a non-attach-only class ID, the engine will silently attach it as a component rather than refusing.

### `objectProc_t` receives a potentially unmasked message ID
The `cmp` argument to `ObjProc` is the component's `pUserData` — not a `struct component*`. The `msg` parameter is forwarded as-is from `OBJ_SendMessageW`; it may include routing bits. Always mask: `(msg & MSG_DATA_MASK)` before switching or comparing.

### Struct Object had embedded subsystem fields (removed)
Before the animation refactor, `struct Object` had `animation`, `animlib`, and `animations` fields directly embedded. These coupled the object core to the animation subsystem and complicated `OBJ_Release`. They are removed; the pattern of adding fields directly to `struct Object` should not be repeated. All new functionality must use a component.

---

## Property Animation (Tweens)

### `byte_t[]` from/to buffers need `memcpy` for alignment
`PropertyAnimation`'s `from` and `to` fields are `byte_t[]` arrays with 1-byte alignment. Reading them via `*(float*)` casts is undefined behaviour on architectures requiring natural alignment (ARM, MIPS). Always use `memcpy` into a local `float` variable before arithmetic.

### Negative `duration` must be handled as immediate completion
`PropertyAnimation.duration` is stored as `int` (can be negative if Lua passes a bad value). Casting it to `uint32_t` before division turns a negative value into a huge positive number, causing the tween to effectively never complete. The handler treats `duration <= 0` as immediate (t = 1.0f).

---

## Lua Bridge Design

### `OBJ_AddComponentByName` vs `OBJ_AddComponent`
Two distinct APIs exist:
- `OBJ_AddComponent(lpObject_t, uint32_t)` — C-only 2-arg API, unrestricted
- `OBJ_AddComponentByName(lua_State*, lpObject_t, const char*)` — 3-arg Lua bridge, checks `IsAttachOnly`, raises `luaL_error` on failure

The names are similar enough to cause confusion. The rename from the original 3-arg `OBJ_AddComponent` was necessary to avoid a conflicting C declaration with the 2-arg public API in `orca.h`. When calling from C, always use the 2-arg form.

### Error propagation: `luaL_error` vs `Con_Error` in Lua bridges
Functions that take a `lua_State*` parameter are Lua bridges and **must** use `luaL_error` to report failures. Using `Con_Error` and returning normally is wrong: the calling Lua script will silently continue with a missing component, which is extremely hard to debug. Broken XML loaded silently is indistinguishable from correct XML that simply doesn't display anything.

---

## Message / Event System

### `Object.Animate` lParam should be `NULL`
`Object.Animate` is declared with an empty message struct. Passing the `lua_State*` as `lParam` creates type confusion for all handlers and blocks any future addition of real message arguments. The `lua_State` should be accessed via the global `core.L` (as in all other handlers); pass `NULL` as lParam.

### Module-level `<message>` is invalid in XML
The module DTD does not permit `<message>` as a direct child of `<module>`. Messages must live inside a `<class>` or `<interface>`'s `<messages>` container. The XML file will parse without error (libxml2 is lenient), but the code generator will silently skip the message, leaving you with an undefined `ID_*` constant that resolves to 0 and causes silent message mis-dispatch.

---

## Code Generation

### `pyphp` silently empties generated files if pyphp is not installed
Running `make` or `make -C tools` when `pyphp` is not installed will **truncate all generated files to zero bytes** because the shell redirect `> file` runs even when the command before it fails. Always verify `python3 -c "import pyphp"` before running code generation. Corrupted generated files look like successful builds until you hit a missing symbol.

### Block comments in PHP templates cause `SyntaxError`
The `pyphp` preprocessor does **not** strip `/* ... */` C-style block comments from PHP code blocks. They pass through to generated Python verbatim, causing `SyntaxError`. Use `//` line comments in PHP templates; pyphp converts `//` to `#`.

### `FOR_EACH_LIST` — only the exact macro signature is safe
The `FOR_EACH_LIST` macro in `include/orcadef.h` uses a specific pattern for the `next` pointer initialisation:
```c
for (TYPE *var = (head), *next = (head) ? (head)->next : NULL; \
     var; \
     var = next, next = var ? var->next : NULL)
```
Do not copy a simplified version; the safe-`next` pattern is essential when the loop body frees or removes items from the list.
