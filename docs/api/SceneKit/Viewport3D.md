# Viewport3D

**Class** | `orca.SceneKit` | extends [Node2D](Node2D.md)

2D viewport component that renders 3D scene content

## Overview

Bridge between 2D UI layout and 3D scene rendering. Allows embedding 3D scenes within 2D user interfaces with independent camera and rendering configuration.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `Camera` | `fixed` | Name of camera to use for 3D scene rendering within this viewport |
| `PreviewCamera` | `fixed` | Name of camera used for design-time preview rendering |
| `HitTestCamera` | `fixed` | Name of camera used for 3D hit testing within the viewport area |
| `RenderPass` | `RenderPass` | Custom render pass chain for viewport-specific effects |
| `Scene` | `fixed` | TODO: add support? |

