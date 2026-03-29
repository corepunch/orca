// Auto-generated from filesystem.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "filesystem.h"

// Object
extern void luaX_pushObject(lua_State *L, struct Object const* value);
extern struct Object* luaX_checkObject(lua_State *L, int index);
// _PACK
extern void luaX_push_PACK(lua_State *L, struct _PACK const* value);
extern struct _PACK* luaX_check_PACK(lua_State *L, int index);

void luaX_pushProjectReference(lua_State *L, struct ProjectReference const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct ProjectReference* self = lua_newuserdata(L, sizeof(struct ProjectReference));
	luaL_setmetatable(L, "ProjectReference");
	memcpy(self, data, sizeof(struct ProjectReference));
}
struct ProjectReference* luaX_checkProjectReference(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "ProjectReference");
}
static int f_new_ProjectReference(lua_State *L) {
	struct ProjectReference* self = lua_newuserdata(L, sizeof(struct ProjectReference));
	luaL_setmetatable(L, "ProjectReference");
	memset(self, 0, sizeof(struct ProjectReference));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Name"), strncpy(self->Name, luaL_optstring(L, -1, ""), sizeof(self->Name)), 1));
		lua_pop(L, (lua_getfield(L, 1, "Path"), strncpy(self->Path, luaL_optstring(L, -1, ""), sizeof(self->Path)), 1));
	} else {
		strncpy(self->Name, luaL_optstring(L, 1, ""), sizeof(self->Name));
		strncpy(self->Path, luaL_optstring(L, 2, ""), sizeof(self->Path));
	}
	return 1;
}


int f_ProjectReference___index(lua_State *L) {
	struct ProjectReference* self = luaX_checkProjectReference(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x0fe07306: lua_pushstring(L, self->Name); return 1; // Name
	case 0xeb66e456: lua_pushstring(L, self->Path); return 1; // Path
	}
	return luaL_error(L, "Unknown field in ProjectReference(%p): %s", self, luaL_checkstring(L, 2));
}
int f_ProjectReference___newindex(lua_State *L) {
	struct ProjectReference* self = luaX_checkProjectReference(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x0fe07306: strncpy(self->Name, luaL_optstring(L, 3, ""), sizeof(self->Name)); return 0; // Name
	case 0xeb66e456: strncpy(self->Path, luaL_optstring(L, 3, ""), sizeof(self->Path)); return 0; // Path
	}
	return luaL_error(L, "Unknown field in ProjectReference(%p): %s", self, luaL_checkstring(L, 2));
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_ProjectReference___fromstring(lua_State *L) {
	fixedString_t Name;
	fixedString_t Path;
	struct ProjectReference self = {0};
	switch (sscanf(luaL_checkstring(L, 1), "%s %s", Name, Path)) {
	case 2: 
		strncpy(self.Name, Name, sizeof(self.Name));
		strncpy(self.Path, Path, sizeof(self.Path));
		return (luaX_pushProjectReference(L, &self), 1);
	default:
		return luaL_error(L, "Invalid format for ProjectReference: %s", luaL_checkstring(L, 1));
	}
}
static int f_ProjectReference___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_ProjectReference(L));  // remove ProjectReference from stack and call constructor
}
int luaopen_orca_ProjectReference(lua_State *L) {
	luaL_newmetatable(L, "ProjectReference");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_ProjectReference },
		{ "fromstring", f_ProjectReference___fromstring },
		{ "__newindex", f_ProjectReference___newindex },
		{ "__index", f_ProjectReference___index },
		{ NULL, NULL },
	}), 0);
	// Make ProjectReference creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_ProjectReference___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushEnginePlugin(lua_State *L, struct EnginePlugin const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct EnginePlugin* self = lua_newuserdata(L, sizeof(struct EnginePlugin));
	luaL_setmetatable(L, "EnginePlugin");
	memcpy(self, data, sizeof(struct EnginePlugin));
}
struct EnginePlugin* luaX_checkEnginePlugin(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "EnginePlugin");
}
static int f_new_EnginePlugin(lua_State *L) {
	struct EnginePlugin* self = lua_newuserdata(L, sizeof(struct EnginePlugin));
	luaL_setmetatable(L, "EnginePlugin");
	memset(self, 0, sizeof(struct EnginePlugin));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Name"), strncpy(self->Name, luaL_optstring(L, -1, ""), sizeof(self->Name)), 1));
	} else {
		strncpy(self->Name, luaL_optstring(L, 1, ""), sizeof(self->Name));
	}
	return 1;
}


