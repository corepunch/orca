#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "fs_xml_inline.h"
#include <source/core/core_properties.h>
#include <source/core/property/property_internal.h>

extern int parse_property(const char* str,
                           struct PropertyType const* prop, void* valueptr);

static void
_InlineSkipDelims(const char **p)
{
  while (**p && (isspace((unsigned char)**p) || **p == ',')) {
    (*p)++;
  }
}

static char *
_InlineStrNDup(const char *s, size_t n)
{
  char *out = (char *)malloc(n + 1);
  if (!out) {
    return NULL;
  }
  memcpy(out, s, n);
  out[n] = '\0';
  return out;
}

static bool_t
_InlineReadToken(const char **p, char **out)
{
  _InlineSkipDelims(p);
  if (!**p || **p == '}') {
    return FALSE;
  }

  const char *start = *p;
  if (**p == '"' || **p == '\'') {
    char quote = *(*p)++;
    start = *p;
    while (**p && **p != quote) {
      (*p)++;
    }
    *out = _InlineStrNDup(start, (size_t)(*p - start));
    if (**p == quote) {
      (*p)++;
    }
    return *out != NULL;
  }

  while (**p &&
         !isspace((unsigned char)**p) &&
         **p != ',' &&
         **p != '}' &&
         **p != '=') {
    (*p)++;
  }
  *out = _InlineStrNDup(start, (size_t)(*p - start));
  return *out != NULL;
}

static bool_t
_InlineReadValue(const char **p, char **out)
{
  _InlineSkipDelims(p);
  if (!**p || **p == '}') {
    return FALSE;
  }

  const char *start = *p;
  if (**p == '"' || **p == '\'') {
    char quote = *(*p)++;
    start = *p;
    while (**p && **p != quote) {
      (*p)++;
    }
    *out = _InlineStrNDup(start, (size_t)(*p - start));
    if (**p == quote) {
      (*p)++;
    }
    return *out != NULL;
  }

  while (**p &&
         !isspace((unsigned char)**p) &&
         **p != ',' &&
         **p != '}') {
    (*p)++;
  }
  *out = _InlineStrNDup(start, (size_t)(*p - start));
  return *out != NULL;
}

static void
_InlineBufferEnsure(char **buf, size_t *cap, size_t need)
{
  if (*cap > need) {
    return;
  }
  size_t newcap = *cap ? *cap : 128;
  while (newcap <= need) {
    newcap *= 2;
  }
  *buf = (char *)realloc(*buf, newcap);
  *cap = newcap;
}

static void
_InlineBufferAppendN(char **buf, size_t *len, size_t *cap,
                     const char *text, size_t text_len)
{
  _InlineBufferEnsure(buf, cap, *len + text_len + 1);
  memcpy(*buf + *len, text, text_len);
  *len += text_len;
  (*buf)[*len] = '\0';
}

static void
_InlineBufferAppend(char **buf, size_t *len, size_t *cap, const char *text)
{
  _InlineBufferAppendN(buf, len, cap, text, strlen(text));
}

static void
_InlineBufferAppendChar(char **buf, size_t *len, size_t *cap, char ch)
{
  _InlineBufferEnsure(buf, cap, *len + 2);
  (*buf)[(*len)++] = ch;
  (*buf)[*len] = '\0';
}

static const char *
_XmlFindTagEnd(const char *p)
{
  char quote = 0;
  while (*p) {
    if (quote) {
      if (*p == quote) {
        quote = 0;
      }
    } else if (*p == '"' || *p == '\'') {
      quote = *p;
    } else if (*p == '>') {
      return p;
    }
    p++;
  }
  return NULL;
}

static bool_t
_XmlIsPositionalProperty(struct ClassDesc const *cls,
                         struct PropertyType const *field)
{
  if (!cls || !field) {
    return FALSE;
  }
  if (cls->NumProperties > 1 &&
      !strcmp(field->Name, "Trigger") &&
      field->DataType == kDataTypeObject) {
    return FALSE;
  }
  return TRUE;
}

static struct PropertyType const *
_XmlFindNextPositionalField(struct ClassDesc const *cls,
                           bool_t *used,
                           int *cursor)
{
  if (!cls) {
    return NULL;
  }
  while (*cursor < (int)cls->NumProperties) {
    int idx = (*cursor)++;
    struct PropertyType const *field = &cls->Properties[idx];
    if ((!used || !used[idx]) && _XmlIsPositionalProperty(cls, field)) {
      return field;
    }
  }
  return NULL;
}

