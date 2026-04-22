// Auto-generated from SpriteKit.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>
#include <include/codegen.h>

#include "SpriteKit.h"


extern const char *_BlendMode[];
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(lua_State *L, const char* str, struct PropertyType const* prop, void* struct_ptr);
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
