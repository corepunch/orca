# DataType

**Enum** | `orca.core`

Specifies the underlying data type of a property.

## Overview

DataType defines the fundamental type of value a property can hold, ranging from basic primitives like Bool and Float to complex structures such as Transform3D or custom Objects. It helps the engine interpret, validate, and serialize property values appropriately.

## Constants

| Name | Description |
|------|-------------|
| `None` | No data type specified. |
| `Bool` | Boolean value representing true or false. |
| `Int` | Signed integer value. |
| `Enum` | Enumeration type represented by integer values mapped to named constants. |
| `Float` | Floating-point numeric value. |
| `Fixed` | Fixed-length string or symbolic value. |
| `LongString` | Extended string data, intended for larger text content. |
| `ObjectTags` | List of tag identifiers associated with an object. |
| `Event` | Event reference used to bind triggers or callbacks. |
| `Struct` | Composite data structure containing multiple fields used for packaging related geometric, visual, and layout properties together. |
| `Object` | Reference to a complex object instance. |

