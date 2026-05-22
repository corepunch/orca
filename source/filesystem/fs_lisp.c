/*
 * fs_lisp.c — Lisp-format UI asset loader.
 *
 * Syntax overview:
 *   (TypeName Key="val" Key=(binding-expr)
 *     (:bind Target="Prop" expr)
 *     (ChildType ...)
 *     "text content")
 *
 * Binding expressions are translated from Lisp to the ORCA expression string
 * recognised by Token_Create / OBJ_AttachPropertyProgram:
 *   (bind "Node.ActualWidth")      → {Node.ActualWidth}
 *   (if  a b c)                    → IF(a, b, c)
 *   (step n x)                     → STEP(n, x)
 *   (vector2 x y)                  → Vector2(x, y)
 *   Any other (fname ...)          → FNAME(...)  (uppercased)
 */

#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <source/core/core_properties.h>
#include "fs_xml_inline.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Forward declaration of the property parser from parsers module */
extern int parse_property(const char *str,
                          struct PropertyType const *prop,
                          void *valueptr);

/* =========================================================================
 * String buffer
 * ====================================================================== */

struct sbuf { char *s; size_t n, cap; };

static void
sbuf_put(struct sbuf *b, const char *s)
{
  if (!s || !*s) return;
  size_t len = strlen(s);
  if (b->n + len + 1 > b->cap) {
    size_t need = b->n + len + 1;
    b->cap = b->cap ? b->cap * 2 : 256;
    if (b->cap < need) b->cap = need;
    b->s = realloc(b->s, b->cap);
  }
  if (!b->s) return;
  memcpy(b->s + b->n, s, len);
  b->n += len;
  b->s[b->n] = '\0';
}

static void
sbuf_putc(struct sbuf *b, char c)
{
  char tmp[2] = {c, '\0'};
  sbuf_put(b, tmp);
}

/* =========================================================================
 * Lexer
 * ====================================================================== */

enum lsp_tok {
  TOK_EOF    = 0,
  TOK_LPAREN,   /* ( */
  TOK_RPAREN,   /* ) */
  TOK_STRING,   /* "..." */
  TOK_ATOM,     /* anything else */
};

struct lsp_lex {
  const char *p;
  char       *str;   /* heap-allocated text for current STRING / ATOM token */
  int         tok;
  int         line;
};

static void
lsp_skip(struct lsp_lex *l)
{
  for (;;) {
    while (*l->p && isspace((unsigned char)*l->p)) {
      if (*l->p == '\n') l->line++;
      l->p++;
    }
    if (*l->p == ';') {
      while (*l->p && *l->p != '\n') l->p++;
    } else {
      break;
    }
  }
}

static void
lsp_next(struct lsp_lex *l)
{
  free(l->str);
  l->str = NULL;
  lsp_skip(l);

  if (!*l->p)       { l->tok = TOK_EOF;    return; }
  if (*l->p == '(') { l->tok = TOK_LPAREN; l->p++; return; }
  if (*l->p == ')') { l->tok = TOK_RPAREN; l->p++; return; }

  if (*l->p == '"') {
    l->p++;
    struct sbuf b = {0};
    while (*l->p && *l->p != '"') {
      if (*l->p == '\\' && l->p[1]) {
        l->p++;
        switch (*l->p) {
          case 'n':  sbuf_putc(&b, '\n'); break;
          case 't':  sbuf_putc(&b, '\t'); break;
          case '"':  sbuf_putc(&b, '"');  break;
          case '\\': sbuf_putc(&b, '\\'); break;
          default:   sbuf_putc(&b, '\\'); sbuf_putc(&b, *l->p); break;
        }
      } else {
        sbuf_putc(&b, *l->p);
      }
      l->p++;
    }
    if (*l->p == '"') l->p++;
    l->str = b.s ? b.s : strdup("");
    if (!l->str) { l->tok = TOK_EOF; return; }
    l->tok = TOK_STRING;
    return;
  }

  /* Atom: up to whitespace, parens, quotes, semicolons */
  const char *start = l->p;
  while (*l->p && !isspace((unsigned char)*l->p) &&
         *l->p != '(' && *l->p != ')' && *l->p != '"' && *l->p != ';') {
    l->p++;
  }
  size_t n = (size_t)(l->p - start);
  l->str = malloc(n + 1);
  if (!l->str) { l->tok = TOK_EOF; return; }
  memcpy(l->str, start, n);
  l->str[n] = '\0';
  l->tok = TOK_ATOM;
}

