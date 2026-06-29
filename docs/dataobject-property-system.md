# DataObject & Property System: Kanzi Reference and ORCA Plan

This document combines a reference on Kanzi's dual property/data system, a gap analysis against ORCA, and the concrete implementation plan for making `Name` a real property on `DataObject`.

---

## 1. Kanzi Property System & DataObject Reference

### Overview

Kanzi uses two complementary systems for storing and accessing data:

1. **PropertyType System** — Metadata definitions for configurable values (editor-facing)
2. **DataObject System** — Runtime data tree with built-in value properties (data-binding-facing)

---

### PropertyType System

#### Purpose

Define named, typed properties that appear in Kanzi Studio's property editors. Used for **configuration** — settings that designers can modify.

#### Definition

```cpp
#include <kanzi/kanzi.hpp>
using namespace kanzi;

// Static member declaration in header
static PropertyType<string> MyCustomProperty;

// Static member definition in cpp
PropertyType<string> XMLDataSource::MyCustomProperty(
    kzMakeFixedString("MyNamespace.MyCustomProperty"),  // Unique string ID
    "",                                                  // Default value
    0,                                                   // Flags
    false,                                               // Is default
    KZ_DECLARE_EDITOR_METADATA
    (
        metadata.displayName = "My Custom Property";
        metadata.tooltip = "Description for Kanzi Studio";
        metadata.editor = "BrowseFileTextEditor";        // Editor widget type
    )
);
```

#### Registration to Class

```cpp
class MyClass : public Node
{
    KZ_METACLASS_BEGIN(MyClass, Node, "MyClass")
    KZ_METACLASS_PROPERTY_TYPE(MyCustomProperty)
    KZ_METACLASS_END()
};
```

#### Usage

```cpp
// Read
string value = getProperty(MyCustomProperty);

// Write
setProperty(MyCustomProperty, "newValue");
```

#### Key Characteristics

- Requires compile-time definition via `PropertyType<T>` template
- Needs `KZ_METACLASS_PROPERTY_TYPE` macro to register with class
- Visible in Kanzi Studio property panel
- Supports custom editor widgets
- Typed: `PropertyType<int>`, `PropertyType<string>`, `PropertyType<float>`, etc.

---

### DataObject System

#### Purpose

Runtime data trees for **dynamic content** — values that change at runtime or come from external sources (files, network, etc.).

#### Built-in Property Types on DataObject

Every DataObject instance automatically has these properties **without any custom PropertyType definitions**:

| Property Type | Description | Example Value |
|---------------|-------------|---------------|
| `Name` | Object identifier | `"speed"`, `"unit"` |
| `DataObjectDataType` | Value type enum | `INTEGER`, `STRING`, `FLOAT`, `BOOL`, `OBJECT`, `LIST` |
| `DataObjectValue` | The actual value | `"90"`, `"km/h"`, `"true"` |

#### DataObject Class Hierarchy

```
DataObject              — Base node (no value, just hierarchy)
├── DataObjectInt       — Integer value
├── DataObjectReal      — Float/double value
├── DataObjectString    — String value
├── DataObjectBool      — Boolean value
└── DataObjectList      — List container
    └── DataObjectStaticList — Static list implementation
```

#### Construction

```cpp
Domain* domain = getDomain();

// Base object (hierarchy node, no value)
auto root = make_shared<DataObject>(domain, "Root");

// Typed objects
auto intObj = make_shared<DataObjectInt>(domain, "speed", 90);
auto realObj = make_shared<DataObjectReal>(domain, "temperature", 23.5);
auto strObj = make_shared<DataObjectString>(domain, "unit", "km/h");
auto boolObj = make_shared<DataObjectBool>(domain, "isActive", true);
auto listObj = make_shared<DataObjectStaticList>(domain, "items");
```

#### Building a Tree

```cpp
auto root = make_shared<DataObject>(domain, "Root");

auto gauges = make_shared<DataObject>(domain, "gauges");
root->addChild(gauges);

auto speed = make_shared<DataObjectInt>(domain, "speed", 90);
gauges->addChild(speed);

auto unit = make_shared<DataObjectString>(domain, "unit", "km/h");
speed->addChild(unit);
```

