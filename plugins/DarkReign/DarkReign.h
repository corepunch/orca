// Auto-generated from DarkReign.xml by tools/templates/header.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __DARKREIGN_H__
#define __DARKREIGN_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;

struct _FTG;


#include "DarkReign_properties.h"
#include "../../source/filesystem/filesystem.h"










/// @brief Loader for FTG archives (Dark Reign game format)
/** FtgPackage component */
typedef struct FtgPackage FtgPackage_t, *FtgPackagePtr, *lpFtgPackage_t;
typedef struct FtgPackage const *FtgPackageCPtr, *lpcFtgPackage_t;
struct FtgPackage {
	const char* FileName;
	struct _FTG* _ftg;
};
ORCA_API void luaX_pushFtgPackage(lua_State *L, struct FtgPackage const* FtgPackage);
ORCA_API struct FtgPackage* luaX_checkFtgPackage(lua_State *L, int idx);

#endif