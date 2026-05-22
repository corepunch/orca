#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "fs_xml_inline.h"

extern int parse_property(const char* str,
                          struct PropertyType const* prop,
                          void* valueptr);

struct reader {
  char const *p;
};

struct strbuf {
  char *s;
  size_t n;
  size_t cap;
};

static void
skip(struct reader *r)
{
  while (*r->p && (isspace((unsigned char)*r->p) || *r->p == ',')) {
    r->p++;
  }
}

static char *
copy(char const *s, size_t n)
{
  char *out = malloc(n + 1);
  if (out) {
    memcpy(out, s, n);
    out[n] = 0;
  }
  return out;
}

static bool_t
read_atom(struct reader *r, char **out)
{
  skip(r);
  if (!*r->p || *r->p == '}') return FALSE;

  char quote = (*r->p == '"' || *r->p == '\'') ? *r->p++ : 0;
  char const *start = r->p;
  while (*r->p && (quote ? *r->p != quote :
         (!isspace((unsigned char)*r->p) && *r->p != ',' && *r->p != '}' && *r->p != '='))) {
    r->p++;
  }
  *out = copy(start, (size_t)(r->p - start));
  if (quote && *r->p == quote) r->p++;
  return *out != NULL;
}

static void
grow(struct strbuf *b, size_t need)
{
  if (b->cap > need) return;
  b->cap = b->cap ? b->cap * 2 : 128;
  while (b->cap <= need) b->cap *= 2;
  b->s = realloc(b->s, b->cap);
}

static void
putn(struct strbuf *b, char const *s, size_t n)
{
  grow(b, b->n + n + 1);
  memcpy(b->s + b->n, s, n);
  b->n += n;
  b->s[b->n] = 0;
}

static void
put(struct strbuf *b, char const *s)
{
  putn(b, s, strlen(s));
}

static void
putc_xml(struct strbuf *b, char c)
{
  grow(b, b->n + 2);
  b->s[b->n++] = c;
  b->s[b->n] = 0;
}

static void
put_escaped(struct strbuf *b, char const *s)
{
  for (; *s; s++) {
    switch (*s) {
      case '&': put(b, "&amp;"); break;
      case '<': put(b, "&lt;"); break;
      case '>': put(b, "&gt;"); break;
      case '"': put(b, "&quot;"); break;
      case '\'': put(b, "&apos;"); break;
      default: putc_xml(b, *s); break;
    }
  }
}

static char const *
body_of(lpcString_t text)
{
  while (*text && isspace((unsigned char)*text)) text++;
  return *text == '{' ? text + 1 : text;
}

static struct PropertyType const *
next_field(struct PropertyType const *fields, int count, bool_t *used, int *cursor)
{
  while (*cursor < count) {
    struct PropertyType const *field = &fields[(*cursor)++];
    if ((!used || !used[field - fields]) &&
        !(field->DataType == kDataTypeObject && !strcmp(field->Name, "Trigger"))) {
      return field;
    }
  }
  return NULL;
}

struct PropertyType const *
_FindStructField(struct StructDesc const *sdesc, lpcString_t name)
{
  FOR_LOOP(i, sdesc->NumProperties) {
    if (!strcmp(sdesc->Properties[i].Name, name)) {
      return &sdesc->Properties[i];
    }
  }
  return NULL;
}

bool_t
_SetStructFieldFromString(struct PropertyType const *pdesc,
                          void *valueptr,
                          lpcString_t value)
{
  char tmp[MAX_PROPERTY_STRING] = {0};
  lpcString_t resolved = (value[0] == '$') ? FS_GetThemeValue(value) : value;
  if (!resolved || !parse_property(resolved, pdesc, tmp)) return FALSE;

  if (pdesc->DataType == kDataTypeString) {
    *(char**)valueptr = *(char**)tmp;
  } else {
    memcpy(valueptr, tmp, pdesc->DataSize);
  }
  return TRUE;
}