#### Accessing Values

```cpp
// The value is stored in the DataObjectValue property
// Access via the property system:
string val = speed->getProperty(DataObjectValue);  // Returns "90"

// Or use typed accessors (if available on subclass):
int intVal = speed->getValue();  // Returns 90
```

#### Notification

```cpp
// After modifying data, notify listeners
speed->notifyModified();
```

---

### XML to DataObject Mapping

#### Source XML

```xml
<gauges>
    <speed type="int">90
        <unit type="string">km/h</unit>
    </speed>
    <fuel type="int">75</fuel>
    <turnindicator>
        <left type="bool">true</left>
    </turnindicator>
</gauges>
```

#### Resulting DataObject Tree

```
Root (DataObject)
└── gauges (DataObject)
    ├── speed (DataObjectInt)        DataObjectValue="90", DataObjectDataType=INTEGER
    │   └── unit (DataObjectString)  DataObjectValue="km/h", DataObjectDataType=STRING
    ├── fuel (DataObjectInt)         DataObjectValue="75", DataObjectDataType=INTEGER
    └── turnindicator (DataObject)
        └── left (DataObjectBool)    DataObjectValue="true", DataObjectDataType=BOOL
```

#### Type Attribute Mapping

| XML `type` attribute | DataObject subclass | DataObjectDataType |
|----------------------|---------------------|-------------------|
| `"int"` | `DataObjectInt` | `INTEGER` |
| `"real"`, `"float"` | `DataObjectReal` | `FLOAT` |
| `"bool"`, `"boolean"` | `DataObjectBool` | `BOOL` |
| `"string"` | `DataObjectString` | `STRING` |
| `"list"` | `DataObjectStaticList` | `LIST` |
| *(none)* | `DataObject` | `OBJECT` |

---

### Data Binding in Kanzi Studio

#### Path Syntax

UI elements bind to data objects using dot-separated paths:

```
DataSourceName.gauges.speed.DataObjectValue
```

#### Binding Components

| Segment | Description |
|---------|-------------|
| `DataSourceName` | Name of the DataSource in Kanzi Studio |
| `gauges` | Child DataObject name |
| `speed` | Child DataObject name |
| `DataObjectValue` | Built-in property holding the value |

#### Example Binding Configuration (from .kzproj)

```xml
<Binding>
    <targetPropertyAttribute>WHOLE_PROPERTY</targetPropertyAttribute>
    <targetPropertyTypeReference>DataObjectValue</targetPropertyTypeReference>
    <sourcePath>Cluster.gauges.speed.DataObjectValue</sourcePath>
</Binding>
```

---

### DataSource Plugin Integration

#### Plugin Class Structure

```cpp
class MyDataSource : public DataSource
{
public:
    // Custom property for configuration (visible in Kanzi Studio)
    static PropertyType<string> ConfigFileProperty;

    KZ_METACLASS_BEGIN(MyDataSource, DataSource, "My_data_source")
    KZ_METACLASS_PROPERTY_TYPE(ConfigFileProperty)
    KZ_METACLASS_END()

    // Return root of data object tree
    DataObjectSharedPtr getData() KZ_OVERRIDE
    {
        return m_root;
    }

private:
    DataObjectSharedPtr m_root;
};
```

#### Loading Flow

```
Application Start
    ↓
loadFromKZB()  ← Load from .kzb binary
    ↓
Read ConfigFileProperty  ← Get external file path
    ↓
Parse external source (XML, JSON, etc.)
    ↓
Build DataObject tree
    ↓
m_root = tree root
    ↓
UI bindings read DataObjectValue properties
```

---

### Key Differences: PropertyType vs DataObjectValue

| Aspect | PropertyType | DataObjectValue |
|--------|--------------|-----------------|
| **Definition** | Compile-time `PropertyType<T>` | Built-in, no definition needed |
| **Registration** | `KZ_METACLASS_PROPERTY_TYPE` | Automatic |
| **Visibility** | Kanzi Studio property panel | Data binding paths only |
| **Use case** | Plugin configuration | Runtime data |
| **Typing** | Strong (`PropertyType<int>`) | String-based, converted via `DataObjectDataType` |
| **Modification** | Designer can edit in Studio | Programmatic only |

