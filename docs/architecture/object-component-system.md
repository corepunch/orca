# Object + Component System

ORCA uses an **Object/Component** architecture. An `Object` is the universal scene node — a named, hierarchical container. Functionality is added by attaching **components** (also called classes) to it.

---

## Objects

Every scene element is an `Object` (`source/core/object.c`). Objects form a parent → children tree and carry:

- A **name**, class name, and source file path
- A linked list of attached **components**
- A flat byte buffer (`data[]`) for property storage
- A **Lua state** pointer for scripting

Objects are created and destroyed via the Lua API (`orca.create`) or loaded from XML project files. Hierarchy is manipulated with `OBJ_AddChild`, `OBJ_RemoveChild`, and related functions defined in `source/core/core.xml`.

---

## Components

A component is a block of typed data plus a **message-handler function** (`ObjProc`) that is attached to an Object. When an event or message is dispatched to an Object, the engine walks its component list and calls each handler in turn.

Each component type is described by a `ClassDesc` struct (`include/orca.h`):

```c
struct ClassDesc {
  objectProc_t ObjProc;         // Message handler — dispatches kMsg* messages
  lpcPropertyType_t Properties; // Sorted array of property descriptors
  lpcString_t ClassName;        // Human-readable name, e.g. "Button"
  uint32_t ParentClasses[16];   // Inherited class IDs (ordered most→least derived)
  uint32_t NumProperties;       // Number of properties defined on this class
  uint32_t ClassID;             // FNV1a hash of ClassName — used for O(1) lookups
  uint32_t ClassSize;           // Size of the per-instance data block
  void const *Defaults;         // Pointer to a struct with default property values
};
```

### Registering a component type

Call `OBJ_RegisterClass(&desc)` at module initialisation time to register a `ClassDesc` with the engine. The engine stores all registered classes in a hash table keyed by `ClassID`.

### Attaching a component to an object

```c
OBJ_AddComponent(obj, ID_Button);  // attach by ClassID hash
```

The engine allocates `sizeof(struct component) + ClassSize` bytes, copies `Defaults` into the new instance, appends it to the object's component linked list, and then sends `kMsgCreate` to the new component.

### Inheritance

Set `ParentClasses` to list the `ClassID`s of base classes. When a component is attached, the engine also attaches all parent classes (if not already present), ensuring that inherited properties and message handling are available.

---

## Properties

Properties are named, typed values stored in a component's data block and exposed to Lua. Each property has:

- A **name** (string, hashed to `uint32_t` with FNV1a)
- A **type** (float, int, string, vec2, vec3, color, …)
- An **offset** within the component's data block
- Optional **default value** (from the `Defaults` struct)

Property lookup goes through the component list: `OBJ_FindShortProperty` searches by name, `OBJ_FindLongProperty` accepts a fully-qualified `ClassName.PropertyName` form.

---

## Message Dispatch

Messages and events are `uint32_t` constants defined in `source/core/core_properties.h` (generated from `core.xml`). The most common ones are:

| Message | When sent |
|---|---|
| `kMsgCreate` | Component just attached to an object |
| `kMsgDestroy` | Component about to be detached / object destroyed |
| `kMsgDraw` | Render frame — component should submit draw calls |
| `kMsgUpdate` | Per-frame update tick |
| `kMsgMouseUp` | Mouse/touch release over this object |
| `kMsgMouseDown` | Mouse/touch press over this object |
| `kMsgHitTest` | Pointer-event hit test — lParam is `HitTestMsgArgs*` with `x`/`y` inputs; returns the hit object as `LRESULT` |

Use `_SendMessage` with named field initializers to dispatch messages that carry input parameters. When the result is returned via `LRESULT` (like `OpenFile`, `HitTest`), cast the return value to the expected pointer type:

```c
// Simple containment check — result discarded
if (_SendMessage(view, HitTest, .x = px, .y = py)) {
    /* some object was hit */
}

// Full hit test — retrieve which object was hit via LRESULT cast
lpObject_t hit = (lpObject_t)_SendMessage(root, HitTest, .x = px, .y = py);
```

Custom messages can be declared in any module's `.xml` file using the `<message>` element and will be hashed into the same `uint32_t` space.

---

## How to Add a New Component Type

### 1. Declare in XML

Add a `<class>` element to your module's `.xml` file:

```xml
<class name="MyComponent" export="MyComponent">
  <summary>Moves an object at a configurable speed.</summary>
  <property name="Speed" type="float">Movement speed (units/sec)</property>
  <property name="Direction" type="vec3">Movement direction (unit vector)</property>
</class>
```

### 2. Regenerate bindings

```bash
cd tools && make
```

This runs the pyphp templates and writes:

- `<module>.h` — C type definitions and Lua push/check helpers
- `<module>_properties.h` — `ID_MyComponent`, `GetMyComponent(obj)` macro, property hash constants
- `<module>_export.c` — Lua wrapper functions and `luaopen_orca_<module>()`

### 3. Write the C implementation

```c
#include "mymodule_properties.h"

struct MyComponent {
    float Speed;
    vec3_t Direction;
};

static LRESULT MyComponent_Proc(lpObject_t obj, uint32_t msg,
                                 wParam_t wParam, lParam_t lParam) {
    struct MyComponent *self = GetMyComponent(obj);
    switch (msg) {
        case kMsgCreate:
            self->Speed     = 1.0f;
            self->Direction = (vec3_t){ 0, 0, 1 };
            break;
        case kMsgUpdate:
            /* move the object each frame */
            break;
    }
    return 0;
}

static const struct MyComponent defaults = {
    .Speed     = 1.0f,
    .Direction = { 0, 0, 1 },
};

static ClassDesc MyComponentClass = {
    .ObjProc       = MyComponent_Proc,
    .ClassName     = "MyComponent",
    .ClassID       = ID_MyComponent,        /* generated hash */
    .ClassSize     = sizeof(struct MyComponent),
    .Defaults      = &defaults,
    .ParentClasses = { ID_Node, 0 },        /* inherit from Node */
};
```

### 4. Register at module init

```c
void on_mymodule_registered(lua_State *L) {
    OBJ_RegisterClass(&MyComponentClass);
}
```

The `on-luaopen` attribute in `<module name="mymodule" on-luaopen="on_mymodule_registered">` wires this callback to the generated `luaopen_orca_mymodule()`.

### 5. Use from Lua

The Lua bindings are auto-generated, so the new component is immediately usable:

```lua
local obj = orca.create("MyComponent")
obj.Speed     = 5.0
obj.Direction = vec3(1, 0, 0)
```
