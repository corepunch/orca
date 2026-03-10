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


/** StencilOp enum */
typedef enum StencilOp {
	kStencilOpKeep, // Keep the current stencil buffer value unchanged
	kStencilOpZero, // Set the stencil buffer value to zero
	kStencilOpReplace, // Replace stencil buffer value with the reference value
	kStencilOpIncrement, // Increment stencil buffer value, clamping to maximum
	kStencilOpIncrementWrap, // Increment stencil buffer value, wrapping to zero at maximum
	kStencilOpDecrement, // Decrement stencil buffer value, clamping to zero
	kStencilOpDecrementWrap, // Decrement stencil buffer value, wrapping to maximum at zero
	kStencilOpInvert, // Bitwise invert the stencil buffer value
} eStencilOp_t;
#define StencilOp_Count 8
ORCA_API const char *StencilOpToString(enum StencilOp value);
ORCA_API enum StencilOp luaX_checkStencilOp(lua_State *L, int idx);
ORCA_API void luaX_pushStencilOp(lua_State *L, enum StencilOp value);
/** ViewportMode enum */
typedef enum ViewportMode {
	kViewportModeRelative, // Coordinates specified as normalized values (0.0 to 1.0) relative to render target size
	kViewportModeAbsolute, // Coordinates specified in absolute pixels
} eViewportMode_t;
#define ViewportMode_Count 2
ORCA_API const char *ViewportModeToString(enum ViewportMode value);
ORCA_API enum ViewportMode luaX_checkViewportMode(lua_State *L, int idx);
ORCA_API void luaX_pushViewportMode(lua_State *L, enum ViewportMode value);
/** CullMode enum */
typedef enum CullMode {
	kCullModeNone, // No face culling - render both front and back faces
	kCullModeBack, // Cull back-facing triangles (standard for solid objects)
	kCullModeFront, // Cull front-facing triangles (useful for interior views or special effects)
} eCullMode_t;
#define CullMode_Count 3
ORCA_API const char *CullModeToString(enum CullMode value);
ORCA_API enum CullMode luaX_checkCullMode(lua_State *L, int idx);
ORCA_API void luaX_pushCullMode(lua_State *L, enum CullMode value);
/** ColorWriteMode enum */
typedef enum ColorWriteMode {
	kColorWriteModeNone, // Disable all color channel writes (useful for depth-only or stencil-only passes)
	kColorWriteModeRGB, // Write to red, green, and blue channels only
	kColorWriteModeRGBA, // Write to all four color channels including alpha
	kColorWriteModeR, // Write to red channel only
	kColorWriteModeG, // Write to green channel only
	kColorWriteModeB, // Write to blue channel only
	kColorWriteModeGB, // Write to green and blue channels only
	kColorWriteModeA, // Write to alpha channel only
} eColorWriteMode_t;
#define ColorWriteMode_Count 8
ORCA_API const char *ColorWriteModeToString(enum ColorWriteMode value);
ORCA_API enum ColorWriteMode luaX_checkColorWriteMode(lua_State *L, int idx);
ORCA_API void luaX_pushColorWriteMode(lua_State *L, enum ColorWriteMode value);
/** FovType enum */
typedef enum FovType {
	kFovTypeXfov, // Field of view specified as horizontal angle in degrees
	kFovTypeYfov, // Field of view specified as vertical angle in degrees (most common)
} eFovType_t;
#define FovType_Count 2
ORCA_API const char *FovTypeToString(enum FovType value);
ORCA_API enum FovType luaX_checkFovType(lua_State *L, int idx);
ORCA_API void luaX_pushFovType(lua_State *L, enum FovType value);
/** ProjectionType enum */
typedef enum ProjectionType {
	kProjectionTypePerspective, // Perspective projection with realistic depth foreshortening
	kProjectionTypeOrthographic, // Orthographic projection with parallel lines (no perspective distortion)
} eProjectionType_t;
#define ProjectionType_Count 2
ORCA_API const char *ProjectionTypeToString(enum ProjectionType value);
ORCA_API enum ProjectionType luaX_checkProjectionType(lua_State *L, int idx);
ORCA_API void luaX_pushProjectionType(lua_State *L, enum ProjectionType value);
/** LightType enum */
typedef enum LightType {
	kLightTypePoint, // Omnidirectional point light source (like a light bulb)
	kLightTypeSpot, // Directional spot light with cone-shaped illumination area
} eLightType_t;
#define LightType_Count 2
ORCA_API const char *LightTypeToString(enum LightType value);
ORCA_API enum LightType luaX_checkLightType(lua_State *L, int idx);
ORCA_API void luaX_pushLightType(lua_State *L, enum LightType value);





