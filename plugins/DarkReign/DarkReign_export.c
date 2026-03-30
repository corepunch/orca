// Auto-generated from DarkReign.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "DarkReign.h"

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
#define FIELD(IDENT, STRUCT, NAME, TYPE, ...) { \
.Name = #NAME, \
.ShortIdentifier = IDENT, \
.DataType = TYPE, \
.Offset = offsetof(struct STRUCT, NAME), \
.DataSize = sizeof(((struct STRUCT*)NULL)->NAME), \
##__VA_ARGS__ \
}
#define STRUCT(NAME, EXPORT, ...) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
if (data == NULL) { lua_pushnil(L); return; } \
struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
luaL_setmetatable(L, EXPORT); \
memcpy(self, data, sizeof(struct NAME)); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { \
return luaL_checkudata(L, idx, EXPORT); \
} \
static int f_new_##NAME(lua_State *L) { \
struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
luaL_setmetatable(L, EXPORT); \
memset(self, 0, sizeof(struct NAME)); \
if (lua_gettop(L) == 1) return 1; \
luaX_struct_new(L, self, _##NAME, sizeof(_##NAME)/sizeof(*_##NAME)); \
return 1; \
} \
int f_##NAME##___index(lua_State *L) { \
struct NAME* self = luaX_check##NAME(L, 1); \
if (luaX_struct_index(L, self, _##NAME, sizeof(_##NAME)/sizeof(*_##NAME))) return 1; \
lua_getmetatable(L, 1); \
lua_pushvalue(L, 2); \
lua_rawget(L, -2); \
return 1; \
} \
int f_##NAME##___newindex(lua_State *L) { \
return luaX_struct_newindex(L, luaX_check##NAME(L, 1), _##NAME, sizeof(_##NAME)/sizeof(*_##NAME)); \
} \
static int f_##NAME##___call(lua_State *L) { \
return ((void)lua_remove(L, 1), f_new_##NAME(L)); \
} \
int luaopen_orca_##NAME(lua_State *L) { \
luaL_newmetatable(L, EXPORT); \
luaL_setfuncs(L, ((luaL_Reg[]) { \
{ "new", f_new_##NAME }, \
{ "__newindex", f_##NAME##___newindex }, \
{ "__index", f_##NAME##___index }, \
__VA_ARGS__ \
{ NULL, NULL }, \
}), 0); \
lua_newtable(L); \
lua_pushcfunction(L, f_##NAME##___call); \
lua_setfield(L, -2, "__call"); \
lua_setmetatable(L, -2); \
return 1; \
}

#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }

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
ORCA_API struct ClassDesc _FtgPackage = {
	.ClassName = "FtgPackage",
	.DefaultName = "FtgPackage",
	.ContentType = "FtgPackage",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Bundle, 0 },
	.ClassID = ID_FtgPackage,
	.ClassSize = sizeof(struct FtgPackage),
	.Properties = FtgPackageProperties,
	.MessageTypes = FtgPackageMessageTypes,
	.ObjProc = FtgPackageProc,
	.Defaults = &FtgPackageDefaults,
	.NumProperties = kFtgPackageNumProperties,
	.NumMessageTypes = kFtgPackageNumMessageTypes,
};



ORCA_API int luaopen_orca_DarkReign(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ NULL, NULL } 
	}));
	void on_darkreign_module_registered(lua_State *L);
	on_darkreign_module_registered(L);
	lua_setfield(L, ((void)lua_pushclass(L, &_FtgPackage), -2), "FtgPackage");
	return 1;
}
