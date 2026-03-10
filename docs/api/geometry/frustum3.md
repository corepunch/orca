# frustum3

**Struct** | `orca.geometry`

3D viewing frustum

## Overview

Represents a truncated pyramid used for camera view volumes and culling. Defined by six planes that bound the visible region.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `left` | `plane3` | Left clipping plane |
| `right` | `plane3` | Right clipping plane |
| `bottom` | `plane3` | Bottom clipping plane |
| `top` | `plane3` | Top clipping plane |
| `front` | `plane3` | Near clipping plane |
| `back` | `plane3` | Far clipping plane |

## Methods

### `calculate`

Extracts frustum planes from projection matrix

**Parameters:** `matrix`: `mat4`

**Returns:** `frustum3`

### `containsPoint`

Tests if a point is inside the frustum

**Parameters:** `point`: `vec3`

**Returns:** `bool`

### `containsSphere`

Tests if a sphere intersects the frustum

**Parameters:** `sphere`: `sphere3`

**Returns:** `bool`

### `containsBox`

Tests if a transformed bounding box intersects the frustum

**Parameters:** `box`: `box3`, `matrix`: `mat4`

**Returns:** `bool`

### `containsAABox`

Tests if an axis-aligned box intersects the frustum

**Parameters:** `box`: `box3`

**Returns:** `bool`