int f_EnginePlugin___index(lua_State *L) {
	struct EnginePlugin* self = luaX_checkEnginePlugin(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x0fe07306: lua_pushstring(L, self->Name); return 1; // Name
	}
	return luaL_error(L, "Unknown field in EnginePlugin(%p): %s", self, luaL_checkstring(L, 2));
}
int f_EnginePlugin___newindex(lua_State *L) {
	struct EnginePlugin* self = luaX_checkEnginePlugin(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x0fe07306: strncpy(self->Name, luaL_optstring(L, 3, ""), sizeof(self->Name)); return 0; // Name
	}
	return luaL_error(L, "Unknown field in EnginePlugin(%p): %s", self, luaL_checkstring(L, 2));
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_EnginePlugin___fromstring(lua_State *L) {
	fixedString_t Name;
	struct EnginePlugin self = {0};
	switch (sscanf(luaL_checkstring(L, 1), "%s", Name)) {
	case 1: 
		strncpy(self.Name, Name, sizeof(self.Name));
		return (luaX_pushEnginePlugin(L, &self), 1);
	default:
		return luaL_error(L, "Invalid format for EnginePlugin: %s", luaL_checkstring(L, 1));
	}
}
static int f_EnginePlugin___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_EnginePlugin(L));  // remove EnginePlugin from stack and call constructor
}
int luaopen_orca_EnginePlugin(lua_State *L) {
	luaL_newmetatable(L, "EnginePlugin");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_EnginePlugin },
		{ "fromstring", f_EnginePlugin___fromstring },
		{ "__newindex", f_EnginePlugin___newindex },
		{ "__index", f_EnginePlugin___index },
		{ NULL, NULL },
	}), 0);
	// Make EnginePlugin creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_EnginePlugin___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushSystemMessage(lua_State *L, struct SystemMessage const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct SystemMessage* self = lua_newuserdata(L, sizeof(struct SystemMessage));
	luaL_setmetatable(L, "SystemMessage");
	memcpy(self, data, sizeof(struct SystemMessage));
}
struct SystemMessage* luaX_checkSystemMessage(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "SystemMessage");
}
static int f_new_SystemMessage(lua_State *L) {
	struct SystemMessage* self = lua_newuserdata(L, sizeof(struct SystemMessage));
	luaL_setmetatable(L, "SystemMessage");
	memset(self, 0, sizeof(struct SystemMessage));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Message"), strncpy(self->Message, luaL_optstring(L, -1, ""), sizeof(self->Message)), 1));
		lua_pop(L, (lua_getfield(L, 1, "Key"), strncpy(self->Key, luaL_optstring(L, -1, ""), sizeof(self->Key)), 1));
		lua_pop(L, (lua_getfield(L, 1, "Command"), strncpy(self->Command, luaL_optstring(L, -1, ""), sizeof(self->Command)), 1));
	} else {
		strncpy(self->Message, luaL_optstring(L, 1, ""), sizeof(self->Message));
		strncpy(self->Key, luaL_optstring(L, 2, ""), sizeof(self->Key));
		strncpy(self->Command, luaL_optstring(L, 3, ""), sizeof(self->Command));
	}
	return 1;
}


