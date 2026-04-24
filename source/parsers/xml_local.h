#include <source/core/core.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

//#define SUPPORT_DYNAMIC_PROPERTIES

ORCA_API void
OBJ_SetSourceFile(struct Object *, lpcString_t);

ORCA_API void
OBJ_SetClassName(struct Object *, lpcString_t);

extern void
PROP_AttachProgram(struct Property *p, enum PropertyAttribute a, struct token* program, lpcString_t code);
