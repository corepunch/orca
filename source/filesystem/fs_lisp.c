/* Lisp scene reader. Lists describe nodes and expressions; assignments attach
   values to named properties. Lower to the format-neutral object-builder tree, preserving the engine
   prefab, controller, property and lifecycle behavior. */
#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <ctype.h>

#define LISP_MAX_DEPTH 128
#define LISP_MAX_ARGS TOKEN_MAX_ARGS

enum lisp_kind { LISP_ATOM, LISP_STRING, LISP_LIST, LISP_ASSIGN };
struct lisp_value {
  enum lisp_kind kind;
  char *text;
  struct lisp_value *items, *next;
};
struct lisp_reader { const char *p; int line; bool_t failed; };

static void
lisp_error(struct lisp_reader *r, const char *message)
{
  if (!r->failed) Con_Error("Lisp line %d: %s", r->line, message);
  r->failed = TRUE;
}

static void
lisp_free(struct lisp_value *v)
{
  while (v) {
    struct lisp_value *next = v->next;
    lisp_free(v->items);
    free(v->text);
    free(v);
    v = next;
  }
}

static void
lisp_space(struct lisp_reader *r)
{
  for (;;) {
    while (isspace((unsigned char)*r->p)) {
      if (*r->p++ == '\n') r->line++;
    }
    if (*r->p != ';') return;
    while (*r->p && *r->p != '\n') r->p++;
  }
}

static struct lisp_value *
lisp_read(struct lisp_reader *r, int depth)
{
  lisp_space(r);
  if (depth > LISP_MAX_DEPTH || !*r->p || *r->p == ')') {
    lisp_error(r, "expected value, or nesting limit exceeded");
    return NULL;
  }
  struct lisp_value *v = calloc(1, sizeof(*v));
  if (!v) { lisp_error(r, "out of memory"); return NULL; }
  if (*r->p == '(') {
    v->kind = LISP_LIST;
    r->p++;
    struct lisp_value **tail = &v->items;
    lisp_space(r);
    while (*r->p && *r->p != ')' && !r->failed) {
      *tail = lisp_read(r, depth + 1);
      if (*tail) tail = &(*tail)->next;
      lisp_space(r);
    }
    if (*r->p != ')') lisp_error(r, "missing ')'");
    else r->p++;
    if (!v->items || v->items->kind != LISP_ATOM)
      lisp_error(r, "list must start with a type or function name");
  } else if (*r->p == '"') {
    v->kind = LISP_STRING;
    const char *start = ++r->p;
    const char *end = start;
    while (*end && *end != '"') {
      if (*end == '\\' && end[1]) end++;
      end++;
    }
    v->text = malloc((size_t)(end - start) + 1);
    if (!v->text) { lisp_error(r, "out of memory"); lisp_free(v); return NULL; }
    char *out = v->text;
    while (*r->p && *r->p != '"' && !r->failed) {
      char ch = *r->p++;
      if (ch == '\n') r->line++;
      if (ch == '\\') {
        ch = *r->p++;
        switch (ch) {
          case 'n': ch = '\n'; break;
          case 'r': ch = '\r'; break;
          case 't': ch = '\t'; break;
          case '\\': case '"': break;
          default: lisp_error(r, "invalid string escape"); break;
        }
        if (!ch) { r->p--; break; }
      }
      *out++ = ch;
    }
    *out = 0;
    if (*r->p != '"') lisp_error(r, "unterminated string");
    else r->p++;
  } else {
    const char *start = r->p;
    while (*r->p && !isspace((unsigned char)*r->p) && !strchr("()\";=", *r->p)) r->p++;
    if (start == r->p) lisp_error(r, "expected atom");
    v->text = strndup(start, (size_t)(r->p - start));
    if (!v->text) lisp_error(r, "out of memory");
    lisp_space(r);
    if (*r->p == '=' && !r->failed) {
      r->p++;
      v->kind = LISP_ASSIGN;
      v->items = lisp_read(r, depth + 1);
      if (v->items && v->items->kind == LISP_ASSIGN)
        lisp_error(r, "assignment requires a value");
    }
  }
  if (r->failed) { lisp_free(v); return NULL; }
  return v;
}

