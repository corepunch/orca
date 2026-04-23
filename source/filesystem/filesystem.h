// Auto-generated from filesystem.xml by tools/templates/header.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;

struct Object;
struct _PACK;


#include "filesystem_properties.h"
#include "../core/core.h"

typedef struct Workspace_ReadCommandsEventArgs Workspace_ReadCommandsMsg_t,* Workspace_ReadCommandsMsgPtr;
typedef struct Project_OpenFileEventArgs Project_OpenFileMsg_t,* Project_OpenFileMsgPtr;
typedef struct Project_FileExistsEventArgs Project_FileExistsMsg_t,* Project_FileExistsMsgPtr;
typedef struct Project_HasChangedFilesEventArgs Project_HasChangedFilesMsg_t,* Project_HasChangedFilesMsgPtr;
typedef struct Project_LoadProjectEventArgs Project_LoadProjectMsg_t,* Project_LoadProjectMsgPtr;


typedef struct ProjectReference ProjectReference_t, *lpProjectReference_t;
typedef struct ProjectReference const cProjectReference_t, *lpcProjectReference_t;
typedef struct EnginePlugin EnginePlugin_t, *lpEnginePlugin_t;
typedef struct EnginePlugin const cEnginePlugin_t, *lpcEnginePlugin_t;
typedef struct SystemMessage SystemMessage_t, *lpSystemMessage_t;
typedef struct SystemMessage const cSystemMessage_t, *lpcSystemMessage_t;
typedef struct ThemeValue ThemeValue_t, *lpThemeValue_t;
typedef struct ThemeValue const cThemeValue_t, *lpcThemeValue_t;


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

/// @brief Reads the contents of a text file
ORCA_API int
FS_ReadTextFile(struct lua_State*, const char*);

/// @brief Loads an object tree from an XML file
ORCA_API struct Object*
FS_LoadObjectFromXML(const char*);

/// @brief Parses an object tree from an XML string
ORCA_API struct Object*
FS_ParseObjectFromXMLString(const char*);

/// @brief Retrieves a theme value based on the provided key and theme group
ORCA_API const char*
FS_GetThemeValue(const char*);


/// @brief External project reference
/** ProjectReference struct */
struct ProjectReference {
	const char* Name; ///< Name of the project, will be used as Project/Library/Resource when referencing it's resources
	const char* Path; ///< Path to the project relative to the workspace
};
ORCA_API void luaX_pushProjectReference(lua_State *L, struct ProjectReference const* ProjectReference);
ORCA_API struct ProjectReference* luaX_checkProjectReference(lua_State *L, int idx);
/// @brief Plugin requirement
/** EnginePlugin struct */
struct EnginePlugin {
	const char* Name; ///< Name of the plugin
};
ORCA_API void luaX_pushEnginePlugin(lua_State *L, struct EnginePlugin const* EnginePlugin);
ORCA_API struct EnginePlugin* luaX_checkEnginePlugin(lua_State *L, int idx);
/// @brief Handler of system messages you can add to your project
/** SystemMessage struct */
struct SystemMessage {
	const char* Message; ///< Message name, i.e. KeyDown
	const char* Key; ///< Associated key for the message, if applicable
	const char* Command; ///< Command to execute when the message is received
};
ORCA_API void luaX_pushSystemMessage(lua_State *L, struct SystemMessage const* SystemMessage);
ORCA_API struct SystemMessage* luaX_checkSystemMessage(lua_State *L, int idx);
/// @brief Key-value pair for theme values
/** ThemeValue struct */
struct ThemeValue {
	const char* Key; ///< The key identifying the theme value, such as "panel-background"
	const char* Value; ///< The value associated with the key, such as a color code like "#1A1A28"
};
ORCA_API void luaX_pushThemeValue(lua_State *L, struct ThemeValue const* ThemeValue);
ORCA_API struct ThemeValue* luaX_checkThemeValue(lua_State *L, int idx);

/** Workspace_ReadCommandsEventArgs struct */
struct Workspace_ReadCommandsEventArgs {
};
ORCA_API void luaX_pushWorkspace_ReadCommandsEventArgs(lua_State *L, struct Workspace_ReadCommandsEventArgs const* data);
ORCA_API struct Workspace_ReadCommandsEventArgs* luaX_checkWorkspace_ReadCommandsEventArgs(lua_State *L, int idx);
/** Project_OpenFileEventArgs struct */
struct Project_OpenFileEventArgs {
	const char* FileName;
};
ORCA_API void luaX_pushProject_OpenFileEventArgs(lua_State *L, struct Project_OpenFileEventArgs const* data);
ORCA_API struct Project_OpenFileEventArgs* luaX_checkProject_OpenFileEventArgs(lua_State *L, int idx);
/** Project_FileExistsEventArgs struct */
struct Project_FileExistsEventArgs {
	const char* FileName; ///< The file path to check for existence
};
ORCA_API void luaX_pushProject_FileExistsEventArgs(lua_State *L, struct Project_FileExistsEventArgs const* data);
ORCA_API struct Project_FileExistsEventArgs* luaX_checkProject_FileExistsEventArgs(lua_State *L, int idx);
/** Project_HasChangedFilesEventArgs struct */
struct Project_HasChangedFilesEventArgs {
};
ORCA_API void luaX_pushProject_HasChangedFilesEventArgs(lua_State *L, struct Project_HasChangedFilesEventArgs const* data);
ORCA_API struct Project_HasChangedFilesEventArgs* luaX_checkProject_HasChangedFilesEventArgs(lua_State *L, int idx);
/** Project_LoadProjectEventArgs struct */
struct Project_LoadProjectEventArgs {
	const char* Path; ///< Directory name to load the bundle
};
ORCA_API void luaX_pushProject_LoadProjectEventArgs(lua_State *L, struct Project_LoadProjectEventArgs const* data);
ORCA_API struct Project_LoadProjectEventArgs* luaX_checkProject_LoadProjectEventArgs(lua_State *L, int idx);

