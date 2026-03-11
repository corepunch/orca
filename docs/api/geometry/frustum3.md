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

### `calculate(matrix: mat4) → frustum3`

Extracts frustum planes from projection matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `matrix` | `mat4` | View-projection matrix |

**Returns:** `frustum3` — Calculated frustum with normalized planes

### `containsPoint(point: vec3) → bool`

Tests if a point is inside the frustum

| Parameter | Type | Description |
|-----------|------|-------------|
| `point` | `vec3` | 3D point to test |

**Returns:** `bool` — True if point is inside all frustum planes

### `containsSphere(sphere: sphere3) → bool`

Tests if a sphere intersects the frustum

| Parameter | Type | Description |
|-----------|------|-------------|
| `sphere` | `sphere3` | Sphere to test |

**Returns:** `bool` — True if sphere intersects or is inside frustum

### `containsBox(box: box3, matrix: mat4) → bool`

Tests if a transformed bounding box intersects the frustum

| Parameter | Type | Description |
|-----------|------|-------------|
| `box` | `box3` | Axis-aligned bounding box |
| `matrix` | `mat4` | Transformation matrix for the box |

**Returns:** `bool` — True if transformed box intersects frustum

### `containsAABox(box: box3) → bool`

Tests if an axis-aligned box intersects the frustum

| Parameter | Type | Description |
|-----------|------|-------------|
| `box` | `box3` | Axis-aligned bounding box to test |

**Returns:** `bool` — True if box intersects frustum