/* Binding functions are explicit, keeping an unknown node type from silently
   becoming an executable expression. The existing VM owns evaluation. */
static const char *
lisp_function(const char *name)
{
  static const char *names[] = {
    "bind", "if", "step", "bool", "vector2", "vector3", "vector4",
    "color4", "add", "sub", "mul", "div", "pow", "min", "max",
    "abs", "floor", "ceil", "round", "sqrt", "clamp", "mix", "scale",
    "linearstep", "and", "or", "equal", "unm", "mod", "rem", "string",
    "float", "int", "vector3x", "vector3y", "vector3z", "colorr", "colorg",
    "colorb", "colora", NULL
  };
  for (int i = 0; names[i]; i++) if (!strcasecmp(name, names[i])) return names[i];
  if (!strcmp(name, "+")) return "add";
  if (!strcmp(name, "-")) return "sub";
  if (!strcmp(name, "*")) return "mul";
  if (!strcmp(name, "/")) return "div";
  return NULL;
}

static void
lisp_put(xmlBufferPtr b, const char *text)
{
  xmlBufferCat(b, XMLSTR(text));
}

static bool_t
lisp_expression(struct lisp_reader *r, struct lisp_value *v, xmlBufferPtr b)
{
  if (v->kind == LISP_STRING) {
    /* The legacy property VM cannot represent embedded quotes in literals. */
    if (strchr(v->text, '"')) {
      lisp_error(r, "binding string literals cannot contain quotes");
      return FALSE;
    }
    lisp_put(b, "\""); lisp_put(b, v->text); lisp_put(b, "\"");
    return TRUE;
  }
  if (v->kind == LISP_ATOM) {
    if (!strcmp(v->text, "true") || !strcmp(v->text, "false")) {
      lisp_put(b, !strcmp(v->text, "true") ? "1" : "0");
      return TRUE;
    }
    const char *p = v->text;
    if (*p == '-') p++;
    bool_t digit = FALSE, dot = FALSE;
    for (; *p; p++) {
      if (isdigit((unsigned char)*p)) digit = TRUE;
      else if (*p == '.' && !dot) dot = TRUE;
      else break;
    }
    if (*p || !digit) { lisp_error(r, "expected a binding number, boolean or string"); return FALSE; }
    lisp_put(b, v->text);
    return TRUE;
  }
  if (v->kind != LISP_LIST) { lisp_error(r, "invalid binding argument"); return FALSE; }
  const char *fn = lisp_function(v->items->text);
  if (!fn) { lisp_error(r, "unknown binding function"); return FALSE; }
  struct lisp_value *arg = v->items->next;
  if (!strcmp(fn, "bind")) {
    if (!arg || arg->next || arg->kind != LISP_STRING || !*arg->text ||
        isdigit((unsigned char)*arg->text) || isspace((unsigned char)*arg->text) || strpbrk(arg->text, "{}\n\r")) {
      lisp_error(r, "bind expects one quoted property path"); return FALSE;
    }
    if (arg->text[0] == '#' && (!strchr(arg->text, '/') || !strchr(arg->text, '/')[1])) {
      lisp_error(r, "named binding paths require an object and property separated by '/'");
      return FALSE;
    }
    lisp_put(b, "{"); lisp_put(b, arg->text); lisp_put(b, "}");
    return TRUE;
  }
  lisp_put(b, fn); lisp_put(b, "(");
  int count = 0;
  for (; arg; arg = arg->next) {
    if (++count > LISP_MAX_ARGS) { lisp_error(r, "too many binding arguments"); return FALSE; }
    if (count > 1) lisp_put(b, ",");
    if (!lisp_expression(r, arg, b)) return FALSE;
  }
  int expected = 2;
  if (!strcmp(fn, "if") || !strcmp(fn, "vector3") || !strcmp(fn, "clamp") ||
      !strcmp(fn, "mix") || !strcmp(fn, "scale") || !strcmp(fn, "linearstep")) expected = 3;
  else if (!strcmp(fn, "vector4") || !strcmp(fn, "color4")) expected = 4;
  else if (!strcmp(fn, "bool") || !strcmp(fn, "abs") || !strcmp(fn, "floor") ||
           !strcmp(fn, "ceil") || !strcmp(fn, "round") || !strcmp(fn, "sqrt") ||
           !strcmp(fn, "unm") || !strcmp(fn, "string") || !strcmp(fn, "float") ||
           !strcmp(fn, "int") || !strncmp(fn, "vector3", 7) || !strncmp(fn, "color", 5)) expected = 1;
  if (count != expected) { lisp_error(r, "wrong number of binding arguments"); return FALSE; }
  lisp_put(b, ")");
  return TRUE;
}

