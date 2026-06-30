#include "property_internal.h"

void
Token_Release(struct token* token)
{
  SafeDelete(token->next, Token_Release);
  FOR_LOOP(i, TOKEN_MAX_ARGS)
  {
    SafeDelete(token->args[i], Token_Release);
  }
  free(token);
}

struct Property *
PROP_AddToList(struct Property *property, struct Property ** list)
{
  ADD_TO_LIST(property, *list);
  return property;
}

//#define DEBUG_PROGRAM

#ifdef DEBUG_PROGRAM
static void
print_name(struct Object *object)
{
  if (OBJ_GetParent(object)) {
    print_name(OBJ_GetParent(object));
  }
  Con_Error("%s/", OBJ_GetName(object));
}
#endif

static void
_ReleaseBindingNode(struct Binding *binding)
{
  if (!binding) {
    return;
  }
  SafeDelete(binding->token, Token_Release);
  /* Expression is a const char* interned from PROP_SetBinding; cast to free */
  free((char *)binding->Expression);
  free(binding);
}

static bool_t
_RunBinding(struct Property *property, struct Binding *binding)
{
  if (!binding->token) return TRUE;
  if (binding->updateFrame == core.frame) return TRUE;
  binding->updateFrame = core.frame;

  struct vm_register r = { 0 };
  if (!OBJ_RunProgram(property->object, binding->token, &r)) {
#ifdef DEBUG_PROGRAM
    print_name(property->object);
#endif
    Con_Error("Binding update failed while running program for %s/%s",
              OBJ_GetName(property->object),
              property->pdesc->Name);
    return FALSE;
  }
  if (!PROP_Import(property, &r)) {
#ifdef DEBUG_PROGRAM
    print_name(property->object);
#endif
    Con_Error("Binding update failed while importing into %s/%s",
              OBJ_GetName(property->object),
              property->pdesc->Name);
    return FALSE;
  }
  return TRUE;
}

bool_t
PROP_Update(struct Property *property)
{
  if (property == NULL)
    return FALSE;
  if (property->updateFrame == core.frame) {
    return FALSE;
  }
  property->updateFrame = core.frame;
  if (property->binding) {
    if (!_RunBinding(property, property->binding)) {
      _ReleaseBindingNode(property->binding);
      property->binding = NULL;
    }
  }
  return TRUE;
}

static void
_ReleaseArrayItems(struct Property *property)
{
  void *storage = property ? (void *)PROP_GetRawValueSlot(property) : NULL;
  if (!property->pdesc->IsArray || !storage) {
    return;
  }
  if (property->pdesc->DataType != kDataTypeObject) {
    return;
  }
  void **items = *(void**)storage;
  int count = ((int*)storage)[sizeof(void*)/sizeof(int)];
  FOR_LOOP(i, count) {
    if (items && items[i]) {
      OBJ_ReleaseRef((struct Object*)items[i]);
    }
  }
}

bool_t
PROP_IsNull(struct Property const *property)
{
  return property == NULL || !(property->flags & PF_MODIFIED);
}

void
PROP_Clear(struct Property *property)
{
  void *storage = property ? (void *)PROP_GetRawValueSlot(property) : NULL;
  void *old_array = NULL;
  if (property->pdesc->DataType == kDataTypeString && storage && *(LPSTR*)storage) {
    free(*(LPSTR*)storage);
  }
  if (property->pdesc->IsArray && storage) {
    old_array = *(void**)storage;
  }
  _ReleaseArrayItems(property);
  if (old_array) {
    free(old_array);
  }
  if (property->pdesc->DataType == kDataTypeObject &&
      storage && !property->pdesc->IsArray) {
    struct Object *object = PROP_GetObjectValue(property);
    if (object) {
      OBJ_ReleaseRef(object);
    }
  }
  if (storage) {
    memset(storage, 0, PROP_GetSize(property));
  }
  property->flags &= ~PF_MODIFIED;
  if (property->pdesc->DataType == kDataTypeString && storage) {
    *(char**)storage = NULL;
  }
}

