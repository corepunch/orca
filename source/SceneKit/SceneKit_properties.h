#define ID_Node3D 0xce61fe5a
#define GetNode3D(_P)((struct Node3D*)((_P)?OBJ_GetComponent(_P,ID_Node3D):NULL))
#define Node3D_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Node3D,sizeof(struct Node3D),_N)
enum Node3DProperties {
	kNode3DLayoutTransform,
	kNode3DLayoutTransformTranslation,
	kNode3DLayoutTransformRotation,
	kNode3DLayoutTransformScale,
	kNode3DRenderTransform,
	kNode3DRenderTransformTranslation,
	kNode3DRenderTransformRotation,
	kNode3DRenderTransformScale,
	kNode3DRenderTransformOrigin,
	kNode3DContentOffset,
	kNode3DMatrix,
	kNode3DCalculatedOffset,
	kNode3DNumProperties
};

#define ID_Node3D_LayoutTransform 0x3cc6155a
#define ID_Node3D_LayoutTransformTranslation 0x34443d45
#define ID_Node3D_LayoutTransformRotation 0xa00afee8
#define ID_Node3D_LayoutTransformScale 0xa3eeb830
#define ID_Node3D_RenderTransform 0xf2ecd3b8
#define ID_Node3D_RenderTransformTranslation 0x8d2c8bcb
#define ID_Node3D_RenderTransformRotation 0x0f3796be
#define ID_Node3D_RenderTransformScale 0x5714ba3a
#define ID_Node3D_RenderTransformOrigin 0xff3e393a
#define ID_Node3D_ContentOffset 0x660ce5d6
#define ID_Node3D_Matrix 0xffeed605
#define ID_Node3D_CalculatedOffset 0x7f8022a7
#define ID_Scene 0xc89b38b3
#define GetScene(_P)((struct Scene*)((_P)?OBJ_GetComponent(_P,ID_Scene):NULL))
#define Scene_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Scene,sizeof(struct Scene),_N)
enum SceneProperties {
	kSceneCamera,
	kScenePreviewCamera,
	kSceneHitTestCamera,
	kSceneRenderPass,
	kSceneBackgroundTimelineSequence,
	kSceneComposerOverride,
	kSceneEnvironmentTexture,
	kSceneNumProperties
};

#define ID_Scene_Camera 0x2856ba68
#define ID_Scene_PreviewCamera 0x9ee5ebda
#define ID_Scene_HitTestCamera 0x30217eeb
#define ID_Scene_RenderPass 0x01a192ae
#define ID_Scene_BackgroundTimelineSequence 0xb8ec9e4d
#define ID_Scene_ComposerOverride 0x7103adc1
#define ID_Scene_EnvironmentTexture 0x8feab57b
#define ID_Model3D 0xc56de5fd
#define GetModel3D(_P)((struct Model3D*)((_P)?OBJ_GetComponent(_P,ID_Model3D):NULL))
#define Model3D_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Model3D,sizeof(struct Model3D),_N)
enum Model3DProperties {
	kModel3DMesh,
	kModel3DMaterial,
	kModel3DNumProperties
};

#define ID_Model3D_Mesh 0xa983d6a0
#define ID_Model3D_Material 0x0c181d04
#define ID_PlaneMeshNode 0x660bed48
#define GetPlaneMeshNode(_P)((struct PlaneMeshNode*)((_P)?OBJ_GetComponent(_P,ID_PlaneMeshNode):NULL))
#define PlaneMeshNode_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_PlaneMeshNode,sizeof(struct PlaneMeshNode),_N)
enum PlaneMeshNodeProperties {
	kPlaneMeshNodePlaneWidth,
	kPlaneMeshNodePlaneHeight,
	kPlaneMeshNodePlaneInvertU,
	kPlaneMeshNodePlaneInvertV,
	kPlaneMeshNodeGenerateTangents,
	kPlaneMeshNodeGenerateNormals,
	kPlaneMeshNodeMeshMaterial,
	kPlaneMeshNodeNumProperties
};

