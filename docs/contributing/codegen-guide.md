# Working With the Orca Codebase — Codegen Guide

This document is the single reference for writing C code in this repository.
Read it before editing any `.c` file. Every pattern here is exact — names,
signatures, and macro spellings are taken directly from the source.

---

## Header map

Only include what you need. Do not include implementation headers from outside
your translation unit.

| What you need | Include |
|---|---|
| Everything public (OBJ_, PROP_, FS_, types) | `#include <include/orca.h>` |
| Public API + Lua bindings | `#include <include/api.h>` |
| Inside `source/core/` files | `#include <source/core/core_local.h>` |
| Inside `source/filesystem/` files | `#include <source/filesystem/fs_local.h>` |
| Generated class/property IDs + Get*() accessors | `#include <core/core_properties.h>` (core) or `#include <UIKit/UIKit_properties.h>` (UIKit) |
| Generated struct definitions | `#include <core/core.h>`, `#include <UIKit/UIKit.h>` |
| Plugin implementations | `#include <include/api.h>` + generated headers for your plugin |

`orca.h` includes `orcadef.h`, `shared.h`, `orcaerror.h`, and `core/core.h`
transitively. You get all macros and typedefs from a single `#include <include/orca.h>`.

---

## Type aliases

All defined in `include/orcadef.h` and `libs/platform/platform.h`:

```c
typedef const char *lpcString_t;   // read-only string (never free)
typedef char       *LPSTR;         // heap string (owner frees)
typedef char        shortStr_t[64];        // stack string, MAX_NAMELEN bytes
typedef char        fixedString_t[256];    // stack string, MAX_PROPERTY_STRING bytes
typedef char        path_t[1024];          // MAX_OSPATH bytes
typedef unsigned char   byte_t;
typedef void           *handle_t;
typedef intptr_t        LRESULT;    // message return value
typedef signed int      HRESULT;    // S_OK / E_FAIL result
typedef uint32_t        wParam_t;   // word message parameter
typedef void           *lParam_t;   // pointer message parameter
typedef unsigned long   longTime_t; // millisecond timestamp
typedef uint32_t        event_t;    // event/message ID stored in a struct field
```

---

## Error checking

`HRESULT` functions return `S_OK` (0) on success, negative values on failure.

```c
// orcaerror.h
#define SUCCEEDED(hr)  (((HRESULT)(hr)) >= 0)
#define FAILED(hr)     (((HRESULT)(hr)) < 0)
#define S_OK     ((HRESULT)0)
#define E_FAIL   ((HRESULT)0x80000008L)
```

Usage:

```c
struct Property *prop = OBJ_FindLongProperty(object, ID_Node2D_Width);
if (!prop) {
    Con_Error("Width not found");
    return FALSE;
}
```

---

## Logging

```c
// orcadef.h — all expand to fprintf, never abort
Con_Error("bad value %d in %s", value, name);   // stderr, prefixed with func name
Con_Warning("soft problem: %s", reason);         // same but "Warning: " prefix
Con_Printf("debug: %g\n", f);                    // stdout (release) / stderr (debug)
```

---

## Memory allocation

```c
// ZeroAlloc — always use instead of malloc+memset. Defined in orcadef.h,
// expands to _zero_alloc(__FILE__, __LINE__, size).
struct Foo *p = ZeroAlloc(sizeof(struct Foo));

// SafeFree — free + null  (orcadef.h)
SafeFree(p);                     // if (p) { free(p); p = NULL; }

// SafeDelete — free with a custom destructor
SafeDelete(token, Token_Release); // if (token) { Token_Release(token); token = NULL; }

// SafeSet — free old value then assign new
SafeSet(str, strdup(new_str), free); // free(str); str = strdup(new_str);
```

---

## Iteration macros

### `FOR_LOOP` — indexed loop

```c
// orcadef.h
// FOR_LOOP(index_var, max_exclusive)
FOR_LOOP(i, cls->NumProperties) {
    struct PropertyType const *pd = &cls->Properties[i];
}
```

`i` is `uint32_t`. `max` is evaluated once. Do **not** modify `i` or `end`
inside the body.

### `FOR_EACH_LIST` — intrusive linked list

