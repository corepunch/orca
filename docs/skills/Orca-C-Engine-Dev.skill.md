---
name: Orca C Engine Dev
summary: >
  Deep C engine development for Orca: filesystem, .cgen classes, property
  system, object/component lifecycle, message dispatch, bindings, triggers,
  and actions. Includes the full codegen reference and generated class index.
---

## Purpose

Expert-level C development on the Orca engine. Covers:

- Adding/changing classes via `.cgen` + codegen
- Property system (read, write, iterate, format, bind)
- Object and component lifecycle
- Message dispatch (HANDLER, _SendMessage, routing bits)
- Bindings and binding expressions
- Triggers and Actions (EventTrigger, OnPropertyChangedTrigger, Setter, HideAction)
- Filesystem / XML serialization

---

## Primary References (read before editing any .c file)

| Document | What it covers |
|---|---|
| `docs/contributing/codegen-guide.md` | **Single canonical C reference** — headers, macros, patterns, pitfalls |
| `docs/architecture/object-component-system.md` | Object/Component model, ClassDesc, PropertyType, lifecycle, how-to-add-a-class |
| `docs/architecture/macros-reference.md` | Complete macro reference: HANDLER, _SendMessage, FOR_*, DECL, WITH, … |
| `docs/architecture/message-actions.md` | Trigger→Action→SendMessageAction dispatch model |
| `docs/contributing/xml-serialization-guide.md` | XML serialization implementation details |
| `docs/architecture/code-generation.md` | .cgen → make modules → generated files pipeline |
| `docs/ADDING_CLASSES.md` | Mandatory checklist for adding or changing components |
| `source/core/core.cgen` | Authoritative class/message/property source for core module |
| `plugins/UIKit/UIKit.cgen` | Authoritative class/message/property source for UIKit |
| `docs/generated/` | Auto-generated per-class markdown — class index, properties, messages |

---

## Header Map

| What you need | Include |
|---|---|
| Everything public (OBJ_, PROP_, FS_, types) | `#include <include/orca.h>` |
| Public API + Lua bindings | `#include <include/api.h>` |
| Inside `source/core/` files | `#include <source/core/core_local.h>` |
| Inside `source/filesystem/` files | `#include <source/filesystem/fs_local.h>` |
| Generated class/property IDs + Get*() accessors | `#include <core/core_properties.h>` or `#include <UIKit/UIKit_properties.h>` |
| Generated struct definitions | `#include <core/core.h>`, `#include <UIKit/UIKit.h>` |
| Plugin implementations | `#include <include/api.h>` + generated headers for your plugin |

---

## Adding a New Class — Mandatory Workflow

> Rule: `.cgen` → codegen → handlers → Xcode project → tests. Never skip or reorder.

### 1. Declare in the module `.cgen`

```xml
<class name="MyComponent">
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

Use `component="true"` for components that augment existing objects (like AnimationPlayer).
Omit it for standalone objects (like AnimationClip) that can be root XML elements.

Every `<handle>` entry produces one switch case in the generated Proc. Handlers without `<handle>` entries are **silently orphaned** — they compile but are never called.

### 2. Regenerate

```bash
make modules
```

Regenerates `<module>.h`, `<module>_properties.h`, and `<module>_export.c`.
After regenerating, verify that `MyComponentProc` in `*_export.c` contains case entries for every declared message.

### 3. Write the C handlers

File: `source/<module>/components/MyComponent.c`

```c
#include <source/core/core_local.h>
#include "core_properties.h"

HANDLER(MyComponent, Object, Start) {
    pMyComponent->Speed = 1.0f;
    return FALSE;
}

HANDLER(MyComponent, Object, Animate) {
    if (!pMyComponent->Speed) return FALSE;
    return FALSE;
}
```

> Do NOT include plugin headers from `source/core/`. Dependency direction is plugins → core, never reverse.

### 4. Register the class

In the module's `on-luaopen` callback:

```c
OBJ_RegisterClass(&_MyComponent);
```

### 5. Register in Xcode project

Add the `.c` file to `orca.xcodeproj/project.pbxproj` in **four** places: PBXBuildFile, PBXFileReference, PBXGroup (components group), PBXSourcesBuildPhase.

### 6. Write tests

`tests/test_mycomponent.lua` — cover creation, property round-trips, message dispatch.

```bash
make test-headless   # no display
xvfb-run make test   # full suite
```

---

## Message System

### HANDLER macro

```c
// HANDLER(ClassName, Namespace, EventName)
// Expands to:
//   LRESULT ClassName_EventName(struct Object *hObject,
//                               struct ClassName *pClassName,
//                               wParam_t wParam,
//                               NS_EventNameMsgPtr pEventName)