/* =========================================================================
 * Binding expression translation: Lisp form → ORCA expression string
 *
 * translate_expr(l, out):
 *   Pre:  l->tok is the first token of an expression.
 *   Post: l->tok is the token AFTER the expression.
 *
 * translate_list_body(l, out):
 *   Pre:  '(' was just consumed and lsp_next was called → l->tok is the
 *         function-name atom.
 *   Post: l->tok is the token AFTER the closing ')'.
 * ====================================================================== */

static void translate_expr(struct lsp_lex *l, struct sbuf *out);

static void
translate_list_body(struct lsp_lex *l, struct sbuf *out)
{
  if (l->tok != TOK_ATOM) {
    Con_Error("Lisp binding: expected function name in list (line %d)", l->line);
    return;
  }

  char fname[64];
  snprintf(fname, sizeof(fname), "%s", l->str);
  lsp_next(l);

  /* (bind "PropPath") → {PropPath} */
  if (!strcasecmp(fname, "bind")) {
    if (l->tok == TOK_STRING) {
      sbuf_putc(out, '{');
      sbuf_put(out, l->str);
      sbuf_putc(out, '}');
      lsp_next(l);
    } else {
      Con_Error("Lisp (bind) expects a string path (line %d)", l->line);
    }
    if (l->tok == TOK_RPAREN) lsp_next(l);
    return;
  }

  /* General function: UPPERCASE(arg, arg, ...) */
  char orca_name[64];
  size_t flen = strlen(fname);
  size_t cap = sizeof(orca_name) - 1;
  for (size_t i = 0; i < flen && i < cap; i++)
    orca_name[i] = (char)toupper((unsigned char)fname[i]);
  orca_name[flen < cap ? flen : cap] = '\0';

  sbuf_put(out, orca_name);
  sbuf_putc(out, '(');

  bool_t first = TRUE;
  while (l->tok != TOK_RPAREN && l->tok != TOK_EOF) {
    if (!first) sbuf_put(out, ", ");
    first = FALSE;
    translate_expr(l, out);
  }

  sbuf_putc(out, ')');
  if (l->tok == TOK_RPAREN) lsp_next(l);
}

static void
translate_expr(struct lsp_lex *l, struct sbuf *out)
{
  if (l->tok == TOK_LPAREN) {
    lsp_next(l);             /* advance to function name */
    translate_list_body(l, out);
    return;
  }
  if (l->tok == TOK_STRING) {
    sbuf_putc(out, '"');
    for (const char *p = l->str; *p; p++) {
      if (*p == '"') sbuf_put(out, "\\\"");
      else sbuf_putc(out, *p);
    }
    sbuf_putc(out, '"');
    lsp_next(l);
    return;
  }
  if (l->tok == TOK_ATOM) {
    sbuf_put(out, l->str);
    lsp_next(l);
    return;
  }
  Con_Error("Lisp: unexpected token %d in binding expression (line %d)", l->tok, l->line);
}

/* Translate the binding expression starting at l->tok, return heap-alloc string
 * in ORCA expression format.  Caller must free(). */
static char *
lsp_translate_binding(struct lsp_lex *l)
{
  struct sbuf out = {0};
  translate_expr(l, &out);
  return out.s ? out.s : strdup("");
}

/* =========================================================================
 * AST: parsed node tree
 * ====================================================================== */

struct lisp_attr {
  char *key;       /* property name */
  char *val;       /* string value, or NULL for binding attrs */
  char *binding;   /* ORCA expression string, or NULL for string attrs */
  struct lisp_attr *next;
};

struct lisp_node {
  char *type;              /* element class name, e.g. "Grid" */
  struct lisp_attr *attrs; /* singly-linked list of attribute k/v pairs */
  struct lisp_node *children; /* singly-linked list of child elements */
  struct lisp_node *next;  /* next sibling */
  char *text;              /* text content, if any */
};

static void
lisp_attr_free(struct lisp_attr *a)
{
  while (a) {
    struct lisp_attr *nx = a->next;
    free(a->key);
    free(a->val);
    free(a->binding);
    free(a);
    a = nx;
  }
}

static void
lisp_node_free(struct lisp_node *n)
{
  while (n) {
    struct lisp_node *nx = n->next;
    free(n->type);
    free(n->text);
    lisp_attr_free(n->attrs);
    lisp_node_free(n->children);
    free(n);
    n = nx;
  }
}

