# Style

**Class** | `orca.UIKit`

Defines a reusable set of property values and visual configurations for a target component type.

## Overview

Style enables consistent appearance and behavior across multiple components by grouping property definitions under a named style. Each style specifies its target type and classification, such as Generic, Theme, or Override. Styles can be applied globally, by theme, or locally to individual instances, allowing flexible and maintainable UI customization.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `TargetType` | `fixed` | Specifies the component type this style targets. Only components of the given type or its derived types will use the style. |
| `Type` | `StyleType` | Defines the classification of this style, such as Generic, Theme, Override, Local, or Template, determining its scope and application behavior. |

