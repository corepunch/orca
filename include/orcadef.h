#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

#define ORCA_PACKAGE_NAME "package.xml"
#define ORCA_PACK_VERSION 105
#define ORCA_API __attribute__((visibility("default")))
#define MAX_NAMELEN 64
#define MAX_ENTITIES 1024
#define MAX_UNIFORMS 64
#define MAX_CAMERAS 64
#define MAX_MSGLEN 4096           // max length of a message
#define MAX_OBJECT_PROPERTIES 256 // for editor
#define FPS_BUFFER_SIZE 64
#define PACKET_HEADER 10 // two ints and a short
#define LERP(a, b, t) (a) * (1 - (t)) + (b) * (t)

// Mesh pointer boxing - tagged pointer system for mesh references
#define MESH_TAG_MASK   0x7ULL
#define MESH_PTR_MASK  (~MESH_TAG_MASK)
#define LOWORD(l) ((uint16_t)(l & 0xFFFF))
#define HIWORD(l) ((uint16_t)((l >> 16) & 0xFFFF))
#define MAKEDWORD(low, high)                                                   \
  ((uint32_t)(((uint16_t)(low)) | ((uint32_t)((uint16_t)(high))) << 16))
#define MFCH(ch, n) ((int)(char)(ch) << n)
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                         \
  (MFCH(ch0, 0) | MFCH(ch1, 8) | MFCH(ch2, 16) | MFCH(ch3, 24))
#define DECLARE_INTERFACE(NAME, SYSNAME)                                       \
  typedef struct _##SYSNAME* P##SYSNAME;                                       \
  static inline P##SYSNAME Get##NAME(void)                                     \
  {                                                                            \
    extern P##SYSNAME _##SYSNAME;                                              \
    return _##SYSNAME;                                                         \
  }                                                                            \
  struct _##SYSNAME

#define HANDLER(CLASS, EVENT)                                                  \
  LRESULT CLASS##_##EVENT(lpObject_t hObject,                              \
                          CLASS##Ptr p##CLASS,                                 \
                          wParam_t wParam,                                       \
                          EVENT##EventPtr p##EVENT)

#define MOUSE_EVENTS_USE_LOCAL_SPACE
#define DEFAULT_FONT_SIZE 16
#define MAX_TAGS 64 // to fit into uint64_t
#define STDMETHOD(NAME) HRESULT(*NAME)
#define DEBUG_LUA_STACK Con_Error("%d", lua_gettop(L));
#define LOG_FILENAME "/tmp/orca.log"
#define bounds_size(bounds) ((bounds).max - (bounds).min)

#define XML_FLAGS XML_PARSE_NOBLANKS | XML_PARSE_COMPACT
#define SV_CMD(CMD, NAME) \
lpcString_t CMD##_##NAME(lua_State* L, xmlNodePtr response, lpcString_t endpoint, reqArg_t const *rargs, uint32_t nargs)

#define TRUE 1
#define FALSE 0

#define INLINE static inline

#define ALLOCATE_MODULE(L, IID, STRUCT) \
memset(lua_newuserdata(L, sizeof(STRUCT)), 0, sizeof(STRUCT)); \
lua_setfield(L, LUA_REGISTRYINDEX, IID);

#define PRINT_LINE(x, y, color, ...)                                           \
  memset(txt, 0, sizeof(txt));                                                 \
  sprintf(txt, __VA_ARGS__);                                                   \
  for (int _i = 0; txt[_i]; _i++) {                                            \
    ((unsigned LPSTR)&buf[_i + (y) * CONSOLE_WIDTH + (x)])[0] = txt[_i];       \
    ((unsigned LPSTR)&buf[_i + (y) * CONSOLE_WIDTH + (x)])[1] = color;         \
  }

#define OPTION_VALUE(NAME, ...)                                                \
  void NAME(lua_State* L, uint32_t index, lpProperty_t property)           \
  {                                                                            \
    static lpcString_t values[] = { __VA_ARGS__, NULL };                       \
    uint32_t number;                                                           \
    if (lua_type(L, index) == LUA_TNUMBER) {                                   \
      number = luaL_checkinteger(L, index);                                    \
    } else {                                                                   \
      number = luaL_checkoption(L, index, NULL, values);                       \
    }                                                                          \
    PROP_SetValue(property, &number);                                          \
  }