/* =========================================================================
 * Parser: Lisp text → lisp_node AST
 * ====================================================================== */

static struct lisp_node *parse_node(struct lsp_lex *l);

/* parse_element_body: l->tok is positioned after reading the element type.
 * Reads attributes, :bind directives, children, and optional text until ')'.
 * Consumes the closing ')' and advances l to the next token. */
static void
parse_element_body(struct lsp_lex *l, struct lisp_node *node)
{
  struct lisp_attr **attr_tail  = &node->attrs;
  struct lisp_node **child_tail = &node->children;

  while (l->tok != TOK_RPAREN && l->tok != TOK_EOF) {

    /* --- Parenthesised form: child element or :bind directive --- */
    if (l->tok == TOK_LPAREN) {
      lsp_next(l);  /* read head of the list */

      if (l->tok == TOK_ATOM && l->str[0] == ':') {
        /* Directive form: (:bind Target="Prop" expr) */
        if (!strcmp(l->str, ":bind")) {
          lsp_next(l);  /* advance to the Target= atom or bare "prop.name" */
          char *target = NULL;

          if (l->tok == TOK_ATOM) {
            size_t klen = strlen(l->str);
            if (klen > 1 && l->str[klen - 1] == '=') {
              /* Key="Value" form: consume the Key= atom, then read the value */
              lsp_next(l);  /* advance past "Key=" to the string/atom value */
              if (l->tok == TOK_STRING || l->tok == TOK_ATOM) {
                target = strdup(l->str);  /* the property path */
                lsp_next(l);              /* advance past value to the expression */
              }
            } else {
              /* Bare atom (no trailing '=') is the target path directly */
              target = strdup(l->str);
              lsp_next(l);
            }
          } else if (l->tok == TOK_STRING) {
            /* Bare quoted string as target path */
            target = strdup(l->str);
            lsp_next(l);
          }

          char *expr = lsp_translate_binding(l);
          if (target && expr) {
            struct lisp_attr *a = calloc(1, sizeof(*a));
            a->key = target;
            a->binding = expr;
            *attr_tail = a;
            attr_tail = &a->next;
          } else {
            free(target);
            free(expr);
          }

          /* Drain any remaining tokens to the closing ')' of :bind */
          while (l->tok != TOK_RPAREN && l->tok != TOK_EOF) lsp_next(l);
          if (l->tok == TOK_RPAREN) lsp_next(l);

        } else {
          /* Unknown directive — skip */
          Con_Error("Lisp: unknown directive '%s' (line %d)", l->str, l->line);
          int depth = 1;
          while (depth > 0 && l->tok != TOK_EOF) {
            lsp_next(l);
            if      (l->tok == TOK_LPAREN) depth++;
            else if (l->tok == TOK_RPAREN) depth--;
          }
          if (l->tok == TOK_RPAREN) lsp_next(l);
        }

      } else {
        /* Child element — l->tok is the type name atom */
        struct lisp_node *child = parse_node(l);
        if (child) { *child_tail = child; child_tail = &child->next; }
      }
      continue;
    }

    /* --- Bare string: text content --- */
    if (l->tok == TOK_STRING) {
      free(node->text);
      node->text = strdup(l->str);
      lsp_next(l);
      continue;
    }

    /* --- Atom: key="val", key=(binding), or bare key (unquoted value) --- */
    if (l->tok == TOK_ATOM) {
      size_t alen = strlen(l->str);
      const char *eq = strchr(l->str, '=');
      if (eq && eq > l->str && eq[1] != '\0') {
        /* Inline atom assignment, e.g. Width=1280 */
        char *key = strndup(l->str, (size_t)(eq - l->str));
        struct lisp_attr *a = calloc(1, sizeof(*a));
        a->key = key;
        a->val = strdup(eq + 1);
        *attr_tail = a;
        attr_tail = &a->next;
        lsp_next(l);

      } else if (alen > 1 && l->str[alen - 1] == '=') {
        char *key = strndup(l->str, alen - 1);
        lsp_next(l);

        if (l->tok == TOK_STRING) {
          struct lisp_attr *a = calloc(1, sizeof(*a));
          a->key = key;
          a->val = strdup(l->str);
          *attr_tail = a;
          attr_tail = &a->next;
          lsp_next(l);

        } else if (l->tok == TOK_LPAREN) {
          /* Inline binding expression: Key=(lisp-expr) */
          char *expr = lsp_translate_binding(l);
          struct lisp_attr *a = calloc(1, sizeof(*a));
          a->key = key;
          a->binding = expr;
          *attr_tail = a;
          attr_tail = &a->next;

        } else if (l->tok == TOK_ATOM) {
          /* Bare-word value, e.g. a number or enum without quotes */
          struct lisp_attr *a = calloc(1, sizeof(*a));
          a->key = key;
          a->val = strdup(l->str);
          *attr_tail = a;
          attr_tail = &a->next;
          lsp_next(l);

        } else {
          Con_Error("Lisp: expected value after '%s=' (line %d)", key, l->line);
          free(key);
        }
      } else {
        Con_Error("Lisp: unexpected atom '%s' in element body (line %d)", l->str, l->line);
        lsp_next(l);
      }
      continue;
    }

    /* Unexpected token — skip */
    Con_Error("Lisp: unexpected token %d in element body (line %d)", l->tok, l->line);
    lsp_next(l);
  }

  /* Consume closing ')' */
  if (l->tok == TOK_RPAREN) lsp_next(l);
}

