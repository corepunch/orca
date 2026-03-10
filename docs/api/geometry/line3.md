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

### `intersect_sphere3`

Tests intersection with a sphere

**Parameters:** `sphere`: `sphere3`, `out`: `vec3`

**Returns:** `bool`

### `intersect_plane3`

Tests intersection with a plane

**Parameters:** `plane`: `plane3`, `out`: `vec3`

**Returns:** `bool`

### `intersect_triangle`

Tests intersection with a triangle

**Parameters:** `triangle`: `triangle3`, `out`: `vec3`

**Returns:** `bool`

### `intersect_box3`

Tests intersection with a 3D bounding box

**Parameters:** `box`: `box3`, `out`: `vec3`

**Returns:** `bool`

