# Object + Component System

ORCA uses an **Object/Component** architecture. An `Object` is the universal scene node — a named, hierarchical container. Functionality is added by attaching **components** (also called classes) to it.

---

## Objects

Every scene element is an `Object` (defined in `source/core/object/object_internal.h`). Objects form a parent → children tree and carry:

- A **name**, class name, and source file path
- A linked list of attached **components** (via the `components` field in the union below)
- A flat byte buffer (`data[]`) for property value storage
- A **Lua state** pointer for scripting

Objects are created and destroyed via the Lua API (`orca.Object()`) or loaded from XML project files. Hierarchy is manipulated with `OBJ_AddChild`, `OBJ_RemoveFromParent`, and related functions.

### Object struct layout

```c
struct Object {
    LPSTR Name;
    uint32_t identifier;        // FNV1a hash of Name

    lpObject_t parent;
    lpObject_t children;        // first child (singly-linked)
    lpObject_t next;            // next sibling

    LPSTR SourceFile;
    LPSTR TextContent;
    LPSTR ClassName;

    // ── Legacy fields being migrated to components ──────────────────────
    lpcKeyframeAnim_t animation; // → AnimationPlayer component
    lpKeyframeAnim_t  animlib;   // → AnimationPlayer component

    union {
        struct {
            struct component*          components;   // component chain
            struct property_animation* animations;   // → PropertyAnimation component
            struct Property*           properties;
            struct state_manager*      stateManager; // → StateManager component
            struct style_class*        classes;      // → StyleClass component
            struct style_sheet*        stylesheet;   // → StyleSheet component
            struct timer*              timers;        // → Timer component
            struct alias*              aliases;       // → Alias component
        };
        void* comps[kCompCount]; // indexed access by enum component_type
    };
    // ────────────────────────────────────────────────────────────────────

    uint32_t alias, unique, userdata, luaObject, flags, rdflags, datasize;
    objectTags_t tags;
    longTime_t dirty;
    lua_State *domain;

    byte_t data[MAX_OBJECT_DATA]; // property values are stored here
};
```

The `comps[kCompCount]` array allows O(1) access to the most common sub-systems by index (`kCompComponents`, `kCompAnimations`, etc.). New functionality should be implemented as proper components and accessed via `OBJ_GetComponent(obj, ID_ClassName)` instead of adding new fields to this struct.

---

## Components

A component is a block of typed data plus a **message-handler function** (`ObjProc`) that is attached to an Object. When a message is dispatched to an Object, the engine walks its component list and calls each handler in turn until one returns non-zero.

Each component type is described by a `ClassDesc` struct (`include/orca.h`):

```c
struct ClassDesc {
    objectProc_t      ObjProc;          // message handler function
    lpcPropertyType_t Properties;       // property descriptor table
    lpcString_t       ClassName;        // human-readable name, e.g. "Button"
    lpcString_t       DefaultName;      // used when no name is supplied
    lpcString_t       ContentType;      // for package auto-detection
    lpcString_t       Xmlns;            // XML namespace
    uint32_t          ParentClasses[16];// base class IDs, zero-terminated
    uint32_t          NumProperties;    // property count
    uint32_t          ClassID;          // FNV1a hash of ClassName
    uint32_t          ClassSize;        // sizeof(struct ClassName)
    uint32_t          MemorySize;       // total size including parents
    void const       *Defaults;         // default values struct
    bool_t            IsAttachOnly;     // cannot be instantiated standalone
};
```

### The `objectProc_t` signature

```c
typedef LRESULT (*objectProc_t)(lpObject_t obj,
                                void*       cmp,
                                uint32_t    message,
                                wParam_t    wParam,
                                lParam_t    lParam);
```

| Parameter | Description |
|---|---|
| `obj` | The owning Object |
| `cmp` | Component user-data block (`cmp->pUserData`) — cast implicitly to `struct ClassName*` |
| `message` | Message ID already masked with `MSG_DATA_MASK` |
| `wParam` | Integer parameter |
| `lParam` | Pointer to message arguments struct |

The dispatcher in `component.c` calls every component's Proc in sequence:

```c
FOR_EACH_LIST(struct component, cmp, _GetComponents(pobj)) {
    if (cmp->pcls->ObjProc) {
        LRESULT res = cmp->pcls->ObjProc(pobj, cmp->pUserData,
                                          MsgID, wParam, lParam);
        if (res) return res;
    }
}
```

### Registering a component type

Call `OBJ_RegisterClass(&_ClassName)` in the module's `on-luaopen` callback. In practice the `REGISTER_CLASS` macro (generated in `*_export.c`) produces the `ClassDesc` instance and `on_module_registered` calls `OBJ_RegisterClass` on it.

### Attaching a component to an object

```c
OBJ_AddComponent(obj, ID_Button);       // attach by ClassID hash (C API)
obj:addComponent("AnimationPlayer")     -- attach by name (Lua API)
```

The engine allocates `sizeof(struct component) + ClassSize` bytes, copies `Defaults` into the new block, appends it to the object's component list, and then recursively attaches any `ParentClasses`.

### Standalone vs. attach-only

| Kind | XML attribute | Macro | Use |
|---|---|---|---|
| Standalone | *(default)* | `REGISTER_CLASS` | Can be created as a root object (e.g. `AnimationClip`) |
| Attach-only | `attach-only="true"` | `REGISTER_ATTACH_ONLY_CLASS` | Can only be added to an existing object (e.g. `AnimationPlayer`, `AnimationCurve`) |