/*static */struct Property *
_PropertyAlloc(lua_State* L, struct Object *object, struct PropertyType const *pt)
{
  (void)L;
  struct Property *property = ZeroAlloc(sizeof(struct Property));
  property->object  = object;
  property->pdesc   = pt;
  return property;
}

struct Property *
PROP_Create(lua_State* L, struct Object *object, struct PropertyType const *pt)
{
  struct Property *property = _PropertyAlloc(L, object, pt);
  _RegisterProperty(object, property);
  return property;
}

void
OBJ_ReleaseProperties(struct Object *hobj)
{
  FOR_EACH_LIST(struct Property, p, OBJ_GetProperties(hobj))
  {
    PROP_Clear(p);
    _ReleaseBindingNode(p->binding);
    p->binding = NULL;
    if (p->flags & PF_OWNS_STORAGE) {
      free(p->value);
      p->value = NULL;
    }
    free(p);
  }
}

static void
_RunObjectPrograms(struct Object *object)
{
  FOR_EACH_LIST(struct Property, p, OBJ_GetProperties(object))
  {
    if (p->binding) {
      PROP_Update(p);
    }
  }
  FOR_EACH_OBJECT(child, object) {
    _RunObjectPrograms(child);
  }
}

void
PROP_RunAllPrograms(struct Object *root)
{
  if (root) {
    _RunObjectPrograms(root);
  }
}

void
PROP_ClearSpecialized(struct Property *pprop) {
  FOR_EACH_LIST(struct Property, p, pprop) {
    if (p->flags & PF_SPECIALIZED) {
      PROP_Clear(p);
      p->flags = (p->flags & ~PF_SPECIALIZED);
      memset(p->value, 0, PROP_GetSize(p));
    }
  }
}

void
PROP_SetTypeSize(struct Property *p, eDataType_t t, uint32_t s)
{
  (void)p;
  (void)t;
  (void)s;
}

void
PROP_SetValuePtr(struct Property *prop, void* data)
{
  prop->value = data;
}

ORCA_API void
PDESC_Print(struct PropertyType const *pdesc, LPSTR buffer, uint32_t len, float const* pf)
{
  switch (pdesc->DataType) {
    case kDataTypeBool:
      strcpy(buffer, *(int*)pf ? "true" : "false");
      break;
    case kDataTypeInt:
      snprintf(buffer, len, "%dd", *(int*)pf);
      break;
    case kDataTypeEnum:
      strncpy(buffer, pdesc->EnumValues[*(int*)pf], len);
      break;
    case kDataTypeColor: {
      byte_t r = pf[0] * 255, g = pf[1] * 255, b = pf[2] * 255, a = pf[3] * 255;
      if (pf[3] < 1) {
        snprintf(buffer, len, "#%02x%02x%02x%02x", a, r, g, b);
      } else {
        snprintf(buffer, len, "#%02x%02x%02x", r, g, b);
      }
      break;
    }
    case kDataTypeStruct:
    case kDataTypeFloat:
      snprintf(buffer, len, "%g", pf[0]);
      FOR_LOOP(i, (int)pdesc->DataSize/sizeof(float)-1) {
        size_t buflen = strlen(buffer);
        snprintf(buffer + buflen, len - buflen, " %g", pf[i+1]);
      }
      break;
    case kDataTypeString:
      if (pf) {
        strncpy(buffer, (lpcString_t)pf, len);
      }
      break;
    case kDataTypeObject:
      if (*(handle_t *)pf) {
        lpcString_t s = "nil";
        if (s) {
          strncpy(buffer, s, len);
        } else {
          *buffer = 0;
        }
      } else {
        strncpy(buffer, "nil", len);
      }
      break;
    default:
      Con_Error("Unknown type %d in property %s\n", pdesc->DataType, pdesc->Name);
      break;
  }
}

ORCA_API void
PROP_Print(struct Property *p, LPSTR buffer, uint32_t len)
{
  PDESC_Print(p->pdesc, buffer, len, PROP_GetValue(p));
}

/* OBJ_FindStructDesc is declared in include/orca.h */

