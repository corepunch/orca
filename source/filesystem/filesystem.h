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

ORCA_API extern struct MessageType ReadCommandsMessage;
ORCA_API extern struct MessageType OpenFileMessage;
ORCA_API extern struct MessageType FileExistsMessage;
ORCA_API extern struct MessageType HasChangedFilesMessage;
ORCA_API extern struct MessageType LoadProjectMessage;

typedef struct ReadCommandsMsgArgs ReadCommandsMsg_t,* ReadCommandsMsgPtr;
typedef struct OpenFileMsgArgs OpenFileMsg_t,* OpenFileMsgPtr;
typedef struct FileExistsMsgArgs FileExistsMsg_t,* FileExistsMsgPtr;
typedef struct HasChangedFilesMsgArgs HasChangedFilesMsg_t,* HasChangedFilesMsgPtr;
typedef struct LoadProjectMsgArgs LoadProjectMsg_t,* LoadProjectMsgPtr;


typedef struct ProjectReference ProjectReference_t, *lpProjectReference_t;
typedef struct ProjectReference const cProjectReference_t, *lpcProjectReference_t;
typedef struct EnginePlugin EnginePlugin_t, *lpEnginePlugin_t;
typedef struct EnginePlugin const cEnginePlugin_t, *lpcEnginePlugin_t;
typedef struct SystemMessage SystemMessage_t, *lpSystemMessage_t;
typedef struct SystemMessage const cSystemMessage_t, *lpcSystemMessage_t;


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

/** ReadCommandsMsgArgs struct */
struct ReadCommandsMsgArgs {
};
ORCA_API void luaX_pushReadCommandsMsgArgs(lua_State *L, struct ReadCommandsMsgArgs const* data);
ORCA_API struct ReadCommandsMsgArgs* luaX_checkReadCommandsMsgArgs(lua_State *L, int idx);
/** OpenFileMsgArgs struct */
struct OpenFileMsgArgs {
	const char* FileName;
};
ORCA_API void luaX_pushOpenFileMsgArgs(lua_State *L, struct OpenFileMsgArgs const* data);
ORCA_API struct OpenFileMsgArgs* luaX_checkOpenFileMsgArgs(lua_State *L, int idx);
/** FileExistsMsgArgs struct */
struct FileExistsMsgArgs {
	const char* FileName; ///< The file path to check for existence
};
ORCA_API void luaX_pushFileExistsMsgArgs(lua_State *L, struct FileExistsMsgArgs const* data);
ORCA_API struct FileExistsMsgArgs* luaX_checkFileExistsMsgArgs(lua_State *L, int idx);
/** HasChangedFilesMsgArgs struct */
struct HasChangedFilesMsgArgs {
};
ORCA_API void luaX_pushHasChangedFilesMsgArgs(lua_State *L, struct HasChangedFilesMsgArgs const* data);
ORCA_API struct HasChangedFilesMsgArgs* luaX_checkHasChangedFilesMsgArgs(lua_State *L, int idx);
/** LoadProjectMsgArgs struct */
struct LoadProjectMsgArgs {
	const char* Path; ///< Directory name to load the bundle
};
ORCA_API void luaX_pushLoadProjectMsgArgs(lua_State *L, struct LoadProjectMsgArgs const* data);
ORCA_API struct LoadProjectMsgArgs* luaX_checkLoadProjectMsgArgs(lua_State *L, int idx);

/** Workspace component */
typedef struct Workspace Workspace_t, *WorkspacePtr, *lpWorkspace_t;
typedef struct Workspace const *WorkspaceCPtr, *lpcWorkspace_t;
struct Workspace {
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
	struct Library* ThemeLibrary;
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
/** ThemeGroup component */
typedef struct ThemeGroup ThemeGroup_t, *ThemeGroupPtr, *lpThemeGroup_t;
typedef struct ThemeGroup const *ThemeGroupCPtr, *lpcThemeGroup_t;
struct ThemeGroup {
	const char* SelectedTheme;
	const char* SelectedDictionary;
	struct Object* _selectedTheme;
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