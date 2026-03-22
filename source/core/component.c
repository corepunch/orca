#include "core_local.h"

struct component
{
  struct component* next;
  lpcClassDesc_t pcls;
  lpObject_t pobj;
  char pUserData[];
};

void print_debug_component(lpObject_t pobj, int line) {
  fprintf(stderr, "%d %s\n", line, _GetComponents(pobj)->pcls->ClassName);
}

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
  
  ADD_TO_LIST_END(struct component, comp, _GetComponents(pobj));

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
  FOR_EACH_LIST(struct component, cmp, _GetComponents(object))
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
  FOR_EACH_LIST(struct component, cmp, _GetComponents(object)) {
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
  FOR_EACH_LIST(struct component, cmp, _GetComponents(object)) {
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
  FOR_EACH_LIST(struct component, cmp, _GetComponents(object)) {
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
//#ifndef KANZI_SUPPORT
//	if (MsgID == kEventUpdateLayout && !(OBJ_GetFlags(pobj) & OF_DIRTY))
//		return FALSE;
//#endif
  FOR_EACH_LIST(struct component, cmp, _GetComponents(pobj))
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

//lpProperty_t
//OBJ_AddComponentProperty2(lua_State* L, struct component* pcmp, lpcString_t name)
//{
//  uint32_t dwIdentifier = fnv1a32(name);
//  FOR_LOOP(n, pcmp->pcls->NumProperties)
//  {
//    lpcPropertyType_t desc = &pcmp->pcls->Properties[n];
//    if (desc->short_identifier == dwIdentifier) {
//      return OBJ_AddComponentProperty(L, pcmp, desc);
//    }
//  }
//  return NULL;
//}

void*
CMP_GetUserData(struct component* component)
{
  return component->pUserData;
}

void*
OBJ_GetComponent(lpObject_t pobj, uint32_t id)
{
  FOR_EACH_LIST(struct component, cmp, _GetComponents(pobj))
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
  FOR_EACH_LIST(struct component, cmp, _GetComponents(pobj))
  {
    free(cmp);
  }
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
  FOR_EACH_LIST(struct component, cmp, _GetComponents(pobj)) {
    fnProc(cmp->pcls, param);
  }
}

void _pushproperty(lua_State*, void *, lpcPropertyType_t);

bool_t
OBJ_PushClassProperty(lua_State *L, lpObject_t object, uint32_t id)
{
  FOR_EACH_LIST(struct component, cmp, _GetComponents(object)) {
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


#include <source/editor/ed_stab_component.h>