int f_SystemMessage___index(lua_State *L) {
	struct SystemMessage* self = luaX_checkSystemMessage(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xae0ed984: lua_pushstring(L, self->Message); return 1; // Message
	case 0xcd1ac90c: lua_pushstring(L, self->Key); return 1; // Key
	case 0xc67c8f52: lua_pushstring(L, self->Command); return 1; // Command
	}
	return luaL_error(L, "Unknown field in SystemMessage(%p): %s", self, luaL_checkstring(L, 2));
}
int f_SystemMessage___newindex(lua_State *L) {
	struct SystemMessage* self = luaX_checkSystemMessage(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xae0ed984: strncpy(self->Message, luaL_optstring(L, 3, ""), sizeof(self->Message)); return 0; // Message
	case 0xcd1ac90c: strncpy(self->Key, luaL_optstring(L, 3, ""), sizeof(self->Key)); return 0; // Key
	case 0xc67c8f52: strncpy(self->Command, luaL_optstring(L, 3, ""), sizeof(self->Command)); return 0; // Command
	}
	return luaL_error(L, "Unknown field in SystemMessage(%p): %s", self, luaL_checkstring(L, 2));
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_SystemMessage___fromstring(lua_State *L) {
	fixedString_t Message;
	fixedString_t Key;
	fixedString_t Command;
	struct SystemMessage self = {0};
	switch (sscanf(luaL_checkstring(L, 1), "%s %s %s", Message, Key, Command)) {
	case 3: 
		strncpy(self.Message, Message, sizeof(self.Message));
		strncpy(self.Key, Key, sizeof(self.Key));
		strncpy(self.Command, Command, sizeof(self.Command));
		return (luaX_pushSystemMessage(L, &self), 1);
	default:
		return luaL_error(L, "Invalid format for SystemMessage: %s", luaL_checkstring(L, 1));
	}
}
static int f_SystemMessage___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_SystemMessage(L));  // remove SystemMessage from stack and call constructor
}
int luaopen_orca_SystemMessage(lua_State *L) {
	luaL_newmetatable(L, "SystemMessage");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_SystemMessage },
		{ "fromstring", f_SystemMessage___fromstring },
		{ "__newindex", f_SystemMessage___newindex },
		{ "__index", f_SystemMessage___index },
		{ NULL, NULL },
	}), 0);
	// Make SystemMessage creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_SystemMessage___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushOpenFileMsgArgs(lua_State *L, struct OpenFileMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct OpenFileMsgArgs* self = lua_newuserdata(L, sizeof(struct OpenFileMsgArgs));
	luaL_setmetatable(L, "OpenFileMsgArgs");
	memcpy(self, data, sizeof(struct OpenFileMsgArgs));
}
struct OpenFileMsgArgs* luaX_checkOpenFileMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "OpenFileMsgArgs");
}
static int f_new_OpenFileMsgArgs(lua_State *L) {
	struct OpenFileMsgArgs* self = lua_newuserdata(L, sizeof(struct OpenFileMsgArgs));
	luaL_setmetatable(L, "OpenFileMsgArgs");
	memset(self, 0, sizeof(struct OpenFileMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "FileName"), self->FileName = lua_type(L, -1) == LUA_TSTRING ? strdup(luaL_checkstring(L, -1)) : NULL, 1));
	} else {
		self->FileName = lua_type(L, 1) == LUA_TSTRING ? strdup(luaL_checkstring(L, 1)) : NULL;
	}
	return 1;
}
int f_OpenFileMsgArgs___index(lua_State *L) {
	struct OpenFileMsgArgs* self = luaX_checkOpenFileMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x5ffdd888: lua_pushstring(L, self->FileName); return 1; // FileName
	}
	return luaL_error(L, "Unknown field in OpenFileMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_OpenFileMsgArgs___newindex(lua_State *L) {
	struct OpenFileMsgArgs* self = luaX_checkOpenFileMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x5ffdd888: self->FileName = lua_type(L, 3) == LUA_TSTRING ? strdup(luaL_checkstring(L, 3)) : NULL; return 0; // FileName
	}
	return luaL_error(L, "Unknown field in OpenFileMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_OpenFileMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_OpenFileMsgArgs(L));  // remove OpenFileMsgArgs from stack and call constructor
}
int luaopen_orca_OpenFileMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "OpenFileMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_OpenFileMsgArgs },
		{ "__newindex", f_OpenFileMsgArgs___newindex },
		{ "__index", f_OpenFileMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make OpenFileMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_OpenFileMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushFileExistsMsgArgs(lua_State *L, struct FileExistsMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct FileExistsMsgArgs* self = lua_newuserdata(L, sizeof(struct FileExistsMsgArgs));
	luaL_setmetatable(L, "FileExistsMsgArgs");
	memcpy(self, data, sizeof(struct FileExistsMsgArgs));
}
struct FileExistsMsgArgs* luaX_checkFileExistsMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "FileExistsMsgArgs");
}
static int f_new_FileExistsMsgArgs(lua_State *L) {
	struct FileExistsMsgArgs* self = lua_newuserdata(L, sizeof(struct FileExistsMsgArgs));
	luaL_setmetatable(L, "FileExistsMsgArgs");
	memset(self, 0, sizeof(struct FileExistsMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "FileName"), self->FileName = lua_type(L, -1) == LUA_TSTRING ? strdup(luaL_checkstring(L, -1)) : NULL, 1));
	} else {
		self->FileName = lua_type(L, 1) == LUA_TSTRING ? strdup(luaL_checkstring(L, 1)) : NULL;
	}
	return 1;
}
int f_FileExistsMsgArgs___index(lua_State *L) {
	struct FileExistsMsgArgs* self = luaX_checkFileExistsMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x5ffdd888: lua_pushstring(L, self->FileName); return 1; // FileName
	}
	return luaL_error(L, "Unknown field in FileExistsMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_FileExistsMsgArgs___newindex(lua_State *L) {
	struct FileExistsMsgArgs* self = luaX_checkFileExistsMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x5ffdd888: self->FileName = lua_type(L, 3) == LUA_TSTRING ? strdup(luaL_checkstring(L, 3)) : NULL; return 0; // FileName
	}
	return luaL_error(L, "Unknown field in FileExistsMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_FileExistsMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_FileExistsMsgArgs(L));  // remove FileExistsMsgArgs from stack and call constructor
}
int luaopen_orca_FileExistsMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "FileExistsMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_FileExistsMsgArgs },
		{ "__newindex", f_FileExistsMsgArgs___newindex },
		{ "__index", f_FileExistsMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make FileExistsMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_FileExistsMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushLoadProjectMsgArgs(lua_State *L, struct LoadProjectMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct LoadProjectMsgArgs* self = lua_newuserdata(L, sizeof(struct LoadProjectMsgArgs));
	luaL_setmetatable(L, "LoadProjectMsgArgs");
	memcpy(self, data, sizeof(struct LoadProjectMsgArgs));
}
struct LoadProjectMsgArgs* luaX_checkLoadProjectMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "LoadProjectMsgArgs");
}
static int f_new_LoadProjectMsgArgs(lua_State *L) {
	struct LoadProjectMsgArgs* self = lua_newuserdata(L, sizeof(struct LoadProjectMsgArgs));
	luaL_setmetatable(L, "LoadProjectMsgArgs");
	memset(self, 0, sizeof(struct LoadProjectMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Path"), self->Path = lua_type(L, -1) == LUA_TSTRING ? strdup(luaL_checkstring(L, -1)) : NULL, 1));
	} else {
		self->Path = lua_type(L, 1) == LUA_TSTRING ? strdup(luaL_checkstring(L, 1)) : NULL;
	}
	return 1;
}
int f_LoadProjectMsgArgs___index(lua_State *L) {
	struct LoadProjectMsgArgs* self = luaX_checkLoadProjectMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xeb66e456: lua_pushstring(L, self->Path); return 1; // Path
	}
	return luaL_error(L, "Unknown field in LoadProjectMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_LoadProjectMsgArgs___newindex(lua_State *L) {
	struct LoadProjectMsgArgs* self = luaX_checkLoadProjectMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xeb66e456: self->Path = lua_type(L, 3) == LUA_TSTRING ? strdup(luaL_checkstring(L, 3)) : NULL; return 0; // Path
	}
	return luaL_error(L, "Unknown field in LoadProjectMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_LoadProjectMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_LoadProjectMsgArgs(L));  // remove LoadProjectMsgArgs from stack and call constructor
}
int luaopen_orca_LoadProjectMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "LoadProjectMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_LoadProjectMsgArgs },
		{ "__newindex", f_LoadProjectMsgArgs___newindex },
		{ "__index", f_LoadProjectMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make LoadProjectMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_LoadProjectMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }


static struct PropertyType const WorkspaceProperties[kWorkspaceNumProperties] = {
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
ORCA_API struct ClassDesc _Workspace = {
	.ClassName = "Workspace",
	.DefaultName = "Workspace",
	.ContentType = "Workspace",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Workspace,
	.ClassSize = sizeof(struct Workspace),
	.Properties = WorkspaceProperties,
	.ObjProc = WorkspaceProc,
	.Defaults = &WorkspaceDefaults,
	.NumProperties = kWorkspaceNumProperties,
};


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
ORCA_API struct ClassDesc _Library = {
	.ClassName = "Library",
	.DefaultName = "Library",
	.ContentType = "Library",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Library,
	.ClassSize = sizeof(struct Library),
	.Properties = LibraryProperties,
	.ObjProc = LibraryProc,
	.Defaults = &LibraryDefaults,
	.NumProperties = kLibraryNumProperties,
};

LRESULT Project_Start(struct Object*, struct Project*, wParam_t, StartMsgPtr);

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
	DECL(0x5488c4f2, Project, CompositionDesignSize, CompositionDesignSize, kDataTypeStruct, .TypeString = "Vector2D"), // Project.CompositionDesignSize
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
};
static struct Project ProjectDefaults = {
};
LRESULT ProjectProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgStart: return Project_Start(object, cmp, wparm, lparm); // Start
	}
	return FALSE;
}
void luaX_pushProject(lua_State *L, struct Project const* Project) {
	luaX_pushObject(L, CMP_GetObject(Project));
}
struct Project* luaX_checkProject(lua_State *L, int idx) {
	return GetProject(luaX_checkObject(L, idx));
}
ORCA_API struct ClassDesc _Project = {
	.ClassName = "Project",
	.DefaultName = "Project",
	.ContentType = "Project",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Project,
	.ClassSize = sizeof(struct Project),
	.Properties = ProjectProperties,
	.ObjProc = ProjectProc,
	.Defaults = &ProjectDefaults,
	.NumProperties = kProjectNumProperties,
};