HANDLER(Button, Node, LeftButtonUp) {
    float x = pLeftButtonUp->x;
    float y = pLeftButtonUp->y;
    _SendMessage(hObject, Object, Start);
    return TRUE;   // handled; stops dispatch
    // return FALSE;  // not handled; passes to next component
}
```

Parameters always available inside HANDLER:
- `hObject` — the owning Object
- `p<ClassName>` — the component data block (cmp already cast)
- `wParam` — integer extra parameter
- `p<EventName>` — typed pointer to the message arguments struct (may be NULL for arg-less messages)

### _SendMessage — typed dispatch

```c
// _SendMessage(object, ClassName, EventName, .field = value, ...)
_SendMessage(object, Object, PropertyChanged, .Property = myProp);
_SendMessage(object, Object, Start);   // no args
_SendMessage(child, StyleController, AddClasses, .ClassNames = "button primary");
```

### OBJ_SendMessageW — raw dispatch

```c
OBJ_SendMessageW(object, ID_Object_Start, 0, NULL);
OBJ_SendMessageW(object, ID_Binding_Compile, 0, prop_ptr);
```

### Message struct naming

Given class `Foo` and message `Bar`:

| What | Name |
|---|---|
| Message ID constant | `ID_Foo_Bar` |
| Argument struct | `struct Foo_BarEventArgs` |
| Typedef (value) | `Foo_BarMsg_t` |
| Typedef (pointer) | `Foo_BarMsgPtr` |
| Handler macro | `HANDLER(Foo, Foo, Bar)` |

### Message routing bits

```c
#define MSG_ROUTING_MASK           3u
#define MSG_DATA_MASK              (~MSG_ROUTING_MASK)
#define ROUTING_BUBBLE             0u
#define ROUTING_TUNNELING_BUBBLING 1u
#define ROUTING_TUNNELING          2u
#define ROUTING_DIRECT             3u
```

Generated Proc switches always mask: `case ID_Object_Start & MSG_DATA_MASK:`

The `routing=` attribute on `<message>` in .cgen sets the lower bits at codegen time.

### Adding a new message

1. Add `<message>` to the class in the `.cgen` file with `<fields>` for any arguments.
2. Add `<handle message="ClassName.EventName"/>` inside the receiving class.
3. Run `make modules`.
4. Write the HANDLER in the `.c` file.

### Core lifecycle messages

| Message | When sent |
|---|---|
| `ID_Object_Create` | Object created (sent once to all initially attached components) |
| `ID_Object_Start` | All children loaded / object fully initialised — **sent automatically by XML loader**; call manually in C when building programmatically |
| `ID_Object_Animate` | Per-frame animation tick |
| `ID_Object_Destroy` | Object being destroyed |
| `ID_Object_Timer` | Timer tick |
| `ID_Object_PropertyChanged` | A property value changed |

---

## Trigger → Action → SendMessageAction Flow

```
EventTrigger receives Trigger.Triggered
  → base Trigger sends Action.Dispatch { Sender = original sender }
  → each child action executes itself
```

`SendMessageAction` is the generic base for generated message actions:

```xml
<class name="SendMessageAction" parent="Action">
  <property name="Target" type="string"/>
  <property name="Mode" type="DispatchMode" default="Send"/>
  <handle message="Action.Dispatch"/>