---

### Summary

- **PropertyType** = Define once in C++, register to class, use for settings
- **DataObject** = Build tree at runtime, values stored in `DataObjectValue`, no PropertyType needed
- **DataObjectValue** = Built-in property on every DataObject, holds the actual value as string
- **DataObjectDataType** = Built-in property indicating how to interpret `DataObjectValue`
- Data binding paths end with `.DataObjectValue` to read the value

---

## 2. ORCA vs Kanzi: Property System Comparison

### Overview

This section compares ORCA's property system with Kanzi's dual-system approach (PropertyType + DataObject), identifying gaps and opportunities for improvement.

---

### Kanzi's Two-System Architecture

Kanzi separates concerns into two complementary systems:

#### 1. PropertyType System (Configuration)
- Compile-time `PropertyType<T>` templates for each property
- Registered to classes via `KZ_METACLASS_PROPERTY_TYPE` macro
- Visible in Kanzi Studio property editors
- Strongly typed, supports custom editor widgets
- Used for designer-facing settings

#### 2. DataObject System (Runtime Data)
- Tree of typed nodes (DataObjectInt, DataObjectReal, DataObjectString, DataObjectBool, DataObjectList)
- Every DataObject has **built-in properties** without custom definitions:
  - `Name` — object identifier
  - `DataObjectDataType` — value type enum (INTEGER, STRING, FLOAT, BOOL, OBJECT, LIST)
  - `DataObjectValue` — the actual value (stored as string)
- Data binding paths end with `.DataObjectValue` to read the value
- `notifyModified()` triggers binding updates

#### Key Insight
DataObject nodes are themselves objects in the property system — each field is a Node. This avoids defining PropertyType for every possible data field. The type information lives in `DataObjectDataType`, not in a compile-time template.

---

### ORCA's Current Architecture

#### Property System
- Properties defined in `.cgen` via `<property name="..." type="...">` on classes
- Codegen produces `PropertyType` structs with FNV-1a32 identifiers
- Each `Object` holds a linked list of `Property` instances
- `Property` struct: `{ next, binding, object, pdesc, value, flags, changeCallback, updateFrame }`
- Supports: bool, int, float, string, enum, color, struct, object, event types

#### Binding System
- `Binding` class with `Expression`, `Mode`, `Enabled` properties
- Expressions compiled to VM token trees (`property_program.c`)
- Path syntax: `{DataContext/PropertyName}`, `{#Alias/Property}`, `{.Property}`
- Binding modes: OneWay, TwoWay, OneWayToSource, Expression

#### DataContext
- `Node.DataContext` property typed as `DataObject*`
- Binding VM resolves `DataContext/Path` by walking up the hierarchy to find a DataContext, then navigating the path
- `_FindDataContextObject()` searches up from node to screen

#### DataObject Class
- Minimal: only has `Name` property
- No `DataObjectValue` or `DataObjectDataType` built-in properties
- No typed subclasses (DataObjectInt, DataObjectString, etc.)
- No `notifyModified()` equivalent for data binding updates

---

### Gap Analysis

| Feature | Kanzi | ORCA | Gap |
|---------|-------|------|-----|
| **DataObject as data tree** | Full tree with typed value nodes | Minimal `Name`-only class | ORCA DataObject cannot hold arbitrary runtime values |
| **Built-in value property** | `DataObjectValue` on every node | None | Each field requires a PropertyType definition |
| **Built-in type property** | `DataObjectDataType` enum | None | No runtime type discrimination |
| **Typed subclasses** | DataObjectInt, DataObjectString, etc. | None | No way to create typed data nodes without PropertyType |
| **Data binding to runtime data** | `DataSource.gauges.speed.DataObjectValue` | `DataContext/PropertyName` (requires PropertyType) | Cannot bind to arbitrary data tree fields |
| **Data modification notification** | `notifyModified()` | `PROP_NotifyChanged()` (requires Property) | DataObject tree changes don't auto-notify bindings |
| **DataSource plugin** | DataSource class with `getData()` returning DataObject root | No equivalent | No plugin pattern for external data sources |
| **XML-to-DataObject mapping** | Automatic from XML type attributes | N/A (data loads as Properties) | No runtime data tree construction from XML |

