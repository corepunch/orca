/* XML scene reader, included only by fs_scene.c in XML builds. */
struct xml_scene_reader { const char *p, *end; bool_t failed; };

static void
xml_scene_space(struct xml_scene_reader *r)
{
  while (r->p < r->end && (isspace((unsigned char)*r->p) || *r->p == ',')) r->p++;
}

/* Tokenizer for legacy {Type positional Name=value} shorthand only. */
static char *
xml_scene_token(struct xml_scene_reader *r)
{
  xml_scene_space(r);
  if (r->p == r->end) return NULL;
  const char *start = r->p;
  if (*r->p == '\'' || *r->p == '"') {
    char quote = *r->p++;
    start = r->p;
    while (r->p < r->end && *r->p != quote) r->p++;
    if (r->p == r->end) { r->failed = TRUE; return NULL; }
    char *value = strndup(start, (size_t)(r->p - start));
    r->p++;
    if (!value) r->failed = TRUE;
    return value;
  }
  if (*r->p == '{') {
    int depth = 0;
    char quote = 0;
    while (r->p < r->end) {
      char c = *r->p++;
      if (quote) { if (c == quote) quote = 0; continue; }
      if (c == '\'' || c == '"') quote = c;
      else if (c == '{') { if (++depth > 128) break; }
      else if (c == '}' && --depth == 0) return strndup(start, (size_t)(r->p - start));
    }
    r->failed = TRUE;
    return NULL;
  }
  while (r->p < r->end && !isspace((unsigned char)*r->p) && !strchr(",=}", *r->p)) r->p++;
  if (r->p == start) { r->failed = TRUE; return NULL; }
  char *value = strndup(start, (size_t)(r->p - start));
  if (!value) r->failed = TRUE;
  return value;
}

static bool_t
xml_scene_fields(xmlNodePtr node, struct xml_scene_reader *r,
                 struct PropertyType const *fields, unsigned count)
{
  bool_t *used = count ? calloc(count, sizeof(*used)) : NULL;
  if (count && !used) return FALSE;
  unsigned cursor = 0;
  while (!r->failed) {
    char *name = xml_scene_token(r);
    if (!name) break;
    char *value = NULL;
    xml_scene_space(r);
    if (r->p < r->end && *r->p == '=') {
      r->p++;
      value = xml_scene_token(r);
      for (unsigned i = 0; i < count; i++)
        if (!strcmp(fields[i].Name, name)) used[i] = TRUE;
    } else {
      value = name;
      name = NULL;
      while (cursor < count && (used[cursor] ||
          (fields[cursor].DataType == kDataTypeObject && !strcmp(fields[cursor].Name, "Trigger")))) cursor++;
      if (cursor < count) { name = strdup(fields[cursor].Name); used[cursor++] = TRUE; }
    }
    if (!name || !value || xmlHasProp(node, XMLSTR(name)) ||
        !xmlSetProp(node, XMLSTR(name), XMLSTR(value))) r->failed = TRUE;
    free(name);
    free(value);
  }
  free(used);
  return !r->failed;
}

/* Convert nonstandard positional tags before libxml reads the document. This
   is attempted only when ordinary XML parsing fails. Valid XML is untouched. */
