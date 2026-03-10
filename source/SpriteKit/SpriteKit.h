#ifndef __SPRITEKIT_H__
#define __SPRITEKIT_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;



#include <include/renderer.h>
#include <source/core/core.h>
#include <source/UIKit/UIKit.h>

#include "SpriteKit_properties.h"
#include "../renderer/api/renderer.h"



typedef struct SpriteFrame SpriteFrame_t, *lpSpriteFrame_t;
typedef struct SpriteFrame const cSpriteFrame_t, *lpcSpriteFrame_t;



/** SpriteFrame struct */
struct SpriteFrame {
	struct rect Rect;
	struct rect UvRect;
};
ORCA_API void luaX_pushSpriteFrame(lua_State *L, struct SpriteFrame const* SpriteFrame);
ORCA_API struct SpriteFrame* luaX_checkSpriteFrame(lua_State *L, int idx);

/** SpriteAnimation component */
typedef struct SpriteAnimation SpriteAnimation_t, *SpriteAnimationPtr, *lpSpriteAnimation_t;
typedef struct SpriteAnimation const *SpriteAnimationCPtr, *lpcSpriteAnimation_t;
struct SpriteAnimation {
	struct Texture* Image;
	float Framerate;
	struct SpriteFrame Frames;
};
ORCA_API void luaX_pushSpriteAnimation(lua_State *L, struct SpriteAnimation const* SpriteAnimation);
ORCA_API struct SpriteAnimation* luaX_checkSpriteAnimation(lua_State *L, int idx);
/** SKNode component */
typedef struct SKNode SKNode_t, *SKNodePtr, *lpSKNode_t;
typedef struct SKNode const *SKNodeCPtr, *lpcSKNode_t;
struct SKNode {
	struct vec2 Position;
	struct vec2 Size;
	struct vec2 Anchor;
	struct mat4 Matrix;
	float _opacity;
};
ORCA_API void luaX_pushSKNode(lua_State *L, struct SKNode const* SKNode);
ORCA_API struct SKNode* luaX_checkSKNode(lua_State *L, int idx);
/** SKScene component */
typedef struct SKScene SKScene_t, *SKScenePtr, *lpSKScene_t;
typedef struct SKScene const *SKSceneCPtr, *lpcSKScene_t;
struct SKScene {
};
ORCA_API void luaX_pushSKScene(lua_State *L, struct SKScene const* SKScene);
ORCA_API struct SKScene* luaX_checkSKScene(lua_State *L, int idx);
/** SKSpriteNode component */
typedef struct SKSpriteNode SKSpriteNode_t, *SKSpriteNodePtr, *lpSKSpriteNode_t;
typedef struct SKSpriteNode const *SKSpriteNodeCPtr, *lpcSKSpriteNode_t;
struct SKSpriteNode {
	struct SpriteAnimation* Animation;
	struct SpriteAnimation* Animation2;
	struct Texture* Image;
	int32_t FreezeFrame;
	enum BlendMode BlendMode;
	struct rect UvRect;
};
ORCA_API void luaX_pushSKSpriteNode(lua_State *L, struct SKSpriteNode const* SKSpriteNode);
ORCA_API struct SKSpriteNode* luaX_checkSKSpriteNode(lua_State *L, int idx);
/** SKLabelNode component */
typedef struct SKLabelNode SKLabelNode_t, *SKLabelNodePtr, *lpSKLabelNode_t;
typedef struct SKLabelNode const *SKLabelNodeCPtr, *lpcSKLabelNode_t;
struct SKLabelNode {
	struct color Color;
	struct SKNode* _node;
};
ORCA_API void luaX_pushSKLabelNode(lua_State *L, struct SKLabelNode const* SKLabelNode);
ORCA_API struct SKLabelNode* luaX_checkSKLabelNode(lua_State *L, int idx);
/** SKView component */
typedef struct SKView SKView_t, *SKViewPtr, *lpSKView_t;
typedef struct SKView const *SKViewCPtr, *lpcSKView_t;
struct SKView {
	float ReferenceWidth;
	float ReferenceHeight;
	fixedString_t Scene;
};
ORCA_API void luaX_pushSKView(lua_State *L, struct SKView const* SKView);
ORCA_API struct SKView* luaX_checkSKView(lua_State *L, int idx);

#endif