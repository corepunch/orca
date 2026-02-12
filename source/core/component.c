#include "core_local.h"

struct component
{
  struct component* next;
  lpcClassDesc_t pcls;
  lpObject_t pobj;
  char pUserData[];
};

struct component*
CMP_Create(lpObject_t pobj, lpcClassDesc_t cls)
{
  uint32_t clsSize = sizeof(struct component) + cls->ClassSize;
  uint32_t propsSize = cls->NumProperties * sizeof(void*);
  struct component* comp = ZeroAlloc(clsSize + propsSize);

  if (!comp)
    return NULL;

  if (cls->Defaults) {
    memcpy(comp->pUserData, cls->Defaults, cls->ClassSize);
  }
  comp->pcls = cls;
  comp->pobj = pobj;

  for (lpcClassDesc_t const* p = cls->ParentClasses; *p; p++) {
    struct component *c = CMP_Create(pobj, *p), *b = c;
    for (; b->next; b = b->next)
      ;
    b->next = comp->next;
    comp->next = c;
  }

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
      lpcPropertyDesc_t pdesc = &cmp->pcls->Properties[i];
      if (pdesc->id->Identifier == ident ||
          pdesc->FullIdentifier == ident)
      {
        return CMP_CreateProperty(NULL, cmp, pdesc);
      }
    }
  }
  return NULL;
}

void
OBJ_EnumClassProperties(lpObject_t object,
                        void (*fnProc)(lpcObject_t,
                                       lpcPropertyDesc_t,
                                       lpcClassDesc_t cdesc,
                                       void const*,
                                       void*),
                        void* parm)
{
  FOR_EACH_LIST(struct component, cmp, _GetComponents(object))
  {
    FOR_LOOP(i, cmp->pcls->NumProperties)
    {
      lpcPropertyDesc_t pdesc = &cmp->pcls->Properties[i];
      if (pdesc->DataType == kDataTypeMatrix3D)
        continue;
      fnProc(object, pdesc, cmp->pcls, cmp->pUserData+pdesc->Offset, parm);
      if (pdesc->DataType == kDataTypeGroup) {
        i += pdesc->NumComponents;
      }
    }
  }
}

bool_t
CMP_SetProperty(struct component* comp, lpProperty_t property)
{
  FOR_LOOP(index, comp->pcls->NumProperties)
  {
    lpcPropertyDesc_t pdesc = comp->pcls->Properties + index;
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

struct component*
OBJ_AddComponent(lpObject_t pobj, lpcClassDesc_t cls)
{
  struct component* comp = CMP_Create(pobj, cls);
  if (comp) {
    ADD_TO_LIST_END(struct component, comp, _GetComponents(pobj));
    return comp;
  } else {
    return NULL;
  }
}

//lpProperty_t
//CMP_CreateProperty2(lua_State* L, struct component* pcmp, lpcString_t name)
//{
//  uint32_t dwIdentifier = fnv1a32(name);
//  FOR_LOOP(n, pcmp->pcls->NumProperties)
//  {
//    lpcPropertyDesc_t desc = &pcmp->pcls->Properties[n];
//    if (desc->short_identifier == dwIdentifier) {
//      return CMP_CreateProperty(L, pcmp, desc);
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

void _pushproperty(lua_State*, eDataType_t, void *, lpcString_t, size_t);

bool_t
OBJ_PushClassProperty(lua_State *L, lpObject_t object, uint32_t id)
{
  FOR_EACH_LIST(struct component, cmp, _GetComponents(object)) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      lpcPropertyDesc_t pdesc = &cmp->pcls->Properties[i];
      if (pdesc->id->Identifier == id) {
        _pushproperty(L, pdesc->DataType, cmp->pUserData + pdesc->Offset, pdesc->TypeString, pdesc->DataSize);
        return TRUE;
      }
    }
  }
  return FALSE;
}


#include <source/editor/ed_stab_component.h>