#define ID_PlaneMeshNode_PlaneWidth 0xccbf00b0
#define ID_PlaneMeshNode_PlaneHeight 0x16eb9527
#define ID_PlaneMeshNode_PlaneInvertU 0x0e445057
#define ID_PlaneMeshNode_PlaneInvertV 0x0f4451ea
#define ID_PlaneMeshNode_GenerateTangents 0x331aaad3
#define ID_PlaneMeshNode_GenerateNormals 0x624df2a3
#define ID_PlaneMeshNode_MeshMaterial 0x7dcb5be2
#define ID_Camera 0xe74c7b6e
#define GetCamera(_P)((struct Camera*)((_P)?OBJ_GetComponent(_P,ID_Camera):NULL))
#define Camera_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Camera,sizeof(struct Camera),_N)
enum CameraProperties {
	kCameraFov,
	kCameraFovType,
	kCameraProjectionType,
	kCameraZNear,
	kCameraZFar,
	kCameraEyeSeparation,
	kCameraFocalDistance,
	kCameraOrthogonalPlaneHeight,
	kCameraZPositive,
	kCameraNumProperties
};

#define ID_Camera_Fov 0xa851ed83
#define ID_Camera_FovType 0x57864ddb
#define ID_Camera_ProjectionType 0x40d9f225
#define ID_Camera_ZNear 0xd1ebbab8
#define ID_Camera_ZFar 0xe59e830b
#define ID_Camera_EyeSeparation 0x8be964c5
#define ID_Camera_FocalDistance 0x7b40da1e
#define ID_Camera_OrthogonalPlaneHeight 0x6a784130
#define ID_Camera_ZPositive 0x901cf905
#define ID_TrajectoryList3D 0x85dc1157
#define GetTrajectoryList3D(_P)((struct TrajectoryList3D*)((_P)?OBJ_GetComponent(_P,ID_TrajectoryList3D):NULL))
#define TrajectoryList3D_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_TrajectoryList3D,sizeof(struct TrajectoryList3D),_N)
enum TrajectoryList3DProperties {
	kTrajectoryList3DTrajectory,
	kTrajectoryList3DScrollAxis,
	kTrajectoryList3DLooping,
	kTrajectoryList3DSpacing,
	kTrajectoryList3DSelectionBehavior,
	kTrajectoryList3DSelectedItemIndex,
	kTrajectoryList3DNumProperties
};

#define ID_TrajectoryList3D_Trajectory 0x566c23da
#define ID_TrajectoryList3D_ScrollAxis 0x3a6746c7
#define ID_TrajectoryList3D_Looping 0xbcc36227
#define ID_TrajectoryList3D_Spacing 0xa5b50a40
#define ID_TrajectoryList3D_SelectionBehavior 0x30abffc9
#define ID_TrajectoryList3D_SelectedItemIndex 0x1b6e1d67
#define ID_Viewport3D 0x43eddddc
#define GetViewport3D(_P)((struct Viewport3D*)((_P)?OBJ_GetComponent(_P,ID_Viewport3D):NULL))
#define Viewport3D_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Viewport3D,sizeof(struct Viewport3D),_N)
enum Viewport3DProperties {
	kViewport3DCamera,
	kViewport3DPreviewCamera,
	kViewport3DHitTestCamera,
	kViewport3DRenderPass,
	kViewport3DScene,
	kViewport3DNumProperties
};

#define ID_Viewport3D_Camera 0xe764c175
#define ID_Viewport3D_PreviewCamera 0x51643a99
#define ID_Viewport3D_HitTestCamera 0xa517a694
#define ID_Viewport3D_RenderPass 0x6653844b
#define ID_Viewport3D_Scene 0xf4ce8b16
#define ID_PrefabView3D 0x8d451711
#define GetPrefabView3D(_P)((struct PrefabView3D*)((_P)?OBJ_GetComponent(_P,ID_PrefabView3D):NULL))
#define PrefabView3D_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_PrefabView3D,sizeof(struct PrefabView3D),_N)
enum PrefabView3DProperties {
	kPrefabView3DSCA,
	kPrefabView3DPrefab,
	kPrefabView3DNumProperties
};

