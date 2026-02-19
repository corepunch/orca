#include "core_local.h"

#define MAX_OBJECT_DATA (MAX_PROPERTY_STRING*32) // 1024

ORCA_API int ScreenWidth = 0, ScreenHeight = 0;

struct Object
{
  LPSTR Name;
  uint32_t identifier;

  lpObject_t parent;
  lpObject_t children;
  lpObject_t next;
  lpObject_t modal;

  LPSTR SourceFile;
  LPSTR TextContent;
  LPSTR ClassName;
  
  lpcKeyframeAnim_t animation;
  lpKeyframeAnim_t animlib;

  union
  {
    struct
    {
      struct component* components;
      struct property_animation* animations;
      lpProperty_t properties;
      struct state_manager* stateManager;
      struct style_class* classes;
      struct style_sheet* stylesheet;
      struct timer* timers;
      struct alias* aliases;
      struct script_callback* callbacks2;
    };
    void* comps[kCompCount];
  };

  uint32_t alias;
  uint32_t unique;
  uint32_t userdata;
  uint32_t luaObject;
  uint32_t flags;
  uint32_t rdflags;
  uint32_t datasize;

  longTime_t dirty;
  
  byte_t   data[MAX_OBJECT_DATA];
};

lpcString_t
OBJ_GetName(lpcObject_t object)
{
  return object->Name ? object->Name : "";
}

lpcString_t
OBJ_GetClassName(lpcObject_t object)
{
  if (object->components) {
    return CMP_GetClassName(object->components);
  } else {
    return "(none)";
  }
}

#ifdef DEBUG_COUNT_OBJECTS
static int counter = 0;
#endif

static uint32_t unique_counter = 0;

ORCA_API lpObject_t
OBJ_MakeNativeObject(lpcClassDesc_t cls) {
  lpObject_t object = ZeroAlloc(sizeof(struct Object));
  object->components = CMP_Create(object, cls);
//  object->window = WI_Get(L);
//  object->game = GetGame(L);
//  object->localization = GetLocalization(L);
  object->unique = ++unique_counter;
//  OBJ_SetDirty(object);
  return object;
}

lpObject_t
OBJ_Create(lua_State* L, lpcClassDesc_t cls)
{
#ifdef DEBUG_COUNT_OBJECTS
  Con_Error("number objects: %d", counter++);
#endif
  lpObject_t object = lua_newuserdata(L, sizeof(struct Object));
  luaL_setmetatable(L, API_TYPE_OBJECT);
  memset(object, 0, sizeof(struct Object));
  object->components = CMP_Create(object, cls);
  object->unique = ++unique_counter;
  OBJ_SetDirty(object);
  OBJ_SetName(object, cls->DefaultName);
  return object;
}

#define ID_TextureTiling 0x861dbc5b

uint32_t
OBJ_GetUniforms(lpObject_t object, struct uniform* pUniforms)
{
  uint32_t numunf = 0;
  bool_t hasTextureTiling = FALSE;
  for (lpProperty_t property = object->properties; property;
       property = PROP_GetNext(property)) {
    if (PROP_IsNull(property))
      continue;
    memset(pUniforms->Value, 0, sizeof(pUniforms->Value));
    pUniforms->Identifier = PROP_GetShortID(property);
    switch (PROP_GetType(property)) {
      case kDataTypeObject:
        if (!strcmp(PROP_GetUserData(property), "Texture")) {
          struct Texture *texture = GetTexture(*(void**)PROP_GetValue(property));
          pUniforms->Type = UT_SAMPLER_2D;
          // PROP_CopyValue(property, pUniforms->Value);
          memcpy(pUniforms->Value, &texture, sizeof(void*));
        } else {
          continue;
        }
        break;
      case kDataTypeFloat:
        pUniforms->Type = UT_FLOAT;
        PROP_CopyValue(property, pUniforms->Value);
        break;
      case kDataTypeVector2D:
        hasTextureTiling |= pUniforms->Identifier == ID_TextureTiling;
        pUniforms->Type = UT_FLOAT_VEC2;
        PROP_CopyValue(property, pUniforms->Value);
        break;
      case kDataTypeVector3D:
        pUniforms->Type = UT_FLOAT_VEC3;
        PROP_CopyValue(property, pUniforms->Value);
        break;
      case kDataTypeVector4D:
        pUniforms->Type = UT_FLOAT_VEC4;
        PROP_CopyValue(property, pUniforms->Value);
        break;
      case kDataTypeInt:
      case kDataTypeBool:
      case kDataTypeEnum:
        pUniforms->Type = UT_FLOAT;
        *pUniforms->Value = *(int*)PROP_GetValue(property);
        break;
      case kDataTypeColor:
        pUniforms->Type = UT_FLOAT_VEC4;
        PROP_CopyValue(property, pUniforms->Value);
        break;
      case kDataTypeGroup:
      default:
        continue;
    }
    pUniforms++;
    numunf++;
  }
  if (!hasTextureTiling) {
    memset(pUniforms->Value, 0, sizeof(pUniforms->Value));
    pUniforms->Identifier = ID_TextureTiling;
    pUniforms->Type = UT_FLOAT_VEC2;
    pUniforms->Value[0] = 1;
    pUniforms->Value[1] = 1;
    pUniforms++;
    numunf++;
  }
  return numunf;
}

