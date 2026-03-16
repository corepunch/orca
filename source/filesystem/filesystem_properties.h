#ifndef __FILESYSTEM_PROPERTIES_H__
#define __FILESYSTEM_PROPERTIES_H__
#define kEventReadCommands 0x23d83fd3
#define kEventOpenFile 0xa2c038cf
#define kEventFileExists 0x38dfc973
#define kEventHasChangedFiles 0x5390a564
#define kEventLoadProject 0x31b9fee2
// Bundle
#define ID_Bundle 0xe6397a25
#define GetBundle(_P) ((struct Bundle*)((_P)?OBJ_GetComponent(_P,ID_Bundle):NULL))
#define Bundle_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Bundle,sizeof(struct Bundle),_N)
enum BundleProperties {
	kBundleNumProperties	
};
// Directory
#define ID_Directory 0x94226a28
#define GetDirectory(_P) ((struct Directory*)((_P)?OBJ_GetComponent(_P,ID_Directory):NULL))
#define Directory_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Directory,sizeof(struct Directory),_N)
#define ID_Directory_Path 0xe78c5471 // Directory.Path
enum DirectoryProperties {
	kDirectoryPath,
	kDirectoryNumProperties	
};
// Package
#define ID_Package 0xfe8f4b9b
#define GetPackage(_P) ((struct Package*)((_P)?OBJ_GetComponent(_P,ID_Package):NULL))
#define Package_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Package,sizeof(struct Package),_N)
#define ID_Package_FileName 0x389d60fa // Package.FileName
enum PackageProperties {
	kPackageFileName,
	kPackageNumProperties	
};
// Workspace
#define ID_Workspace 0x27419f56
#define GetWorkspace(_P) ((struct Workspace*)((_P)?OBJ_GetComponent(_P,ID_Workspace):NULL))
#define Workspace_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Workspace,sizeof(struct Workspace),_N)
enum WorkspaceProperties {
	kWorkspaceNumProperties	
};
// Project
#define ID_Project 0x7b5fea5e
#define GetProject(_P) ((struct Project*)((_P)?OBJ_GetComponent(_P,ID_Project):NULL))
#define Project_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Project,sizeof(struct Project),_N)
#define ID_Project_HalfFloatTextureFormat 0xf064c913 // Project.HalfFloatTextureFormat
#define ID_Project_HalfFloatTextureFormatLinear 0xec96a44e // Project.HalfFloatTextureFormatLinear
#define ID_Project_HalfFloatColorAttachment 0xd0c866f1 // Project.HalfFloatColorAttachment
#define ID_Project_RenderToMipmapLevels 0xa8a014d6 // Project.RenderToMipmapLevels
#define ID_Project_ExternalTexture 0x3a64c056 // Project.ExternalTexture
#define ID_Project_StartupScreen 0x7987d405 // Project.StartupScreen
#define ID_Project_PreviewWindowBackgroundColor 0xdd25ae63 // Project.PreviewWindowBackgroundColor
#define ID_Project_MessageLimitPerFrame 0xfc6c2af2 // Project.MessageLimitPerFrame
#define ID_Project_GlobalTimelineStartTime 0x5f34f579 // Project.GlobalTimelineStartTime
#define ID_Project_GlobalTimelineEndTime 0x37b6c4b8 // Project.GlobalTimelineEndTime
#define ID_Project_BinaryExportDirectory 0xa8845962 // Project.BinaryExportDirectory
#define ID_Project_ApplicationExportDirectory 0x6ae977af // Project.ApplicationExportDirectory
#define ID_Project_IsMasterProject 0x9edad02f // Project.IsMasterProject
#define ID_Project_OptimizeMeshes 0x94c711f2 // Project.OptimizeMeshes
#define ID_Project_TargetPlatformVertexCacheSize 0x082331a7 // Project.TargetPlatformVertexCacheSize
#define ID_Project_PlotAnimations 0x12e9528c // Project.PlotAnimations
#define ID_Project_RoundImagesToNearestPowerOf2 0x77694415 // Project.RoundImagesToNearestPowerOf2
#define ID_Project_ProjectExportShaderSourceCode 0xd333f99a // Project.ProjectExportShaderSourceCode
#define ID_Project_ProjectExportMainKzbWithBakedThemes 0x57f88dac // Project.ProjectExportMainKzbWithBakedThemes
#define ID_Project_ProjectGroupByThemeNameInBakedThemeExport 0xcd87c9e0 // Project.ProjectGroupByThemeNameInBakedThemeExport
#define ID_Project_IsLocalizationEnabled 0x1f9a2b02 // Project.IsLocalizationEnabled
#define ID_Project_FullScreenPreviewLayer 0x44433db4 // Project.FullScreenPreviewLayer
#define ID_Project_ShowChildrenInLayerThumbnails 0xeff04efb // Project.ShowChildrenInLayerThumbnails
#define ID_Project_CompositionDesignSize 0x36df81ed // Project.CompositionDesignSize
#define ID_Project_ProjectUsePremultipliedAlpha 0xc9d6b678 // Project.ProjectUsePremultipliedAlpha
#define ID_Project_ProjectRemoveICCProfilesOfPngs 0xded57e15 // Project.ProjectRemoveICCProfilesOfPngs
#define ID_Project_BinaryFileName 0x5a451f18 // Project.BinaryFileName
#define ID_Project_IsAssetPackage 0xfebfd568 // Project.IsAssetPackage
#define ID_Project_KanziConnectEnabled 0xb2dbb3f4 // Project.KanziConnectEnabled
#define ID_Project_DefaultMaterial 0x3c678158 // Project.DefaultMaterial
#define ID_Project_WindowWidth 0xf811b418 // Project.WindowWidth
#define ID_Project_WindowHeight 0xf64e53af // Project.WindowHeight
#define ID_Project_PropertyTypes 0x5273d526 // Project.PropertyTypes
#define ID_Project_NumPropertyTypes 0x13e23766 // Project.NumPropertyTypes
#define ID_Project_ProjectReferences 0xcfc1761b // Project.ProjectReferences
#define ID_Project_NumProjectReferences 0x6c9cdb5b // Project.NumProjectReferences
#define ID_Project_SystemMessages 0xd3829a01 // Project.SystemMessages
#define ID_Project_NumSystemMessages 0x89eac941 // Project.NumSystemMessages
#define ID_Project_Plugins 0xf63a7450 // Project.Plugins
#define ID_Project_NumPlugins 0x2ed36890 // Project.NumPlugins
enum ProjectProperties {
	kProjectHalfFloatTextureFormat,
	kProjectHalfFloatTextureFormatLinear,
	kProjectHalfFloatColorAttachment,
	kProjectRenderToMipmapLevels,
	kProjectExternalTexture,
	kProjectStartupScreen,
	kProjectPreviewWindowBackgroundColor,
	kProjectMessageLimitPerFrame,
	kProjectGlobalTimelineStartTime,
	kProjectGlobalTimelineEndTime,
	kProjectBinaryExportDirectory,
	kProjectApplicationExportDirectory,
	kProjectIsMasterProject,
	kProjectOptimizeMeshes,
	kProjectTargetPlatformVertexCacheSize,
	kProjectPlotAnimations,
	kProjectRoundImagesToNearestPowerOf2,
	kProjectProjectExportShaderSourceCode,
	kProjectProjectExportMainKzbWithBakedThemes,
	kProjectProjectGroupByThemeNameInBakedThemeExport,
	kProjectIsLocalizationEnabled,
	kProjectFullScreenPreviewLayer,
	kProjectShowChildrenInLayerThumbnails,
	kProjectCompositionDesignSize,
	kProjectProjectUsePremultipliedAlpha,
	kProjectProjectRemoveICCProfilesOfPngs,
	kProjectBinaryFileName,
	kProjectIsAssetPackage,
	kProjectKanziConnectEnabled,
	kProjectDefaultMaterial,
	kProjectWindowWidth,
	kProjectWindowHeight,
	kProjectPropertyTypes,
	kProjectNumPropertyTypes,
	kProjectProjectReferences,
	kProjectNumProjectReferences,
	kProjectSystemMessages,
	kProjectNumSystemMessages,
	kProjectPlugins,
	kProjectNumPlugins,
	kProjectNumProperties	
};
// Library
#define ID_Library 0xa8532270
#define GetLibrary(_P) ((struct Library*)((_P)?OBJ_GetComponent(_P,ID_Library):NULL))
#define Library_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Library,sizeof(struct Library),_N)
#define ID_Library_IsExternal 0x9363c61d // Library.IsExternal
enum LibraryProperties {
	kLibraryIsExternal,
	kLibraryNumProperties	
};
// AnimationClipLibrary
#define ID_AnimationClipLibrary 0xb18f0186
#define GetAnimationClipLibrary(_P) ((struct AnimationClipLibrary*)((_P)?OBJ_GetComponent(_P,ID_AnimationClipLibrary):NULL))
#define AnimationClipLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_AnimationClipLibrary,sizeof(struct AnimationClipLibrary),_N)
enum AnimationClipLibraryProperties {
	kAnimationClipLibraryNumProperties	
};
// ScreenLibrary
#define ID_ScreenLibrary 0x33aae09c
#define GetScreenLibrary(_P) ((struct ScreenLibrary*)((_P)?OBJ_GetComponent(_P,ID_ScreenLibrary):NULL))
#define ScreenLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ScreenLibrary,sizeof(struct ScreenLibrary),_N)
enum ScreenLibraryProperties {
	kScreenLibraryNumProperties	
};
// MaterialTypeLibrary
#define ID_MaterialTypeLibrary 0xa8705423
#define GetMaterialTypeLibrary(_P) ((struct MaterialTypeLibrary*)((_P)?OBJ_GetComponent(_P,ID_MaterialTypeLibrary):NULL))
#define MaterialTypeLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_MaterialTypeLibrary,sizeof(struct MaterialTypeLibrary),_N)
enum MaterialTypeLibraryProperties {
	kMaterialTypeLibraryNumProperties	
};
// MaterialLibrary
#define ID_MaterialLibrary 0xa52f6c1b
#define GetMaterialLibrary(_P) ((struct MaterialLibrary*)((_P)?OBJ_GetComponent(_P,ID_MaterialLibrary):NULL))
#define MaterialLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_MaterialLibrary,sizeof(struct MaterialLibrary),_N)
enum MaterialLibraryProperties {
	kMaterialLibraryNumProperties	
};
// BrushLibrary
#define ID_BrushLibrary 0xaa7539c6
#define GetBrushLibrary(_P) ((struct BrushLibrary*)((_P)?OBJ_GetComponent(_P,ID_BrushLibrary):NULL))
#define BrushLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_BrushLibrary,sizeof(struct BrushLibrary),_N)
enum BrushLibraryProperties {
	kBrushLibraryNumProperties	
};
// MeshLibrary
#define ID_MeshLibrary 0x11d562a7
#define GetMeshLibrary(_P) ((struct MeshLibrary*)((_P)?OBJ_GetComponent(_P,ID_MeshLibrary):NULL))
#define MeshLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_MeshLibrary,sizeof(struct MeshLibrary),_N)
enum MeshLibraryProperties {
	kMeshLibraryNumProperties	
};
// TimelineSequenceLibrary
#define ID_TimelineSequenceLibrary 0x6a7a199a
#define GetTimelineSequenceLibrary(_P) ((struct TimelineSequenceLibrary*)((_P)?OBJ_GetComponent(_P,ID_TimelineSequenceLibrary):NULL))
#define TimelineSequenceLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_TimelineSequenceLibrary,sizeof(struct TimelineSequenceLibrary),_N)
enum TimelineSequenceLibraryProperties {
	kTimelineSequenceLibraryNumProperties	
};
// SceneObjectLibrary
#define ID_SceneObjectLibrary 0x9cebd1a3
#define GetSceneObjectLibrary(_P) ((struct SceneObjectLibrary*)((_P)?OBJ_GetComponent(_P,ID_SceneObjectLibrary):NULL))
#define SceneObjectLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SceneObjectLibrary,sizeof(struct SceneObjectLibrary),_N)
enum SceneObjectLibraryProperties {
	kSceneObjectLibraryNumProperties	
};
// ComposerLibrary
#define ID_ComposerLibrary 0x739c88a6
#define GetComposerLibrary(_P) ((struct ComposerLibrary*)((_P)?OBJ_GetComponent(_P,ID_ComposerLibrary):NULL))
#define ComposerLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ComposerLibrary,sizeof(struct ComposerLibrary),_N)
enum ComposerLibraryProperties {
	kComposerLibraryNumProperties	
};
// PipelineItemLibrary
#define ID_PipelineItemLibrary 0x9da5b82f
#define GetPipelineItemLibrary(_P) ((struct PipelineItemLibrary*)((_P)?OBJ_GetComponent(_P,ID_PipelineItemLibrary):NULL))
#define PipelineItemLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_PipelineItemLibrary,sizeof(struct PipelineItemLibrary),_N)
enum PipelineItemLibraryProperties {
	kPipelineItemLibraryNumProperties	
};
// SceneLibrary
#define ID_SceneLibrary 0xfa6e5926
#define GetSceneLibrary(_P) ((struct SceneLibrary*)((_P)?OBJ_GetComponent(_P,ID_SceneLibrary):NULL))
#define SceneLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SceneLibrary,sizeof(struct SceneLibrary),_N)
enum SceneLibraryProperties {
	kSceneLibraryNumProperties	
};
// TrajectoryLibrary
#define ID_TrajectoryLibrary 0x081b51d3
#define GetTrajectoryLibrary(_P) ((struct TrajectoryLibrary*)((_P)?OBJ_GetComponent(_P,ID_TrajectoryLibrary):NULL))
#define TrajectoryLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_TrajectoryLibrary,sizeof(struct TrajectoryLibrary),_N)
enum TrajectoryLibraryProperties {
	kTrajectoryLibraryNumProperties	
};
// TransitionLibrary
#define ID_TransitionLibrary 0xcf1f17a5
#define GetTransitionLibrary(_P) ((struct TransitionLibrary*)((_P)?OBJ_GetComponent(_P,ID_TransitionLibrary):NULL))
#define TransitionLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_TransitionLibrary,sizeof(struct TransitionLibrary),_N)
enum TransitionLibraryProperties {
	kTransitionLibraryNumProperties	
};
// SplineLibrary
#define ID_SplineLibrary 0xb16c4967
#define GetSplineLibrary(_P) ((struct SplineLibrary*)((_P)?OBJ_GetComponent(_P,ID_SplineLibrary):NULL))
#define SplineLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SplineLibrary,sizeof(struct SplineLibrary),_N)
enum SplineLibraryProperties {
	kSplineLibraryNumProperties	
};
// PrefabLibrary
#define ID_PrefabLibrary 0x0b066b16
#define GetPrefabLibrary(_P) ((struct PrefabLibrary*)((_P)?OBJ_GetComponent(_P,ID_PrefabLibrary):NULL))
#define PrefabLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_PrefabLibrary,sizeof(struct PrefabLibrary),_N)
enum PrefabLibraryProperties {
	kPrefabLibraryNumProperties	
};
// ProfileLibrary
#define ID_ProfileLibrary 0xd4de6821
#define GetProfileLibrary(_P) ((struct ProfileLibrary*)((_P)?OBJ_GetComponent(_P,ID_ProfileLibrary):NULL))
#define ProfileLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ProfileLibrary,sizeof(struct ProfileLibrary),_N)
enum ProfileLibraryProperties {
	kProfileLibraryNumProperties	
};
// EnginePluginLibrary
#define ID_EnginePluginLibrary 0x92d1b7eb
#define GetEnginePluginLibrary(_P) ((struct EnginePluginLibrary*)((_P)?OBJ_GetComponent(_P,ID_EnginePluginLibrary):NULL))
#define EnginePluginLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_EnginePluginLibrary,sizeof(struct EnginePluginLibrary),_N)
enum EnginePluginLibraryProperties {
	kEnginePluginLibraryNumProperties	
};
// ShortcutLibrary
#define ID_ShortcutLibrary 0x425801c6
#define GetShortcutLibrary(_P) ((struct ShortcutLibrary*)((_P)?OBJ_GetComponent(_P,ID_ShortcutLibrary):NULL))
#define ShortcutLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ShortcutLibrary,sizeof(struct ShortcutLibrary),_N)
enum ShortcutLibraryProperties {
	kShortcutLibraryNumProperties	
};
// LayerLibrary
#define ID_LayerLibrary 0x309f6b49
#define GetLayerLibrary(_P) ((struct LayerLibrary*)((_P)?OBJ_GetComponent(_P,ID_LayerLibrary):NULL))
#define LayerLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_LayerLibrary,sizeof(struct LayerLibrary),_N)
enum LayerLibraryProperties {
	kLayerLibraryNumProperties	
};
// AnimationLibrary
#define ID_AnimationLibrary 0x1818e708
#define GetAnimationLibrary(_P) ((struct AnimationLibrary*)((_P)?OBJ_GetComponent(_P,ID_AnimationLibrary):NULL))
#define AnimationLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_AnimationLibrary,sizeof(struct AnimationLibrary),_N)
enum AnimationLibraryProperties {
	kAnimationLibraryNumProperties	
};
// TagLibrary
#define ID_TagLibrary 0x7e634fa6
#define GetTagLibrary(_P) ((struct TagLibrary*)((_P)?OBJ_GetComponent(_P,ID_TagLibrary):NULL))
#define TagLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_TagLibrary,sizeof(struct TagLibrary),_N)
enum TagLibraryProperties {
	kTagLibraryNumProperties	
};
// ThemeLibrary
#define ID_ThemeLibrary 0x508975b5
#define GetThemeLibrary(_P) ((struct ThemeLibrary*)((_P)?OBJ_GetComponent(_P,ID_ThemeLibrary):NULL))
#define ThemeLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ThemeLibrary,sizeof(struct ThemeLibrary),_N)
enum ThemeLibraryProperties {
	kThemeLibraryNumProperties	
};
// ResourceExportTagLibrary
#define ID_ResourceExportTagLibrary 0x7ca6ddcc
#define GetResourceExportTagLibrary(_P) ((struct ResourceExportTagLibrary*)((_P)?OBJ_GetComponent(_P,ID_ResourceExportTagLibrary):NULL))
#define ResourceExportTagLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ResourceExportTagLibrary,sizeof(struct ResourceExportTagLibrary),_N)
enum ResourceExportTagLibraryProperties {
	kResourceExportTagLibraryNumProperties	
};
// LocaleLibrary
#define ID_LocaleLibrary 0xc56f541a
#define GetLocaleLibrary(_P) ((struct LocaleLibrary*)((_P)?OBJ_GetComponent(_P,ID_LocaleLibrary):NULL))
#define LocaleLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_LocaleLibrary,sizeof(struct LocaleLibrary),_N)
enum LocaleLibraryProperties {
	kLocaleLibraryNumProperties	
};
// DataSourceLibrary
#define ID_DataSourceLibrary 0xe1462dd3
#define GetDataSourceLibrary(_P) ((struct DataSourceLibrary*)((_P)?OBJ_GetComponent(_P,ID_DataSourceLibrary):NULL))
#define DataSourceLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_DataSourceLibrary,sizeof(struct DataSourceLibrary),_N)
enum DataSourceLibraryProperties {
	kDataSourceLibraryNumProperties	
};
// PageTransitionCollectionLibrary
#define ID_PageTransitionCollectionLibrary 0x29c80844
#define GetPageTransitionCollectionLibrary(_P) ((struct PageTransitionCollectionLibrary*)((_P)?OBJ_GetComponent(_P,ID_PageTransitionCollectionLibrary):NULL))
#define PageTransitionCollectionLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_PageTransitionCollectionLibrary,sizeof(struct PageTransitionCollectionLibrary),_N)
enum PageTransitionCollectionLibraryProperties {
	kPageTransitionCollectionLibraryNumProperties	
};
// TextureLibrary
#define ID_TextureLibrary 0x99f2a01f
#define GetTextureLibrary(_P) ((struct TextureLibrary*)((_P)?OBJ_GetComponent(_P,ID_TextureLibrary):NULL))
#define TextureLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_TextureLibrary,sizeof(struct TextureLibrary),_N)
enum TextureLibraryProperties {
	kTextureLibraryNumProperties	
};
// StyleLibrary
#define ID_StyleLibrary 0x5c0ac569
#define GetStyleLibrary(_P) ((struct StyleLibrary*)((_P)?OBJ_GetComponent(_P,ID_StyleLibrary):NULL))
#define StyleLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_StyleLibrary,sizeof(struct StyleLibrary),_N)
enum StyleLibraryProperties {
	kStyleLibraryNumProperties	
};
// StateManagerLibrary
#define ID_StateManagerLibrary 0x1f8da040
#define GetStateManagerLibrary(_P) ((struct StateManagerLibrary*)((_P)?OBJ_GetComponent(_P,ID_StateManagerLibrary):NULL))
#define StateManagerLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_StateManagerLibrary,sizeof(struct StateManagerLibrary),_N)
enum StateManagerLibraryProperties {
	kStateManagerLibraryNumProperties	
};
// ConnectServiceLibrary
#define ID_ConnectServiceLibrary 0x66ac9ce3
#define GetConnectServiceLibrary(_P) ((struct ConnectServiceLibrary*)((_P)?OBJ_GetComponent(_P,ID_ConnectServiceLibrary):NULL))
#define ConnectServiceLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ConnectServiceLibrary,sizeof(struct ConnectServiceLibrary),_N)
enum ConnectServiceLibraryProperties {
	kConnectServiceLibraryNumProperties	
};
// ConnectUserServiceLibrary
#define ID_ConnectUserServiceLibrary 0x445ea848
#define GetConnectUserServiceLibrary(_P) ((struct ConnectUserServiceLibrary*)((_P)?OBJ_GetComponent(_P,ID_ConnectUserServiceLibrary):NULL))
#define ConnectUserServiceLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ConnectUserServiceLibrary,sizeof(struct ConnectUserServiceLibrary),_N)
enum ConnectUserServiceLibraryProperties {
	kConnectUserServiceLibraryNumProperties	
};
// SpriteLibrary
#define ID_SpriteLibrary 0xb1288547
#define GetSpriteLibrary(_P) ((struct SpriteLibrary*)((_P)?OBJ_GetComponent(_P,ID_SpriteLibrary):NULL))
#define SpriteLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SpriteLibrary,sizeof(struct SpriteLibrary),_N)
enum SpriteLibraryProperties {
	kSpriteLibraryNumProperties	
};
// SpriteAnimationLibrary
#define ID_SpriteAnimationLibrary 0x9ef864fd
#define GetSpriteAnimationLibrary(_P) ((struct SpriteAnimationLibrary*)((_P)?OBJ_GetComponent(_P,ID_SpriteAnimationLibrary):NULL))
#define SpriteAnimationLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_SpriteAnimationLibrary,sizeof(struct SpriteAnimationLibrary),_N)
enum SpriteAnimationLibraryProperties {
	kSpriteAnimationLibraryNumProperties	
};
// ImageLibrary
#define ID_ImageLibrary 0x533a469d
#define GetImageLibrary(_P) ((struct ImageLibrary*)((_P)?OBJ_GetComponent(_P,ID_ImageLibrary):NULL))
#define ImageLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ImageLibrary,sizeof(struct ImageLibrary),_N)
enum ImageLibraryProperties {
	kImageLibraryNumProperties	
};
// FontLibrary
#define ID_FontLibrary 0xb033dd0b
#define GetFontLibrary(_P) ((struct FontLibrary*)((_P)?OBJ_GetComponent(_P,ID_FontLibrary):NULL))
#define FontLibrary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_FontLibrary,sizeof(struct FontLibrary),_N)
enum FontLibraryProperties {
	kFontLibraryNumProperties	
};
// LocaleReferenceItem
#define ID_LocaleReferenceItem 0xe30b821b
#define GetLocaleReferenceItem(_P) ((struct LocaleReferenceItem*)((_P)?OBJ_GetComponent(_P,ID_LocaleReferenceItem):NULL))
#define LocaleReferenceItem_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_LocaleReferenceItem,sizeof(struct LocaleReferenceItem),_N)
enum LocaleReferenceItemProperties {
	kLocaleReferenceItemNumProperties	
};
// Tag
#define ID_Tag 0xf8835433
#define GetTag(_P) ((struct Tag*)((_P)?OBJ_GetComponent(_P,ID_Tag):NULL))
#define Tag_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Tag,sizeof(struct Tag),_N)
#define ID_Tag_TagIsInherited 0x66bab379 // Tag.TagIsInherited
enum TagProperties {
	kTagTagIsInherited,
	kTagNumProperties	
};
// EnginePlugin
#define ID_EnginePlugin 0xa58ca5d0
#define GetEnginePlugin(_P) ((struct EnginePlugin*)((_P)?OBJ_GetComponent(_P,ID_EnginePlugin):NULL))
#define EnginePlugin_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_EnginePlugin,sizeof(struct EnginePlugin),_N)
enum EnginePluginProperties {
	kEnginePluginNumProperties	
};
// EngineMetaclass
#define ID_EngineMetaclass 0xc12f7a06
#define GetEngineMetaclass(_P) ((struct EngineMetaclass*)((_P)?OBJ_GetComponent(_P,ID_EngineMetaclass):NULL))
#define EngineMetaclass_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_EngineMetaclass,sizeof(struct EngineMetaclass),_N)
#define ID_EngineMetaclass_BaseClassName 0x03191574 // EngineMetaclass.BaseClassName
enum EngineMetaclassProperties {
	kEngineMetaclassBaseClassName,
	kEngineMetaclassNumProperties	
};
// ThemeGroup
#define ID_ThemeGroup 0xa8bfcd45
#define GetThemeGroup(_P) ((struct ThemeGroup*)((_P)?OBJ_GetComponent(_P,ID_ThemeGroup):NULL))
#define ThemeGroup_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ThemeGroup,sizeof(struct ThemeGroup),_N)
#define ID_ThemeGroup_SelectedDictionary 0x6de33964 // ThemeGroup.SelectedDictionary
enum ThemeGroupProperties {
	kThemeGroupSelectedDictionary,
	kThemeGroupNumProperties	
};
// Theme
#define ID_Theme 0x4eaf7c92
#define GetTheme(_P) ((struct Theme*)((_P)?OBJ_GetComponent(_P,ID_Theme):NULL))
#define Theme_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Theme,sizeof(struct Theme),_N)
#define ID_Theme_IsThemeVisible 0x17736be5 // Theme.IsThemeVisible
enum ThemeProperties {
	kThemeIsThemeVisible,
	kThemeNumProperties	
};
// Entry
#define ID_Entry 0x9cbfed33
#define GetEntry(_P) ((struct Entry*)((_P)?OBJ_GetComponent(_P,ID_Entry):NULL))
#define Entry_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Entry,sizeof(struct Entry),_N)
enum EntryProperties {
	kEntryNumProperties	
};
// ThemeDefaultValuesDictionary
#define ID_ThemeDefaultValuesDictionary 0x8f7890b5
#define GetThemeDefaultValuesDictionary(_P) ((struct ThemeDefaultValuesDictionary*)((_P)?OBJ_GetComponent(_P,ID_ThemeDefaultValuesDictionary):NULL))
#define ThemeDefaultValuesDictionary_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ThemeDefaultValuesDictionary,sizeof(struct ThemeDefaultValuesDictionary),_N)
enum ThemeDefaultValuesDictionaryProperties {
	kThemeDefaultValuesDictionaryNumProperties	
};
#endif