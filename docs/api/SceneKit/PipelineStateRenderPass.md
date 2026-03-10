# PipelineStateRenderPass

**Class** | `orca.SceneKit` | extends [RenderPass](RenderPass.md)

Render pass for configuring graphics pipeline state

## Overview

Sets up the complete graphics pipeline state including blending, culling, depth testing, stencil operations, and viewport configuration. Essential for controlling how geometry is rasterized and composited.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `BlendMode` | `BlendMode` | Color blending mode for transparency and compositing effects |
| `ColorWriteMode` | `ColorWriteMode` | Which color channels are written to the framebuffer |
| `CullMode` | `CullMode` | Triangle face culling mode for performance optimization |
| `DepthTestFunction` | `CompareFunc` | Comparison function for depth buffer testing |
| `DepthWriteEnabled` | `bool` | Whether to write depth values to the depth buffer |
| `ViewportMode` | `ViewportMode` | Coordinate system for viewport rectangle specification |
| `ScissorMode` | `ViewportMode` | Coordinate system for scissor rectangle specification |
| `Viewport` | `vec4` | Viewport rectangle (x, y, width, height) for rendering area |
| `Scissor` | `vec4` | Scissor rectangle (x, y, width, height) for pixel clipping |
| `StencilTestFunction` | `CompareFunc` | Comparison function for stencil buffer testing |
| `StencilReferenceValue` | `int` | Reference value used in stencil test comparisons |
| `StencilMask` | `int` | Bitmask applied to stencil values before testing |
| `StencilFailOperation` | `StencilOp` | Operation when stencil test fails |
| `StencilPassDepthFailOperation` | `StencilOp` | Operation when stencil test passes but depth test fails |
| `StencilPassDepthPassOperation` | `StencilOp` | Operation when both stencil and depth tests pass |
| `StencilWriteEnabled` | `bool` | Whether to write computed stencil values to the stencil buffer |