</class>
```

Every `.cgen` `<message>` automatically gets a generated action class. The XML-facing class name is the message name:

```xml
<Screen.ShowModal Path="Example/Screens/GetStartedPopup"/>
<Popup.ClosePopup ReturnValue="1"/>
<Node.RightButtonUp Target="../Receiver"/>
```

Generated action handler finds the payload component, uses its ClassID as the message ID, and sends via `OBJ_SendMessageW` (Send mode) or `axPostMessageDataW` (Post mode).

C handlers continue to receive typed payload structs:

```c
HANDLER(Screen, Screen, ShowModal) {
    if (!pShowModal || !pShowModal->Path) return FALSE;
    // pShowModal->Path is the Path field from XML
}
```

---

## Property System

### Finding a property

| Function | Key type | Creates slot? | Returns | Public? |
|---|---|---|---|---|
| `OBJ_FindLongProperty(obj, id)` | `uint32_t` FullIdentifier | Yes | `Property*` | Yes |
| `OBJ_FindShortProperty(obj, name)` | `lpcString_t` bare name | Yes | `Property*` | Yes |
| `OBJ_ReadProperty(obj, id, buf)` | `uint32_t` FullIdentifier | No | `bool_t` | Yes |
| `OBJ_FindImplicitProperty(obj, name)` | bare name | No | `PropertyType const*` | Internal |
| `OBJ_FindExplicitProperty(obj, name)` | "Cat.Name" | No | `PropertyType const*` | Internal |
| `OBJ_FindPropertyByPath(obj, path)` | slash path | No | `Property*` | Internal |

**Rule:** use `OBJ_FindLongProperty` with an `ID_*` constant in component code — unambiguous, creates the slot.

### Reading

```c
struct Property *prop = OBJ_FindLongProperty(object, ID_Node2D_Width);
if (!prop || PROP_IsNull(prop)) return;
float width = *(float *)PROP_GetValue(prop);
```

For strings — `PROP_GetValue` returns pointer-to-pointer:

```c
char **slot = (char **)PROP_GetValue(prop);
lpcString_t s = slot ? *slot : NULL;
```

### Setting

```c
float value = 100.0f;
OBJ_SetPropertyValue(object, "Width", &value);   // by name
PROP_SetValue(prop, &value);                      // via pointer (fastest)
PROP_SetStringValue(prop, "hello");               // string convenience
```

### Iterating all properties (serialization / introspection)

```c
struct my_ctx { int count; };

static void count_cb(struct Object const *obj,
                     struct PropertyType const *pd,
                     struct ClassDesc const *cls,
                     void const *cdata,
                     void *param) {
    ((struct my_ctx *)param)->count++;
}

struct my_ctx ctx = {0};
OBJ_EnumClassProperties(object, count_cb, &ctx);
// Pass a plain C function pointer — never a block literal (^{})
```

Always skip read-only properties in serialization:

```c
if (pd->IsReadOnly) continue;
```

### Formatting a property value to string

```c
char buf[256];
if (OBJ_FormatPropertyValue(pd, value, buf, sizeof(buf)) && buf[0]) {
    // use buf
}
// Returns FALSE for kDataTypeObject, kDataTypeEvent, or NULL value
```

### PropertyType descriptor fields

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
    uint32_t     DataSize;       // sizeof the field
    bool_t       IsArray;        // true → {void *items; int count}
    bool_t       IsInherited;
    bool_t       IsReadOnly;     // skip in serialization
    bool_t       IsHidden;
    float        Step, UpperBound, LowerBound;
};
```

| Constant | C type |
|---|---|
| `kDataTypeBool` | `bool_t` (int) |
| `kDataTypeInt` | `int32_t` |
| `kDataTypeEnum` | `int32_t` (index into EnumValues[]) |
| `kDataTypeFloat` | `float` |
| `kDataTypeString` | `char *` (heap, never directly copy) |
| `kDataTypeColor` | `float[4]` RGBA 0.0–1.0 |
| `kDataTypeStruct` | varies — use `OBJ_FindStructDesc(pd->TypeString)` |
| `kDataTypeObject` | `struct Object *` reference |
| `kDataTypeEvent` | `event_t` (not serializable) |

### Struct descriptors

```c
struct StructDesc const *sd = OBJ_FindStructDesc("Thickness");
FOR_LOOP(i, sd->NumProperties) {
    struct PropertyType const *field = &sd->Properties[i];
    void const *fval = (char *)base + field->Offset;
}
// OBJ_FindStructDesc returns NULL for unknown names — always null-check
```

