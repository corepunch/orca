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
  
  lpcKeyframeAnim_t animation;
  lpKeyframeAnim_t animlib;

  union
  {
    struct
    {
      struct component* components;
      struct property_animation* animations;
      struct Property* properties;
      struct state_manager* stateManager;
      struct style_class* classes;
      struct style_sheet* stylesheet;
      struct timer* timers;
      struct alias* aliases;
      struct script_callback* callbacks2;
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

lpKeyframeAnim_t ANIM_FindByName(lpKeyframeAnim_t anims, lpcString_t name);

#endif /* __OBJECT_INTERNAL_H__ */