---

### The Core Problem: DataObject Is Not a Data Container

In Kanzi, a DataObject tree like this is natural:

```
Root (DataObject)
└── gauges (DataObject)
    ├── speed (DataObjectInt)    DataObjectValue=90
    │   └── unit (DataObjectString)  DataObjectValue="km/h"
    └── fuel (DataObjectInt)     DataObjectValue=75
```

Each field is an object node. Binding to `DataSource.gauges.speed.DataObjectValue` works because `DataObjectValue` is a built-in property on every DataObject.

In ORCA, `DataObject` only has `Name`. To achieve the same, you'd need:

```xml
<DataObject Name="gauges">
  <DataObject Name="speed">
    <DataObject Name="value" type="int">90</DataObject>  <!-- requires PropertyType -->
    <DataObject Name="unit" type="string">km/h</DataObject>  <!-- requires PropertyType -->
  </DataObject>
</DataObject>
```

Or create custom classes for each data type, which defeats the purpose of a generic data system.

---

### Recommended Improvements

#### 1. Add Built-in Properties to DataObject

Add `DataObjectValue` and `DataObjectDataType` as built-in properties:

```xml
<class name="DataObject">
  <properties>
    <property name="Name" type="string">Object identifier</property>
    <property name="Value" type="string">The actual value (stored as string)</property>
    <property name="DataType" type="DataObjectDataType">Value type discriminator</property>
  </properties>
</class>
```

This allows binding to `DataContext/Name`, `DataContext/Value` without defining PropertyType for each field.

#### 2. Add Typed DataObject Subclasses

```xml
<class name="DataObjectInt" parent="DataObject">
  <properties>
    <property name="IntValue" type="int">Integer value</property>
  </properties>
</class>

<class name="DataObjectString" parent="DataObject">
  <properties>
    <property name="StringValue" type="string">String value</property>
  </properties>
</class>
```