LRESULT Directory_LoadProject(struct Object*, struct Directory*, wParam_t, LoadProjectMsgPtr);
LRESULT Directory_OpenFile(struct Object*, struct Directory*, wParam_t, OpenFileMsgPtr);
LRESULT Directory_FileExists(struct Object*, struct Directory*, wParam_t, FileExistsMsgPtr);
LRESULT Directory_HasChangedFiles(struct Object*, struct Directory*, wParam_t, HasChangedFilesMsgPtr);
LRESULT Directory_Destroy(struct Object*, struct Directory*, wParam_t, DestroyMsgPtr);

static struct PropertyType const DirectoryProperties[kDirectoryNumProperties] = {
	DECL(0xeb66e456, Directory, Path, Path, kDataTypeString), // Directory.Path
};
static struct Directory DirectoryDefaults = {
};
LRESULT DirectoryProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgLoadProject: return Directory_LoadProject(object, cmp, wparm, lparm); // LoadProject
		case kMsgOpenFile: return Directory_OpenFile(object, cmp, wparm, lparm); // OpenFile
		case kMsgFileExists: return Directory_FileExists(object, cmp, wparm, lparm); // FileExists
		case kMsgHasChangedFiles: return Directory_HasChangedFiles(object, cmp, wparm, lparm); // HasChangedFiles
		case kMsgDestroy: return Directory_Destroy(object, cmp, wparm, lparm); // Destroy
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
ORCA_API struct ClassDesc _Directory = {
	.ClassName = "Directory",
	.DefaultName = "Directory",
	.ContentType = "Directory",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Project, 0 },
	.ClassID = ID_Directory,
	.ClassSize = sizeof(struct Directory),
	.Properties = DirectoryProperties,
	.ObjProc = DirectoryProc,
	.Defaults = &DirectoryDefaults,
	.NumProperties = kDirectoryNumProperties,
};