/** Workspace component */
typedef struct Workspace Workspace_t, *WorkspacePtr, *lpWorkspace_t;
typedef struct Workspace const *WorkspaceCPtr, *lpcWorkspace_t;
struct Workspace {
	event_t ReadCommands;
};
ORCA_API void luaX_pushWorkspace(lua_State *L, struct Workspace const* Workspace);
ORCA_API struct Workspace* luaX_checkWorkspace(lua_State *L, int idx);
/** Library component */
typedef struct Library Library_t, *LibraryPtr, *lpLibrary_t;
typedef struct Library const *LibraryCPtr, *lpcLibrary_t;
struct Library {
	bool_t IsExternal;
};
ORCA_API void luaX_pushLibrary(lua_State *L, struct Library const* Library);
ORCA_API struct Library* luaX_checkLibrary(lua_State *L, int idx);
/** Project component */
typedef struct Project Project_t, *ProjectPtr, *lpProject_t;
typedef struct Project const *ProjectCPtr, *lpcProject_t;
struct Project {
	bool_t HalfFloatTextureFormat;
	bool_t HalfFloatTextureFormatLinear;
	bool_t HalfFloatColorAttachment;
	bool_t RenderToMipmapLevels;
	bool_t ExternalTexture;
	const char* StartupScreen;
	const char* StartupViewController;
	struct color PreviewWindowBackgroundColor;
	int32_t MessageLimitPerFrame;
	int32_t GlobalTimelineStartTime;
	int32_t GlobalTimelineEndTime;
	const char* BinaryExportDirectory;
	const char* ApplicationExportDirectory;
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
	bool_t ProjectUsePremultipliedAlpha;
	bool_t ProjectRemoveICCProfilesOfPngs;
	const char* BinaryFileName;
	bool_t IsAssetPackage;
	bool_t KanziConnectEnabled;
	const char* DefaultMaterial;
	int32_t WindowWidth;
	int32_t WindowHeight;
	struct PropertyType* PropertyTypes;
	int32_t NumPropertyTypes;
	struct ProjectReference* ProjectReferences;
	int32_t NumProjectReferences;
	struct SystemMessage* SystemMessages;
	int32_t NumSystemMessages;
	struct EnginePlugin* EnginePlugins;
	int32_t NumEnginePlugins;
	struct Library* AnimationClipLibrary;
	struct Library* ScreenLibrary;
	struct Library* MaterialTypeLibrary;
	struct Library* MaterialLibrary;
	struct Library* BrushLibrary;
	struct Library* MeshLibrary;
	struct Library* TimelineSequenceLibrary;
	struct Library* SceneObjectLibrary;
	struct Library* ComposerLibrary;
	struct Library* PipelineItemLibrary;
	struct Library* SceneLibrary;
	struct Library* TrajectoryLibrary;
	struct Library* TransitionLibrary;
	struct Library* SplineLibrary;
	struct Library* PrefabLibrary;
	struct Library* ProfileLibrary;
	struct Library* ShortcutLibrary;
	struct Library* LayerLibrary;
	struct Library* AnimationLibrary;
	struct Library* TagLibrary;
	struct ThemeValue* ThemeLibrary;
	int32_t NumThemeLibrary;
	struct Library* ResourceExportTagLibrary;
	struct Library* LocaleLibrary;
	struct Library* DataSourceLibrary;
	struct Library* PageTransitionCollectionLibrary;
	struct Library* TextureLibrary;
	struct Library* StyleLibrary;
	struct Library* StateManagerLibrary;
	struct Library* ConnectServiceLibrary;
	struct Library* ConnectUserServiceLibrary;
	struct Library* SpriteLibrary;
	struct Library* SpriteAnimationLibrary;
	struct Library* ImageLibrary;
	struct Library* FontLibrary;
	bool_t isPackage;
	event_t OpenFile;
	event_t FileExists;
	event_t HasChangedFiles;
	event_t LoadProject;
};
ORCA_API void luaX_pushProject(lua_State *L, struct Project const* Project);
ORCA_API struct Project* luaX_checkProject(lua_State *L, int idx);
/** Directory component */
typedef struct Directory Directory_t, *DirectoryPtr, *lpDirectory_t;
typedef struct Directory const *DirectoryCPtr, *lpcDirectory_t;
struct Directory {
	const char* Path;
	void* _monitoredfiles;
};
ORCA_API void luaX_pushDirectory(lua_State *L, struct Directory const* Directory);
ORCA_API struct Directory* luaX_checkDirectory(lua_State *L, int idx);
/** Package component */
typedef struct Package Package_t, *PackagePtr, *lpPackage_t;
typedef struct Package const *PackageCPtr, *lpcPackage_t;
struct Package {
	const char* FileName;
	struct _PACK* _package;
};
ORCA_API void luaX_pushPackage(lua_State *L, struct Package const* Package);
ORCA_API struct Package* luaX_checkPackage(lua_State *L, int idx);
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