static char *
xml_scene_expand_positional(const char *source)
{
  xmlBufferPtr out = xmlBufferCreate();
  if (!out) return NULL;
  const char *p = source;
  bool_t valid = TRUE;
  while (*p && valid) {
    const char *lt = strchr(p, '<');
    if (!lt) { xmlBufferCat(out, XMLSTR(p)); break; }
    xmlBufferAdd(out, XMLSTR(p), (int)(lt - p));
    const char *end = lt + 1;
    char quote = 0;
    for (; *end; end++) {
      if (quote) { if (*end == quote) quote = 0; }
      else if (*end == '\'' || *end == '"') quote = *end;
      else if (*end == '>') break;
    }
    if (!*end) { valid = FALSE; break; }
    const char *head = lt + 1, *head_end = head;
    while (isalnum((unsigned char)*head_end) || strchr("_-.:", *head_end)) {
      if (!*head_end) break;
      head_end++;
    }
    char *type = strndup(head, (size_t)(head_end - head));
    struct ClassDesc const *cls = type ? OBJ_FindClass(type) : NULL;
    bool_t quoted = FALSE;
    for (const char *it = head_end; it < end; it++) if (*it == '\'' || *it == '"') quoted = TRUE;
    if (!cls || quoted || *head == '/' || *head == '!' || *head == '?') {
      xmlBufferAdd(out, XMLSTR(lt), (int)(end + 1 - lt));
    } else {
      const char *body_end = end;
      while (body_end > head_end && isspace((unsigned char)body_end[-1])) body_end--;
      bool_t closed = body_end > head_end && body_end[-1] == '/';
      if (closed) body_end--;
      struct xml_scene_reader r = { .p = head_end, .end = body_end };
      xmlNodePtr node = xmlNewNode(NULL, XMLSTR(type));
      xmlBufferPtr tag = xmlBufferCreate();
      valid = node && tag && xml_scene_fields(node, &r, cls->Properties, cls->NumProperties);
      if (valid) {
        xmlNodeDump(tag, NULL, node, 0, 0);
        size_t n = xmlBufferLength(tag);
        const xmlChar *text = xmlBufferContent(tag);
        if (closed) xmlBufferAdd(out, text, (int)n);
        else if (n >= 2 && text[n - 2] == '/') {
          xmlBufferAdd(out, text, (int)n - 2);
          xmlBufferCat(out, XMLSTR(">"));
        } else xmlBufferAdd(out, text, (int)n);
      }
      if (tag) xmlBufferFree(tag);
      if (node) xmlFreeNode(node);
    }
    free(type);
    p = end + 1;
  }
  char *result = valid ? strdup((char const *)xmlBufferContent(out)) : NULL;
  xmlBufferFree(out);
  return result;
}

static xmlDocPtr
xml_scene_document(const char *source, size_t length, const char *path)
{
  if (!source || length > INT_MAX || memchr(source, 0, length)) return NULL;
  int flags = XML_FLAGS | XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING;
  xmlDocPtr doc = xmlReadMemory(source, (int)length, path, NULL, flags);
  if (!doc) {
    char *expanded = xml_scene_expand_positional(source);
    if (expanded) {
      doc = xmlReadMemory(expanded, (int)strlen(expanded), path, NULL, flags);
      free(expanded);
    }
  }
  if (doc) doc->_private = (void *)FS_XML_SCENE_DOCUMENT;
  else Con_Error("Failed to parse XML scene '%s'", path ? path : "<string>");
  return doc;
}

static xmlDocPtr
xml_scene_read_file(const char *path)
{
  struct file *file = FS_LoadFile(path);
  if (!file) return NULL;
  /* The legacy positional scanner expects a terminator. */
  char *source = memchr(file->data, 0, file->size) ? NULL : strndup(file->data, file->size);
  xmlDocPtr doc = source ? xml_scene_document(source, file->size, path) : NULL;
  free(source);
  FS_FreeFile(file);
  return doc;
}

static xmlNodePtr
xml_scene_inline(const char *text, struct StructDesc const *sd)
{
  size_t n = strlen(text);
  if (*text == '<') {
    xmlDocPtr doc = xml_scene_document(text, n, NULL);
    xmlNodePtr root = doc ? xmlDocGetRootElement(doc) : NULL;
    xmlNodePtr copy = root ? xmlCopyNode(root, 1) : NULL;
    if (doc) xmlFreeDoc(doc);
    return copy;
  }
  if (n < 2 || text[0] != '{' || text[n - 1] != '}') return NULL;
  struct xml_scene_reader r = { .p = text + 1, .end = text + n - 1 };
  char *type = sd ? strdup(sd->StructName) : xml_scene_token(&r);
  struct ClassDesc const *cls = !sd && type ? OBJ_FindClass(type) : NULL;
  xmlNodePtr node = type && (sd || cls) ? xmlNewNode(NULL, XMLSTR(type)) : NULL;
  bool_t valid = node && xml_scene_fields(node, &r, sd ? sd->Properties : cls->Properties,
                                        sd ? sd->NumProperties : cls->NumProperties);
  free(type);
  if (!valid && node) { xmlFreeNode(node); node = NULL; }
  return node;
}

