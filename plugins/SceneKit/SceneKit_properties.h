// Auto-generated from SceneKit.xml by tools/templates/properties.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __SCENEKIT_PROPERTIES_H__
#define __SCENEKIT_PROPERTIES_H__

// Node3D
#define ID_Node3D 0xce61fe5a
#define GetNode3D(_P) ((struct Node3D*)((_P)?OBJ_GetComponent(_P,ID_Node3D):NULL))
#define Node3D_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Node3D,sizeof(struct Node3D),_N)
#define Node3D_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Node3D,_N)
#define ID_Node3D_LayoutTransform 0x3cc6155a // Node3D.LayoutTransform
#define ID_Node3D_LayoutTransformTranslation 0x34443d45 // Node3D.LayoutTransformTranslation
#define ID_Node3D_LayoutTransformRotation 0xa00afee8 // Node3D.LayoutTransformRotation
#define ID_Node3D_LayoutTransformScale 0xa3eeb830 // Node3D.LayoutTransformScale
#define ID_Node3D_RenderTransform 0xf2ecd3b8 // Node3D.RenderTransform
#define ID_Node3D_RenderTransformTranslation 0x8d2c8bcb // Node3D.RenderTransformTranslation
#define ID_Node3D_RenderTransformRotation 0x0f3796be // Node3D.RenderTransformRotation
#define ID_Node3D_RenderTransformScale 0x5714ba3a // Node3D.RenderTransformScale
#define ID_Node3D_RenderTransformOrigin 0xff3e393a // Node3D.RenderTransformOrigin
#define ID_Node3D_ContentOffset 0x660ce5d6 // Node3D.ContentOffset
#define ID_Node3D_Matrix 0xffeed605 // Node3D.Matrix
#define ID_Node3D_CalculatedOffset 0x7f8022a7 // Node3D.CalculatedOffset
#define kNode3DNumMessageTypes 1
enum Node3DMessages {
	kNode3DRender,
};
#define ID_Node3D_Render 0x74fae2ac // Node3D.Render
#define kNode3DNumProperties 12
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
};
// Scene
#define ID_Scene 0xc89b38b3
#define GetScene(_P) ((struct Scene*)((_P)?OBJ_GetComponent(_P,ID_Scene):NULL))
#define Scene_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Scene,sizeof(struct Scene),_N)
#define Scene_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Scene,_N)
#define ID_Scene_Camera 0x2856ba68 // Scene.Camera
#define ID_Scene_PreviewCamera 0x9ee5ebda // Scene.PreviewCamera
#define ID_Scene_HitTestCamera 0x30217eeb // Scene.HitTestCamera
#define ID_Scene_RenderPass 0x01a192ae // Scene.RenderPass
#define ID_Scene_BackgroundTimelineSequence 0xb8ec9e4d // Scene.BackgroundTimelineSequence
#define ID_Scene_ComposerOverride 0x7103adc1 // Scene.ComposerOverride
#define ID_Scene_EnvironmentTexture 0x8feab57b // Scene.EnvironmentTexture
#define kSceneNumMessageTypes 0
#define kSceneNumProperties 7
enum SceneProperties {
	kSceneCamera,
	kScenePreviewCamera,
	kSceneHitTestCamera,
	kSceneRenderPass,
	kSceneBackgroundTimelineSequence,
	kSceneComposerOverride,
	kSceneEnvironmentTexture,
};
// Model3D
#define ID_Model3D 0xc56de5fd
#define GetModel3D(_P) ((struct Model3D*)((_P)?OBJ_GetComponent(_P,ID_Model3D):NULL))
#define Model3D_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Model3D,sizeof(struct Model3D),_N)
#define Model3D_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Model3D,_N)
#define ID_Model3D_Mesh 0xa983d6a0 // Model3D.Mesh
#define ID_Model3D_Material 0x0c181d04 // Model3D.Material
#define kModel3DNumMessageTypes 0
#define kModel3DNumProperties 2
enum Model3DProperties {
	kModel3DMesh,
	kModel3DMaterial,
};
// PlaneMeshNode
#define ID_PlaneMeshNode 0x660bed48
#define GetPlaneMeshNode(_P) ((struct PlaneMeshNode*)((_P)?OBJ_GetComponent(_P,ID_PlaneMeshNode):NULL))
#define PlaneMeshNode_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_PlaneMeshNode,sizeof(struct PlaneMeshNode),_N)
#define PlaneMeshNode_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_PlaneMeshNode,_N)
#define ID_PlaneMeshNode_PlaneWidth 0xccbf00b0 // PlaneMeshNode.PlaneWidth
#define ID_PlaneMeshNode_PlaneHeight 0x16eb9527 // PlaneMeshNode.PlaneHeight
#define ID_PlaneMeshNode_PlaneInvertU 0x0e445057 // PlaneMeshNode.PlaneInvertU
#define ID_PlaneMeshNode_PlaneInvertV 0x0f4451ea // PlaneMeshNode.PlaneInvertV
#define ID_PlaneMeshNode_GenerateTangents 0x331aaad3 // PlaneMeshNode.GenerateTangents
#define ID_PlaneMeshNode_GenerateNormals 0x624df2a3 // PlaneMeshNode.GenerateNormals
#define ID_PlaneMeshNode_MeshMaterial 0x7dcb5be2 // PlaneMeshNode.MeshMaterial
#define kPlaneMeshNodeNumMessageTypes 0
#define kPlaneMeshNodeNumProperties 7
enum PlaneMeshNodeProperties {
	kPlaneMeshNodePlaneWidth,
	kPlaneMeshNodePlaneHeight,
	kPlaneMeshNodePlaneInvertU,
	kPlaneMeshNodePlaneInvertV,
	kPlaneMeshNodeGenerateTangents,
	kPlaneMeshNodeGenerateNormals,
	kPlaneMeshNodeMeshMaterial,
};
// Camera
#define ID_Camera 0xe74c7b6e
#define GetCamera(_P) ((struct Camera*)((_P)?OBJ_GetComponent(_P,ID_Camera):NULL))
#define Camera_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Camera,sizeof(struct Camera),_N)
#define Camera_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Camera,_N)
#define ID_Camera_Fov 0xa851ed83 // Camera.Fov
#define ID_Camera_FovType 0x57864ddb // Camera.FovType
#define ID_Camera_ProjectionType 0x40d9f225 // Camera.ProjectionType
#define ID_Camera_ZNear 0xd1ebbab8 // Camera.ZNear
#define ID_Camera_ZFar 0xe59e830b // Camera.ZFar
#define ID_Camera_EyeSeparation 0x8be964c5 // Camera.EyeSeparation
#define ID_Camera_FocalDistance 0x7b40da1e // Camera.FocalDistance
#define ID_Camera_OrthogonalPlaneHeight 0x6a784130 // Camera.OrthogonalPlaneHeight
#define ID_Camera_ZPositive 0x901cf905 // Camera.ZPositive
#define kCameraNumMessageTypes 0
#define kCameraNumProperties 9
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
};
// TrajectoryList3D
#define ID_TrajectoryList3D 0x85dc1157
#define GetTrajectoryList3D(_P) ((struct TrajectoryList3D*)((_P)?OBJ_GetComponent(_P,ID_TrajectoryList3D):NULL))
#define TrajectoryList3D_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_TrajectoryList3D,sizeof(struct TrajectoryList3D),_N)
#define TrajectoryList3D_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_TrajectoryList3D,_N)
#define ID_TrajectoryList3D_Trajectory 0x566c23da // TrajectoryList3D.Trajectory
#define ID_TrajectoryList3D_ScrollAxis 0x3a6746c7 // TrajectoryList3D.ScrollAxis
#define ID_TrajectoryList3D_Looping 0xbcc36227 // TrajectoryList3D.Looping
#define ID_TrajectoryList3D_Spacing 0xa5b50a40 // TrajectoryList3D.Spacing
#define ID_TrajectoryList3D_SelectionBehavior 0x30abffc9 // TrajectoryList3D.SelectionBehavior
#define ID_TrajectoryList3D_SelectedItemIndex 0x1b6e1d67 // TrajectoryList3D.SelectedItemIndex
#define kTrajectoryList3DNumMessageTypes 0
#define kTrajectoryList3DNumProperties 6
enum TrajectoryList3DProperties {
	kTrajectoryList3DTrajectory,
	kTrajectoryList3DScrollAxis,
	kTrajectoryList3DLooping,
	kTrajectoryList3DSpacing,
	kTrajectoryList3DSelectionBehavior,
	kTrajectoryList3DSelectedItemIndex,
};
// Viewport3D
#define ID_Viewport3D 0x43eddddc
#define GetViewport3D(_P) ((struct Viewport3D*)((_P)?OBJ_GetComponent(_P,ID_Viewport3D):NULL))
#define Viewport3D_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Viewport3D,sizeof(struct Viewport3D),_N)
#define Viewport3D_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Viewport3D,_N)
#define ID_Viewport3D_Camera 0xe764c175 // Viewport3D.Camera
#define ID_Viewport3D_PreviewCamera 0x51643a99 // Viewport3D.PreviewCamera
#define ID_Viewport3D_HitTestCamera 0xa517a694 // Viewport3D.HitTestCamera
#define ID_Viewport3D_RenderPass 0x6653844b // Viewport3D.RenderPass
#define ID_Viewport3D_Scene 0xf4ce8b16 // Viewport3D.Scene
#define kViewport3DNumMessageTypes 0
#define kViewport3DNumProperties 5
enum Viewport3DProperties {
	kViewport3DCamera,
	kViewport3DPreviewCamera,
	kViewport3DHitTestCamera,
	kViewport3DRenderPass,
	kViewport3DScene,
};
// PrefabView3D
#define ID_PrefabView3D 0x8d451711
#define GetPrefabView3D(_P) ((struct PrefabView3D*)((_P)?OBJ_GetComponent(_P,ID_PrefabView3D):NULL))
#define PrefabView3D_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_PrefabView3D,sizeof(struct PrefabView3D),_N)
#define PrefabView3D_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_PrefabView3D,_N)
#define ID_PrefabView3D_SCA 0x10804bee // PrefabView3D.SCA
#define ID_PrefabView3D_Prefab 0xc507610b // PrefabView3D.Prefab
#define kPrefabView3DNumMessageTypes 0
#define kPrefabView3DNumProperties 2
enum PrefabView3DProperties {
	kPrefabView3DSCA,
	kPrefabView3DPrefab,
};
// RenderPass
#define ID_RenderPass 0xf64bbf80
#define GetRenderPass(_P) ((struct RenderPass*)((_P)?OBJ_GetComponent(_P,ID_RenderPass):NULL))
#define RenderPass_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_RenderPass,sizeof(struct RenderPass),_N)
#define RenderPass_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_RenderPass,_N)
#define kRenderPassNumMessageTypes 0
#define kRenderPassNumProperties 0
// CompositionTargetRenderPass
#define ID_CompositionTargetRenderPass 0xc47b4cd5
#define GetCompositionTargetRenderPass(_P) ((struct CompositionTargetRenderPass*)((_P)?OBJ_GetComponent(_P,ID_CompositionTargetRenderPass):NULL))
#define CompositionTargetRenderPass_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_CompositionTargetRenderPass,sizeof(struct CompositionTargetRenderPass),_N)
#define CompositionTargetRenderPass_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_CompositionTargetRenderPass,_N)
#define kCompositionTargetRenderPassNumMessageTypes 0
#define kCompositionTargetRenderPassNumProperties 0
// BlitRenderPass
#define ID_BlitRenderPass 0xd49cc3cb
#define GetBlitRenderPass(_P) ((struct BlitRenderPass*)((_P)?OBJ_GetComponent(_P,ID_BlitRenderPass):NULL))
#define BlitRenderPass_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_BlitRenderPass,sizeof(struct BlitRenderPass),_N)
#define BlitRenderPass_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_BlitRenderPass,_N)
#define kBlitRenderPassNumMessageTypes 0
#define kBlitRenderPassNumProperties 0
// ClearRenderPass
#define ID_ClearRenderPass 0xd66243d7
#define GetClearRenderPass(_P) ((struct ClearRenderPass*)((_P)?OBJ_GetComponent(_P,ID_ClearRenderPass):NULL))
#define ClearRenderPass_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ClearRenderPass,sizeof(struct ClearRenderPass),_N)
#define ClearRenderPass_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_ClearRenderPass,_N)
#define ID_ClearRenderPass_ClearColor 0x5ab85c8b // ClearRenderPass.ClearColor
#define ID_ClearRenderPass_ClearDepth 0x57443a59 // ClearRenderPass.ClearDepth
#define ID_ClearRenderPass_ClearStencil 0xee631518 // ClearRenderPass.ClearStencil
#define kClearRenderPassNumMessageTypes 0
#define kClearRenderPassNumProperties 3
enum ClearRenderPassProperties {
	kClearRenderPassClearColor,
	kClearRenderPassClearDepth,
	kClearRenderPassClearStencil,
};
// DrawObjectsRenderPass
#define ID_DrawObjectsRenderPass 0x4c8c74ac
#define GetDrawObjectsRenderPass(_P) ((struct DrawObjectsRenderPass*)((_P)?OBJ_GetComponent(_P,ID_DrawObjectsRenderPass):NULL))
#define DrawObjectsRenderPass_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_DrawObjectsRenderPass,sizeof(struct DrawObjectsRenderPass),_N)
#define DrawObjectsRenderPass_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_DrawObjectsRenderPass,_N)
#define ID_DrawObjectsRenderPass_Camera 0x83af3705 // DrawObjectsRenderPass.Camera
#define ID_DrawObjectsRenderPass_IncludeTags 0xadfb2f9f // DrawObjectsRenderPass.IncludeTags
#define ID_DrawObjectsRenderPass_ExcludeTags 0xae9a6ab9 // DrawObjectsRenderPass.ExcludeTags
#define kDrawObjectsRenderPassNumMessageTypes 0
#define kDrawObjectsRenderPassNumProperties 3
enum DrawObjectsRenderPassProperties {
	kDrawObjectsRenderPassCamera,
	kDrawObjectsRenderPassIncludeTags,
	kDrawObjectsRenderPassExcludeTags,
};
// PipelineStateRenderPass
#define ID_PipelineStateRenderPass 0x4d11b74d
#define GetPipelineStateRenderPass(_P) ((struct PipelineStateRenderPass*)((_P)?OBJ_GetComponent(_P,ID_PipelineStateRenderPass):NULL))
#define PipelineStateRenderPass_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_PipelineStateRenderPass,sizeof(struct PipelineStateRenderPass),_N)
#define PipelineStateRenderPass_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_PipelineStateRenderPass,_N)
#define ID_PipelineStateRenderPass_BlendMode 0x27f55ce7 // PipelineStateRenderPass.BlendMode
#define ID_PipelineStateRenderPass_ColorWriteMode 0xd331f8d4 // PipelineStateRenderPass.ColorWriteMode
#define ID_PipelineStateRenderPass_CullMode 0x6111985e // PipelineStateRenderPass.CullMode
#define ID_PipelineStateRenderPass_DepthTestFunction 0x18caa69a // PipelineStateRenderPass.DepthTestFunction
#define ID_PipelineStateRenderPass_DepthWriteEnabled 0xddc484b2 // PipelineStateRenderPass.DepthWriteEnabled
#define ID_PipelineStateRenderPass_ViewportMode 0x42be6088 // PipelineStateRenderPass.ViewportMode
#define ID_PipelineStateRenderPass_ScissorMode 0xe1567812 // PipelineStateRenderPass.ScissorMode
#define ID_PipelineStateRenderPass_Viewport 0xa08f902f // PipelineStateRenderPass.Viewport
#define ID_PipelineStateRenderPass_Scissor 0x3c3ab5e5 // PipelineStateRenderPass.Scissor
#define ID_PipelineStateRenderPass_StencilTestFunction 0x3abaf22b // PipelineStateRenderPass.StencilTestFunction
#define ID_PipelineStateRenderPass_StencilReferenceValue 0xb056ef45 // PipelineStateRenderPass.StencilReferenceValue
#define ID_PipelineStateRenderPass_StencilMask 0xebd6e4fb // PipelineStateRenderPass.StencilMask
#define ID_PipelineStateRenderPass_StencilFailOperation 0xd2f7a7b8 // PipelineStateRenderPass.StencilFailOperation
#define ID_PipelineStateRenderPass_StencilPassDepthFailOperation 0x56c00124 // PipelineStateRenderPass.StencilPassDepthFailOperation
#define ID_PipelineStateRenderPass_StencilPassDepthPassOperation 0x5a2909d3 // PipelineStateRenderPass.StencilPassDepthPassOperation
#define ID_PipelineStateRenderPass_StencilWriteEnabled 0xd004a2ef // PipelineStateRenderPass.StencilWriteEnabled
#define kPipelineStateRenderPassNumMessageTypes 0
#define kPipelineStateRenderPassNumProperties 16
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
};
// TextBlock3D
#define ID_TextBlock3D 0x75943e14
#define GetTextBlock3D(_P) ((struct TextBlock3D*)((_P)?OBJ_GetComponent(_P,ID_TextBlock3D):NULL))
#define TextBlock3D_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_TextBlock3D,sizeof(struct TextBlock3D),_N)
#define TextBlock3D_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_TextBlock3D,_N)
#define kTextBlock3DNumMessageTypes 0
#define kTextBlock3DNumProperties 0
// Light3D
#define ID_Light3D 0x112895d0
#define GetLight3D(_P) ((struct Light3D*)((_P)?OBJ_GetComponent(_P,ID_Light3D):NULL))
#define Light3D_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Light3D,sizeof(struct Light3D),_N)
#define Light3D_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Light3D,_N)
#define ID_Light3D_Color 0xe4bce9cd // Light3D.Color
#define ID_Light3D_SpotAngle 0x78d3a059 // Light3D.SpotAngle
#define ID_Light3D_Intensity 0x1e090eab // Light3D.Intensity
#define ID_Light3D_Range 0xb9a837e7 // Light3D.Range
#define ID_Light3D_Type 0xe3c77eba // Light3D.Type
#define kLight3DNumMessageTypes 0
#define kLight3DNumProperties 5
enum Light3DProperties {
	kLight3DColor,
	kLight3DSpotAngle,
	kLight3DIntensity,
	kLight3DRange,
	kLight3DType,
};
// SpriteView
#define ID_SpriteView 0x366e2265
#define GetSpriteView(_P) ((struct SpriteView*)((_P)?OBJ_GetComponent(_P,ID_SpriteView):NULL))
#define SpriteView_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SpriteView,sizeof(struct SpriteView),_N)
#define SpriteView_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_SpriteView,_N)
#define ID_SpriteView_Image 0x1b80b626 // SpriteView.Image
#define ID_SpriteView_Bounds 0x15deb274 // SpriteView.Bounds
#define kSpriteViewNumMessageTypes 0
#define kSpriteViewNumProperties 2
enum SpriteViewProperties {
	kSpriteViewImage,
	kSpriteViewBounds,
};


	

#define ID_Render 0x350cf42d
#define ID_RenderMsgArgs_ViewDef 0xbebba0a1 // RenderMsgArgs.ViewDef
	

#endif