static void
_InlineBufferAppendEscaped(char **buf, size_t *len, size_t *cap, const char *text)
{
  for (const unsigned char *p = (const unsigned char *)text; *p; ++p) {
    switch (*p) {
      case '&': _InlineBufferAppend(buf, len, cap, "&amp;"); break;
      case '<': _InlineBufferAppend(buf, len, cap, "&lt;"); break;
      case '>': _InlineBufferAppend(buf, len, cap, "&gt;"); break;
      case '"': _InlineBufferAppend(buf, len, cap, "&quot;"); break;
      case '\'': _InlineBufferAppend(buf, len, cap, "&apos;"); break;
      default: _InlineBufferAppendChar(buf, len, cap, (char)*p); break;
    }
  }
}

static struct PropertyType const *
_FindNextPositionalField(struct PropertyType const *fields, int count,
                         bool_t *used, int *cursor)
{
  while (*cursor < count) {
    int idx = (*cursor)++;
    if (!used || !used[idx]) {
      return &fields[idx];
    }
  }
  return NULL;
}

struct PropertyType const *
_FindStructField(struct StructDesc const *sdesc, lpcString_t name)
{
  FOR_LOOP(i, sdesc->NumProperties) {
    struct PropertyType const *field = &sdesc->Properties[i];
    if (!strcmp(field->Name, name)) {
      return field;
    }
  }
  return NULL;
}

bool_t
_SetStructFieldFromString(struct PropertyType const *pdesc, void *valueptr,
                          lpcString_t value)
{
  lpcString_t resolved = (value[0] == '$') ? FS_GetThemeValue(value) : NULL;
  if (!resolved) {
    resolved = value;
  }

  char tmpbuf[MAX_PROPERTY_STRING] = {0};
  if (!parse_property(resolved, pdesc, tmpbuf)) {
    return FALSE;
  }

  if (pdesc->DataType == kDataTypeString) {
    *(char**)valueptr = *(char**)tmpbuf;
  } else {
    memcpy(valueptr, tmpbuf, pdesc->DataSize);
  }
  return TRUE;
}

void
_ClearStructValue(struct StructDesc const *sdesc, void *value)
{
  FOR_LOOP(i, sdesc->NumProperties) {
    struct PropertyType const *field = &sdesc->Properties[i];
    if (field->DataType == kDataTypeString && field->DataSize == sizeof(char*)) {
      char **slot = (char **)((char*)value + field->Offset);
      if (*slot) {
        free(*slot);
        *slot = NULL;
      }
    }
  }
}

static bool_t
_LoadStructTokens(struct PropertyType const *pdesc,
                  struct StructDesc const *sdesc,
                  lpcString_t text,
                  void *dst)
{
  const char *p = text;
  char *token = NULL;
  bool_t ok = TRUE;
  int positional = 0;
  bool_t *assigned = (bool_t *)calloc((size_t)sdesc->NumProperties, sizeof(bool_t));

  if (!assigned) {
    Con_Error("Could not allocate struct parse state for '%s'", pdesc->Name);
    return FALSE;
  }

  while (ok) {
    if (!_InlineReadToken(&p, &token)) {
      break;
    }

    _InlineSkipDelims(&p);
    if (*p == '=') {
      p++;
      char *value = NULL;
      if (!_InlineReadValue(&p, &value)) {
        Con_Error("Missing value for inline struct field '%s' on property '%s'",
                  token, pdesc->Name);
        free(token);
        ok = FALSE;
        break;
      }
      struct PropertyType const *field = _FindStructField(sdesc, token);
      if (!field) {
        Con_Error("Unknown field '%s' for struct '%s' in property '%s'",
                  token, sdesc->StructName, pdesc->Name);
        free(token);
        free(value);
        ok = FALSE;
        break;
      }
      if (!_SetStructFieldFromString(field, (char*)dst + field->Offset, value)) {
        ok = FALSE;
      }
      assigned[field - sdesc->Properties] = TRUE;
      free(token);
      free(value);
      continue;
    }

    struct PropertyType const *field = _FindNextPositionalField(sdesc->Properties,
                                                                (int)sdesc->NumProperties,
                                                                assigned,
                                                                &positional);
    if (!field) {
      Con_Error("Too many positional values for struct '%s' in property '%s'",
                sdesc->StructName, pdesc->Name);
      free(token);
      ok = FALSE;
      break;
    }
    if (assigned[field - sdesc->Properties]) {
      int next = positional;
      while (next < (int)sdesc->NumProperties && assigned[next]) {
        next++;
      }
      positional = next;
      field = (positional < (int)sdesc->NumProperties) ? &sdesc->Properties[positional++] : NULL;
    }
    if (!field) {
      Con_Error("Too many positional values for struct '%s' in property '%s'",
                sdesc->StructName, pdesc->Name);
      free(token);
      ok = FALSE;
      break;
    }

    if (!_SetStructFieldFromString(field, (char*)dst + field->Offset, token)) {
      ok = FALSE;
    }
    assigned[field - sdesc->Properties] = TRUE;
    free(token);
    token = NULL;
  }

  free(assigned);
  return ok;
}