LRESULT Package_LoadProject(struct Object*, struct Package*, wParam_t, LoadProjectMsgPtr);
LRESULT Package_OpenFile(struct Object*, struct Package*, wParam_t, OpenFileMsgPtr);
LRESULT Package_FileExists(struct Object*, struct Package*, wParam_t, FileExistsMsgPtr);
LRESULT Package_HasChangedFiles(struct Object*, struct Package*, wParam_t, HasChangedFilesMsgPtr);
LRESULT Package_Destroy(struct Object*, struct Package*, wParam_t, DestroyMsgPtr);

static struct PropertyType const PackageProperties[kPackageNumProperties] = {
	DECL(0x5ffdd888, Package, FileName, FileName, kDataTypeString), // Package.FileName
};
static struct Package PackageDefaults = {
};
LRESULT PackageProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgLoadProject: return Package_LoadProject(object, cmp, wparm, lparm); // LoadProject
		case kMsgOpenFile: return Package_OpenFile(object, cmp, wparm, lparm); // OpenFile
		case kMsgFileExists: return Package_FileExists(object, cmp, wparm, lparm); // FileExists
		case kMsgHasChangedFiles: return Package_HasChangedFiles(object, cmp, wparm, lparm); // HasChangedFiles
		case kMsgDestroy: return Package_Destroy(object, cmp, wparm, lparm); // Destroy
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
ORCA_API struct ClassDesc _Package = {
	.ClassName = "Package",
	.DefaultName = "Package",
	.ContentType = "Package",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Project, 0 },
	.ClassID = ID_Package,
	.ClassSize = sizeof(struct Package),
	.Properties = PackageProperties,
	.ObjProc = PackageProc,
	.Defaults = &PackageDefaults,
	.NumProperties = kPackageNumProperties,
};


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
ORCA_API struct ClassDesc _LocaleReferenceItem = {
	.ClassName = "LocaleReferenceItem",
	.DefaultName = "LocaleReferenceItem",
	.ContentType = "LocaleReferenceItem",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_LocaleReferenceItem,
	.ClassSize = sizeof(struct LocaleReferenceItem),
	.Properties = LocaleReferenceItemProperties,
	.ObjProc = LocaleReferenceItemProc,
	.Defaults = &LocaleReferenceItemDefaults,
	.NumProperties = kLocaleReferenceItemNumProperties,
};


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
ORCA_API struct ClassDesc _Tag = {
	.ClassName = "Tag",
	.DefaultName = "Tag",
	.ContentType = "Tag",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Tag,
	.ClassSize = sizeof(struct Tag),
	.Properties = TagProperties,
	.ObjProc = TagProc,
	.Defaults = &TagDefaults,
	.NumProperties = kTagNumProperties,
};