static char *
xml_scene_trim(char *s)
{
  while (isspace((unsigned char)*s)) s++;
  char *end = s + strlen(s);
  while (end > s && isspace((unsigned char)end[-1])) *--end = 0;
  return s;
}

static xmlNodePtr
xml_scene_binding(const char *target, const char *text)
{
  size_t n = strlen(text);
  if (n <= 10 || text[n - 1] != '}') return NULL;
  char *body = strndup(text + 9, n - 10);
  if (!body) return NULL;
  char *save = NULL, *path = strtok_r(body, ",", &save), *source = NULL;
  const char *mode = "OneWay";
  bool_t valid = path != NULL;
  if (path) {
    path = xml_scene_trim(path);
    if (!strncmp(path, "Path=", 5)) path = xml_scene_trim(path + 5);
    valid = *path != 0;
  }
  for (char *opt = strtok_r(NULL, ",", &save); opt && valid; opt = strtok_r(NULL, ",", &save)) {
    opt = xml_scene_trim(opt);
    char *eq = strchr(opt, '=');
    if (!eq) { valid = FALSE; break; }
    *eq++ = 0;
    eq = xml_scene_trim(eq);
    if (!strcmp(xml_scene_trim(opt), "Source")) source = eq;
    else if (!strcmp(opt, "Mode") && (!strcmp(eq, "OneWay") || !strcmp(eq, "TwoWay") ||
             !strcmp(eq, "OneWayToSource") || !strcmp(eq, "Expression"))) mode = eq;
    else valid = FALSE;
  }
  xmlNodePtr binding = valid ? xmlNewNode(NULL, XMLSTR("BindingExpression")) : NULL;
  if (binding) {
    xmlBufferPtr expr = xmlBufferCreate();
    if (!expr) { xmlFreeNode(binding); free(body); return NULL; }
    xmlBufferCat(expr, XMLSTR("{"));
    xmlBufferCat(expr, XMLSTR(path));
    if (source) { xmlBufferCat(expr, XMLSTR(", Source=")); xmlBufferCat(expr, XMLSTR(source)); }
    xmlBufferCat(expr, XMLSTR("}"));
    valid = xmlSetProp(binding, XMLSTR("Target"), XMLSTR(target)) &&
            xmlSetProp(binding, XMLSTR("Mode"), XMLSTR(mode));
    if (valid) xmlNodeSetContent(binding, xmlBufferContent(expr));
    xmlBufferFree(expr);
    if (!valid) { xmlFreeNode(binding); binding = NULL; }
  }
  free(body);
  return binding;
}

/* Called after prefab/class creation, so inherited properties and aliases are
   resolved against the actual object rather than guessed from the XML tag. */
