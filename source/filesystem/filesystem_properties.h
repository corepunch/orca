// Auto-generated from filesystem.xml by tools/templates/properties.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __FILESYSTEM_PROPERTIES_H__
#define __FILESYSTEM_PROPERTIES_H__
#define kEventReadCommands 0x23d83fd3
#define kEventOpenFile 0xa2c038cf
#define kEventFileExists 0x38dfc973
#define kEventHasChangedFiles 0x5390a564
#define kEventLoadProject 0x31b9fee2
// Workspace
#define ID_Workspace 0x27419f56
#define GetWorkspace(_P) ((struct Workspace*)((_P)?OBJ_GetComponent(_P,ID_Workspace):NULL))
#define Workspace_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Workspace,sizeof(struct Workspace),_N)
enum WorkspaceProperties {
	kWorkspaceNumProperties	
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
#define ID_Project_EnginePlugins 0x77af126a // Project.EnginePlugins
#define ID_Project_NumEnginePlugins 0x7ee941aa // Project.NumEnginePlugins
#define ID_Project_AnimationClipLibrary 0x627fe7a3 // Project.AnimationClipLibrary
#define ID_Project_ScreenLibrary 0x96b5dfdf // Project.ScreenLibrary
#define ID_Project_MaterialTypeLibrary 0xc8162e4c // Project.MaterialTypeLibrary
#define ID_Project_MaterialLibrary 0xe224a5f4 // Project.MaterialLibrary
#define ID_Project_BrushLibrary 0x71e8c14b // Project.BrushLibrary
#define ID_Project_MeshLibrary 0xfb24db10 // Project.MeshLibrary
#define ID_Project_TimelineSequenceLibrary 0x02be7185 // Project.TimelineSequenceLibrary
#define ID_Project_SceneObjectLibrary 0xa7d7f772 // Project.SceneObjectLibrary
#define ID_Project_ComposerLibrary 0xb1abe3ad // Project.ComposerLibrary
#define ID_Project_PipelineItemLibrary 0x01518b58 // Project.PipelineItemLibrary
#define ID_Project_SceneLibrary 0x81ceeab3 // Project.SceneLibrary
#define ID_Project_TrajectoryLibrary 0xbe5cb9f4 // Project.TrajectoryLibrary
#define ID_Project_TransitionLibrary 0x93a3d926 // Project.TransitionLibrary
#define ID_Project_SplineLibrary 0xd6e6a60c // Project.SplineLibrary
#define ID_Project_PrefabLibrary 0xedb47045 // Project.PrefabLibrary
#define ID_Project_ProfileLibrary 0x77ddc960 // Project.ProfileLibrary
#define ID_Project_ShortcutLibrary 0xe90dcf95 // Project.ShortcutLibrary
#define ID_Project_LayerLibrary 0x9182cf80 // Project.LayerLibrary
#define ID_Project_AnimationLibrary 0xd329f231 // Project.AnimationLibrary
#define ID_Project_TagLibrary 0xab31a12b // Project.TagLibrary
#define ID_Project_ThemeLibrary 0xa48d59a0 // Project.ThemeLibrary
#define ID_Project_ResourceExportTagLibrary 0x604432ed // Project.ResourceExportTagLibrary
#define ID_Project_LocaleLibrary 0x6c5cabed // Project.LocaleLibrary
#define ID_Project_DataSourceLibrary 0x925844f8 // Project.DataSourceLibrary
#define ID_Project_PageTransitionCollectionLibrary 0x2b659913 // Project.PageTransitionCollectionLibrary
#define ID_Project_TextureLibrary 0x29113c3e // Project.TextureLibrary
#define ID_Project_StyleLibrary 0x931b9b40 // Project.StyleLibrary
#define ID_Project_StateManagerLibrary 0x7cb98ef7 // Project.StateManagerLibrary
#define ID_Project_ConnectServiceLibrary 0x658a1168 // Project.ConnectServiceLibrary
#define ID_Project_ConnectUserServiceLibrary 0xd2d03423 // Project.ConnectUserServiceLibrary
#define ID_Project_SpriteLibrary 0x3f9a6f04 // Project.SpriteLibrary
#define ID_Project_SpriteAnimationLibrary 0x664d619c // Project.SpriteAnimationLibrary
#define ID_Project_ImageLibrary 0x07fed878 // Project.ImageLibrary
#define ID_Project_FontLibrary 0x8f87dd48 // Project.FontLibrary
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
	kProjectEnginePlugins,
	kProjectNumEnginePlugins,
	kProjectAnimationClipLibrary,
	kProjectScreenLibrary,
	kProjectMaterialTypeLibrary,
	kProjectMaterialLibrary,
	kProjectBrushLibrary,
	kProjectMeshLibrary,
	kProjectTimelineSequenceLibrary,
	kProjectSceneObjectLibrary,
	kProjectComposerLibrary,
	kProjectPipelineItemLibrary,
	kProjectSceneLibrary,
	kProjectTrajectoryLibrary,
	kProjectTransitionLibrary,
	kProjectSplineLibrary,
	kProjectPrefabLibrary,
	kProjectProfileLibrary,
	kProjectShortcutLibrary,
	kProjectLayerLibrary,
	kProjectAnimationLibrary,
	kProjectTagLibrary,
	kProjectThemeLibrary,
	kProjectResourceExportTagLibrary,
	kProjectLocaleLibrary,
	kProjectDataSourceLibrary,
	kProjectPageTransitionCollectionLibrary,
	kProjectTextureLibrary,
	kProjectStyleLibrary,
	kProjectStateManagerLibrary,
	kProjectConnectServiceLibrary,
	kProjectConnectUserServiceLibrary,
	kProjectSpriteLibrary,
	kProjectSpriteAnimationLibrary,
	kProjectImageLibrary,
	kProjectFontLibrary,
	kProjectNumProperties	
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
// ThemeGroup
#define ID_ThemeGroup 0xa8bfcd45
#define GetThemeGroup(_P) ((struct ThemeGroup*)((_P)?OBJ_GetComponent(_P,ID_ThemeGroup):NULL))
#define ThemeGroup_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_ThemeGroup,sizeof(struct ThemeGroup),_N)
#define ID_ThemeGroup_SelectedTheme 0xfdc31565 // ThemeGroup.SelectedTheme
#define ID_ThemeGroup_SelectedDictionary 0x6de33964 // ThemeGroup.SelectedDictionary
enum ThemeGroupProperties {
	kThemeGroupSelectedTheme,
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