# ImageView

**Class** | `orca.UIKit` | extends [Node2D](Node2D.md)

Displays image content with advanced rendering and scaling options

## Overview

Renders static images with support for nine-patch scaling, blend modes, and viewport control. Handles both loaded images and images loaded from file paths.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `Src` | `fixed` | Image source path or resource identifier |
| `Image` | `Texture` | Direct reference to loaded image |
| `Edges` | `vec4` | Edge inset values for nine-patch rendering |
| `Insets` | `vec4` | Content inset values for image positioning |
| `Viewbox` | `vec4` | Viewport rectangle for image cropping/scaling |
| `Stretch` | `Stretch` | Determines how the image is scaled to fill its allocated layout space |