/* parse_node: l->tok is the element type name (ATOM), after '(' was consumed.
 * Returns the new node; l->tok points past the closing ')' on return. */
static struct lisp_node *
parse_node(struct lsp_lex *l)
{
  if (l->tok != TOK_ATOM) {
    Con_Error("Lisp: expected element type name (line %d), got tok=%d", l->line, l->tok);
    /* Drain the malformed list */
    int depth = 1;
    while (depth > 0 && l->tok != TOK_EOF) {
      lsp_next(l);
      if      (l->tok == TOK_LPAREN) depth++;
      else if (l->tok == TOK_RPAREN) depth--;
    }
    if (l->tok == TOK_RPAREN) lsp_next(l);
    return NULL;
  }

  struct lisp_node *node = calloc(1, sizeof(*node));
  node->type = strdup(l->str);
  lsp_next(l);
  parse_element_body(l, node);
  return node;
}

/* parse_file: parse a complete Lisp UI source string and return the root node. */
static struct lisp_node *
parse_file(const char *src)
{
  struct lsp_lex l = { .p = src, .line = 1 };
  lsp_next(&l);
  if (l.tok != TOK_LPAREN) {
    Con_Error("Lisp: expected '(' at start of file");
    free(l.str);
    return NULL;
  }
  lsp_next(&l);  /* advance to element type name */
  struct lisp_node *root = parse_node(&l);
  free(l.str);
  return root;
}

/* =========================================================================
 * Object factory: lisp_node → ORCA Object
 * ====================================================================== */

static struct Object *apply_node(struct lisp_node *node);

/* Look up a property descriptor by "implicit" (short name) or explicit (Ns.Name). */
static struct PropertyType const *
lisp_propdesc(struct Object *o, const char *name)
{
  struct PropertyType const *pd = OBJ_FindImplicitProperty(o, name);
  return pd ? pd : OBJ_FindExplicitProperty(o, name);
}

/* Handle special property names that need engine-level calls. */
static bool_t
apply_special_attr(struct Object *o, const char *key, const char *val)
{
  if (!strcmp(key, "ClassName") || !strcmp(key, "PlaceholderTemplate")) return TRUE;
  if (!strcmp(key, "Name") || !strcmp(key, "id")) {
    OBJ_SetName(o, val);
    return TRUE;
  }
  if (!strcmp(key, "class")) {
    _SendMessage(o, StyleController, AddClasses, .ClassNames = val);
    return TRUE;
  }
  return FALSE;
}

/* Append a trigger object to the object's Triggers array. */
static void
lisp_append_trigger(struct Property *triggers_prop, struct Object *trigger)
{
  struct PropertyType const *pd = PROP_GetDesc(triggers_prop);
  void *old_items = NULL;
  int   old_count = 0;

  if (!pd || !pd->IsArray || pd->DataType != kDataTypeObject) {
    OBJ_ReleaseRef(trigger);
    return;
  }
  if (PROP_GetValue(triggers_prop)) {
    memcpy(&old_items, PROP_GetValue(triggers_prop), sizeof(old_items));
    memcpy(&old_count,
           (char *)PROP_GetValue(triggers_prop) + sizeof(old_items),
           sizeof(old_count));
  }
  int count = old_count + 1;
  struct Object **items = calloc((size_t)count, sizeof(struct Object *));
  if (!items) { OBJ_ReleaseRef(trigger); return; }
  if (old_items && old_count > 0)
    memcpy(items, old_items, (size_t)old_count * sizeof(struct Object *));
  items[old_count] = trigger;

  struct { void *items; int count; } value = { items, count };
  PROP_SetValue(triggers_prop, &value);
}

