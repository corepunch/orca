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
static struct PropertyType const FtgPackageProperties[kFtgPackageNumProperties] = {
	DECL(0x5ffdd888, FtgPackage, FileName, FileName, kDataTypeString), // FtgPackage.FileName
};
static struct FtgPackage FtgPackageDefaults = {
};
LRESULT FtgPackageProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Project_LoadProject: return FtgPackage_LoadProject(object, cmp, wparm, lparm); // Project.LoadProject
		case ID_Project_OpenFile: return FtgPackage_OpenFile(object, cmp, wparm, lparm); // Project.OpenFile
		case ID_Project_FileExists: return FtgPackage_FileExists(object, cmp, wparm, lparm); // Project.FileExists
		case ID_Project_HasChangedFiles: return FtgPackage_HasChangedFiles(object, cmp, wparm, lparm); // Project.HasChangedFiles
		case ID_Object_Destroy: return FtgPackage_Destroy(object, cmp, wparm, lparm); // Object.Destroy
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
	void on_darkreign_module_registered(lua_State *L);
	on_darkreign_module_registered(L);
	lua_setfield(L, ((void)lua_pushclass(L, &_FtgPackage), -2), "FtgPackage");
	return 1;
}
