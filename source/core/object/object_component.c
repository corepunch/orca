#include "object_internal.h"

struct component*
OBJ_AddComponent(struct Object *pobj, uint32_t class_id)
{
  struct ClassDesc const *cls = OBJ_FindClassW(class_id);
  if (!cls) {
    Con_Error("Class ID 0x%08x not found\n", class_id);
    return NULL;
  }
  uint32_t clsSize = sizeof(struct component) + cls->ClassSize;
  uint32_t propsSize = cls->NumProperties * sizeof(void*);
  struct component* comp = ZeroAlloc(clsSize + propsSize);

  assert(comp);

  if (cls->Defaults) {
    memcpy(comp->pUserData, cls->Defaults, cls->ClassSize);
  }

  comp->pcls = cls;
  comp->pobj = pobj;
  
  ADD_TO_LIST_END(struct component, comp, pobj->components);

  for (uint32_t const* p = cls->ParentClasses; *p; OBJ_AddComponent(pobj, *(p++)));

  return comp;
}

struct Object *
CMP_GetOwner(struct component* hcmp)
{
  return hcmp->pobj;
}

struct Property *
_CreateClassProperty(struct Object *object, uint32_t ident)
{
  FOR_EACH_LIST(struct component, cmp, object->components)
  {
    FOR_LOOP(i, cmp->pcls->NumProperties)
    {
      struct PropertyType const *pdesc = &cmp->pcls->Properties[i];
      if (pdesc->ShortIdentifier == ident ||
          pdesc->FullIdentifier == ident)
      {
        return OBJ_AddComponentProperty(NULL, cmp, pdesc);
      }
    }
  }
  return NULL;
}

struct PropertyType const *
OBJ_FindImplicitProperty(struct Object *object, lpcString_t name)
{
  uint32_t identifier = fnv1a32(name);
//  return PROP_FindByShortID(object->properties, identifier);
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      struct PropertyType const *pdesc = &cmp->pcls->Properties[i];
      if (pdesc->ShortIdentifier == identifier) {
        return (struct PropertyType const *)pdesc;
      }
    }
  }
  return NULL;
}

struct PropertyType const *
OBJ_FindExplicitProperty(struct Object *object, lpcString_t name)
{
  uint32_t identifier = fnv1a32(name);
  //  return PROP_FindByShortID(object->properties, identifier);
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      struct PropertyType const *pdesc = &cmp->pcls->Properties[i];
      if (pdesc->FullIdentifier == identifier) {
        return (struct PropertyType const *)pdesc;
      }
    }
  }
  return OBJ_FindPropertyType(identifier);
}

void
OBJ_EnumClassProperties(struct Object *object,
                        void (*fnProc)(struct Object const *,
                                       struct PropertyType const *,
                                       struct ClassDesc const *cdesc,
                                       void const*,
                                       void*),
                        void* parm)
{
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      struct PropertyType const *pdesc = &cmp->pcls->Properties[i];
      fnProc(object, pdesc, cmp->pcls, cmp->pUserData+pdesc->Offset, parm);
    }
  }
}

bool_t
CMP_SetProperty(struct component* comp, struct Property *property)
{
  FOR_LOOP(index, comp->pcls->NumProperties)
  {
    struct PropertyType const *pdesc = comp->pcls->Properties + index;
    if (PROP_GetLongIdentifier(property) == pdesc->FullIdentifier) {
      struct Property ** properties = (void*)(comp->pUserData + comp->pcls->ClassSize);
      properties[index] = property;
      PROP_SetFlag(property, PF_PROPERTY_TYPE);
      PROP_SetValuePtr(property, comp->pUserData + pdesc->Offset);
      return TRUE;
    }
  }
  return comp->next && CMP_SetProperty(comp->next, property);
}

lpcString_t
CMP_GetClassName(struct component* hcmp)
{
  return hcmp->pcls->ClassName;
}