bool_t
_LoadStructFromXmlFragment(struct Property *prop,
                           struct PropertyType const *pdesc,
                           lpcString_t text,
                           lpcString_t prop_name)
{
  (void)prop_name;

  struct StructDesc const *sdesc = OBJ_FindStructDesc(pdesc->TypeString);
  if (!sdesc) {
    Con_Error("Could not resolve struct descriptor '%s' for property '%s'",
              pdesc->TypeString, pdesc->Name);
    return FALSE;
  }

  void *tmp = calloc(1, (size_t)sdesc->StructSize);
  if (!tmp) {
    Con_Error("Could not allocate struct storage for property '%s'", pdesc->Name);
    return FALSE;
  }

  const char *body = text;
  while (*body && isspace((unsigned char)*body)) {
    body++;
  }
  if (*body == '{') {
    body++;
  }
  size_t len = strlen(body);
  while (len > 0 && isspace((unsigned char)body[len - 1])) {
    len--;
  }
  if (len > 0 && body[len - 1] == '}') {
    len--;
  }

  char *copy = _InlineStrNDup(body, len);
  if (!copy) {
    free(tmp);
    return FALSE;
  }

  bool_t ok = _LoadStructTokens(pdesc, sdesc, copy, tmp);
  if (ok) {
    PROP_SetValue(prop, tmp);
  } else {
    _ClearStructValue(sdesc, tmp);
  }

  free(copy);
  free(tmp);
  return ok;
}

static struct ClassDesc const *
_FindInlineClass(lpcString_t name)
{
  return OBJ_FindClass(name);
}

static bool_t
_BuildInlineObjectXml(struct ClassDesc const *cls,
                      lpcString_t text,
                      int positional_start,
                      char **out_xml)
{
  const char *p = text;
  while (*p && isspace((unsigned char)*p)) {
    p++;
  }
  if (*p == '{') {
    p++;
  }
  char *type = NULL;
  if (!_InlineReadToken(&p, &type) || !type || !*type) {
    free(type);
    return FALSE;
  }

  struct ClassDesc const *target = cls ? cls : _FindInlineClass(type);
  if (!target) {
    Con_Error("Unknown inline object type '%s'", type);
    free(type);
    return FALSE;
  }

  size_t cap = 0;
  size_t len = 0;
  char *buf = NULL;
  _InlineBufferAppendChar(&buf, &len, &cap, '<');
  _InlineBufferAppend(&buf, &len, &cap, type);

  int positional = positional_start;
  bool_t *assigned = (bool_t *)calloc((size_t)target->NumProperties, sizeof(bool_t));
  if (!assigned) {
    free(type);
    free(buf);
    return FALSE;
  }

  while (TRUE) {
    char *name = NULL;
    if (!_InlineReadToken(&p, &name)) {
      break;
    }
    _InlineSkipDelims(&p);
    if (*p == '=') {
      p++;
      char *value = NULL;
      if (!_InlineReadValue(&p, &value)) {
        Con_Error("Missing inline object value for '%s'", name);
        free(name);
        free(assigned);
        free(type);
        free(buf);
        return FALSE;
      }
      for (int i = 0; i < (int)target->NumProperties; i++) {
        if (!assigned[i] && !strcmp(target->Properties[i].Name, name)) {
          assigned[i] = TRUE;
          break;
        }
      }
      _InlineBufferAppendChar(&buf, &len, &cap, ' ');
      _InlineBufferAppend(&buf, &len, &cap, name);
      _InlineBufferAppend(&buf, &len, &cap, "=\"");
      _InlineBufferAppendEscaped(&buf, &len, &cap, value);
      _InlineBufferAppendChar(&buf, &len, &cap, '"');
      free(value);
      free(name);
      continue;
    }

    struct PropertyType const *field = _FindNextPositionalField(target->Properties,
                                                                (int)target->NumProperties,
                                                                assigned,
                                                                &positional);
    if (!field) {
      Con_Error("Too many positional values for inline object '%s'", type);
      free(name);
      free(assigned);
      free(type);
      free(buf);
      return FALSE;
    }
    assigned[field - target->Properties] = TRUE;

    _InlineBufferAppendChar(&buf, &len, &cap, ' ');
    _InlineBufferAppend(&buf, &len, &cap, field->Name);
    _InlineBufferAppend(&buf, &len, &cap, "=\"");
    _InlineBufferAppendEscaped(&buf, &len, &cap, name);
    _InlineBufferAppendChar(&buf, &len, &cap, '"');
    free(name);
  }

  _InlineBufferAppend(&buf, &len, &cap, "/>");
  free(assigned);
  free(type);
  *out_xml = buf;
  return TRUE;
}

