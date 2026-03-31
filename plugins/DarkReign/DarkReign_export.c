// Auto-generated from DarkReign.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "DarkReign.h"

#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }

// _FTG
extern void luaX_push_FTG(lua_State *L, struct _FTG const* value);
extern struct _FTG* luaX_check_FTG(lua_State *L, int index);

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
extern int write_property(lua_State *L, int idx, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);

#define STRUCT(NAME, EXPORT) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
	if (data == NULL) { lua_pushnil(L); return; } \
	struct NAME* ud = lua_newuserdata(L, sizeof(struct NAME)); \
	luaL_setmetatable(L, #EXPORT); \
	memcpy(ud, data, sizeof(struct NAME)); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { return luaL_checkudata(L, idx, #EXPORT); } \
static int f_new_##NAME(lua_State *L) { \
	struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
	luaL_setmetatable(L, #EXPORT); \
	memset(self, 0, sizeof(struct NAME)); \
	if (lua_istable(L, 1)) \
    for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
			lua_pop(L, (lua_getfield(L, 1, _##NAME[i].Name), read_property(L, -1, &_##NAME[i], ((char*)self)+_##NAME[i].Offset), 1)); \
	else for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		read_property(L, i + 1, &_##NAME[i], ((char*)self)+_##NAME[i].Offset); \
	return 1; \
} \
static int f_##NAME##___index(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (write_property(L, -1, &_##NAME[i], ((char*)luaX_check##NAME(L, 1))+_##NAME[i].Offset), 1); \
	for (uint32_t i = 0; i < sizeof(_##NAME##_Methods) / sizeof(*_##NAME##_Methods); i++) { \
		if (strcmp(_##NAME##_Methods[i].name, luaL_checkstring(L, 2)) == 0) { \
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
			parse_property(tok, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); \
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
	.MessageTypes = NAME##MessageTypes, \
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
	.NumMessageTypes = k##NAME##NumMessageTypes, \
};
LRESULT FtgPackage_LoadProject(struct Object*, struct FtgPackage*, wParam_t, LoadProjectMsgPtr);
LRESULT FtgPackage_OpenFile(struct Object*, struct FtgPackage*, wParam_t, OpenFileMsgPtr);
LRESULT FtgPackage_FileExists(struct Object*, struct FtgPackage*, wParam_t, FileExistsMsgPtr);
LRESULT FtgPackage_HasChangedFiles(struct Object*, struct FtgPackage*, wParam_t, HasChangedFilesMsgPtr);
LRESULT FtgPackage_Destroy(struct Object*, struct FtgPackage*, wParam_t, DestroyMsgPtr);
static struct MessageType FtgPackageMessageTypes[kFtgPackageNumMessageTypes] = {	
};
static struct PropertyType const FtgPackageProperties[kFtgPackageNumProperties] = {
	DECL(0x5ffdd888, FtgPackage, FileName, FileName, kDataTypeString), // FtgPackage.FileName
};
static struct FtgPackage FtgPackageDefaults = {
};
LRESULT FtgPackageProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgLoadProject: return FtgPackage_LoadProject(object, cmp, wparm, lparm); // LoadProject
		case kMsgOpenFile: return FtgPackage_OpenFile(object, cmp, wparm, lparm); // OpenFile
		case kMsgFileExists: return FtgPackage_FileExists(object, cmp, wparm, lparm); // FileExists
		case kMsgHasChangedFiles: return FtgPackage_HasChangedFiles(object, cmp, wparm, lparm); // HasChangedFiles
		case kMsgDestroy: return FtgPackage_Destroy(object, cmp, wparm, lparm); // Destroy
	}
	return FALSE;
}
void luaX_pushFtgPackage(lua_State *L, struct FtgPackage const* FtgPackage) {
	luaX_pushObject(L, CMP_GetObject(FtgPackage));
}
struct FtgPackage* luaX_checkFtgPackage(lua_State *L, int idx) {
	return GetFtgPackage(luaX_checkObject(L, idx));
}
#define ID_Bundle 0xe6397a25
REGISTER_CLASS(FtgPackage, ID_Bundle, 0);


ORCA_API int luaopen_orca_DarkReign(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ NULL, NULL } 
	}));
	lua_setfield(L, ((void)lua_pushclass(L, &_FtgPackage), -2), "FtgPackage");
	void on_darkreign_module_registered(lua_State *L);
	on_darkreign_module_registered(L);
	return 1;
}
