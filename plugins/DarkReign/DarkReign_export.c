// Auto-generated from DarkReign.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>
#include <include/codegen.h>

#include "DarkReign.h"

// _FTG
extern void luaX_push_FTG(lua_State *L, struct _FTG const* value);
extern struct _FTG* luaX_check_FTG(lua_State *L, int index);

extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);



HANDLER(FtgPackage, Project, LoadProject);
HANDLER(FtgPackage, Project, OpenFile);
HANDLER(FtgPackage, Project, FileExists);
HANDLER(FtgPackage, Project, HasChangedFiles);
HANDLER(FtgPackage, Object, Destroy);
COMPONENT_PROPERTIES(FtgPackage) = {
	DECL_STRING(0x5ffdd888, FtgPackage, FileName, FileName), // FtgPackage.FileName
};
COMPONENT_DEFAULTS(FtgPackage) = {
};
COMPONENT_PROC_BEGIN(FtgPackage)
		DISPATCH(FtgPackage, Project, LoadProject) // Project.LoadProject
		DISPATCH(FtgPackage, Project, OpenFile) // Project.OpenFile
		DISPATCH(FtgPackage, Project, FileExists) // Project.FileExists
		DISPATCH(FtgPackage, Project, HasChangedFiles) // Project.HasChangedFiles
		DISPATCH(FtgPackage, Object, Destroy) // Object.Destroy
COMPONENT_PROC_END()
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
	void on_darkreign_module_registered(lua_State *L);
	on_darkreign_module_registered(L);
	lua_setfield(L, ((void)lua_pushclass(L, &_FtgPackage), -2), "FtgPackage");
	return 1;
}