LRESULT ThemeGroup_Attached(struct Object*, struct ThemeGroup*, wParam_t, AttachedMsgPtr);

static struct PropertyType const ThemeGroupProperties[kThemeGroupNumProperties] = {
	DECL(0x75516381, ThemeGroup, SelectedTheme, SelectedTheme, kDataTypeString), // ThemeGroup.SelectedTheme
	DECL(0x1cf2c938, ThemeGroup, SelectedDictionary, SelectedDictionary, kDataTypeString), // ThemeGroup.SelectedDictionary
};
static struct ThemeGroup ThemeGroupDefaults = {
};
LRESULT ThemeGroupProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgAttached: return ThemeGroup_Attached(object, cmp, wparm, lparm); // Attached
	}
	return FALSE;
}
void luaX_pushThemeGroup(lua_State *L, struct ThemeGroup const* ThemeGroup) {
	luaX_pushObject(L, CMP_GetObject(ThemeGroup));
}
struct ThemeGroup* luaX_checkThemeGroup(lua_State *L, int idx) {
	return GetThemeGroup(luaX_checkObject(L, idx));
}
ORCA_API struct ClassDesc _ThemeGroup = {
	.ClassName = "ThemeGroup",
	.DefaultName = "ThemeGroup",
	.ContentType = "ThemeGroup",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_ThemeGroup,
	.ClassSize = sizeof(struct ThemeGroup),
	.Properties = ThemeGroupProperties,
	.ObjProc = ThemeGroupProc,
	.Defaults = &ThemeGroupDefaults,
	.NumProperties = kThemeGroupNumProperties,
};


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
ORCA_API struct ClassDesc _Theme = {
	.ClassName = "Theme",
	.DefaultName = "Theme",
	.ContentType = "Theme",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Theme,
	.ClassSize = sizeof(struct Theme),
	.Properties = ThemeProperties,
	.ObjProc = ThemeProc,
	.Defaults = &ThemeDefaults,
	.NumProperties = kThemeNumProperties,
};


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
ORCA_API struct ClassDesc _Entry = {
	.ClassName = "Entry",
	.DefaultName = "Entry",
	.ContentType = "Entry",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Entry,
	.ClassSize = sizeof(struct Entry),
	.Properties = EntryProperties,
	.ObjProc = EntryProc,
	.Defaults = &EntryDefaults,
	.NumProperties = kEntryNumProperties,
};


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
ORCA_API struct ClassDesc _ThemeDefaultValuesDictionary = {
	.ClassName = "ThemeDefaultValuesDictionary",
	.DefaultName = "ThemeDefaultValuesDictionary",
	.ContentType = "ThemeDefaultValuesDictionary",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_ThemeDefaultValuesDictionary,
	.ClassSize = sizeof(struct ThemeDefaultValuesDictionary),
	.Properties = ThemeDefaultValuesDictionaryProperties,
	.ObjProc = ThemeDefaultValuesDictionaryProc,
	.Defaults = &ThemeDefaultValuesDictionaryDefaults,
	.NumProperties = kThemeDefaultValuesDictionaryNumProperties,
};


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
	lua_setfield(L, ((void)luaopen_orca_OpenFileMsgArgs(L), -2), "OpenFileMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_FileExistsMsgArgs(L), -2), "FileExistsMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_LoadProjectMsgArgs(L), -2), "LoadProjectMsgArgs");
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