#define ID_PrefabView3D_SCA 0x10804bee
#define ID_PrefabView3D_Prefab 0xc507610b
#define ID_RenderPass 0xf64bbf80
#define GetRenderPass(_P)((struct RenderPass*)((_P)?OBJ_GetComponent(_P,ID_RenderPass):NULL))
#define RenderPass_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_RenderPass,sizeof(struct RenderPass),_N)
enum RenderPassProperties {
	kRenderPassNumProperties
};

#define ID_CompositionTargetRenderPass 0xc47b4cd5
#define GetCompositionTargetRenderPass(_P)((struct CompositionTargetRenderPass*)((_P)?OBJ_GetComponent(_P,ID_CompositionTargetRenderPass):NULL))
#define CompositionTargetRenderPass_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_CompositionTargetRenderPass,sizeof(struct CompositionTargetRenderPass),_N)
enum CompositionTargetRenderPassProperties {
	kCompositionTargetRenderPassNumProperties
};

#define ID_BlitRenderPass 0xd49cc3cb
#define GetBlitRenderPass(_P)((struct BlitRenderPass*)((_P)?OBJ_GetComponent(_P,ID_BlitRenderPass):NULL))
#define BlitRenderPass_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_BlitRenderPass,sizeof(struct BlitRenderPass),_N)
enum BlitRenderPassProperties {
	kBlitRenderPassNumProperties
};

#define ID_ClearRenderPass 0xd66243d7
#define GetClearRenderPass(_P)((struct ClearRenderPass*)((_P)?OBJ_GetComponent(_P,ID_ClearRenderPass):NULL))
#define ClearRenderPass_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_ClearRenderPass,sizeof(struct ClearRenderPass),_N)
enum ClearRenderPassProperties {
	kClearRenderPassClearColor,
	kClearRenderPassClearDepth,
	kClearRenderPassClearStencil,
	kClearRenderPassNumProperties
};

#define ID_ClearRenderPass_ClearColor 0x5ab85c8b
#define ID_ClearRenderPass_ClearDepth 0x57443a59
#define ID_ClearRenderPass_ClearStencil 0xee631518
#define ID_DrawObjectsRenderPass 0x4c8c74ac
#define GetDrawObjectsRenderPass(_P)((struct DrawObjectsRenderPass*)((_P)?OBJ_GetComponent(_P,ID_DrawObjectsRenderPass):NULL))
#define DrawObjectsRenderPass_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_DrawObjectsRenderPass,sizeof(struct DrawObjectsRenderPass),_N)
enum DrawObjectsRenderPassProperties {
	kDrawObjectsRenderPassCamera,
	kDrawObjectsRenderPassIncludeTags,
	kDrawObjectsRenderPassExcludeTags,
	kDrawObjectsRenderPassNumProperties
};

