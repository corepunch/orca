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

### `lerp(other: color, t: float) → color`

Linear interpolation between colors

| Parameter | Type | Description |
|-----------|------|-------------|
| `other` | `color` | Target color to interpolate towards |
| `t` | `float` | Interpolation factor (0.0 = this color, 1.0 = other color) |

**Returns:** `color` — Interpolated color

### `parse(code: string) → color`

Parses color from string representation

| Parameter | Type | Description |
|-----------|------|-------------|
| `code` | `string` | Color string to parse |

**Returns:** `color` — Parsed color value