#define ADD_TO_LIST(property, list)                                            \
  (property)->next = list;                                                     \
  list = property;

#define ADD_TO_LIST_END(type, object, list)                                    \
  if (!(list)) {                                                               \
    ADD_TO_LIST(object, (list));                                               \
  } else {                                                                     \
    type* __last = (list);                                                     \
    while (__last->next)                                                       \
      __last = __last->next;                                                   \
    __last->next = object;                                                     \
  }

#define REMOVE_FROM_LIST(type, property, list)                                 \
  if ((list) == property) {                                                    \
    (list) = (list)->next;                                                     \
    property->next = NULL;                                                     \
  } else if ((list)) {                                                         \
    for (type* a = (list), *b = (list)->next; b; a = b, b = b->next) {         \
      if (b == property) {                                                     \
        a->next = b->next;                                                     \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    property->next = NULL;                                                     \
  }

#define FOR_EACH_LIST(type, property, list)                                    \
  for (type* property = list, *next = list ? (list)->next : NULL; property;    \
       property = next, next = next ? next->next : NULL)

#define xmlForEach(child, xml)                                                 \
  FOR_EACH_LIST(xmlNode, child, (xml)->children)                               \
  if (child->type == XML_ELEMENT_NODE)

#define xmlFindAll(CHILD, XML, TYPE)                                           \
  FOR_EACH_LIST(xmlNode, CHILD, (XML)->children)                               \
  if (CHILD->type == XML_ELEMENT_NODE &&                                       \
      !xmlStrcmp(CHILD->name, TYPE))

#define xmlFindAllText(CHILD, CONTENT, XML, TYPE)                              \
  xmlFindAll(CHILD, XML, TYPE)                                                 \
  xmlWith(xmlChar, CONTENT, xmlNodeGetContent(CHILD), xmlFree)

#define xmlWith(type, var, expr, free)                                         \
  for (type* var = expr; var; free(var), var = NULL)

#define FOR_LOOP(property, max)                                                \
  for (uint32_t property = 0, end = (max); property < end; ++property)

#define MAKELONG(low, high)                                                    \
  ((uint32_t)(((uint16_t)(low)) | ((uint32_t)((uint16_t)(high)) << 16)))

#define SafeDelete(x, free) if ((x)) { free(x); (x) = 0; }
#define SafeFree(x) SafeDelete(x, free)
#define SafeSet(x, value, free) SafeDelete(x, free); (x) = value;
#define ZeroAlloc(size) _zero_alloc(__FILE__, __LINE__, size)
#define SkipSpace(s) while (isspace(*(s))) (s)++;

#define LUA_FUNCTION(NAME, ...) (*NAME)(lua_State *L, ##__VA_ARGS__);
#define LUA_METHOD(NAME, ...) LUA_FUNCTION(NAME, INTERFACE *self, ##__VA_ARGS__);
#define LUA_CONST_METHOD(NAME, ...) LUA_FUNCTION(NAME, INTERFACE const *self, ##__VA_ARGS__);
#define STD_METHOD(NAME, ...) (*NAME)(INTERFACE *self, ##__VA_ARGS__);
#define STD_CONST_METHOD(NAME, ...) (*NAME)(INTERFACE const *self, ##__VA_ARGS__);
#define LUA_INTERFACE(NAME) struct NAME
#define LUA_INIT()
#define XMLSTR(s) ((xmlChar *)(s))

#define FWD_STRUCT(NAME)\
typedef struct NAME NAME##_t, *lp##NAME##_t;\
typedef struct NAME const c##NAME##_t, *lpc##NAME##_t;

enum
{
  type_none,
  type_enum,
  type_bool,
  type_int8,
  type_uint8,
  type_int16,
  type_uint16,
  type_int32,
  type_uint32,
  type_int64,
  type_uint64,
  type_service,
};

INLINE void
_keep(void const* ptr)
{
}

#define Con_Error(FMT, ...) \
fprintf(stderr, "%s: " FMT "\n", __func__, ##__VA_ARGS__)

#define Con_Warning(FMT, ...) \
Con_Error("Warning: " FMT, ##__VA_ARGS__)

#endif
