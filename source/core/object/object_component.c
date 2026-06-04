#include "object_internal.h"

extern int parse_property(const char* str,
                          struct PropertyType const* prop,
                          void* valueptr);
extern void read_property(lua_State *L,
                          int idx,
                          struct PropertyType const* prop,
                          void* valueptr);
extern int luaX_readProperty(lua_State* L, int idx, struct Property *p);
extern int write_property(lua_State *L,
                          struct PropertyType const* prop,
                          void const* valueptr);

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

  comp->pcls = cls;
  comp->pobj = pobj;

  if (cls->Defaults) {
    memcpy(comp->pUserData, cls->Defaults, cls->ClassSize);
    /* For UIData objects, also seed the typedata slot from defaults.
     * ClassDesc.TypedataOffset holds offsetof(UIData, ClassName) — set in Phase 4b. */
    if (cls->TypedataOffset != UINT32_MAX) {
      memcpy(pobj->typedata + cls->TypedataOffset, cls->Defaults, cls->ClassSize);
    }
  }

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
  /* For UIData objects, search all registered classes for the property
   * descriptor without needing the component list. */
  if (OBJ_UsesTypedata(object)) {
    extern struct game core;
    FOR_LOOP(ci, MAX_CLASSES) {
      struct ClassDesc const *cls = core.classes[ci];
      if (!cls || cls->TypedataOffset == UINT32_MAX) continue;
      FOR_LOOP(i, cls->NumProperties) {
        struct PropertyType const *pdesc = &cls->Properties[i];
        if (pdesc->ShortIdentifier == ident || pdesc->FullIdentifier == ident) {
          struct Property *p = PROP_Create(NULL, object, pdesc);
          return p;
        }
      }
    }
  }
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
  if (OBJ_UsesTypedata(object)) {
    extern struct game core;
    FOR_LOOP(ci, MAX_CLASSES) {
      struct ClassDesc const *cls = core.classes[ci];
      if (!cls || cls->TypedataOffset == UINT32_MAX) continue;
      FOR_LOOP(i, cls->NumProperties) {
        struct PropertyType const *pdesc = &cls->Properties[i];
        if (pdesc->ShortIdentifier == identifier)
          return pdesc;
      }
    }
  }
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
  if (OBJ_UsesTypedata(object)) {
    extern struct game core;
    FOR_LOOP(ci, MAX_CLASSES) {
      struct ClassDesc const *cls = core.classes[ci];
      if (!cls || cls->TypedataOffset == UINT32_MAX) continue;
      FOR_LOOP(i, cls->NumProperties) {
        struct PropertyType const *pdesc = &cls->Properties[i];
        if (pdesc->FullIdentifier == identifier)
          return pdesc;
      }
    }
  }
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

struct PropertyShorthand const *
OBJ_FindImplicitShorthand(struct Object *object, lpcString_t name)
{
  uint32_t identifier = fnv1a32(name);
  if (OBJ_UsesTypedata(object)) {
    extern struct game core;
    FOR_LOOP(ci, MAX_CLASSES) {
      struct ClassDesc const *cls = core.classes[ci];
      if (!cls || cls->TypedataOffset == UINT32_MAX || !cls->Shorthands) continue;
      FOR_LOOP(i, cls->NumShorthands) {
        struct PropertyShorthand const *sh = &cls->Shorthands[i];
        if (sh->ShortIdentifier == identifier) return sh;
      }
    }
  }
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumShorthands) {
      struct PropertyShorthand const *sh = &cmp->pcls->Shorthands[i];
      if (sh->ShortIdentifier == identifier) {
        return sh;
      }
    }
  }
  return NULL;
}

struct PropertyShorthand const *
OBJ_FindExplicitShorthand(struct Object *object, lpcString_t name)
{
  uint32_t identifier = fnv1a32(name);
  if (OBJ_UsesTypedata(object)) {
    extern struct game core;
    FOR_LOOP(ci, MAX_CLASSES) {
      struct ClassDesc const *cls = core.classes[ci];
      if (!cls || cls->TypedataOffset == UINT32_MAX || !cls->Shorthands) continue;
      FOR_LOOP(i, cls->NumShorthands) {
        struct PropertyShorthand const *sh = &cls->Shorthands[i];
        if (sh->FullIdentifier == identifier) return sh;
      }
    }
  }
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumShorthands) {
      struct PropertyShorthand const *sh = &cmp->pcls->Shorthands[i];
      if (sh->FullIdentifier == identifier) {
        return sh;
      }
    }
  }
  return OBJ_FindPropertyShorthand(identifier);
}