Every node struct has a `next` pointer of the same type.

```c
// orcadef.h
// FOR_EACH_LIST(type, var, list_head)
FOR_EACH_LIST(struct component, cmp, object->components) {
    // cmp is valid; cmp->next is pre-fetched into `next` before the body
    // Safe to free(cmp) inside the body.
}
FOR_EACH_LIST(struct Property, p, object->properties) {
    // p, p->pdesc, PROP_GetValue(p) all valid
}
```

### `FOR_EACH_PROPERTY` — walk a property list

```c
// orca.h
// FOR_EACH_PROPERTY(var, list_head)
FOR_EACH_PROPERTY(p, OBJ_GetProperties(object)) {
    PROP_Print(p, buf, sizeof(buf));
}
```

Equivalent to `FOR_EACH_LIST(struct Property, p, ...)`.

### `FOR_EACH_OBJECT` — walk an object's direct children

```c
// orca.h
// FOR_EACH_OBJECT(var, parent_object)
FOR_EACH_OBJECT(child, object) {
    OBJ_SendMessageW(child, ID_Node_UpdateGeometry, 0, NULL);
}
```

Safe to detach children inside the loop (next pointer cached before body).

### `FOR_EACH_LAYOUTABLE` — walk children that have a Node2D component

```c
FOR_EACH_LAYOUTABLE(child, parent) {
    // GetNode2D(child) is non-NULL here
    struct Node2D *n2d = GetNode2D(child);
}
```

### `FOR_EACH_CHILD` — apply a function to every child

```c
// orca.h
// FOR_EACH_CHILD(object, func, extra_args...)
FOR_EACH_CHILD(parent, OBJ_SetDirty);
FOR_EACH_CHILD(parent, _SendThemeChanged, .recursive = TRUE);
```

### `xmlForEach` — walk XML element children

```c
// orcadef.h — skips non-element nodes automatically
// xmlForEach(child_var, xml_node_ptr)
xmlForEach(c, x) {
    // c->type == XML_ELEMENT_NODE guaranteed
    xmlChar *val = xmlGetProp(c, XMLSTR("Name"));
}
```

### `xmlFindAll` — walk XML children matching a tag

```c
xmlFindAll(child, node, XMLSTR("property")) {
    // child->name == "property"
}
```

---

## RAII-style scope with `WITH`

```c
// orcadef.h
// WITH(type, var, init_expr, cleanup_func)
// Runs cleanup_func(var) at end of scope (or after break).
WITH(char, copy, strdup(input), free) {
    // copy is non-NULL; free(copy) is called when block exits
    process(copy);
}

WITH(xmlChar, text, xmlNodeGetContent(node), xmlFree) {
    if (text) handle((lpcString_t)text);
}
```

`WITH` is a single-iteration `for` loop. `break` exits cleanly (cleanup runs).
Nesting is fine. Do not `return` inside a `WITH` block — cleanup will not run.

---

## Component accessor macros — `Get*(object)`

Generated in `generated/core/core_properties.h` and `generated/UIKit/UIKit_properties.h`.
Each returns the component data pointer cast to the right struct type, or NULL
if the component is not attached:

```c
// Pattern: Get<ClassName>(object_ptr) → struct <ClassName>* or NULL
struct Node          *node   = GetNode(object);
struct Node2D        *n2d    = GetNode2D(object);
struct TextBlock     *tb     = GetTextBlock(object);
struct Button        *btn    = GetButton(object);
struct Screen        *scr    = GetScreen(object);
struct StackView     *sv     = GetStackView(object);
struct StyleController *sc   = GetStyleController(object);
struct Binding       *bnd    = GetBinding(object);    // Binding component
struct ItemsControl  *ic     = GetItemsControl(object);
```

Always null-check before dereferencing:

```c
struct Node2D *n2d = GetNode2D(object);
if (!n2d) return FALSE;
float w = n2d->Size.Axis[0].Actual;
```

The underlying call is `OBJ_GetComponent(object, ID_ClassName)`.

---

## Object class type check

```c
// orca.h
// OBJ_CLASS(object, ClassName) — true if object's first component is ClassName
if (OBJ_CLASS(object, TextBlock)) { ... }
if (OBJ_CLASS(object, Screen))    { ... }
```

