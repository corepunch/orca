# NinePatchImage

**Class** | `orca.UIKit` | extends [Node2D](Node2D.md)

Specialized image view for nine-patch (stretchable) images

## Overview

Renders images that can be stretched while preserving corner and edge regions. Commonly used for UI panels, buttons, and borders that need to scale to different sizes.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `StretchTypeTop` | `float` | Top edge stretch behavior factor |
| `StretchTypeBottom` | `float` | Bottom edge stretch behavior factor |
| `StretchTypeLeft` | `float` | Left edge stretch behavior factor |
| `StretchTypeRight` | `float` | Right edge stretch behavior factor |
| `StretchTypeCenter` | `float` | Center region stretch behavior factor |
| `ImageTopLeft` | `Texture` | Top-left corner image (non-stretchable) |
| `ImageTop` | `Texture` | Top edge image (horizontally stretchable) |
| `ImageTopRight` | `Texture` | Top-right corner image (non-stretchable) |
| `ImageLeft` | `Texture` | Left edge image (vertically stretchable) |
| `ImageCenter` | `Texture` | Center region image (stretchable in both directions) |
| `ImageRight` | `Texture` | Right edge image (vertically stretchable) |
| `ImageBottomLeft` | `Texture` | Bottom-left corner image (non-stretchable) |
| `ImageBottom` | `Texture` | Bottom edge image (horizontally stretchable) |
| `ImageBottomRight` | `Texture` | Bottom-right corner image (non-stretchable) |

