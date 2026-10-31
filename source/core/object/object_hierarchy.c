#include "object_internal.h"

struct Object *
OBJ_AddChild(struct Object *self, struct Object *child)
{
  if (!self || !child) return NULL;
  for (struct Object *it = self; it; it = it->parent)
    if (it == child) return NULL;
  if (child->parent == self) {
    OBJ_MoveBefore(child, NULL);
    return child;
  }
  PROP_InvalidateBindingPaths();
  struct Object *oldParent = child->parent;
  // Acquire the new hierarchy reference before releasing the old parent's.
  OBJ_AddRef(child);
  if (oldParent) {
    REMOVE_FROM_LIST(struct Object, child, oldParent->children);
    child->parent = NULL;
    OBJ_ReleaseRef(child);
    OBJ_SetDirty(oldParent);
  }
  ADD_TO_LIST_END(struct Object, child, self->children);
  child->parent = self;
  OBJ_SetDirty(self);
  struct Object_AttachedEventArgs attached_args = { .Sender = child };
  OBJ_SendMessageW(child, ID_Object_Start, 0, NULL);
  OBJ_SendMessageW(child, ID_Object_Attached, 0, &attached_args);
  _SendMessage(child, StyleController, ThemeChanged, .recursive = TRUE);
  if (oldParent) OBJ_NotifyCollectionChanged(oldParent);
  OBJ_NotifyCollectionChanged(self);
  return child;
}

struct Object *
OBJ_FindImmediateChild(struct Object *object, uint32_t identifier)
{
  FOR_EACH_LIST(struct Object, child, object->children)
  {
    if (child->identifier == identifier) {
      return child;
    }
  }
  return NULL;
}

struct Object *
OBJ_FindByPath(struct Object *object, lpcString_t szObjectPath)
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
  struct Object *pChild = OBJ_FindChildByID(object, dwID);
  if (pChild) {
    return *szSplit ? OBJ_FindByPath(pChild, ++szSplit) : pChild;
  }
  struct Property *property = memchr(szObjectPath, '.', szSplit - szObjectPath)
    ? OBJ_FindLongProperty(object, dwID) : OBJ_FindShortProperty(object, dwID);
  if (!property || PROP_GetDesc(property)->DataType != kDataTypeObject) return NULL;
  PROP_Update(property);
  void const *value = PROP_GetValue(property);
  struct Object *target = value ? *(struct Object *const *)value : NULL;
  return target && *szSplit ? OBJ_FindByPath(target, szSplit + 1) : target;
}

struct Property *
OBJ_FindPropertyByPath(struct Object *object, lpcString_t path)
{
  if (!object) return NULL;
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
      struct Object *child = OBJ_FindImmediateChild(object, identifier);
      if (child) {
        return OBJ_FindPropertyByPath(child, ++split);
      }
      // Object values are normalized by the property API; do not reinterpret
      // their storage as component userdata.
      struct Property *prop = memchr(path, '.', split - path)
        ? OBJ_FindLongProperty(object, identifier) : OBJ_FindShortProperty(object, identifier);
      if (prop && PROP_GetDesc(prop)->DataType == kDataTypeObject) {
        struct Object *target = PROP_GetObjectValue(prop);
        if (target) return OBJ_FindPropertyByPath(target, ++split);
      }
      return NULL;
    }
  } else {
    return strchr(path, '.') ? OBJ_FindLongProperty(object, fnv1a32(path))
                             : OBJ_FindShortProperty(object, fnv1a32(path));
  }
}

struct Object *
OBJ_FindChild(struct Object *object,
              lpcString_t name,
              bool_t recursive)
{
  FOR_EACH_LIST(struct Object, child, object->children) {
    if (child->Name && !strcmp(child->Name, name)) {
      return child;
    }
    struct Object *found;
    if (recursive && (found = OBJ_FindChild(child, name, recursive))) {
      return found;
    }
  }
  return NULL;
}

struct Object *OBJ_FindChildByID(struct Object *object, uint32_t id) {
  FOR_EACH_LIST(struct Object, pChild, object->children)
  {
    if (pChild->identifier == id)
      return pChild;
  }
  return NULL;
}

struct Object *
OBJ_FindChildOfClass(struct Object *self, uint32_t comp_id)
{
  FOR_EACH_OBJECT(it, self) {
    if (OBJ_GetComponent(it, comp_id)) {
      return it;
    }
  }
  return NULL;
}

struct Object *
OBJ_FindParentOfClass(struct Object *self, uint32_t comp_id)
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
OBJ_MoveBefore(struct Object *self, struct Object *sibling)
{
  if (!self || !self->parent || sibling == self || self->next == sibling) return;
  struct Object *parent = self->parent;
  if (sibling && sibling->parent != parent) return;
  PROP_InvalidateBindingPaths();
  REMOVE_FROM_LIST(struct Object, self, parent->children);
  struct Object **slot = &parent->children;
  while (*slot && *slot != sibling) slot = &(*slot)->next;
  self->next = *slot;
  *slot = self;
  OBJ_SetDirty(parent);
  OBJ_NotifyCollectionChanged(parent);
}

void OBJ_MoveToFront(struct Object *object) { OBJ_MoveBefore(object, NULL); }

void
OBJ_MoveBeforeFromLua(lua_State *L, struct Object *self)
{
  OBJ_MoveBefore(self, lua_isnoneornil(L, 2) ? NULL : luaX_checkObject(L, 2));
}
