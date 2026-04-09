# C Macros Reference

This page is a complete reference for every macro that appears frequently when writing or reading ORCA component code. Understanding these macros is essential because **all generated `*_export.c` code is built entirely from them**, and hand-written component `.c` files use the same vocabulary.

---

## Component Registration

### `REGISTER_CLASS(NAME, parent_ids…)`

Defines the global `struct ClassDesc _NAME` that describes a component type which can be **instantiated standalone** (e.g. `AnimationClip`, `Button`). Call `OBJ_RegisterClass(&_NAME)` at module init to activate it.

```c
// Generated in *_export.c — do not write by hand
REGISTER_CLASS(Button, ID_Node2D);

// Expands to:
ORCA_API struct ClassDesc _Button = {
    .ClassName    = "Button",
    .DefaultName  = "Button",
    .ContentType  = "Button",
    .Xmlns        = "http://schemas.corepunch.com/orca/2006/xml/presentation",
    .ParentClasses = { ID_Node2D, 0 },
    .ClassID      = ID_Button,
    .ClassSize    = sizeof(struct Button),
    .Properties   = ButtonProperties,
    .ObjProc      = ButtonProc,
    .Defaults     = &ButtonDefaults,
    .NumProperties = kButtonNumProperties,
};
```

The `ParentClasses` variadic list comes from the XML `<class>` element's inheritance chain. An entry of `0` terminates the list.

---

### `REGISTER_ATTACH_ONLY_CLASS(NAME, parent_ids…)`

Same as `REGISTER_CLASS` but sets `.IsAttachOnly = TRUE`. Attach-only components **cannot be instantiated standalone**; they must be added to an existing object with `OBJ_AddComponent`. Example: `AnimationPlayer`.

The `attach-only="true"` attribute in the XML `<class>` element triggers this macro instead of `REGISTER_CLASS`.

---

## Property Declaration

### `DECL(SHORT, CLASS, NAME, FIELD, TYPE, …)`

Declares one row in a component's `PropertyType` table. The table is a `static struct PropertyType const ClassNameProperties[kClassNameNumProperties]` array defined in `*_export.c`.

| Argument | Meaning |
|---|---|
| `SHORT` | Pre-computed FNV1a hash of the bare property name (e.g. `"Speed"`) |
| `CLASS` | The component struct name (e.g. `AnimationPlayer`) |
| `NAME` | Property name — used to compute `ID_CLASS_NAME` and as the `.Name` string |
| `FIELD` | The C struct field that backs this property |
| `TYPE` | One of the `kDataType*` enum values |
| `…` | Optional extra field initialisers (e.g. `.TypeString = "Keyframe"`, `.EnumValues = _AnimationMode`) |

```c
// Typical usage in generated code:
DECL(0x0a6b8020, AnimationPlayer, Speed, Speed, kDataTypeFloat),
// → { .Name="Speed", .Category="AnimationPlayer",
//     .ShortIdentifier=0x0a6b8020,
//     .FullIdentifier=ID_AnimationPlayer_Speed,
//     .Offset=offsetof(struct AnimationPlayer, Speed),
//     .DataSize=sizeof(((struct AnimationPlayer*)NULL)->Speed),
//     .DataType=kDataTypeFloat }
```

---

### `ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE, …)`

Like `DECL` but adds `.IsArray = TRUE`. This causes the code generator to also emit a `NumNAME` companion property that stores the element count. Used for properties like `Keyframes` that map to a heap-allocated array plus a count field.

```c
ARRAY_DECL(0xf893ff8e, AnimationCurve, Keyframes, Keyframes, kDataTypeStruct,
           .TypeString = "Keyframe"),
// → PropertyType with .IsArray=TRUE; pairs with NumKeyframes property
```

---

## Message Handling

### `HANDLER(CLASS, NS, EVENT)`

Forward-declares (in `*_export.c`) and defines (in `CLASS.c`) a strongly-typed message handler function. This is the primary pattern for component message handling.

**Macro signature:**
```c
#define HANDLER(CLASS, NS, EVENT)                    \
  LRESULT CLASS##_##EVENT(struct Object* hObject,    \
                          struct CLASS* p##CLASS,    \
                          wParam_t wParam,           \
                          NS##_##EVENT##MsgPtr p##EVENT)
```

**How it connects to the generated Proc:**

`*_export.c` forward-declares each handler listed in `<handles>` and then calls them from the component's `Proc` switch:

```c
// In *_export.c (generated) — forward declarations:
HANDLER(AnimationPlayer, Object, Start);
HANDLER(AnimationPlayer, Object, Animate);

// The generated Proc:
LRESULT AnimationPlayerProc(struct Object* object, void* cmp,
                             uint32_t message, wParam_t wparm, lParam_t lparm) {
    switch (message & MSG_DATA_MASK) {
        case ID_Object_Start   & MSG_DATA_MASK:
            return AnimationPlayer_Start(object, cmp, wparm, lparm);
        case ID_Object_Animate & MSG_DATA_MASK:
            return AnimationPlayer_Animate(object, cmp, wparm, lparm);
    }
    return FALSE;
}
```

**Handwritten implementation in `AnimationPlayer.c`:**
```c
HANDLER(AnimationPlayer, Object, Start) {
    // hObject  — the Object this component is attached to
    // pAnimationPlayer — the component data (same as cmp, already cast)
    // pStart   — Object_StartMsgPtr (empty struct for this message)
    pAnimationPlayer->CurrentTime = 0.0f;
    return FALSE;
}

HANDLER(AnimationPlayer, Object, Animate) {
    if (!pAnimationPlayer->Playing) return FALSE;
    // advance CurrentTime, evaluate curves…
    return FALSE;
}
```

**Parameter naming convention:**

| Parameter | Type | Description |
|---|---|---|
| `hObject` | `struct Object*` | The object the component is attached to |
| `p<CLASS>` | `struct CLASS*` | The component's data block (`cmp` already cast) |
| `wParam` | `wParam_t` | Integer extra parameter (varies by message) |
| `p<EVENT>` | `NS_EVENTMsgPtr` | Typed pointer to the message arguments struct |

**The `<handles>` XML element** drives this: every `<handle message="NS.Event"/>` inside a `<class>` produces one `HANDLER` forward declaration and one switch case in the Proc.

---

### `_SendMessage(OBJECT, CLASS, MESSAGE, …)`

Convenience macro that builds the message arguments struct inline and dispatches it synchronously through `OBJ_SendMessageW`:

```c
#define _SendMessage(OBJECT, CLASS, MESSAGE, ...) \
    OBJ_SendMessageW(OBJECT, ID_##CLASS##_##MESSAGE, 0, \
                     &(CLASS##_##MESSAGE##Msg_t){ __VA_ARGS__ })
```

```c
// Send Object.PropertyChanged with a specific property pointer:
_SendMessage(object, Object, PropertyChanged, .Property = myProp);

// Send Object.Start with no extra arguments:
_SendMessage(object, Object, Start);
```

The struct type `CLASS_MESSAGEMsg_t` is generated in `<module>.h` from the message's `<fields>` in XML.

---

## Object Hierarchy

### `FOR_EACH_OBJECT(var, parent)`

Safely iterates the immediate children of `parent`. Caches `next` before the body executes so children can be added or removed during iteration:

```c
#define FOR_EACH_OBJECT(var, parent)                       \
  for (lpObject_t var = OBJ_GetFirstChild(parent),         \
         next = var ? OBJ_GetNext(var) : NULL;             \
       var;                                                \
       var = next, next = next ? OBJ_GetNext(next) : NULL)
```

```c
// Recurse into every child:
FOR_EACH_OBJECT(child, object) {
    OBJ_Animate(L, child);
}
```

---

### `FOR_EACH_CHILD(object, func, …)`

Calls `func(child, …)` on every direct child — a shorthand when the body is a single function call:

```c
#define FOR_EACH_CHILD(object, func, ...) \
    FOR_EACH_OBJECT(child, object) func(child, ##__VA_ARGS__)

FOR_EACH_CHILD(root, OBJ_ClearDirtyFlags);
```

---

### `FOR_EACH_LIST(TYPE, var, head)`

Iterates a singly-linked list whose nodes are of type `TYPE` and have a `next` pointer. Caches `next` before the body so nodes may be freed inline:

```c
#define FOR_EACH_LIST(type, property, list)                                    \
  for (type* property = list, *next = list ? (list)->next : NULL; property;    \
       property = next, next = next ? next->next : NULL)
```

```c
FOR_EACH_LIST(struct timer, t, _GetTimers(object)) {
    if (t->expired) {
        REMOVE_FROM_LIST(struct timer, t, _GetTimers(object));
        free(t);
    }
}
```

---

### `FOR_LOOP(var, count)`

Simple counted loop used throughout generated and hand-written code:

```c
#define FOR_LOOP(var, count) for (int var = 0; var < (count); var++)

FOR_LOOP(i, cls->NumProperties) {
    // process property i
}
```

---

## Linked-List Helpers

### `ADD_TO_LIST(item, list_head)` / `ADD_TO_LIST_END(…)`

Prepends or appends `item` to a singly-linked list via the `next` pointer.