static bool_t
FS_NormalizeXmlSceneAttributes(struct Object *object, xmlNodePtr node)
{
  if (!node->doc || node->doc->_private != (void *)FS_XML_SCENE_DOCUMENT) return TRUE;
  for (xmlAttrPtr attr = node->properties, next; attr; attr = next) {
    next = attr->next;
    xmlChar *text = xmlGetProp(node, attr->name);
    if (!text) return FALSE;
    char *key = strdup((char const *)attr->name);
    if (!key) { xmlFree(text); return FALSE; }
    key[0] = (char)toupper((unsigned char)key[0]);
    struct PropertyType const *pd = OBJ_FindImplicitPropertyType(object, key);
    if (!pd) pd = OBJ_FindExplicitPropertyType(object, key);
    struct PropertyShorthand const *sh = OBJ_FindImplicitShorthand(object, key);
    if (!sh) sh = OBJ_FindExplicitShorthand(object, key);
    const char *type = pd && pd->DataType == kDataTypeStruct ? pd->TypeString : sh ? sh->TypeString : NULL;
    struct StructDesc const *sd = type ? OBJ_FindStructDesc(type) : NULL;
    bool_t binding = !strncmp((char *)text, "{Binding ", 9);
    bool_t binding_object = !strncmp((char *)text, "{BindingExpression ", 19) ||
                            !strncmp((char *)text, "<Binding", 8);
    bool_t inline_value = (*text == '{' || *text == '<') &&
      (sd || (pd && (pd->DataType == kDataTypeObject || pd->DataType == kDataTypeEvent)) ||
       binding_object);
    if (!binding && !inline_value) { free(key); xmlFree(text); continue; }
    xmlNodePtr value = binding ? xml_scene_binding(key, (char *)text)
                               : xml_scene_inline((char *)text, binding_object ? NULL : sd);
    xmlFree(text);
    if (!value) { free(key); return Con_Error("Invalid XML value for '%s'", attr->name), FALSE; }
    if (binding || !strcmp((char *)value->name, "Binding") || !strcmp((char *)value->name, "BindingExpression")) {
      if (!xmlSetProp(value, XMLSTR("Target"), XMLSTR(key))) {
        free(key); xmlFreeNode(value); return FALSE;
      }
      xmlAddChild(node, value);
    } else {
      xmlNodePtr property = xmlNewChild(node, NULL, XMLSTR(key), NULL);
      if (!property) { free(key); xmlFreeNode(value); return FALSE; }
      property->_private = (void *)1; /* named property, even when a class has the same name */
      xmlAddChild(property, value);
    }
    free(key);
    xmlRemoveProp(attr);
  }
  return TRUE;
}

static struct Object *
xml_scene_build(xmlDocPtr doc, xmlNodePtr root, const char *path)
{
  struct Object *object = root ? FS_BuildObjectTree(root, path) : NULL;
  if (doc) xmlFreeDoc(doc);
  return object;
}

ORCA_API struct Object *
FS_LoadObjectFromXmlString(lpcString_t source)
{
  xmlDocPtr doc = source ? xml_scene_document(source, strlen(source), NULL) : NULL;
  return xml_scene_build(doc, doc ? xmlDocGetRootElement(doc) : NULL, NULL);
}

ORCA_API struct Object *
FS_LoadObjectFromXml(lpcString_t path)
{
  xmlDocPtr doc = xml_scene_read_file(path);
  return xml_scene_build(doc, doc ? xmlDocGetRootElement(doc) : NULL, path);
}

struct Object *
FS_LoadObjectFromXmlLibrary(lpcString_t path)
{
  const char *slash = strrchr(path, '/');
  path_t file;
  if (!slash || !slash[1]) return NULL;
  int n = snprintf(file, sizeof(file), "%.*s.xml", (int)(slash - path), path);
  if (n <= 0 || n >= (int)sizeof(file) || !FS_FileExists(file)) return NULL;
  xmlDocPtr doc = xml_scene_read_file(file);
  xmlNodePtr root = doc ? xmlDocGetRootElement(doc) : NULL, entry = NULL;
  for (xmlNodePtr child = root ? root->children : NULL; child; child = child->next) {
    xmlChar *name = child->type == XML_ELEMENT_NODE ? xmlGetProp(child, XMLSTR("Name")) : NULL;
    bool_t match = name && !strcmp((char *)name, slash + 1);
    if (name) xmlFree(name);
    if (match) { entry = child; break; }
  }
  return xml_scene_build(doc, entry, path);
}

void
FS_EnumXmlLibrary(lpcString_t path, void (*fn)(lpcString_t name, void *arg), void *arg)
{
  if (!FS_FileExists(path)) return;
  xmlDocPtr doc = xml_scene_read_file(path);
  xmlNodePtr root = doc ? xmlDocGetRootElement(doc) : NULL;
  bool_t standalone = root && !strcmp((char *)root->name, "FontFamily");
  for (xmlNodePtr child = root ? (standalone ? root : root->children) : NULL;
       child; child = standalone ? NULL : child->next) {
    xmlChar *name = child->type == XML_ELEMENT_NODE ? xmlGetProp(child, XMLSTR("Name")) : NULL;
    if (name && *name) fn((char *)name, arg);
    if (name) xmlFree(name);
  }
  if (doc) xmlFreeDoc(doc);
}
