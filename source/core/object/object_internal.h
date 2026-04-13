#ifndef __OBJECT_INTERNAL_H__
#define __OBJECT_INTERNAL_H__

#include <source/core/core_local.h>

#define MAX_OBJECT_DATA (MAX_PROPERTY_STRING*32) // 1024

struct Object
{
  LPSTR Name;
  uint32_t identifier;

  lpObject_t parent;
  lpObject_t children;
  lpObject_t next;

  LPSTR SourceFile;
  LPSTR TextContent;
  LPSTR ClassName;

  union
  {
    struct
    {
      struct component* components;
      struct Property* properties;
      // aliases moved to the Aliases attach-only component
      // stateManager moved to StateManagerController component
    };
    void* comps[kCompCount];
  };

  uint32_t alias;
  uint32_t unique;
  uint32_t userdata;
  uint32_t luaObject;
  uint32_t flags;
  uint32_t rdflags;
  uint32_t datasize;
  objectTags_t tags;

  longTime_t dirty;
  
  lua_State *domain;
  
  byte_t   data[MAX_OBJECT_DATA];
};

#endif /* __OBJECT_INTERNAL_H__ */
