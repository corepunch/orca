# PropertyType

**Struct** | `orca.core`

Defines a custom property type that can be attached to engine objects.

## Overview

PropertyType allows defining additional data fields beyond built-in engine properties. Each PropertyType specifies its data category, default value, and behavioral attributes such as layout and render impact. It serves as a flexible mechanism for extending object metadata in projects.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `Name` | `fixed` | Unique name identifier for the property type. |
| `Category` | `fixed` | Organizational category for this property, used for grouping in editors and UIs. |
| `DataType` | `DataType` | Underlying data type that determines how the property value is interpreted and stored. |
| `DefaultValue` | `fixed` | Default value assigned when the property is not explicitly set. |
| `TypeString` | `fixed` | String representation of the property type, used to store enum values or struct type names. |
| `AffectLayout` | `bool` | Indicates whether this property affects element layout (e.g., size or alignment). |
| `AffectRender` | `bool` | Indicates whether this property influences the rendering output. |
| `IsReadOnly` | `bool` | If true, the property value cannot be modified at runtime or through the editor. |
| `IsHidden` | `bool` | If true, the property is excluded from the UI or inspector views. |
| `IsInherited` | `bool` | Specifies whether the property value can be inherited from parent components. |
| `Key` | `fixed` | Internal key name used for property identification and lookup. |
| `Value` | `fixed` | Runtime value stored in this property instance. |
| `Step` | `float` | Increment step used for numeric adjustments in UI controls. |
| `UpperBound` | `float` | Maximum allowed value for numeric properties. |
| `LowerBound` | `float` | Minimum allowed value for numeric properties. |
| `ShortIdentifier` | `uint` | Unique short identifier for the property type, automatically generated from implicit property name. |
| `FullIdentifier` | `uint` | Unique full identifier for the property type, automatically generated from explicit (ie. Grid.Columns) property name. |
| `Offset` | `uint` | Byte offset of the property within the structure. |
| `DataSize` | `uint` | Size of the property data in bytes. |
| `IsArray` | `bool` | Indicates whether the property is an array type, will generate Num* property to indicate the number of elements. |

