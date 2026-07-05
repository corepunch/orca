#include "object_internal.h"

struct Object *
OBJ_AddChild(struct Object *self, struct Object *child)
{
  if (child->parent == self) {
    REMOVE_FROM_LIST(struct Object, child, self->children);
    REMOVE_FROM_LIST(struct Object, child, self);
  } else if (child->parent) {
    REMOVE_FROM_LIST(struct Object, child, child->parent->children);
    REMOVE_FROM_LIST(struct Object, child, child->parent);
    OBJ_ReleaseRef(child);
  }
  OBJ_AddRef(child);
  ADD_TO_LIST_END(struct Object, child, self->children);
  child->parent = self;
  OBJ_SetDirty(self);
  struct Object_AttachedEventArgs attached_args = { .Sender = child };
  OBJ_SendMessageW(child, ID_Object_Start, 0, NULL);
  OBJ_SendMessageW(child, ID_Object_Attached, 0, &attached_args);
  _SendMessage(child, StyleController, ThemeChanged, .recursive = TRUE);
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
  } else {
    return NULL;
  }
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
      // Try as an object-typed property — read the component pointer and follow it
      struct Property *prop = OBJ_FindLongProperty(object, identifier);
      if (prop && PROP_GetDesc(prop)->DataType == kDataTypeObject) {
        void const *val = PROP_GetValue(prop);
        if (val) {
          struct component *cmp = NULL;
          memcpy(&cmp, val, sizeof(cmp));
          if (cmp) {
            struct Object *target = CMP_GetObject(cmp);
            if (target) {
              return OBJ_FindPropertyByPath(target, ++split);
            }
          }
        }
      }
      return NULL;
    }
  } else {
    return OBJ_FindLongProperty(object, fnv1a32(path));
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
OBJ_MoveToFront(struct Object *object)
{
  if (object->parent) {
    REMOVE_FROM_LIST(struct Object, object, object->parent->children);
    if (!object->parent->children) {
      ADD_TO_LIST(object, object->parent->children);
    } else {
      struct Object *it = object->parent->children;
      while (it->next) it = it->next;
      it->next = object;
    }
  }
}
