# Module XML File Guide

## Overview

This guide explains how to create and maintain module XML files in the ORCA framework. Module XML files are used to define the API interface between C code and Lua, automatically generating C headers, bindings, and HTML documentation.

## What Are Module XML Files?

Module XML files describe the public API of C modules in a declarative format. The `conv-module.py` script processes these XML files to generate:

1. **C Header Files** (`.h`) - Type definitions and function declarations
2. **C Implementation Files** (`.c`) - Lua bindings and registration code
3. **HTML Documentation** - Formatted API reference at `docs/index.html`
4. **DTD Schema** - XML validation schema at `tools/schemas/orca.dtd` (for UI components, generated from module files)

## Location and Usage

Module XML files are located in the `source/` directory alongside their corresponding C source code:

- `source/geometry/geom.xml` - Geometry types (vectors, matrices, etc.)
- `source/core/core.xml` - Core engine objects and components
- `source/renderer/api/renderer.xml` - Rendering interfaces
- `source/UIKit/UIKit.xml` - UI components
- `source/SceneKit/SceneKit.xml` - 3D scene graph components
- `source/filesystem/filesystem.xml` - File system access

To generate code from these XML files, run:

```bash
make modules
```

This executes `tools/conv-module.py` which processes all module XML files.

## XML File Structure

Every module XML file must have this basic structure:

```xml
<?xml version="1.0"?>
<!DOCTYPE module SYSTEM "https://corepunch.github.io/orca/schemas/module.dtd">
<module name="modulename" namespace="orca">
  <!-- Module content goes here -->
</module>
```

### Root Element: `<module>`

The root element defines the module's identity and namespace:

**Attributes:**
- `name` (required) - The module name (e.g., "geometry", "core")
- `namespace` (required) - Lua namespace (typically "orca")
- `on-luaopen` (optional) - Callback function name to execute when module loads
- `prefix` (optional) - Prefix for generated C identifiers

**Example:**
```xml
<module name="geometry" namespace="orca">
  <!-- ... -->
</module>
```

## Top-Level Elements

Module XML files can contain the following top-level elements:

### 1. `<enums>` - Enumeration Definitions

Defines an enumeration type with named constants.

**Attributes:**
- `name` (required) - Name of the enumeration type

**Child Elements:**
- `<summary>` - Brief description
- `<details>` - Detailed explanation
- `<enum>` - Individual enumeration value

**Example:**
```xml
<enums name="RotationOrder">
  <summary>Euler angle rotation order enumeration</summary>
  <details>Specifies the order in which rotations are applied around the X, Y, and Z axes.</details>
  <enum name="XYZ">Rotate around X axis first, then Y, then Z</enum>
  <enum name="XZY">Rotate around X axis first, then Z, then Y</enum>
  <enum name="YZX">Rotate around Y axis first, then Z, then X</enum>
</enums>
```

### 2. `<struct>` - Structure Definitions

Defines a C struct with fields and methods.

**Attributes:**
- `name` (required) - Name of the struct
- `prefix` (optional) - Prefix for C constants (e.g., "VEC2_")
- `export` (optional) - Name exported to Lua (defaults to struct name)
- `sealed` (optional) - If "true", prevents field access from Lua

**Child Elements:**
- `<summary>` - Brief description
- `<details>` - Detailed explanation
- `<field>` - Data field definition
- `<method>` - Method definition

**Example:**
```xml
<struct name="vec2" prefix="VEC2_" export="Vector2D" sealed="true">
  <summary>2D vector structure</summary>
  <details>Represents a point or direction in 2D space.</details>
  <field type="float" name="x">X coordinate component</field>
  <field type="float" name="y">Y coordinate component</field>
  <method name="Add" export="__add" const="true">
    <summary>Adds two vectors component-wise</summary>
    <arg type="vec2" name="other" pointer="true" const="true">Vector to add</arg>
    <returns type="vec2">Sum of the two vectors</returns>
  </method>
</struct>
```

### 3. `<interface>` - Interface Definitions

Similar to `<struct>`, but defines an abstract interface or base class.

**Attributes:** Same as `<struct>`, plus:
- `no-check` (optional) - If "true", skips type checking in generated code