#define ID_DrawObjectsRenderPass_Camera 0x83af3705
#define ID_DrawObjectsRenderPass_IncludeTags 0xadfb2f9f
#define ID_DrawObjectsRenderPass_ExcludeTags 0xae9a6ab9
#define ID_PipelineStateRenderPass 0x4d11b74d
#define GetPipelineStateRenderPass(_P)((struct PipelineStateRenderPass*)((_P)?OBJ_GetComponent(_P,ID_PipelineStateRenderPass):NULL))
#define PipelineStateRenderPass_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_PipelineStateRenderPass,sizeof(struct PipelineStateRenderPass),_N)
enum PipelineStateRenderPassProperties {
	kPipelineStateRenderPassBlendMode,
	kPipelineStateRenderPassColorWriteMode,
	kPipelineStateRenderPassCullMode,
	kPipelineStateRenderPassDepthTestFunction,
	kPipelineStateRenderPassDepthWriteEnabled,
	kPipelineStateRenderPassViewportMode,
	kPipelineStateRenderPassScissorMode,
	kPipelineStateRenderPassViewport,
	kPipelineStateRenderPassScissor,
	kPipelineStateRenderPassStencilTestFunction,
	kPipelineStateRenderPassStencilReferenceValue,
	kPipelineStateRenderPassStencilMask,
	kPipelineStateRenderPassStencilFailOperation,
	kPipelineStateRenderPassStencilPassDepthFailOperation,
	kPipelineStateRenderPassStencilPassDepthPassOperation,
	kPipelineStateRenderPassStencilWriteEnabled,
	kPipelineStateRenderPassNumProperties
};

#define ID_PipelineStateRenderPass_BlendMode 0x27f55ce7
#define ID_PipelineStateRenderPass_ColorWriteMode 0xd331f8d4
#define ID_PipelineStateRenderPass_CullMode 0x6111985e
#define ID_PipelineStateRenderPass_DepthTestFunction 0x18caa69a
#define ID_PipelineStateRenderPass_DepthWriteEnabled 0xddc484b2
#define ID_PipelineStateRenderPass_ViewportMode 0x42be6088
#define ID_PipelineStateRenderPass_ScissorMode 0xe1567812
#define ID_PipelineStateRenderPass_Viewport 0xa08f902f
#define ID_PipelineStateRenderPass_Scissor 0x3c3ab5e5
#define ID_PipelineStateRenderPass_StencilTestFunction 0x3abaf22b
#define ID_PipelineStateRenderPass_StencilReferenceValue 0xb056ef45
#define ID_PipelineStateRenderPass_StencilMask 0xebd6e4fb
#define ID_PipelineStateRenderPass_StencilFailOperation 0xd2f7a7b8
#define ID_PipelineStateRenderPass_StencilPassDepthFailOperation 0x56c00124
#define ID_PipelineStateRenderPass_StencilPassDepthPassOperation 0x5a2909d3
#define ID_PipelineStateRenderPass_StencilWriteEnabled 0xd004a2ef
#define ID_TextBlock3D 0x75943e14
#define GetTextBlock3D(_P)((struct TextBlock3D*)((_P)?OBJ_GetComponent(_P,ID_TextBlock3D):NULL))
#define TextBlock3D_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_TextBlock3D,sizeof(struct TextBlock3D),_N)
enum TextBlock3DProperties {
	kTextBlock3DNumProperties
};

#define ID_Light3D 0x112895d0
#define GetLight3D(_P)((struct Light3D*)((_P)?OBJ_GetComponent(_P,ID_Light3D):NULL))
#define Light3D_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Light3D,sizeof(struct Light3D),_N)
enum Light3DProperties {
	kLight3DColor,
	kLight3DSpotAngle,
	kLight3DIntensity,
	kLight3DRange,
	kLight3DType,
	kLight3DNumProperties
};

#define ID_Light3D_Color 0xe4bce9cd
#define ID_Light3D_SpotAngle 0x78d3a059
#define ID_Light3D_Intensity 0x1e090eab
#define ID_Light3D_Range 0xb9a837e7
#define ID_Light3D_Type 0xe3c77eba
#define ID_SpriteView 0x366e2265
#define GetSpriteView(_P)((struct SpriteView*)((_P)?OBJ_GetComponent(_P,ID_SpriteView):NULL))
#define SpriteView_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_SpriteView,sizeof(struct SpriteView),_N)
enum SpriteViewProperties {
	kSpriteViewImage,
	kSpriteViewBounds,
	kSpriteViewNumProperties
};

#define ID_SpriteView_Image 0x1b80b626
#define ID_SpriteView_Bounds 0x15deb274
