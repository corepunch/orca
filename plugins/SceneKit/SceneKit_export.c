// Auto-generated from SceneKit.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>
#include <include/codegen.h>

#include "SceneKit.h"


extern const char *_BlendMode[];
extern const char *_CompareFunc[];
ENUM(StencilOp, "Keep", "Zero", "Replace", "Increment", "IncrementWrap", "Decrement", "DecrementWrap", "Invert")
ENUM(ViewportMode, "Relative", "Absolute")
ENUM(CullMode, "None", "Back", "Front")
ENUM(ColorWriteMode, "None", "RGB", "RGBA", "R", "G", "B", "GB", "A")
ENUM(FovType, "Xfov", "Yfov")
ENUM(ProjectionType, "Perspective", "Orthographic")
ENUM(LightType, "Point", "Spot")
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);


static luaL_Reg _Node3D_RenderEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node3D_RenderEventArgs[] = {
	DECL_STRUCT_TYPE(0xce9ab61f, Node3D_RenderEventArgs, ViewDef, ViewDef, ViewDef), // Node3D_RenderEventArgs.ViewDef
};

STRUCT(Node3D_RenderEventArgs, Node3D_RenderEventArgs);
HANDLER(Node3D, Node, UpdateMatrix);
COMPONENT_PROPERTIES(Node3D) = {
	DECL_STRUCT_TYPE(0x3f19bf01, Node3D, LayoutTransform, LayoutTransform, Transform3D), // Node3D.LayoutTransform
	DECL_STRUCT_TYPE(0xfc7e27e0, Node3D, LayoutTransformTranslation, LayoutTransform.translation, Vector3D), // Node3D.LayoutTransformTranslation
	DECL_STRUCT_TYPE(0x9560ef43, Node3D, LayoutTransformRotation, LayoutTransform.rotation, Vector3D), // Node3D.LayoutTransformRotation
	DECL_STRUCT_TYPE(0x5a2c3595, Node3D, LayoutTransformScale, LayoutTransform.scale, Vector3D), // Node3D.LayoutTransformScale
	DECL_STRUCT_TYPE(0xe9e55063, Node3D, RenderTransform, RenderTransform, Transform3D), // Node3D.RenderTransform
	DECL_STRUCT_TYPE(0xb8e70ec2, Node3D, RenderTransformTranslation, RenderTransform.translation, Vector3D), // Node3D.RenderTransformTranslation
	DECL_STRUCT_TYPE(0x3c611efd, Node3D, RenderTransformRotation, RenderTransform.rotation, Vector3D), // Node3D.RenderTransformRotation
	DECL_STRUCT_TYPE(0xd037e21b, Node3D, RenderTransformScale, RenderTransform.scale, Vector3D), // Node3D.RenderTransformScale
	DECL_STRUCT_TYPE(0xdc65ec6d, Node3D, RenderTransformOrigin, RenderTransformOrigin, Vector3D), // Node3D.RenderTransformOrigin
	DECL_STRUCT_TYPE(0x35a57c45, Node3D, ContentOffset, ContentOffset, Vector3D), // Node3D.ContentOffset
	DECL_STRUCT_TYPE(0xe9d1810c, Node3D, Matrix, Matrix, Matrix3D), // Node3D.Matrix
	DECL_FLOAT(0xd5710b2e, Node3D, CalculatedOffset, CalculatedOffset), // Node3D.CalculatedOffset
	DECL_EVENT(0x350cf42d, Node3D, Render), // Node3D.Render
};
COMPONENT_DEFAULTS(Node3D) = {
		
  .LayoutTransform = {.scale={1,1,1}},
		
  .RenderTransform = {.scale={1,1,1}},
};
COMPONENT_PROC_BEGIN(Node3D)
		DISPATCH(Node3D, Node, UpdateMatrix) // Node.UpdateMatrix
