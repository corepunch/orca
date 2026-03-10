# orca.SceneKit

API reference for the `SceneKit` module.

## Enumerations

- [**StencilOp**](StencilOp.md) — Operations performed on stencil buffer values when stencil tests pass or fail
- [**ViewportMode**](ViewportMode.md) — Coordinate systems for viewport and scissor rectangle specification
- [**CullMode**](CullMode.md) — Face culling modes to optimize rendering by discarding non-visible triangle faces
- [**ColorWriteMode**](ColorWriteMode.md) — Control which color channels are written to the framebuffer during rendering
- [**FovType**](FovType.md) — Field of view calculation methods for camera projection
- [**ProjectionType**](ProjectionType.md) — Camera projection modes for 3D to 2D transformation
- [**LightType**](LightType.md) — Types of light sources for scene illumination

## Classes

- [**Node3D**](Node3D.md) — Base class for all 3D scene graph nodes with transform and positioning capabilities
- [**Scene**](Scene.md) — Root container for 3D scene content with camera and rendering configuration
- [**Model3D**](Model3D.md) — 3D model renderer that displays mesh geometry with materials
- [**PlaneMeshNode**](PlaneMeshNode.md) — Procedural plane mesh generator for creating rectangular surfaces
- [**Camera**](Camera.md) — 3D camera for scene viewing and projection configuration
- [**TrajectoryList3D**](TrajectoryList3D.md) — Layout container that arranges child nodes along a 3D trajectory path
- [**Viewport3D**](Viewport3D.md) — 2D viewport component that renders 3D scene content
- [**PrefabView3D**](PrefabView3D.md) — Dynamic loader for 3D prefab assets and scene files
- [**RenderPass**](RenderPass.md) — Base class for render pass components in the rendering pipeline
- [**CompositionTargetRenderPass**](CompositionTargetRenderPass.md) — Render pass that outputs to the final composition target
- [**BlitRenderPass**](BlitRenderPass.md) — Render pass for copying (blitting) between render targets
- [**ClearRenderPass**](ClearRenderPass.md) — Render pass for clearing framebuffer contents
- [**DrawObjectsRenderPass**](DrawObjectsRenderPass.md) — Render pass for drawing scene objects with tag-based filtering
- [**PipelineStateRenderPass**](PipelineStateRenderPass.md) — Render pass for configuring graphics pipeline state
- [**TextBlock3D**](TextBlock3D.md) — 3D text renderer for displaying typography in 3D space
- [**Light3D**](Light3D.md) — 3D light source for scene illumination and shadow casting
- [**SpriteView**](SpriteView.md) — Billboard sprite renderer for 2D images in 3D space

