# color

**Struct** | `orca.geometry`

RGBA color representation

## Overview

Represents a color with red, green, blue, and alpha (transparency) components. Values typically range from 0.0 to 1.0.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `r` | `float` | Red component (0.0 to 1.0) |
| `g` | `float` | Green component (0.0 to 1.0) |
| `b` | `float` | Blue component (0.0 to 1.0) |
| `a` | `float` | Alpha (transparency) component (0.0 = transparent, 1.0 = opaque) |

## Methods

### `lerp`

Linear interpolation between colors

**Parameters:** `other`: `color`, `t`: `float`

**Returns:** `color`

### `parse`

Parses color from string representation

**Parameters:** `code`: `string`

**Returns:** `color`