Or alternatively, keep a single DataObject with `Value` as string and `DataType` as enum, converting at access time (Kanzi's approach).

#### 3. Add DataObject.Change Notification

```c
// When DataObject.Value is set, notify bindings
void DataObject_NotifyModified(struct Object *dataObj) {
    PROP_NotifyChanged(OBJ_FindProperty(dataObj, "Value"));
    // Bubble up to notify parent bindings
    _SendMessage(dataObj, Object, PropertyChanged, ...);
}
```

#### 4. Add DataSource Plugin Pattern

```xml
<interface name="DataSource" prefix="DS_" export="DataSource">
  <methods>
    <method name="GetData">
      <summary>Returns the root DataObject tree</summary>
      <returns type="DataObject" pointer="true"/>
    </method>
  </methods>
</interface>
```

This allows external data sources (JSON, XML, network) to provide data trees that participate in binding.

#### 5. Extend Binding Path Resolution

Currently binding paths resolve against the object hierarchy. For DataObject trees, paths should also work against DataObject children by name:

```
DataContext/gauges.speed.Value
```

The binding VM's `argument` handler already walks `DataContext` paths — extend it to navigate DataObject children by name.

---

### Implementation Priority

1. **DataObject.Value + DataType properties** (high) — Enables data binding to runtime data
2. **DataObject.NotifyModified** (high) — Enables reactive data updates
3. **Typed subclasses or string-coercion approach** (medium) — Choose one strategy
4. **DataSource interface** (medium) — Plugin pattern for external data
5. **Binding path navigation for DataObject trees** (low) — May already work with DataContext

---

### Open Questions

1. **String-coercion vs typed subclasses**: Kanzi stores all values as strings in `DataObjectValue` and uses `DataObjectDataType` for interpretation. ORCA could follow this simpler approach, or provide typed subclasses for performance. Which is preferred?

2. **DataObject tree construction**: Should DataObject trees be constructable from XML at load time (like Kanzi), or only programmatically at runtime?

3. **DataSource lifetime**: Should DataSource objects be managed by the engine or by application code?

4. **Backward compatibility**: Adding Value/DataType to DataObject is additive. Existing code using DataObject only for Name should continue working.

---

## 3. Plan: Make `Name` a Property on DataObject

### Problem

`{DataContext/Name}` binding expression fails because `Name` is a virtual property on `struct Object` (handled by hardcoded switch cases in `OBJ_GetProperty`/`OBJ_SetProperty`), not a registered C property. The binding evaluator calls `OBJ_FindPropertyByPath` which only finds registered properties.

---

### Steps

#### Step 1: Add `Name` property to DataObject in `.cgen`

**File:** `source/core/core.cgen`

Change DataObject from:
```xml
<class name="DataObject">
  <summary>Base class for data-holding objects within the framework</summary>
  <details>Provides fundamental data management capabilities and serves as the base for objects that participate in data binding operations.</details>
</class>
```

To:
```xml
<class name="DataObject">
  <summary>Base class for data-holding objects within the framework</summary>
  <details>Provides fundamental data management capabilities and serves as the base for objects that participate in data binding operations.</details>
  <properties>
    <property name="Name" type="string">The name of this data object, used for identification in data binding.</property>
  </properties>
</class>
```

After `make modules`, this generates:
- `struct DataObject { const char* Name; }` (was empty)
- `ID_DataObject_Name` property ID constant in `core_properties.h`
- `DECL(...)` entry in `DataObjectProperties[]` in `core_export.c`
- Accessor cases in `DataObject_GetProperty` / `DataObject_ReadPropertyAtOffset`

#### Step 2: Sync `DataObject.Name` with `object->Name`

**File:** `source/core/object/object_lua_props.c`

The conflict: `OBJ_SetProperty` has a hardcoded `case p_Name` that calls `OBJ_SetName` and returns early — it never reaches the normal property setter. So `DataObject.Name` would never get set through Lua (`obj.Name = "foo"`).

Fix: In the `p_Name` case of `OBJ_SetProperty` (and `p_id` which shares the case), after calling `OBJ_SetName`, also update the DataObject `Name` property if present:

```c
case p_id:
case p_Name: {
    OBJ_SetName(self, luaL_checkstring(L, 3));
    struct Property *nameProp = OBJ_FindLongProperty(self, ID_DataObject_Name);
    if (nameProp) {
        PROP_SetValue(nameProp, OBJ_GetName(self));
    }
    return TRUE;
}
```

Note: `ID_DataObject_Name` is defined in the generated `core_properties.h`, which is already included transitively through `object_internal.h` -> `core_local.h` -> `orca.h` -> `core/core.h` -> `core/core_properties.h`.

For `OBJ_GetProperty`, the existing `p_Name` case reads from `OBJ_GetName(self)` (which reads `object->Name`). This stays as-is — `object->Name` is the source of truth for the object hierarchy, and it stays in sync because `OBJ_SetName` is always called.

#### Step 3: Rebuild

```bash
make modules  # regenerate from .cgen
make          # rebuild
```

#### Step 4: Upgrade test to verify binding resolution

**File:** `tests/test_listbox.lua`

Update `test_listbox_binding_expression` to verify that `{DataContext/Name}` now resolves to the actual name value (e.g. "Alice", "Bob") instead of nil/0.

---

### Tradeoffs

| Aspect | Detail |
|--------|--------|
| Scope | Only affects DataObject — no changes to Node, Screen, or other classes |
| Backward compat | `obj.Name` in Lua still works via `OBJ_GetProperty` switch case; DataObject instances additionally have a real property |
| Source of truth | `object->Name` remains source of truth; DataObject.Name is synced from it |
| sizeof change | `struct DataObject` gains a `const char*` field (was empty); all existing allocations via the registration system pick this up automatically |

---

### Verification

- `test_listbox.lua` — all tests pass, binding resolves to actual name
- `test_styles_lua.lua` — all 52 CSS tests pass
- No regressions in other test suites
