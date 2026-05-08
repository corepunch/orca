// Auto-generated from filesystem.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>
#include <include/codegen.h>

#include "filesystem.h"

// Object
extern void luaX_pushObject(lua_State *L, struct Object const* value);
extern struct Object* luaX_checkObject(lua_State *L, int index);
// _PACK
extern void luaX_push_PACK(lua_State *L, struct _PACK const* value);
extern struct _PACK* luaX_check_PACK(lua_State *L, int index);

extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);
static struct PropertyType _ProjectReference[] = {
	DECL_STRING(0x0fe07306, ProjectReference, Name, Name), // ProjectReference.Name
	DECL_STRING(0xeb66e456, ProjectReference, Path, Path), // ProjectReference.Path
};
static luaL_Reg _ProjectReference_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _EnginePlugin[] = {
	DECL_STRING(0x0fe07306, EnginePlugin, Name, Name), // EnginePlugin.Name
};
static luaL_Reg _EnginePlugin_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _SystemMessage[] = {
	DECL_STRING(0xae0ed984, SystemMessage, Message, Message), // SystemMessage.Message
	DECL_STRING(0xcd1ac90c, SystemMessage, Key, Key), // SystemMessage.Key
	DECL_STRING(0xc67c8f52, SystemMessage, Command, Command), // SystemMessage.Command
};
static luaL_Reg _SystemMessage_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _ThemeValue[] = {
	DECL_STRING(0xcd1ac90c, ThemeValue, Key, Key), // ThemeValue.Key
	DECL_STRING(0xd147f96a, ThemeValue, Value, Value), // ThemeValue.Value
};
static luaL_Reg _ThemeValue_Methods[] = {
	{ NULL, NULL }
};

STRUCT(ProjectReference, ProjectReference);
STRUCT(EnginePlugin, EnginePlugin);
STRUCT(SystemMessage, SystemMessage);
STRUCT(ThemeValue, ThemeValue);

static luaL_Reg _Workspace_ReadCommandsEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Workspace_ReadCommandsEventArgs[] = {
};
static luaL_Reg _Project_OpenFileEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Project_OpenFileEventArgs[] = {
	DECL_STRING(0x5ffdd888, Project_OpenFileEventArgs, FileName, FileName), // Project_OpenFileEventArgs.FileName
};
static luaL_Reg _Project_FileExistsEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Project_FileExistsEventArgs[] = {
	DECL_STRING(0x5ffdd888, Project_FileExistsEventArgs, FileName, FileName), // Project_FileExistsEventArgs.FileName
};
static luaL_Reg _Project_HasChangedFilesEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Project_HasChangedFilesEventArgs[] = {
};
static luaL_Reg _Project_LoadProjectEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Project_LoadProjectEventArgs[] = {
	DECL_STRING(0xeb66e456, Project_LoadProjectEventArgs, Path, Path), // Project_LoadProjectEventArgs.Path
};