### `REMOVE_FROM_LIST(TYPE, item, list_head)`

Removes `item` from the list by walking the list and relinking.

---

## Lua Binding Helpers

### `luaX_parsefield(type, name, table_idx, func, …)`

Reads the field `name` from the Lua table at `table_idx` and stores it in a local variable:

```c
#define luaX_parsefield(type, name, unit, func, ...)  \
    lua_getfield(L, unit, #name);                     \
    type name = func(L, -1, ##__VA_ARGS__);           \
    lua_pop(L, 1);
```

```c
// Read a duration integer from table at stack index 2:
luaX_parsefield(int, duration, 2, luaL_checknumber);

// Read an enum option:
luaX_parsefield(enum ipo_type, ipo, 2, luaL_checkoption, "linear", ipo_text);
```

---

### `WITH(type, var, expr, cleanup)`

RAII-style scope that runs `cleanup(var)` at the end of the block, similar to a scoped resource guard. Used heavily for `xmlChar*` strings that must be freed with `xmlFree`:

```c
WITH(xmlChar, Path, xmlGetProp(xml, XMLSTR("Path")), xmlFree) {
    strncpy(anim->path, (LPSTR)Path, sizeof(anim->path));
}
// xmlFree(Path) is called here automatically
```

---

## Generated ID Constants

`*_properties.h` (generated from `*_export.c`) defines:

| Constant | Meaning |
|---|---|
| `ID_ClassName` | FNV1a hash of `"ClassName"` — the component's `ClassID` |
| `ID_ClassName_PropertyName` | FNV1a hash of `"ClassName.PropertyName"` — the property's full identifier; message IDs include routing bits |
| `GetClassName(obj)` | Inline that calls `OBJ_GetComponent(obj, ID_ClassName)` and casts the result |
| `kClassNameNumProperties` | Compile-time count of properties |
| `enum ClassNameProperties` | Index enum: `kClassNameSpeed`, `kClassNameCurrentTime`, etc. |

```c
// Access component data:
struct AnimationPlayer *ap = GetAnimationPlayer(object);
if (ap && ap->Playing) { … }

// Find a property by index:
lpProperty_t p = AnimationPlayer_GetProperty(object, kAnimationPlayerSpeed);
```

---

## Message ID Routing Bits

Every message ID embeds its routing strategy in the **lower 2 bits**. `OBJ_RaiseEvent` uses these bits for bubbling/tunneling; `OBJ_SendMessageW` forwards the ID as-is to each component's Proc. Generated `*Proc` switch statements always mask with `MSG_DATA_MASK` when comparing message IDs:

```c
#define MSG_ROUTING_MASK           3u
#define MSG_DATA_MASK              (~MSG_ROUTING_MASK)
#define ROUTING_BUBBLE             0u
#define ROUTING_TUNNELING_BUBBLING 1u
#define ROUTING_TUNNELING          2u
#define ROUTING_DIRECT             3u
```

Switch cases in generated `*Proc` functions always mask the incoming message ID:

```c
case ID_Object_Start & MSG_DATA_MASK:  // strips routing bits for comparison
```

The `routing=` attribute on `<message>` elements in XML sets the lower bits at code-generation time. Call sites that send a message directly (e.g. `OBJ_SendMessage`) may strip routing bits before calling, but handlers must always mask defensively.

---

## The `objectProc_t` Signature

The component message handler function pointer type is:

```c
typedef LRESULT (*objectProc_t)(lpObject_t, void*, uint32_t, wParam_t, lParam_t);
```

Parameters:

| Position | Type | Description |
|---|---|---|
| 1 | `lpObject_t` | The owning Object |
| 2 | `void*` | Component user data (`cmp->pUserData`) — cast to `struct ClassName*` |
| 3 | `uint32_t` | Message ID as forwarded from `OBJ_SendMessageW` (routing bits may be present; generated Procs mask with `MSG_DATA_MASK` in their switch) |
| 4 | `wParam_t` | Integer parameter |
| 5 | `lParam_t` | Pointer parameter (cast to the message args struct) |

`OBJ_SendMessageW` in `component.c` calls every component's Proc in order and returns early if any returns non-zero:

```c
FOR_EACH_LIST(struct component, cmp, _GetComponents(pobj)) {
    if (cmp->pcls->ObjProc) {
        LRESULT res = cmp->pcls->ObjProc(pobj, cmp->pUserData,
                                          MsgID, wParam, lParam);
        if (res) return res;
    }
}
```

The `HANDLER` macro aligns with this: its second parameter (`struct CLASS* p##CLASS`) receives `cmp->pUserData` without an explicit cast because C allows implicit `void*`-to-pointer conversion.
