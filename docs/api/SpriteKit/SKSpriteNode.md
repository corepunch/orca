# SKSpriteNode

**Class** | `orca.SpriteKit` | extends [SKNode](SKNode.md)

2D sprite node that renders an image or animation

## Overview

Renders 2D sprite content using either a static image or an animated sprite animation. Supports UV coordinates for sprite atlas sheets and various blend modes.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `Animation` | `SpriteAnimation` | Primary sprite animation from a sprite atlas |
| `Animation2` | `SpriteAnimation` | Secondary sprite animation (e.g., shadow layer) |
| `Image` | `Texture` | Static image texture to display |
| `FreezeFrame` | `int` | Freeze animation at a specific frame index (-1 for normal playback) |
| `BlendMode` | `BlendMode` | Blend mode for rendering |
| `UvRect` | `rect` | UV texture coordinates for sprite atlas sampling |