static struct PropertyShorthand const *
find_shorthand(struct Object *object, lpcString_t name)
{
  struct PropertyShorthand const *sh = OBJ_FindImplicitShorthand(object, name);
  return sh ? sh : OBJ_FindExplicitShorthand(object, name);
}

static bool_t
apply_shorthand_struct(struct Object *object,
                       struct PropertyShorthand const *sh,
                       void *value,
                       uint64_t present_mask,
                       bool_t has_present_mask)
{
  bool_t ok = TRUE;
  FOR_LOOP(i, sh->NumTargets) {
    struct PropertyShorthandTarget const *target = &sh->Targets[i];
    if (has_present_mask && target->PresentBit &&
        !(present_mask & target->PresentBit)) {
      continue;
    }
    void *src = (char *)value + target->Offset;
    struct Property *property = NULL;
    if (FAILED(OBJ_FindLongProperty(object, target->PropertyID, &property)) || !property) {
      ok = FALSE;
      continue;
    }
    PROP_SetValue(property, src);
    if (PROP_GetType(property) == kDataTypeString) {
      free(*(char **)src);
      *(char **)src = NULL;
    }
  }
  return ok;
}

bool_t
OBJ_SetShorthandValueFromString(struct Object *object,
                                lpcString_t name,
                                lpcString_t value)
{
  struct PropertyShorthand const *sh = find_shorthand(object, name);
  if (!sh || !value) {
    return FALSE;
  }

  struct PropertyType fake = {
    .Name = sh->Name,
    .Category = sh->Category,
    .ShortIdentifier = sh->ShortIdentifier,
    .FullIdentifier = sh->FullIdentifier,
    .Offset = 0,
    .DataSize = sh->StructSize,
    .DataType = kDataTypeStruct,
    .TypeString = sh->TypeString,
  };
  void *tmp = calloc(1, sh->StructSize);
  if (!tmp) {
    return FALSE;
  }
  bool_t parsed = parse_property(value, &fake, tmp);
  bool_t has_present_mask = FALSE;
  uint64_t present_mask = parsed ? OBJ_GetStructParseMask(&has_present_mask) : 0;
  bool_t ok = parsed &&
              apply_shorthand_struct(object, sh, tmp,
                                     present_mask, has_present_mask);
  free(tmp);
  return ok;
}

bool_t
OBJ_SetShorthandValueFromStruct(struct Object *object,
                                lpcString_t name,
                                void *value)
{
  struct PropertyShorthand const *sh = find_shorthand(object, name);
  if (!sh || !value) {
    return FALSE;
  }
  return apply_shorthand_struct(object, sh, value, 0, FALSE);
}

bool_t
OBJ_SetShorthandValueFromLua(lua_State *L,
                             struct Object *object,
                             lpcString_t name,
                             int value_idx)
{
  struct PropertyShorthand const *sh = find_shorthand(object, name);
  if (!sh) {
    return FALSE;
  }

  value_idx = lua_absindex(L, value_idx);
  if (lua_istable(L, value_idx)) {
    bool_t ok = TRUE;
    FOR_LOOP(i, sh->NumTargets) {
      struct PropertyShorthandTarget const *target = &sh->Targets[i];
      struct Property *property = NULL;
      if (FAILED(OBJ_FindLongProperty(object, target->PropertyID, &property)) || !property) {
        ok = FALSE;
        continue;
      }
      lua_getfield(L, value_idx, target->Name);
      if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_getfield(L, value_idx, PROP_GetName(property));
      }
      if (!lua_isnil(L, -1)) {
        luaX_readProperty(L, -1, property);
      }
      lua_pop(L, 1);
    }
    return ok;
  }
  if (lua_isstring(L, value_idx)) {
    return OBJ_SetShorthandValueFromString(object, name, lua_tostring(L, value_idx));
  }

  struct PropertyType fake = {
    .Name = sh->Name,
    .Category = sh->Category,
    .ShortIdentifier = sh->ShortIdentifier,
    .FullIdentifier = sh->FullIdentifier,
    .Offset = 0,
    .DataSize = sh->StructSize,
    .DataType = kDataTypeStruct,
    .TypeString = sh->TypeString,
  };
  void *tmp = calloc(1, sh->StructSize);
  if (!tmp) {
    return FALSE;
  }
  read_property(L, value_idx, &fake, tmp);
  bool_t ok = apply_shorthand_struct(object, sh, tmp, 0, FALSE);
  free(tmp);
  return ok;
}