void
OBJ_UpdateProperties(lpObject_t object)
{
  for (lpProperty_t p = object->properties; p; p = PROP_GetNext(p)) {
    PROP_Update(p);
  }
  FOR_EACH_CHILD(object, OBJ_UpdateProperties);
}

bool_t
OBJ_CheckName(lpcObject_t object, lpcString_t szName)
{
  return !strcmp(OBJ_GetName(object), szName);
}

void
OBJ_SetName(lpObject_t object, lpcString_t szName)
{
  SafeFree(object->Name);
  object->Name = strdup(szName);
  object->identifier = fnv1a32(object->Name);
}

void OBJ_AddChild(lpObject_t self, lpObject_t child, bool_t is_template)
{
  if (child->parent) {
    REMOVE_FROM_LIST(struct Object, child, child->parent->children);
  }
  ADD_TO_LIST_END(struct Object, child, self->children);
  child->parent = self;
  OBJ_SendMessageW(child, kEventAttached, 0, self);
  if (is_template) {
    OBJ_SetFlags(self, OBJ_GetFlags(self) | OF_TEMPLATE);
  }
}

void
OBJ_LoadPrefabs(lua_State* L, lpObject_t object)
{
  if (!OBJ_IsHidden(object)) {
    OBJ_SendMessage(object, "LoadView", 0, L);
  }
  FOR_EACH_OBJECT(child, object) {
    OBJ_LoadPrefabs(L, child);
  }
}

void
OBJ_SetDirty(lpObject_t pobj)
{
  pobj->dirty = core.realtime;
  for (lpObject_t hParent = pobj; hParent; hParent = hParent->parent) {
    hParent->flags |= OF_DIRTY;
  }
}

void
OBJ_ClearDirtyFlags(lpObject_t pobj)
{
  pobj->flags &= ~OF_DIRTY;
  FOR_EACH_CHILD(pobj, OBJ_ClearDirtyFlags);
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
    //		return OBJ_FindShortProperty(object, path);
    
    lpProperty_t plist = OBJ_GetProperties(object);
    lpProperty_t p = PROP_FindByFullName(plist, path);
    if (p) {
      return p;
    } else if (!strcmp(path, "Material.BlendIntensity")||
               !strcmp(path, "_BlendIntensity"))
    { // HACK: a workaround or expected behaviour?
      return PROP_FindByFullName(plist, "Node.Opacity");
//    } else if (!strcmp(path, "Node.ActualWidth")) {
//      return PROP_FindByFullName(plist, "Node.ActualWidth");
//    } else if (!strcmp(path, "Node.ActualHeight")) {
//      return PROP_FindByFullName(plist, "Node.ActualHeight");
//      return p;
    } else {
//      OBJ_FindLongProperty(object, path, &p);
      return p;
    }
  }
}

// void OBJ_ExecuteEventCallback(lua_State *L, lpObject_t  pobj, char const
// * szCallback, uint32_t dwNumArgs) { 	lpObject_t  hRoot =
// OBJ_GetParent(pobj) ? GetObjectRoot(OBJ_GetParent(pobj)) : NULL; 	if
// (hRoot) { 		lua_geti(L, LUA_REGISTRYINDEX, OBJ_GetLong(pobj,
// kGetObjectLongLuaObject)); for (int i = 0; i < dwNumArgs; lua_pushvalue(L, 3 +
// (i++)));
//		luaX_executecallback(L, hRoot, szCallback, dwNumArgs + 1);
//	}
// }

