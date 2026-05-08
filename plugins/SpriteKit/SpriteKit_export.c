// Auto-generated from SpriteKit.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>
#include <include/codegen.h>

#include "SpriteKit.h"


extern const char *_BlendMode[];
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);
static struct PropertyType _SpriteFrame[] = {
	DECL_STRUCT_TYPE(0x6b109927, SpriteFrame, Rect, Rect, Rectangle), // SpriteFrame.Rect
	DECL_FLOAT(0x0a21a2ed, SpriteFrame, RectX, Rect.x), // SpriteFrame.RectX
	DECL_FLOAT(0x0921a15a, SpriteFrame, RectY, Rect.y), // SpriteFrame.RectY
	DECL_FLOAT(0x25c57ce9, SpriteFrame, RectWidth, Rect.width), // SpriteFrame.RectWidth
	DECL_FLOAT(0x68097dd8, SpriteFrame, RectHeight, Rect.height), // SpriteFrame.RectHeight
	DECL_STRUCT_TYPE(0xae3d25c0, SpriteFrame, UvRect, UvRect, Rectangle), // SpriteFrame.UvRect
	DECL_FLOAT(0xe2422e48, SpriteFrame, UvRectX, UvRect.x), // SpriteFrame.UvRectX
	DECL_FLOAT(0xe3422fdb, SpriteFrame, UvRectY, UvRect.y), // SpriteFrame.UvRectY
	DECL_FLOAT(0xec9d6ecc, SpriteFrame, UvRectWidth, UvRect.width), // SpriteFrame.UvRectWidth
	DECL_FLOAT(0xdbdb8abb, SpriteFrame, UvRectHeight, UvRect.height), // SpriteFrame.UvRectHeight
};
static luaL_Reg _SpriteFrame_Methods[] = {
	{ NULL, NULL }
};

STRUCT(SpriteFrame, SpriteFrame);

static luaL_Reg _SKNode_RenderEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _SKNode_RenderEventArgs[] = {
	DECL_STRUCT_TYPE(0xce9ab61f, SKNode_RenderEventArgs, ViewDef, ViewDef, ViewDef), // SKNode_RenderEventArgs.ViewDef
};

STRUCT(SKNode_RenderEventArgs, SKNode_RenderEventArgs);
COMPONENT_PROPERTIES(SpriteAnimation) = {
	DECL_OBJECT(0x590ca79a, SpriteAnimation, Image, Image, Texture), // SpriteAnimation.Image
	DECL_FLOAT(0xbebf2a84, SpriteAnimation, Framerate, Framerate), // SpriteAnimation.Framerate
	ARRAY_DECL_STRUCT_TYPE(0xf03e266f, SpriteAnimation, Frames, Frames, SpriteFrame), // SpriteAnimation.Frames
	DECL_INT(0x32caacb1, SpriteAnimation, NumFrames, NumFrames), // SpriteAnimation.NumFrames
};
COMPONENT_DEFAULTS(SpriteAnimation) = {
};
COMPONENT_PROC_BEGIN(SpriteAnimation)
COMPONENT_PROC_END()
void luaX_pushSpriteAnimation(lua_State *L, struct SpriteAnimation const* SpriteAnimation) {
	luaX_pushObject(L, CMP_GetObject(SpriteAnimation));
}
struct SpriteAnimation* luaX_checkSpriteAnimation(lua_State *L, int idx) {
	return GetSpriteAnimation(luaX_checkObject(L, idx));
}
REGISTER_CLASS(SpriteAnimation, 0);
HANDLER(SKNode, Node, UpdateMatrix);
COMPONENT_PROPERTIES(SKNode) = {
	DECL_STRUCT_TYPE(0xe27f342a, SKNode, Position, Position, Vector2D), // SKNode.Position
	DECL_STRUCT_TYPE(0xa6478e7c, SKNode, Size, Size, Vector2D), // SKNode.Size
	DECL_STRUCT_TYPE(0xb54055d4, SKNode, Anchor, Anchor, Vector2D), // SKNode.Anchor
	DECL_EVENT(0x350cf42d, SKNode, Render), // SKNode.Render
};
COMPONENT_DEFAULTS(SKNode) = {
};
COMPONENT_PROC_BEGIN(SKNode)
		DISPATCH(SKNode, Node, UpdateMatrix) // Node.UpdateMatrix
