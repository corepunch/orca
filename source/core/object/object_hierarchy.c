#include "object_internal.h"

#include <plugins/UIKit/UIKit.h>

lpObject_t OBJ_AddChild(lpObject_t self, lpObject_t child, bool_t is_template)
{
  if (child->parent) {
    REMOVE_FROM_LIST(struct Object, child, child->parent->children);
  }
  ADD_TO_LIST_END(struct Object, child, self->children);
  child->parent = self;
  OBJ_SendMessageW(child, ID_Attached, 0, self);
  if (is_template) {
    OBJ_SetFlags(self, OBJ_GetFlags(self) | OF_TEMPLATE);
  }
  return child;
}

void
OBJ_LoadPrefabs(lua_State* L, lpObject_t object)
{
  if (!OBJ_IsHidden(object)) {
    OBJ_SendMessage(object, "LoadView", 0, &L);
  }
  FOR_EACH_OBJECT(child, object) {
    OBJ_LoadPrefabs(L, child);
  }
}

lpObject_t
OBJ_FindImmediateChild(lpObject_t object, uint32_t identifier)
{
  FOR_EACH_LIST(struct Object, child, object->children)
  {
    if (child->identifier == identifier) {
      return child;
    }
  }
  return NULL;
}

lpObject_t
OBJ_FindByPath(lpObject_t object, lpcString_t szObjectPath)
{
  if (*szObjectPath == '\0')
    return object;
  lpcString_t szSplit = szObjectPath;
  for (; *szSplit && *szSplit != '/'; szSplit++)
    ;
  if (!strncmp(szObjectPath, ".", szSplit - szObjectPath)) {
    return *szSplit ? OBJ_FindByPath(object, ++szSplit) : object;
  }
  if (!strncmp(szObjectPath, "..", szSplit - szObjectPath)) {
    if (object->parent) {
      return *szSplit ? OBJ_FindByPath(object->parent, ++szSplit) : object;
    } else {
      return NULL;
    }
  }
  int const dwID = fnv1a32_range(szObjectPath, szSplit);
  lpObject_t pChild = OBJ_FindChildByID(object, dwID);
  if (pChild) {
    return *szSplit ? OBJ_FindByPath(pChild, ++szSplit) : pChild;
  } else {
    return NULL;
  }
}

lpProperty_t
OBJ_FindPropertyByPath(lpObject_t object, lpcString_t path)
{
  lpcString_t split = path;
  for (; *split && *split != '/'; split++)
    ;
  if (!strncmp(path, ".", split - path)) {
    return *split ? OBJ_FindPropertyByPath(object, ++split) : NULL;
  }
  if (!strncmp(path, "..", split - path)) {
    if (object->parent) {
      return *split ? OBJ_FindPropertyByPath(object->parent, ++split) : NULL;
    } else {
      return NULL;
    }
  }
  if (*split) {
    uint32_t identifier = fnv1a32_range(path, split);
    if (object->identifier == identifier) {
      return OBJ_FindPropertyByPath(object, ++split);
    } else {
      lpObject_t child = OBJ_FindImmediateChild(object, identifier);
      return child ? OBJ_FindPropertyByPath(child, ++split) : NULL;
    }
  } else {
    lpProperty_t plist = OBJ_GetProperties(object);
//    lpProperty_t p = PROP_FindByFullName(plist, path);
    lpProperty_t p;
    if (SUCCEEDED(OBJ_FindLongProperty(object, fnv1a32(path), &p))) {
      return p;
    } else if (!strcmp(path, "Material.BlendIntensity")||
               !strcmp(path, "_BlendIntensity"))
    { // HACK: a workaround or expected behaviour?
      return PROP_FindByFullName(plist, "Node.Opacity");
    } else {
      return p;
    }
  }
}

lpObject_t
OBJ_FindChild(lpObject_t object,
              lpcString_t name,
              bool_t recursive)
{
  FOR_EACH_LIST(struct Object, child, object->children) {
    if (child->Name && !strcmp(child->Name, name)) {
      return child;
    }
    lpObject_t found;
    if (recursive && (found = OBJ_FindChild(child, name, recursive))) {
      return found;
    }
  }
  return NULL;
}

lpObject_t OBJ_FindChildByID(lpObject_t object, uint32_t id) {
  FOR_EACH_LIST(struct Object, pChild, object->children)
  {
    if (pChild->identifier == id)
      return pChild;
  }
  return NULL;
}

lpObject_t
OBJ_FindChildOfClass(lpObject_t self, uint32_t comp_id)
{
  FOR_EACH_OBJECT(it, self) {
    if (OBJ_GetComponent(it, comp_id)) {
      return it;
    }
  }
  return NULL;
}

lpObject_t
OBJ_FindParentOfClass(lpObject_t self, uint32_t comp_id)
{
  if (OBJ_GetComponent(self, comp_id)) {
    return self;
  } else if ((self = OBJ_GetParent(self))) {
    return OBJ_FindParentOfClass(self, comp_id);
  } else {
    return NULL;
  }
}

void
OBJ_MoveToFront(lpObject_t object)
{
  if (object->parent) {
    REMOVE_FROM_LIST(struct Object, object, object->parent->children);
    if (!object->parent->children) {
      ADD_TO_LIST(object, object->parent->children);
    } else {
      lpObject_t it = object->parent->children;
      while (it->next) it = it->next;
      it->next = object;
    }
  }
}

lpObject_t
OBJ_GetModal(lpcObject_t self)
{
  if (OBJ_GetComponent((lpObject_t)self, ID_Screen)) {
    return self->next;
  } else {
    return NULL;
  }
}

static int modal_continue(lua_State *L, int status, lua_KContext ctx)
{
  struct Screen* modal = GetScreen((lpObject_t)ctx);
  if (!isnan(modal->DialogResult)) {
    if (modal->DialogResult) {
      lua_pushboolean(L, TRUE);
      lua_pushboolean(L, modal->DialogResult != 2);
    } else {
      lua_pushboolean(L, FALSE);
      lua_pushstring(L, "Cancelled");
    }
    OBJ_RemoveFromParent(L, (lpObject_t)ctx);
    return 2; // resume Lua script after ShowModal()
  } else {
    return lua_yieldk(L, 0, ctx, modal_continue);
  }
}

int
OBJ_ShowModal(lua_State* L, lpObject_t self, lpObject_t modal)
{
  while (OBJ_GetParent(self) && !OBJ_GetComponent(self, ID_Screen)) {
    self = OBJ_GetParent(self);
  }
  if (!self) {
    Con_Error("Could not find Screen for object %s", OBJ_GetName(self));
    return 0;
  }
  if (modal->parent) {
    REMOVE_FROM_LIST(struct Object, modal, modal->parent->children);
  }
  lpObject_t *next = &self->next;
  while (*next) next = &(*next)->next;
  *next = modal;
  modal->parent = self;
  modal->flags |= OF_NOACTIVATE;
  return lua_yieldk(L, 0, (lua_KContext)modal, modal_continue);
}
