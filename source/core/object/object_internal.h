#ifndef __OBJECT_INTERNAL_H__
#define __OBJECT_INTERNAL_H__

#include <source/core/core_local.h>

/* Generated components may end on a four-byte field. Their trailing property
 * cache still needs pointer alignment on 64-bit targets. */
static inline size_t CMP_PropertyOffset(size_t classSize)
{
  size_t alignment = _Alignof(struct Property *);
  return (classSize + alignment - 1) & ~(alignment - 1);
}

// Closure for controller event handlers: self = controller table (with .view = root node).
int _bind_handler_closure(lua_State *L);
void OBJ_NotifyCollectionChanged(struct Object *object);
void OBJ_ClearCollectionObservers(struct Object *object);

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

  struct component* components;
  struct Property* properties;

  uint32_t alias;
  uint32_t unique;
  uint32_t flags;

  longTime_t dirty;
  event_t body_ref;
  event_t controller_ref;
};


#endif /* __OBJECT_INTERNAL_H__ */