/* Create an inline EventTrigger from a {ShorthandObject ...} string value. */
static void
lisp_inline_event(struct Object *o,
                  struct PropertyType const *pd,
                  const char *value)
{
  struct Property *triggers = NULL;
  struct PropertyType const *td = OBJ_FindImplicitProperty(o, "Triggers");
  struct Object *action  = _LoadObjectFromXmlFragment(value, 0);
  struct Object *trigger = action ? OBJ_Create(ID_EventTrigger) : NULL;
  char routed[MAX_PROPERTY_STRING] = {0};

  if (!td || FAILED(OBJ_FindLongProperty(o, td->FullIdentifier, &triggers)) || !trigger) {
    if (action) OBJ_ReleaseRef(action);
    Con_Error("Lisp: property '%s' does not support inline trigger shorthand", pd->Name);
    return;
  }

  snprintf(routed, sizeof(routed),
           (pd->Category && *pd->Category) ? "%s.%s" : "%s",
           (pd->Category && *pd->Category) ? pd->Category : pd->Name,
           pd->Name);
  const char *rv = routed;
  if (FAILED(OBJ_SetPropertyValue(trigger, "RoutedEvent", &rv))) {
    OBJ_ReleaseRef(trigger);
    OBJ_ReleaseRef(action);
    return;
  }

  OBJ_AddChild(trigger, action, FALSE);
  lisp_append_trigger(triggers, trigger);
}

/* Apply a string-valued attribute: theme lookup, object refs, inline objects,
 * event triggers, binding shorthands, and typed property values. */
static void
apply_str_attr(struct Object *o, const char *key, const char *val)
{
  struct PropertyType const *pd = lisp_propdesc(o, key);
  if (!pd) {
    Con_Error("Lisp: unknown property '%s' on '%s'", key, OBJ_GetClassName(o));
    return;
  }

  /* {Binding path} shorthand → expression program */
  size_t vlen = strlen(val);
  if (vlen > 10 && !strncmp(val, "{Binding ", 9) && val[vlen - 1] == '}') {
    char expr[MAX_PROPERTY_STRING];
    snprintf(expr, sizeof(expr), "{%.*s}", (int)(vlen - 10), val + 9);
    OBJ_AttachPropertyProgram(o, key, expr, kBindingModeExpression, TRUE);
    return;
  }

  /* Theme lookup: $key → resolved string */
  const char *resolved = val;
  char theme_buf[MAX_PROPERTY_STRING] = {0};
  if (val[0] == '$') {
    const char *tv = FS_GetThemeValue(val);
    if (!tv) { Con_Error("Lisp: missing theme key '%s'", val); return; }
    strncpy(theme_buf, tv, sizeof(theme_buf) - 1);
    resolved = theme_buf;
  }

  struct Property *p = NULL;
  if (FAILED(OBJ_FindLongProperty(o, pd->FullIdentifier, &p))) {
    Con_Error("Lisp: could not get property slot '%s'", key);
    return;
  }

  /* @path → object reference */
  if (resolved[0] == '@') {
    struct Object *ref = OBJ_FindByPath(o, resolved + 1);
    if (ref && pd->DataType == kDataTypeObject) PROP_SetValue(p, &ref);
    else Con_Error("Lisp: could not resolve object reference '%s'", resolved + 1);
    return;
  }

  /* { or < → inline object / event trigger */
  if (resolved[0] == '{' || resolved[0] == '<') {
    if (pd->DataType == kDataTypeEvent) {
      lisp_inline_event(o, pd, resolved);
    } else if (pd->DataType == kDataTypeStruct) {
      _LoadStructFromXmlFragment(p, pd, resolved);
    } else {
      struct Object *child = _LoadObjectFromXmlFragment(resolved, 0);
      if (child) {
        if (GetBinding(child) || GetBindingExpression(child)) {
          OBJ_SendMessageW(child, ID_Binding_Compile, 0, p);
          OBJ_ReleaseRef(child);
        } else if (pd->DataType == kDataTypeObject) {
          PROP_SetValue(p, &child);
        } else {
          OBJ_ReleaseRef(child);
          Con_Error("Lisp: unsupported inline object for property '%s'", key);
        }
      }
    }
    return;
  }

  /* Default: parse as typed property value */
  char tmp[MAX_PROPERTY_STRING] = {0};
  if (!parse_property(resolved, pd, tmp)) return;
  PROP_SetValue(p, tmp);
  if (pd->DataType == kDataTypeString) free(*(char **)tmp);
}