**Example:**
```xml
<interface name="Object" prefix="OBJ_" export="Object" no-check="true">
  <summary>Core engine host object.</summary>
  <init/>
  <method name="Clear" export="clear" lua="true">
    <summary>Clear all children of the object.</summary>
  </method>
</interface>
```

### 4. `<component>` - Component Definitions

Defines a component that can be attached to objects. Components extend the object system.

**Attributes:**
- `name` (required) - Component name
- `parent` (optional) - Parent component to inherit from
- `concept` (optional) - Interface this component implements

**Child Elements:**
- `<summary>` - Brief description
- `<details>` - Detailed explanation
- `<property>` - Component property
- `<handles>` - Event handler declaration

**Example:**
```xml
<component name="PropertyType">
  <summary>Defines a custom property type.</summary>
  <handles event="Attached"/>
  <property name="Category" type="fixed">Property category.</property>
  <property name="DataType" type="DataType">Underlying data type.</property>
</component>
```

### 5. `<function>` - Global Function Definitions

Defines a global function in the module namespace.

**Attributes:**
- `name` (required) - Function name

**Child Elements:**
- `<summary>` - Brief description
- `<details>` - Detailed explanation
- `<arg>` - Function argument
- `<returns>` - Return value description

**Example:**
```xml
<function name="GetFocus">
  <summary>Retrieves currently active object.</summary>
  <returns type="Object" pointer="true">Pointer to the focused object</returns>
</function>
```

### 6. `<event>` - Event Definitions

Declares an event that can be handled by components.

**Attributes:**
- `name` (required) - Event name
- `type` (optional) - Event parameter type

**Example:**
```xml
<event name="LeftMouseDown" type="WI_Message"/>
<event name="WindowPaint"/>
```

### 7. `<resource>` - Resource Type Definitions

Declares a resource type that can be loaded by the engine.

**Attributes:**
- `type` (required) - Resource type name
- `no-lua` (optional) - If "true", excludes from Lua bindings

**Example:**
```xml
<resource type="localization"/>
<resource type="Property" no-lua="true"/>
```

### 8. `<include>` - C Header Includes

Specifies additional C header files to include in generated code.

**Attributes:**
- `file` (required) - Path to header file

**Example:**
```xml
<include file="source/renderer/api/renderer.h"/>
```

### 9. `<external>` - External Type References

Declares types defined elsewhere that are referenced in this module.

**Attributes:**
- `struct` (required) - Name of external struct

**Example:**
```xml
<external struct="Object"/>
```

## Child Element Details

### `<field>` - Struct/Interface Fields

Defines a data member in a struct or interface.

**Attributes:**
- `name` (required) - Field name
- `type` (required) - Data type
- `array` (optional) - If present, field is an array
- `pointer` (optional) - If "true", field is a pointer
- `const` (optional) - If "true", field is const

**Content:** Description text

**Example:**
```xml
<field type="float" name="x">X coordinate component</field>
<field name="children" type="Object" array="true" pointer="true">Child objects</field>
```

### `<method>` - Struct/Interface/Component Methods

Defines a method or member function.

**Attributes:**
- `name` (required) - Method name
- `export` (optional) - Name exported to Lua (e.g., "__add" for operator overloading)
- `const` (optional) - If "true", method is const
- `lua` (optional) - If "true", method is Lua-only

**Child Elements:**
- `<summary>` - Brief description
- `<details>` - Detailed explanation
- `<arg>` - Method argument
- `<returns>` - Return value description

**Example:**
```xml
<method name="Add" export="__add" const="true">
  <summary>Adds two vectors component-wise</summary>
  <arg type="vec2" name="other" pointer="true" const="true">Vector to add</arg>
  <returns type="vec2">Sum of the two vectors</returns>
</method>
```

### `<arg>` - Function/Method Arguments

Defines a function or method parameter.

**Attributes:**
- `name` (required) - Parameter name
- `type` (required) - Parameter type
- `pointer` (optional) - If "true", parameter is a pointer
- `const` (optional) - If "true", parameter is const

**Content:** Description text

**Example:**
```xml
<arg type="float" name="s">Scale factor to apply</arg>
```

