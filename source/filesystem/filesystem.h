#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;

struct Object;
struct _PACK;


#include "filesystem_properties.h"
#include "../core/core.h"

typedef void* ReadCommandsEventPtr;
typedef struct OpenFileArgs* OpenFileEventPtr;
typedef struct FileExistsArgs* FileExistsEventPtr;
typedef void* HasChangedFilesEventPtr;
typedef struct LoadProjectArgs* LoadProjectEventPtr;


typedef struct ProjectReference ProjectReference_t, *lpProjectReference_t;
typedef struct ProjectReference const cProjectReference_t, *lpcProjectReference_t;
typedef struct ProjectPlugin ProjectPlugin_t, *lpProjectPlugin_t;
typedef struct ProjectPlugin const cProjectPlugin_t, *lpcProjectPlugin_t;
typedef struct SystemMessage SystemMessage_t, *lpSystemMessage_t;
typedef struct SystemMessage const cSystemMessage_t, *lpcSystemMessage_t;
typedef struct OpenFileArgs OpenFileArgs_t, *lpOpenFileArgs_t;
typedef struct OpenFileArgs const cOpenFileArgs_t, *lpcOpenFileArgs_t;
typedef struct FileExistsArgs FileExistsArgs_t, *lpFileExistsArgs_t;
typedef struct FileExistsArgs const cFileExistsArgs_t, *lpcFileExistsArgs_t;
typedef struct LoadProjectArgs LoadProjectArgs_t, *lpLoadProjectArgs_t;
typedef struct LoadProjectArgs const cLoadProjectArgs_t, *lpcLoadProjectArgs_t;

/// @brief Gets the base filename from a path
ORCA_API const char*
FS_GetBaseName(const char*);
/// @brief Extracts the directory path from a full file path
ORCA_API const char*
FS_GetDirName(const char*, const char*, int32_t);
/// @brief Joins two path segments into a complete path
ORCA_API const char*
FS_JoinPaths(const char*, int32_t, const char*, const char*);
/// @brief Converts a module name to a file path
ORCA_API const char*
FS_PathFromModule(const char*);
/// @brief Checks if any tracked files have been modified
ORCA_API bool_t
FS_HasChangedFiles(void);
/// @brief Creates a directory at the specified path
ORCA_API bool_t
FS_MakeDirectory(const char*);
/// @brief Checks if a file exists at the specified path
ORCA_API bool_t
FS_FileExists(const char*);
/// @brief Adds a package to the resource search path
ORCA_API struct Object*
FS_LoadBundle(struct lua_State*, const char*);
/// @brief Sets the current workspace
ORCA_API void
FS_SetWorkspace(struct Object*);
/// @brief Gets the current workspace
ORCA_API struct Object*
FS_GetWorkspace(void);
/// @brief Registers object in the workspace tree
ORCA_API void
FS_RegisterObject(struct Object*, const char*);


/// @brief External project reference
/** ProjectReference struct */
struct ProjectReference {
	fixedString_t Name; ///< Name of the project, will be used as Project/Library/Resource when referencing it's resources
	fixedString_t Path; ///< Path to the project relative to the workspace
};
ORCA_API void luaX_pushProjectReference(lua_State *L, struct ProjectReference const* ProjectReference);
ORCA_API struct ProjectReference* luaX_checkProjectReference(lua_State *L, int idx);
/// @brief Plugin requirement
/** ProjectPlugin struct */
struct ProjectPlugin {
	fixedString_t Name; ///< Name of the plugin
};
ORCA_API void luaX_pushProjectPlugin(lua_State *L, struct ProjectPlugin const* ProjectPlugin);
ORCA_API struct ProjectPlugin* luaX_checkProjectPlugin(lua_State *L, int idx);
/// @brief Handler of system messages you can add to your project
/** SystemMessage struct */
struct SystemMessage {
	fixedString_t Message; ///< Message name, i.e. KeyDown
	fixedString_t Key; ///< Associated key for the message, if applicable
	fixedString_t Command; ///< Command to execute when the message is received
};
ORCA_API void luaX_pushSystemMessage(lua_State *L, struct SystemMessage const* SystemMessage);
ORCA_API struct SystemMessage* luaX_checkSystemMessage(lua_State *L, int idx);
/** OpenFileArgs struct */
struct OpenFileArgs {
	const char* FileName;
};
ORCA_API void luaX_pushOpenFileArgs(lua_State *L, struct OpenFileArgs const* OpenFileArgs);
ORCA_API struct OpenFileArgs* luaX_checkOpenFileArgs(lua_State *L, int idx);
/// @brief Checks if a file exists at the specified path
/** FileExistsArgs struct */
struct FileExistsArgs {
	const char* FileName; ///< The file path to check for existence
};
ORCA_API void luaX_pushFileExistsArgs(lua_State *L, struct FileExistsArgs const* FileExistsArgs);
ORCA_API struct FileExistsArgs* luaX_checkFileExistsArgs(lua_State *L, int idx);
/// @brief Loads a bundle from the specified path
/** LoadProjectArgs struct */
struct LoadProjectArgs {
	const char* Path; ///< Directory name to load the bundle
};
ORCA_API void luaX_pushLoadProjectArgs(lua_State *L, struct LoadProjectArgs const* LoadProjectArgs);
ORCA_API struct LoadProjectArgs* luaX_checkLoadProjectArgs(lua_State *L, int idx);

