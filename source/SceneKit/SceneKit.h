#ifndef __SCENEKIT_H__
#define __SCENEKIT_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;


#include <include/renderer.h>
#include <source/core/core.h>
#include <source/UIKit/UIKit.h>

#include "SceneKit_properties.h"
#include "../renderer/api/renderer.h"


/// @brief Operations performed on stencil buffer values when stencil tests pass or fail
/** StencilOp enum */
typedef enum StencilOp {
	kStencilOpKeep, ///< Keep the current stencil buffer value unchanged
	kStencilOpZero, ///< Set the stencil buffer value to zero
	kStencilOpReplace, ///< Replace stencil buffer value with the reference value
	kStencilOpIncrement, ///< Increment stencil buffer value, clamping to maximum
	kStencilOpIncrementWrap, ///< Increment stencil buffer value, wrapping to zero at maximum
	kStencilOpDecrement, ///< Decrement stencil buffer value, clamping to zero
	kStencilOpDecrementWrap, ///< Decrement stencil buffer value, wrapping to maximum at zero
	kStencilOpInvert, ///< Bitwise invert the stencil buffer value
} eStencilOp_t;
#define StencilOp_Count 8
ORCA_API const char *StencilOpToString(enum StencilOp value);
ORCA_API enum StencilOp luaX_checkStencilOp(lua_State *L, int idx);
ORCA_API void luaX_pushStencilOp(lua_State *L, enum StencilOp value);
/// @brief Coordinate systems for viewport and scissor rectangle specification
/** ViewportMode enum */
typedef enum ViewportMode {
	kViewportModeRelative, ///< Coordinates specified as normalized values (0.0 to 1.0) relative to render target size
	kViewportModeAbsolute, ///< Coordinates specified in absolute pixels
} eViewportMode_t;
#define ViewportMode_Count 2
ORCA_API const char *ViewportModeToString(enum ViewportMode value);
ORCA_API enum ViewportMode luaX_checkViewportMode(lua_State *L, int idx);
ORCA_API void luaX_pushViewportMode(lua_State *L, enum ViewportMode value);
/// @brief Face culling modes to optimize rendering by discarding non-visible triangle faces
/** CullMode enum */
typedef enum CullMode {
	kCullModeNone, ///< No face culling - render both front and back faces
	kCullModeBack, ///< Cull back-facing triangles (standard for solid objects)
	kCullModeFront, ///< Cull front-facing triangles (useful for interior views or special effects)
} eCullMode_t;
#define CullMode_Count 3
ORCA_API const char *CullModeToString(enum CullMode value);
ORCA_API enum CullMode luaX_checkCullMode(lua_State *L, int idx);
ORCA_API void luaX_pushCullMode(lua_State *L, enum CullMode value);
/// @brief Control which color channels are written to the framebuffer during rendering
/** ColorWriteMode enum */
typedef enum ColorWriteMode {
	kColorWriteModeNone, ///< Disable all color channel writes (useful for depth-only or stencil-only passes)
	kColorWriteModeRGB, ///< Write to red, green, and blue channels only
	kColorWriteModeRGBA, ///< Write to all four color channels including alpha
	kColorWriteModeR, ///< Write to red channel only
	kColorWriteModeG, ///< Write to green channel only
	kColorWriteModeB, ///< Write to blue channel only
	kColorWriteModeGB, ///< Write to green and blue channels only
	kColorWriteModeA, ///< Write to alpha channel only
} eColorWriteMode_t;
#define ColorWriteMode_Count 8
ORCA_API const char *ColorWriteModeToString(enum ColorWriteMode value);
ORCA_API enum ColorWriteMode luaX_checkColorWriteMode(lua_State *L, int idx);
ORCA_API void luaX_pushColorWriteMode(lua_State *L, enum ColorWriteMode value);
/// @brief Field of view calculation methods for camera projection
/** FovType enum */
typedef enum FovType {
	kFovTypeXfov, ///< Field of view specified as horizontal angle in degrees
	kFovTypeYfov, ///< Field of view specified as vertical angle in degrees (most common)
} eFovType_t;
#define FovType_Count 2
ORCA_API const char *FovTypeToString(enum FovType value);
ORCA_API enum FovType luaX_checkFovType(lua_State *L, int idx);
ORCA_API void luaX_pushFovType(lua_State *L, enum FovType value);
/// @brief Camera projection modes for 3D to 2D transformation
/** ProjectionType enum */
typedef enum ProjectionType {
	kProjectionTypePerspective, ///< Perspective projection with realistic depth foreshortening
	kProjectionTypeOrthographic, ///< Orthographic projection with parallel lines (no perspective distortion)
} eProjectionType_t;
#define ProjectionType_Count 2
ORCA_API const char *ProjectionTypeToString(enum ProjectionType value);
ORCA_API enum ProjectionType luaX_checkProjectionType(lua_State *L, int idx);
ORCA_API void luaX_pushProjectionType(lua_State *L, enum ProjectionType value);
/// @brief Types of light sources for scene illumination
/** LightType enum */
typedef enum LightType {
	kLightTypePoint, ///< Omnidirectional point light source (like a light bulb)
	kLightTypeSpot, ///< Directional spot light with cone-shaped illumination area
} eLightType_t;
#define LightType_Count 2
ORCA_API const char *LightTypeToString(enum LightType value);
ORCA_API enum LightType luaX_checkLightType(lua_State *L, int idx);
ORCA_API void luaX_pushLightType(lua_State *L, enum LightType value);





