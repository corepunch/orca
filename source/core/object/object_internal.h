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

/* Storage-family IDs — fnv1a32 of the root class name */
#define SUPER_ID_NODE         0x3468032du
#define SUPER_ID_NODE2D       0x6c63a2abu
#define SUPER_ID_NODE3D       0xce61fe5au
#define SUPER_ID_RESOURCE     0x23b0d815u
#define SUPER_ID_ACTION       0x0a77a91fu
#define SUPER_ID_PROJECT      0x7b5fea5eu

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

  struct component* components;
  struct Property* properties;

  uint32_t alias;
  uint32_t unique;
  uint32_t flags;

  uint32_t class_id;            /* concrete class — Button, TextBlock, … */
  uint32_t super_id;            /* storage family — SUPER_ID_NODE2D, … */
  struct ClassDesc const *type; /* pointer to concrete ClassDesc */

  longTime_t dirty;

  char typedata[];              /* storage family data — UIData, SceneData, … */
};

/* True when the object uses the monolithic typedata block for storage.
 * False for classes that inherit from Node2D but haven't registered a
 * TypedataOffset (e.g. Viewport3D from SceneKit). */
static INLINE bool_t
OBJ_UsesTypedata(struct Object const *object)
{
  return object &&
         object->super_id == SUPER_ID_NODE2D &&
         object->type &&
         object->type->TypedataOffset != UINT32_MAX;
}

void OBJ_ApplyInheritedProperties(struct Object *object);
void OBJ_PropagateInheritedProperty(struct Object *object, struct Property *property);

#endif /* __OBJECT_INTERNAL_H__ */
