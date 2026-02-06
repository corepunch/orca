#include <include/api.h>
#include <include/orca.h>

#include "core_local.h"

struct style_class
{
  struct style_class* next;
  shortStr_t value;
  byte_t flags;
  byte_t opacity;
};

struct style_sheet
{
  struct style_sheet* next;
  uint32_t class_id;
  uint32_t prop_id;
  uint32_t flags;
  shortStr_t classname;
  shortStr_t name;
  shortStr_t value;
};

struct _ADDCLASSSTRUCT
{
  lpcString_t name;
  uint32_t id;
  uint32_t flags;
  lpObject_t obj;
};

void
OBJ_AddStyleClass(lpObject_t obj,
                  lpcString_t name,
                  lpcString_t property,
                  lpcString_t value,
                  uint32_t flags)
{
  struct style_sheet* ss = ZeroAlloc(sizeof(struct style_sheet));
  strncpy(ss->classname, name, sizeof(ss->classname));
  strncpy(ss->value, value, sizeof(ss->value));
  strncpy(ss->name, property, sizeof(ss->name));
  ss->class_id = fnv1a32(name);
  ss->prop_id = fnv1a32(property);
  ss->flags = flags;
  ss->next = NULL;
  ADD_TO_LIST_END(struct style_sheet, ss, _GetStyles(obj));
}

static void
_AddStyleClass(lpObject_t obj, struct style_sheet* other, void* param)
{
  struct _ADDCLASSSTRUCT* inp = param;
  struct style_sheet* ss = ZeroAlloc(sizeof(struct style_sheet));
  memcpy(ss, other, sizeof(struct style_sheet));
  strncpy(ss->classname, inp->name, sizeof(shortStr_t));
  ss->class_id = inp->id;
  ss->flags = inp->flags;
  ss->next = NULL;
  ADD_TO_LIST(ss, _GetStyles(obj));
}

struct style_class*
_ParseClass(lpcString_t str)
{
  struct style_class* cls = ZeroAlloc(sizeof(struct style_class));
  for (lpcString_t s = strchr(str, ':'); s; s = strchr((str = s + 1), ':')) {
    if (!strncmp(str, "hover", s - str))
      cls->flags |= STYLE_HOVER;
    else if (!strncmp(str, "focus", s - str))
      cls->flags |= STYLE_FOCUS;
		else if (!strncmp(str, "active", s - str))
			cls->flags |= STYLE_SELECT;
    else if (!strncmp(str, "dark", s - str))
      cls->flags |= STYLE_DARK;
  }
  lpcString_t szOpacity = strchr(str, '/');
  if (szOpacity) {
    strncpy(cls->value, str, MIN(sizeof(shortStr_t), szOpacity - str));
    cls->opacity = atoi(szOpacity + 1);
  } else {
    strncpy(cls->value, str, sizeof(shortStr_t));
    cls->opacity = 100;
  }
  return cls;
}

void
_AddClass(lpObject_t obj, struct style_class* cls)
{
  ADD_TO_LIST_END(struct style_class, cls, _GetClasses(obj));
  if (cls->flags & STYLE_HOVER) {
    OBJ_SetFlags(obj, OBJ_GetFlags(obj) | OF_HOVERABLE);
  }
}

void
OBJ_ParseClassAttribute(lpObject_t obj, lpcString_t original)
{
  if (!*original)
    return;
  xmlWith(char, classes, strdup(original), free)
  {
    for (lpcString_t s = strtok(classes, " "); s; s = strtok(NULL, " ")) {
      _AddClass(obj, _ParseClass(s));
    }
  }
}

void
OBJ_AddClass(lpObject_t hobj, lpcString_t cls)
{
  _AddClass(hobj, _ParseClass(cls));
}