static bool_t
_AppendObjectToArrayProperty(struct Property *prop, struct Object *item)
{
  struct PropertyType const *pdesc = PROP_GetDesc(prop);
  if (!pdesc || !pdesc->IsArray || pdesc->DataType != kDataTypeObject) {
    return FALSE;
  }

  void *current_items = NULL;
  int current_count = 0;
  if (prop->value) {
    memcpy(&current_items, prop->value, sizeof(current_items));
    memcpy(&current_count,
           (char *)prop->value + sizeof(current_items),
           sizeof(current_count));
  }

  int new_count = current_count + 1;
  struct Object **items = (struct Object **)calloc((size_t)new_count, sizeof(struct Object *));
  if (!items) {
    return FALSE;
  }

  if (current_items && current_count > 0) {
    memcpy(items, current_items, (size_t)current_count * sizeof(struct Object *));
  }
  items[current_count] = item;

  void *next[2] = { items, NULL };
  memcpy((char *)next + sizeof(void *), &new_count, sizeof(new_count));
  PROP_SetValue(prop, next);
  return TRUE;
}

struct Object *
_LoadObjectFromXmlFragment(lpcString_t text,
                           lpcString_t prop_name,
                           int positional_start)
{
  (void)prop_name;

  while (*text && isspace((unsigned char)*text)) {
    text++;
  }

  if (*text == '<') {
    return FS_LoadObjectFromXmlString(text);
  }
  if (*text != '{') {
    return FS_LoadObjectFromXmlString(text);
  }

  char *xml = NULL;
  if (!_BuildInlineObjectXml(NULL, text, positional_start, &xml)) {
    return NULL;
  }

  struct Object *result = FS_LoadObjectFromXmlString(xml);
  free(xml);
  return result;
}

bool_t
_LoadEventTriggerFromXmlFragment(struct Object *obj,
                                 struct PropertyType const *pdesc,
                                 lpcString_t text)
{
  if (!obj || !pdesc) {
    return FALSE;
  }

  struct PropertyType const *triggers_desc = OBJ_FindImplicitProperty(obj, "Triggers");
  if (!triggers_desc) {
    Con_Error("Property '%s' does not support inline trigger shorthand", pdesc->Name);
    return FALSE;
  }

  struct Property *triggers_prop = NULL;
  if (FAILED(OBJ_FindLongProperty(obj, triggers_desc->FullIdentifier, &triggers_prop))) {
    Con_Error("Could not get trigger array slot for '%s'", pdesc->Name);
    return FALSE;
  }

  struct Object *action = _LoadObjectFromXmlFragment(text, pdesc->Name, 1);
  if (!action) {
    return FALSE;
  }

  struct Object *trigger = OBJ_Create(ID_EventTrigger);
  if (!trigger) {
    OBJ_ReleaseRef(action);
    return FALSE;
  }

  char routed_event[MAX_PROPERTY_STRING] = {0};
  if (pdesc->Category && *pdesc->Category) {
    snprintf(routed_event, sizeof(routed_event), "%s.%s", pdesc->Category, pdesc->Name);
  } else {
    snprintf(routed_event, sizeof(routed_event), "%s", pdesc->Name);
  }

  lpcString_t routed_event_value = routed_event;
  if (FAILED(OBJ_SetPropertyValue(trigger, "RoutedEvent", &routed_event_value))) {
    OBJ_ReleaseRef(trigger);
    OBJ_ReleaseRef(action);
    return FALSE;
  }

  OBJ_AddChild(trigger, action, FALSE);
  if (!_AppendObjectToArrayProperty(triggers_prop, trigger)) {
    OBJ_ReleaseRef(trigger);
    return FALSE;
  }

  return TRUE;
}

