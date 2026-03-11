# line3

**Struct** | `orca.geometry`

3D line segment

## Overview

Represents a line segment in 3D space defined by two endpoints. Provides intersection testing with various geometric primitives.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `a` | `vec3` | Start point of the line segment |
| `b` | `vec3` | End point of the line segment |

## Methods

### `intersect_sphere3(sphere: sphere3, out: vec3) → bool`

Tests intersection with a sphere

| Parameter | Type | Description |
|-----------|------|-------------|
| `sphere` | `sphere3` | Sphere to test intersection with |
| `out` | `vec3` | Output parameter for intersection point |

**Returns:** `bool` — True if intersection occurs, false otherwise

### `intersect_plane3(plane: plane3, out: vec3) → bool`

Tests intersection with a plane

| Parameter | Type | Description |
|-----------|------|-------------|
| `plane` | `plane3` | Plane to test intersection with |
| `out` | `vec3` | Output parameter for intersection point |

**Returns:** `bool` — True if intersection occurs, false otherwise

### `intersect_triangle(triangle: triangle3, out: vec3) → bool`

Tests intersection with a triangle

| Parameter | Type | Description |
|-----------|------|-------------|
| `triangle` | `triangle3` | Triangle to test intersection with |
| `out` | `vec3` | Output parameter for intersection point |

**Returns:** `bool` — True if intersection occurs within triangle, false otherwise

### `intersect_box3(box: box3, out: vec3) → bool`

Tests intersection with a 3D bounding box

| Parameter | Type | Description |
|-----------|------|-------------|
| `box` | `box3` | Bounding box to test intersection with |
| `out` | `vec3` | Output parameter for intersection point |

**Returns:** `bool` — True if intersection occurs, false otherwise