---

## Bindings

### Attaching a binding (XML deserializer pattern)

```c
OBJ_AttachPropertyProgram(object, "Node2D.Width", "{Screen.Width} * 0.5",
                           kBindingModeExpression, TRUE);
```

### Checking and reading a binding

```c
if (PROP_HasProgram(prop)) {
    lpcString_t expr = PROP_GetBindingExpression(prop);
    // expr is the original source string, may be NULL for old bindings
}
```

---

## Object Lifecycle

```c
// Create
struct Object *obj = OBJ_Create(ID_TextBlock);

// Build hierarchy
OBJ_AddChild(parent, child);

// Initialise (call after all children added and attributes set)
OBJ_SendMessageW(obj, ID_Object_Start, 0, NULL);

// Reference counting
OBJ_AddRef(obj);
OBJ_ReleaseRef(obj);   // frees when refcount hits zero

// Traversal
lpcString_t name       = OBJ_GetName(obj);
lpcString_t class_name = OBJ_GetClassName(obj);  // first (most-derived) component
struct Object *parent  = OBJ_GetParent(obj);
struct Object *child   = OBJ_GetFirstChild(obj);
struct Object *sibling = OBJ_GetNext(obj);
uint32_t flags         = OBJ_GetFlags(obj);

// Look up by name/path
struct Object *found = OBJ_FindByPath(root, "Body/NavBar");

// Type check
if (OBJ_CLASS(object, TextBlock)) { ... }

// Flags
if (OBJ_GetFlags(obj) & OF_TEMPLATE) { /* prefab instance */ }
if (OBJ_GetFlags(obj) & OF_DIRTY)    { /* needs layout update */ }
```

---

## Component Accessors

```c
// Get<ClassName>(object_ptr) → struct <ClassName>* or NULL
struct Node          *node   = GetNode(object);
struct Node2D        *n2d    = GetNode2D(object);
struct TextBlock     *tb     = GetTextBlock(object);
struct Button        *btn    = GetButton(object);
struct Screen        *scr    = GetScreen(object);
struct ItemsControl  *ic     = GetItemsControl(object);
struct Binding       *bnd    = GetBinding(object);
```

Always null-check before dereferencing. The underlying call is `OBJ_GetComponent(object, ID_ClassName)`.

---

## Iteration Macros

```c
// Indexed loop
FOR_LOOP(i, cls->NumProperties) { ... }

// Intrusive linked list (caches next before body — safe to free inside)
FOR_EACH_LIST(struct component, cmp, object->components) { ... }
FOR_EACH_LIST(struct Property, p, object->properties) { ... }

// Walk immediate children (caches next — safe to detach inside)
FOR_EACH_OBJECT(child, object) {
    OBJ_SendMessageW(child, ID_Node_UpdateGeometry, 0, NULL);
}

// Walk children that have a Node2D component
FOR_EACH_LAYOUTABLE(child, parent) {
    struct Node2D *n2d = GetNode2D(child);
}

// Apply a function to every child
FOR_EACH_CHILD(parent, OBJ_SetDirty);
FOR_EACH_CHILD(parent, _SendThemeChanged, .recursive = TRUE);

// Walk property list
FOR_EACH_PROPERTY(p, OBJ_GetProperties(object)) {
    PROP_Print(p, buf, sizeof(buf));
}

// XML children
xmlForEach(c, xmlNode) { /* element children only */ }
xmlFindAll(child, node, XMLSTR("property")) { ... }
```

---

## Memory and RAII

```c
struct Foo *p = ZeroAlloc(sizeof(struct Foo));  // malloc+memset zero
SafeFree(p);                                    // free + null
SafeDelete(token, Token_Release);               // custom destructor + null
SafeSet(str, strdup(new_str), free);            // free old then assign

// RAII scope — cleanup_func(var) at end of block
// Do NOT return inside a WITH block — use break instead
WITH(xmlChar, text, xmlNodeGetContent(node), xmlFree) {
    if (text) handle((lpcString_t)text);
}
```

---

## Linked List Helpers

```c
ADD_TO_LIST(new_prop, object->properties);       // prepend
ADD_TO_LIST_END(struct component, comp, object->components);  // append
REMOVE_FROM_LIST(struct component, cmp, object->components);
```

