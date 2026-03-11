# box3

**Struct** | `orca.geometry`

3D axis-aligned bounding box

## Overview

Represents a rectangular volume in 3D space defined by minimum and maximum corner points. Used for 3D collision detection and spatial culling.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `min` | `vec3` | Minimum corner point |
| `max` | `vec3` | Maximum corner point |

## Methods

### `center() → vec3`

Calculates the center point of the box

**Returns:** `vec3` — Center point of the bounding box

