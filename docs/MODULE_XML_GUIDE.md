# Module XML File Guide

## Overview

This guide explains how to create and maintain module XML files in the ORCA framework. Module XML files are used to define the API interface between C code and Lua, automatically generating C headers, bindings, and HTML documentation.

## What Are Module XML Files?

Module XML files describe the public API of C modules in a declarative format. The toolchain in `tools/` processes these XML files to generate:

1. **C Header Files** (`.h`) - Type definitions and function declarations
2. **C Export Files** (`_export.c`) - Lua bindings and registration code
3. **Properties Headers** (`_properties.h`) - Property hash constants
4. **API Documentation** - Markdown docs at `docs/api/`
5. **DTD Schema** - UI component XML schema (generated from module files)

## Location and Usage

Module XML files are located alongside their corresponding C source code:

- `source/geometry/geometry.xml` - Geometry types (vectors, matrices, etc.)
- `source/core/core.xml` - Core engine objects and components
- `source/renderer/api/renderer.xml` - Rendering interfaces
- `plugins/UIKit/UIKit.xml` - UI components
- `plugins/SceneKit/SceneKit.xml` - 3D scene graph components
- `plugins/SpriteKit/SpriteKit.xml` - 2D sprite components
- `source/filesystem/filesystem.xml` - File system access

To generate code from these XML files, run:

```bash
cd tools && make
```

## XML File Structure

Every module XML file must have this basic structure:

```xml
<?xml version="1.0"?>
<!DOCTYPE module SYSTEM "https://corepunch.github.io/orca/schemas/module.dtd">
<module name="modulename" namespace="orca">
  <includes>
    <include file="path/to/header.h"/>
  </includes>
  <externals>
    <external struct="ExternalType"/>
  </externals>
  <types>
    <enums name="MyEnum">...</enums>
  </types>
  <structs>
    <struct name="MyStruct">...</struct>
  </structs>
  <interfaces>
    <interface name="MyInterface">...</interface>
  </interfaces>
  <classes>
    <class name="MyClass">...</class>
  </classes>
  <functions>
    <function name="MyFunction">...</function>
  </functions>
</module>
```

Type definitions are grouped into typed container elements at the module root. This keeps all enumerations in `<types>`, all structs in `<structs>`, all interfaces in `<interfaces>`, all components in `<classes>`, and all global functions in `<functions>`.

### Root Element: `<module>`

The root element defines the module's identity and namespace.

**Attributes:**
- `name` (required) - The module name (e.g., "geometry", "core")
- `namespace` (required) - Lua namespace (typically "orca")
- `on-luaopen` (optional) - Callback function name to execute when module loads
- `prefix` (optional) - Prefix for generated C identifiers

## Top-Level Container Elements

Type definitions at the module level are always wrapped in a typed container:

| Container | Contents | Element type |
|-----------|----------|--------------|
| `<types>` | Enumeration definitions | `<enums>` |
| `<structs>` | C struct definitions | `<struct>` |
| `<interfaces>` | Abstract interface definitions | `<interface>` |
| `<classes>` | Component (class) definitions | `<class>` |
| `<functions>` | Global function definitions | `<function>` |

## Type Definition Elements

### 1. `<enums>` — Enumeration Definitions

Defines an enumeration type with named constants.

**Attributes:**
- `name` (required) - Name of the enumeration type

**Child Elements:**
- `<summary>` - Brief description
- `<details>` - Detailed explanation
- `<enum>` - Individual enumeration value

**Example:**
```xml
<types>
  <enums name="RotationOrder">
    <summary>Euler angle rotation order enumeration</summary>
    <enum name="XYZ">Rotate around X axis first, then Y, then Z</enum>
    <enum name="XZY">Rotate around X axis first, then Z, then Y</enum>
  </enums>
</types>
```

### 2. `<struct>` — Structure Definitions

Defines a C struct with fields and methods. Child elements are grouped into container elements.

**Attributes:**
- `name` (required) - Name of the struct
- `prefix` (optional) - Prefix for C constants (e.g., "VEC2_")
- `export` (optional) - Name exported to Lua (defaults to struct name)
- `sealed` (optional) - If "true", prevents recursive field access from Lua

**Child Elements (in order):**
- `<summary>` - Brief description
- `<details>` - Detailed explanation
- `<fields>` - Container for `<field>` elements
- `<methods>` - Container for `<method>` elements

