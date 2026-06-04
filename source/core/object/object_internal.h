#ifndef __OBJECT_INTERNAL_H__
#define __OBJECT_INTERNAL_H__

#include <source/core/core_local.h>

struct component
{
  struct component* next;
  struct ClassDesc const *pcls;
  struct Object *pobj;
  char pUserData[];
};

struct Object
{
  LPSTR Name;
  uint32_t identifier;
  uint32_t refcount;

  struct Object *parent;
  struct Object *children;
  struct Object *next;

  LPSTR SourceFile;
  LPSTR TextContent;

  struct component* components; /* fallback for classes without a TypedataOffset */
  struct Property* properties;

  uint32_t alias;
  uint32_t unique;
  uint32_t flags;

  uint32_t class_id;            /* concrete class — Button, TextBlock, … */
  struct ClassDesc const *type; /* pointer to concrete ClassDesc */

  longTime_t dirty;

  char typedata[];              /* storage family data — UIData, SceneData, … */
};

/* True when the object uses the monolithic typedata block for storage.
 * False for classes without a registered TypedataOffset (e.g. Viewport3D). */
static INLINE bool_t
OBJ_UsesTypedata(struct Object const *object)
{
  return object &&
         object->type &&
         object->type->TypedataOffset != UINT32_MAX;
}

void OBJ_ApplyInheritedProperties(struct Object *object);
void OBJ_PropagateInheritedProperty(struct Object *object, struct Property *property);

#endif /* __OBJECT_INTERNAL_H__ */