---

## Logging and Error Handling

```c
Con_Error("bad value %d in %s", value, name);    // stderr, never abort
Con_Warning("soft problem: %s", reason);
Con_Printf("debug: %g\n", f);

// HRESULT checks
if (FAILED(hr)) return E_FAIL;
if (SUCCEEDED(hr)) { ... }
```

---

## XML Helpers

```c
_xmlSetProp(node, "Name", value);
_xmlSetPropFormat(node, "Width", "%g", 42.0f);
_xmlNodeSetContent(node, text);
_xmlNodeSetContentFormat(node, "%d", count);
XMLSTR(s)   // cast const char* → xmlChar*
```

---

## StyleClass / CSS Classes

```c
char buf[1024];
lpcString_t classes = OBJ_GetRawStyleClasses(object, buf, sizeof(buf));
_SendMessage(object, StyleController, AddClasses, .ClassNames = "button primary");
_SendMessage(object, StyleController, AddClass,   .ClassName  = "hover");
```

---

## Generated File Locations

| File | Purpose |
|---|---|
| `include/orca.h` | Public API: OBJ_, PROP_, FS_, type macros |
| `include/orcadef.h` | Core macros: HANDLER, _SendMessage, FOR_*, WITH, ZeroAlloc |
| `include/orcaerror.h` | SUCCEEDED, FAILED, S_OK, E_FAIL |
| `generated/core/core.h` | struct Node, struct Binding, message typedefs |
| `generated/core/core_properties.h` | ID_* constants, GetNode/GetNode2D/… accessors |
| `generated/UIKit/UIKit.h` | struct TextBlock, struct Button, struct Screen |
| `generated/UIKit/UIKit_properties.h` | UIKit ID_* and Get*() accessors |
| `generated/geometry/geometry.h` | eDataType_t enum, struct PropertyType definition |
| `source/core/core_local.h` | Private core types — only for core/ |
| `source/filesystem/fs_xml.c` | XML deserializer |
| `source/filesystem/fs_xml_write.c` | XML serializer |

**Never hand-edit generated files** (`*_export.c`, `*.h` under `generated/`). Edit XML/templates and regenerate with `make modules`.

---

## Common Pitfalls

| Anti-pattern | Symptom | Fix |
|---|---|---|
| Writing C handlers without a `<class>` in XML | Fails to compile: `struct MyComponent` undefined | Write XML first, run codegen |
| `<class>` declared but no `<handle>` entries | Component attaches but does nothing silently | Add `<handle message="..."/>` for every handler |
| `#include <plugins/...>` inside `source/core/` | Circular build dependency | Remove the include; use a message or registered accessor |
| No test file | Silent regressions and empty-Proc bugs undetected | Always write a Lua test |
| Xcode project not updated | macOS/Xcode build fails | Add file to `project.pbxproj` in all four sections |
| `OBJ_EnumClassProperties` with a block literal | Compile error | Use plain C function pointer + context struct |
| `return` inside `WITH` block | Cleanup doesn't run | Use `break` instead |
| `FOR_LOOP` with signed comparisons | Silently incorrect results (uint32_t vs negative) | Use `int` locals |
| String via `PROP_GetValue` not double-dereferenced | Stale pointer or garbage | `char **slot = PROP_GetValue(prop); lpcString_t s = *slot;` |
| Serializing IsReadOnly properties | No-ops at best, parse errors at worst | Check `if (pd->IsReadOnly) continue;` |
| `OF_TEMPLATE` object serialized as inline tree | Loses prefab link | Emit `<LayerPrefabPlaceholder PlaceholderTemplate="..."/>` |

---

## Available Classes — Quick Index

Generated docs at `docs/generated/<module>/classes/ClassName.md` for full properties and messages.

### core (`source/core/core.cgen`)