static xmlNodePtr lisp_tree(struct lisp_reader *r, struct lisp_value *v);

static void
lisp_property(struct lisp_reader *r, xmlNodePtr node, struct lisp_value *v)
{
  if (xmlHasProp(node, XMLSTR(v->text))) {
    lisp_error(r, "duplicate property"); return;
  }
  for (xmlNodePtr c = node->children; c; c = c->next) {
    if (c->type == XML_ELEMENT_NODE && !strcmp((char *)c->name, v->text)) {
      lisp_error(r, "duplicate property"); return;
    }
  }
  struct lisp_value *value = v->items;
  if (value->kind != LISP_LIST) {
    xmlSetProp(node, XMLSTR(v->text), XMLSTR(value->text));
    return;
  }
  const char *head = value->items->text;
  if (lisp_function(head)) {
    xmlBufferPtr expr = xmlBufferCreate();
    if (!expr) { lisp_error(r, "out of memory"); return; }
    if (lisp_expression(r, value, expr)) {
      xmlNodePtr binding = xmlNewChild(node, NULL, XMLSTR("BindingExpression"), NULL);
      if (!binding || !xmlSetProp(binding, XMLSTR("Target"), XMLSTR(v->text)))
        lisp_error(r, "out of memory");
      else xmlAddChild(binding, xmlNewText(xmlBufferContent(expr)));
    }
    xmlBufferFree(expr);
  } else {
    xmlNodePtr property = xmlNewChild(node, NULL, XMLSTR(v->text), NULL);
    if (!property) { lisp_error(r, "out of memory"); return; }
    property->_private = (void *)1;
    if (!strcmp(head, "list")) {
    for (struct lisp_value *item = value->items->next; item && !r->failed; item = item->next) {
      xmlNodePtr child = lisp_tree(r, item);
      if (child) xmlAddChild(property, child);
    }
  } else {
    xmlNodePtr child = lisp_tree(r, value);
    if (child) xmlAddChild(property, child);
    }
  }
}

static xmlNodePtr
lisp_tree(struct lisp_reader *r, struct lisp_value *v)
{
  if (v->kind != LISP_LIST) { lisp_error(r, "expected a node list"); return NULL; }
  xmlNodePtr node = xmlNewNode(NULL, XMLSTR(v->items->text));
  if (!node) { lisp_error(r, "out of memory"); return NULL; }
  for (struct lisp_value *item = v->items->next; item && !r->failed; item = item->next) {
    if (item->kind == LISP_ASSIGN) {
      for (struct lisp_value *prev = v->items->next; prev != item; prev = prev->next) {
        if (prev->kind == LISP_ASSIGN && !strcmp(prev->text, item->text))
          lisp_error(r, "duplicate property");
      }
      if (!r->failed) lisp_property(r, node, item);
    }
    else if (item->kind == LISP_LIST) {
      xmlNodePtr child = lisp_tree(r, item);
      if (child) xmlAddChild(node, child);
    } else if (item->kind == LISP_STRING) {
      xmlAddChild(node, xmlNewText(XMLSTR(item->text)));
    } else lisp_error(r, "node arguments must be assignments, child nodes or quoted text");
  }
  if (r->failed) { xmlFreeNode(node); return NULL; }
  return node;
}