Attach-only components set `IsAttachOnly = TRUE` in their `ClassDesc`. `lua_pushclass` uses this flag to suppress standalone constructor generation.

### Inheritance

`ParentClasses` lists base class IDs. Attaching a component also attaches every listed parent (if not already present), so inherited properties and message handling are automatically available.

---

## Properties

Properties are named, typed values stored in a component's data block and exposed to Lua. Each is described by a `PropertyType` entry in the component's `Properties` array:

```c
struct PropertyType {
    const char  *Name;             // bare property name, e.g. "Speed"
    const char  *Category;         // component name, e.g. "AnimationPlayer"
    enum DataType DataType;         // kDataTypeFloat, kDataTypeString, …
    const char  *TypeString;        // struct/object type name for kDataTypeStruct/Object
    const char **EnumValues;        // null-terminated string array for kDataTypeEnum
    uint32_t     ShortIdentifier;   // FNV1a("Speed")
    uint32_t     FullIdentifier;    // FNV1a("AnimationPlayer.Speed") | routing bits
    uint32_t     Offset;            // offsetof(struct AnimationPlayer, Speed)
    uint32_t     DataSize;          // sizeof(float)
    bool_t       IsArray;           // true for array properties
    // … AffectLayout, AffectRender, IsReadOnly, …
};
```

The `DECL` and `ARRAY_DECL` macros in `*_export.c` build these entries. See [Macros Reference](macros-reference.md) for details.

Property values are stored inside the Object's flat `data[]` buffer. Each component property's `Offset` is relative to the component's user-data block, which itself sits inside `data[]`.

---

## Message Dispatch

Messages are `uint32_t` constants with routing bits in the lower 2 bits. Core messages are defined in `source/core/core_properties.h` (generated from `core.xml`):

| Message | When sent |
|---|---|
| `ID_Object_Create` | Component just attached |
| `ID_Object_Start` | All children loaded / object fully initialised |
| `ID_Object_Animate` | Per-frame animation tick |
| `ID_Object_Destroy` | Object being destroyed |
| `ID_Object_Timer` | Timer tick |
| `ID_Object_PropertyChanged` | A property value changed |

Use `_SendMessage` to dispatch with typed arguments:

```c
_SendMessage(object, Object, Start);
_SendMessage(object, Object, PropertyChanged, .Property = myProp);
```

Custom messages are declared with `<message>` in any module XML. The `routing=` attribute controls bubbling/tunneling behaviour.

See [Macros Reference](macros-reference.md) for full documentation of `HANDLER`, `_SendMessage`, and related macros.

---

## Object Struct Refactoring Direction

The current `struct Object` contains a number of embedded subsystem fields — `timers`, `animations`, `stateManager`, `stylesheet`, `classes`, `aliases` — that predate the component architecture. These are **legacy fields** that violate the principle that functionality belongs in components.

**The goal:** every subsystem moves into a proper `attach-only` component:

| Legacy field | Target component |
|---|---|
| `animation` / `animlib` | `AnimationPlayer` / `AnimationClip` |
| `animations` (property tweens) | `PropertyAnimation` |
| `timers` | `Timer` |
| `stateManager` | `StateManager` |
| `stylesheet` / `classes` | `StyleSheet` / `StyleClass` |
| `aliases` | `Alias` |

Migration rules:
1. Implement the component with `<handles>` for the messages it needs.
2. Register it as `attach-only` via `REGISTER_ATTACH_ONLY_CLASS`.
3. Remove the corresponding field from `struct Object`.
4. Remove the `_GetXxx(obj)` macro and the manual release call from `OBJ_Release`.

The `AnimationPlayer` / `AnimationClip` / `AnimationCurve` set is the first completed migration of this kind.

---

## How to Add a New Component Type

### 1. Declare in XML

```xml
<class name="MyComponent" attach-only="true">
  <summary>What this component does.</summary>
  <handles>
    <handle message="Object.Start"/>
    <handle message="Object.Animate"/>
  </handles>
  <properties>
    <property name="Speed" type="float" default="1.0">Movement speed</property>
  </properties>
</class>
```

Use `attach-only="true"` if the component must be added to an existing object rather than created standalone.

### 2. Regenerate bindings

```bash
cd tools && make
```

Produces `<module>.h`, `<module>_properties.h`, and `<module>_export.c`.

### 3. Write the C implementation

Create `source/<module>/components/MyComponent.c`:

```c
#include <source/core/core_local.h>
#include "<module>_properties.h"

HANDLER(MyComponent, Object, Start) {
    pMyComponent->Speed = 1.0f;
    return FALSE;
}

HANDLER(MyComponent, Object, Animate) {
    if (!pMyComponent->Speed) return FALSE;
    // per-frame logic here
    return FALSE;
}
```

The `HANDLER` macro provides the correct function signature; the `*_export.c` forward-declares it and wires it into the generated `MyComponentProc` switch. See [Macros Reference](macros-reference.md).

### 4. Register at module init

The `REGISTER_CLASS` / `REGISTER_ATTACH_ONLY_CLASS` macro in `*_export.c` defines the `ClassDesc`. Wire it up in the `on-luaopen` callback:

```c
void on_mymodule_registered(lua_State *L) {
    OBJ_RegisterClass(&_MyComponent);
}
```

### 5. Use from Lua

```lua
-- Standalone class:
local clip = orca.AnimationClip()
clip.StopTime = 2.0

-- Attach-only:
obj:addComponent("AnimationPlayer")
obj.Clip    = clip
obj.Playing = true
```
