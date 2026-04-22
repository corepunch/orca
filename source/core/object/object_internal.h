#ifndef __OBJECT_INTERNAL_H__
#define __OBJECT_INTERNAL_H__

#include <source/core/core_local.h>

#define MAX_OBJECT_DATA (MAX_PROPERTY_STRING*32) // 1024

struct component
{
  struct component* next;
  lpcClassDesc_t pcls;
  lpObject_t pobj;
  char pUserData[];
};

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

  struct component* components;
  struct Property* properties;

  uint32_t alias;
  uint32_t unique;
  uint32_t userdata;
  uint32_t flags;
  uint32_t rdflags;
  uint32_t datasize;
  objectTags_t tags;

  longTime_t dirty;
  
  lua_State *domain;
  
  byte_t   data[MAX_OBJECT_DATA];
};

#endif /* __OBJECT_INTERNAL_H__ */