void
_ClearStructValue(struct StructDesc const *sdesc, void *value)
{
  FOR_LOOP(i, sdesc->NumProperties) {
    struct PropertyType const *field = &sdesc->Properties[i];
    if (field->DataType == kDataTypeString && field->DataSize == sizeof(char*)) {
      free(*(char **)((char *)value + field->Offset));
    }
  }
}

bool_t
_LoadStructFromXmlFragment(struct Property *prop,
                           struct PropertyType const *pdesc,
                           lpcString_t text)
{
  struct StructDesc const *sdesc = OBJ_FindStructDesc(pdesc->TypeString);
  if (!sdesc) {
    Con_Error("Could not resolve struct descriptor '%s' for property '%s'",
              pdesc->TypeString, pdesc->Name);
    return FALSE;
  }

  void *tmp = calloc(1, (size_t)sdesc->StructSize);
  bool_t *used = calloc((size_t)sdesc->NumProperties, sizeof(bool_t));
  bool_t ok = tmp && used;
  struct reader r = { body_of(text) };
  int cursor = 0;

  while (ok) {
    char *name = NULL;
    char *value = NULL;
    if (!read_atom(&r, &name)) break;
    skip(&r);

    struct PropertyType const *field = NULL;
    if (*r.p == '=') {
      r.p++;
      ok = read_atom(&r, &value);
      field = ok ? _FindStructField(sdesc, name) : NULL;
    } else {
      value = name;
      name = NULL;
      field = next_field(sdesc->Properties, (int)sdesc->NumProperties, used, &cursor);
    }

    if (!field) {
      Con_Error("Unknown or extra field while parsing struct '%s' for property '%s'",
                sdesc->StructName, pdesc->Name);
      ok = FALSE;
    } else if (!_SetStructFieldFromString(field, (char *)tmp + field->Offset, value)) {
      ok = FALSE;
    } else {
      used[field - sdesc->Properties] = TRUE;
    }
    free(name);
    free(value);
  }

  if (ok) {
    PROP_SetValue(prop, tmp);
  } else if (tmp) {
    _ClearStructValue(sdesc, tmp);
  }
  free(used);
  free(tmp);
  return ok;
}

static bool_t
inline_object_xml(lpcString_t text, int positional_start, char **out)
{
  struct reader r = { body_of(text) };
  char *type = NULL;
  if (!read_atom(&r, &type)) return FALSE;

  struct ClassDesc const *cls = OBJ_FindClass(type);
  if (!cls) {
    Con_Error("Unknown inline object type '%s'", type);
    free(type);
    return FALSE;
  }

  struct strbuf xml = {0};
  bool_t *used = calloc((size_t)cls->NumProperties, sizeof(bool_t));
  int cursor = positional_start;
  bool_t ok = used != NULL;
  putc_xml(&xml, '<');
  put(&xml, type);

  while (ok) {
    char *name = NULL;
    char *value = NULL;
    if (!read_atom(&r, &name)) break;
    skip(&r);

    struct PropertyType const *field = NULL;
    if (*r.p == '=') {
      r.p++;
      ok = read_atom(&r, &value);
      FOR_LOOP(i, cls->NumProperties) {
        if (!strcmp(cls->Properties[i].Name, name)) {
          field = &cls->Properties[i];
          break;
        }
      }
    } else {
      value = name;
      name = NULL;
      field = next_field(cls->Properties, (int)cls->NumProperties, used, &cursor);
    }

    if (!field && name && (!strcmp(name, "Name") || !strcmp(name, "id") ||
                           !strcmp(name, "class") || strchr(name, '.'))) {
      putc_xml(&xml, ' ');
      put(&xml, name);
      put(&xml, "=\"");
      put_escaped(&xml, value);
      putc_xml(&xml, '"');
    } else if (!field) {
      Con_Error("Unknown or extra field while parsing inline object '%s'", type);
      ok = FALSE;
    } else {
      used[field - cls->Properties] = TRUE;
      putc_xml(&xml, ' ');
      put(&xml, field->Name);
      put(&xml, "=\"");
      put_escaped(&xml, value);
      putc_xml(&xml, '"');
    }
    free(name);
    free(value);
  }

  put(&xml, "/>");
  free(used);
  free(type);
  if (!ok) free(xml.s);
  *out = ok ? xml.s : NULL;
  return ok;
}