void
PROP_ExecuteChangedCallback(lua_State* L,
                            lpObject_t pobj,
                            lpProperty_t hProperty)
{
  lpcString_t szCallback = PROP_GetCallbackEvent(hProperty);
  lpObject_t hRoot = OBJ_GetRoot(pobj);
  if (hRoot) {
    lua_geti(L, LUA_REGISTRYINDEX, OBJ_GetLuaObject(pobj));
    luaX_pushProperty(L, hProperty);
    luaX_executecallback(L, hRoot, szCallback, 2);
  }
}


void
OBJ_Awake(lua_State* L, lpObject_t object)
{
  if (!(object->flags & OF_UPDATED_ONCE)) {
    lpcString_t cb = OBJ_FindCallbackForID(object, kEventAwake);
    if (cb) {
//      WI_PostMessageW(object, kEventAwake, 0, object);
      luaX_pushObject(L, object);
      lua_getfield(L, -1, cb);
      lua_insert(L, -2); // Move callback before obj
      if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        Con_Error("%s", luaL_checkstring(L, -1));
        lua_pop(L, 1);
      }
    }
    OBJ_ApplyStyles(object, FALSE);
    object->flags |= OF_UPDATED_ONCE;
  }
  FOR_EACH_OBJECT(child, object) OBJ_Awake(L, child);
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

void
OBJ_ProcessFunctions(lpObject_t object, lpcString_t name)
{
  void OBJ_ProcessFunctions2(lpObject_t, lpcString_t);
  OBJ_ProcessFunctions2(object, name);
  if (name[0] == 'o' && name[1] == 'n') {
    lpcString_t suffix = "Changed";
    size_t str_len = strlen(name);
    size_t suffix_len = 7; // strlen(suffix);
    if (str_len >= suffix_len &&
        strcmp(name + str_len - suffix_len, suffix) == 0) {
      size_t property_len = str_len - suffix_len;
      shortStr_t pname = { 0 };
      lpProperty_t pProp = NULL;
      strncpy(pname, name + 2, property_len - 2);
      if (SUCCEEDED(OBJ_FindShortProperty(object, pname, &pProp))) {
        PROP_SetFlag(pProp, PF_HASCHANGECALLBACK);
        //			} else {
        //				Con_Error("Could not find property for
        //%s", name);
      }
    }
  }
}

//#include <source/filesystem/filesystem.h>

static lpProperty_t
_CreateProjectProperty(lpObject_t object, uint32_t ident)
{
  lpcPropertyDesc_t pt = OBJ_FindPropertyType(ident);
  if (pt) {
    return PROP_Create(NULL,object,pt->id->Name,pt->DataType,pt->TypeString);
  }
  return NULL;
}

HRESULT
OBJ_FindLongProperty(lpObject_t object,
                     lpcString_t long_name,
                     lpProperty_t* ppProp)
{
  uint32_t identifier = fnv1a32(long_name);
  if ((*ppProp = PROP_FindByLongID(object->properties, identifier))) {
    return NOERROR;
  } else if ((*ppProp = _CreateClassProperty(object, identifier))) {
    return NOERROR;
  } else if ((*ppProp = _CreateProjectProperty(object, identifier))) {
    return NOERROR;
  } else {
    return E_ITEMNOTFOUND;
  }
}

HRESULT
OBJ_FindShortProperty(lpObject_t object,
                      lpcString_t short_name,
                      lpProperty_t* ppProp)
{
  uint32_t identifier = fnv1a32(short_name);
  if ((*ppProp = PROP_FindByShortID(object->properties, identifier))) {
    return NOERROR;
  } else {
    return OBJ_FindLongProperty(object, short_name, ppProp);
  }
}

int
OBJ_GetInteger(lpcObject_t object, uint32_t ident, int fallback)
{
  for (lpcProperty_t p = object->properties; p; p = PROP_GetNext(p)) {
    if (PROP_GetLongIdentifier(p) == ident) {
      switch (PROP_GetType(p)) {
        case kDataTypeInt:
        case kDataTypeEnum:
        case kDataTypeBool:
          return *(int *)PROP_GetValue(p);
        default:
          return fallback;
      }
    }
  }
  return fallback;
}

HRESULT
OBJ_SetPropertyValue(lpObject_t object,
                     lpcString_t name,
                     void const* value)
{
  lpProperty_t prop;
  HRESULT hr = OBJ_FindShortProperty(object, name, &prop);
  if (SUCCEEDED(hr)) {
    PROP_SetValue(prop, value);
  }
  return hr;
}