COMPONENT_PROC_END()
void luaX_pushNode3D(lua_State *L, struct Node3D const* Node3D) {
	luaX_pushObject(L, CMP_GetObject(Node3D));
}
struct Node3D* luaX_checkNode3D(lua_State *L, int idx) {
	return GetNode3D(luaX_checkObject(L, idx));
}
#define ID_Node 0x3468032d
REGISTER_CLASS(Node3D, ID_Node, 0);
HANDLER(Scene, Node, UpdateMatrix);
COMPONENT_PROPERTIES(Scene) = {
	DECL_STRING(0xe74c7b6e, Scene, Camera, Camera), // Scene.Camera
	DECL_STRING(0x14a89218, Scene, PreviewCamera, PreviewCamera), // Scene.PreviewCamera
	DECL_STRING(0xf497b459, Scene, HitTestCamera, HitTestCamera), // Scene.HitTestCamera
	DECL_OBJECT(0xf64bbf80, Scene, RenderPass, RenderPass, RenderPass), // Scene.RenderPass
	DECL_STRING(0x1fe3e9af, Scene, BackgroundTimelineSequence, BackgroundTimelineSequence), // Scene.BackgroundTimelineSequence
	DECL_OBJECT(0xedb9befb, Scene, ComposerOverride, ComposerOverride, RenderPass), // Scene.ComposerOverride
	DECL_OBJECT(0x886f1cad, Scene, EnvironmentTexture, EnvironmentTexture, Texture), // Scene.EnvironmentTexture
};
COMPONENT_DEFAULTS(Scene) = {
};
COMPONENT_PROC_BEGIN(Scene)
		DISPATCH(Scene, Node, UpdateMatrix) // Node.UpdateMatrix
