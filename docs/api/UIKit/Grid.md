# Grid

**Class** | `orca.UIKit` | extends [Node2D](Node2D.md)

Grid-based layout container with configurable rows and columns

## Overview

Arranges children in a 2D grid pattern with flexible sizing. Supports both fixed and dynamic grid definitions.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `Columns` | `fixed` | Column definition string (e.g., "1fr 2fr 100px") |
| `Rows` | `fixed` | Row definition string (e.g., "auto 1fr auto") |
| `Direction` | `Direction` | Primary fill direction for auto-placement |
| `Spacing` | `float` | Space between grid cells |
| `CellWidth` | `float` | Default width for auto-sized cells |
| `CellHeight` | `float` | Default height for auto-sized cells |

