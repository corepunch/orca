// Auto-generated from SceneKit.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "SceneKit.h"

#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }


extern const char *_BlendMode[];
extern const char *_CompareFunc[];
#define ENUM(NAME, ...) \
ORCA_API const char *_##NAME[] = {__VA_ARGS__, NULL}; \
const char *NAME##ToString(enum NAME value) { \
	return (assert(value >= 0 && value < sizeof(_##NAME) / sizeof(*_##NAME) - 1), _##NAME[value]); \
} \
enum NAME luaX_check##NAME(lua_State *L, int idx) { \
	return luaL_checkoption(L, idx, NULL, _##NAME); \
} \
void luaX_push##NAME(lua_State *L, enum NAME value) { \
	lua_pushstring(L, (assert(value >= 0 && value < sizeof(_##NAME) / sizeof(*_##NAME) - 1), _##NAME[value])); \
}
ENUM(StencilOp, "Keep", "Zero", "Replace", "Increment", "IncrementWrap", "Decrement", "DecrementWrap", "Invert")
ENUM(ViewportMode, "Relative", "Absolute")
ENUM(CullMode, "None", "Back", "Front")
ENUM(ColorWriteMode, "None", "RGB", "RGBA", "R", "G", "B", "GB", "A")
ENUM(FovType, "Xfov", "Yfov")
ENUM(ProjectionType, "Perspective", "Orthographic")
ENUM(LightType, "Point", "Spot")
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, int idx, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(const char* str, struct PropertyType const* prop, void const* struct_ptr);

#define STRUCT(NAME, EXPORT) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
	if (data == NULL) { lua_pushnil(L); return; } \
	memcpy((luaL_setmetatable(L, #EXPORT), lua_newuserdata(L, sizeof(struct NAME))), data, sizeof(struct NAME)); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { return luaL_checkudata(L, idx, #EXPORT); } \
static int f_new_##NAME(lua_State *L) { \
	struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
	memset((luaL_setmetatable(L, #EXPORT), self), 0, sizeof(struct NAME)); \
	if (lua_istable(L, 1)) \
    for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
			lua_pop(L, (lua_getfield(L, 1, _##NAME[i].Name), read_property(L, -1, &_##NAME[i], self), 1)); \
	else for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		read_property(L, i + 1, &_##NAME[i], self); \
	return 1; \
} \
static int f_##NAME##___index(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (write_property(L, -1, &_##NAME[i], luaX_check##NAME(L, 1)), 1); \
	for (uint32_t i = 0; i < sizeof(_##NAME##_Methods) / sizeof(*_##NAME##_Methods); i++) { \
		if (strcmp(_##NAME##_Methods[i].name, luaL_checkstring(L, 2)) == 0) { \
			lua_pushcfunction(L, _##NAME##_Methods[i].func); \
			return 1; \
		} \
	} \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___newindex(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (read_property(L, 3, &_##NAME[i], luaX_check##NAME(L, 1)), 0); \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___call(lua_State *L) { \
	return ((void)lua_remove(L, 1), f_new_##NAME(L)); \
} \
static int f_##NAME##___fromstring(lua_State *L) { \
	char* tmp = strdup(luaL_checkstring(L, 1)),* tok = strtok(tmp, " "); \
	struct NAME self; \
	memset(&self, 0, sizeof(struct NAME)); \
	for (uint32_t i = 0; tok && i < sizeof(_##NAME) / sizeof(*_##NAME); i++, tok = strtok(NULL, " ")) \
		if (_##NAME[i].DataType != kDataTypeStruct) \
			parse_property(tok, &_##NAME[i], &self); \
	free(tmp); \
	return (luaX_push##NAME(L, &self), 1); \
} \
int luaopen_orca_##NAME(lua_State *L) { \
	luaL_newmetatable(L, #EXPORT); \
	luaL_setfuncs(L, ((luaL_Reg[]) { \
		{ "new", f_new_##NAME }, \
		{ "fromstring", f_##NAME##___fromstring }, \
		{ "__newindex", f_##NAME##___newindex }, \
		{ "__index", f_##NAME##___index }, \
		{ NULL, NULL }, \
	}), 0); \
	luaL_setfuncs(L, _##NAME##_Methods, 0); \
	/* Make struct creatable via constructor-like syntax */ \
	lua_newtable(L); \
	lua_pushcfunction(L, f_##NAME##___call); \
	lua_setfield(L, -2, "__call"); \
	lua_setmetatable(L, -2); \
	return 1; \
}



#define REGISTER_CLASS(NAME, ...) \
ORCA_API struct ClassDesc _##NAME = { \
	.ClassName = #NAME, \
	.DefaultName = #NAME, \
	.ContentType = #NAME, \
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
	.ParentClasses = { __VA_ARGS__ }, \
	.ClassID = ID_##NAME, \
	.ClassSize = sizeof(struct NAME), \
	.Properties = NAME##Properties, \
	.MessageTypes = NAME##MessageTypes, \
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
	.NumMessageTypes = k##NAME##NumMessageTypes, \
};
LRESULT Node3D_UpdateMatrix(struct Object*, struct Node3D*, wParam_t, UpdateMatrixMsgPtr);
static struct MessageType Node3DMessageTypes[kNode3DNumMessageTypes] = {	
};
static struct PropertyType const Node3DProperties[kNode3DNumProperties] = {
	DECL(0x3f19bf01, Node3D, LayoutTransform, LayoutTransform, kDataTypeStruct, .TypeString = "Transform3D"), // Node3D.LayoutTransform
	DECL(0xfc7e27e0, Node3D, LayoutTransformTranslation, LayoutTransform.translation, kDataTypeStruct, .TypeString = "Vector3D"), // Node3D.LayoutTransformTranslation
	DECL(0x9560ef43, Node3D, LayoutTransformRotation, LayoutTransform.rotation, kDataTypeStruct, .TypeString = "Vector3D"), // Node3D.LayoutTransformRotation
	DECL(0x5a2c3595, Node3D, LayoutTransformScale, LayoutTransform.scale, kDataTypeStruct, .TypeString = "Vector3D"), // Node3D.LayoutTransformScale
	DECL(0xe9e55063, Node3D, RenderTransform, RenderTransform, kDataTypeStruct, .TypeString = "Transform3D"), // Node3D.RenderTransform
	DECL(0xb8e70ec2, Node3D, RenderTransformTranslation, RenderTransform.translation, kDataTypeStruct, .TypeString = "Vector3D"), // Node3D.RenderTransformTranslation
	DECL(0x3c611efd, Node3D, RenderTransformRotation, RenderTransform.rotation, kDataTypeStruct, .TypeString = "Vector3D"), // Node3D.RenderTransformRotation
	DECL(0xd037e21b, Node3D, RenderTransformScale, RenderTransform.scale, kDataTypeStruct, .TypeString = "Vector3D"), // Node3D.RenderTransformScale
	DECL(0xdc65ec6d, Node3D, RenderTransformOrigin, RenderTransformOrigin, kDataTypeStruct, .TypeString = "Vector3D"), // Node3D.RenderTransformOrigin
	DECL(0x35a57c45, Node3D, ContentOffset, ContentOffset, kDataTypeStruct, .TypeString = "Vector3D"), // Node3D.ContentOffset
	DECL(0xe9d1810c, Node3D, Matrix, Matrix, kDataTypeStruct, .TypeString = "Matrix3D"), // Node3D.Matrix
	DECL(0xd5710b2e, Node3D, CalculatedOffset, CalculatedOffset, kDataTypeFloat), // Node3D.CalculatedOffset
};
static struct Node3D Node3DDefaults = {
		
  .LayoutTransform = {.scale={1,1,1}},
		
  .RenderTransform = {.scale={1,1,1}},
};
LRESULT Node3DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgUpdateMatrix: return Node3D_UpdateMatrix(object, cmp, wparm, lparm); // UpdateMatrix
	}
	return FALSE;
}
void luaX_pushNode3D(lua_State *L, struct Node3D const* Node3D) {
	luaX_pushObject(L, CMP_GetObject(Node3D));
}
struct Node3D* luaX_checkNode3D(lua_State *L, int idx) {
	return GetNode3D(luaX_checkObject(L, idx));
}
#define ID_Node 0x3468032d
REGISTER_CLASS(Node3D, ID_Node, 0);
LRESULT Scene_UpdateMatrix(struct Object*, struct Scene*, wParam_t, UpdateMatrixMsgPtr);
static struct MessageType SceneMessageTypes[kSceneNumMessageTypes] = {	
};
static struct PropertyType const SceneProperties[kSceneNumProperties] = {
	DECL(0xe74c7b6e, Scene, Camera, Camera, kDataTypeString), // Scene.Camera
	DECL(0x14a89218, Scene, PreviewCamera, PreviewCamera, kDataTypeString), // Scene.PreviewCamera
	DECL(0xf497b459, Scene, HitTestCamera, HitTestCamera, kDataTypeString), // Scene.HitTestCamera
	DECL(0xf64bbf80, Scene, RenderPass, RenderPass, kDataTypeObject, .TypeString = "RenderPass"), // Scene.RenderPass
	DECL(0x1fe3e9af, Scene, BackgroundTimelineSequence, BackgroundTimelineSequence, kDataTypeString), // Scene.BackgroundTimelineSequence
	DECL(0xedb9befb, Scene, ComposerOverride, ComposerOverride, kDataTypeObject, .TypeString = "RenderPass"), // Scene.ComposerOverride
	DECL(0x886f1cad, Scene, EnvironmentTexture, EnvironmentTexture, kDataTypeObject, .TypeString = "Texture"), // Scene.EnvironmentTexture
};
static struct Scene SceneDefaults = {
};
LRESULT SceneProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgUpdateMatrix: return Scene_UpdateMatrix(object, cmp, wparm, lparm); // UpdateMatrix
	}
	return FALSE;
}
void luaX_pushScene(lua_State *L, struct Scene const* Scene) {
	luaX_pushObject(L, CMP_GetObject(Scene));
}
struct Scene* luaX_checkScene(lua_State *L, int idx) {
	return GetScene(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(Scene, ID_Node3D, 0);
LRESULT Model3D_Render(struct Object*, struct Model3D*, wParam_t, RenderMsgPtr);
static struct MessageType Model3DMessageTypes[kModel3DNumMessageTypes] = {	
};
static struct PropertyType const Model3DProperties[kModel3DNumProperties] = {
	DECL(0x07e055dc, Model3D, Mesh, Mesh, kDataTypeObject, .TypeString = "Mesh"), // Model3D.Mesh
	DECL(0xcbd54f80, Model3D, Material, Material, kDataTypeObject, .TypeString = "Material"), // Model3D.Material
};
static struct Model3D Model3DDefaults = {
};
LRESULT Model3DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgRender: return Model3D_Render(object, cmp, wparm, lparm); // Render
	}
	return FALSE;
}
void luaX_pushModel3D(lua_State *L, struct Model3D const* Model3D) {
	luaX_pushObject(L, CMP_GetObject(Model3D));
}
struct Model3D* luaX_checkModel3D(lua_State *L, int idx) {
	return GetModel3D(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(Model3D, ID_Node3D, 0);
LRESULT PlaneMeshNode_Render(struct Object*, struct PlaneMeshNode*, wParam_t, RenderMsgPtr);
static struct MessageType PlaneMeshNodeMessageTypes[kPlaneMeshNodeNumMessageTypes] = {	
};
static struct PropertyType const PlaneMeshNodeProperties[kPlaneMeshNodeNumProperties] = {
	DECL(0x8f8d39cf, PlaneMeshNode, PlaneWidth, PlaneWidth, kDataTypeFloat), // PlaneMeshNode.PlaneWidth
	DECL(0x2d44a1f2, PlaneMeshNode, PlaneHeight, PlaneHeight, kDataTypeFloat), // PlaneMeshNode.PlaneHeight
	DECL(0x0a07d70c, PlaneMeshNode, PlaneInvertU, PlaneInvertU, kDataTypeBool), // PlaneMeshNode.PlaneInvertU
	DECL(0x0d07dbc5, PlaneMeshNode, PlaneInvertV, PlaneInvertV, kDataTypeBool), // PlaneMeshNode.PlaneInvertV
	DECL(0x57852420, PlaneMeshNode, GenerateTangents, GenerateTangents, kDataTypeBool), // PlaneMeshNode.GenerateTangents
	DECL(0x909a95b6, PlaneMeshNode, GenerateNormals, GenerateNormals, kDataTypeBool), // PlaneMeshNode.GenerateNormals
	DECL(0xe2d8fb55, PlaneMeshNode, MeshMaterial, MeshMaterial, kDataTypeObject, .TypeString = "Material"), // PlaneMeshNode.MeshMaterial
};
static struct PlaneMeshNode PlaneMeshNodeDefaults = {
};
LRESULT PlaneMeshNodeProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgRender: return PlaneMeshNode_Render(object, cmp, wparm, lparm); // Render
	}
	return FALSE;
}
void luaX_pushPlaneMeshNode(lua_State *L, struct PlaneMeshNode const* PlaneMeshNode) {
	luaX_pushObject(L, CMP_GetObject(PlaneMeshNode));
}
struct PlaneMeshNode* luaX_checkPlaneMeshNode(lua_State *L, int idx) {
	return GetPlaneMeshNode(luaX_checkObject(L, idx));
}
#define ID_Model3D 0xc56de5fd
REGISTER_CLASS(PlaneMeshNode, ID_Model3D, 0);
static struct MessageType CameraMessageTypes[kCameraNumMessageTypes] = {	
};
static struct PropertyType const CameraProperties[kCameraNumProperties] = {
	DECL(0x137e217c, Camera, Fov, Fov, kDataTypeFloat), // Camera.Fov
	DECL(0x0ef1c6f4, Camera, FovType, FovType, kDataTypeEnum, .EnumValues = _FovType), // Camera.FovType
	DECL(0x87320ce8, Camera, ProjectionType, ProjectionType, kDataTypeEnum, .EnumValues = _ProjectionType), // Camera.ProjectionType
	DECL(0x15af1da7, Camera, ZNear, ZNear, kDataTypeFloat), // Camera.ZNear
	DECL(0x993918c6, Camera, ZFar, ZFar, kDataTypeFloat), // Camera.ZFar
	DECL(0xd6d304c6, Camera, EyeSeparation, EyeSeparation, kDataTypeFloat), // Camera.EyeSeparation
	DECL(0x7e322435, Camera, FocalDistance, FocalDistance, kDataTypeFloat), // Camera.FocalDistance
	DECL(0xfa573077, Camera, OrthogonalPlaneHeight, OrthogonalPlaneHeight, kDataTypeFloat), // Camera.OrthogonalPlaneHeight
	DECL(0x026159d2, Camera, ZPositive, ZPositive, kDataTypeBool), // Camera.ZPositive
};
static struct Camera CameraDefaults = {
};
LRESULT CameraProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushCamera(lua_State *L, struct Camera const* Camera) {
	luaX_pushObject(L, CMP_GetObject(Camera));
}
struct Camera* luaX_checkCamera(lua_State *L, int idx) {
	return GetCamera(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(Camera, ID_Node3D, 0);
LRESULT TrajectoryList3D_UpdateMatrix(struct Object*, struct TrajectoryList3D*, wParam_t, UpdateMatrixMsgPtr);
static struct MessageType TrajectoryList3DMessageTypes[kTrajectoryList3DNumMessageTypes] = {	
};
static struct PropertyType const TrajectoryList3DProperties[kTrajectoryList3DNumProperties] = {
	DECL(0x4cf7cbf8, TrajectoryList3D, Trajectory, Trajectory, kDataTypeObject, .TypeString = "Trajectory"), // TrajectoryList3D.Trajectory
	DECL(0xeea06ebd, TrajectoryList3D, ScrollAxis, ScrollAxis, kDataTypeStruct, .TypeString = "Vector2D"), // TrajectoryList3D.ScrollAxis
	DECL(0x343782cd, TrajectoryList3D, Looping, Looping, kDataTypeBool), // TrajectoryList3D.Looping
	DECL(0x8777939e, TrajectoryList3D, Spacing, Spacing, kDataTypeFloat), // TrajectoryList3D.Spacing
	DECL(0xc742f477, TrajectoryList3D, SelectionBehavior, SelectionBehavior, kDataTypeFloat), // TrajectoryList3D.SelectionBehavior
	DECL(0x8af08301, TrajectoryList3D, SelectedItemIndex, SelectedItemIndex, kDataTypeFloat), // TrajectoryList3D.SelectedItemIndex
};
static struct TrajectoryList3D TrajectoryList3DDefaults = {
};
LRESULT TrajectoryList3DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgUpdateMatrix: return TrajectoryList3D_UpdateMatrix(object, cmp, wparm, lparm); // UpdateMatrix
	}
	return FALSE;
}
void luaX_pushTrajectoryList3D(lua_State *L, struct TrajectoryList3D const* TrajectoryList3D) {
	luaX_pushObject(L, CMP_GetObject(TrajectoryList3D));
}
struct TrajectoryList3D* luaX_checkTrajectoryList3D(lua_State *L, int idx) {
	return GetTrajectoryList3D(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(TrajectoryList3D, ID_Node3D, 0);
LRESULT Viewport3D_ForegroundContent(struct Object*, struct Viewport3D*, wParam_t, ForegroundContentMsgPtr);
static struct MessageType Viewport3DMessageTypes[kViewport3DNumMessageTypes] = {	
};
static struct PropertyType const Viewport3DProperties[kViewport3DNumProperties] = {
	DECL(0xe74c7b6e, Viewport3D, Camera, Camera, kDataTypeString), // Viewport3D.Camera
	DECL(0x14a89218, Viewport3D, PreviewCamera, PreviewCamera, kDataTypeString), // Viewport3D.PreviewCamera
	DECL(0xf497b459, Viewport3D, HitTestCamera, HitTestCamera, kDataTypeString), // Viewport3D.HitTestCamera
	DECL(0xf64bbf80, Viewport3D, RenderPass, RenderPass, kDataTypeObject, .TypeString = "RenderPass"), // Viewport3D.RenderPass
	DECL(0xc89b38b3, Viewport3D, Scene, Scene, kDataTypeString), // Viewport3D.Scene
};
static struct Viewport3D Viewport3DDefaults = {
};
LRESULT Viewport3DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgForegroundContent: return Viewport3D_ForegroundContent(object, cmp, wparm, lparm); // ForegroundContent
	}
	return FALSE;
}
void luaX_pushViewport3D(lua_State *L, struct Viewport3D const* Viewport3D) {
	luaX_pushObject(L, CMP_GetObject(Viewport3D));
}
struct Viewport3D* luaX_checkViewport3D(lua_State *L, int idx) {
	return GetViewport3D(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(Viewport3D, ID_Node2D, 0);
LRESULT PrefabView3D_LoadView(struct Object*, struct PrefabView3D*, wParam_t, LoadViewMsgPtr);
static struct MessageType PrefabView3DMessageTypes[kPrefabView3DNumMessageTypes] = {	
};
static struct PropertyType const PrefabView3DProperties[kPrefabView3DNumProperties] = {
	DECL(0x57f28ff6, PrefabView3D, SCA, SCA, kDataTypeString), // PrefabView3D.SCA
	DECL(0xd6415ba3, PrefabView3D, Prefab, Prefab, kDataTypeString), // PrefabView3D.Prefab
};
static struct PrefabView3D PrefabView3DDefaults = {
};
LRESULT PrefabView3DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgLoadView: return PrefabView3D_LoadView(object, cmp, wparm, lparm); // LoadView
	}
	return FALSE;
}
void luaX_pushPrefabView3D(lua_State *L, struct PrefabView3D const* PrefabView3D) {
	luaX_pushObject(L, CMP_GetObject(PrefabView3D));
}
struct PrefabView3D* luaX_checkPrefabView3D(lua_State *L, int idx) {
	return GetPrefabView3D(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(PrefabView3D, ID_Node3D, 0);
static struct MessageType RenderPassMessageTypes[kRenderPassNumMessageTypes] = {	
};
static struct PropertyType const RenderPassProperties[kRenderPassNumProperties] = {
};
static struct RenderPass RenderPassDefaults = {
};
LRESULT RenderPassProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushRenderPass(lua_State *L, struct RenderPass const* RenderPass) {
	luaX_pushObject(L, CMP_GetObject(RenderPass));
}
struct RenderPass* luaX_checkRenderPass(lua_State *L, int idx) {
	return GetRenderPass(luaX_checkObject(L, idx));
}
#define ID_Node 0x3468032d
REGISTER_CLASS(RenderPass, ID_Node, 0);
static struct MessageType CompositionTargetRenderPassMessageTypes[kCompositionTargetRenderPassNumMessageTypes] = {	
};
static struct PropertyType const CompositionTargetRenderPassProperties[kCompositionTargetRenderPassNumProperties] = {
};
static struct CompositionTargetRenderPass CompositionTargetRenderPassDefaults = {
};
LRESULT CompositionTargetRenderPassProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushCompositionTargetRenderPass(lua_State *L, struct CompositionTargetRenderPass const* CompositionTargetRenderPass) {
	luaX_pushObject(L, CMP_GetObject(CompositionTargetRenderPass));
}
struct CompositionTargetRenderPass* luaX_checkCompositionTargetRenderPass(lua_State *L, int idx) {
	return GetCompositionTargetRenderPass(luaX_checkObject(L, idx));
}
#define ID_RenderPass 0xf64bbf80
REGISTER_CLASS(CompositionTargetRenderPass, ID_RenderPass, 0);
static struct MessageType BlitRenderPassMessageTypes[kBlitRenderPassNumMessageTypes] = {	
};
static struct PropertyType const BlitRenderPassProperties[kBlitRenderPassNumProperties] = {
};
static struct BlitRenderPass BlitRenderPassDefaults = {
};
LRESULT BlitRenderPassProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushBlitRenderPass(lua_State *L, struct BlitRenderPass const* BlitRenderPass) {
	luaX_pushObject(L, CMP_GetObject(BlitRenderPass));
}
struct BlitRenderPass* luaX_checkBlitRenderPass(lua_State *L, int idx) {
	return GetBlitRenderPass(luaX_checkObject(L, idx));
}
#define ID_RenderPass 0xf64bbf80
REGISTER_CLASS(BlitRenderPass, ID_RenderPass, 0);
static struct MessageType ClearRenderPassMessageTypes[kClearRenderPassNumMessageTypes] = {	
};
static struct PropertyType const ClearRenderPassProperties[kClearRenderPassNumProperties] = {
	DECL(0xeb16b675, ClearRenderPass, ClearColor, ClearColor, kDataTypeColor), // ClearRenderPass.ClearColor
	DECL(0xa444e35b, ClearRenderPass, ClearDepth, ClearDepth, kDataTypeFloat), // ClearRenderPass.ClearDepth
	DECL(0x222bca4e, ClearRenderPass, ClearStencil, ClearStencil, kDataTypeInt), // ClearRenderPass.ClearStencil
};
static struct ClearRenderPass ClearRenderPassDefaults = {
};
LRESULT ClearRenderPassProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushClearRenderPass(lua_State *L, struct ClearRenderPass const* ClearRenderPass) {
	luaX_pushObject(L, CMP_GetObject(ClearRenderPass));
}
struct ClearRenderPass* luaX_checkClearRenderPass(lua_State *L, int idx) {
	return GetClearRenderPass(luaX_checkObject(L, idx));
}
#define ID_RenderPass 0xf64bbf80
REGISTER_CLASS(ClearRenderPass, ID_RenderPass, 0);
static struct MessageType DrawObjectsRenderPassMessageTypes[kDrawObjectsRenderPassNumMessageTypes] = {	
};
static struct PropertyType const DrawObjectsRenderPassProperties[kDrawObjectsRenderPassNumProperties] = {
	DECL(0xe74c7b6e, DrawObjectsRenderPass, Camera, Camera, kDataTypeString), // DrawObjectsRenderPass.Camera
	DECL(0x785c377a, DrawObjectsRenderPass, IncludeTags, IncludeTags, kDataTypeString), // DrawObjectsRenderPass.IncludeTags
	DECL(0x2de02b74, DrawObjectsRenderPass, ExcludeTags, ExcludeTags, kDataTypeString), // DrawObjectsRenderPass.ExcludeTags
};
static struct DrawObjectsRenderPass DrawObjectsRenderPassDefaults = {
};
LRESULT DrawObjectsRenderPassProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushDrawObjectsRenderPass(lua_State *L, struct DrawObjectsRenderPass const* DrawObjectsRenderPass) {
	luaX_pushObject(L, CMP_GetObject(DrawObjectsRenderPass));
}
struct DrawObjectsRenderPass* luaX_checkDrawObjectsRenderPass(lua_State *L, int idx) {
	return GetDrawObjectsRenderPass(luaX_checkObject(L, idx));
}
#define ID_RenderPass 0xf64bbf80
REGISTER_CLASS(DrawObjectsRenderPass, ID_RenderPass, 0);
static struct MessageType PipelineStateRenderPassMessageTypes[kPipelineStateRenderPassNumMessageTypes] = {	
};
static struct PropertyType const PipelineStateRenderPassProperties[kPipelineStateRenderPassNumProperties] = {
	DECL(0x0038792b, PipelineStateRenderPass, BlendMode, BlendMode, kDataTypeEnum, .EnumValues = _BlendMode), // PipelineStateRenderPass.BlendMode
	DECL(0x9d0d3c20, PipelineStateRenderPass, ColorWriteMode, ColorWriteMode, kDataTypeEnum, .EnumValues = _ColorWriteMode), // PipelineStateRenderPass.ColorWriteMode
	DECL(0xfe9cfa12, PipelineStateRenderPass, CullMode, CullMode, kDataTypeEnum, .EnumValues = _CullMode), // PipelineStateRenderPass.CullMode
	DECL(0xb7e582be, PipelineStateRenderPass, DepthTestFunction, DepthTestFunction, kDataTypeEnum, .EnumValues = _CompareFunc), // PipelineStateRenderPass.DepthTestFunction
	DECL(0x8ec3072e, PipelineStateRenderPass, DepthWriteEnabled, DepthWriteEnabled, kDataTypeBool), // PipelineStateRenderPass.DepthWriteEnabled
	DECL(0x2dd5f6cc, PipelineStateRenderPass, ViewportMode, ViewportMode, kDataTypeEnum, .EnumValues = _ViewportMode), // PipelineStateRenderPass.ViewportMode
	DECL(0x8c6862de, PipelineStateRenderPass, ScissorMode, ScissorMode, kDataTypeEnum, .EnumValues = _ViewportMode), // PipelineStateRenderPass.ScissorMode
	DECL(0x60b5afe3, PipelineStateRenderPass, Viewport, Viewport, kDataTypeStruct, .TypeString = "Vector4D"), // PipelineStateRenderPass.Viewport
	DECL(0x24b3e321, PipelineStateRenderPass, Scissor, Scissor, kDataTypeStruct, .TypeString = "Vector4D"), // PipelineStateRenderPass.Scissor
	DECL(0xc770fa47, PipelineStateRenderPass, StencilTestFunction, StencilTestFunction, kDataTypeEnum, .EnumValues = _CompareFunc), // PipelineStateRenderPass.StencilTestFunction
	DECL(0x2ddc4e49, PipelineStateRenderPass, StencilReferenceValue, StencilReferenceValue, kDataTypeInt), // PipelineStateRenderPass.StencilReferenceValue
	DECL(0x82336fe7, PipelineStateRenderPass, StencilMask, StencilMask, kDataTypeInt), // PipelineStateRenderPass.StencilMask
	DECL(0x8785338c, PipelineStateRenderPass, StencilFailOperation, StencilFailOperation, kDataTypeEnum, .EnumValues = _StencilOp), // PipelineStateRenderPass.StencilFailOperation
	DECL(0xfc4e75c8, PipelineStateRenderPass, StencilPassDepthFailOperation, StencilPassDepthFailOperation, kDataTypeEnum, .EnumValues = _StencilOp), // PipelineStateRenderPass.StencilPassDepthFailOperation
	DECL(0x6452d1b7, PipelineStateRenderPass, StencilPassDepthPassOperation, StencilPassDepthPassOperation, kDataTypeEnum, .EnumValues = _StencilOp), // PipelineStateRenderPass.StencilPassDepthPassOperation
	DECL(0xb0b1cdf3, PipelineStateRenderPass, StencilWriteEnabled, StencilWriteEnabled, kDataTypeBool), // PipelineStateRenderPass.StencilWriteEnabled
};
static struct PipelineStateRenderPass PipelineStateRenderPassDefaults = {
		
  .ViewportMode = kViewportModeRelative,
		
  .ScissorMode = kViewportModeRelative,
};
LRESULT PipelineStateRenderPassProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushPipelineStateRenderPass(lua_State *L, struct PipelineStateRenderPass const* PipelineStateRenderPass) {
	luaX_pushObject(L, CMP_GetObject(PipelineStateRenderPass));
}
struct PipelineStateRenderPass* luaX_checkPipelineStateRenderPass(lua_State *L, int idx) {
	return GetPipelineStateRenderPass(luaX_checkObject(L, idx));
}
#define ID_RenderPass 0xf64bbf80
REGISTER_CLASS(PipelineStateRenderPass, ID_RenderPass, 0);
LRESULT TextBlock3D_Render(struct Object*, struct TextBlock3D*, wParam_t, RenderMsgPtr);
LRESULT TextBlock3D_Create(struct Object*, struct TextBlock3D*, wParam_t, CreateMsgPtr);
static struct MessageType TextBlock3DMessageTypes[kTextBlock3DNumMessageTypes] = {	
};
static struct PropertyType const TextBlock3DProperties[kTextBlock3DNumProperties] = {
};
static struct TextBlock3D TextBlock3DDefaults = {
};
LRESULT TextBlock3DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgRender: return TextBlock3D_Render(object, cmp, wparm, lparm); // Render
		case kMsgCreate: return TextBlock3D_Create(object, cmp, wparm, lparm); // Create
	}
	return FALSE;
}
void luaX_pushTextBlock3D(lua_State *L, struct TextBlock3D const* TextBlock3D) {
	luaX_pushObject(L, CMP_GetObject(TextBlock3D));
}
struct TextBlock3D* luaX_checkTextBlock3D(lua_State *L, int idx) {
	return GetTextBlock3D(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
#define ID_TextBlockConcept 0x4903089d
REGISTER_CLASS(TextBlock3D, ID_Node3D, ID_TextBlockConcept, 0);
LRESULT Light3D_Render(struct Object*, struct Light3D*, wParam_t, RenderMsgPtr);
static struct MessageType Light3DMessageTypes[kLight3DNumMessageTypes] = {	
};
static struct PropertyType const Light3DProperties[kLight3DNumProperties] = {
	DECL(0xe5b43cf8, Light3D, Color, Color, kDataTypeColor), // Light3D.Color
	DECL(0xe2c2c340, Light3D, SpotAngle, SpotAngle, kDataTypeStruct, .TypeString = "Vector2D"), // Light3D.SpotAngle
	DECL(0x53b4f9aa, Light3D, Intensity, Intensity, kDataTypeFloat), // Light3D.Intensity
	DECL(0xa311e772, Light3D, Range, Range, kDataTypeFloat), // Light3D.Range
	DECL(0xd155d06d, Light3D, Type, Type, kDataTypeEnum, .EnumValues = _LightType), // Light3D.Type
};
static struct Light3D Light3DDefaults = {
		
  .Intensity = 1,
		
  .Range = 10,
};
LRESULT Light3DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgRender: return Light3D_Render(object, cmp, wparm, lparm); // Render
	}
	return FALSE;
}
void luaX_pushLight3D(lua_State *L, struct Light3D const* Light3D) {
	luaX_pushObject(L, CMP_GetObject(Light3D));
}
struct Light3D* luaX_checkLight3D(lua_State *L, int idx) {
	return GetLight3D(luaX_checkObject(L, idx));
}
#define ID_Node3D 0xce61fe5a
REGISTER_CLASS(Light3D, ID_Node3D, 0);
LRESULT SpriteView_Render(struct Object*, struct SpriteView*, wParam_t, RenderMsgPtr);
static struct MessageType SpriteViewMessageTypes[kSpriteViewNumMessageTypes] = {	
};
static struct PropertyType const SpriteViewProperties[kSpriteViewNumProperties] = {
	DECL(0x590ca79a, SpriteView, Image, Image, kDataTypeObject, .TypeString = "Texture"), // SpriteView.Image
	DECL(0x2d2c5028, SpriteView, Bounds, Bounds, kDataTypeStruct, .TypeString = "Vector4D"), // SpriteView.Bounds
};
static struct SpriteView SpriteViewDefaults = {
};
LRESULT SpriteViewProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgRender: return SpriteView_Render(object, cmp, wparm, lparm); // Render
	}
	return FALSE;
}
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