`ID_<ClassName>` constants are in `core_properties.h` / `UIKit_properties.h`.

---

## Message dispatch — `_SendMessage` and `OBJ_SendMessageW`

### Typed dispatch — `_SendMessage`

The normal way to send a message with struct arguments.

```c
// orcadef.h
// _SendMessage(object, ClassName, EventName, .field = value, ...)
// Expands to OBJ_SendMessageW with a stack-allocated *EventName*Msg_t struct.

_SendMessage(object, StyleController, AddClasses, .ClassNames = "button primary");
_SendMessage(object, Object, PropertyChanged, .Property = prop);
_SendMessage(object, StyleController, ThemeChanged, .recursive = TRUE);
_SendMessage(child, Object, Start);   // no args — fields default to zero
```

The struct type is `<NS>_<Event>Msg_t` (typedef'd in `core.h`).
Its fields are the message's declared parameters from the `.cgen` definition.

### Raw dispatch — `OBJ_SendMessageW`

Use when you have a pre-hashed message ID and raw `lParam`:

```c
OBJ_SendMessageW(object, ID_Object_Start, 0, NULL);
OBJ_SendMessageW(object, ID_Binding_Compile, 0, prop_ptr);
```

`lParam` is `void*`. Cast to the expected struct pointer on the receiving side.

### String-based dispatch (avoid in hot paths)

```c
OBJ_SendMessage(object, "Node.UpdateGeometry", 0, NULL);
```

---

## Writing a message handler — `HANDLER`

```c
// orcadef.h
// HANDLER(ClassName, Namespace, EventName)
// Expands to:
//   LRESULT ClassName_EventName(
//       struct Object *hObject,
//       struct ClassName *pClassName,
//       wParam_t wParam,
//       NS_EventNameMsgPtr pEventName)

HANDLER(Button, Node, LeftButtonUp) {
    // hObject   — the owning Object
    // pButton   — pointer into the Button component's data block
    // wParam    — word param (usually 0)
    // pLeftButtonUp — struct Node_MouseMessageEventArgs*
    float x = pLeftButtonUp->x;
    float y = pLeftButtonUp->y;
    _SendMessage(hObject, Object, Start);
    return TRUE;   // handled; stops further dispatch
    // return FALSE;  // not handled; passes to next component
}

HANDLER(StyleController, StyleController, AddClasses) {
    // pAddClasses — struct StyleController_AddClassesEventArgs*
    lpcString_t names = pAddClasses->ClassNames;
    return TRUE;
}

HANDLER(Node2D, Object, Start) {
    // pStart is NULL (Object.Start carries no args)
    (void)pStart;
    return FALSE; // allow further processing
}
```

**Return value:** `TRUE` (non-zero) stops dispatch. `FALSE` (0) continues to
the next component in the chain.

The handler function must be referenced in the class's `ObjProc` dispatch table
(in the `*_export.c` generated file or a hand-written equivalent).

---

## Linked list management

```c
// orcadef.h
// ADD_TO_LIST(node, list_head) — prepend
ADD_TO_LIST(new_prop, object->properties);  // new_prop->next = object->properties; object->properties = new_prop;

// ADD_TO_LIST_END(type, node, list_head) — append
ADD_TO_LIST_END(struct component, comp, object->components);

// REMOVE_FROM_LIST(type, node, list_head)
REMOVE_FROM_LIST(struct component, cmp, object->components);
```

All require `node->next` to exist as a field of the same struct type.

---

## XML helpers

```c
// orcadef.h
#define XMLSTR(s) ((xmlChar *)(s))   // cast const char* → xmlChar*

// orca.h / fs_local.h — convenience wrappers over libxml2
_xmlSetProp(node, "Name", value);                         // xmlSetProp with XMLSTR casts
_xmlSetPropFormat(node, "Width", "%g", 42.0f);            // snprintf + xmlSetProp
_xmlNodeSetContent(node, text);                           // xmlNodeSetContent with XMLSTR
_xmlNodeSetContentFormat(node, "%d", count);

// Iterating XML children (orcadef.h)
xmlForEach(child, parent_node)   { /* element children only */ }
xmlFindAll(child, parent_node, XMLSTR("property")) { ... }
xmlFindAllText(child, text, parent_node, XMLSTR("value")) {
    // text is xmlChar*, auto-freed at scope exit
}
```

---

## Property access

### Finding a property — choose the right function

There are several find functions along two axes: *what you search through* and *what key you use*.

**From an `Object` — slot-creating, returns `Property*`:**

```c
// orca.h — allocates a property slot if not present, returns property pointer directly
struct Property *prop = OBJ_FindLongProperty(object, ID_Node2D_Width);
if (!prop)
    return;  // property not defined on this object

prop = OBJ_FindShortProperty(object, "Width");
if (!prop)
    return;  // searches by bare name — first matching component wins
             // ambiguous when multiple components have "Width"; prefer Long form
```

**Direct value read — no slot at all:**

```c
// orca.h — copies value into buf, returns bool_t
float w = 0;
OBJ_ReadProperty(object, ID_Node2D_Width, &w);
```

**Descriptor only — no slot, no value (internal/XML-loader use):**

```c
// Declared in core_local.h / object_component.c — not in public orca.h
struct PropertyType const *pd;
pd = OBJ_FindImplicitProperty(object, "Width");        // short name, first match
pd = OBJ_FindExplicitProperty(object, "Node2D.Width"); // full "Category.Name" string
```

The XML deserializer uses these before deciding how to parse an attribute.

**By slash-separated path (binding VM / internal):**

```c
// Resolves "Body/NavBar/Width", "./Opacity", "../Sibling/Color"
struct Property *p = OBJ_FindPropertyByPath(root, "Body/NavBar/Width");
```

**Summary:**

| Function | Key type | Creates slot? | Returns | Public? |
|---|---|---|---|---|
| `OBJ_FindLongProperty(obj, id)` | `uint32_t` FullIdentifier | Yes | `Property*` | Yes |
| `OBJ_FindShortProperty(obj, name)` | `lpcString_t` bare name | Yes | `Property*` | Yes |
| `OBJ_ReadProperty(obj, id, buf)` | `uint32_t` FullIdentifier | No | `bool_t` | Yes |
| `OBJ_FindImplicitProperty(obj, name)` | `lpcString_t` bare name | No | `PropertyType const*` | Internal |
| `OBJ_FindExplicitProperty(obj, name)` | `lpcString_t` "Cat.Name" | No | `PropertyType const*` | Internal |
| `OBJ_FindPropertyByPath(obj, path)` | `lpcString_t` slash path | No | `Property*` | Internal |

**Rule of thumb:** use `OBJ_FindLongProperty` with an `ID_*` constant in component code — unambiguous, creates the slot.

### Reading a property value

```c
struct Property *prop = OBJ_FindLongProperty(object, ID_Node2D_Width);
if (!prop)
    return;
if (PROP_IsNull(prop)) return;    // no value set
float width = *(float *)PROP_GetValue(prop);
```

`PROP_GetValue` returns `void const*`. Cast to the right type based on
`PROP_GetType(prop)` or `PROP_GetDesc(prop)->DataType`.

### Setting a property value

```c
// By name (allocates slot if needed)
float value = 100.0f;
OBJ_SetPropertyValue(object, "Width", &value);

// Via property pointer (fastest)
PROP_SetValue(prop, &value);
PROP_SetStringValue(prop, "hello");   // for kDataTypeString only
```

### Iterating all properties of an object (for serialization, introspection)

```c
// orca.h — walks every component and its properties
// Callback signature: (object, PropertyType, ClassDesc, component_data_ptr, user_param)
struct my_ctx { int count; };

static void count_cb(struct Object const *obj,
                     struct PropertyType const *pd,
                     struct ClassDesc const *cls,
                     void const *cdata,
                     void *param)
{
    ((struct my_ctx *)param)->count++;
}

struct my_ctx ctx = {0};
OBJ_EnumClassProperties(object, count_cb, &ctx);
```

**Do not use a block literal (`^{...}`)** — the callback is a plain C function
pointer.

### Formatting a property value to string

```c
// include/orca.h — added to fix broken PDESC_Print
// Returns TRUE when buf was populated. Returns FALSE for kDataTypeObject,
// kDataTypeEvent, or NULL value.
char buf[256];
if (OBJ_FormatPropertyValue(pd, value, buf, sizeof(buf)) && buf[0]) {
    // use buf
}
```

Handles `kDataTypeStruct` correctly via `OBJ_FindStructDesc(pd->TypeString)`.

---

## Binding expressions

### Attaching a binding at load time (XML deserializer pattern)

```c
// Attaches and compiles an expression to a named property
OBJ_AttachPropertyProgram(object, "Node2D.Width", "{Screen.Width} * 0.5",
                           kBindingModeExpression, TRUE);
```

### Checking and reading a binding

```c
if (PROP_HasProgram(prop)) {
    // Binding is compiled and active.
    lpcString_t expr = PROP_GetBindingExpression(prop);
    // expr is the original source string (e.g. "{Screen.Width} * 0.5")
    // May be NULL for bindings set before this field was introduced.
}
```

---

## StyleClass / CSS classes

```c
// Reading the class string from an object (include/orca.h)
char buf[1024];
lpcString_t classes = OBJ_GetRawStyleClasses(object, buf, sizeof(buf));
// classes is "button primary" or NULL if none set.

// Setting classes via message (normal path)
_SendMessage(object, StyleController, AddClasses, .ClassNames = "button primary");
_SendMessage(object, StyleController, AddClass,   .ClassName  = "hover");
```

---

## Generated code layout

### `*_properties.h` — ID constants and `Get*()` accessors

```
generated/core/core_properties.h     — core classes (Node, Node2D, Binding, …)
generated/UIKit/UIKit_properties.h   — UIKit classes (TextBlock, Button, Screen, …)
```

These define:
- `ID_ClassName` — `uint32_t` FNV1a hash of the class name
- `ID_ClassName_PropertyName` — FNV1a hash of `"ClassName.PropertyName"`
- `GetClassName(obj)` — typed component accessor macro

### `*.h` — struct definitions

```
generated/core/core.h     — struct Node, struct Node2D, struct Binding, message typedefs
generated/UIKit/UIKit.h   — struct TextBlock, struct Button, struct Screen, …
```

These define:
- `struct ClassName` — the component data block layout
- `ClassName_EventNameMsg_t` / `*MsgPtr` — message argument typedefs
- `struct ClassName_EventNameEventArgs` — message argument struct

### `*_export.c` — Lua bindings (do not edit by hand)

Generated from `.cgen` files. Contains registration, `luaopen_*`, and the
dispatch table for each class.

### `.cgen` — module definition source (XML)

```
source/core/core.cgen
plugins/UIKit/UIKit.cgen
```

Define classes, properties, messages, shorthands, and struct types.
Run `make` to regenerate the outputs.

---

## Message struct naming pattern

Given a class `Foo` and message `Bar`:

| What | Name |
|---|---|
| Message ID constant | `ID_Foo_Bar` |
| Argument struct | `struct Foo_BarEventArgs` |
| Typedef (value) | `Foo_BarMsg_t` |
| Typedef (pointer) | `Foo_BarMsgPtr` |
| Handler macro | `HANDLER(Foo, Foo, Bar)` — 3rd arg is message namespace |

The handler's last parameter is named `p<Event>` (`pBar` above) and has type
`Foo_BarMsgPtr`. When a message carries no arguments, the pointer is `NULL`.

Example from `core.h`:
```c
// Node.LeftButtonDown, Node.LeftButtonUp, Node.LeftButtonDragged all share:
struct Node_MouseMessageEventArgs {
    float x, y;           // pointer position in local space
    int32_t deltaX, deltaY;
    enum MouseButton button;
    int32_t clickCount;
};
typedef struct Node_MouseMessageEventArgs Node_LeftButtonDownMsg_t, *Node_LeftButtonDownMsgPtr;
```

---

## Object lifecycle

```c
// Create
struct Object *obj = OBJ_Create(ID_TextBlock);  // ID from *_properties.h

// Build hierarchy
OBJ_AddChild(parent, child);    // child->parent = parent; appended to parent->children

// Initialise (call after all children added and attributes set)
OBJ_SendMessageW(obj, ID_Object_Start, 0, NULL);

// Reference counting
OBJ_AddRef(obj);
OBJ_ReleaseRef(obj);   // frees when refcount hits zero

// Traversal
lpcString_t name       = OBJ_GetName(obj);
lpcString_t class_name = OBJ_GetClassName(obj);   // first component's ClassName
lpcString_t src_file   = OBJ_GetSourceFile(obj);
lpcString_t text       = OBJ_GetTextContent(obj);
struct Object *parent  = OBJ_GetParent(obj);
struct Object *child   = OBJ_GetFirstChild(obj);
struct Object *sibling = OBJ_GetNext(obj);
uint32_t flags         = OBJ_GetFlags(obj);        // OF_TEMPLATE, OF_DIRTY, …
OBJ_SetFlags(obj, flags | OF_DIRTY);

// Look up by name/path
struct Object *found = OBJ_FindByPath(root, "Body/NavBar");

// Type flags (OF_* in orca.h)
if (OBJ_GetFlags(obj) & OF_TEMPLATE) { /* prefab instance */ }
if (OBJ_GetFlags(obj) & OF_DIRTY)    { /* needs layout update */ }
```

---

## PropertyType descriptor fields (quick reference)

Every property is described by a `struct PropertyType` (defined in
`generated/geometry/geometry.h`):

```c
struct PropertyType {
    const char  *Name;           // bare name, e.g. "Width"
    const char  *Category;       // owner component, e.g. "Node2D"
    eDataType_t  DataType;       // kDataTypeBool / Int / Float / String / …
    const char  *TypeString;     // struct or class name for Struct/Object types
    const char **EnumValues;     // null-terminated array for kDataTypeEnum
    uint32_t     ShortIdentifier;// FNV1a("Width")
    uint32_t     FullIdentifier; // FNV1a("Node2D.Width")
    uint32_t     Offset;         // offsetof inside component user-data block
    uint32_t     DataSize;       // sizeof the field (or element size for arrays)
    bool_t       IsArray;        // true → value is {void *items; int count}
    bool_t       IsInherited;    // inherits from parent object
    bool_t       IsReadOnly;     // computed at runtime; skip in serialization
    bool_t       IsHidden;       // hidden from editor inspector
    float        Step, UpperBound, LowerBound; // editor hints
};
```

`eDataType_t` values (from `generated/geometry/geometry.h`):

| Constant | C type | Notes |
|---|---|---|
| `kDataTypeBool` | `bool_t` (int) | "true"/"false" |
| `kDataTypeInt` | `int32_t` | decimal |
| `kDataTypeEnum` | `int32_t` | index into `pd->EnumValues[]` |
| `kDataTypeFloat` | `float` | `%g` format |
| `kDataTypeString` | `char *` | heap pointer, never directly copy |
| `kDataTypeColor` | `float[4]` | RGBA, 0.0–1.0 |
| `kDataTypeStruct` | varies | use `OBJ_FindStructDesc(pd->TypeString)` |
| `kDataTypeObject` | `struct Object *` | reference, not inline |
| `kDataTypeEvent` | `event_t` | not serializable |

---

## Struct descriptors

For `kDataTypeStruct` properties, the struct layout is described by
`struct StructDesc`:

```c
struct StructDesc const *sd = OBJ_FindStructDesc("Thickness"); // e.g. "vec2", "color"
FOR_LOOP(i, sd->NumProperties) {
    struct PropertyType const *field = &sd->Properties[i];
    void const *fval = (char *)base + field->Offset;
    // field->DataType, field->DataSize, field->Name all valid
}
```

`OBJ_FindStructDesc` returns NULL for unknown names — always null-check.

---

## Common pitfalls

**1. Using `struct Binding` from `property_internal.h` vs. the generated one**
There are two `struct Binding` definitions:
- `generated/core/core.h` — the component data struct (XML `<Binding>` element).
  Fields: `Expression`, `Mode`, `Enabled`, `property`, `token`, `updateFrame`.
- The *Property*'s runtime binding is stored at `prop->binding` (type
  `struct Binding*`, same generated struct). `token` is the compiled VM tree;
  `Expression` is the original source string (populated by `PROP_SetBinding`).