static bool_t
_FormatValueToBuffer(struct PropertyType const *pd,
                     void const *value,
                     LPSTR buf,
                     uint32_t len);

static bool_t
_FormatStructToBuffer(struct PropertyType const *pd,
                      void const *value,
                      LPSTR buf,
                      uint32_t len)
{
  struct StructDesc const *sd = pd->TypeString ? OBJ_FindStructDesc(pd->TypeString) : NULL;
  if (!sd) {
    /* Fallback: raw float array */
    size_t count = pd->DataSize / sizeof(float);
    float const *f = (float const *)value;
    size_t pos = 0;
    for (size_t i = 0; i < count && pos < len; i++) {
      if (i) { buf[pos++] = ' '; buf[pos] = 0; }
      int written = snprintf(buf + pos, len - pos, "%g", f[i]);
      if (written > 0) pos += (size_t)written;
    }
    return pos > 0;
  }
  size_t pos = 0;
  bool_t any = FALSE;
  FOR_LOOP(i, (int)sd->NumProperties) {
    struct PropertyType const *field = &sd->Properties[i];
    void const *fval = (char const *)value + field->Offset;
    if (field->DataType == kDataTypeObject || field->DataType == kDataTypeEvent) continue;
    char tmp[MAX_PROPERTY_STRING] = {0};
    if (_FormatValueToBuffer(field, fval, tmp, sizeof(tmp)) && tmp[0]) {
      if (any && pos < len - 1) { buf[pos++] = ' '; buf[pos] = 0; }
      size_t n = strlen(tmp);
      if (n > len - pos - 1) n = len - pos - 1;
      memcpy(buf + pos, tmp, n);
      pos += n;
      buf[pos] = 0;
      any = TRUE;
    }
  }
  return any;
}

static bool_t
_FormatValueToBuffer(struct PropertyType const *pd,
                     void const *value,
                     LPSTR buf,
                     uint32_t len)
{
  if (!value || !len) return FALSE;
  buf[0] = 0;
  switch (pd->DataType) {
    case kDataTypeBool:
      snprintf(buf, len, "%s", *(int const *)value ? "true" : "false");
      return TRUE;
    case kDataTypeInt:
      snprintf(buf, len, "%d", *(int const *)value);
      return TRUE;
    case kDataTypeEnum: {
      lpcString_t const *ev = pd->EnumValues;
      int idx = *(int const *)value;
      if (ev && ev[idx]) { snprintf(buf, len, "%s", ev[idx]); return TRUE; }
      snprintf(buf, len, "%d", idx);
      return TRUE;
    }
    case kDataTypeFloat:
      snprintf(buf, len, "%g", *(float const *)value);
      return TRUE;
    case kDataTypeColor: {
      float const *f = (float const *)value;
      uint8_t r = (uint8_t)(f[0] * 255.0f + 0.5f);
      uint8_t g = (uint8_t)(f[1] * 255.0f + 0.5f);
      uint8_t b = (uint8_t)(f[2] * 255.0f + 0.5f);
      uint8_t a = (uint8_t)(f[3] * 255.0f + 0.5f);
      if (a < 255)
        snprintf(buf, len, "#%02x%02x%02x%02x", a, r, g, b);
      else
        snprintf(buf, len, "#%02x%02x%02x", r, g, b);
      return TRUE;
    }
    case kDataTypeString: {
      lpcString_t str = *(lpcString_t const *)value;
      if (str) snprintf(buf, len, "%s", str);
      return TRUE;
    }
    case kDataTypeStruct:
      return _FormatStructToBuffer(pd, value, buf, len);
    case kDataTypeObject:
    case kDataTypeEvent:
    default:
      return FALSE;
  }
}

/* Public API: format any property value to a string.
 * Returns TRUE when buf was populated, FALSE for non-serializable types
 * (kDataTypeObject, kDataTypeEvent) or NULL value. */
ORCA_API bool_t
OBJ_FormatPropertyValue(struct PropertyType const *pd,
                        void const *value,
                        LPSTR buf,
                        uint32_t len)
{
  return _FormatValueToBuffer(pd, value, buf, len);
}
