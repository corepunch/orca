# ClearRenderPass

**Class** | `orca.SceneKit` | extends [RenderPass](RenderPass.md)

Render pass for clearing framebuffer contents

## Overview

Clears color, depth, and stencil buffers to specified values. Essential for proper multi-pass rendering and avoiding artifacts from previous frame data.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `ClearColor` | `color` | Color value to clear the color buffer (RGBA format) |
| `ClearDepth` | `float` | Depth value to clear the depth buffer (typically 1.0 for far plane) |
| `ClearStencil` | `int` | Integer value to clear the stencil buffer (typically 0) |

