#ifndef __SPRITEKIT_PROPERTIES_H__
#define __SPRITEKIT_PROPERTIES_H__
// SpriteAnimation
#define ID_SpriteAnimation 0x261d60ba
#define GetSpriteAnimation(_P) ((struct SpriteAnimation*)((_P)?OBJ_GetComponent(_P,ID_SpriteAnimation):NULL))
#define SpriteAnimation_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SpriteAnimation,sizeof(struct SpriteAnimation),_N)
#define ID_SpriteAnimation_Image 0xb710ed59 // SpriteAnimation.Image
#define ID_SpriteAnimation_Framerate 0xc51f93cb // SpriteAnimation.Framerate
#define ID_SpriteAnimation_Frames 0xcbc4645e // SpriteAnimation.Frames
enum SpriteAnimationProperties {
	kSpriteAnimationImage,
	kSpriteAnimationFramerate,
	kSpriteAnimationFrames,
	kSpriteAnimationNumProperties	
};
// SKNode
#define ID_SKNode 0x819821fb
#define GetSKNode(_P) ((struct SKNode*)((_P)?OBJ_GetComponent(_P,ID_SKNode):NULL))
#define SKNode_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SKNode,sizeof(struct SKNode),_N)
#define ID_SKNode_Position 0xa99d4e4c // SKNode.Position
#define ID_SKNode_Size 0xeaed98c6 // SKNode.Size
#define ID_SKNode_Anchor 0xf86abfb6 // SKNode.Anchor
enum SKNodeProperties {
	kSKNodePosition,
	kSKNodeSize,
	kSKNodeAnchor,
	kSKNodeNumProperties	
};
// SKScene
#define ID_SKScene 0xb8c15a09
#define GetSKScene(_P) ((struct SKScene*)((_P)?OBJ_GetComponent(_P,ID_SKScene):NULL))
#define SKScene_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SKScene,sizeof(struct SKScene),_N)
enum SKSceneProperties {
	kSKSceneNumProperties	
};
// SKSpriteNode
#define ID_SKSpriteNode 0x4f1c372e
#define GetSKSpriteNode(_P) ((struct SKSpriteNode*)((_P)?OBJ_GetComponent(_P,ID_SKSpriteNode):NULL))
#define SKSpriteNode_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SKSpriteNode,sizeof(struct SKSpriteNode),_N)
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
	kSKSpriteNodeNumProperties	
};
// SKLabelNode
#define ID_SKLabelNode 0x01306533
#define GetSKLabelNode(_P) ((struct SKLabelNode*)((_P)?OBJ_GetComponent(_P,ID_SKLabelNode):NULL))
#define SKLabelNode_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SKLabelNode,sizeof(struct SKLabelNode),_N)
#define ID_SKLabelNode_Color 0x84bf37d6 // SKLabelNode.Color
enum SKLabelNodeProperties {
	kSKLabelNodeColor,
	kSKLabelNodeNumProperties	
};
// SKView
#define ID_SKView 0x7c02453e
#define GetSKView(_P) ((struct SKView*)((_P)?OBJ_GetComponent(_P,ID_SKView):NULL))
#define SKView_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SKView,sizeof(struct SKView),_N)
#define ID_SKView_ReferenceWidth 0x5c81a8f7 // SKView.ReferenceWidth
#define ID_SKView_ReferenceHeight 0x4b5832ba // SKView.ReferenceHeight
#define ID_SKView_Scene 0x71075038 // SKView.Scene
enum SKViewProperties {
	kSKViewReferenceWidth,
	kSKViewReferenceHeight,
	kSKViewScene,
	kSKViewNumProperties	
};
#endif