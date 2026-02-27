#define ID_SpriteAnimation 0x261d60ba
#define GetSpriteAnimation(_P)((struct SpriteAnimation*)((_P)?OBJ_GetComponent(_P,ID_SpriteAnimation):NULL))
#define SpriteAnimation_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_SpriteAnimation,sizeof(struct SpriteAnimation),_N)
enum SpriteAnimationProperties {
	kSpriteAnimationImage,
	kSpriteAnimationFramerate,
	kSpriteAnimationNumFrames,
	kSpriteAnimationNumProperties
};

#define ID_SpriteAnimation_Image 0xb710ed59
#define ID_SpriteAnimation_Framerate 0xc51f93cb
#define ID_SpriteAnimation_NumFrames 0x37cc69de
#define ID_SKNode 0x819821fb
#define GetSKNode(_P)((struct SKNode*)((_P)?OBJ_GetComponent(_P,ID_SKNode):NULL))
#define SKNode_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_SKNode,sizeof(struct SKNode),_N)
enum SKNodeProperties {
	kSKNodePosition,
	kSKNodeSize,
	kSKNodeAnchor,
	kSKNodeNumProperties
};

#define ID_SKNode_Position 0xa99d4e4c
#define ID_SKNode_Size 0xeaed98c6
#define ID_SKNode_Anchor 0xf86abfb6
#define ID_SKScene 0xb8c15a09
#define GetSKScene(_P)((struct SKScene*)((_P)?OBJ_GetComponent(_P,ID_SKScene):NULL))
#define SKScene_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_SKScene,sizeof(struct SKScene),_N)
enum SKSceneProperties {
	kSKSceneNumProperties
};

#define ID_SKSpriteNode 0x4f1c372e
#define GetSKSpriteNode(_P)((struct SKSpriteNode*)((_P)?OBJ_GetComponent(_P,ID_SKSpriteNode):NULL))
#define SKSpriteNode_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_SKSpriteNode,sizeof(struct SKSpriteNode),_N)
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

#define ID_SKSpriteNode_Animation 0xdfc01dce
#define ID_SKSpriteNode_Animation2 0x376f33b4
#define ID_SKSpriteNode_Image 0x7d43a6bd
#define ID_SKSpriteNode_FreezeFrame 0x371f13e0
#define ID_SKSpriteNode_BlendMode 0x240fc724
#define ID_SKSpriteNode_UvRect 0xc27a0d8d
#define ID_SKSpriteNode_UvRectX 0xfb23c64f
#define ID_SKSpriteNode_UvRectY 0xfa23c4bc
#define ID_SKSpriteNode_UvRectWidth 0xfcdbf497
#define ID_SKSpriteNode_UvRectHeight 0xea8ad8da
#define ID_SKLabelNode 0x01306533
#define GetSKLabelNode(_P)((struct SKLabelNode*)((_P)?OBJ_GetComponent(_P,ID_SKLabelNode):NULL))
#define SKLabelNode_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_SKLabelNode,sizeof(struct SKLabelNode),_N)
enum SKLabelNodeProperties {
	kSKLabelNodeColor,
	kSKLabelNodeNumProperties
};

#define ID_SKLabelNode_Color 0x84bf37d6
#define ID_SKView 0x7c02453e
#define GetSKView(_P)((struct SKView*)((_P)?OBJ_GetComponent(_P,ID_SKView):NULL))
#define SKView_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_SKView,sizeof(struct SKView),_N)
enum SKViewProperties {
	kSKViewReferenceWidth,
	kSKViewReferenceHeight,
	kSKViewScene,
	kSKViewNumProperties
};

#define ID_SKView_ReferenceWidth 0x5c81a8f7
#define ID_SKView_ReferenceHeight 0x4b5832ba
#define ID_SKView_Scene 0x71075038