COMPONENT_PROC_END()
void luaX_pushScene(lua_State *L, struct Scene const* Scene) {
	luaX_pushObject(L, CMP_GetObject(Scene));
}
struct Scene* luaX_checkScene(lua_State *L, int idx) {
	return GetScene(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(Scene, ID_Node3D, 0);
HANDLER(Model3D, Node3D, Render);
COMPONENT_PROPERTIES(Model3D) = {
	DECL_OBJECT(0x07e055dc, Model3D, Mesh, Mesh, Mesh), // Model3D.Mesh
	DECL_OBJECT(0xcbd54f80, Model3D, Material, Material, Material), // Model3D.Material
};
COMPONENT_DEFAULTS(Model3D) = {
};
COMPONENT_PROC_BEGIN(Model3D)
		DISPATCH(Model3D, Node3D, Render) // Node3D.Render
COMPONENT_PROC_END()
void luaX_pushModel3D(lua_State *L, struct Model3D const* Model3D) {
	luaX_pushObject(L, CMP_GetObject(Model3D));
}
struct Model3D* luaX_checkModel3D(lua_State *L, int idx) {
	return GetModel3D(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(Model3D, ID_Node3D, 0);
HANDLER(PlaneMeshNode, Node3D, Render);
COMPONENT_PROPERTIES(PlaneMeshNode) = {
	DECL_FLOAT(0x8f8d39cf, PlaneMeshNode, PlaneWidth, PlaneWidth), // PlaneMeshNode.PlaneWidth
	DECL_FLOAT(0x2d44a1f2, PlaneMeshNode, PlaneHeight, PlaneHeight), // PlaneMeshNode.PlaneHeight
	DECL_BOOL(0x0a07d70c, PlaneMeshNode, PlaneInvertU, PlaneInvertU), // PlaneMeshNode.PlaneInvertU
	DECL_BOOL(0x0d07dbc5, PlaneMeshNode, PlaneInvertV, PlaneInvertV), // PlaneMeshNode.PlaneInvertV
	DECL_BOOL(0x57852420, PlaneMeshNode, GenerateTangents, GenerateTangents), // PlaneMeshNode.GenerateTangents
	DECL_BOOL(0x909a95b6, PlaneMeshNode, GenerateNormals, GenerateNormals), // PlaneMeshNode.GenerateNormals
	DECL_OBJECT(0xe2d8fb55, PlaneMeshNode, MeshMaterial, MeshMaterial, Material), // PlaneMeshNode.MeshMaterial
};
COMPONENT_DEFAULTS(PlaneMeshNode) = {
};
COMPONENT_PROC_BEGIN(PlaneMeshNode)
		DISPATCH(PlaneMeshNode, Node3D, Render) // Node3D.Render
COMPONENT_PROC_END()
void luaX_pushPlaneMeshNode(lua_State *L, struct PlaneMeshNode const* PlaneMeshNode) {
	luaX_pushObject(L, CMP_GetObject(PlaneMeshNode));
}
struct PlaneMeshNode* luaX_checkPlaneMeshNode(lua_State *L, int idx) {
	return GetPlaneMeshNode(luaX_checkObject(L, idx));
}
#define ID_Model3D 0xc56de5fd
REGISTER_CLASS(PlaneMeshNode, ID_Model3D, 0);
COMPONENT_PROPERTIES(Camera) = {
	DECL_FLOAT(0x137e217c, Camera, Fov, Fov), // Camera.Fov
	DECL_ENUM(0x0ef1c6f4, Camera, FovType, FovType, FovType), // Camera.FovType
	DECL_ENUM(0x87320ce8, Camera, ProjectionType, ProjectionType, ProjectionType), // Camera.ProjectionType
	DECL_FLOAT(0x15af1da7, Camera, ZNear, ZNear), // Camera.ZNear
	DECL_FLOAT(0x993918c6, Camera, ZFar, ZFar), // Camera.ZFar
	DECL_FLOAT(0xd6d304c6, Camera, EyeSeparation, EyeSeparation), // Camera.EyeSeparation
	DECL_FLOAT(0x7e322435, Camera, FocalDistance, FocalDistance), // Camera.FocalDistance
	DECL_FLOAT(0xfa573077, Camera, OrthogonalPlaneHeight, OrthogonalPlaneHeight), // Camera.OrthogonalPlaneHeight
	DECL_BOOL(0x026159d2, Camera, ZPositive, ZPositive), // Camera.ZPositive
};
COMPONENT_DEFAULTS(Camera) = {
};
COMPONENT_PROC_BEGIN(Camera)
COMPONENT_PROC_END()
void luaX_pushCamera(lua_State *L, struct Camera const* Camera) {
	luaX_pushObject(L, CMP_GetObject(Camera));
}
struct Camera* luaX_checkCamera(lua_State *L, int idx) {
	return GetCamera(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(Camera, ID_Node3D, 0);
HANDLER(TrajectoryList3D, Node, UpdateMatrix);
COMPONENT_PROPERTIES(TrajectoryList3D) = {
	DECL_OBJECT(0x4cf7cbf8, TrajectoryList3D, Trajectory, Trajectory, Trajectory), // TrajectoryList3D.Trajectory
	DECL_STRUCT_TYPE(0xeea06ebd, TrajectoryList3D, ScrollAxis, ScrollAxis, Vector2D), // TrajectoryList3D.ScrollAxis
	DECL_BOOL(0x343782cd, TrajectoryList3D, Looping, Looping), // TrajectoryList3D.Looping
	DECL_FLOAT(0x8777939e, TrajectoryList3D, Spacing, Spacing), // TrajectoryList3D.Spacing
	DECL_FLOAT(0xc742f477, TrajectoryList3D, SelectionBehavior, SelectionBehavior), // TrajectoryList3D.SelectionBehavior
	DECL_FLOAT(0x8af08301, TrajectoryList3D, SelectedItemIndex, SelectedItemIndex), // TrajectoryList3D.SelectedItemIndex
};
COMPONENT_DEFAULTS(TrajectoryList3D) = {
};
COMPONENT_PROC_BEGIN(TrajectoryList3D)
		DISPATCH(TrajectoryList3D, Node, UpdateMatrix) // Node.UpdateMatrix
COMPONENT_PROC_END()
void luaX_pushTrajectoryList3D(lua_State *L, struct TrajectoryList3D const* TrajectoryList3D) {
	luaX_pushObject(L, CMP_GetObject(TrajectoryList3D));
}
struct TrajectoryList3D* luaX_checkTrajectoryList3D(lua_State *L, int idx) {
	return GetTrajectoryList3D(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(TrajectoryList3D, ID_Node3D, 0);
HANDLER(Viewport3D, Node2D, ForegroundContent);
COMPONENT_PROPERTIES(Viewport3D) = {
	DECL_STRING(0xe74c7b6e, Viewport3D, Camera, Camera), // Viewport3D.Camera
	DECL_STRING(0x14a89218, Viewport3D, PreviewCamera, PreviewCamera), // Viewport3D.PreviewCamera
	DECL_STRING(0xf497b459, Viewport3D, HitTestCamera, HitTestCamera), // Viewport3D.HitTestCamera
	DECL_OBJECT(0xf64bbf80, Viewport3D, RenderPass, RenderPass, RenderPass), // Viewport3D.RenderPass
	DECL_STRING(0xc89b38b3, Viewport3D, Scene, Scene), // Viewport3D.Scene
};
COMPONENT_DEFAULTS(Viewport3D) = {
};
COMPONENT_PROC_BEGIN(Viewport3D)
		DISPATCH(Viewport3D, Node2D, ForegroundContent) // Node2D.ForegroundContent
COMPONENT_PROC_END()
void luaX_pushViewport3D(lua_State *L, struct Viewport3D const* Viewport3D) {
	luaX_pushObject(L, CMP_GetObject(Viewport3D));
}
struct Viewport3D* luaX_checkViewport3D(lua_State *L, int idx) {
	return GetViewport3D(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(Viewport3D, ID_Node2D, 0);
HANDLER(PrefabView3D, Object, Start);
HANDLER(PrefabView3D, Object, PropertyChanged);
HANDLER(PrefabView3D, Node, LoadView);
COMPONENT_PROPERTIES(PrefabView3D) = {
	DECL_STRING(0xd6415ba3, PrefabView3D, Prefab, Prefab), // PrefabView3D.Prefab
};
COMPONENT_DEFAULTS(PrefabView3D) = {
};
COMPONENT_PROC_BEGIN(PrefabView3D)
		DISPATCH(PrefabView3D, Object, Start) // Object.Start
		DISPATCH(PrefabView3D, Object, PropertyChanged) // Object.PropertyChanged
		DISPATCH(PrefabView3D, Node, LoadView) // Node.LoadView
COMPONENT_PROC_END()
void luaX_pushPrefabView3D(lua_State *L, struct PrefabView3D const* PrefabView3D) {
	luaX_pushObject(L, CMP_GetObject(PrefabView3D));
}
struct PrefabView3D* luaX_checkPrefabView3D(lua_State *L, int idx) {
	return GetPrefabView3D(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(PrefabView3D, ID_Node3D, 0);
COMPONENT_PROPERTIES(RenderPass) = {
};
COMPONENT_DEFAULTS(RenderPass) = {
};
COMPONENT_PROC_BEGIN(RenderPass)
COMPONENT_PROC_END()
void luaX_pushRenderPass(lua_State *L, struct RenderPass const* RenderPass) {
	luaX_pushObject(L, CMP_GetObject(RenderPass));
}
struct RenderPass* luaX_checkRenderPass(lua_State *L, int idx) {
	return GetRenderPass(luaX_checkObject(L, idx));
}
#define ID_Node 0x3468032d
REGISTER_CLASS(RenderPass, ID_Node, 0);
COMPONENT_PROPERTIES(CompositionTargetRenderPass) = {
};
COMPONENT_DEFAULTS(CompositionTargetRenderPass) = {
};
COMPONENT_PROC_BEGIN(CompositionTargetRenderPass)
COMPONENT_PROC_END()
void luaX_pushCompositionTargetRenderPass(lua_State *L, struct CompositionTargetRenderPass const* CompositionTargetRenderPass) {
	luaX_pushObject(L, CMP_GetObject(CompositionTargetRenderPass));
}
struct CompositionTargetRenderPass* luaX_checkCompositionTargetRenderPass(lua_State *L, int idx) {
	return GetCompositionTargetRenderPass(luaX_checkObject(L, idx));
}
#define ID_RenderPass 0xf64bbf80
REGISTER_CLASS(CompositionTargetRenderPass, ID_RenderPass, 0);
COMPONENT_PROPERTIES(BlitRenderPass) = {
};
COMPONENT_DEFAULTS(BlitRenderPass) = {
};
COMPONENT_PROC_BEGIN(BlitRenderPass)
COMPONENT_PROC_END()
void luaX_pushBlitRenderPass(lua_State *L, struct BlitRenderPass const* BlitRenderPass) {
	luaX_pushObject(L, CMP_GetObject(BlitRenderPass));
}
struct BlitRenderPass* luaX_checkBlitRenderPass(lua_State *L, int idx) {
	return GetBlitRenderPass(luaX_checkObject(L, idx));
}
#define ID_RenderPass 0xf64bbf80
REGISTER_CLASS(BlitRenderPass, ID_RenderPass, 0);
COMPONENT_PROPERTIES(ClearRenderPass) = {
	DECL_COLOR(0xeb16b675, ClearRenderPass, ClearColor, ClearColor), // ClearRenderPass.ClearColor
	DECL_FLOAT(0xa444e35b, ClearRenderPass, ClearDepth, ClearDepth), // ClearRenderPass.ClearDepth
	DECL_INT(0x222bca4e, ClearRenderPass, ClearStencil, ClearStencil), // ClearRenderPass.ClearStencil
};
COMPONENT_DEFAULTS(ClearRenderPass) = {
};
COMPONENT_PROC_BEGIN(ClearRenderPass)
COMPONENT_PROC_END()
void luaX_pushClearRenderPass(lua_State *L, struct ClearRenderPass const* ClearRenderPass) {
	luaX_pushObject(L, CMP_GetObject(ClearRenderPass));
}
struct ClearRenderPass* luaX_checkClearRenderPass(lua_State *L, int idx) {
	return GetClearRenderPass(luaX_checkObject(L, idx));
}
#define ID_RenderPass 0xf64bbf80
REGISTER_CLASS(ClearRenderPass, ID_RenderPass, 0);
COMPONENT_PROPERTIES(DrawObjectsRenderPass) = {
	DECL_STRING(0xe74c7b6e, DrawObjectsRenderPass, Camera, Camera), // DrawObjectsRenderPass.Camera
	DECL_STRING(0x785c377a, DrawObjectsRenderPass, IncludeTags, IncludeTags), // DrawObjectsRenderPass.IncludeTags
	DECL_STRING(0x2de02b74, DrawObjectsRenderPass, ExcludeTags, ExcludeTags), // DrawObjectsRenderPass.ExcludeTags
};
COMPONENT_DEFAULTS(DrawObjectsRenderPass) = {
};
COMPONENT_PROC_BEGIN(DrawObjectsRenderPass)
COMPONENT_PROC_END()
void luaX_pushDrawObjectsRenderPass(lua_State *L, struct DrawObjectsRenderPass const* DrawObjectsRenderPass) {
	luaX_pushObject(L, CMP_GetObject(DrawObjectsRenderPass));
}
struct DrawObjectsRenderPass* luaX_checkDrawObjectsRenderPass(lua_State *L, int idx) {
	return GetDrawObjectsRenderPass(luaX_checkObject(L, idx));
}
#define ID_RenderPass 0xf64bbf80
REGISTER_CLASS(DrawObjectsRenderPass, ID_RenderPass, 0);
COMPONENT_PROPERTIES(PipelineStateRenderPass) = {
	DECL_ENUM(0x0038792b, PipelineStateRenderPass, BlendMode, BlendMode, BlendMode), // PipelineStateRenderPass.BlendMode
	DECL_ENUM(0x9d0d3c20, PipelineStateRenderPass, ColorWriteMode, ColorWriteMode, ColorWriteMode), // PipelineStateRenderPass.ColorWriteMode
	DECL_ENUM(0xfe9cfa12, PipelineStateRenderPass, CullMode, CullMode, CullMode), // PipelineStateRenderPass.CullMode
	DECL_ENUM(0xb7e582be, PipelineStateRenderPass, DepthTestFunction, DepthTestFunction, CompareFunc), // PipelineStateRenderPass.DepthTestFunction
	DECL_BOOL(0x8ec3072e, PipelineStateRenderPass, DepthWriteEnabled, DepthWriteEnabled), // PipelineStateRenderPass.DepthWriteEnabled
	DECL_ENUM(0x2dd5f6cc, PipelineStateRenderPass, ViewportMode, ViewportMode, ViewportMode), // PipelineStateRenderPass.ViewportMode
	DECL_ENUM(0x8c6862de, PipelineStateRenderPass, ScissorMode, ScissorMode, ViewportMode), // PipelineStateRenderPass.ScissorMode
	DECL_STRUCT_TYPE(0x60b5afe3, PipelineStateRenderPass, Viewport, Viewport, Vector4D), // PipelineStateRenderPass.Viewport
	DECL_STRUCT_TYPE(0x24b3e321, PipelineStateRenderPass, Scissor, Scissor, Vector4D), // PipelineStateRenderPass.Scissor
	DECL_ENUM(0xc770fa47, PipelineStateRenderPass, StencilTestFunction, StencilTestFunction, CompareFunc), // PipelineStateRenderPass.StencilTestFunction
	DECL_INT(0x2ddc4e49, PipelineStateRenderPass, StencilReferenceValue, StencilReferenceValue), // PipelineStateRenderPass.StencilReferenceValue
	DECL_INT(0x82336fe7, PipelineStateRenderPass, StencilMask, StencilMask), // PipelineStateRenderPass.StencilMask
	DECL_ENUM(0x8785338c, PipelineStateRenderPass, StencilFailOperation, StencilFailOperation, StencilOp), // PipelineStateRenderPass.StencilFailOperation
	DECL_ENUM(0xfc4e75c8, PipelineStateRenderPass, StencilPassDepthFailOperation, StencilPassDepthFailOperation, StencilOp), // PipelineStateRenderPass.StencilPassDepthFailOperation
	DECL_ENUM(0x6452d1b7, PipelineStateRenderPass, StencilPassDepthPassOperation, StencilPassDepthPassOperation, StencilOp), // PipelineStateRenderPass.StencilPassDepthPassOperation
	DECL_BOOL(0xb0b1cdf3, PipelineStateRenderPass, StencilWriteEnabled, StencilWriteEnabled), // PipelineStateRenderPass.StencilWriteEnabled
};
COMPONENT_DEFAULTS(PipelineStateRenderPass) = {
		
  .ViewportMode = kViewportModeRelative,
		
  .ScissorMode = kViewportModeRelative,
};
COMPONENT_PROC_BEGIN(PipelineStateRenderPass)
COMPONENT_PROC_END()
void luaX_pushPipelineStateRenderPass(lua_State *L, struct PipelineStateRenderPass const* PipelineStateRenderPass) {
	luaX_pushObject(L, CMP_GetObject(PipelineStateRenderPass));
}
struct PipelineStateRenderPass* luaX_checkPipelineStateRenderPass(lua_State *L, int idx) {
	return GetPipelineStateRenderPass(luaX_checkObject(L, idx));
}
#define ID_RenderPass 0xf64bbf80
REGISTER_CLASS(PipelineStateRenderPass, ID_RenderPass, 0);
HANDLER(TextBlock3D, Node3D, Render);
HANDLER(TextBlock3D, Object, Create);
COMPONENT_PROPERTIES(TextBlock3D) = {
};
COMPONENT_DEFAULTS(TextBlock3D) = {
};
COMPONENT_PROC_BEGIN(TextBlock3D)
		DISPATCH(TextBlock3D, Node3D, Render) // Node3D.Render
		DISPATCH(TextBlock3D, Object, Create) // Object.Create
COMPONENT_PROC_END()
void luaX_pushTextBlock3D(lua_State *L, struct TextBlock3D const* TextBlock3D) {
	luaX_pushObject(L, CMP_GetObject(TextBlock3D));
}
struct TextBlock3D* luaX_checkTextBlock3D(lua_State *L, int idx) {
	return GetTextBlock3D(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
#define ID_TextBlockConcept 0x4903089d
REGISTER_CLASS(TextBlock3D, ID_Node3D, ID_TextBlockConcept, 0);
HANDLER(Light3D, Node3D, Render);
COMPONENT_PROPERTIES(Light3D) = {
	DECL_COLOR(0xe5b43cf8, Light3D, Color, Color), // Light3D.Color
	DECL_STRUCT_TYPE(0xe2c2c340, Light3D, SpotAngle, SpotAngle, Vector2D), // Light3D.SpotAngle
	DECL_FLOAT(0x53b4f9aa, Light3D, Intensity, Intensity), // Light3D.Intensity
	DECL_FLOAT(0xa311e772, Light3D, Range, Range), // Light3D.Range
	DECL_ENUM(0xd155d06d, Light3D, Type, Type, LightType), // Light3D.Type
};
COMPONENT_DEFAULTS(Light3D) = {
		
  .Intensity = 1,
		
  .Range = 10,
};
COMPONENT_PROC_BEGIN(Light3D)
		DISPATCH(Light3D, Node3D, Render) // Node3D.Render
COMPONENT_PROC_END()
void luaX_pushLight3D(lua_State *L, struct Light3D const* Light3D) {
	luaX_pushObject(L, CMP_GetObject(Light3D));
}
struct Light3D* luaX_checkLight3D(lua_State *L, int idx) {
	return GetLight3D(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(Light3D, ID_Node3D, 0);
HANDLER(SpriteView, Node3D, Render);
COMPONENT_PROPERTIES(SpriteView) = {
	DECL_OBJECT(0x590ca79a, SpriteView, Image, Image, Texture), // SpriteView.Image
	DECL_STRUCT_TYPE(0x2d2c5028, SpriteView, Bounds, Bounds, Vector4D), // SpriteView.Bounds
};
COMPONENT_DEFAULTS(SpriteView) = {
};
COMPONENT_PROC_BEGIN(SpriteView)
		DISPATCH(SpriteView, Node3D, Render) // Node3D.Render
COMPONENT_PROC_END()
void luaX_pushSpriteView(lua_State *L, struct SpriteView const* SpriteView) {
	luaX_pushObject(L, CMP_GetObject(SpriteView));
}
struct SpriteView* luaX_checkSpriteView(lua_State *L, int idx) {
	return GetSpriteView(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(SpriteView, ID_Node3D, 0);

ORCA_API int luaopen_orca_SceneKit(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ NULL, NULL } 
	}));
		lua_setfield(L, ((void)luaopen_orca_Node3D_RenderEventArgs(L), -2), "Node3D_RenderEventArgs");
	lua_setfield(L, ((void)lua_pushclass(L, &_Node3D), -2), "Node3D");
	lua_setfield(L, ((void)lua_pushclass(L, &_Scene), -2), "Scene");
	lua_setfield(L, ((void)lua_pushclass(L, &_Model3D), -2), "Model3D");
	lua_setfield(L, ((void)lua_pushclass(L, &_PlaneMeshNode), -2), "PlaneMeshNode");
	lua_setfield(L, ((void)lua_pushclass(L, &_Camera), -2), "Camera");
	lua_setfield(L, ((void)lua_pushclass(L, &_TrajectoryList3D), -2), "TrajectoryList3D");
	lua_setfield(L, ((void)lua_pushclass(L, &_Viewport3D), -2), "Viewport3D");
	lua_setfield(L, ((void)lua_pushclass(L, &_PrefabView3D), -2), "PrefabView3D");
	lua_setfield(L, ((void)lua_pushclass(L, &_RenderPass), -2), "RenderPass");
	lua_setfield(L, ((void)lua_pushclass(L, &_CompositionTargetRenderPass), -2), "CompositionTargetRenderPass");
	lua_setfield(L, ((void)lua_pushclass(L, &_BlitRenderPass), -2), "BlitRenderPass");
	lua_setfield(L, ((void)lua_pushclass(L, &_ClearRenderPass), -2), "ClearRenderPass");
	lua_setfield(L, ((void)lua_pushclass(L, &_DrawObjectsRenderPass), -2), "DrawObjectsRenderPass");
	lua_setfield(L, ((void)lua_pushclass(L, &_PipelineStateRenderPass), -2), "PipelineStateRenderPass");
	lua_setfield(L, ((void)lua_pushclass(L, &_TextBlock3D), -2), "TextBlock3D");
	lua_setfield(L, ((void)lua_pushclass(L, &_Light3D), -2), "Light3D");
	lua_setfield(L, ((void)lua_pushclass(L, &_SpriteView), -2), "SpriteView");
	return 1;
}