static const char * const PREFAB_TYPES[] = {
  "LayerPrefabPlaceholder",
  "ObjectPrefabPlaceholder",
  "LibraryPlaceholder",
  NULL
};

static bool_t
is_prefab_type(const char *type)
{
  for (int i = 0; PREFAB_TYPES[i]; i++)
    if (!strcmp(type, PREFAB_TYPES[i])) return TRUE;
  return FALSE;
}

/* Convert a parsed AST node to an ORCA Object. */
static struct Object *
apply_node(struct lisp_node *node)
{
  if (!node) return NULL;

  bool_t is_prefab = is_prefab_type(node->type);
  struct Object *o = NULL;

  if (is_prefab) {
    const char *tmpl = NULL;
    for (struct lisp_attr *a = node->attrs; a; a = a->next) {
      if (!strcmp(a->key, "PlaceholderTemplate") && a->val) { tmpl = a->val; break; }
    }
    if (tmpl) {
      o = FS_LoadObject(tmpl);
      if (o) OBJ_SetFlags(o, OBJ_GetFlags(o) | OF_TEMPLATE);
    }
    if (!o) {
      Con_Error("Lisp: prefab placeholder missing PlaceholderTemplate");
      return NULL;
    }
  } else {
    struct ClassDesc const *cls = OBJ_FindClass(node->type);
    if (!cls) {
      Con_Error("Lisp: unknown element type '%s'", node->type);
      return NULL;
    }
    o = OBJ_Create(cls->ClassID);
  }

  if (!o) return NULL;

  /* Apply attributes */
  for (struct lisp_attr *a = node->attrs; a; a = a->next) {
    if (!strcmp(a->key, "PlaceholderTemplate")) continue;

    if (a->binding) {
      OBJ_AttachPropertyProgram(o, a->key, a->binding, kBindingModeExpression, TRUE);
    } else if (a->val) {
      if (!apply_special_attr(o, a->key, a->val))
        apply_str_attr(o, a->key, a->val);
    }
  }

  /* Text content: replicate XML loader behaviour (short-circuit, no children) */
  if (node->text) {
    OBJ_SetTextContent(o, node->text);
    if (!is_prefab) OBJ_SendMessageW(o, ID_Object_Start, 0, NULL);
    return o;
  }

  /* Child elements */
  for (struct lisp_node *child = node->children; child; child = child->next) {
    struct Object *child_obj = apply_node(child);
    if (child_obj) OBJ_AddChild(o, child_obj, FALSE);
  }

  if (!is_prefab) OBJ_SendMessageW(o, ID_Object_Start, 0, NULL);
  return o;
}

/* =========================================================================
 * Public API
 * ====================================================================== */

ORCA_API struct Object *
FS_LoadObjectFromLispString(const char *lisp_string)
{
  if (!lisp_string || !*lisp_string) {
    Con_Error("FS_LoadObjectFromLispString: empty input");
    return NULL;
  }
  struct lisp_node *root = parse_file(lisp_string);
  if (!root) {
    Con_Error("FS_LoadObjectFromLispString: parse failed");
    return NULL;
  }
  struct Object *o = apply_node(root);
  lisp_node_free(root);
  if (!o) Con_Error("FS_LoadObjectFromLispString: object creation failed");
  return o;
}

ORCA_API struct Object *
FS_LoadObjectFromLisp(const char *path)
{
  struct file *fp = FS_LoadFile(path);
  if (!fp) {
    Con_Error("Lisp: failed to load file '%s'", path);
    return NULL;
  }
  char *src = malloc(fp->size + 1);
  if (!src) { FS_FreeFile(fp); return NULL; }
  memcpy(src, fp->data, fp->size);
  src[fp->size] = '\0';
  FS_FreeFile(fp);

  struct Object *o = FS_LoadObjectFromLispString(src);
  free(src);
  if (o) OBJ_SetSourceFile(o, path);
  return o;
}
