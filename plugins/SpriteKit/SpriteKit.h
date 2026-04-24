// Auto-generated from SpriteKit.xml by tools/templates/header.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __SPRITEKIT_H__
#define __SPRITEKIT_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;


#include <include/renderer.h>
#include <source/core/core.h>
#include <plugins/UIKit/UIKit.h>

#include "SpriteKit_properties.h"
#include "../../source/renderer/api/renderer.h"

typedef struct SKNode_RenderEventArgs SKNode_RenderMsg_t,* SKNode_RenderMsgPtr;




/** SpriteFrame struct */
struct SpriteFrame {
	struct rect Rect;
	struct rect UvRect;
};
ORCA_API void luaX_pushSpriteFrame(lua_State *L, struct SpriteFrame const* SpriteFrame);
ORCA_API struct SpriteFrame* luaX_checkSpriteFrame(lua_State *L, int idx);


/** SKNode_RenderEventArgs struct */
struct SKNode_RenderEventArgs {
	struct ViewDef* ViewDef; ///< The view definition for rendering
};
ORCA_API void luaX_pushSKNode_RenderEventArgs(lua_State *L, struct SKNode_RenderEventArgs const* data);
ORCA_API struct SKNode_RenderEventArgs* luaX_checkSKNode_RenderEventArgs(lua_State *L, int idx);

/** SpriteAnimation component */
typedef struct SpriteAnimation SpriteAnimation_t, *SpriteAnimationPtr, *lpSpriteAnimation_t;
typedef struct SpriteAnimation const *SpriteAnimationCPtr, *lpcSpriteAnimation_t;
struct SpriteAnimation {
	struct Texture* Image; ///< Spritesheet texture
	float Framerate; ///< Frames per second
	struct SpriteFrame* Frames; ///< Variable-length frame data array
	int32_t NumFrames;
	uint32_t const* Palette;
};
ORCA_API void luaX_pushSpriteAnimation(lua_State *L, struct SpriteAnimation const* SpriteAnimation);
ORCA_API struct SpriteAnimation* luaX_checkSpriteAnimation(lua_State *L, int idx);

/// @brief Base 2D sprite node with transform and matrix
/** SKNode component */
typedef struct SKNode SKNode_t, *SKNodePtr, *lpSKNode_t;
typedef struct SKNode const *SKNodeCPtr, *lpcSKNode_t;
struct SKNode {
	struct vec2 Position; ///< Position of SKNode in space of SKScene
	struct vec2 Size; ///< Size of SKNode. If not specified, defaults to the size of the associated sprite or animation frame
	struct vec2 Anchor; ///< Anchor point for positioning
	struct mat4 Matrix; ///< Final combined transformation matrix (computed automatically)
	float _opacity;
	event_t Render;
};
ORCA_API void luaX_pushSKNode(lua_State *L, struct SKNode const* SKNode);
ORCA_API struct SKNode* luaX_checkSKNode(lua_State *L, int idx);

/// @brief Root container for a 2D sprite scene
/** SKScene component */
typedef struct SKScene SKScene_t, *SKScenePtr, *lpSKScene_t;
typedef struct SKScene const *SKSceneCPtr, *lpcSKScene_t;
struct SKScene {
};
ORCA_API void luaX_pushSKScene(lua_State *L, struct SKScene const* SKScene);
ORCA_API struct SKScene* luaX_checkSKScene(lua_State *L, int idx);

/// @brief 2D sprite node that renders an image or animation
/** SKSpriteNode component */
typedef struct SKSpriteNode SKSpriteNode_t, *SKSpriteNodePtr, *lpSKSpriteNode_t;
typedef struct SKSpriteNode const *SKSpriteNodeCPtr, *lpcSKSpriteNode_t;
struct SKSpriteNode {
	struct SpriteAnimation* Animation; ///< Primary sprite animation from a sprite atlas
	struct SpriteAnimation* Animation2; ///< Secondary sprite animation (e.g., shadow layer)
	struct Texture* Image; ///< Static image texture to display
	int32_t FreezeFrame; ///< Freeze animation at a specific frame index (-1 for normal playback)
	enum BlendMode BlendMode; ///< Blend mode for rendering
	struct rect UvRect; ///< UV texture coordinates for sprite atlas sampling
};
ORCA_API void luaX_pushSKSpriteNode(lua_State *L, struct SKSpriteNode const* SKSpriteNode);
ORCA_API struct SKSpriteNode* luaX_checkSKSpriteNode(lua_State *L, int idx);

/// @brief 2D text label node for displaying text in sprite space
/** SKLabelNode component */
typedef struct SKLabelNode SKLabelNode_t, *SKLabelNodePtr, *lpSKLabelNode_t;
typedef struct SKLabelNode const *SKLabelNodeCPtr, *lpcSKLabelNode_t;
struct SKLabelNode {
	struct color Color; ///< Color of the text
	struct SKNode* _node;
};
ORCA_API void luaX_pushSKLabelNode(lua_State *L, struct SKLabelNode const* SKLabelNode);
ORCA_API struct SKLabelNode* luaX_checkSKLabelNode(lua_State *L, int idx);

/// @brief 2D viewport that renders an SKScene into the UI hierarchy
/** SKView component */
typedef struct SKView SKView_t, *SKViewPtr, *lpSKView_t;
typedef struct SKView const *SKViewCPtr, *lpcSKView_t;
struct SKView {
	float ReferenceWidth; ///< Reference width for the sprite coordinate system
	float ReferenceHeight; ///< Reference height for the sprite coordinate system
	const char* Scene; ///< Name of SKScene to render
};
ORCA_API void luaX_pushSKView(lua_State *L, struct SKView const* SKView);
ORCA_API struct SKView* luaX_checkSKView(lua_State *L, int idx);

#endif