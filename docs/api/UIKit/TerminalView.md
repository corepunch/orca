# TerminalView

**Class** | `orca.UIKit` | extends [Node2D](Node2D.md)

Displays a fixed-size text grid using a monospace layout

## Overview

Renders a character matrix (e.g., 80×24) where each cell represents a single glyph. Functions like a monospace canvas for text-mode visualization, ideal for console-style output, terminal emulation, or retro graphics where characters act as pixels.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `BufferWidth` | `int` | Buffer width |
| `BufferHeight` | `int` | Buffer height |
| `Cursor` | `int` | Cursor position |
| `SelectedIndex` | `int` | Selected item index |
| `DropShadow` | `bool` | Draw shadow underneath this view |