**Example:**
```xml
<structs>
  <struct name="vec2" prefix="VEC2_" export="Vector2D" sealed="true">
    <summary>2D vector structure</summary>
    <details>Represents a point or direction in 2D space.</details>
    <fields>
      <field type="float" name="x">X coordinate component</field>
      <field type="float" name="y">Y coordinate component</field>
    </fields>
    <methods>
      <method name="Add" export="__add" const="true">
        <summary>Adds two vectors component-wise</summary>
        <arg type="vec2" name="other" pointer="true" const="true">Vector to add</arg>
        <returns type="vec2">Sum of the two vectors</returns>
      </method>
    </methods>
  </struct>
</structs>
```

### 3. `<interface>` — Interface Definitions

Similar to `<struct>`, but defines an abstract interface. Methods and topics are grouped in `<methods>`, messages in `<messages>`.

**Attributes:** Same as `<struct>`, plus:
- `no-check` (optional) - If "true", skips type checking in generated code
- `parent` (optional) - Parent interface name

**Child Elements (in order):**
- `<summary>` - Brief description
- `<details>` - Detailed explanation
- `<init>` - Initialization marker
- `<methods>` - Container for `<method>` and `<topic>` elements
- `<messages>` - Container for `<message>` elements

**Example (simple):**
```xml
<interfaces>
  <interface name="Object" prefix="OBJ_" export="Object" no-check="true">
    <summary>Core engine host object.</summary>
    <methods>
      <method name="Clear" export="clear" lua="true">
        <summary>Clear all children of the object.</summary>
      </method>
    </methods>
  </interface>
</interfaces>
```

**Example (with topic separators):**
```xml
<interfaces>
  <interface name="Object" prefix="OBJ_" export="Object" no-check="true">
    <summary>Core engine host object.</summary>
    <methods>
      <topic title="Lifecycle">Manages object creation, initialization, and destruction.</topic>
      <method name="Awake" lua="true">
        <summary>Initializes the object when loaded.</summary>
      </method>
      <method name="Clear" export="clear" lua="true">
        <summary>Clear all children of the object.</summary>
      </method>
      <topic title="Hierarchy">Navigates and manipulates the parent-child relationship tree.</topic>
      <method name="AddChild">
        <summary>Add a child object.</summary>
        <arg name="child" type="Object" pointer="true">The object to add as a child</arg>
      </method>
    </methods>
    <messages>
      <message name="Create" routing="Direct"/>
      <message name="Destroy" routing="Direct"/>
    </messages>
  </interface>
</interfaces>
```

#### `<topic>` — Inline Section Separator

A `<topic>` is a separator placed **inside** the `<methods>` container between method elements. It marks the start of a new named section in the generated documentation.

**Attributes:**
- `title` (required) - The section heading used in generated documentation

**Rules:**
- A `<topic>` may only appear directly inside a `<methods>` element within an `<interface>`.
- Methods following a `<topic>` belong to that section until the next `<topic>` or the end of `<methods>`.
- `<topic>` is invisible to code generation; only `docs.php` renders topics as section headings.

**When to use topics:**
- Use topics when an interface has more than ~10 methods with clear functional groups.
- See `source/core/core.xml` for a real-world example with 10 topic separators.

### 4. `<class>` — Component Definitions

Defines a component that can be attached to objects. All child element types are grouped into containers.

**Attributes:**
- `name` (required) - Component name
- `parent` (optional) - Parent component to inherit from
- `concept` (optional) - Interface this component implements
- `children` (optional) - Type of child nodes this component accepts
- `prefix` (optional) - Prefix for generated C identifiers

**Child Elements (in order):**
- `<summary>` - Brief description
- `<details>` - Detailed explanation
- `<xmlns>` - XML namespace declaration for this component
- `<handles>` - Container for `<handle>` elements (messages handled by this class)
- `<properties>` - Container for `<property>` elements
- `<fields>` - Container for `<field>` elements (internal/private data)
- `<methods>` - Container for `<method>` elements
- `<messages>` - Container for `<message>` elements

**Example:**
```xml
<classes>
  <class name="SKNode" parent="Node" children="SKNode">
    <summary>Base 2D sprite node with transform and matrix</summary>
    <details>Provides fundamental 2D transformation properties.</details>
    <handles>
      <handle message="Node.UpdateMatrix"/>
    </handles>
    <properties>
      <property name="Position" type="vec2">Position of SKNode in space</property>
      <property name="Size" type="vec2">Size of SKNode</property>
    </properties>
    <fields>
      <field name="Matrix" type="mat4">Final combined transformation matrix</field>
      <field name="_opacity" type="float"/>
    </fields>
    <messages>
      <message name="Render">
        <fields>
          <field name="ViewDef" type="ViewDef" pointer="true">The view definition for rendering</field>
        </fields>
      </message>
    </messages>
  </class>
</classes>
```

