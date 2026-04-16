// Auto-generated from filesystem.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "filesystem.h"

#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }

// Object
extern void luaX_pushObject(lua_State *L, struct Object const* value);
extern struct Object* luaX_checkObject(lua_State *L, int index);
// _PACK
extern void luaX_push_PACK(lua_State *L, struct _PACK const* value);
extern struct _PACK* luaX_check_PACK(lua_State *L, int index);

#define ENUM(NAME, ...) \
ORCA_API const char *_##NAME[] = {__VA_ARGS__, NULL}; \
const char *NAME##ToString(enum NAME value) { \
	return (assert(value >= 0 && value < sizeof(_##NAME) / sizeof(*_##NAME) - 1), _##NAME[value]); \
} \
enum NAME luaX_check##NAME(lua_State *L, int idx) { \
	return luaL_checkoption(L, idx, NULL, _##NAME); \
} \
void luaX_push##NAME(lua_State *L, enum NAME value) { \
	lua_pushstring(L, (assert(value >= 0 && value < sizeof(_##NAME) / sizeof(*_##NAME) - 1), _##NAME[value])); \
}
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(lua_State *L, const char* str, struct PropertyType const* prop, void* struct_ptr);

#define STRUCT(NAME, EXPORT) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
	if (data == NULL) { lua_pushnil(L); return; } \
	memcpy(lua_newuserdata(L, sizeof(struct NAME)), data, sizeof(struct NAME)); \
	luaL_setmetatable(L, #EXPORT); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { return luaL_checkudata(L, idx, #EXPORT); } \
static int f_new_##NAME(lua_State *L) { \
	struct NAME self; \
	memset(&self, 0, sizeof(struct NAME)); \
	if (lua_islightuserdata(L, 1)) { \
		memcpy(&self, lua_touserdata(L, 1), sizeof(struct NAME)); \
	} else if (lua_istable(L, 1)) \
		for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); lua_pop(L, 1), i++) { \
			if (lua_getfield(L, 1, _##NAME[i].Name)) \
				read_property(L, -1, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); } \
	else for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		read_property(L, i + 1, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); \
	luaX_push##NAME(L, &self); \
	return 1; \
} \
static int f_##NAME##___index(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (write_property(L, &_##NAME[i], ((char*)luaX_check##NAME(L, 1))+_##NAME[i].Offset), 1); \
	for (uint32_t i = 0; i < sizeof(_##NAME##_Methods) / sizeof(*_##NAME##_Methods); i++) { \
		if (_##NAME##_Methods[i].name && strcmp(_##NAME##_Methods[i].name, luaL_checkstring(L, 2)) == 0) { \
			lua_pushcfunction(L, _##NAME##_Methods[i].func); \
			return 1; \
		} \
	} \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___newindex(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (read_property(L, 3, &_##NAME[i], ((char*)luaX_check##NAME(L, 1))+_##NAME[i].Offset), 0); \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___call(lua_State *L) { \
  lua_insert(L, (lua_getfield(L, 1, "new"), 2)); \
  lua_call(L, lua_gettop(L) - 2, 1); \
	return 1; \
} \
static int f_##NAME##___fromstring(lua_State *L) { \
	char* tmp = strdup(luaL_checkstring(L, 1)),* tok = strtok(tmp, " "); \
	struct NAME self; \
	memset(&self, 0, sizeof(struct NAME)); \
	for (uint32_t i = 0; tok && i < sizeof(_##NAME) / sizeof(*_##NAME); i++, tok = strtok(NULL, " ")) \
		if (_##NAME[i].DataType != kDataTypeStruct) \
			parse_property(L, tok, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); \
	free(tmp); \
	return (luaX_push##NAME(L, &self), 1); \
} \
int luaopen_orca_##NAME(lua_State *L) { \
	luaL_newmetatable(L, #EXPORT); \
	luaL_setfuncs(L, ((luaL_Reg[]) { \
		{ "new", f_new_##NAME }, \
		{ "fromstring", f_##NAME##___fromstring }, \
		{ "__newindex", f_##NAME##___newindex }, \
		{ "__index", f_##NAME##___index }, \
		{ NULL, NULL }, \
	}), 0); \
	luaL_setfuncs(L, _##NAME##_Methods, 0); \
	/* Make struct creatable via constructor-like syntax */ \
	lua_newtable(L); \
	lua_pushcfunction(L, f_##NAME##___call); \
	lua_setfield(L, -2, "__call"); \
	lua_setmetatable(L, -2); \
	return 1; \
}
static struct PropertyType _ProjectReference[] = {
	DECL(0x0fe07306, ProjectReference, Name, Name, kDataTypeString), // ProjectReference.Name
	DECL(0xeb66e456, ProjectReference, Path, Path, kDataTypeString), // ProjectReference.Path
};
static luaL_Reg _ProjectReference_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _EnginePlugin[] = {
	DECL(0x0fe07306, EnginePlugin, Name, Name, kDataTypeString), // EnginePlugin.Name
};
static luaL_Reg _EnginePlugin_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _SystemMessage[] = {
	DECL(0xae0ed984, SystemMessage, Message, Message, kDataTypeString), // SystemMessage.Message
	DECL(0xcd1ac90c, SystemMessage, Key, Key, kDataTypeString), // SystemMessage.Key
	DECL(0xc67c8f52, SystemMessage, Command, Command, kDataTypeString), // SystemMessage.Command
};
static luaL_Reg _SystemMessage_Methods[] = {
	{ NULL, NULL }
};

STRUCT(ProjectReference, ProjectReference);
STRUCT(EnginePlugin, EnginePlugin);
STRUCT(SystemMessage, SystemMessage);

static luaL_Reg _Workspace_ReadCommandsEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Workspace_ReadCommandsEventArgs[] = {
};
static luaL_Reg _Project_OpenFileEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Project_OpenFileEventArgs[] = {
	DECL(0x5ffdd888, Project_OpenFileEventArgs, FileName, FileName, kDataTypeString), // Project_OpenFileEventArgs.FileName
};
static luaL_Reg _Project_FileExistsEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Project_FileExistsEventArgs[] = {
	DECL(0x5ffdd888, Project_FileExistsEventArgs, FileName, FileName, kDataTypeString), // Project_FileExistsEventArgs.FileName
};
static luaL_Reg _Project_HasChangedFilesEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Project_HasChangedFilesEventArgs[] = {
};
static luaL_Reg _Project_LoadProjectEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Project_LoadProjectEventArgs[] = {
	DECL(0xeb66e456, Project_LoadProjectEventArgs, Path, Path, kDataTypeString), // Project_LoadProjectEventArgs.Path
};

STRUCT(Workspace_ReadCommandsEventArgs, Workspace_ReadCommandsEventArgs);
STRUCT(Project_OpenFileEventArgs, Project_OpenFileEventArgs);
STRUCT(Project_FileExistsEventArgs, Project_FileExistsEventArgs);
STRUCT(Project_HasChangedFilesEventArgs, Project_HasChangedFilesEventArgs);
STRUCT(Project_LoadProjectEventArgs, Project_LoadProjectEventArgs);
#define REGISTER_CLASS(NAME, ...) \
ORCA_API struct ClassDesc _##NAME = { \
	.ClassName = #NAME, \
	.DefaultName = #NAME, \
	.ContentType = #NAME, \
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
	.ParentClasses = { __VA_ARGS__ }, \
	.ClassID = ID_##NAME, \
	.ClassSize = sizeof(struct NAME), \
	.Properties = NAME##Properties, \
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
};
#define REGISTER_ATTACH_ONLY_CLASS(NAME, ...) \
ORCA_API struct ClassDesc _##NAME = { \
	.ClassName = #NAME, \
	.DefaultName = #NAME, \
	.ContentType = #NAME, \
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
	.ParentClasses = { __VA_ARGS__ }, \
	.ClassID = ID_##NAME, \
	.ClassSize = sizeof(struct NAME), \
	.Properties = NAME##Properties, \
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
	.IsAttachOnly = TRUE, \
};
static struct PropertyType const WorkspaceProperties[kWorkspaceNumProperties] = {
	DECL(0x23d83fd3, Workspace, ReadCommands, ReadCommands, kDataTypeEvent, .TypeString = "Workspace_ReadCommandsEventArgs"), // Workspace.ReadCommands
};
static struct Workspace WorkspaceDefaults = {
};
LRESULT WorkspaceProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushWorkspace(lua_State *L, struct Workspace const* Workspace) {
	luaX_pushObject(L, CMP_GetObject(Workspace));
}
struct Workspace* luaX_checkWorkspace(lua_State *L, int idx) {
	return GetWorkspace(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Workspace, 0);
static struct PropertyType const LibraryProperties[kLibraryNumProperties] = {
	DECL(0x1cb8f23a, Library, IsExternal, IsExternal, kDataTypeBool), // Library.IsExternal
};
static struct Library LibraryDefaults = {
};
LRESULT LibraryProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushLibrary(lua_State *L, struct Library const* Library) {
	luaX_pushObject(L, CMP_GetObject(Library));
}
struct Library* luaX_checkLibrary(lua_State *L, int idx) {
	return GetLibrary(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Library, 0);
HANDLER(Project, Object, Start);
static struct PropertyType const ProjectProperties[kProjectNumProperties] = {
	DECL(0xbcd19216, Project, HalfFloatTextureFormat, HalfFloatTextureFormat, kDataTypeBool), // Project.HalfFloatTextureFormat
	DECL(0xfba1938f, Project, HalfFloatTextureFormatLinear, HalfFloatTextureFormatLinear, kDataTypeBool), // Project.HalfFloatTextureFormatLinear
	DECL(0x1275837c, Project, HalfFloatColorAttachment, HalfFloatColorAttachment, kDataTypeBool), // Project.HalfFloatColorAttachment
	DECL(0x6b1586db, Project, RenderToMipmapLevels, RenderToMipmapLevels, kDataTypeBool), // Project.RenderToMipmapLevels
	DECL(0x377aea3d, Project, ExternalTexture, ExternalTexture, kDataTypeBool), // Project.ExternalTexture
	DECL(0xb3cce876, Project, StartupScreen, StartupScreen, kDataTypeString), // Project.StartupScreen
	DECL(0xe1bb158a, Project, PreviewWindowBackgroundColor, PreviewWindowBackgroundColor, kDataTypeColor), // Project.PreviewWindowBackgroundColor
	DECL(0xb92de767, Project, MessageLimitPerFrame, MessageLimitPerFrame, kDataTypeInt), // Project.MessageLimitPerFrame
	DECL(0xe310dade, Project, GlobalTimelineStartTime, GlobalTimelineStartTime, kDataTypeInt), // Project.GlobalTimelineStartTime
	DECL(0x5d90d07b, Project, GlobalTimelineEndTime, GlobalTimelineEndTime, kDataTypeInt), // Project.GlobalTimelineEndTime
	DECL(0x527cf409, Project, BinaryExportDirectory, BinaryExportDirectory, kDataTypeString), // Project.BinaryExportDirectory
	DECL(0x5d547d12, Project, ApplicationExportDirectory, ApplicationExportDirectory, kDataTypeString), // Project.ApplicationExportDirectory
	DECL(0xf6c72bc0, Project, IsMasterProject, IsMasterProject, kDataTypeBool), // Project.IsMasterProject
	DECL(0x3762ffdf, Project, OptimizeMeshes, OptimizeMeshes, kDataTypeBool), // Project.OptimizeMeshes
	DECL(0xda2eb7cc, Project, TargetPlatformVertexCacheSize, TargetPlatformVertexCacheSize, kDataTypeInt), // Project.TargetPlatformVertexCacheSize
	DECL(0xdc968859, Project, PlotAnimations, PlotAnimations, kDataTypeBool), // Project.PlotAnimations
	DECL(0x66a084f4, Project, RoundImagesToNearestPowerOf2, RoundImagesToNearestPowerOf2, kDataTypeBool), // Project.RoundImagesToNearestPowerOf2
	DECL(0xeb929bed, Project, ProjectExportShaderSourceCode, ProjectExportShaderSourceCode, kDataTypeBool), // Project.ProjectExportShaderSourceCode
	DECL(0xc3c2d763, Project, ProjectExportMainKzbWithBakedThemes, ProjectExportMainKzbWithBakedThemes, kDataTypeBool), // Project.ProjectExportMainKzbWithBakedThemes
	DECL(0xad8222eb, Project, ProjectGroupByThemeNameInBakedThemeExport, ProjectGroupByThemeNameInBakedThemeExport, kDataTypeBool), // Project.ProjectGroupByThemeNameInBakedThemeExport
	DECL(0x18da08d1, Project, IsLocalizationEnabled, IsLocalizationEnabled, kDataTypeBool), // Project.IsLocalizationEnabled
	DECL(0xdf498299, Project, FullScreenPreviewLayer, FullScreenPreviewLayer, kDataTypeBool), // Project.FullScreenPreviewLayer
	DECL(0x76bfa6c4, Project, ShowChildrenInLayerThumbnails, ShowChildrenInLayerThumbnails, kDataTypeBool), // Project.ShowChildrenInLayerThumbnails
	DECL(0x5734b5cd, Project, ProjectUsePremultipliedAlpha, ProjectUsePremultipliedAlpha, kDataTypeBool), // Project.ProjectUsePremultipliedAlpha
	DECL(0xb617d580, Project, ProjectRemoveICCProfilesOfPngs, ProjectRemoveICCProfilesOfPngs, kDataTypeBool), // Project.ProjectRemoveICCProfilesOfPngs
	DECL(0x0f9ae251, Project, BinaryFileName, BinaryFileName, kDataTypeString), // Project.BinaryFileName
	DECL(0x76ac7909, Project, IsAssetPackage, IsAssetPackage, kDataTypeBool), // Project.IsAssetPackage
	DECL(0x982e8227, Project, KanziConnectEnabled, KanziConnectEnabled, kDataTypeBool), // Project.KanziConnectEnabled
	DECL(0x0c93d3a3, Project, DefaultMaterial, DefaultMaterial, kDataTypeString), // Project.DefaultMaterial
	DECL(0xdc5503a7, Project, WindowWidth, WindowWidth, kDataTypeInt), // Project.WindowWidth
	DECL(0xbd75892a, Project, WindowHeight, WindowHeight, kDataTypeInt), // Project.WindowHeight
	ARRAY_DECL(0x3cee6129, Project, PropertyTypes, PropertyTypes, kDataTypeStruct, .TypeString = "PropertyType"), // Project.PropertyTypes
	DECL(0x5d64948b, Project, NumPropertyTypes, NumPropertyTypes, kDataTypeInt), // Project.NumPropertyTypes
	ARRAY_DECL(0x0a978b48, Project, ProjectReferences, ProjectReferences, kDataTypeStruct, .TypeString = "ProjectReference"), // Project.ProjectReferences
	DECL(0xc405deba, Project, NumProjectReferences, NumProjectReferences, kDataTypeInt), // Project.NumProjectReferences
	ARRAY_DECL(0x2fd1aed8, Project, SystemMessages, SystemMessages, kDataTypeStruct, .TypeString = "SystemMessage"), // Project.SystemMessages
	DECL(0xbf690676, Project, NumSystemMessages, NumSystemMessages, kDataTypeInt), // Project.NumSystemMessages
	ARRAY_DECL(0x3f68bf99, Project, EnginePlugins, EnginePlugins, kDataTypeStruct, .TypeString = "EnginePlugin"), // Project.EnginePlugins
	DECL(0x252d1cd3, Project, NumEnginePlugins, NumEnginePlugins, kDataTypeInt), // Project.NumEnginePlugins
	DECL(0xb18f0186, Project, AnimationClipLibrary, AnimationClipLibrary, kDataTypeObject, .TypeString = "Library"), // Project.AnimationClipLibrary
	DECL(0x33aae09c, Project, ScreenLibrary, ScreenLibrary, kDataTypeObject, .TypeString = "Library"), // Project.ScreenLibrary
	DECL(0xa8705423, Project, MaterialTypeLibrary, MaterialTypeLibrary, kDataTypeObject, .TypeString = "Library"), // Project.MaterialTypeLibrary
	DECL(0xa52f6c1b, Project, MaterialLibrary, MaterialLibrary, kDataTypeObject, .TypeString = "Library"), // Project.MaterialLibrary
	DECL(0xaa7539c6, Project, BrushLibrary, BrushLibrary, kDataTypeObject, .TypeString = "Library"), // Project.BrushLibrary
	DECL(0x11d562a7, Project, MeshLibrary, MeshLibrary, kDataTypeObject, .TypeString = "Library"), // Project.MeshLibrary
	DECL(0x6a7a199a, Project, TimelineSequenceLibrary, TimelineSequenceLibrary, kDataTypeObject, .TypeString = "Library"), // Project.TimelineSequenceLibrary
	DECL(0x9cebd1a3, Project, SceneObjectLibrary, SceneObjectLibrary, kDataTypeObject, .TypeString = "Library"), // Project.SceneObjectLibrary
	DECL(0x739c88a6, Project, ComposerLibrary, ComposerLibrary, kDataTypeObject, .TypeString = "Library"), // Project.ComposerLibrary
	DECL(0x9da5b82f, Project, PipelineItemLibrary, PipelineItemLibrary, kDataTypeObject, .TypeString = "Library"), // Project.PipelineItemLibrary
	DECL(0xfa6e5926, Project, SceneLibrary, SceneLibrary, kDataTypeObject, .TypeString = "Library"), // Project.SceneLibrary
	DECL(0x081b51d3, Project, TrajectoryLibrary, TrajectoryLibrary, kDataTypeObject, .TypeString = "Library"), // Project.TrajectoryLibrary
	DECL(0xcf1f17a5, Project, TransitionLibrary, TransitionLibrary, kDataTypeObject, .TypeString = "Library"), // Project.TransitionLibrary
	DECL(0xb16c4967, Project, SplineLibrary, SplineLibrary, kDataTypeObject, .TypeString = "Library"), // Project.SplineLibrary
	DECL(0x0b066b16, Project, PrefabLibrary, PrefabLibrary, kDataTypeObject, .TypeString = "Library"), // Project.PrefabLibrary
	DECL(0xd4de6821, Project, ProfileLibrary, ProfileLibrary, kDataTypeObject, .TypeString = "Library"), // Project.ProfileLibrary
	DECL(0x425801c6, Project, ShortcutLibrary, ShortcutLibrary, kDataTypeObject, .TypeString = "Library"), // Project.ShortcutLibrary
	DECL(0x309f6b49, Project, LayerLibrary, LayerLibrary, kDataTypeObject, .TypeString = "Library"), // Project.LayerLibrary
	DECL(0x1818e708, Project, AnimationLibrary, AnimationLibrary, kDataTypeObject, .TypeString = "Library"), // Project.AnimationLibrary
	DECL(0x7e634fa6, Project, TagLibrary, TagLibrary, kDataTypeObject, .TypeString = "Library"), // Project.TagLibrary
	DECL(0x508975b5, Project, ThemeLibrary, ThemeLibrary, kDataTypeObject, .TypeString = "Library"), // Project.ThemeLibrary
	DECL(0x7ca6ddcc, Project, ResourceExportTagLibrary, ResourceExportTagLibrary, kDataTypeObject, .TypeString = "Library"), // Project.ResourceExportTagLibrary
	DECL(0xc56f541a, Project, LocaleLibrary, LocaleLibrary, kDataTypeObject, .TypeString = "Library"), // Project.LocaleLibrary
	DECL(0xe1462dd3, Project, DataSourceLibrary, DataSourceLibrary, kDataTypeObject, .TypeString = "Library"), // Project.DataSourceLibrary
	DECL(0x29c80844, Project, PageTransitionCollectionLibrary, PageTransitionCollectionLibrary, kDataTypeObject, .TypeString = "Library"), // Project.PageTransitionCollectionLibrary
	DECL(0x99f2a01f, Project, TextureLibrary, TextureLibrary, kDataTypeObject, .TypeString = "Library"), // Project.TextureLibrary
	DECL(0x5c0ac569, Project, StyleLibrary, StyleLibrary, kDataTypeObject, .TypeString = "Library"), // Project.StyleLibrary
	DECL(0x1f8da040, Project, StateManagerLibrary, StateManagerLibrary, kDataTypeObject, .TypeString = "Library"), // Project.StateManagerLibrary
	DECL(0x66ac9ce3, Project, ConnectServiceLibrary, ConnectServiceLibrary, kDataTypeObject, .TypeString = "Library"), // Project.ConnectServiceLibrary
	DECL(0x445ea848, Project, ConnectUserServiceLibrary, ConnectUserServiceLibrary, kDataTypeObject, .TypeString = "Library"), // Project.ConnectUserServiceLibrary
	DECL(0xb1288547, Project, SpriteLibrary, SpriteLibrary, kDataTypeObject, .TypeString = "Library"), // Project.SpriteLibrary
	DECL(0x9ef864fd, Project, SpriteAnimationLibrary, SpriteAnimationLibrary, kDataTypeObject, .TypeString = "Library"), // Project.SpriteAnimationLibrary
	DECL(0x533a469d, Project, ImageLibrary, ImageLibrary, kDataTypeObject, .TypeString = "Library"), // Project.ImageLibrary
	DECL(0xb033dd0b, Project, FontLibrary, FontLibrary, kDataTypeObject, .TypeString = "Library"), // Project.FontLibrary
	DECL(0xa2c038cf, Project, OpenFile, OpenFile, kDataTypeEvent, .TypeString = "Project_OpenFileEventArgs"), // Project.OpenFile
	DECL(0x38dfc973, Project, FileExists, FileExists, kDataTypeEvent, .TypeString = "Project_FileExistsEventArgs"), // Project.FileExists
	DECL(0x5390a564, Project, HasChangedFiles, HasChangedFiles, kDataTypeEvent, .TypeString = "Project_HasChangedFilesEventArgs"), // Project.HasChangedFiles
	DECL(0x31b9fee2, Project, LoadProject, LoadProject, kDataTypeEvent, .TypeString = "Project_LoadProjectEventArgs"), // Project.LoadProject
};
static struct Project ProjectDefaults = {
};
LRESULT ProjectProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Start: return Project_Start(object, cmp, wparm, lparm); // Object.Start
	}
	return FALSE;
}
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
static struct PropertyType const DirectoryProperties[kDirectoryNumProperties] = {
	DECL(0xeb66e456, Directory, Path, Path, kDataTypeString), // Directory.Path
};
static struct Directory DirectoryDefaults = {
};
LRESULT DirectoryProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Project_LoadProject: return Directory_LoadProject(object, cmp, wparm, lparm); // Project.LoadProject
		case ID_Project_OpenFile: return Directory_OpenFile(object, cmp, wparm, lparm); // Project.OpenFile
		case ID_Project_FileExists: return Directory_FileExists(object, cmp, wparm, lparm); // Project.FileExists
		case ID_Project_HasChangedFiles: return Directory_HasChangedFiles(object, cmp, wparm, lparm); // Project.HasChangedFiles
		case ID_Object_Destroy: return Directory_Destroy(object, cmp, wparm, lparm); // Object.Destroy
	}
	return FALSE;
}
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
static struct PropertyType const PackageProperties[kPackageNumProperties] = {
	DECL(0x5ffdd888, Package, FileName, FileName, kDataTypeString), // Package.FileName
};
static struct Package PackageDefaults = {
};
LRESULT PackageProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Project_LoadProject: return Package_LoadProject(object, cmp, wparm, lparm); // Project.LoadProject
		case ID_Project_OpenFile: return Package_OpenFile(object, cmp, wparm, lparm); // Project.OpenFile
		case ID_Project_FileExists: return Package_FileExists(object, cmp, wparm, lparm); // Project.FileExists
		case ID_Project_HasChangedFiles: return Package_HasChangedFiles(object, cmp, wparm, lparm); // Project.HasChangedFiles
		case ID_Object_Destroy: return Package_Destroy(object, cmp, wparm, lparm); // Object.Destroy
	}
	return FALSE;
}
void luaX_pushPackage(lua_State *L, struct Package const* Package) {
	luaX_pushObject(L, CMP_GetObject(Package));
}
struct Package* luaX_checkPackage(lua_State *L, int idx) {
	return GetPackage(luaX_checkObject(L, idx));
}
#define ID_Project 0x7b5fea5e
REGISTER_CLASS(Package, ID_Project, 0);
static struct PropertyType const LocaleReferenceItemProperties[kLocaleReferenceItemNumProperties] = {
};
static struct LocaleReferenceItem LocaleReferenceItemDefaults = {
};
LRESULT LocaleReferenceItemProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushLocaleReferenceItem(lua_State *L, struct LocaleReferenceItem const* LocaleReferenceItem) {
	luaX_pushObject(L, CMP_GetObject(LocaleReferenceItem));
}
struct LocaleReferenceItem* luaX_checkLocaleReferenceItem(lua_State *L, int idx) {
	return GetLocaleReferenceItem(luaX_checkObject(L, idx));
}
REGISTER_CLASS(LocaleReferenceItem, 0);
static struct PropertyType const TagProperties[kTagNumProperties] = {
	DECL(0xc35a8c07, Tag, TagIsInherited, TagIsInherited, kDataTypeBool), // Tag.TagIsInherited
};
static struct Tag TagDefaults = {
};
LRESULT TagProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushTag(lua_State *L, struct Tag const* Tag) {
	luaX_pushObject(L, CMP_GetObject(Tag));
}
struct Tag* luaX_checkTag(lua_State *L, int idx) {
	return GetTag(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Tag, 0);
HANDLER(ThemeGroup, Object, Attached);
static struct PropertyType const ThemeGroupProperties[kThemeGroupNumProperties] = {
	DECL(0x75516381, ThemeGroup, SelectedTheme, SelectedTheme, kDataTypeString), // ThemeGroup.SelectedTheme
	DECL(0x1cf2c938, ThemeGroup, SelectedDictionary, SelectedDictionary, kDataTypeString), // ThemeGroup.SelectedDictionary
};
static struct ThemeGroup ThemeGroupDefaults = {
};
LRESULT ThemeGroupProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Attached: return ThemeGroup_Attached(object, cmp, wparm, lparm); // Object.Attached
	}
	return FALSE;
}
void luaX_pushThemeGroup(lua_State *L, struct ThemeGroup const* ThemeGroup) {
	luaX_pushObject(L, CMP_GetObject(ThemeGroup));
}
struct ThemeGroup* luaX_checkThemeGroup(lua_State *L, int idx) {
	return GetThemeGroup(luaX_checkObject(L, idx));
}
REGISTER_CLASS(ThemeGroup, 0);
static struct PropertyType const ThemeProperties[kThemeNumProperties] = {
	DECL(0x1ed11084, Theme, IsThemeVisible, IsThemeVisible, kDataTypeBool), // Theme.IsThemeVisible
};
static struct Theme ThemeDefaults = {
};
LRESULT ThemeProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushTheme(lua_State *L, struct Theme const* Theme) {
	luaX_pushObject(L, CMP_GetObject(Theme));
}
struct Theme* luaX_checkTheme(lua_State *L, int idx) {
	return GetTheme(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Theme, 0);
static struct PropertyType const EntryProperties[kEntryNumProperties] = {
};
static struct Entry EntryDefaults = {
};
LRESULT EntryProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushEntry(lua_State *L, struct Entry const* Entry) {
	luaX_pushObject(L, CMP_GetObject(Entry));
}
struct Entry* luaX_checkEntry(lua_State *L, int idx) {
	return GetEntry(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Entry, 0);
static struct PropertyType const ThemeDefaultValuesDictionaryProperties[kThemeDefaultValuesDictionaryNumProperties] = {
};
static struct ThemeDefaultValuesDictionary ThemeDefaultValuesDictionaryDefaults = {
};
LRESULT ThemeDefaultValuesDictionaryProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
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
		{ NULL, NULL } 
	}));
	void on_filesystem_module_registered(lua_State *L);
	on_filesystem_module_registered(L);
	lua_setfield(L, ((void)luaopen_orca_ProjectReference(L), -2), "ProjectReference");
	lua_setfield(L, ((void)luaopen_orca_EnginePlugin(L), -2), "EnginePlugin");
	lua_setfield(L, ((void)luaopen_orca_SystemMessage(L), -2), "SystemMessage");
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
	lua_setfield(L, ((void)lua_pushclass(L, &_ThemeGroup), -2), "ThemeGroup");
	lua_setfield(L, ((void)lua_pushclass(L, &_Theme), -2), "Theme");
	lua_setfield(L, ((void)lua_pushclass(L, &_Entry), -2), "Entry");
	lua_setfield(L, ((void)lua_pushclass(L, &_ThemeDefaultValuesDictionary), -2), "ThemeDefaultValuesDictionary");
	return 1;
}
