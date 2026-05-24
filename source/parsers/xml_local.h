#include <source/core/core.h>
#include <source/parsers/p_xml_dom.h>

//#define SUPPORT_DYNAMIC_PROPERTIES

ORCA_API void
OBJ_SetSourceFile(struct Object *, lpcString_t);

ORCA_API void
OBJ_SetClassName(struct Object *, lpcString_t);

extern void
PROP_AttachProgram(struct Property *p, struct token* program);