STRUCT(Workspace_ReadCommandsEventArgs, Workspace_ReadCommandsEventArgs);
STRUCT(Project_OpenFileEventArgs, Project_OpenFileEventArgs);
STRUCT(Project_FileExistsEventArgs, Project_FileExistsEventArgs);
STRUCT(Project_HasChangedFilesEventArgs, Project_HasChangedFilesEventArgs);
STRUCT(Project_LoadProjectEventArgs, Project_LoadProjectEventArgs);
COMPONENT_PROPERTIES(Workspace) = {
	DECL_EVENT(0x23d83fd3, Workspace, ReadCommands), // Workspace.ReadCommands
};
COMPONENT_DEFAULTS(Workspace) = {
};
COMPONENT_PROC_BEGIN(Workspace)
COMPONENT_PROC_END()
void luaX_pushWorkspace(lua_State *L, struct Workspace const* Workspace) {
	luaX_pushObject(L, CMP_GetObject(Workspace));
}
struct Workspace* luaX_checkWorkspace(lua_State *L, int idx) {
	return GetWorkspace(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Workspace, 0);
COMPONENT_PROPERTIES(Library) = {
	DECL_BOOL(0x1cb8f23a, Library, IsExternal, IsExternal), // Library.IsExternal
};
COMPONENT_DEFAULTS(Library) = {
};
COMPONENT_PROC_BEGIN(Library)
COMPONENT_PROC_END()
void luaX_pushLibrary(lua_State *L, struct Library const* Library) {
	luaX_pushObject(L, CMP_GetObject(Library));
}
struct Library* luaX_checkLibrary(lua_State *L, int idx) {
	return GetLibrary(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Library, 0);
HANDLER(Project, Object, Start);
COMPONENT_PROPERTIES(Project) = {
	DECL_BOOL(0xbcd19216, Project, HalfFloatTextureFormat, HalfFloatTextureFormat), // Project.HalfFloatTextureFormat
	DECL_BOOL(0xfba1938f, Project, HalfFloatTextureFormatLinear, HalfFloatTextureFormatLinear), // Project.HalfFloatTextureFormatLinear
	DECL_BOOL(0x1275837c, Project, HalfFloatColorAttachment, HalfFloatColorAttachment), // Project.HalfFloatColorAttachment
	DECL_BOOL(0x6b1586db, Project, RenderToMipmapLevels, RenderToMipmapLevels), // Project.RenderToMipmapLevels
	DECL_BOOL(0x377aea3d, Project, ExternalTexture, ExternalTexture), // Project.ExternalTexture
	DECL_STRING(0xb3cce876, Project, StartupScreen, StartupScreen), // Project.StartupScreen
	DECL_STRING(0x5fac3f0f, Project, StartupViewController, StartupViewController), // Project.StartupViewController
	DECL_COLOR(0xe1bb158a, Project, PreviewWindowBackgroundColor, PreviewWindowBackgroundColor), // Project.PreviewWindowBackgroundColor
	DECL_INT(0xb92de767, Project, MessageLimitPerFrame, MessageLimitPerFrame), // Project.MessageLimitPerFrame
	DECL_INT(0xe310dade, Project, GlobalTimelineStartTime, GlobalTimelineStartTime), // Project.GlobalTimelineStartTime
	DECL_INT(0x5d90d07b, Project, GlobalTimelineEndTime, GlobalTimelineEndTime), // Project.GlobalTimelineEndTime
	DECL_STRING(0x527cf409, Project, BinaryExportDirectory, BinaryExportDirectory), // Project.BinaryExportDirectory
	DECL_STRING(0x5d547d12, Project, ApplicationExportDirectory, ApplicationExportDirectory), // Project.ApplicationExportDirectory
	DECL_BOOL(0xf6c72bc0, Project, IsMasterProject, IsMasterProject), // Project.IsMasterProject
	DECL_BOOL(0x3762ffdf, Project, OptimizeMeshes, OptimizeMeshes), // Project.OptimizeMeshes
	DECL_INT(0xda2eb7cc, Project, TargetPlatformVertexCacheSize, TargetPlatformVertexCacheSize), // Project.TargetPlatformVertexCacheSize
	DECL_BOOL(0xdc968859, Project, PlotAnimations, PlotAnimations), // Project.PlotAnimations
	DECL_BOOL(0x66a084f4, Project, RoundImagesToNearestPowerOf2, RoundImagesToNearestPowerOf2), // Project.RoundImagesToNearestPowerOf2
	DECL_BOOL(0xeb929bed, Project, ProjectExportShaderSourceCode, ProjectExportShaderSourceCode), // Project.ProjectExportShaderSourceCode
	DECL_BOOL(0xc3c2d763, Project, ProjectExportMainKzbWithBakedThemes, ProjectExportMainKzbWithBakedThemes), // Project.ProjectExportMainKzbWithBakedThemes
	DECL_BOOL(0xad8222eb, Project, ProjectGroupByThemeNameInBakedThemeExport, ProjectGroupByThemeNameInBakedThemeExport), // Project.ProjectGroupByThemeNameInBakedThemeExport
	DECL_BOOL(0x18da08d1, Project, IsLocalizationEnabled, IsLocalizationEnabled), // Project.IsLocalizationEnabled
	DECL_BOOL(0xdf498299, Project, FullScreenPreviewLayer, FullScreenPreviewLayer), // Project.FullScreenPreviewLayer
	DECL_BOOL(0x76bfa6c4, Project, ShowChildrenInLayerThumbnails, ShowChildrenInLayerThumbnails), // Project.ShowChildrenInLayerThumbnails
	DECL_BOOL(0x5734b5cd, Project, ProjectUsePremultipliedAlpha, ProjectUsePremultipliedAlpha), // Project.ProjectUsePremultipliedAlpha
	DECL_BOOL(0xb617d580, Project, ProjectRemoveICCProfilesOfPngs, ProjectRemoveICCProfilesOfPngs), // Project.ProjectRemoveICCProfilesOfPngs
	DECL_STRING(0x0f9ae251, Project, BinaryFileName, BinaryFileName), // Project.BinaryFileName
	DECL_BOOL(0x76ac7909, Project, IsAssetPackage, IsAssetPackage), // Project.IsAssetPackage
	DECL_BOOL(0x982e8227, Project, KanziConnectEnabled, KanziConnectEnabled), // Project.KanziConnectEnabled
	DECL_STRING(0x0c93d3a3, Project, DefaultMaterial, DefaultMaterial), // Project.DefaultMaterial
	DECL_INT(0xdc5503a7, Project, WindowWidth, WindowWidth), // Project.WindowWidth
	DECL_INT(0xbd75892a, Project, WindowHeight, WindowHeight), // Project.WindowHeight
	ARRAY_DECL_STRUCT_TYPE(0x3cee6129, Project, PropertyTypes, PropertyTypes, PropertyType), // Project.PropertyTypes
	DECL_INT(0x5d64948b, Project, NumPropertyTypes, NumPropertyTypes), // Project.NumPropertyTypes
	ARRAY_DECL_STRUCT_TYPE(0x0a978b48, Project, ProjectReferences, ProjectReferences, ProjectReference), // Project.ProjectReferences
	DECL_INT(0xc405deba, Project, NumProjectReferences, NumProjectReferences), // Project.NumProjectReferences
	ARRAY_DECL_STRUCT_TYPE(0x2fd1aed8, Project, SystemMessages, SystemMessages, SystemMessage), // Project.SystemMessages
	DECL_INT(0xbf690676, Project, NumSystemMessages, NumSystemMessages), // Project.NumSystemMessages
	ARRAY_DECL_STRUCT_TYPE(0x3f68bf99, Project, EnginePlugins, EnginePlugins, EnginePlugin), // Project.EnginePlugins
	DECL_INT(0x252d1cd3, Project, NumEnginePlugins, NumEnginePlugins), // Project.NumEnginePlugins
	DECL_OBJECT(0xb18f0186, Project, AnimationClipLibrary, AnimationClipLibrary, Library), // Project.AnimationClipLibrary
	DECL_OBJECT(0x33aae09c, Project, ScreenLibrary, ScreenLibrary, Library), // Project.ScreenLibrary
	DECL_OBJECT(0xa8705423, Project, MaterialTypeLibrary, MaterialTypeLibrary, Library), // Project.MaterialTypeLibrary
	DECL_OBJECT(0xa52f6c1b, Project, MaterialLibrary, MaterialLibrary, Library), // Project.MaterialLibrary
	DECL_OBJECT(0xaa7539c6, Project, BrushLibrary, BrushLibrary, Library), // Project.BrushLibrary
	DECL_OBJECT(0x11d562a7, Project, MeshLibrary, MeshLibrary, Library), // Project.MeshLibrary
	DECL_OBJECT(0x6a7a199a, Project, TimelineSequenceLibrary, TimelineSequenceLibrary, Library), // Project.TimelineSequenceLibrary
	DECL_OBJECT(0x9cebd1a3, Project, SceneObjectLibrary, SceneObjectLibrary, Library), // Project.SceneObjectLibrary
	DECL_OBJECT(0x739c88a6, Project, ComposerLibrary, ComposerLibrary, Library), // Project.ComposerLibrary
	DECL_OBJECT(0x9da5b82f, Project, PipelineItemLibrary, PipelineItemLibrary, Library), // Project.PipelineItemLibrary
	DECL_OBJECT(0xfa6e5926, Project, SceneLibrary, SceneLibrary, Library), // Project.SceneLibrary
	DECL_OBJECT(0x081b51d3, Project, TrajectoryLibrary, TrajectoryLibrary, Library), // Project.TrajectoryLibrary
	DECL_OBJECT(0xcf1f17a5, Project, TransitionLibrary, TransitionLibrary, Library), // Project.TransitionLibrary
	DECL_OBJECT(0xb16c4967, Project, SplineLibrary, SplineLibrary, Library), // Project.SplineLibrary
	DECL_OBJECT(0x0b066b16, Project, PrefabLibrary, PrefabLibrary, Library), // Project.PrefabLibrary
	DECL_OBJECT(0xd4de6821, Project, ProfileLibrary, ProfileLibrary, Library), // Project.ProfileLibrary
	DECL_OBJECT(0x425801c6, Project, ShortcutLibrary, ShortcutLibrary, Library), // Project.ShortcutLibrary
	DECL_OBJECT(0x309f6b49, Project, LayerLibrary, LayerLibrary, Library), // Project.LayerLibrary
	DECL_OBJECT(0x1818e708, Project, AnimationLibrary, AnimationLibrary, Library), // Project.AnimationLibrary
	DECL_OBJECT(0x7e634fa6, Project, TagLibrary, TagLibrary, Library), // Project.TagLibrary
	ARRAY_DECL_STRUCT_TYPE(0x508975b5, Project, ThemeLibrary, ThemeLibrary, ThemeValue), // Project.ThemeLibrary
	DECL_INT(0xa8550cf7, Project, NumThemeLibrary, NumThemeLibrary), // Project.NumThemeLibrary
	DECL_OBJECT(0x7ca6ddcc, Project, ResourceExportTagLibrary, ResourceExportTagLibrary, Library), // Project.ResourceExportTagLibrary
	DECL_OBJECT(0xc56f541a, Project, LocaleLibrary, LocaleLibrary, Library), // Project.LocaleLibrary
	DECL_OBJECT(0xe1462dd3, Project, DataSourceLibrary, DataSourceLibrary, Library), // Project.DataSourceLibrary
	DECL_OBJECT(0x29c80844, Project, PageTransitionCollectionLibrary, PageTransitionCollectionLibrary, Library), // Project.PageTransitionCollectionLibrary
	DECL_OBJECT(0x99f2a01f, Project, TextureLibrary, TextureLibrary, Library), // Project.TextureLibrary
	DECL_OBJECT(0x5c0ac569, Project, StyleLibrary, StyleLibrary, Library), // Project.StyleLibrary
	DECL_OBJECT(0x1f8da040, Project, StateManagerLibrary, StateManagerLibrary, Library), // Project.StateManagerLibrary
	DECL_OBJECT(0x66ac9ce3, Project, ConnectServiceLibrary, ConnectServiceLibrary, Library), // Project.ConnectServiceLibrary
	DECL_OBJECT(0x445ea848, Project, ConnectUserServiceLibrary, ConnectUserServiceLibrary, Library), // Project.ConnectUserServiceLibrary
	DECL_OBJECT(0xb1288547, Project, SpriteLibrary, SpriteLibrary, Library), // Project.SpriteLibrary
	DECL_OBJECT(0x9ef864fd, Project, SpriteAnimationLibrary, SpriteAnimationLibrary, Library), // Project.SpriteAnimationLibrary
	DECL_OBJECT(0x533a469d, Project, ImageLibrary, ImageLibrary, Library), // Project.ImageLibrary
	DECL_OBJECT(0xb033dd0b, Project, FontLibrary, FontLibrary, Library), // Project.FontLibrary
	DECL_EVENT(0xa2c038cf, Project, OpenFile), // Project.OpenFile
	DECL_EVENT(0x38dfc973, Project, FileExists), // Project.FileExists
	DECL_EVENT(0x5390a564, Project, HasChangedFiles), // Project.HasChangedFiles
	DECL_EVENT(0x31b9fee2, Project, LoadProject), // Project.LoadProject
};
COMPONENT_DEFAULTS(Project) = {
};
COMPONENT_PROC_BEGIN(Project)
		DISPATCH(Project, Object, Start) // Object.Start
COMPONENT_PROC_END()
void luaX_pushProject(lua_State *L, struct Project const* Project) {
	luaX_pushObject(L, CMP_GetObject(Project));
}
struct Project* luaX_checkProject(lua_State *L, int idx) {
	return GetProject(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Project, 0);
HANDLER(Directory, Project, LoadProject);
HANDLER(Directory, Project, OpenFile);
HANDLER(Directory, Project, FileExists);
HANDLER(Directory, Project, HasChangedFiles);
HANDLER(Directory, Object, Destroy);
COMPONENT_PROPERTIES(Directory) = {
	DECL_STRING(0xeb66e456, Directory, Path, Path), // Directory.Path
};
COMPONENT_DEFAULTS(Directory) = {
};
COMPONENT_PROC_BEGIN(Directory)
		DISPATCH(Directory, Project, LoadProject) // Project.LoadProject
		DISPATCH(Directory, Project, OpenFile) // Project.OpenFile
		DISPATCH(Directory, Project, FileExists) // Project.FileExists
		DISPATCH(Directory, Project, HasChangedFiles) // Project.HasChangedFiles
		DISPATCH(Directory, Object, Destroy) // Object.Destroy
COMPONENT_PROC_END()
void luaX_pushDirectory(lua_State *L, struct Directory const* Directory) {
	luaX_pushObject(L, CMP_GetObject(Directory));
}
struct Directory* luaX_checkDirectory(lua_State *L, int idx) {
	return GetDirectory(luaX_checkObject(L, idx));
}
#define ID_Project 0x7b5fea5e
REGISTER_CLASS(Directory, ID_Project, 0);
HANDLER(Package, Project, LoadProject);
HANDLER(Package, Project, OpenFile);
HANDLER(Package, Project, FileExists);
HANDLER(Package, Project, HasChangedFiles);
HANDLER(Package, Object, Destroy);
COMPONENT_PROPERTIES(Package) = {
	DECL_STRING(0x5ffdd888, Package, FileName, FileName), // Package.FileName
};
COMPONENT_DEFAULTS(Package) = {
};
COMPONENT_PROC_BEGIN(Package)
		DISPATCH(Package, Project, LoadProject) // Project.LoadProject
		DISPATCH(Package, Project, OpenFile) // Project.OpenFile
		DISPATCH(Package, Project, FileExists) // Project.FileExists
		DISPATCH(Package, Project, HasChangedFiles) // Project.HasChangedFiles
		DISPATCH(Package, Object, Destroy) // Object.Destroy
COMPONENT_PROC_END()
void luaX_pushPackage(lua_State *L, struct Package const* Package) {
	luaX_pushObject(L, CMP_GetObject(Package));
}
struct Package* luaX_checkPackage(lua_State *L, int idx) {
	return GetPackage(luaX_checkObject(L, idx));
}
#define ID_Project 0x7b5fea5e
REGISTER_CLASS(Package, ID_Project, 0);
COMPONENT_PROPERTIES(LocaleReferenceItem) = {
};
COMPONENT_DEFAULTS(LocaleReferenceItem) = {
};
COMPONENT_PROC_BEGIN(LocaleReferenceItem)
COMPONENT_PROC_END()
void luaX_pushLocaleReferenceItem(lua_State *L, struct LocaleReferenceItem const* LocaleReferenceItem) {
	luaX_pushObject(L, CMP_GetObject(LocaleReferenceItem));
}
struct LocaleReferenceItem* luaX_checkLocaleReferenceItem(lua_State *L, int idx) {
	return GetLocaleReferenceItem(luaX_checkObject(L, idx));
}
REGISTER_CLASS(LocaleReferenceItem, 0);
COMPONENT_PROPERTIES(Tag) = {
	DECL_BOOL(0xc35a8c07, Tag, TagIsInherited, TagIsInherited), // Tag.TagIsInherited
};
COMPONENT_DEFAULTS(Tag) = {
};
COMPONENT_PROC_BEGIN(Tag)
COMPONENT_PROC_END()
void luaX_pushTag(lua_State *L, struct Tag const* Tag) {
	luaX_pushObject(L, CMP_GetObject(Tag));
}
struct Tag* luaX_checkTag(lua_State *L, int idx) {
	return GetTag(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Tag, 0);
COMPONENT_PROPERTIES(Entry) = {
};
COMPONENT_DEFAULTS(Entry) = {
};
COMPONENT_PROC_BEGIN(Entry)
COMPONENT_PROC_END()
void luaX_pushEntry(lua_State *L, struct Entry const* Entry) {
	luaX_pushObject(L, CMP_GetObject(Entry));
}
struct Entry* luaX_checkEntry(lua_State *L, int idx) {
	return GetEntry(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Entry, 0);
COMPONENT_PROPERTIES(ThemeDefaultValuesDictionary) = {
};
COMPONENT_DEFAULTS(ThemeDefaultValuesDictionary) = {
};
COMPONENT_PROC_BEGIN(ThemeDefaultValuesDictionary)
COMPONENT_PROC_END()
void luaX_pushThemeDefaultValuesDictionary(lua_State *L, struct ThemeDefaultValuesDictionary const* ThemeDefaultValuesDictionary) {
	luaX_pushObject(L, CMP_GetObject(ThemeDefaultValuesDictionary));
}
struct ThemeDefaultValuesDictionary* luaX_checkThemeDefaultValuesDictionary(lua_State *L, int idx) {
	return GetThemeDefaultValuesDictionary(luaX_checkObject(L, idx));
}
REGISTER_CLASS(ThemeDefaultValuesDictionary, 0);
int f_FS_GetBaseName(lua_State *L) {
	const char* path = luaL_checkstring(L, 1);
	const char* result_ = FS_GetBaseName(path);
	lua_pushstring(L, result_);
	return 1;
}
int f_FS_GetDirName(lua_State *L) {
	const char* path = luaL_checkstring(L, 1);
	const char* out = luaL_checkstring(L, 2);
	int32_t maxlen = (int32_t)luaL_checkinteger(L, 3);
	const char* result_ = FS_GetDirName(path, out, maxlen);
	lua_pushstring(L, result_);
	return 1;
}
int f_FS_JoinPaths(lua_State *L) {
	path_t buffer = {0};
	int32_t size = sizeof(buffer);
	const char* path1 = luaL_checkstring(L, 1);
	const char* path2 = luaL_checkstring(L, 2);
	const char* result_ = FS_JoinPaths(buffer, size, path1, path2);
	lua_pushstring(L, result_);
	return 1;
}
int f_FS_PathFromModule(lua_State *L) {
	const char* module = luaL_checkstring(L, 1);
	const char* result_ = FS_PathFromModule(module);
	lua_pushstring(L, result_);
	return 1;
}
int f_FS_HasChangedFiles(lua_State *L) {
	bool_t result_ = FS_HasChangedFiles();
	lua_pushboolean(L, result_);
	return 1;
}
int f_FS_MakeDirectory(lua_State *L) {
	const char* path = luaL_checkstring(L, 1);
	bool_t result_ = FS_MakeDirectory(path);
	lua_pushboolean(L, result_);
	return 1;
}
int f_FS_FileExists(lua_State *L) {
	const char* path = luaL_checkstring(L, 1);
	bool_t result_ = FS_FileExists(path);
	lua_pushboolean(L, result_);
	return 1;
}
int f_FS_LoadBundle(lua_State *L) {
	const char* path = luaL_checkstring(L, 1);
	struct Object* result_ = FS_LoadBundle(L, path);
	luaX_pushObject(L, result_);
	return 1;
}
int f_FS_SetWorkspace(lua_State *L) {
	struct Object* workspace = luaX_checkObject(L, 1);
	FS_SetWorkspace(workspace );
	return 0;
}
int f_FS_GetWorkspace(lua_State *L) {
	struct Object* result_ = FS_GetWorkspace();
	luaX_pushObject(L, result_);
	return 1;
}
int f_FS_ReadTextFile(lua_State *L) {
	const char* path = luaL_checkstring(L, 1);
	return FS_ReadTextFile(L, path);
}
int f_FS_LoadObject(lua_State *L) {
	const char* path = luaL_checkstring(L, 1);
	struct Object* result_ = FS_LoadObject(path);
	luaX_pushObject(L, result_);
	return 1;
}
int f_FS_LoadObjectFromXml(lua_State *L) {
	const char* path = luaL_checkstring(L, 1);
	struct Object* result_ = FS_LoadObjectFromXml(path);
	luaX_pushObject(L, result_);
	return 1;
}
int f_FS_LoadObjectFromXmlString(lua_State *L) {
	const char* xmlString = luaL_checkstring(L, 1);
	struct Object* result_ = FS_LoadObjectFromXmlString(xmlString);
	luaX_pushObject(L, result_);
	return 1;
}
int f_FS_LoadObjectFromCss(lua_State *L) {
	const char* path = luaL_checkstring(L, 1);
	struct Object* result_ = FS_LoadObjectFromCss(path);
	luaX_pushObject(L, result_);
	return 1;
}
int f_FS_LoadObjectFromCssString(lua_State *L) {
	const char* cssString = luaL_checkstring(L, 1);
	struct Object* result_ = FS_LoadObjectFromCssString(cssString);
	luaX_pushObject(L, result_);
	return 1;
}
int f_FS_GetThemeValue(lua_State *L) {
	const char* key = luaL_checkstring(L, 1);
	const char* result_ = FS_GetThemeValue(key);
	lua_pushstring(L, result_);
	return 1;
}

ORCA_API int luaopen_orca_filesystem(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ "getBaseName", f_FS_GetBaseName },
		{ "getDirName", f_FS_GetDirName },
		{ "joinPaths", f_FS_JoinPaths },
		{ "pathFromModule", f_FS_PathFromModule },
		{ "hasChangedFiles", f_FS_HasChangedFiles },
		{ "makeDirectory", f_FS_MakeDirectory },
		{ "fileExists", f_FS_FileExists },
		{ "loadBundle", f_FS_LoadBundle },
		{ "setWorkspace", f_FS_SetWorkspace },
		{ "getWorkspace", f_FS_GetWorkspace },
		{ "readTextFile", f_FS_ReadTextFile },
		{ "loadObject", f_FS_LoadObject },
		{ "loadObjectFromXml", f_FS_LoadObjectFromXml },
		{ "loadObjectFromXmlString", f_FS_LoadObjectFromXmlString },
		{ "loadObjectFromCss", f_FS_LoadObjectFromCss },
		{ "loadObjectFromCssString", f_FS_LoadObjectFromCssString },
		{ "getThemeValue", f_FS_GetThemeValue },
		{ NULL, NULL } 
	}));
	void on_filesystem_module_registered(lua_State *L);
	on_filesystem_module_registered(L);
	lua_setfield(L, ((void)luaopen_orca_ProjectReference(L), -2), "ProjectReference");
	lua_setfield(L, ((void)luaopen_orca_EnginePlugin(L), -2), "EnginePlugin");
	lua_setfield(L, ((void)luaopen_orca_SystemMessage(L), -2), "SystemMessage");
	lua_setfield(L, ((void)luaopen_orca_ThemeValue(L), -2), "ThemeValue");
		lua_setfield(L, ((void)luaopen_orca_Workspace_ReadCommandsEventArgs(L), -2), "Workspace_ReadCommandsEventArgs");
		lua_setfield(L, ((void)luaopen_orca_Project_OpenFileEventArgs(L), -2), "Project_OpenFileEventArgs");
		lua_setfield(L, ((void)luaopen_orca_Project_FileExistsEventArgs(L), -2), "Project_FileExistsEventArgs");
		lua_setfield(L, ((void)luaopen_orca_Project_HasChangedFilesEventArgs(L), -2), "Project_HasChangedFilesEventArgs");
		lua_setfield(L, ((void)luaopen_orca_Project_LoadProjectEventArgs(L), -2), "Project_LoadProjectEventArgs");
	lua_setfield(L, ((void)lua_pushclass(L, &_Workspace), -2), "Workspace");
	lua_setfield(L, ((void)lua_pushclass(L, &_Library), -2), "Library");
	lua_setfield(L, ((void)lua_pushclass(L, &_Project), -2), "Project");
	lua_setfield(L, ((void)lua_pushclass(L, &_Directory), -2), "Directory");
	lua_setfield(L, ((void)lua_pushclass(L, &_Package), -2), "Package");
	lua_setfield(L, ((void)lua_pushclass(L, &_LocaleReferenceItem), -2), "LocaleReferenceItem");
	lua_setfield(L, ((void)lua_pushclass(L, &_Tag), -2), "Tag");
	lua_setfield(L, ((void)lua_pushclass(L, &_Entry), -2), "Entry");
	lua_setfield(L, ((void)lua_pushclass(L, &_ThemeDefaultValuesDictionary), -2), "ThemeDefaultValuesDictionary");
	return 1;
}