/** Node3D component */
typedef struct Node3D Node3D_t, *Node3DPtr, *lpNode3D_t;
typedef struct Node3D const *Node3DCPtr, *lpcNode3D_t;
struct Node3D {
	struct transform3 LayoutTransform;
	struct transform3 RenderTransform;
	struct vec3 RenderTransformOrigin;
	struct vec3 ContentOffset;
	struct mat4 Matrix;
	float CalculatedOffset;
	float _opacity;
};
ORCA_API void luaX_pushNode3D(lua_State *L, struct Node3D const* Node3D);
ORCA_API struct Node3D* luaX_checkNode3D(lua_State *L, int idx);
/** Scene component */
typedef struct Scene Scene_t, *ScenePtr, *lpScene_t;
typedef struct Scene const *SceneCPtr, *lpcScene_t;
struct Scene {
	fixedString_t Camera;
	fixedString_t PreviewCamera;
	fixedString_t HitTestCamera;
	struct RenderPass* RenderPass;
	fixedString_t BackgroundTimelineSequence;
	struct RenderPass* ComposerOverride;
	struct Texture* EnvironmentTexture;
};
ORCA_API void luaX_pushScene(lua_State *L, struct Scene const* Scene);
ORCA_API struct Scene* luaX_checkScene(lua_State *L, int idx);
/** Model3D component */
typedef struct Model3D Model3D_t, *Model3DPtr, *lpModel3D_t;
typedef struct Model3D const *Model3DCPtr, *lpcModel3D_t;
struct Model3D {
	struct Mesh* Mesh;
	struct Material* Material;
};
ORCA_API void luaX_pushModel3D(lua_State *L, struct Model3D const* Model3D);
ORCA_API struct Model3D* luaX_checkModel3D(lua_State *L, int idx);
/** PlaneMeshNode component */
typedef struct PlaneMeshNode PlaneMeshNode_t, *PlaneMeshNodePtr, *lpPlaneMeshNode_t;
typedef struct PlaneMeshNode const *PlaneMeshNodeCPtr, *lpcPlaneMeshNode_t;
struct PlaneMeshNode {
	float PlaneWidth;
	float PlaneHeight;
	bool_t PlaneInvertU;
	bool_t PlaneInvertV;
	bool_t GenerateTangents;
	bool_t GenerateNormals;
	struct Material* MeshMaterial;
};
ORCA_API void luaX_pushPlaneMeshNode(lua_State *L, struct PlaneMeshNode const* PlaneMeshNode);
ORCA_API struct PlaneMeshNode* luaX_checkPlaneMeshNode(lua_State *L, int idx);
/** Camera component */
typedef struct Camera Camera_t, *CameraPtr, *lpCamera_t;
typedef struct Camera const *CameraCPtr, *lpcCamera_t;
struct Camera {
	float Fov;
	enum FovType FovType;
	enum ProjectionType ProjectionType;
	float ZNear;
	float ZFar;
	float EyeSeparation;
	float FocalDistance;
	float OrthogonalPlaneHeight;
	bool_t ZPositive;
};
ORCA_API void luaX_pushCamera(lua_State *L, struct Camera const* Camera);
ORCA_API struct Camera* luaX_checkCamera(lua_State *L, int idx);
/** TrajectoryList3D component */
typedef struct TrajectoryList3D TrajectoryList3D_t, *TrajectoryList3DPtr, *lpTrajectoryList3D_t;
typedef struct TrajectoryList3D const *TrajectoryList3DCPtr, *lpcTrajectoryList3D_t;
struct TrajectoryList3D {
	struct Trajectory* Trajectory;
	struct vec2 ScrollAxis;
	bool_t Looping;
	float Spacing;
	float SelectionBehavior;
	float SelectedItemIndex;
};
ORCA_API void luaX_pushTrajectoryList3D(lua_State *L, struct TrajectoryList3D const* TrajectoryList3D);
ORCA_API struct TrajectoryList3D* luaX_checkTrajectoryList3D(lua_State *L, int idx);
/** Viewport3D component */
typedef struct Viewport3D Viewport3D_t, *Viewport3DPtr, *lpViewport3D_t;
typedef struct Viewport3D const *Viewport3DCPtr, *lpcViewport3D_t;
struct Viewport3D {
	fixedString_t Camera;
	fixedString_t PreviewCamera;
	fixedString_t HitTestCamera;
	struct RenderPass* RenderPass;
	fixedString_t Scene;
};
ORCA_API void luaX_pushViewport3D(lua_State *L, struct Viewport3D const* Viewport3D);
ORCA_API struct Viewport3D* luaX_checkViewport3D(lua_State *L, int idx);
/** PrefabView3D component */
typedef struct PrefabView3D PrefabView3D_t, *PrefabView3DPtr, *lpPrefabView3D_t;
typedef struct PrefabView3D const *PrefabView3DCPtr, *lpcPrefabView3D_t;
struct PrefabView3D {
	fixedString_t SCA;
	fixedString_t Prefab;
	int32_t _loadedSCA;
	int32_t _loadedPrefab;
};
ORCA_API void luaX_pushPrefabView3D(lua_State *L, struct PrefabView3D const* PrefabView3D);
ORCA_API struct PrefabView3D* luaX_checkPrefabView3D(lua_State *L, int idx);
/** RenderPass component */
typedef struct RenderPass RenderPass_t, *RenderPassPtr, *lpRenderPass_t;
typedef struct RenderPass const *RenderPassCPtr, *lpcRenderPass_t;
struct RenderPass {
};
ORCA_API void luaX_pushRenderPass(lua_State *L, struct RenderPass const* RenderPass);
ORCA_API struct RenderPass* luaX_checkRenderPass(lua_State *L, int idx);
/** CompositionTargetRenderPass component */
typedef struct CompositionTargetRenderPass CompositionTargetRenderPass_t, *CompositionTargetRenderPassPtr, *lpCompositionTargetRenderPass_t;
typedef struct CompositionTargetRenderPass const *CompositionTargetRenderPassCPtr, *lpcCompositionTargetRenderPass_t;
struct CompositionTargetRenderPass {
};
ORCA_API void luaX_pushCompositionTargetRenderPass(lua_State *L, struct CompositionTargetRenderPass const* CompositionTargetRenderPass);
ORCA_API struct CompositionTargetRenderPass* luaX_checkCompositionTargetRenderPass(lua_State *L, int idx);
/** BlitRenderPass component */
typedef struct BlitRenderPass BlitRenderPass_t, *BlitRenderPassPtr, *lpBlitRenderPass_t;
typedef struct BlitRenderPass const *BlitRenderPassCPtr, *lpcBlitRenderPass_t;
struct BlitRenderPass {
};
ORCA_API void luaX_pushBlitRenderPass(lua_State *L, struct BlitRenderPass const* BlitRenderPass);
ORCA_API struct BlitRenderPass* luaX_checkBlitRenderPass(lua_State *L, int idx);
/** ClearRenderPass component */
typedef struct ClearRenderPass ClearRenderPass_t, *ClearRenderPassPtr, *lpClearRenderPass_t;
typedef struct ClearRenderPass const *ClearRenderPassCPtr, *lpcClearRenderPass_t;
struct ClearRenderPass {
	struct color ClearColor;
	float ClearDepth;
	int32_t ClearStencil;
};
ORCA_API void luaX_pushClearRenderPass(lua_State *L, struct ClearRenderPass const* ClearRenderPass);
ORCA_API struct ClearRenderPass* luaX_checkClearRenderPass(lua_State *L, int idx);
/** DrawObjectsRenderPass component */
typedef struct DrawObjectsRenderPass DrawObjectsRenderPass_t, *DrawObjectsRenderPassPtr, *lpDrawObjectsRenderPass_t;
typedef struct DrawObjectsRenderPass const *DrawObjectsRenderPassCPtr, *lpcDrawObjectsRenderPass_t;
struct DrawObjectsRenderPass {
	fixedString_t Camera;
	objectTags_t IncludeTags;
	objectTags_t ExcludeTags;
};
ORCA_API void luaX_pushDrawObjectsRenderPass(lua_State *L, struct DrawObjectsRenderPass const* DrawObjectsRenderPass);
ORCA_API struct DrawObjectsRenderPass* luaX_checkDrawObjectsRenderPass(lua_State *L, int idx);
/** PipelineStateRenderPass component */
typedef struct PipelineStateRenderPass PipelineStateRenderPass_t, *PipelineStateRenderPassPtr, *lpPipelineStateRenderPass_t;
typedef struct PipelineStateRenderPass const *PipelineStateRenderPassCPtr, *lpcPipelineStateRenderPass_t;
struct PipelineStateRenderPass {
	enum BlendMode BlendMode;
	enum ColorWriteMode ColorWriteMode;
	enum CullMode CullMode;
	enum CompareFunc DepthTestFunction;
	bool_t DepthWriteEnabled;
	enum ViewportMode ViewportMode;
	enum ViewportMode ScissorMode;
	struct vec4 Viewport;
	struct vec4 Scissor;
	enum CompareFunc StencilTestFunction;
	int32_t StencilReferenceValue;
	int32_t StencilMask;
	enum StencilOp StencilFailOperation;
	enum StencilOp StencilPassDepthFailOperation;
	enum StencilOp StencilPassDepthPassOperation;
	bool_t StencilWriteEnabled;
};
ORCA_API void luaX_pushPipelineStateRenderPass(lua_State *L, struct PipelineStateRenderPass const* PipelineStateRenderPass);
ORCA_API struct PipelineStateRenderPass* luaX_checkPipelineStateRenderPass(lua_State *L, int idx);
/** TextBlock3D component */
typedef struct TextBlock3D TextBlock3D_t, *TextBlock3DPtr, *lpTextBlock3D_t;
typedef struct TextBlock3D const *TextBlock3DCPtr, *lpcTextBlock3D_t;
struct TextBlock3D {
	struct Node3D* _node3D;
};
ORCA_API void luaX_pushTextBlock3D(lua_State *L, struct TextBlock3D const* TextBlock3D);
ORCA_API struct TextBlock3D* luaX_checkTextBlock3D(lua_State *L, int idx);
/** Light3D component */
typedef struct Light3D Light3D_t, *Light3DPtr, *lpLight3D_t;
typedef struct Light3D const *Light3DCPtr, *lpcLight3D_t;
struct Light3D {
	struct color Color;
	struct vec2 SpotAngle;
	float Intensity;
	float Range;
	enum LightType Type;
};
ORCA_API void luaX_pushLight3D(lua_State *L, struct Light3D const* Light3D);
ORCA_API struct Light3D* luaX_checkLight3D(lua_State *L, int idx);
/** SpriteView component */
typedef struct SpriteView SpriteView_t, *SpriteViewPtr, *lpSpriteView_t;
typedef struct SpriteView const *SpriteViewCPtr, *lpcSpriteView_t;
struct SpriteView {
	struct Texture* Image;
	struct vec4 Bounds;
};
ORCA_API void luaX_pushSpriteView(lua_State *L, struct SpriteView const* SpriteView);
ORCA_API struct SpriteView* luaX_checkSpriteView(lua_State *L, int idx);

#endif