COMPONENT_PROC_END()
void luaX_pushSKNode(lua_State *L, struct SKNode const* SKNode) {
	luaX_pushObject(L, CMP_GetObject(SKNode));
}
struct SKNode* luaX_checkSKNode(lua_State *L, int idx) {
	return GetSKNode(luaX_checkObject(L, idx));
}
#define ID_Node 0x3468032d
REGISTER_CLASS(SKNode, ID_Node, 0);
HANDLER(SKScene, Node, UpdateMatrix);
COMPONENT_PROPERTIES(SKScene) = {
};
COMPONENT_DEFAULTS(SKScene) = {
};
COMPONENT_PROC_BEGIN(SKScene)
		DISPATCH(SKScene, Node, UpdateMatrix) // Node.UpdateMatrix
COMPONENT_PROC_END()
void luaX_pushSKScene(lua_State *L, struct SKScene const* SKScene) {
	luaX_pushObject(L, CMP_GetObject(SKScene));
}
struct SKScene* luaX_checkSKScene(lua_State *L, int idx) {
	return GetSKScene(luaX_checkObject(L, idx));
}
#define ID_SKNode 0x819821fb
REGISTER_CLASS(SKScene, ID_SKNode, 0);
HANDLER(SKSpriteNode, SKNode, Render);
COMPONENT_PROPERTIES(SKSpriteNode) = {
	DECL_OBJECT(0x41e389fd, SKSpriteNode, Animation, Animation, SpriteAnimation), // SKSpriteNode.Animation
	DECL_OBJECT(0x8831f0dd, SKSpriteNode, Animation2, Animation2, SpriteAnimation), // SKSpriteNode.Animation2
	DECL_OBJECT(0x590ca79a, SKSpriteNode, Image, Image, Texture), // SKSpriteNode.Image
	DECL_INT(0xa9f98a53, SKSpriteNode, FreezeFrame, FreezeFrame), // SKSpriteNode.FreezeFrame
	DECL_ENUM(0x0038792b, SKSpriteNode, BlendMode, BlendMode, BlendMode), // SKSpriteNode.BlendMode
	DECL_STRUCT_TYPE(0xae3d25c0, SKSpriteNode, UvRect, UvRect, Rectangle), // SKSpriteNode.UvRect
	DECL_FLOAT(0xe2422e48, SKSpriteNode, UvRectX, UvRect.x), // SKSpriteNode.UvRectX
	DECL_FLOAT(0xe3422fdb, SKSpriteNode, UvRectY, UvRect.y), // SKSpriteNode.UvRectY
	DECL_FLOAT(0xec9d6ecc, SKSpriteNode, UvRectWidth, UvRect.width), // SKSpriteNode.UvRectWidth
	DECL_FLOAT(0xdbdb8abb, SKSpriteNode, UvRectHeight, UvRect.height), // SKSpriteNode.UvRectHeight
};
COMPONENT_DEFAULTS(SKSpriteNode) = {
		
  .FreezeFrame = -1,
		
  .BlendMode = kBlendModeAlphaAutomatic,
};
COMPONENT_PROC_BEGIN(SKSpriteNode)
		DISPATCH(SKSpriteNode, SKNode, Render) // SKNode.Render
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(SKLabelNode) = {
	DECL_COLOR(0xe5b43cf8, SKLabelNode, Color, Color), // SKLabelNode.Color
};
COMPONENT_DEFAULTS(SKLabelNode) = {
};
COMPONENT_PROC_BEGIN(SKLabelNode)
		DISPATCH(SKLabelNode, SKNode, Render) // SKNode.Render
		DISPATCH(SKLabelNode, Object, Create) // Object.Create
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(SKView) = {
	DECL_FLOAT(0x499d2ae6, SKView, ReferenceWidth, ReferenceWidth), // SKView.ReferenceWidth
	DECL_FLOAT(0xf011cff9, SKView, ReferenceHeight, ReferenceHeight), // SKView.ReferenceHeight
	DECL_STRING(0xc89b38b3, SKView, Scene, Scene), // SKView.Scene
};
COMPONENT_DEFAULTS(SKView) = {
};
COMPONENT_PROC_BEGIN(SKView)
		DISPATCH(SKView, Node2D, ForegroundContent) // Node2D.ForegroundContent
COMPONENT_PROC_END()
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
