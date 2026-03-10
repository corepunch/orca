# DrawObjectsRenderPass

**Class** | `orca.SceneKit` | extends [RenderPass](RenderPass.md)

Render pass for drawing scene objects with tag-based filtering

## Overview

Renders scene geometry based on object tags and camera configuration. Allows selective rendering of object groups for advanced multi-pass effects and optimization.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `Camera` | `fixed` | Name of camera to use for object rendering and view/projection matrices |
| `IncludeTags` | `objectTags` | Object tags that must be present for objects to be rendered |
| `ExcludeTags` | `objectTags` | Object tags that prevent objects from being rendered |