void OBJAPI(AddStyleSheet, lpcString_t name)
{
  uint32_t classID = *name == '.' ? fnv1a32(name + 1) : 0;
  if (lua_type(L, 3) != LUA_TTABLE) {
    lua_error(L);
    return;
  }
  lua_pushvalue(L, 3);
  lua_pushnil(L);
  while (lua_next(L, -2)) {
    lpcString_t prop = luaL_checkstring(L, -2);
    uint32_t propID = fnv1a32(prop);
    if (propID == CSS_APPLY) {
      LPSTR str = strdup(luaL_checkstring(L, -1));
      for (lpcString_t s = strtok(str, " "); s; s = strtok(NULL, " ")) {
        struct style_class* cls = _ParseClass(s);
        struct _ADDCLASSSTRUCT add_class = {
          .name = name,
          .id = classID,
          .flags = cls->flags,
          .obj = self,
        };
        OBJ_EnumStyleClasses(self, cls->value, _AddStyleClass, &add_class);
        free(cls);
      }
      free(str);
      lua_pop(L, 1);
      continue;
    }
    struct style_sheet* ss = ZeroAlloc(sizeof(struct style_sheet));
    ss->class_id = classID;
    ss->prop_id = propID;
    strncpy(ss->classname, name, sizeof(shortStr_t));
    strncpy(ss->name, prop, sizeof(shortStr_t));
    strtok(ss->classname, ":");
    for (lpcString_t s = strtok(NULL, ":"); s; s = strtok(NULL, ":")) {
      if (!strcmp(s, "hover"))
        ss->flags |= STYLE_HOVER;
      else if (!strcmp(s, "focus"))
        ss->flags |= STYLE_FOCUS;
      else if (!strcmp(s, "active"))
        ss->flags |= STYLE_SELECT;
      else if (!strcmp(s, "dark"))
        ss->flags |= STYLE_DARK;
    }
    switch (lua_type(L, -1)) {
      case LUA_TSTRING:
        strncpy(ss->value, luaL_checkstring(L, -1), sizeof(shortStr_t));
        break;
      case LUA_TNUMBER:
        sprintf(ss->value, "%f", luaL_checknumber(L, -1));
        break;
      case LUA_TBOOLEAN:
        strcpy(ss->value, lua_toboolean(L, -1) ? "true" : "false");
        break;
      default:
        lua_error(L);
        return;
    }
    ss->class_id = fnv1a32(ss->classname + 1);
    ADD_TO_LIST(ss, _GetStyles(self));
//    *pss = ss;
//    pss = &ss->next;
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
}

void
OBJ_EnumStyleClasses(lpObject_t pobj,
                     lpcString_t classname,
                     STYLEPROC Proc,
                     void* param)
{
  uint32_t dwClassID = fnv1a32(classname);
  for (lpObject_t p = pobj; p; p = OBJ_GetParent(p)) {
    FOR_EACH_LIST(struct style_sheet, ss, _GetStyles(p)) {
      if (ss->class_id == dwClassID) {
        if (ss->flags & STYLE_HOVER) {
          OBJ_SetFlags(pobj, OF_HOVERABLE);
        }
        if ((OBJ_GetStyleFlags(pobj)&ss->flags) == ss->flags) {
          Proc(pobj, ss, param);
        }
      }
    }
  }
}

void
OBJ_ApplyClass(lpObject_t pobj, struct style_sheet* ss, void* parm)
{
  lpProperty_t hProperty;
  if (SUCCEEDED(OBJ_FindShortProperty(pobj, ss->name, &hProperty))) {
    if (ss->flags) {
      PROP_SetFlag(hProperty, PF_SPECIALIZED);
    } else if (PROP_GetFlags(hProperty) & PF_SPECIALIZED) {
      // if :hover or :focus was set, ignore default value
      return;
    }
    PROP_Parse(hProperty, ss->value);
    if (PROP_GetType(hProperty) == kDataTypeColor && parm) {
      struct color clr;
      PROP_CopyValue(hProperty, &clr);
      clr.a = ((struct style_class*)parm)->opacity / 100.f;
      PROP_SetValue(hProperty, &clr);
    }
  } else {
    Con_Error("Can't find property %s", ss->name);
  }
}

uint32_t
OBJ_GetStyleFlags(lpObject_t pobj)
{
  uint32_t dwValue = 0;
  if (core_GetHover() == pobj)
    dwValue |= STYLE_HOVER;
  if (core_GetFocus() == pobj)
    dwValue |= STYLE_FOCUS;
	if (OBJ_GetFlags(pobj) & OF_SELECTED)
		dwValue |= STYLE_SELECT;
  if (WI_IsDarkTheme())
    dwValue |= STYLE_DARK;
  return dwValue;
}

void
OBJ_ClearStyleClasses(lpObject_t pobj)
{
  FOR_EACH_LIST(struct style_class, cls, _GetClasses(pobj)) free(cls);
  FOR_EACH_LIST(struct style_sheet, ss, _GetStyles(pobj)) free(ss);
}

void
OBJ_ApplyStyles(lpObject_t object, bool_t recursive)
{
  PROP_ClearSpecialized(OBJ_GetProperties(object));

  if (!OBJ_GetParent(object)) {
    FOR_EACH_LIST(struct style_sheet, ss, _GetStyles(object))
    {
      if (*(uint32_t const*)ss->classname == *(uint32_t const*)"body") {
        OBJ_ApplyClass(object, ss, &(struct style_class){ .opacity = 100 });
      }
    }
  }

  FOR_EACH_LIST(struct style_class, cls, _GetClasses(object))
  {
    if ((OBJ_GetStyleFlags(object) & cls->flags) == cls->flags) {
      OBJ_EnumStyleClasses(object, cls->value, OBJ_ApplyClass, cls);
    }
  }
	
	if (recursive) {
		FOR_EACH_CHILD(object, OBJ_ApplyStyles, recursive);
	}
}