void
OBJ_RemoveFromParent(lua_State* L, lpObject_t self, bool_t dropModal)
{
  lpObject_t parent = OBJ_GetParent(self);
  if (dropModal && parent && OBJ_GetFlags(parent)&OF_NOACTIVATE) {
    OBJ_RemoveFromParent(L, parent, dropModal);
    return;
  }
  if (self->parent) {
    REMOVE_FROM_LIST(struct Object, self, self->parent->children);
    if (self->parent->modal == self) {
      self->parent->modal = NULL;
    }
    self->parent = NULL;
  }
  if (core.focus == self) {
    core.focus = NULL;
  }
  if (core.hover == self) {
    core.hover = NULL;
  }
  if (core.hover2 == self) {
    core.hover2 = NULL;
  }
  OBJ_Clear(L, self);
  WI_RemoveFromQueue(self);
  if (self->luaObject) {
    luaL_unref(L, LUA_REGISTRYINDEX, self->luaObject);
    self->luaObject = 0;
  }
}

void
OBJ_Clear(lua_State* L, lpObject_t pobj)
{
  FOR_EACH_OBJECT(other, pobj) {
    OBJ_Clear(L, other);
    OBJ_RemoveFromParent(L, other, FALSE);
  }
  pobj->children = NULL;
}

void
OBJ_Release(lua_State* L, lpObject_t pobj)
{
#ifdef DEBUG_COUNT_OBJECTS
  counter--;
#endif
//  if (core.focus == pobj) {
//    core.focus = NULL;
//  }
//	if (core.hover == pobj) {
//		core.hover = NULL;
//	}
//	if (core.hover2 == pobj) {
//		core.hover2 = NULL;
//	}

  OBJ_Clear(L, pobj);
  OBJ_SendMessage(pobj, "Destroy", 0, NULL);
  OBJ_RemoveFromParent(L, pobj, FALSE);

//  if (pobj->parent) {
//    REMOVE_FROM_LIST(struct Object, pobj, pobj->parent->children);
//    pobj->parent = NULL;
//  }

  if (pobj->animlib) {
    ANIM_Release(pobj->animlib);
  }
//  WI_RemoveFromQueue(pobj);

  OBJ_ReleaseTimers(pobj);
  OBJ_ReleaseAnimations(pobj);
  OBJ_ClearStyleClasses(pobj);
  OBJ_ReleaseComponents(pobj);
  OBJ_ReleaseProperties(pobj);
  OBJ_ReleaseAliases(pobj);

  SafeFree(pobj->SourceFile);
  SafeFree(pobj->TextContent);
  SafeFree(pobj->Name);
  SafeFree(pobj->ClassName);
}

void
OBJ_SetTextContent(lpObject_t pobj, lpcString_t szValue)
{
  SafeSet(pobj->TextContent, strdup(szValue), free);
  OBJ_SetDirty(pobj);
}

void
OBJ_SetSourceFile(lpObject_t pobj, lpcString_t szFilename)
{
  SafeSet(pobj->SourceFile, strdup(szFilename), free);
}

