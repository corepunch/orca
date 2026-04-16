// Auto-generated from SpriteKit.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "SpriteKit.h"

#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }


extern const char *_BlendMode[];
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
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(lua_State *L, const char* str, struct PropertyType const* prop, void* struct_ptr);

#define STRUCT(NAME, EXPORT) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
	if (data == NULL) { lua_pushnil(L); return; } \
	memcpy(lua_newuserdata(L, sizeof(struct NAME)), data, sizeof(struct NAME)); \
	luaL_setmetatable(L, #EXPORT); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { return luaL_checkudata(L, idx, #EXPORT); } \
static int f_new_##NAME(lua_State *L) { \
	struct NAME self; \
	memset(&self, 0, sizeof(struct NAME)); \
	if (lua_islightuserdata(L, 1)) { \
		memcpy(&self, lua_touserdata(L, 1), sizeof(struct NAME)); \
	} else if (lua_istable(L, 1)) \
		for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); lua_pop(L, 1), i++) { \
			if (lua_getfield(L, 1, _##NAME[i].Name)) \
				read_property(L, -1, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); } \
	else for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		read_property(L, i + 1, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); \
	luaX_push##NAME(L, &self); \
	return 1; \
} \
static int f_##NAME##___index(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (write_property(L, &_##NAME[i], ((char*)luaX_check##NAME(L, 1))+_##NAME[i].Offset), 1); \
	for (uint32_t i = 0; i < sizeof(_##NAME##_Methods) / sizeof(*_##NAME##_Methods); i++) { \
		if (_##NAME##_Methods[i].name && strcmp(_##NAME##_Methods[i].name, luaL_checkstring(L, 2)) == 0) { \
			lua_pushcfunction(L, _##NAME##_Methods[i].func); \
			return 1; \
		} \
	} \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___newindex(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (read_property(L, 3, &_##NAME[i], ((char*)luaX_check##NAME(L, 1))+_##NAME[i].Offset), 0); \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___call(lua_State *L) { \
  lua_insert(L, (lua_getfield(L, 1, "new"), 2)); \
  lua_call(L, lua_gettop(L) - 2, 1); \
	return 1; \
} \
static int f_##NAME##___fromstring(lua_State *L) { \
	char* tmp = strdup(luaL_checkstring(L, 1)),* tok = strtok(tmp, " "); \
	struct NAME self; \
	memset(&self, 0, sizeof(struct NAME)); \
	for (uint32_t i = 0; tok && i < sizeof(_##NAME) / sizeof(*_##NAME); i++, tok = strtok(NULL, " ")) \
		if (_##NAME[i].DataType != kDataTypeStruct) \
			parse_property(L, tok, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); \
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
static struct PropertyType _SpriteFrame[] = {
	DECL(0x6b109927, SpriteFrame, Rect, Rect, kDataTypeStruct, .TypeString = "Rectangle"), // SpriteFrame.Rect
	DECL(0x0a21a2ed, SpriteFrame, RectX, Rect.x, kDataTypeFloat), // SpriteFrame.RectX
	DECL(0x0921a15a, SpriteFrame, RectY, Rect.y, kDataTypeFloat), // SpriteFrame.RectY
	DECL(0x25c57ce9, SpriteFrame, RectWidth, Rect.width, kDataTypeFloat), // SpriteFrame.RectWidth
	DECL(0x68097dd8, SpriteFrame, RectHeight, Rect.height, kDataTypeFloat), // SpriteFrame.RectHeight
	DECL(0xae3d25c0, SpriteFrame, UvRect, UvRect, kDataTypeStruct, .TypeString = "Rectangle"), // SpriteFrame.UvRect
	DECL(0xe2422e48, SpriteFrame, UvRectX, UvRect.x, kDataTypeFloat), // SpriteFrame.UvRectX
	DECL(0xe3422fdb, SpriteFrame, UvRectY, UvRect.y, kDataTypeFloat), // SpriteFrame.UvRectY
	DECL(0xec9d6ecc, SpriteFrame, UvRectWidth, UvRect.width, kDataTypeFloat), // SpriteFrame.UvRectWidth
	DECL(0xdbdb8abb, SpriteFrame, UvRectHeight, UvRect.height, kDataTypeFloat), // SpriteFrame.UvRectHeight
};
static luaL_Reg _SpriteFrame_Methods[] = {
	{ NULL, NULL }
};

STRUCT(SpriteFrame, SpriteFrame);

static luaL_Reg _SKNode_RenderEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _SKNode_RenderEventArgs[] = {
	DECL(0xce9ab61f, SKNode_RenderEventArgs, ViewDef, ViewDef, kDataTypeStruct, .TypeString = "ViewDef"), // SKNode_RenderEventArgs.ViewDef
};

STRUCT(SKNode_RenderEventArgs, SKNode_RenderEventArgs);
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
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
};
#define REGISTER_ATTACH_ONLY_CLASS(NAME, ...) \
ORCA_API struct ClassDesc _##NAME = { \
	.ClassName = #NAME, \
	.DefaultName = #NAME, \
	.ContentType = #NAME, \
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
	.ParentClasses = { __VA_ARGS__ }, \
	.ClassID = ID_##NAME, \
	.ClassSize = sizeof(struct NAME), \
	.Properties = NAME##Properties, \
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
	.IsAttachOnly = TRUE, \
};
static struct PropertyType const SpriteAnimationProperties[kSpriteAnimationNumProperties] = {
	DECL(0x590ca79a, SpriteAnimation, Image, Image, kDataTypeObject, .TypeString = "Texture"), // SpriteAnimation.Image
	DECL(0xbebf2a84, SpriteAnimation, Framerate, Framerate, kDataTypeFloat), // SpriteAnimation.Framerate
	ARRAY_DECL(0xf03e266f, SpriteAnimation, Frames, Frames, kDataTypeStruct, .TypeString = "SpriteFrame"), // SpriteAnimation.Frames
	DECL(0x32caacb1, SpriteAnimation, NumFrames, NumFrames, kDataTypeInt), // SpriteAnimation.NumFrames
};
static struct SpriteAnimation SpriteAnimationDefaults = {
};
LRESULT SpriteAnimationProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushSpriteAnimation(lua_State *L, struct SpriteAnimation const* SpriteAnimation) {
	luaX_pushObject(L, CMP_GetObject(SpriteAnimation));
}
struct SpriteAnimation* luaX_checkSpriteAnimation(lua_State *L, int idx) {
	return GetSpriteAnimation(luaX_checkObject(L, idx));
}
REGISTER_CLASS(SpriteAnimation, 0);
HANDLER(SKNode, Node, UpdateMatrix);
static struct PropertyType const SKNodeProperties[kSKNodeNumProperties] = {
	DECL(0xe27f342a, SKNode, Position, Position, kDataTypeStruct, .TypeString = "Vector2D"), // SKNode.Position
	DECL(0xa6478e7c, SKNode, Size, Size, kDataTypeStruct, .TypeString = "Vector2D"), // SKNode.Size
	DECL(0xb54055d4, SKNode, Anchor, Anchor, kDataTypeStruct, .TypeString = "Vector2D"), // SKNode.Anchor
	DECL(0x350cf42d, SKNode, Render, Render, kDataTypeEvent, .TypeString = "SKNode_RenderEventArgs"), // SKNode.Render
};
static struct SKNode SKNodeDefaults = {
};
LRESULT SKNodeProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node_UpdateMatrix: return SKNode_UpdateMatrix(object, cmp, wparm, lparm); // Node.UpdateMatrix
	}
	return FALSE;
}
void luaX_pushSKNode(lua_State *L, struct SKNode const* SKNode) {
	luaX_pushObject(L, CMP_GetObject(SKNode));
}
struct SKNode* luaX_checkSKNode(lua_State *L, int idx) {
	return GetSKNode(luaX_checkObject(L, idx));
}
#define ID_Node 0x3468032d
REGISTER_CLASS(SKNode, ID_Node, 0);
HANDLER(SKScene, Node, UpdateMatrix);
static struct PropertyType const SKSceneProperties[kSKSceneNumProperties] = {
};
static struct SKScene SKSceneDefaults = {
};
LRESULT SKSceneProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node_UpdateMatrix: return SKScene_UpdateMatrix(object, cmp, wparm, lparm); // Node.UpdateMatrix
	}
	return FALSE;
}
void luaX_pushSKScene(lua_State *L, struct SKScene const* SKScene) {
	luaX_pushObject(L, CMP_GetObject(SKScene));
}
struct SKScene* luaX_checkSKScene(lua_State *L, int idx) {
	return GetSKScene(luaX_checkObject(L, idx));
}
#define ID_SKNode 0x819821fb
REGISTER_CLASS(SKScene, ID_SKNode, 0);
HANDLER(SKSpriteNode, SKNode, Render);
static struct PropertyType const SKSpriteNodeProperties[kSKSpriteNodeNumProperties] = {
	DECL(0x41e389fd, SKSpriteNode, Animation, Animation, kDataTypeObject, .TypeString = "SpriteAnimation"), // SKSpriteNode.Animation
	DECL(0x8831f0dd, SKSpriteNode, Animation2, Animation2, kDataTypeObject, .TypeString = "SpriteAnimation"), // SKSpriteNode.Animation2
	DECL(0x590ca79a, SKSpriteNode, Image, Image, kDataTypeObject, .TypeString = "Texture"), // SKSpriteNode.Image
	DECL(0xa9f98a53, SKSpriteNode, FreezeFrame, FreezeFrame, kDataTypeInt), // SKSpriteNode.FreezeFrame
	DECL(0x0038792b, SKSpriteNode, BlendMode, BlendMode, kDataTypeEnum, .EnumValues = _BlendMode), // SKSpriteNode.BlendMode
	DECL(0xae3d25c0, SKSpriteNode, UvRect, UvRect, kDataTypeStruct, .TypeString = "Rectangle"), // SKSpriteNode.UvRect
	DECL(0xe2422e48, SKSpriteNode, UvRectX, UvRect.x, kDataTypeFloat), // SKSpriteNode.UvRectX
	DECL(0xe3422fdb, SKSpriteNode, UvRectY, UvRect.y, kDataTypeFloat), // SKSpriteNode.UvRectY
	DECL(0xec9d6ecc, SKSpriteNode, UvRectWidth, UvRect.width, kDataTypeFloat), // SKSpriteNode.UvRectWidth
	DECL(0xdbdb8abb, SKSpriteNode, UvRectHeight, UvRect.height, kDataTypeFloat), // SKSpriteNode.UvRectHeight
};
static struct SKSpriteNode SKSpriteNodeDefaults = {
		
  .FreezeFrame = -1,
		
  .BlendMode = kBlendModeAlphaAutomatic,
};
LRESULT SKSpriteNodeProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_SKNode_Render: return SKSpriteNode_Render(object, cmp, wparm, lparm); // SKNode.Render
	}
	return FALSE;
}
void luaX_pushSKSpriteNode(lua_State *L, struct SKSpriteNode const* SKSpriteNode) {
	luaX_pushObject(L, CMP_GetObject(SKSpriteNode));
}
struct SKSpriteNode* luaX_checkSKSpriteNode(lua_State *L, int idx) {
	return GetSKSpriteNode(luaX_checkObject(L, idx));
}
#define ID_SKNode 0x819821fb
REGISTER_CLASS(SKSpriteNode, ID_SKNode, 0);
HANDLER(SKLabelNode, SKNode, Render);
HANDLER(SKLabelNode, Object, Create);
static struct PropertyType const SKLabelNodeProperties[kSKLabelNodeNumProperties] = {
	DECL(0xe5b43cf8, SKLabelNode, Color, Color, kDataTypeColor), // SKLabelNode.Color
};
static struct SKLabelNode SKLabelNodeDefaults = {
};
LRESULT SKLabelNodeProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_SKNode_Render: return SKLabelNode_Render(object, cmp, wparm, lparm); // SKNode.Render
		case ID_Object_Create: return SKLabelNode_Create(object, cmp, wparm, lparm); // Object.Create
	}
	return FALSE;
}
void luaX_pushSKLabelNode(lua_State *L, struct SKLabelNode const* SKLabelNode) {
	luaX_pushObject(L, CMP_GetObject(SKLabelNode));
}
struct SKLabelNode* luaX_checkSKLabelNode(lua_State *L, int idx) {
	return GetSKLabelNode(luaX_checkObject(L, idx));
}
#define ID_SKNode 0x819821fb
#define ID_TextBlockConcept 0x4903089d
REGISTER_CLASS(SKLabelNode, ID_SKNode, ID_TextBlockConcept, 0);
HANDLER(SKView, Node2D, ForegroundContent);
static struct PropertyType const SKViewProperties[kSKViewNumProperties] = {
	DECL(0x499d2ae6, SKView, ReferenceWidth, ReferenceWidth, kDataTypeFloat), // SKView.ReferenceWidth
	DECL(0xf011cff9, SKView, ReferenceHeight, ReferenceHeight, kDataTypeFloat), // SKView.ReferenceHeight
	DECL(0xc89b38b3, SKView, Scene, Scene, kDataTypeString), // SKView.Scene
};
static struct SKView SKViewDefaults = {
};
LRESULT SKViewProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node2D_ForegroundContent: return SKView_ForegroundContent(object, cmp, wparm, lparm); // Node2D.ForegroundContent
	}
	return FALSE;
}
void luaX_pushSKView(lua_State *L, struct SKView const* SKView) {
	luaX_pushObject(L, CMP_GetObject(SKView));
}
struct SKView* luaX_checkSKView(lua_State *L, int idx) {
	return GetSKView(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(SKView, ID_Node2D, 0);

ORCA_API int luaopen_orca_SpriteKit(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ NULL, NULL } 
	}));
	void on_spritekit_module_registered(lua_State *L);
	on_spritekit_module_registered(L);
	lua_setfield(L, ((void)luaopen_orca_SpriteFrame(L), -2), "SpriteFrame");
	lua_setfield(L, ((void)luaopen_orca_SKNode_RenderEventArgs(L), -2), "SKNode_RenderEventArgs");
	lua_setfield(L, ((void)lua_pushclass(L, &_SpriteAnimation), -2), "SpriteAnimation");
	lua_setfield(L, ((void)lua_pushclass(L, &_SKNode), -2), "SKNode");
	lua_setfield(L, ((void)lua_pushclass(L, &_SKScene), -2), "SKScene");
	lua_setfield(L, ((void)lua_pushclass(L, &_SKSpriteNode), -2), "SKSpriteNode");
	lua_setfield(L, ((void)lua_pushclass(L, &_SKLabelNode), -2), "SKLabelNode");
	lua_setfield(L, ((void)lua_pushclass(L, &_SKView), -2), "SKView");
	return 1;
}
