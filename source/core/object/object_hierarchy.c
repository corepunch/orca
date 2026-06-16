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
      return child ? OBJ_FindPropertyByPath(child, ++split) : NULL;
    }
  } else {
    struct Property *p = OBJ_FindLongProperty(object, fnv1a32(path));
    if (p) {
      return p;
    } else if (!strcmp(path, "Material.BlendIntensity")||
               !strcmp(path, "_BlendIntensity"))
    { // HACK: a workaround or expected behaviour?
      return OBJ_FindLongProperty(object, ID_Node_Opacity);
    } else {
      return p;
    }
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