char *
_NormalizeXmlShorthand(lpcString_t xml)
{
  if (!xml) return NULL;

  size_t cap = strlen(xml) + 1;
  char *out = (char *)calloc(cap, 1);
  size_t len = 0;
  const char *p = xml;
  if (!out) return NULL;

  while (*p) {
    const char *lt = strchr(p, '<');
    if (!lt) {
      _InlineBufferAppend(&out, &len, &cap, p);
      break;
    }

    _InlineBufferAppendN(&out, &len, &cap, p, (size_t)(lt - p));

    if (!strncmp(lt, "<!--", 4)) {
      const char *end = strstr(lt + 4, "-->");
      if (!end) {
        _InlineBufferAppend(&out, &len, &cap, lt);
        break;
      }
      _InlineBufferAppendN(&out, &len, &cap, lt, (size_t)(end + 3 - lt));
      p = end + 3;
      continue;
    }

    if (!strncmp(lt, "<?", 2)) {
      const char *end = strstr(lt + 2, "?>");
      if (!end) {
        _InlineBufferAppend(&out, &len, &cap, lt);
        break;
      }
      _InlineBufferAppendN(&out, &len, &cap, lt, (size_t)(end + 2 - lt));
      p = end + 2;
      continue;
    }

    if (lt[1] == '!' || lt[1] == '/') {
      const char *end = strchr(lt, '>');
      if (!end) {
        _InlineBufferAppend(&out, &len, &cap, lt);
        break;
      }
      _InlineBufferAppendN(&out, &len, &cap, lt, (size_t)(end + 1 - lt));
      p = end + 1;
      continue;
    }

    const char *name_start = lt + 1;
    const char *name_end = name_start;
    while (*name_end &&
           (isalnum((unsigned char)*name_end) ||
            *name_end == '_' ||
            *name_end == '-' ||
            *name_end == ':' ||
            *name_end == '.')) {
      name_end++;
    }
    if (name_end == name_start) {
      _InlineBufferAppendChar(&out, &len, &cap, *lt);
      p = lt + 1;
      continue;
    }

    const char *end = _XmlFindTagEnd(name_end);
    if (!end) {
      _InlineBufferAppend(&out, &len, &cap, lt);
      break;
    }

    char *class_name = _InlineStrNDup(name_start, (size_t)(name_end - name_start));
    struct ClassDesc const *cls = class_name ? OBJ_FindClass(class_name) : NULL;
    free(class_name);
    if (!cls) {
      _InlineBufferAppendN(&out, &len, &cap, lt, (size_t)(end + 1 - lt));
      p = end + 1;
      continue;
    }

    const char *body_start = name_end;
    const char *body_end = end;
    while (body_end > body_start && isspace((unsigned char)body_end[-1])) {
      body_end--;
    }
    bool_t self_closing = FALSE;
    if (body_end > body_start && body_end[-1] == '/') {
      self_closing = TRUE;
      body_end--;
      while (body_end > body_start && isspace((unsigned char)body_end[-1])) {
        body_end--;
      }
    }

    const char *q = body_start;
    while (q < body_end && isspace((unsigned char)*q)) {
      q++;
    }
    if (q >= body_end) {
      _InlineBufferAppendN(&out, &len, &cap, lt, (size_t)(end + 1 - lt));
      p = end + 1;
      continue;
    }

    const char *token_start = q;
    while (q < body_end && !isspace((unsigned char)*q) && *q != '=') {
      q++;
    }
    const char *token_end = q;
    const char *after_token = q;
    while (after_token < body_end && isspace((unsigned char)*after_token)) {
      after_token++;
    }

    if (after_token < body_end && *after_token == '=') {
      _InlineBufferAppendN(&out, &len, &cap, lt, (size_t)(end + 1 - lt));
      p = end + 1;
      continue;
    }

    int cursor = 0;
    struct PropertyType const *field = _XmlFindNextPositionalField(cls, NULL, &cursor);
    if (!field) {
      _InlineBufferAppendN(&out, &len, &cap, lt, (size_t)(end + 1 - lt));
      p = end + 1;
      continue;
    }

    _InlineBufferAppendChar(&out, &len, &cap, '<');
    _InlineBufferAppendN(&out, &len, &cap, name_start, (size_t)(name_end - name_start));
    _InlineBufferAppendChar(&out, &len, &cap, ' ');
    _InlineBufferAppend(&out, &len, &cap, field->Name);
    _InlineBufferAppend(&out, &len, &cap, "=\"");
    _InlineBufferAppendN(&out, &len, &cap, token_start, (size_t)(token_end - token_start));
    _InlineBufferAppendChar(&out, &len, &cap, '"');
    if (after_token < body_end) {
      _InlineBufferAppendChar(&out, &len, &cap, ' ');
    }
    _InlineBufferAppendN(&out, &len, &cap, after_token, (size_t)(body_end - after_token));
    _InlineBufferAppend(&out, &len, &cap, self_closing ? "/>" : ">");

    p = end + 1;
  }

  return out;
}
