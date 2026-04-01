// Auto-generated from SpriteKit.xml by tools/templates/properties.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __SPRITEKIT_PROPERTIES_H__
#define __SPRITEKIT_PROPERTIES_H__

// SpriteAnimation
#define ID_SpriteAnimation 0x261d60ba
#define GetSpriteAnimation(_P) ((struct SpriteAnimation*)((_P)?OBJ_GetComponent(_P,ID_SpriteAnimation):NULL))
#define SpriteAnimation_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SpriteAnimation,sizeof(struct SpriteAnimation),_N)
#define SpriteAnimation_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_SpriteAnimation,_N)
#define ID_SpriteAnimation_Image 0xb710ed59 // SpriteAnimation.Image
#define ID_SpriteAnimation_Framerate 0xc51f93cb // SpriteAnimation.Framerate
#define ID_SpriteAnimation_Frames 0xcbc4645e // SpriteAnimation.Frames
#define ID_SpriteAnimation_NumFrames 0x37cc69de // SpriteAnimation.NumFrames
#define kSpriteAnimationNumMessageTypes 0
#define kSpriteAnimationNumProperties 4
enum SpriteAnimationProperties {
	kSpriteAnimationImage,
	kSpriteAnimationFramerate,
	kSpriteAnimationFrames,
	kSpriteAnimationNumFrames,
};
// SKNode
#define ID_SKNode 0x819821fb
#define GetSKNode(_P) ((struct SKNode*)((_P)?OBJ_GetComponent(_P,ID_SKNode):NULL))
#define SKNode_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SKNode,sizeof(struct SKNode),_N)
#define SKNode_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_SKNode,_N)
#define ID_SKNode_Position 0xa99d4e4c // SKNode.Position
#define ID_SKNode_Size 0xeaed98c6 // SKNode.Size
#define ID_SKNode_Anchor 0xf86abfb6 // SKNode.Anchor
#define kSKNodeNumMessageTypes 1
enum SKNodeMessages {
	kSKNodeRender,
};
#define ID_SKNode_Render 0x450c46e3 // SKNode.Render
#define kSKNodeNumProperties 3
enum SKNodeProperties {
	kSKNodePosition,
	kSKNodeSize,
	kSKNodeAnchor,
};
// SKScene
#define ID_SKScene 0xb8c15a09
#define GetSKScene(_P) ((struct SKScene*)((_P)?OBJ_GetComponent(_P,ID_SKScene):NULL))
#define SKScene_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SKScene,sizeof(struct SKScene),_N)
#define SKScene_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_SKScene,_N)
#define kSKSceneNumMessageTypes 0
#define kSKSceneNumProperties 0
// SKSpriteNode
#define ID_SKSpriteNode 0x4f1c372e
#define GetSKSpriteNode(_P) ((struct SKSpriteNode*)((_P)?OBJ_GetComponent(_P,ID_SKSpriteNode):NULL))
#define SKSpriteNode_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SKSpriteNode,sizeof(struct SKSpriteNode),_N)
#define SKSpriteNode_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_SKSpriteNode,_N)
#define ID_SKSpriteNode_Animation 0xdfc01dce // SKSpriteNode.Animation
#define ID_SKSpriteNode_Animation2 0x376f33b4 // SKSpriteNode.Animation2
#define ID_SKSpriteNode_Image 0x7d43a6bd // SKSpriteNode.Image
#define ID_SKSpriteNode_FreezeFrame 0x371f13e0 // SKSpriteNode.FreezeFrame
#define ID_SKSpriteNode_BlendMode 0x240fc724 // SKSpriteNode.BlendMode
#define ID_SKSpriteNode_UvRect 0xc27a0d8d // SKSpriteNode.UvRect
#define ID_SKSpriteNode_UvRectX 0xfb23c64f // SKSpriteNode.UvRectX
#define ID_SKSpriteNode_UvRectY 0xfa23c4bc // SKSpriteNode.UvRectY
#define ID_SKSpriteNode_UvRectWidth 0xfcdbf497 // SKSpriteNode.UvRectWidth
#define ID_SKSpriteNode_UvRectHeight 0xea8ad8da // SKSpriteNode.UvRectHeight
#define kSKSpriteNodeNumMessageTypes 0
#define kSKSpriteNodeNumProperties 10
enum SKSpriteNodeProperties {
	kSKSpriteNodeAnimation,
	kSKSpriteNodeAnimation2,
	kSKSpriteNodeImage,
	kSKSpriteNodeFreezeFrame,
	kSKSpriteNodeBlendMode,
	kSKSpriteNodeUvRect,
	kSKSpriteNodeUvRectX,
	kSKSpriteNodeUvRectY,
	kSKSpriteNodeUvRectWidth,
	kSKSpriteNodeUvRectHeight,
};
// SKLabelNode
#define ID_SKLabelNode 0x01306533
#define GetSKLabelNode(_P) ((struct SKLabelNode*)((_P)?OBJ_GetComponent(_P,ID_SKLabelNode):NULL))
#define SKLabelNode_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SKLabelNode,sizeof(struct SKLabelNode),_N)
#define SKLabelNode_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_SKLabelNode,_N)
#define ID_SKLabelNode_Color 0x84bf37d6 // SKLabelNode.Color
#define kSKLabelNodeNumMessageTypes 0
#define kSKLabelNodeNumProperties 1
enum SKLabelNodeProperties {
	kSKLabelNodeColor,
};
// SKView
#define ID_SKView 0x7c02453e
#define GetSKView(_P) ((struct SKView*)((_P)?OBJ_GetComponent(_P,ID_SKView):NULL))
#define SKView_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SKView,sizeof(struct SKView),_N)
#define SKView_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_SKView,_N)
#define ID_SKView_ReferenceWidth 0x5c81a8f7 // SKView.ReferenceWidth
#define ID_SKView_ReferenceHeight 0x4b5832ba // SKView.ReferenceHeight
#define ID_SKView_Scene 0x71075038 // SKView.Scene
#define kSKViewNumMessageTypes 0
#define kSKViewNumProperties 3
enum SKViewProperties {
	kSKViewReferenceWidth,
	kSKViewReferenceHeight,
	kSKViewScene,
};


#define ID_SpriteFrame 0xad475fd9
#define ID_SpriteFrame_Rect 0xa1f930f7 // SpriteFrame.Rect
#define ID_SpriteFrame_RectX 0xaa47a37d // SpriteFrame.RectX
#define ID_SpriteFrame_RectY 0xa947a1ea // SpriteFrame.RectY
#define ID_SpriteFrame_RectWidth 0x2e93ded9 // SpriteFrame.RectWidth
#define ID_SpriteFrame_RectHeight 0x42515548 // SpriteFrame.RectHeight
#define ID_SpriteFrame_UvRect 0x85675610 // SpriteFrame.UvRect
#define ID_SpriteFrame_UvRectX 0x49acd358 // SpriteFrame.UvRectX
#define ID_SpriteFrame_UvRectY 0x4aacd4eb // SpriteFrame.UvRectY
#define ID_SpriteFrame_UvRectWidth 0x9b750efc // SpriteFrame.UvRectWidth
#define ID_SpriteFrame_UvRectHeight 0x3fb6116b // SpriteFrame.UvRectHeight
	

#define ID_Render 0x350cf42d
#define ID_RenderMsgArgs_ViewDef 0xbebba0a1 // RenderMsgArgs.ViewDef
	

#endif