### 5. `<function>` — Global Function Definitions

Defines a global function in the module namespace.

**Attributes:**
- `name` (required) - Function name

**Child Elements:**
- `<summary>`, `<details>`, `<arg>`, `<returns>`

**Example:**
```xml
<functions>
  <function name="GetFocus">
    <summary>Retrieves currently active object.</summary>
    <returns type="Object" pointer="true">Pointer to the focused object</returns>
  </function>
</functions>
```

### 6. `<message>` — Message Definitions

Declares a message that can be handled by components. Fields inside messages are grouped in a `<fields>` container.

**Attributes:**
- `name` (required) - Message name
- `type` (optional) - Message parameter type (external struct)
- `same-as` (optional) - Inherit fields from another message
- `routing` (optional) - Routing strategy: `Bubbling`, `TunnelingBubbling`, `Tunneling`, or `Direct`

**Child Elements:**
- `<summary>`, `<details>`
- `<fields>` - Container for `<field>` elements

**Example:**
```xml
<message name="PropertyChanged" routing="Direct">
  <fields>
    <field name="Property" type="Property" pointer="true">The property that changed</field>
  </fields>
</message>
```

### 7. `<includes>` and `<externals>` — Include and External Directives

C header includes and external type references are grouped into container elements at the module level.

```xml
<module name="UIKit" namespace="orca">
  <includes>
    <include file="include/renderer.h"/>
    <include file="source/core/core.h"/>
  </includes>
  <externals>
    <external struct="Object"/>
    <external struct="ViewDef"/>
  </externals>
  <!-- ... -->
</module>
```

### 8. `<resource>` — Resource Type Definitions

Declares a resource type that can be loaded by the engine.

**Attributes:**
- `type` (required) - Resource type name
- `no-lua` (optional) - If "true", excludes from Lua bindings

**Example:**
```xml
<resource type="localization"/>
```

## Child Element Details

### `<field>` — Struct/Class Fields

Defines a data member. Fields are always inside a `<fields>` container.

**Attributes:**
- `name` (required) - Field name
- `type` (required) - Data type
- `fixed-array` (optional) - Array size (if a fixed-size array)
- `pointer` (optional) - If "true", field is a pointer
- `const` (optional) - If "true", field is const

**Example:**
```xml
<fields>
  <field type="float" name="x">X coordinate component</field>
  <field name="children" type="Object" array="true" pointer="true">Child objects</field>
</fields>
```

### `<method>` — Methods

Defines a method or member function. Methods are always inside a `<methods>` container.

**Attributes:**
- `name` (required) - Method name
- `export` (optional) - Name exported to Lua (e.g., "__add" for operator overloading)
- `const` (optional) - If "true", method does not modify the object
- `lua` (optional) - If "true", method is Lua-accessible
- `static` (optional) - If "true", method is static (no `this` parameter)
- `private` (optional) - If "true", method is excluded from public bindings

**Child Elements:** `<summary>`, `<details>`, `<arg>`, `<returns>`

**Example:**
```xml
<methods>
  <method name="Add" export="__add" const="true">
    <summary>Adds two vectors component-wise</summary>
    <arg type="vec2" name="other" pointer="true" const="true">Vector to add</arg>
    <returns type="vec2">Sum of the two vectors</returns>
  </method>
</methods>
```

### `<property>` — Component Properties

Defines a property on a component. Properties are always inside a `<properties>` container.

**Attributes:**
- `name` (required) - Property name
- `type` (required) - Property data type
- `readonly` (optional) - If "true", property is read-only
- `pointer` (optional) - If "true", property holds a pointer
- `default` (optional) - Default value for the property

**Example:**
```xml
<properties>
  <property name="Opacity" type="float" default="1">Opacity of the node.</property>
  <property name="Image" type="Texture" pointer="true">Texture to display.</property>
</properties>
```

### `<handles>` and `<handle>` — Message Handlers

Declares which messages a component handles. The `<handles>` container holds one or more `<handle>` children.

**`<handle>` Attributes:**
- `message` (required) - Name of message to handle (format: `ClassName.MessageName`)

**Example:**
```xml
<handles>
  <handle message="Node.UpdateMatrix"/>
  <handle message="Object.Create"/>
</handles>
```

### `<arg>` — Function/Method Arguments

