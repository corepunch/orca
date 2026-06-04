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

  struct component* components;
  struct Property* properties;

  uint32_t alias;
  uint32_t unique;
  uint32_t flags;

  longTime_t dirty;
};


#endif /* __OBJECT_INTERNAL_H__ */