| Class | Role |
|---|---|
| Node | Base class for all UI engine nodes |
| AnimationCurve | Single animated property curve with keyframes |
| AnimationClip | Reusable animation asset (standalone) |
| AnimationPlayer | Component managing animation playback |
| PropertyAnimation | One-shot property tween, self-destructs |
| StyleSheet | Container for StyleRule definitions |
| StyleRule | Maps a class name (+ pseudo-class) to property overrides |
| StyleController | Manages CSS-style classes and stylesheet rules |
| StateManager | Container for StateGroup definitions |
| StateGroup | Tracks one controller property → maps values to States |
| State | Property overrides when StateGroup controller matches Value |
| StateManagerController | Applies state-driven overrides from a StateManager |
| ResourceDictionary | Base class for alias and locale resource containers |
| Locale | Localization resource (text and asset strings) |
| DataObject | Base class for data-holding objects |
| Binding | Declarative property binding (attaches program to parent) |
| BindingExpression | Legacy explicit-expression binding alias |
| Action | Base class for executable trigger actions |
| SendMessageAction | Base for generated message actions (Target, Mode) |
| Trigger | Base class for event-driven actions |
| OnPropertyChangedTrigger | Responds to property value changes on specified nodes |
| OnAttachedTrigger | Triggers when element is attached to the visual tree |
| EventTrigger | Responds to routed UI events (mouse, keyboard, etc.) |
| Setter | Applies property values when triggered |
| HideAction | Hides the object at a given path when triggered |

### UIKit (`plugins/UIKit/UIKit.cgen`)

| Class | Role |
|---|---|
| Node2D | 2D layout node (position, size, margins, transforms) |
| TextBlock | Renders text with style, wrapping, alignment |
| Input | Text input field |
| Button | Clickable button |
| Label | Non-interactive text label |
| StackView | Linear stack layout (horizontal or vertical) |
| Form | Form container |
| Screen | Full-screen modal or page root |
| Grid | CSS-grid-style layout container |
| ImageView | Image display |
| NinePatchImage | Scalable nine-patch image |
| ConsoleView | Embedded terminal output view |
| Page | Navigation page |
| PageHost | Hosts a navigation stack |
| PageViewport | Viewport for a page within a PageHost |
| Popup | Floating overlay |
| ListBox | Scrollable item list (built on ItemsControl + StackView) |
| ItemsControl | Data-driven item container base |
| PrefabView2D | Inline prefab instance |
| Control | Base for interactive UI elements |
| Brush | Base class for fill brushes |
| ColorBrush | Solid color fill |
| Cinematic | Cinematic/animation playback control |

### filesystem (`source/filesystem/filesystem.cgen`)

| Class | Role |
|---|---|
| Workspace | Root project workspace |
| Library | Library asset container |
| Project | Loadable project |
| Directory | Directory node in the asset tree |
| Package | Asset package |
| ThemeGroup | Group of Theme objects |
| Theme | Visual theme resource |

### SceneKit (`plugins/SceneKit/SceneKit.cgen`)

| Class | Role |
|---|---|
| Node3D | 3D scene node |
| Scene | 3D scene root |
| Model3D | 3D mesh model |
| Camera | 3D camera |
| Viewport3D | 3D viewport rendering |
| RenderPass | Render pass descriptor |
| Light3D | 3D light source |
| SpriteView | 2D sprite within a 3D scene |

---

## Object Struct Refactoring Direction

Legacy fields in `struct Object` being migrated to proper components:

| Legacy field | Target component | Status |
|---|---|---|
| `animation` / `animlib` | AnimationPlayer / AnimationClip | Done — fields removed |
| `animations` (property tweens) | PropertyAnimation | Done — field removed |
| `timers` | Timer | Planned |
| `stateManager` | StateManager | Planned |
| `stylesheet` / `classes` | StyleSheet / StyleClass | Planned |
| `aliases` | Alias | Planned |

Migration rule: implement component → register → remove struct field → remove `_GetXxx(obj)` macro.

---

## Working Rules

- Edit `.cgen` first, then run `make modules`, then write handlers.
- Never hand-edit generated files under `generated/` or `include/orca.h`.
- Do not `#include` plugin headers from `source/core/`.
- Add or update Lua tests when public behavior changes.
- Add new `.c` files to `orca.xcodeproj/project.pbxproj` in all four sections.
- Consult `docs/generated/<module>/classes/ClassName.md` for authoritative property and message lists before writing new code against a class.