**Attributes:** `name` (required), `type` (required), `pointer` (optional), `const` (optional)

### `<returns>` — Return Value

**Attributes:** `type` (required), `pointer` (optional), `const` (optional)

### Documentation Elements

- `<summary>` - Single-line description
- `<details>` - Multi-paragraph description (can contain `<p>` tags and `<snippet>` refs)
- `<xmlns>` - XML namespace for the component in UI XML files

## Common Data Types

**Primitive Types:** `bool`, `int`, `uint`, `float`, `string`, `void`

**Geometry Types:** `vec2`, `vec3`, `vec4`, `rect`, `mat2`, `mat3`, `mat4`, `color`

**Engine Types:** `Object`, `Texture`, `Material`, and custom types defined in other modules

## Example: Complete Module

```xml
<?xml version="1.0"?>
<!DOCTYPE module SYSTEM "https://corepunch.github.io/orca/schemas/module.dtd">
<module name="example" namespace="orca">
  <includes>
    <include file="source/core/core.h"/>
  </includes>
  <externals>
    <external struct="lua_State"/>
  </externals>

  <enums name="Direction">
    <summary>Cardinal directions</summary>
    <enum name="North">Northward direction</enum>
    <enum name="South">Southward direction</enum>
  </enums>

  <struct name="point" export="Point">
    <summary>2D point</summary>
    <fields>
      <field type="int" name="x">X coordinate</field>
      <field type="int" name="y">Y coordinate</field>
    </fields>
    <methods>
      <method name="Move">
        <summary>Move point in a direction</summary>
        <arg type="Direction" name="dir">Direction to move</arg>
        <arg type="int" name="distance">Distance to move</arg>
      </method>
    </methods>
  </struct>

  <class name="Marker" parent="Node2D">
    <summary>A visual marker placed in 2D space</summary>
    <handles>
      <handle message="Object.Create"/>
    </handles>
    <properties>
      <property name="Color" type="color">Marker color</property>
      <property name="Radius" type="float" default="5">Marker radius in pixels</property>
    </properties>
    <fields>
      <field name="_mesh" type="mesh" pointer="true"/>
    </fields>
    <messages>
      <message name="Clicked">
        <fields>
          <field name="X" type="int">Click X coordinate</field>
          <field name="Y" type="int">Click Y coordinate</field>
        </fields>
      </message>
    </messages>
  </class>

  <function name="CreatePoint">
    <summary>Create a new point</summary>
    <arg type="int" name="x">Initial X coordinate</arg>
    <arg type="int" name="y">Initial Y coordinate</arg>
    <returns type="point">New point instance</returns>
  </function>
</module>
```

## Schema Validation

Module XML files should reference the DTD schema for validation:

```xml
<?xml version="1.0"?>
<!DOCTYPE module SYSTEM "https://corepunch.github.io/orca/schemas/module.dtd">
```

## Best Practices

1. **Always include documentation** — Every element should have at least a `<summary>`
2. **Use container elements** — Group `<property>`, `<field>`, `<method>`, `<message>` and `<handle>` elements inside their respective container elements (`<properties>`, `<fields>`, `<methods>`, `<messages>`, `<handles>`)
3. **Use `<topic>` for large interfaces** — If a `<methods>` block has more than ~10 methods, add `<topic title="...">` separators between method groups (Lifecycle, Hierarchy, etc.)
4. **Follow element order** — For `<class>`: summary, details, xmlns, handles, properties, fields, methods, messages. For `<struct>`: summary, details, fields, methods
5. **Test generation** — Run `cd tools && make` after changes to verify XML is valid
6. **Check generated code** — Review generated `.h` and `_export.c` files to ensure correctness

## Troubleshooting

**Error: "Expected 'module' tag"**
- Ensure your root element is `<module>`

**Missing functions in Lua**
- Ensure the `export` attribute is set correctly on the method/function
- Verify `lua="true"` is set for Lua-accessible methods

**Type errors in generation**
- Verify all referenced types are defined (in this module or via `<externals>`)
- Ensure `<includes>` directives are present for external headers

**Fields not appearing in struct**
- Ensure `<field>` elements are inside a `<fields>` container directly in the struct or class
- Fields inside `<message>` elements are message parameters, not struct members

## Further Reading

- See existing module XML files in `source/` and `plugins/` for real-world examples
- Check `docs/schemas/module.dtd` for the complete schema definition
- See `tools/model/module.php` for the PHP data model that parses XML files
- See `tools/templates/` for the code generation templates