LRESULT
OBJ_SendMessageW(struct Object *pobj, uint32_t MsgID, wParam_t wParam, lParam_t lParam)
{
  if (MsgID == ID_Object_Animate) {
    OBJ_SetFlags(pobj, OBJ_GetFlags(pobj) & ~OF_ANIMATE_QUEUED);
  }
//#ifndef KANZI_SUPPORT
//	if (MsgID == kMsgUpdateLayout && !(OBJ_GetFlags(pobj) & OF_DIRTY))
//		return FALSE;
//#endif
  FOR_EACH_LIST(struct component, cmp, pobj->components)
  {
    if (cmp->pcls->ObjProc) {
      LRESULT res = cmp->pcls->ObjProc(pobj, cmp->pUserData, MsgID, wParam, lParam);
      if (res) {
        return res;
      }
    }
  }
  return FALSE;
}

LRESULT
OBJ_SendMessage(struct Object *pobj, lpcString_t Msg, wParam_t wParam, lParam_t lParam)
{
  return OBJ_SendMessageW(pobj, fnv1a32(Msg), wParam, lParam);
}

void
OBJ_RequestAnimate(struct Object *pobj)
{
  if (!pobj) return;
  uint32_t flags = OBJ_GetFlags(pobj);
  if (flags & OF_ANIMATE_QUEUED) return;
  OBJ_SetFlags(pobj, flags | OF_ANIMATE_QUEUED);
  axPostMessageW(pobj, ID_Object_Animate, 0, NULL);
}

void*
CMP_GetUserData(struct component* component)
{
  return component->pUserData;
}

void*
OBJ_GetComponent(struct Object *pobj, uint32_t id)
{
  FOR_EACH_LIST(struct component, cmp, pobj->components)
  {
    if (cmp->pcls->ClassID == id) {
      return cmp->pUserData;
    }
  }
  return NULL;
}

void
OBJ_ReleaseComponents(struct Object *pobj)
{
  FOR_EACH_LIST(struct component, cmp, pobj->components)
  {
    free(cmp);
  }
}

void
CMP_Detach(void* userdata)
{
  if (!userdata) return;
  // pUserData is a flexible array member at the end of struct component.
  // Subtract its offset to recover the containing struct component pointer.
  struct component *self = (struct component*)((char*)userdata - offsetof(struct component, pUserData));
  struct Object *pobj = self->pobj;
  REMOVE_FROM_LIST(struct component, self, pobj->components);
  free(self);
}

struct Object *
CMP_GetObject(void const* udata)
{
  if (udata == NULL) {
    return NULL;
  }
  struct component const *cmp = udata - offsetof(struct component, pUserData);
  return cmp->pobj;
}

void
OBJ_EnumObjectClasses(struct Object *pobj,
                      void (*fnProc)(struct ClassDesc const *, void*),
                      void* param)
{
  FOR_EACH_LIST(struct component, cmp, pobj->components) {
    fnProc(cmp->pcls, param);
  }
}

extern int write_property(lua_State *L, struct PropertyType const* prop, void const* valueptr);

bool_t
OBJ_PushClassProperty(lua_State *L, struct Object *object, uint32_t id)
{
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      struct PropertyType const *pdesc = &cmp->pcls->Properties[i];
      if (pdesc->ShortIdentifier == id) {
        write_property(L, pdesc, cmp->pUserData + pdesc->Offset);
        return TRUE;
      }
    }
  }
  return FALSE;
}


struct PropertyType const *
MSG_FindByShortID(struct Object *obj, uint32_t ident)
{
  FOR_EACH_LIST(struct component, cmp, obj->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      struct PropertyType const *msg = &cmp->pcls->Properties[i];
      if (msg->DataType == kDataTypeEvent && msg->ShortIdentifier == ident) {
        return msg;
      }
    }
  }
  return 0;
}

bool_t
OBJ_IsPrefabView(struct Object const *object)
{
  FOR_EACH_LIST(struct component, cmp, object ? object->components : NULL) {
    switch (cmp->pcls->ClassID) {
      case 0xe741d328: /* ID_PrefabView2D */
      case 0x8d451711: /* ID_PrefabView3D */
        return TRUE;
    }
  }
  return FALSE;
}
