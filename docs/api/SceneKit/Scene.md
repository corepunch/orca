# Scene

**Class** | `orca.SceneKit` | extends [Node3D](Node3D.md)

Root container for 3D scene content with camera and rendering configuration

## Overview

Represents a complete 3D scene that can be rendered to a viewport. Manages cameras, render passes, and global scene properties like environment lighting.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `Camera` | `fixed` | Name of the primary camera used for scene rendering |
| `PreviewCamera` | `fixed` | Name of camera used for preview rendering in design tools |
| `HitTestCamera` | `fixed` | Name of camera used for 3D hit testing and mouse interaction |
| `RenderPass` | `RenderPass` | Custom render pass chain for advanced rendering effects |
| `BackgroundTimelineSequence` | `fixed` | TODO: add support? |
| `ComposerOverride` | `RenderPass` | Override render pass for post-processing composition effects |
| `EnvironmentTexture` | `Texture` | Environment map texture for reflections and ambient lighting |

