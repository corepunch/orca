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

    struct component*     components;   // component chain
    struct Property*      properties;

    uint32_t alias, unique, userdata, luaObject, flags, rdflags, datasize;
    objectTags_t tags;
    longTime_t dirty;
    lua_State *domain;

    byte_t data[MAX_OBJECT_DATA]; // extra property values stored here
};
```

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
| `cmp` | Pointer directly to the component instance data (passed from the component header's `pUserData` field) — cast to `struct ClassName*` |
| `message` | Message ID as forwarded by `OBJ_SendMessageW` — may include routing bits; generated `*Proc` switches mask with `MSG_DATA_MASK` |
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
| Standalone | *(default)* | `REGISTER_CLASS` | May be created as a root object or attached as a component (e.g. `AnimationClip`) |
| Attach-only | `attach-only="true"` | `REGISTER_ATTACH_ONLY_CLASS` | Intended to be attached to an existing object via `addComponent` (e.g. `AnimationPlayer`) |

Attach-only components set `IsAttachOnly = TRUE` in their `ClassDesc`. This is currently an architectural convention: `OBJ_AddComponentByName` (the Lua bridge) checks `IsAttachOnly` and raises an error if a non-attach-only class is passed; however, direct C callers of `OBJ_AddComponent(pobj, class_id)` are not restricted.

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

Property values for component-defined properties are stored in the component's user-data block (heap-allocated alongside the `struct component` header). The `Offset` field is relative to the component's `pUserData` start. The object's `data[]` flat buffer provides overflow storage for dynamic/extra properties not tied to a specific component.

---

## Message Dispatch

Messages are `uint32_t` constants with routing bits in the lower 2 bits. Core messages are defined in `source/core/core_properties.h` (generated from `core.xml`):

| Message | When sent |
|---|---|
| `ID_Object_Create` | Object has been created (sent once to all initially attached components) |
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

The current `struct Object` contains a number of embedded subsystem fields — `timers`, `stateManager`, `stylesheet`, `classes`, `aliases` — that predate the component architecture. These are **legacy fields** that violate the principle that functionality belongs in components.

**The goal:** every subsystem moves into a proper `attach-only` component:

| Legacy field | Target component | Status |
|---|---|---|
| `animation` / `animlib` | `AnimationPlayer` / `AnimationClip` | **Done** — fields removed |
| `animations` (property tweens) | `PropertyAnimation` | **Done** — field removed |
| `timers` | `Timer` | Planned |
| `stateManager` | `StateManager` | Planned |
| `stylesheet` / `classes` | `StyleSheet` / `StyleClass` | Planned |
| `aliases` | `Alias` | Planned |

Migration rules:
1. Implement the component with `<handles>` for the messages it needs.
2. Register it as `attach-only` via `REGISTER_ATTACH_ONLY_CLASS`.
3. Remove the corresponding field from `struct Object`.
4. Remove the `_GetXxx(obj)` macro and the manual release call from `OBJ_Release`.

---

## How to Add a New Component Type

> **Rule: XML → codegen → handlers → Xcode project → tests.**  Do not skip or reorder any step.  Skipping the XML step means `struct MyComponent`, the message IDs, and the `REGISTER_*` macro are never generated — the C file will not compile.  Skipping tests means silent integration failures that are invisible until runtime.

### 1. Declare in the module XML

Add a `<class>` entry to the relevant module `.xml` file (e.g. `source/core/core.xml`). Every message handler must have a matching `<handle>` entry, and every message the component dispatches must be declared under `<messages>`.  **Handlers without `<handle>` entries are orphaned — the generated Proc switch will not contain them and they will never be called.**

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
  <messages>
    <message name="SpeedChanged" routing="Direct">
      <fields>
        <field name="NewSpeed" type="float"/>
      </fields>
    </message>
  </messages>
</class>
```

Use `attach-only="true"` for components that augment existing objects; omit it for standalone data objects (like `AnimationClip`) that may be created as root objects.

### 2. Regenerate bindings

```bash
cd tools && make
```

This regenerates `<module>.h` (struct + accessor macro), `<module>_properties.h` (FNV hash IDs), and `<module>_export.c` (Proc switch + REGISTER macro + Lua bindings). **After regenerating, open `<module>_export.c` and verify that `MyComponentProc` contains `case` entries for every declared message.** An empty `switch {}` means the XML step was incomplete.

### 3. Write the C handlers

Create `source/<module>/components/MyComponent.c`. The file includes only the module-local header and the generated properties header:

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

The `HANDLER` macro provides the correct function signature. `*_export.c` forward-declares each handler and wires it into `MyComponentProc`. See [Macros Reference](macros-reference.md).

> **Do not include plugin headers from core.** `source/core/` must never `#include <plugins/UIKit/UIKit.h>` or any other plugin header. The dependency direction is plugins → core, not the reverse.

### 4. Register the class at module init

The `REGISTER_CLASS` / `REGISTER_ATTACH_ONLY_CLASS` macro in `*_export.c` defines `_MyComponent`.  Wire it into the module's `on-luaopen` callback:

```c
OBJ_RegisterClass(&_MyComponent);
```

### 5. Register the file in the Xcode project

New `.c` files must be added to `orca.xcodeproj/project.pbxproj` in **four** places — `PBXBuildFile`, `PBXFileReference`, `PBXGroup` (components group), and `PBXSourcesBuildPhase`.  Omitting this step silently excludes the file from the macOS/Xcode build.

### 6. Write tests

Add a Lua test file `tests/test_mycomponent.lua` covering:
- Object creation, component attachment
- Property set/get round-trips
- At least one message dispatch with observable side-effects

Run tests with:

```bash
make test-headless          # layout/property tests, no display
xvfb-run make test          # full suite, needs virtual framebuffer
```

Without tests, an empty `MyComponentProc` (caused by missing `<handle>` entries in XML) will silently do nothing — impossible to distinguish from correct code that simply has no visible output.

### 7. Use from Lua

```lua
-- Standalone class:
local clip = orca.AnimationClip()
clip.StopTime = 2.0

-- Attach-only:
obj:addComponent("AnimationPlayer")
obj.Clip    = clip
obj.Playing = true
```

---

## Common Component Development Pitfalls

| Anti-pattern | Symptom | Fix |
|---|---|---|
| Writing C handlers without a `<class>` in XML | Does not compile: `struct MyComponent` undefined | Write the XML first, then run codegen |
| `<class>` declared but no `<handle>` entries | Component attaches but does nothing silently | Add `<handle message="..."/>` for every handler |
| Old global implementation left alongside new component | Duplicate paths; component does nothing, old code still runs | Delete the old code path once the component is complete |
| `#include <plugins/...>` inside `source/core/` | Circular build dependency; breaks plugin-free builds | Remove the include; use a message or registered accessor instead |
| No test file | Silent regressions and empty-Proc bugs undetected | Always write a Lua test alongside the implementation |
| Xcode project not updated | macOS/Xcode build fails with missing symbol | Add the file to `project.pbxproj` in all four sections |
