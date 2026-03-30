// Auto-generated from SpriteKit.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "SpriteKit.h"


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
#define FIELD(IDENT, STRUCT, NAME, TYPE, ...) { \
.Name = #NAME, \
.ShortIdentifier = IDENT, \
.DataType = TYPE, \
.Offset = offsetof(struct STRUCT, NAME), \
.DataSize = sizeof(((struct STRUCT*)NULL)->NAME), \
##__VA_ARGS__ \
}
#define STRUCT(NAME, EXPORT, ...) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
if (data == NULL) { lua_pushnil(L); return; } \
struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
luaL_setmetatable(L, EXPORT); \
memcpy(self, data, sizeof(struct NAME)); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { \
return luaL_checkudata(L, idx, EXPORT); \
} \
static int f_new_##NAME(lua_State *L) { \
struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
luaL_setmetatable(L, EXPORT); \
memset(self, 0, sizeof(struct NAME)); \
if (lua_gettop(L) == 1) return 1; \
luaX_struct_new(L, self, _##NAME, sizeof(_##NAME)/sizeof(*_##NAME)); \
return 1; \
} \
int f_##NAME##___index(lua_State *L) { \
struct NAME* self = luaX_check##NAME(L, 1); \
if (luaX_struct_index(L, self, _##NAME, sizeof(_##NAME)/sizeof(*_##NAME))) return 1; \
lua_getmetatable(L, 1); \
lua_pushvalue(L, 2); \
lua_rawget(L, -2); \
return 1; \
} \
int f_##NAME##___newindex(lua_State *L) { \
return luaX_struct_newindex(L, luaX_check##NAME(L, 1), _##NAME, sizeof(_##NAME)/sizeof(*_##NAME)); \
} \
static int f_##NAME##___call(lua_State *L) { \
return ((void)lua_remove(L, 1), f_new_##NAME(L)); \
} \
int luaopen_orca_##NAME(lua_State *L) { \
luaL_newmetatable(L, EXPORT); \
luaL_setfuncs(L, ((luaL_Reg[]) { \
{ "new", f_new_##NAME }, \
{ "__newindex", f_##NAME##___newindex }, \
{ "__index", f_##NAME##___index }, \
__VA_ARGS__ \
{ NULL, NULL }, \
}), 0); \
lua_newtable(L); \
lua_pushcfunction(L, f_##NAME##___call); \
lua_setfield(L, -2, "__call"); \
lua_setmetatable(L, -2); \
return 1; \
}
static struct PropertyType _SpriteFrame[] = {
FIELD(0x6b109927, SpriteFrame, Rect, kDataTypeStruct, .TypeString = "Rectangle"),
FIELD(0xae3d25c0, SpriteFrame, UvRect, kDataTypeStruct, .TypeString = "Rectangle"),
};

STRUCT(SpriteFrame, "SpriteFrame",
)
#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }


static struct MessageType SpriteAnimationMessageTypes[kSpriteAnimationNumMessageTypes] = {	
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
ORCA_API struct ClassDesc _SpriteAnimation = {
	.ClassName = "SpriteAnimation",
	.DefaultName = "SpriteAnimation",
	.ContentType = "SpriteAnimation",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_SpriteAnimation,
	.ClassSize = sizeof(struct SpriteAnimation),
	.Properties = SpriteAnimationProperties,
	.MessageTypes = SpriteAnimationMessageTypes,
	.ObjProc = SpriteAnimationProc,
	.Defaults = &SpriteAnimationDefaults,
	.NumProperties = kSpriteAnimationNumProperties,
	.NumMessageTypes = kSpriteAnimationNumMessageTypes,
};

LRESULT SKNode_UpdateMatrix(struct Object*, struct SKNode*, wParam_t, UpdateMatrixMsgPtr);

static struct MessageType SKNodeMessageTypes[kSKNodeNumMessageTypes] = {	
};

static struct PropertyType const SKNodeProperties[kSKNodeNumProperties] = {
	DECL(0xe27f342a, SKNode, Position, Position, kDataTypeStruct, .TypeString = "Vector2D"), // SKNode.Position
	DECL(0xa6478e7c, SKNode, Size, Size, kDataTypeStruct, .TypeString = "Vector2D"), // SKNode.Size
	DECL(0xb54055d4, SKNode, Anchor, Anchor, kDataTypeStruct, .TypeString = "Vector2D"), // SKNode.Anchor
};
static struct SKNode SKNodeDefaults = {
};
LRESULT SKNodeProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgUpdateMatrix: return SKNode_UpdateMatrix(object, cmp, wparm, lparm); // UpdateMatrix
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
ORCA_API struct ClassDesc _SKNode = {
	.ClassName = "SKNode",
	.DefaultName = "SKNode",
	.ContentType = "SKNode",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node, 0 },
	.ClassID = ID_SKNode,
	.ClassSize = sizeof(struct SKNode),
	.Properties = SKNodeProperties,
	.MessageTypes = SKNodeMessageTypes,
	.ObjProc = SKNodeProc,
	.Defaults = &SKNodeDefaults,
	.NumProperties = kSKNodeNumProperties,
	.NumMessageTypes = kSKNodeNumMessageTypes,
};

LRESULT SKScene_UpdateMatrix(struct Object*, struct SKScene*, wParam_t, UpdateMatrixMsgPtr);

static struct MessageType SKSceneMessageTypes[kSKSceneNumMessageTypes] = {	
};

static struct PropertyType const SKSceneProperties[kSKSceneNumProperties] = {
};
static struct SKScene SKSceneDefaults = {
};
LRESULT SKSceneProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgUpdateMatrix: return SKScene_UpdateMatrix(object, cmp, wparm, lparm); // UpdateMatrix
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
ORCA_API struct ClassDesc _SKScene = {
	.ClassName = "SKScene",
	.DefaultName = "SKScene",
	.ContentType = "SKScene",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_SKNode, 0 },
	.ClassID = ID_SKScene,
	.ClassSize = sizeof(struct SKScene),
	.Properties = SKSceneProperties,
	.MessageTypes = SKSceneMessageTypes,
	.ObjProc = SKSceneProc,
	.Defaults = &SKSceneDefaults,
	.NumProperties = kSKSceneNumProperties,
	.NumMessageTypes = kSKSceneNumMessageTypes,
};

LRESULT SKSpriteNode_Render(struct Object*, struct SKSpriteNode*, wParam_t, RenderMsgPtr);

static struct MessageType SKSpriteNodeMessageTypes[kSKSpriteNodeNumMessageTypes] = {	
};

static struct PropertyType const SKSpriteNodeProperties[kSKSpriteNodeNumProperties] = {
	DECL(0x41e389fd, SKSpriteNode, Animation, Animation, kDataTypeObject, .TypeString = "SpriteAnimation"), // SKSpriteNode.Animation
	DECL(0x8831f0dd, SKSpriteNode, Animation2, Animation2, kDataTypeObject, .TypeString = "SpriteAnimation"), // SKSpriteNode.Animation2
	DECL(0x590ca79a, SKSpriteNode, Image, Image, kDataTypeObject, .TypeString = "Texture"), // SKSpriteNode.Image
	DECL(0xa9f98a53, SKSpriteNode, FreezeFrame, FreezeFrame, kDataTypeInt), // SKSpriteNode.FreezeFrame
	DECL(0x0038792b, SKSpriteNode, BlendMode, BlendMode, kDataTypeEnum, .TypeString = "AlphaAutomatic,Opaque,Alpha,Additive,PremultipliedAlpha,MixedAlpha", .EnumValues = _BlendMode), // SKSpriteNode.BlendMode
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
		case kMsgRender: return SKSpriteNode_Render(object, cmp, wparm, lparm); // Render
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
ORCA_API struct ClassDesc _SKSpriteNode = {
	.ClassName = "SKSpriteNode",
	.DefaultName = "SKSpriteNode",
	.ContentType = "SKSpriteNode",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_SKNode, 0 },
	.ClassID = ID_SKSpriteNode,
	.ClassSize = sizeof(struct SKSpriteNode),
	.Properties = SKSpriteNodeProperties,
	.MessageTypes = SKSpriteNodeMessageTypes,
	.ObjProc = SKSpriteNodeProc,
	.Defaults = &SKSpriteNodeDefaults,
	.NumProperties = kSKSpriteNodeNumProperties,
	.NumMessageTypes = kSKSpriteNodeNumMessageTypes,
};

LRESULT SKLabelNode_Render(struct Object*, struct SKLabelNode*, wParam_t, RenderMsgPtr);
LRESULT SKLabelNode_Create(struct Object*, struct SKLabelNode*, wParam_t, CreateMsgPtr);

static struct MessageType SKLabelNodeMessageTypes[kSKLabelNodeNumMessageTypes] = {	
};

static struct PropertyType const SKLabelNodeProperties[kSKLabelNodeNumProperties] = {
	DECL(0xe5b43cf8, SKLabelNode, Color, Color, kDataTypeColor), // SKLabelNode.Color
};
static struct SKLabelNode SKLabelNodeDefaults = {
};
LRESULT SKLabelNodeProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgRender: return SKLabelNode_Render(object, cmp, wparm, lparm); // Render
		case kMsgCreate: return SKLabelNode_Create(object, cmp, wparm, lparm); // Create
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
ORCA_API struct ClassDesc _SKLabelNode = {
	.ClassName = "SKLabelNode",
	.DefaultName = "SKLabelNode",
	.ContentType = "SKLabelNode",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_SKNode, ID_TextBlockConcept, 0 },
	.ClassID = ID_SKLabelNode,
	.ClassSize = sizeof(struct SKLabelNode),
	.Properties = SKLabelNodeProperties,
	.MessageTypes = SKLabelNodeMessageTypes,
	.ObjProc = SKLabelNodeProc,
	.Defaults = &SKLabelNodeDefaults,
	.NumProperties = kSKLabelNodeNumProperties,
	.NumMessageTypes = kSKLabelNodeNumMessageTypes,
};

LRESULT SKView_ForegroundContent(struct Object*, struct SKView*, wParam_t, ForegroundContentMsgPtr);

static struct MessageType SKViewMessageTypes[kSKViewNumMessageTypes] = {	
};

static struct PropertyType const SKViewProperties[kSKViewNumProperties] = {
	DECL(0x499d2ae6, SKView, ReferenceWidth, ReferenceWidth, kDataTypeFloat), // SKView.ReferenceWidth
	DECL(0xf011cff9, SKView, ReferenceHeight, ReferenceHeight, kDataTypeFloat), // SKView.ReferenceHeight
	DECL(0xc89b38b3, SKView, Scene, Scene, kDataTypeString), // SKView.Scene
};
static struct SKView SKViewDefaults = {
};
LRESULT SKViewProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgForegroundContent: return SKView_ForegroundContent(object, cmp, wparm, lparm); // ForegroundContent
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
ORCA_API struct ClassDesc _SKView = {
	.ClassName = "SKView",
	.DefaultName = "SKView",
	.ContentType = "SKView",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, 0 },
	.ClassID = ID_SKView,
	.ClassSize = sizeof(struct SKView),
	.Properties = SKViewProperties,
	.MessageTypes = SKViewMessageTypes,
	.ObjProc = SKViewProc,
	.Defaults = &SKViewDefaults,
	.NumProperties = kSKViewNumProperties,
	.NumMessageTypes = kSKViewNumMessageTypes,
};



ORCA_API int luaopen_orca_SpriteKit(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ NULL, NULL } 
	}));
	void on_spritekit_module_registered(lua_State *L);
	on_spritekit_module_registered(L);
	lua_setfield(L, ((void)luaopen_orca_SpriteFrame(L), -2), "SpriteFrame");
	lua_setfield(L, ((void)lua_pushclass(L, &_SpriteAnimation), -2), "SpriteAnimation");
	lua_setfield(L, ((void)lua_pushclass(L, &_SKNode), -2), "SKNode");
	lua_setfield(L, ((void)lua_pushclass(L, &_SKScene), -2), "SKScene");
	lua_setfield(L, ((void)lua_pushclass(L, &_SKSpriteNode), -2), "SKSpriteNode");
	lua_setfield(L, ((void)lua_pushclass(L, &_SKLabelNode), -2), "SKLabelNode");
	lua_setfield(L, ((void)lua_pushclass(L, &_SKView), -2), "SKView");
	return 1;
}