/// @brief Base class for all bundles
/** Bundle component */
typedef struct Bundle Bundle_t, *BundlePtr, *lpBundle_t;
typedef struct Bundle const *BundleCPtr, *lpcBundle_t;
struct Bundle {
};
ORCA_API void luaX_pushBundle(lua_State *L, struct Bundle const* Bundle);
ORCA_API struct Bundle* luaX_checkBundle(lua_State *L, int idx);
/** Directory component */
typedef struct Directory Directory_t, *DirectoryPtr, *lpDirectory_t;
typedef struct Directory const *DirectoryCPtr, *lpcDirectory_t;
struct Directory {
	fixedString_t Path;
	void* _monitoredfiles;
};
ORCA_API void luaX_pushDirectory(lua_State *L, struct Directory const* Directory);
ORCA_API struct Directory* luaX_checkDirectory(lua_State *L, int idx);
/** Package component */
typedef struct Package Package_t, *PackagePtr, *lpPackage_t;
typedef struct Package const *PackageCPtr, *lpcPackage_t;
struct Package {
	fixedString_t FileName;
	struct _PACK* _package;
};
ORCA_API void luaX_pushPackage(lua_State *L, struct Package const* Package);
ORCA_API struct Package* luaX_checkPackage(lua_State *L, int idx);
/** Workspace component */
typedef struct Workspace Workspace_t, *WorkspacePtr, *lpWorkspace_t;
typedef struct Workspace const *WorkspaceCPtr, *lpcWorkspace_t;
struct Workspace {
};
ORCA_API void luaX_pushWorkspace(lua_State *L, struct Workspace const* Workspace);
ORCA_API struct Workspace* luaX_checkWorkspace(lua_State *L, int idx);
/** Project component */
typedef struct Project Project_t, *ProjectPtr, *lpProject_t;
typedef struct Project const *ProjectCPtr, *lpcProject_t;
struct Project {
	bool_t HalfFloatTextureFormat;
	bool_t HalfFloatTextureFormatLinear;
	bool_t HalfFloatColorAttachment;
	bool_t RenderToMipmapLevels;
	bool_t ExternalTexture;
	fixedString_t StartupScreen;
	struct color PreviewWindowBackgroundColor;
	int32_t MessageLimitPerFrame;
	int32_t GlobalTimelineStartTime;
	int32_t GlobalTimelineEndTime;
	fixedString_t BinaryExportDirectory;
	fixedString_t ApplicationExportDirectory;
	bool_t IsMasterProject;
	bool_t OptimizeMeshes;
	int32_t TargetPlatformVertexCacheSize;
	bool_t PlotAnimations;
	bool_t RoundImagesToNearestPowerOf2;
	bool_t ProjectExportShaderSourceCode;
	bool_t ProjectExportMainKzbWithBakedThemes;
	bool_t ProjectGroupByThemeNameInBakedThemeExport;
	bool_t IsLocalizationEnabled;
	bool_t FullScreenPreviewLayer;
	bool_t ShowChildrenInLayerThumbnails;
	struct vec2 CompositionDesignSize;
	bool_t ProjectUsePremultipliedAlpha;
	bool_t ProjectRemoveICCProfilesOfPngs;
	fixedString_t BinaryFileName;
	bool_t IsAssetPackage;
	bool_t KanziConnectEnabled;
	fixedString_t DefaultMaterial;
	int32_t WindowWidth;
	int32_t WindowHeight;
	struct PropertyType* PropertyTypes;
	int32_t NumPropertyTypes;
	struct ProjectReference* ProjectReferences;
	int32_t NumProjectReferences;
	struct SystemMessage* SystemMessages;
	int32_t NumSystemMessages;
	struct ProjectPlugin* Plugins;
	int32_t NumPlugins;
	bool_t isPackage;
};
ORCA_API void luaX_pushProject(lua_State *L, struct Project const* Project);
ORCA_API struct Project* luaX_checkProject(lua_State *L, int idx);
/** Library component */
typedef struct Library Library_t, *LibraryPtr, *lpLibrary_t;
typedef struct Library const *LibraryCPtr, *lpcLibrary_t;
struct Library {
	bool_t IsExternal;
};
ORCA_API void luaX_pushLibrary(lua_State *L, struct Library const* Library);
ORCA_API struct Library* luaX_checkLibrary(lua_State *L, int idx);
/** AnimationClipLibrary component */
typedef struct AnimationClipLibrary AnimationClipLibrary_t, *AnimationClipLibraryPtr, *lpAnimationClipLibrary_t;
typedef struct AnimationClipLibrary const *AnimationClipLibraryCPtr, *lpcAnimationClipLibrary_t;
struct AnimationClipLibrary {
};
ORCA_API void luaX_pushAnimationClipLibrary(lua_State *L, struct AnimationClipLibrary const* AnimationClipLibrary);
ORCA_API struct AnimationClipLibrary* luaX_checkAnimationClipLibrary(lua_State *L, int idx);
/** ScreenLibrary component */
typedef struct ScreenLibrary ScreenLibrary_t, *ScreenLibraryPtr, *lpScreenLibrary_t;
typedef struct ScreenLibrary const *ScreenLibraryCPtr, *lpcScreenLibrary_t;
struct ScreenLibrary {
};
ORCA_API void luaX_pushScreenLibrary(lua_State *L, struct ScreenLibrary const* ScreenLibrary);
ORCA_API struct ScreenLibrary* luaX_checkScreenLibrary(lua_State *L, int idx);
/** MaterialTypeLibrary component */
typedef struct MaterialTypeLibrary MaterialTypeLibrary_t, *MaterialTypeLibraryPtr, *lpMaterialTypeLibrary_t;
typedef struct MaterialTypeLibrary const *MaterialTypeLibraryCPtr, *lpcMaterialTypeLibrary_t;
struct MaterialTypeLibrary {
};
ORCA_API void luaX_pushMaterialTypeLibrary(lua_State *L, struct MaterialTypeLibrary const* MaterialTypeLibrary);
ORCA_API struct MaterialTypeLibrary* luaX_checkMaterialTypeLibrary(lua_State *L, int idx);
/** MaterialLibrary component */
typedef struct MaterialLibrary MaterialLibrary_t, *MaterialLibraryPtr, *lpMaterialLibrary_t;
typedef struct MaterialLibrary const *MaterialLibraryCPtr, *lpcMaterialLibrary_t;
struct MaterialLibrary {
};
ORCA_API void luaX_pushMaterialLibrary(lua_State *L, struct MaterialLibrary const* MaterialLibrary);
ORCA_API struct MaterialLibrary* luaX_checkMaterialLibrary(lua_State *L, int idx);
/** BrushLibrary component */
typedef struct BrushLibrary BrushLibrary_t, *BrushLibraryPtr, *lpBrushLibrary_t;
typedef struct BrushLibrary const *BrushLibraryCPtr, *lpcBrushLibrary_t;
struct BrushLibrary {
};
ORCA_API void luaX_pushBrushLibrary(lua_State *L, struct BrushLibrary const* BrushLibrary);
ORCA_API struct BrushLibrary* luaX_checkBrushLibrary(lua_State *L, int idx);
/** MeshLibrary component */
typedef struct MeshLibrary MeshLibrary_t, *MeshLibraryPtr, *lpMeshLibrary_t;
typedef struct MeshLibrary const *MeshLibraryCPtr, *lpcMeshLibrary_t;
struct MeshLibrary {
};
ORCA_API void luaX_pushMeshLibrary(lua_State *L, struct MeshLibrary const* MeshLibrary);
ORCA_API struct MeshLibrary* luaX_checkMeshLibrary(lua_State *L, int idx);
/** TimelineSequenceLibrary component */
typedef struct TimelineSequenceLibrary TimelineSequenceLibrary_t, *TimelineSequenceLibraryPtr, *lpTimelineSequenceLibrary_t;
typedef struct TimelineSequenceLibrary const *TimelineSequenceLibraryCPtr, *lpcTimelineSequenceLibrary_t;
struct TimelineSequenceLibrary {
};
ORCA_API void luaX_pushTimelineSequenceLibrary(lua_State *L, struct TimelineSequenceLibrary const* TimelineSequenceLibrary);
ORCA_API struct TimelineSequenceLibrary* luaX_checkTimelineSequenceLibrary(lua_State *L, int idx);
/** SceneObjectLibrary component */
typedef struct SceneObjectLibrary SceneObjectLibrary_t, *SceneObjectLibraryPtr, *lpSceneObjectLibrary_t;
typedef struct SceneObjectLibrary const *SceneObjectLibraryCPtr, *lpcSceneObjectLibrary_t;
struct SceneObjectLibrary {
};
ORCA_API void luaX_pushSceneObjectLibrary(lua_State *L, struct SceneObjectLibrary const* SceneObjectLibrary);
ORCA_API struct SceneObjectLibrary* luaX_checkSceneObjectLibrary(lua_State *L, int idx);
/** ComposerLibrary component */
typedef struct ComposerLibrary ComposerLibrary_t, *ComposerLibraryPtr, *lpComposerLibrary_t;
typedef struct ComposerLibrary const *ComposerLibraryCPtr, *lpcComposerLibrary_t;
struct ComposerLibrary {
};
ORCA_API void luaX_pushComposerLibrary(lua_State *L, struct ComposerLibrary const* ComposerLibrary);
ORCA_API struct ComposerLibrary* luaX_checkComposerLibrary(lua_State *L, int idx);
/** PipelineItemLibrary component */
typedef struct PipelineItemLibrary PipelineItemLibrary_t, *PipelineItemLibraryPtr, *lpPipelineItemLibrary_t;
typedef struct PipelineItemLibrary const *PipelineItemLibraryCPtr, *lpcPipelineItemLibrary_t;
struct PipelineItemLibrary {
};
ORCA_API void luaX_pushPipelineItemLibrary(lua_State *L, struct PipelineItemLibrary const* PipelineItemLibrary);
ORCA_API struct PipelineItemLibrary* luaX_checkPipelineItemLibrary(lua_State *L, int idx);
/** SceneLibrary component */
typedef struct SceneLibrary SceneLibrary_t, *SceneLibraryPtr, *lpSceneLibrary_t;
typedef struct SceneLibrary const *SceneLibraryCPtr, *lpcSceneLibrary_t;
struct SceneLibrary {
};
ORCA_API void luaX_pushSceneLibrary(lua_State *L, struct SceneLibrary const* SceneLibrary);
ORCA_API struct SceneLibrary* luaX_checkSceneLibrary(lua_State *L, int idx);
/** TrajectoryLibrary component */
typedef struct TrajectoryLibrary TrajectoryLibrary_t, *TrajectoryLibraryPtr, *lpTrajectoryLibrary_t;
typedef struct TrajectoryLibrary const *TrajectoryLibraryCPtr, *lpcTrajectoryLibrary_t;
struct TrajectoryLibrary {
};
ORCA_API void luaX_pushTrajectoryLibrary(lua_State *L, struct TrajectoryLibrary const* TrajectoryLibrary);
ORCA_API struct TrajectoryLibrary* luaX_checkTrajectoryLibrary(lua_State *L, int idx);
/** TransitionLibrary component */
typedef struct TransitionLibrary TransitionLibrary_t, *TransitionLibraryPtr, *lpTransitionLibrary_t;
typedef struct TransitionLibrary const *TransitionLibraryCPtr, *lpcTransitionLibrary_t;
struct TransitionLibrary {
};
ORCA_API void luaX_pushTransitionLibrary(lua_State *L, struct TransitionLibrary const* TransitionLibrary);
ORCA_API struct TransitionLibrary* luaX_checkTransitionLibrary(lua_State *L, int idx);
/** SplineLibrary component */
typedef struct SplineLibrary SplineLibrary_t, *SplineLibraryPtr, *lpSplineLibrary_t;
typedef struct SplineLibrary const *SplineLibraryCPtr, *lpcSplineLibrary_t;
struct SplineLibrary {
};
ORCA_API void luaX_pushSplineLibrary(lua_State *L, struct SplineLibrary const* SplineLibrary);
ORCA_API struct SplineLibrary* luaX_checkSplineLibrary(lua_State *L, int idx);
/** PrefabLibrary component */
typedef struct PrefabLibrary PrefabLibrary_t, *PrefabLibraryPtr, *lpPrefabLibrary_t;
typedef struct PrefabLibrary const *PrefabLibraryCPtr, *lpcPrefabLibrary_t;
struct PrefabLibrary {
};
ORCA_API void luaX_pushPrefabLibrary(lua_State *L, struct PrefabLibrary const* PrefabLibrary);
ORCA_API struct PrefabLibrary* luaX_checkPrefabLibrary(lua_State *L, int idx);
/** ProfileLibrary component */
typedef struct ProfileLibrary ProfileLibrary_t, *ProfileLibraryPtr, *lpProfileLibrary_t;
typedef struct ProfileLibrary const *ProfileLibraryCPtr, *lpcProfileLibrary_t;
struct ProfileLibrary {
};
ORCA_API void luaX_pushProfileLibrary(lua_State *L, struct ProfileLibrary const* ProfileLibrary);
ORCA_API struct ProfileLibrary* luaX_checkProfileLibrary(lua_State *L, int idx);
/** EnginePluginLibrary component */
typedef struct EnginePluginLibrary EnginePluginLibrary_t, *EnginePluginLibraryPtr, *lpEnginePluginLibrary_t;
typedef struct EnginePluginLibrary const *EnginePluginLibraryCPtr, *lpcEnginePluginLibrary_t;
struct EnginePluginLibrary {
};
ORCA_API void luaX_pushEnginePluginLibrary(lua_State *L, struct EnginePluginLibrary const* EnginePluginLibrary);
ORCA_API struct EnginePluginLibrary* luaX_checkEnginePluginLibrary(lua_State *L, int idx);
/** ShortcutLibrary component */
typedef struct ShortcutLibrary ShortcutLibrary_t, *ShortcutLibraryPtr, *lpShortcutLibrary_t;
typedef struct ShortcutLibrary const *ShortcutLibraryCPtr, *lpcShortcutLibrary_t;
struct ShortcutLibrary {
};
ORCA_API void luaX_pushShortcutLibrary(lua_State *L, struct ShortcutLibrary const* ShortcutLibrary);
ORCA_API struct ShortcutLibrary* luaX_checkShortcutLibrary(lua_State *L, int idx);
/** LayerLibrary component */
typedef struct LayerLibrary LayerLibrary_t, *LayerLibraryPtr, *lpLayerLibrary_t;
typedef struct LayerLibrary const *LayerLibraryCPtr, *lpcLayerLibrary_t;
struct LayerLibrary {
};
ORCA_API void luaX_pushLayerLibrary(lua_State *L, struct LayerLibrary const* LayerLibrary);
ORCA_API struct LayerLibrary* luaX_checkLayerLibrary(lua_State *L, int idx);
/** AnimationLibrary component */
typedef struct AnimationLibrary AnimationLibrary_t, *AnimationLibraryPtr, *lpAnimationLibrary_t;
typedef struct AnimationLibrary const *AnimationLibraryCPtr, *lpcAnimationLibrary_t;
struct AnimationLibrary {
};
ORCA_API void luaX_pushAnimationLibrary(lua_State *L, struct AnimationLibrary const* AnimationLibrary);
ORCA_API struct AnimationLibrary* luaX_checkAnimationLibrary(lua_State *L, int idx);
/** TagLibrary component */
typedef struct TagLibrary TagLibrary_t, *TagLibraryPtr, *lpTagLibrary_t;
typedef struct TagLibrary const *TagLibraryCPtr, *lpcTagLibrary_t;
struct TagLibrary {
};
ORCA_API void luaX_pushTagLibrary(lua_State *L, struct TagLibrary const* TagLibrary);
ORCA_API struct TagLibrary* luaX_checkTagLibrary(lua_State *L, int idx);
/** ThemeLibrary component */
typedef struct ThemeLibrary ThemeLibrary_t, *ThemeLibraryPtr, *lpThemeLibrary_t;
typedef struct ThemeLibrary const *ThemeLibraryCPtr, *lpcThemeLibrary_t;
struct ThemeLibrary {
};
ORCA_API void luaX_pushThemeLibrary(lua_State *L, struct ThemeLibrary const* ThemeLibrary);
ORCA_API struct ThemeLibrary* luaX_checkThemeLibrary(lua_State *L, int idx);
/** ResourceExportTagLibrary component */
typedef struct ResourceExportTagLibrary ResourceExportTagLibrary_t, *ResourceExportTagLibraryPtr, *lpResourceExportTagLibrary_t;
typedef struct ResourceExportTagLibrary const *ResourceExportTagLibraryCPtr, *lpcResourceExportTagLibrary_t;
struct ResourceExportTagLibrary {
};
ORCA_API void luaX_pushResourceExportTagLibrary(lua_State *L, struct ResourceExportTagLibrary const* ResourceExportTagLibrary);
ORCA_API struct ResourceExportTagLibrary* luaX_checkResourceExportTagLibrary(lua_State *L, int idx);
/** LocaleLibrary component */
typedef struct LocaleLibrary LocaleLibrary_t, *LocaleLibraryPtr, *lpLocaleLibrary_t;
typedef struct LocaleLibrary const *LocaleLibraryCPtr, *lpcLocaleLibrary_t;
struct LocaleLibrary {
};
ORCA_API void luaX_pushLocaleLibrary(lua_State *L, struct LocaleLibrary const* LocaleLibrary);
ORCA_API struct LocaleLibrary* luaX_checkLocaleLibrary(lua_State *L, int idx);
/** DataSourceLibrary component */
typedef struct DataSourceLibrary DataSourceLibrary_t, *DataSourceLibraryPtr, *lpDataSourceLibrary_t;
typedef struct DataSourceLibrary const *DataSourceLibraryCPtr, *lpcDataSourceLibrary_t;
struct DataSourceLibrary {
};
ORCA_API void luaX_pushDataSourceLibrary(lua_State *L, struct DataSourceLibrary const* DataSourceLibrary);
ORCA_API struct DataSourceLibrary* luaX_checkDataSourceLibrary(lua_State *L, int idx);
/** PageTransitionCollectionLibrary component */
typedef struct PageTransitionCollectionLibrary PageTransitionCollectionLibrary_t, *PageTransitionCollectionLibraryPtr, *lpPageTransitionCollectionLibrary_t;
typedef struct PageTransitionCollectionLibrary const *PageTransitionCollectionLibraryCPtr, *lpcPageTransitionCollectionLibrary_t;
struct PageTransitionCollectionLibrary {
};
ORCA_API void luaX_pushPageTransitionCollectionLibrary(lua_State *L, struct PageTransitionCollectionLibrary const* PageTransitionCollectionLibrary);
ORCA_API struct PageTransitionCollectionLibrary* luaX_checkPageTransitionCollectionLibrary(lua_State *L, int idx);
/** TextureLibrary component */
typedef struct TextureLibrary TextureLibrary_t, *TextureLibraryPtr, *lpTextureLibrary_t;
typedef struct TextureLibrary const *TextureLibraryCPtr, *lpcTextureLibrary_t;
struct TextureLibrary {
};
ORCA_API void luaX_pushTextureLibrary(lua_State *L, struct TextureLibrary const* TextureLibrary);
ORCA_API struct TextureLibrary* luaX_checkTextureLibrary(lua_State *L, int idx);
/** StyleLibrary component */
typedef struct StyleLibrary StyleLibrary_t, *StyleLibraryPtr, *lpStyleLibrary_t;
typedef struct StyleLibrary const *StyleLibraryCPtr, *lpcStyleLibrary_t;
struct StyleLibrary {
};
ORCA_API void luaX_pushStyleLibrary(lua_State *L, struct StyleLibrary const* StyleLibrary);
ORCA_API struct StyleLibrary* luaX_checkStyleLibrary(lua_State *L, int idx);
/** StateManagerLibrary component */
typedef struct StateManagerLibrary StateManagerLibrary_t, *StateManagerLibraryPtr, *lpStateManagerLibrary_t;
typedef struct StateManagerLibrary const *StateManagerLibraryCPtr, *lpcStateManagerLibrary_t;
struct StateManagerLibrary {
};
ORCA_API void luaX_pushStateManagerLibrary(lua_State *L, struct StateManagerLibrary const* StateManagerLibrary);
ORCA_API struct StateManagerLibrary* luaX_checkStateManagerLibrary(lua_State *L, int idx);
/** ConnectServiceLibrary component */
typedef struct ConnectServiceLibrary ConnectServiceLibrary_t, *ConnectServiceLibraryPtr, *lpConnectServiceLibrary_t;
typedef struct ConnectServiceLibrary const *ConnectServiceLibraryCPtr, *lpcConnectServiceLibrary_t;
struct ConnectServiceLibrary {
};
ORCA_API void luaX_pushConnectServiceLibrary(lua_State *L, struct ConnectServiceLibrary const* ConnectServiceLibrary);
ORCA_API struct ConnectServiceLibrary* luaX_checkConnectServiceLibrary(lua_State *L, int idx);
/** ConnectUserServiceLibrary component */
typedef struct ConnectUserServiceLibrary ConnectUserServiceLibrary_t, *ConnectUserServiceLibraryPtr, *lpConnectUserServiceLibrary_t;
typedef struct ConnectUserServiceLibrary const *ConnectUserServiceLibraryCPtr, *lpcConnectUserServiceLibrary_t;
struct ConnectUserServiceLibrary {
};
ORCA_API void luaX_pushConnectUserServiceLibrary(lua_State *L, struct ConnectUserServiceLibrary const* ConnectUserServiceLibrary);
ORCA_API struct ConnectUserServiceLibrary* luaX_checkConnectUserServiceLibrary(lua_State *L, int idx);
/** SpriteLibrary component */
typedef struct SpriteLibrary SpriteLibrary_t, *SpriteLibraryPtr, *lpSpriteLibrary_t;
typedef struct SpriteLibrary const *SpriteLibraryCPtr, *lpcSpriteLibrary_t;
struct SpriteLibrary {
};
ORCA_API void luaX_pushSpriteLibrary(lua_State *L, struct SpriteLibrary const* SpriteLibrary);
ORCA_API struct SpriteLibrary* luaX_checkSpriteLibrary(lua_State *L, int idx);
/** SpriteAnimationLibrary component */
typedef struct SpriteAnimationLibrary SpriteAnimationLibrary_t, *SpriteAnimationLibraryPtr, *lpSpriteAnimationLibrary_t;
typedef struct SpriteAnimationLibrary const *SpriteAnimationLibraryCPtr, *lpcSpriteAnimationLibrary_t;
struct SpriteAnimationLibrary {
};
ORCA_API void luaX_pushSpriteAnimationLibrary(lua_State *L, struct SpriteAnimationLibrary const* SpriteAnimationLibrary);
ORCA_API struct SpriteAnimationLibrary* luaX_checkSpriteAnimationLibrary(lua_State *L, int idx);
/** ImageLibrary component */
typedef struct ImageLibrary ImageLibrary_t, *ImageLibraryPtr, *lpImageLibrary_t;
typedef struct ImageLibrary const *ImageLibraryCPtr, *lpcImageLibrary_t;
struct ImageLibrary {
};
ORCA_API void luaX_pushImageLibrary(lua_State *L, struct ImageLibrary const* ImageLibrary);
ORCA_API struct ImageLibrary* luaX_checkImageLibrary(lua_State *L, int idx);
/** FontLibrary component */
typedef struct FontLibrary FontLibrary_t, *FontLibraryPtr, *lpFontLibrary_t;
typedef struct FontLibrary const *FontLibraryCPtr, *lpcFontLibrary_t;
struct FontLibrary {
};
ORCA_API void luaX_pushFontLibrary(lua_State *L, struct FontLibrary const* FontLibrary);
ORCA_API struct FontLibrary* luaX_checkFontLibrary(lua_State *L, int idx);
/** LocaleReferenceItem component */
typedef struct LocaleReferenceItem LocaleReferenceItem_t, *LocaleReferenceItemPtr, *lpLocaleReferenceItem_t;
typedef struct LocaleReferenceItem const *LocaleReferenceItemCPtr, *lpcLocaleReferenceItem_t;
struct LocaleReferenceItem {
};
ORCA_API void luaX_pushLocaleReferenceItem(lua_State *L, struct LocaleReferenceItem const* LocaleReferenceItem);
ORCA_API struct LocaleReferenceItem* luaX_checkLocaleReferenceItem(lua_State *L, int idx);
/** Tag component */
typedef struct Tag Tag_t, *TagPtr, *lpTag_t;
typedef struct Tag const *TagCPtr, *lpcTag_t;
struct Tag {
	bool_t TagIsInherited;
};
ORCA_API void luaX_pushTag(lua_State *L, struct Tag const* Tag);
ORCA_API struct Tag* luaX_checkTag(lua_State *L, int idx);
/** EnginePlugin component */
typedef struct EnginePlugin EnginePlugin_t, *EnginePluginPtr, *lpEnginePlugin_t;
typedef struct EnginePlugin const *EnginePluginCPtr, *lpcEnginePlugin_t;
struct EnginePlugin {
};
ORCA_API void luaX_pushEnginePlugin(lua_State *L, struct EnginePlugin const* EnginePlugin);
ORCA_API struct EnginePlugin* luaX_checkEnginePlugin(lua_State *L, int idx);
/** EngineMetaclass component */
typedef struct EngineMetaclass EngineMetaclass_t, *EngineMetaclassPtr, *lpEngineMetaclass_t;
typedef struct EngineMetaclass const *EngineMetaclassCPtr, *lpcEngineMetaclass_t;
struct EngineMetaclass {
	fixedString_t BaseClassName;
};
ORCA_API void luaX_pushEngineMetaclass(lua_State *L, struct EngineMetaclass const* EngineMetaclass);
ORCA_API struct EngineMetaclass* luaX_checkEngineMetaclass(lua_State *L, int idx);
/** ThemeGroup component */
typedef struct ThemeGroup ThemeGroup_t, *ThemeGroupPtr, *lpThemeGroup_t;
typedef struct ThemeGroup const *ThemeGroupCPtr, *lpcThemeGroup_t;
struct ThemeGroup {
	fixedString_t SelectedDictionary;
};
ORCA_API void luaX_pushThemeGroup(lua_State *L, struct ThemeGroup const* ThemeGroup);
ORCA_API struct ThemeGroup* luaX_checkThemeGroup(lua_State *L, int idx);
/** Theme component */
typedef struct Theme Theme_t, *ThemePtr, *lpTheme_t;
typedef struct Theme const *ThemeCPtr, *lpcTheme_t;
struct Theme {
	bool_t IsThemeVisible;
};
ORCA_API void luaX_pushTheme(lua_State *L, struct Theme const* Theme);
ORCA_API struct Theme* luaX_checkTheme(lua_State *L, int idx);
/** Entry component */
typedef struct Entry Entry_t, *EntryPtr, *lpEntry_t;
typedef struct Entry const *EntryCPtr, *lpcEntry_t;
struct Entry {
};
ORCA_API void luaX_pushEntry(lua_State *L, struct Entry const* Entry);
ORCA_API struct Entry* luaX_checkEntry(lua_State *L, int idx);
/** ThemeDefaultValuesDictionary component */
typedef struct ThemeDefaultValuesDictionary ThemeDefaultValuesDictionary_t, *ThemeDefaultValuesDictionaryPtr, *lpThemeDefaultValuesDictionary_t;
typedef struct ThemeDefaultValuesDictionary const *ThemeDefaultValuesDictionaryCPtr, *lpcThemeDefaultValuesDictionary_t;
struct ThemeDefaultValuesDictionary {
};
ORCA_API void luaX_pushThemeDefaultValuesDictionary(lua_State *L, struct ThemeDefaultValuesDictionary const* ThemeDefaultValuesDictionary);
ORCA_API struct ThemeDefaultValuesDictionary* luaX_checkThemeDefaultValuesDictionary(lua_State *L, int idx);

#endif