Do **not** re-define `struct Binding` anywhere. Use the generated one.

**2. `FOR_LOOP` index type is `uint32_t`**
Comparing `uint32_t i` against a negative value is always true. Cast or use
`int` locals if you need signed comparisons.

**3. `WITH` and `return`**
`WITH` is a `for` loop. A `return` inside exits the function without running
the cleanup. Use `break` to exit early.

**4. `OBJ_GetClassName` returns the first component's class name**
For multi-inherited objects (`ListBox` inherits `StackView` and `ItemsControl`),
`OBJ_GetClassName` returns `"ListBox"` — the outermost/most-derived class.
This is what you want for the XML element tag.

**5. `OBJ_EnumClassProperties` takes a plain C function pointer**
Do not pass a block literal (`^{}`). Use a helper function + context struct:

```c
struct ctx { int n; };
static void cb(struct Object const *o, struct PropertyType const *pd,
               struct ClassDesc const *cls, void const *data, void *param) {
    ((struct ctx *)param)->n++;
}
struct ctx c = {0};
OBJ_EnumClassProperties(object, cb, &c);
```

**6. String properties are heap-allocated `char *`**
`PROP_GetValue` for `kDataTypeString` returns `void const*` pointing to a
`char *` slot (pointer-to-pointer). Dereference once to get the string:

```c
char **slot = (char **)PROP_GetValue(prop);
lpcString_t s = slot ? *slot : NULL;
```

`PROP_SetValue` for a string property **copies** the string internally.
`PROP_SetStringValue` is the clean wrapper.

**7. `OF_TEMPLATE` objects should serialize as placeholders**
Objects with `OBJ_GetFlags(obj) & OF_TEMPLATE` were loaded from a prefab.
`OBJ_GetSourceFile(obj)` returns the prefab path. Emit
`<LayerPrefabPlaceholder PlaceholderTemplate="..."/>`, not the expanded tree.

**8. `IsReadOnly` properties must be skipped in serialization**
Properties like computed layout positions are marked `pd->IsReadOnly = TRUE`.
Writing them back produces no-ops at best and parse errors at worst.
Always check `if (pd->IsReadOnly) continue;` when iterating properties for
output.

---

## Minimal handler skeleton

```c
#include <include/api.h>
#include <core/core_properties.h>
#include <UIKit/UIKit_properties.h>
#include <UIKit/UIKit.h>

// Called when a Button receives Node.LeftButtonUp
HANDLER(Button, Node, LeftButtonUp)
{
    (void)wParam;
    if (!pLeftButtonUp) return FALSE;

    struct Node2D *n2d = GetNode2D(hObject);
    if (!n2d) return FALSE;

    Con_Printf("click at %.1f, %.1f", pLeftButtonUp->x, pLeftButtonUp->y);

    // Change a property
    float new_width = 200.0f;
    OBJ_SetPropertyValue(hObject, "Width", &new_width);

    // Send a message up the tree
    _SendMessage(OBJ_GetParent(hObject), Object, PropertyChanged,
                 .Property = NULL);

    return TRUE; // stop dispatch
}
```

---

## Key file locations

| File | Purpose |
|---|---|
| `include/orca.h` | Public API: OBJ_, PROP_, FS_, type macros |
| `include/orcadef.h` | Core macros: HANDLER, _SendMessage, FOR_*, WITH, ZeroAlloc, … |
| `include/orcaerror.h` | SUCCEEDED, FAILED, S_OK, E_FAIL |
| `generated/core/core.h` | struct Node, struct Binding, message typedefs |
| `generated/core/core_properties.h` | ID_* constants, GetNode/GetNode2D/… accessors |
| `generated/UIKit/UIKit.h` | struct TextBlock, struct Button, struct Screen, … |
| `generated/UIKit/UIKit_properties.h` | UIKit ID_* and GetTextBlock/GetButton/… |
| `generated/geometry/geometry.h` | eDataType_t enum, struct PropertyType definition |
| `source/core/core_local.h` | Private core types (struct game, MAX_* limits) — only for core/ |
| `source/filesystem/fs_xml.c` | XML deserializer (reference for serializer mirror) |
| `source/filesystem/fs_xml_write.c` | XML serializer |
| `docs/contributing/xml-serialization-guide.md` | XML serialization specifics |
