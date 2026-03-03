#include <source/core/core.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

//#define SUPPORT_DYNAMIC_PROPERTIES

ORCA_API void
OBJ_SetSourceFile(lpObject_t, lpcString_t);

ORCA_API void
OBJ_SetClassName(lpObject_t, lpcString_t);

extern void
PROP_AttachProgram(lpProperty_t p, enum PropertyAttribute a, struct token* program, lpcString_t code);

int f_loadProject(lua_State* L);
