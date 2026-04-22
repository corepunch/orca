#include "object_internal.h"

struct component*
OBJ_AddComponent(lpObject_t pobj, uint32_t class_id)
{
  lpcClassDesc_t cls = OBJ_FindClassW(class_id);
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

lpObject_t
CMP_GetOwner(struct component* hcmp)
{
  return hcmp->pobj;
}

lpProperty_t
_CreateClassProperty(lpObject_t object, uint32_t ident)
{
  FOR_EACH_LIST(struct component, cmp, object->components)
  {
    FOR_LOOP(i, cmp->pcls->NumProperties)
    {
      lpcPropertyType_t pdesc = &cmp->pcls->Properties[i];
      if (pdesc->ShortIdentifier == ident ||
          pdesc->FullIdentifier == ident)
      {
        return OBJ_AddComponentProperty(NULL, cmp, pdesc);
      }
    }
  }
  return NULL;
}

lpcPropertyType_t
OBJ_FindImplicitProperty(lpObject_t object, lpcString_t name)
{
  uint32_t identifier = fnv1a32(name);
//  return PROP_FindByShortID(object->properties, identifier);
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      lpcPropertyType_t pdesc = &cmp->pcls->Properties[i];
      if (pdesc->ShortIdentifier == identifier) {
        return (lpPropertyType_t)pdesc;
      }
    }
  }
  return NULL;
}

lpcPropertyType_t
OBJ_FindExplicitProperty(lpObject_t object, lpcString_t name)
{
  uint32_t identifier = fnv1a32(name);
  //  return PROP_FindByShortID(object->properties, identifier);
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      lpcPropertyType_t pdesc = &cmp->pcls->Properties[i];
      if (pdesc->FullIdentifier == identifier) {
        return (lpPropertyType_t)pdesc;
      }
    }
  }
  return OBJ_FindPropertyType(identifier);
}

void
OBJ_EnumClassProperties(lpObject_t object,
                        void (*fnProc)(lpcObject_t,
                                       lpcPropertyType_t,
                                       lpcClassDesc_t cdesc,
                                       void const*,
                                       void*),
                        void* parm)
{
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      lpcPropertyType_t pdesc = &cmp->pcls->Properties[i];
      fnProc(object, pdesc, cmp->pcls, cmp->pUserData+pdesc->Offset, parm);
    }
  }
}

bool_t
CMP_SetProperty(struct component* comp, lpProperty_t property)
{
  FOR_LOOP(index, comp->pcls->NumProperties)
  {
    lpcPropertyType_t pdesc = comp->pcls->Properties + index;
    if (PROP_GetLongIdentifier(property) == pdesc->FullIdentifier) {
      lpProperty_t* properties = (void*)(comp->pUserData + comp->pcls->ClassSize);
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
OBJ_SendMessageW(lpObject_t pobj, uint32_t MsgID, wParam_t wParam, lParam_t lParam)
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
OBJ_SendMessage(lpObject_t pobj, lpcString_t Msg, wParam_t wParam, lParam_t lParam)
{
  return OBJ_SendMessageW(pobj, fnv1a32(Msg), wParam, lParam);
}

void
OBJ_RequestAnimate(lpObject_t pobj)
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
OBJ_GetComponent(lpObject_t pobj, uint32_t id)
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
OBJ_ReleaseComponents(lpObject_t pobj)
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
  lpObject_t pobj = self->pobj;
  REMOVE_FROM_LIST(struct component, self, pobj->components);
  free(self);
}

lpObject_t
CMP_GetObject(void const* udata)
{
  if (udata == NULL) {
    return NULL;
  }
  struct component const *cmp = udata - offsetof(struct component, pUserData);
  return cmp->pobj;
}

void
OBJ_EnumObjectClasses(lpObject_t pobj,
                      void (*fnProc)(lpcClassDesc_t, void*),
                      void* param)
{
  FOR_EACH_LIST(struct component, cmp, pobj->components) {
    fnProc(cmp->pcls, param);
  }
}

void _pushproperty(lua_State*, void *, lpcPropertyType_t);

bool_t
OBJ_PushClassProperty(lua_State *L, lpObject_t object, uint32_t id)
{
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      lpcPropertyType_t pdesc = &cmp->pcls->Properties[i];
      if (pdesc->ShortIdentifier == id) {
        _pushproperty(L, cmp->pUserData + pdesc->Offset, pdesc);
        return TRUE;
      }
    }
  }
  return FALSE;
}


lpcPropertyType_t
MSG_FindByShortID(lpObject_t obj, uint32_t ident)
{
  FOR_EACH_LIST(struct component, cmp, obj->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      lpcPropertyType_t msg = &cmp->pcls->Properties[i];
      if (msg->DataType == kDataTypeEvent && msg->ShortIdentifier == ident) {
        return msg;
      }
    }
  }
  return 0;
}

bool_t
OBJ_IsPrefabView(lpcObject_t object)
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

static void
_OBJ_LoadPrefabsRecursive(lpObject_t object)
{
  if (!object) return;

  if (OBJ_IsPrefabView(object)) {
    _SendMessage(object, Node, LoadView, .lua_state = OBJ_GetDomain(object));
  }

  FOR_EACH_OBJECT(child, object) {
    _OBJ_LoadPrefabsRecursive(child);
  }
}

void
OBJ_LoadPrefabs(lpObject_t object)
{
  _OBJ_LoadPrefabsRecursive(object);
}