/// @brief Base class for all 3D scene graph nodes with transform and positioning capabilities
/** Node3D component */
typedef struct Node3D Node3D_t, *Node3DPtr, *lpNode3D_t;
typedef struct Node3D const *Node3DCPtr, *lpcNode3D_t;
struct Node3D {
	struct transform3 LayoutTransform; ///< Transform applied during layout phase for positioning within parent containers
	struct transform3 RenderTransform; ///< Additional transform applied during rendering for animations and effects
	struct vec3 RenderTransformOrigin; ///< Origin point for render transform operations (rotation, scaling center)
	struct vec3 ContentOffset; ///< Offset applied to child content within this node's local coordinate system
	struct mat4 Matrix; ///< Final combined transformation matrix (computed automatically)
	float CalculatedOffset; ///< Computed offset value used for trajectory list positioning
	float _opacity;
};
ORCA_API void luaX_pushNode3D(lua_State *L, struct Node3D const* Node3D);
ORCA_API struct Node3D* luaX_checkNode3D(lua_State *L, int idx);
/// @brief Root container for 3D scene content with camera and rendering configuration
/** Scene component */
typedef struct Scene Scene_t, *ScenePtr, *lpScene_t;
typedef struct Scene const *SceneCPtr, *lpcScene_t;
struct Scene {
	fixedString_t Camera; ///< Name of the primary camera used for scene rendering
	fixedString_t PreviewCamera; ///< Name of camera used for preview rendering in design tools
	fixedString_t HitTestCamera; ///< Name of camera used for 3D hit testing and mouse interaction
	struct RenderPass* RenderPass; ///< Custom render pass chain for advanced rendering effects
	fixedString_t BackgroundTimelineSequence; ///< TODO: add support?
	struct RenderPass* ComposerOverride; ///< Override render pass for post-processing composition effects
	struct Texture* EnvironmentTexture; ///< Environment map texture for reflections and ambient lighting
};
ORCA_API void luaX_pushScene(lua_State *L, struct Scene const* Scene);
ORCA_API struct Scene* luaX_checkScene(lua_State *L, int idx);
/// @brief 3D model renderer that displays mesh geometry with materials
/** Model3D component */
typedef struct Model3D Model3D_t, *Model3DPtr, *lpModel3D_t;
typedef struct Model3D const *Model3DCPtr, *lpcModel3D_t;
struct Model3D {
	struct Mesh* Mesh; ///< 3D mesh geometry defining the shape of the model
	struct Material* Material; ///< Material defining surface appearance, textures, and shading properties
};
ORCA_API void luaX_pushModel3D(lua_State *L, struct Model3D const* Model3D);
ORCA_API struct Model3D* luaX_checkModel3D(lua_State *L, int idx);
/// @brief Procedural plane mesh generator for creating rectangular surfaces
/** PlaneMeshNode component */
typedef struct PlaneMeshNode PlaneMeshNode_t, *PlaneMeshNodePtr, *lpPlaneMeshNode_t;
typedef struct PlaneMeshNode const *PlaneMeshNodeCPtr, *lpcPlaneMeshNode_t;
struct PlaneMeshNode {
	float PlaneWidth; ///< Width of the generated plane in world units
	float PlaneHeight; ///< Height of the generated plane in world units
	bool_t PlaneInvertU; ///< Invert horizontal texture coordinates (flip texture horizontally)
	bool_t PlaneInvertV; ///< Invert vertical texture coordinates (flip texture vertically)
	bool_t GenerateTangents; ///< Generate tangent vectors for normal mapping (requires additional memory)
	bool_t GenerateNormals; ///< Generate vertex normals for proper lighting (usually true for lit surfaces)
	struct Material* MeshMaterial; ///< Override material specifically for the generated mesh geometry
};
ORCA_API void luaX_pushPlaneMeshNode(lua_State *L, struct PlaneMeshNode const* PlaneMeshNode);
ORCA_API struct PlaneMeshNode* luaX_checkPlaneMeshNode(lua_State *L, int idx);
/// @brief 3D camera for scene viewing and projection configuration
/** Camera component */
typedef struct Camera Camera_t, *CameraPtr, *lpCamera_t;
typedef struct Camera const *CameraCPtr, *lpcCamera_t;
struct Camera {
	float Fov; ///< Field of view angle in degrees (60-90 typical for perspective cameras)
	enum FovType FovType; ///< Whether field of view is measured horizontally or vertically
	enum ProjectionType ProjectionType; ///< Perspective or orthographic projection mode
	float ZNear; ///< Near clipping plane distance (objects closer than this are not rendered)
	float ZFar; ///< Far clipping plane distance (objects farther than this are not rendered)
	float EyeSeparation; ///< Distance between left and right eyes for stereoscopic rendering
	float FocalDistance; ///< Focus distance for depth of field and stereo convergence
	float OrthogonalPlaneHeight; ///< Height of viewing volume for orthographic projection
	bool_t ZPositive; ///< Whether positive Z axis points toward or away from viewer
};
ORCA_API void luaX_pushCamera(lua_State *L, struct Camera const* Camera);
ORCA_API struct Camera* luaX_checkCamera(lua_State *L, int idx);
/// @brief Layout container that arranges child nodes along a 3D trajectory path
/** TrajectoryList3D component */
typedef struct TrajectoryList3D TrajectoryList3D_t, *TrajectoryList3DPtr, *lpTrajectoryList3D_t;
typedef struct TrajectoryList3D const *TrajectoryList3DCPtr, *lpcTrajectoryList3D_t;
struct TrajectoryList3D {
	struct Trajectory* Trajectory; ///< 3D path shape defining the trajectory curve for child positioning
	struct vec2 ScrollAxis; ///< Primary and secondary scroll directions for trajectory navigation
	bool_t Looping; ///< Whether trajectory wraps around at the ends (creates infinite loop)
	float Spacing; ///< Distance between adjacent items along the trajectory path
	float SelectionBehavior; ///< Controls how selection affects item positioning and scaling
	float SelectedItemIndex; ///< Index of currently selected item (supports fractional values for smooth transitions)
};
ORCA_API void luaX_pushTrajectoryList3D(lua_State *L, struct TrajectoryList3D const* TrajectoryList3D);
ORCA_API struct TrajectoryList3D* luaX_checkTrajectoryList3D(lua_State *L, int idx);
/// @brief 2D viewport component that renders 3D scene content
/** Viewport3D component */
typedef struct Viewport3D Viewport3D_t, *Viewport3DPtr, *lpViewport3D_t;
typedef struct Viewport3D const *Viewport3DCPtr, *lpcViewport3D_t;
struct Viewport3D {
	fixedString_t Camera; ///< Name of camera to use for 3D scene rendering within this viewport
	fixedString_t PreviewCamera; ///< Name of camera used for design-time preview rendering
	fixedString_t HitTestCamera; ///< Name of camera used for 3D hit testing within the viewport area
	struct RenderPass* RenderPass; ///< Custom render pass chain for viewport-specific effects
	fixedString_t Scene; ///< TODO: add support?
};
ORCA_API void luaX_pushViewport3D(lua_State *L, struct Viewport3D const* Viewport3D);
ORCA_API struct Viewport3D* luaX_checkViewport3D(lua_State *L, int idx);
/// @brief Dynamic loader for 3D prefab assets and scene files
/** PrefabView3D component */
typedef struct PrefabView3D PrefabView3D_t, *PrefabView3DPtr, *lpPrefabView3D_t;
typedef struct PrefabView3D const *PrefabView3DCPtr, *lpcPrefabView3D_t;
struct PrefabView3D {
	fixedString_t SCA; ///< Adds "hmi_plugins/{{SCA}}" to search paths and loads from "prefabs/{{SCA}}"
	fixedString_t Prefab; ///< Path to prefab asset file to instantiate
	int32_t _loadedSCA;
	int32_t _loadedPrefab;
};
ORCA_API void luaX_pushPrefabView3D(lua_State *L, struct PrefabView3D const* PrefabView3D);
ORCA_API struct PrefabView3D* luaX_checkPrefabView3D(lua_State *L, int idx);
/// @brief Base class for render pass components in the rendering pipeline
/** RenderPass component */
typedef struct RenderPass RenderPass_t, *RenderPassPtr, *lpRenderPass_t;
typedef struct RenderPass const *RenderPassCPtr, *lpcRenderPass_t;
struct RenderPass {
};
ORCA_API void luaX_pushRenderPass(lua_State *L, struct RenderPass const* RenderPass);
ORCA_API struct RenderPass* luaX_checkRenderPass(lua_State *L, int idx);
/// @brief Render pass that outputs to the final composition target
/** CompositionTargetRenderPass component */
typedef struct CompositionTargetRenderPass CompositionTargetRenderPass_t, *CompositionTargetRenderPassPtr, *lpCompositionTargetRenderPass_t;
typedef struct CompositionTargetRenderPass const *CompositionTargetRenderPassCPtr, *lpcCompositionTargetRenderPass_t;
struct CompositionTargetRenderPass {
};
ORCA_API void luaX_pushCompositionTargetRenderPass(lua_State *L, struct CompositionTargetRenderPass const* CompositionTargetRenderPass);
ORCA_API struct CompositionTargetRenderPass* luaX_checkCompositionTargetRenderPass(lua_State *L, int idx);
/// @brief Render pass for copying (blitting) between render targets
/** BlitRenderPass component */
typedef struct BlitRenderPass BlitRenderPass_t, *BlitRenderPassPtr, *lpBlitRenderPass_t;
typedef struct BlitRenderPass const *BlitRenderPassCPtr, *lpcBlitRenderPass_t;
struct BlitRenderPass {
};
ORCA_API void luaX_pushBlitRenderPass(lua_State *L, struct BlitRenderPass const* BlitRenderPass);
ORCA_API struct BlitRenderPass* luaX_checkBlitRenderPass(lua_State *L, int idx);
/// @brief Render pass for clearing framebuffer contents
/** ClearRenderPass component */
typedef struct ClearRenderPass ClearRenderPass_t, *ClearRenderPassPtr, *lpClearRenderPass_t;
typedef struct ClearRenderPass const *ClearRenderPassCPtr, *lpcClearRenderPass_t;
struct ClearRenderPass {
	struct color ClearColor; ///< Color value to clear the color buffer (RGBA format)
	float ClearDepth; ///< Depth value to clear the depth buffer (typically 1.0 for far plane)
	int32_t ClearStencil; ///< Integer value to clear the stencil buffer (typically 0)
};
ORCA_API void luaX_pushClearRenderPass(lua_State *L, struct ClearRenderPass const* ClearRenderPass);
ORCA_API struct ClearRenderPass* luaX_checkClearRenderPass(lua_State *L, int idx);
/// @brief Render pass for drawing scene objects with tag-based filtering
/** DrawObjectsRenderPass component */
typedef struct DrawObjectsRenderPass DrawObjectsRenderPass_t, *DrawObjectsRenderPassPtr, *lpDrawObjectsRenderPass_t;
typedef struct DrawObjectsRenderPass const *DrawObjectsRenderPassCPtr, *lpcDrawObjectsRenderPass_t;
struct DrawObjectsRenderPass {
	fixedString_t Camera; ///< Name of camera to use for object rendering and view/projection matrices
	objectTags_t IncludeTags; ///< Object tags that must be present for objects to be rendered
	objectTags_t ExcludeTags; ///< Object tags that prevent objects from being rendered
};
ORCA_API void luaX_pushDrawObjectsRenderPass(lua_State *L, struct DrawObjectsRenderPass const* DrawObjectsRenderPass);
ORCA_API struct DrawObjectsRenderPass* luaX_checkDrawObjectsRenderPass(lua_State *L, int idx);
/// @brief Render pass for configuring graphics pipeline state
/** PipelineStateRenderPass component */
typedef struct PipelineStateRenderPass PipelineStateRenderPass_t, *PipelineStateRenderPassPtr, *lpPipelineStateRenderPass_t;
typedef struct PipelineStateRenderPass const *PipelineStateRenderPassCPtr, *lpcPipelineStateRenderPass_t;
struct PipelineStateRenderPass {
	enum BlendMode BlendMode; ///< Color blending mode for transparency and compositing effects
	enum ColorWriteMode ColorWriteMode; ///< Which color channels are written to the framebuffer
	enum CullMode CullMode; ///< Triangle face culling mode for performance optimization
	enum CompareFunc DepthTestFunction; ///< Comparison function for depth buffer testing
	bool_t DepthWriteEnabled; ///< Whether to write depth values to the depth buffer
	enum ViewportMode ViewportMode; ///< Coordinate system for viewport rectangle specification
	enum ViewportMode ScissorMode; ///< Coordinate system for scissor rectangle specification
	struct vec4 Viewport; ///< Viewport rectangle (x, y, width, height) for rendering area
	struct vec4 Scissor; ///< Scissor rectangle (x, y, width, height) for pixel clipping
	enum CompareFunc StencilTestFunction; ///< Comparison function for stencil buffer testing
	int32_t StencilReferenceValue; ///< Reference value used in stencil test comparisons
	int32_t StencilMask; ///< Bitmask applied to stencil values before testing
	enum StencilOp StencilFailOperation; ///< Operation when stencil test fails
	enum StencilOp StencilPassDepthFailOperation; ///< Operation when stencil test passes but depth test fails
	enum StencilOp StencilPassDepthPassOperation; ///< Operation when both stencil and depth tests pass
	bool_t StencilWriteEnabled; ///< Whether to write computed stencil values to the stencil buffer
};
ORCA_API void luaX_pushPipelineStateRenderPass(lua_State *L, struct PipelineStateRenderPass const* PipelineStateRenderPass);
ORCA_API struct PipelineStateRenderPass* luaX_checkPipelineStateRenderPass(lua_State *L, int idx);
/// @brief 3D text renderer for displaying typography in 3D space
/** TextBlock3D component */
typedef struct TextBlock3D TextBlock3D_t, *TextBlock3DPtr, *lpTextBlock3D_t;
typedef struct TextBlock3D const *TextBlock3DCPtr, *lpcTextBlock3D_t;
struct TextBlock3D {
	struct Node3D* _node3D;
};
ORCA_API void luaX_pushTextBlock3D(lua_State *L, struct TextBlock3D const* TextBlock3D);
ORCA_API struct TextBlock3D* luaX_checkTextBlock3D(lua_State *L, int idx);
/// @brief 3D light source for scene illumination and shadow casting
/** Light3D component */
typedef struct Light3D Light3D_t, *Light3DPtr, *lpLight3D_t;
typedef struct Light3D const *Light3DCPtr, *lpcLight3D_t;
struct Light3D {
	struct color Color; ///< RGB color and intensity of the light emission
	struct vec2 SpotAngle; ///< Inner and outer cone angles for spot lights (X=inner, Y=outer in degrees)
	float Intensity; ///< Brightness multiplier for light output (1.0 = normal intensity)
	float Range; ///< Maximum distance at which light affects surfaces (for performance optimization)
	enum LightType Type; ///< Type of light source (point or spot)
};
ORCA_API void luaX_pushLight3D(lua_State *L, struct Light3D const* Light3D);
ORCA_API struct Light3D* luaX_checkLight3D(lua_State *L, int idx);
/// @brief Billboard sprite renderer for 2D images in 3D space
/** SpriteView component */
typedef struct SpriteView SpriteView_t, *SpriteViewPtr, *lpSpriteView_t;
typedef struct SpriteView const *SpriteViewCPtr, *lpcSpriteView_t;
struct SpriteView {
	struct Texture* Image; ///< 2D image texture to display on the billboard sprite
	struct vec4 Bounds; ///< Texture coordinates and sizing (X=U, Y=V, Z=width, W=height)
};
ORCA_API void luaX_pushSpriteView(lua_State *L, struct SpriteView const* SpriteView);
ORCA_API struct SpriteView* luaX_checkSpriteView(lua_State *L, int idx);

#endif
