# DrawBrushEventArgs

**Struct** | `orca.UIKit`

Event data structure for brush drawing operations

## Overview

Contains information required for rendering a brush, including transformation matrices, target textures, and brush configuration.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `projection` | `mat4` | Projection matrix for 3D to 2D transformation |
| `image` | `Texture` | Target image handle for rendering |
| `brush` | `BrushShorthand` | Brush configuration to draw with |
| `borderOffset` | `float` | Border offset adjustment for rendering |
| `borderWidth` | `vec4` | Border width for each edge (top, right, bottom, left) |
| `foreground` | `bool` | True if drawing foreground, false for background |
| `viewdef` | `ViewDef` | View definition context for rendering |