static struct Object *
lisp_load(const char *source, const char *path)
{
  if (!source) return NULL;
  struct lisp_reader r = { .p = source, .line = 1 };
  struct lisp_value *value = lisp_read(&r, 0);
  lisp_space(&r);
  if (*r.p) lisp_error(&r, "unexpected content after root node");
  xmlNodePtr root = !r.failed && value ? lisp_tree(&r, value) : NULL;
  struct Object *o = root ? FS_BuildObjectTree(root, path) : NULL;
  if (root) xmlFreeNode(root);
  lisp_free(value);
  return o;
}

ORCA_API struct Object *
FS_LoadObjectFromLispString(lpcString_t source)
{
  return lisp_load(source, NULL);
}

ORCA_API struct Object *
FS_LoadObjectFromLisp(lpcString_t path)
{
  struct file *fp = FS_LoadFile(path);
  if (!fp) return NULL;
  if (memchr(fp->data, 0, fp->size)) {
    Con_Error("Lisp file '%s' contains a NUL byte", path);
    FS_FreeFile(fp);
    return NULL;
  }
  char *source = strndup((char const *)fp->data, fp->size);
  FS_FreeFile(fp);
  struct Object *o = source ? lisp_load(source, path) : NULL;
  free(source);
  return o;
}

static xmlNodePtr
lisp_read_file_tree(lpcString_t path)
{
  struct file *fp = FS_LoadFile(path);
  if (!fp) return NULL;
  char *source = memchr(fp->data, 0, fp->size) ? NULL : strndup((char const *)fp->data, fp->size);
  FS_FreeFile(fp);
  if (!source) return NULL;
  struct lisp_reader r = { .p = source, .line = 1 };
  struct lisp_value *value = lisp_read(&r, 0);
  lisp_space(&r);
  if (*r.p) lisp_error(&r, "unexpected content after root node");
  xmlNodePtr root = !r.failed && value ? lisp_tree(&r, value) : NULL;
  lisp_free(value);
  free(source);
  return root;
}

struct Object *
FS_LoadObjectFromLispLibrary(lpcString_t path)
{
  const char *slash = strrchr(path, '/');
  path_t file;
  if (!slash || !slash[1]) return NULL;
  int n = snprintf(file, sizeof(file), "%.*s.lisp", (int)(slash - path), path);
  if (n <= 0 || n >= (int)sizeof(file) || !FS_FileExists(file)) return NULL;
  xmlNodePtr root = lisp_read_file_tree(file);
  struct Object *object = NULL;
  for (xmlNodePtr child = root ? root->children : NULL; child; child = child->next) {
    xmlChar *name = child->type == XML_ELEMENT_NODE ? xmlGetProp(child, XMLSTR("Name")) : NULL;
    bool_t match = name && !strcmp((char *)name, slash + 1);
    if (name) xmlFree(name);
    if (match) { object = FS_BuildObjectTree(child, path); break; }
  }
  if (root) xmlFreeNode(root);
  return object;
}

void
FS_EnumLispLibrary(lpcString_t path, void (*fn)(lpcString_t name, void *arg), void *arg)
{
  if (!FS_FileExists(path)) return;
  xmlNodePtr root = lisp_read_file_tree(path);
  /* A standalone family also publishes its declared alias. */
  bool_t standalone = root && !strcmp((char const *)root->name, "FontFamily");
  for (xmlNodePtr child = root ? (standalone ? root : root->children) : NULL;
       child; child = standalone ? NULL : child->next) {
    xmlChar *name = child->type == XML_ELEMENT_NODE ? xmlGetProp(child, XMLSTR("Name")) : NULL;
    if (name && *name) fn((char const *)name, arg);
    if (name) xmlFree(name);
  }
  if (root) xmlFreeNode(root);
}