void
OBJ_SetClassName(lpObject_t pobj, lpcString_t szClassName)
{
  SafeSet(pobj->ClassName, strdup(szClassName), free);
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

void
OBJ_EmitPropertyChangedEvents(lua_State* L, lpObject_t object)
{
  SM_EnsureStateManagerInitialized(object);
  PROP_ProcessEvents(L, OBJ_GetProperties(object), object);
  OBJ_UpdateTimers(object);
  FOR_EACH_OBJECT(it, object) OBJ_EmitPropertyChangedEvents(L, it);
}

HRESULT
_RegisterProperty(lpObject_t object, lpProperty_t property)
{
  uint32_t psize = PROP_GetSize(property);
  assert(object->datasize + psize < MAX_OBJECT_DATA);
  PROP_AddToList(property, &object->properties);
  if (!CMP_SetProperty(object->components, property)) {
    PROP_SetValuePtr(property, object->data + object->datasize);
    object->datasize += psize;
  }
  return NOERROR;
}

void*
OBJ_GetObjectComponent(lpObject_t pobj, enum component_type c)
{
  return &pobj->comps[c];
}

lpKeyframeAnim_t
ANIM_FindByName(lpKeyframeAnim_t anims, lpcString_t name);

void
OBJ_SetFocus(lpObject_t pobj)
{
  if (core.focus == pobj || (pobj && (pobj->flags & OF_NOACTIVATE)))
    return;
  if (core.focus) {
    SV_PostMessage(core.focus, "KillFocus", 0, pobj);
    SV_PostMessage(core.focus, "ThemeChanged", 0, 0);
  }
  if (pobj) {
    SV_PostMessage(pobj, "SetFocus", 0, core.focus);
    SV_PostMessage(pobj, "ThemeChanged", 0, 0);
  }
  core.focus = pobj;
}

lpObject_t
core_GetFocus(void)
{
  if (core.focus && !OBJ_IsHidden(core.focus)) {
    lpObject_t focused = core.focus;
    for (; OBJ_GetModal(focused); focused = OBJ_GetModal(focused));
    return focused;
  } else {
    return NULL;
  }
}

bool_t OBJ_IsFocused(lpcObject_t pobj) {
  return core.focus == pobj;
}

void OBJ_SetHover(lpObject_t pobj) {
  core.hover2 = pobj;
}

lpObject_t core_GetHover(void) {
  return core.hover;
}

lpcString_t OBJ_GetSourceFile(lpcObject_t object) {
  return object->SourceFile;
}

lpcString_t OBJ_GetTextContent(lpcObject_t object) {
  return object->TextContent;
}

lpObject_t OBJ_GetParent(lpcObject_t object) {
  return object->parent;
}

lpObject_t OBJ_GetFirstChild(lpcObject_t object) {
  return object->children;
}

lpObject_t OBJ_GetNext(lpcObject_t object) {
  return object->next;
}

lpObject_t OBJ_GetRoot(lpObject_t object) {
  if (object->SourceFile) {
    return object;
  } else if (object->parent) {
    return OBJ_GetRoot(object->parent);
  } else {
    return object;
  }
}

lpProperty_t OBJ_GetProperties(lpcObject_t object) {
  return object->properties;
}

lpObject_t OBJ_FindChildByID(lpObject_t object, uint32_t id) {
//  if (object->identifier == id)
//    return object;
  FOR_EACH_LIST(struct Object, pChild, object->children)
  {
    if (pChild->identifier == id)
      return pChild;
//    lpObject_t pChildChild = OBJ_FindChildByID(pChild, id);
//    if (pChildChild) {
//      return pChildChild;
//    }
  }
  return NULL;
}

long OBJ_GetTimestamp(lpcObject_t pobj) {
  return pobj->dirty;
}

uint32_t OBJ_GetStyle(lpcObject_t pobj) {
  return pobj->flags;
}

void OBJ_SetStyle(lpObject_t pobj, uint32_t dwFlags) {
  pobj->flags = dwFlags;
}

uint32_t OBJ_GetFlags(lpcObject_t pobj) {
  return pobj->flags;
}

void OBJ_SetFlags(lpObject_t pobj, uint32_t dwFlags) {
  pobj->flags = dwFlags;
}

uint32_t OBJ_GetIdentifier(lpcObject_t pobj) {
  return pobj->identifier;
}

uint32_t OBJ_GetUniqueID(lpcObject_t pobj) {
  return pobj->unique;
}

uint32_t OBJ_GetLuaObject(lpcObject_t pobj) {
  return pobj->luaObject;
}

uint32_t OBJ_GetAlias(lpcObject_t pobj) {
  return pobj->alias;
}

void OBJ_SetLuaObject(lpObject_t pobj, uint32_t lua_id) {
  pobj->luaObject = lua_id;
}

void OBJ_SetAlias(lpObject_t pobj, uint32_t alias) {
  pobj->alias = alias;
}

lpProperty_t
OBJ_GetPropertyAtIndex(lpObject_t object,
                       uint32_t classid,
                       size_t classsize,
                       uint32_t index)
{
  void* userdata = OBJ_GetComponent(object, classid);
  if (!userdata)
    return NULL;
  lpProperty_t* props = (void*)(userdata + classsize);
  return props[index];
}


void
OBJ_SetAnimation(lpObject_t object, lpcString_t name)
{
  object->animation = ANIM_FindByName(object->animlib, name);
}

lpcKeyframeAnim_t
OBJ_GetAnimation(lpcObject_t object)
{
  return object->animation;
}

void
OBJ_AddAnimation(lpObject_t object, lpKeyframeAnim_t anim)
{
  ADD_TO_LIST(anim, object->animlib);
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
OBJ_GetModal(lpcObject_t self)
{
  return self->modal;
}

void
OBJ_SetModal(lpObject_t self, lpObject_t modal)
{
  if (self->modal) {
    OBJ_RemoveFromParent(core.L, self->modal, FALSE);
    self->modal = NULL;
  }
  if (modal) {
    self->modal = modal;
    modal->parent = self;
    modal->flags |= OF_NOACTIVATE;
  }
}

#include <source/editor/ed_stab_object.h>