struct Object *
_LoadObjectFromXmlFragment(lpcString_t text, int positional_start)
{
  while (*text && isspace((unsigned char)*text)) text++;
  if (*text == '<') return FS_LoadObjectFromXmlString(text);
  if (*text != '{') return FS_LoadObjectFromXmlString(text);

  char *xml = NULL;
  struct Object *obj = inline_object_xml(text, positional_start, &xml)
    ? FS_LoadObjectFromXmlString(xml)
    : NULL;
  free(xml);
  return obj;
}

static char const *
tag_end(char const *p)
{
  char quote = 0;
  for (; *p; p++) {
    if (quote) {
      if (*p == quote) quote = 0;
    } else if (*p == '"' || *p == '\'') {
      quote = *p;
    } else if (*p == '>') {
      return p;
    }
  }
  return NULL;
}

static bool_t
name_char(char c)
{
  return isalnum((unsigned char)c) || c == '_' || c == '-' || c == ':' || c == '.';
}

char *
_ExpandXmlPositionalArgs(lpcString_t xml)
{
  struct strbuf out = {0};
  char const *p = xml;

  while (*p) {
    char const *lt = strchr(p, '<');
    if (!lt) {
      put(&out, p);
      break;
    }

    putn(&out, p, (size_t)(lt - p));
    char const *end = tag_end(lt + 1);
    if (!end) {
      put(&out, lt);
      break;
    }
    if (lt[1] == '/' || lt[1] == '!' || lt[1] == '?') {
      putn(&out, lt, (size_t)(end + 1 - lt));
      p = end + 1;
      continue;
    }

    char const *name = lt + 1;
    char const *name_end = name;
    while (name_char(*name_end)) name_end++;

    char *type = copy(name, (size_t)(name_end - name));
    struct ClassDesc const *cls = type ? OBJ_FindClass(type) : NULL;
    free(type);
    if (!cls) {
      putn(&out, lt, (size_t)(end + 1 - lt));
      p = end + 1;
      continue;
    }

    char const *body = name_end;
    char const *body_end = end;
    bool_t self_close = FALSE;
    while (body_end > body && isspace((unsigned char)body_end[-1])) body_end--;
    if (body_end > body && body_end[-1] == '/') {
      self_close = TRUE;
      body_end--;
      while (body_end > body && isspace((unsigned char)body_end[-1])) body_end--;
    }

    char const *tok = body;
    while (tok < body_end && isspace((unsigned char)*tok)) tok++;
    char const *tok_end = tok;
    while (tok_end < body_end && !isspace((unsigned char)*tok_end) && *tok_end != '=') tok_end++;
    char const *rest = tok_end;
    while (rest < body_end && isspace((unsigned char)*rest)) rest++;

    int cursor = 0;
    struct PropertyType const *field = next_field(cls->Properties, (int)cls->NumProperties, NULL, &cursor);
    if (tok == tok_end || (rest < body_end && *rest == '=') || !field) {
      putn(&out, lt, (size_t)(end + 1 - lt));
      p = end + 1;
      continue;
    }

    putc_xml(&out, '<');
    putn(&out, name, (size_t)(name_end - name));
    putc_xml(&out, ' ');
    put(&out, field->Name);
    put(&out, "=\"");
    char *value = copy(tok, (size_t)(tok_end - tok));
    if (value) {
      put_escaped(&out, value);
      free(value);
    }
    putc_xml(&out, '"');
    if (rest < body_end) {
      putc_xml(&out, ' ');
      putn(&out, rest, (size_t)(body_end - rest));
    }
    put(&out, self_close ? "/>" : ">");
    p = end + 1;
  }

  return out.s;
}