bool_t
OBJ_PushShorthandValue(lua_State *L,
                       struct Object *object,
                       lpcString_t name)
{
  struct PropertyShorthand const *sh = find_shorthand(object, name);
  if (!sh) {
    return FALSE;
  }
  void *tmp = calloc(1, sh->StructSize);
  if (!tmp) {
    lua_pushnil(L);
    return TRUE;
  }
  FOR_LOOP(i, sh->NumTargets) {
    struct PropertyShorthandTarget const *target = &sh->Targets[i];
    struct Property *property = NULL;
    if (SUCCEEDED(OBJ_FindLongProperty(object, target->PropertyID, &property)) &&
        property && !PROP_IsNull(property)) {
      memcpy((char *)tmp + target->Offset, PROP_GetValue(property), PROP_GetSize(property));
    }
  }
  struct PropertyType fake = {
    .Name = sh->Name,
    .Category = sh->Category,
    .ShortIdentifier = sh->ShortIdentifier,
    .FullIdentifier = sh->FullIdentifier,
    .Offset = 0,
    .DataSize = sh->StructSize,
    .DataType = kDataTypeStruct,
    .TypeString = sh->TypeString,
  };
  write_property(L, &fake, tmp);
  free(tmp);
  return TRUE;
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
      /* UIKit properties have UIData-relative offsets; others are component-relative. */
      struct Object *obj = comp->pobj;
      if (obj && obj->type && obj->type->TypedataOffset != UINT32_MAX) {
        PROP_SetValuePtr(property, obj->typedata + pdesc->Offset);
      } else {
        PROP_SetValuePtr(property, comp->pUserData + pdesc->Offset);
      }
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

static bool_t
_IsNodeTriggerMessage(uint32_t MsgID)
{
  switch (MsgID) {
    case ID_Node_LeftButtonDown:
    case ID_Node_RightButtonDown:
    case ID_Node_OtherButtonDown:
    case ID_Node_LeftButtonUp:
    case ID_Node_RightButtonUp:
    case ID_Node_OtherButtonUp:
    case ID_Node_LeftButtonDragged:
    case ID_Node_RightButtonDragged:
    case ID_Node_OtherButtonDragged:
    case ID_Node_LeftDoubleClick:
    case ID_Node_RightDoubleClick:
    case ID_Node_OtherDoubleClick:
    case ID_Node_MouseMoved:
    case ID_Node_ScrollWheel:
    case ID_Node_DragDrop:
    case ID_Node_DragEnter:
    case ID_Node_SetFocus:
    case ID_Node_KillFocus:
    case ID_Node_KeyDown:
    case ID_Node_KeyUp:
    case ID_Node_TextInput:
      return TRUE;
    default:
      return FALSE;
  }
}

static lpcString_t
_NodeTriggerMessageName(uint32_t MsgID)
{
  switch (MsgID) {
    case ID_Node_LeftButtonDown: return "Node.LeftButtonDown";
    case ID_Node_RightButtonDown: return "Node.RightButtonDown";
    case ID_Node_OtherButtonDown: return "Node.OtherButtonDown";
    case ID_Node_LeftButtonUp: return "Node.LeftButtonUp";
    case ID_Node_RightButtonUp: return "Node.RightButtonUp";
    case ID_Node_OtherButtonUp: return "Node.OtherButtonUp";
    case ID_Node_LeftButtonDragged: return "Node.LeftButtonDragged";
    case ID_Node_RightButtonDragged: return "Node.RightButtonDragged";
    case ID_Node_OtherButtonDragged: return "Node.OtherButtonDragged";
    case ID_Node_LeftDoubleClick: return "Node.LeftDoubleClick";
    case ID_Node_RightDoubleClick: return "Node.RightDoubleClick";
    case ID_Node_OtherDoubleClick: return "Node.OtherDoubleClick";
    case ID_Node_MouseMoved: return "Node.MouseMoved";
    case ID_Node_ScrollWheel: return "Node.ScrollWheel";
    case ID_Node_DragDrop: return "Node.DragDrop";
    case ID_Node_DragEnter: return "Node.DragEnter";
    case ID_Node_SetFocus: return "Node.SetFocus";
    case ID_Node_KillFocus: return "Node.KillFocus";
    case ID_Node_KeyDown: return "Node.KeyDown";
    case ID_Node_KeyUp: return "Node.KeyUp";
    case ID_Node_TextInput: return "Node.TextInput";
    default:
      return NULL;
  }
}

static struct Object *
_NodeTriggerSender(struct Object *receiver, uint32_t MsgID, wParam_t wParam, lParam_t lParam)
{
  (void)MsgID;
  (void)wParam;
  (void)lParam;
  return receiver;
}

static LRESULT
_FireRoutedTrigger(struct Object *hObject, lpcString_t configured_event, lpcString_t routed_event, struct Object *sender)
{
  if (!configured_event || !*configured_event || strcmp(configured_event, routed_event)) {
    return FALSE;
  }
  return _SendMessage(hObject, Trigger, Triggered,
                .Sender = sender ? sender : hObject);
}

static LRESULT
_DispatchEventTriggerSpecialCase(struct Object *receiver,
                                 uint32_t MsgID,
                                 wParam_t wParam,
                                 lParam_t lParam,
                                 struct Object *sender_override)
{
  lpcString_t routed_event = _NodeTriggerMessageName(MsgID);
  if (!routed_event) {
    return FALSE;
  }
  struct Object *sender = sender_override ? sender_override : _NodeTriggerSender(receiver, MsgID, wParam, lParam);

  struct EventTrigger const *event_trigger = GetEventTrigger(receiver);
  if (event_trigger && _FireRoutedTrigger(receiver, event_trigger->RoutedEvent, routed_event, sender)) {
    return TRUE;
  }
  return FALSE;
}

static LRESULT
_DispatchNodeTriggers(struct Object *node_object, uint32_t MsgID, wParam_t wParam, lParam_t lParam)
{
  // This may be called for any object receiving a Node.* message.
  // Only objects with an attached Node component can host Node.Triggers.
  struct Node *node = GetNode(node_object);
  if (!node || node->NumTriggers <= 0 || !node->Triggers) {
    return FALSE;
  }

  lParam_t trigger_param = lParam;
  struct Object **triggers = (struct Object **)node->Triggers;
  // Trigger arrays may contain NULL holes (e.g. sparse/partially initialized slots).
  FOR_LOOP(i, node->NumTriggers) {
    struct Object *trigger = triggers[i];
    if (!trigger) {
      continue;
    }
    LRESULT handled = _DispatchEventTriggerSpecialCase(trigger,
                                                       MsgID,
                                                       wParam,
                                                       trigger_param,
                                                       node_object);
    if (handled) {
      return handled;
    }
  }
  return FALSE;
}

LRESULT
OBJ_SendMessageW(struct Object *pobj, uint32_t MsgID, wParam_t wParam, lParam_t lParam)
{
  if (MsgID == ID_Object_Animate) {
    OBJ_SetFlags(pobj, OBJ_GetFlags(pobj) & ~OF_ANIMATE_QUEUED);
    OBJ_AnimateProperties(pobj);
  }
//#ifndef KANZI_SUPPORT
//	if (MsgID == kMsgUpdateLayout && !(OBJ_GetFlags(pobj) & OF_DIRTY))
//		return FALSE;
//#endif
  /* Lifecycle messages broadcast to every handler regardless of return value.
   * All other messages use first-handled-wins. */
  bool_t is_broadcast = (MsgID == ID_Object_Create  ||
                         MsgID == ID_Object_Destroy ||
                         MsgID == ID_Object_Release);
  LRESULT broadcast_result = FALSE;

  /* UIData objects: walk the global class registry instead of component list.
   * Classes without a registered TypedataOffset (e.g. Viewport3D from SceneKit)
   * Classes without a TypedataOffset use the component list instead. */
  if (OBJ_UsesTypedata(pobj)) {
    extern struct game core;
    FOR_LOOP(ci, MAX_CLASSES) {
      struct ClassDesc const *cls = core.classes[ci];
      if (!cls || !cls->ObjProc || cls->TypedataOffset == UINT32_MAX) continue;
      void *cmp_data = pobj->typedata + cls->TypedataOffset;
      LRESULT res = cls->ObjProc(pobj, cmp_data, MsgID, wParam, lParam);
      if (is_broadcast) {
        if (res) broadcast_result = res;
      } else if (res) {
        return res;
      }
    }
    if (is_broadcast && broadcast_result) return broadcast_result;
  }

  for (struct component *cmp = pobj->components; cmp; ) {
    struct component *next = cmp->next;
    if (cmp->pcls->ObjProc) {
      void *cmp_data = cmp->pUserData;
      LRESULT res = cmp->pcls->ObjProc(pobj, cmp_data, MsgID, wParam, lParam);
      if (is_broadcast) {
        if (res) broadcast_result = res;
      } else if (res) {
        return res;
      }
    }
    cmp = next;
  }
  if (is_broadcast && broadcast_result) return broadcast_result;
  if (_IsNodeTriggerMessage(MsgID)) {
    LRESULT handled = _DispatchEventTriggerSpecialCase(pobj, MsgID, wParam, lParam, NULL);
    if (handled) {
      return handled;
    }
    return _DispatchNodeTriggers(pobj, MsgID, wParam, lParam);
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
