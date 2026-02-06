#include <source/core/core.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

//#define SUPPORT_DYNAMIC_PROPERTIES

ORCA_API void
OBJ_SetSourceFile(lpObject_t, lpcString_t);

ORCA_API void
OBJ_SetClassName(lpObject_t, lpcString_t);

lpObject_t
OBJ_LoadDocument(lua_State* L, xmlDocPtr);

lpcString_t
PDESC_Parse(lpObject_t hobj, lpcPropertyDesc_t pdesc, lpProperty_t prop, lpcString_t string, void* dest);

int
PROP_Parse2(lua_State* L, lpObject_t, lpcString_t szKey, lpcString_t szValue);

lpProperty_t
PROP_CreateFromString(lua_State* L, lpObject_t object, lpcString_t name, lpcString_t typestring);

int f_loadProject(lua_State* L);