### `<returns>` - Return Value

Describes what a function or method returns.

**Attributes:**
- `type` (required) - Return type
- `pointer` (optional) - If "true", returns a pointer
- `const` (optional) - If "true", returns const value

**Content:** Description text

**Example:**
```xml
<returns type="vec2">New scaled vector</returns>
```

### `<property>` - Component Properties

Defines a property on a component.

**Attributes:**
- `name` (required) - Property name
- `type` (required) - Property data type

**Content:** Description text

**Example:**
```xml
<property name="Category" type="fixed">Organizational category for this property.</property>
```

### `<handles>` - Event Handlers

Declares that a component handles a specific event.

**Attributes:**
- `event` (required) - Name of event being handled

**Example:**
```xml
<handles event="Attached"/>
```

### `<init>` - Initialization Marker

Indicates that a struct/interface has an initialization method.

**Example:**
```xml
<init/>
```

### Documentation Elements

- `<summary>` - Single-line description (plain text)
- `<details>` - Multi-paragraph description (can contain HTML `<p>` tags and inline code)
- `<snippet>` - Code example inclusion

**Example:**
```xml
<summary>2D vector structure</summary>
<details>
  <p>Represents a point or direction in 2D space with x and y components.</p>
  <p>Supports common vector operations like addition, scaling, and distance calculations.</p>
</details>
<snippet src="docs/examples/vector_usage.lua"/>
```

## Common Data Types

Module XML files use these common type names:

**Primitive Types:**
- `bool` - Boolean value
- `int` - Integer number
- `float` - Floating-point number
- `string` - String (const char*)
- `void` - No value

**Geometry Types:**
- `vec2`, `vec3`, `vec4` - 2D/3D/4D vectors
- `box2`, `box3` - 2D/3D bounding boxes
- `rect` - 2D rectangle
- `mat2`, `mat3`, `mat4` - 2D/3D/4D matrices
- `transform2d`, `transform3d` - Transformation matrices

**Engine Types:**
- `Object` - Base object type
- Custom types defined in other modules

## Example: Complete Module

Here's a minimal complete example:

```xml
<?xml version="1.0"?>
<!DOCTYPE module SYSTEM "https://corepunch.github.io/orca/schemas/module.dtd">
<module name="example" namespace="orca">
  <enums name="Direction">
    <summary>Cardinal directions</summary>
    <enum name="North">Northward direction</enum>
    <enum name="South">Southward direction</enum>
    <enum name="East">Eastward direction</enum>
    <enum name="West">Westward direction</enum>
  </enums>

  <struct name="point" export="Point">
    <summary>2D point</summary>
    <field type="int" name="x">X coordinate</field>
    <field type="int" name="y">Y coordinate</field>
    <method name="Move">
      <summary>Move point in a direction</summary>
      <arg type="Direction" name="dir">Direction to move</arg>
      <arg type="int" name="distance">Distance to move</arg>
    </method>
  </struct>

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

The DTD schema is published at `https://corepunch.github.io/orca/schemas/module.dtd` via GitHub Pages, ensuring it's always accessible for validation.

## Best Practices

1. **Always include documentation** - Every element should have at least a `<summary>`
2. **Use consistent naming** - Follow C naming conventions for elements
3. **Group related items** - Place related structs, enums, and functions together
4. **Keep it simple** - XML should describe the API, not implementation details
5. **Test generation** - Run `make modules` after changes to verify XML is valid
6. **Check generated code** - Review the generated `.h` and `.c` files to ensure correctness

## Troubleshooting

**Error: "Expected 'module' tag"**
- Ensure your root element is `<module>`, not something else

**Error: "File not found"**
- Check that the XML file path in the Makefile is correct

**Missing functions in Lua**
- Ensure the `export` attribute is set correctly
- Check that the function/method is properly defined in the XML

**Type errors**
- Verify all referenced types are defined (in this module or via `<external>`)
- Ensure `<include>` directives are present for external headers

## Further Reading

- See `tools/conv-module.py` for the complete implementation
- Check existing module XML files in `source/` for real-world examples
- Review generated files in `source/*/` to understand the output
- See `docs/index.html` for the generated API documentation
