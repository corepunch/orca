#include <source/core/core_local.h>

#define MAX_REGISTERS 64
#define MAX_ARGS 16

ORCA_API struct Object *root_node = NULL;

typedef struct
{
  path_t path;
} argument_t;

struct lexer_state
{
  lpcString_t filename;
  shortStr_t registers[MAX_REGISTERS];
  argument_t arguments[MAX_ARGS];
  uint32_t numargs;
};

bool_t
isoperator(int c)
{
  return c && strchr("+-%*/=<>&|!^~", c) != NULL;
}

struct token*
_TokenMake(lpcString_t from, lpcString_t to, uint32_t type)
{
  assert(from && to);
  struct token* token = ZeroAlloc(sizeof(struct token) + to - from);
  token->type = type;
  strncpy(token->text, from, to - from);
  return token;
}

struct token*
_TokenMakeCall(lpcString_t name)
{
  struct token* token = ZeroAlloc(sizeof(struct token) + strlen(name));
  token->type = tok_call;
  strcpy(token->text, name);
  return token;
}

struct token*
_TokenMakeOperator(char ch)
{
  switch (ch) {
    case '=': return _TokenMake(&ch, &ch, tok_assign);
    case '+': return _TokenMakeCall("ADD");
    case '-': return _TokenMakeCall("SUB");
    case '*': return _TokenMakeCall("MUL");
    case '/': return _TokenMakeCall("DIV");
    case '^': return _TokenMakeCall("POW");
    default:
      Con_Error("Unsupported %c", ch);
      assert(0);
      return NULL;
  }
}

static struct token*
_TokenMaybeExtract(struct token *tok, lpcString_t name, lpcString_t end)
{
  struct {
    lpcString_t name;
    lpcString_t call;
  } extractors[] = {
    { "COLORR", "COLORR" }, { "COLORG", "COLORG" },
    { "COLORB", "COLORB" }, { "COLORA", "COLORA" },
    { "VECTORX", "VECTOR3X" }, { "VECTORY", "VECTOR3Y" },
    { "VECTORZ", "VECTOR3Z" }, { NULL, NULL },
  };

  for (int i = 0; extractors[i].name; i++) {
    if (strlen(extractors[i].name) == (size_t)(end - name) &&
        !strncasecmp(name, extractors[i].name, (size_t)(end - name))) {
      struct token *call = _TokenMakeCall(extractors[i].call);
      call->args[0] = tok;
      return call;
    }
  }
  return tok;
}

lpcString_t*
skip_space(lpcString_t* s)
{
  SkipSpace(*s);
  if (**s == '#') {
    while (**s && **s != '\n')
      (*s)++;
    return skip_space(s);
  }
  return s;
}

bool_t
eat_token(lpcString_t* s, int tok)
{
  skip_space(s);
  if (**s == tok) {
    ++(*s);
    SkipSpace(*s);
    return TRUE;
  } else {
    return FALSE;
  }
}

int lex_get_register(lpcString_t str, struct lexer_state* lex)
{
  FOR_LOOP(i, MAX_REGISTERS)
  {
    if (!lex->registers[i][0]) {
      strncpy(lex->registers[i], str, sizeof(shortStr_t));
      return i;
    }
    if (!strcmp(str, lex->registers[i])) {
      return i;
    }
  }
  return -1;
}

struct token*
parse_expression(lpcString_t*, struct lexer_state*);

static struct token*
_TokenParse(lpcString_t* str, struct lexer_state* lex)
{
  lpcString_t beg = *skip_space(str);

  if (eat_token(str, '+')) {
    Con_Error("%s: unexpected '+' in code, skipping", lex->filename);
  }

  if (eat_token(str, '-')) {
    struct token* neg = _TokenMakeCall("UNM");
    neg->args[0] = _TokenParse(str, lex);
    return neg;
  }

  if (eat_token(str, '{')) {
    struct token* tok = NULL;
    if (isdigit(**str)) {
      int arg = atoi(*str) % MAX_ARGS;
      argument_t* a = &lex->arguments[arg];
      tok = _TokenMake(a->path, strchr(a->path, 0), tok_argument);
      *str = strchr(*str, '}') + 1;
    } else {
      if (**str == '@')
        (*str)++;
      lpcString_t content = *str;
      *str = strchr(*str, '}');
      if (!*str) {
        Con_Error("Expected }");
        return NULL;
      }
      lpcString_t content_end = *str;
      (*str)++;  /* skip '}' */

      /* Look for ", Source=X" suffix inside {PropPath, Source=X} */
      lpcString_t source_sep = NULL;
      for (lpcString_t p = content; p < content_end; p++) {
        if (*p == ',') {
          lpcString_t q = p + 1;
          while (q < content_end && *q == ' ') q++;
          if (content_end - q >= 7 && !strncmp(q, "Source=", 7)) {
            source_sep = p;
            break;
          }
        }
      }

      if (source_sep) {
        lpcString_t prop_end = source_sep;
        while (prop_end > content && prop_end[-1] == ' ') prop_end--;

        lpcString_t src = source_sep + 1;
        while (src < content_end && (*src == ' ' || *src == ',')) src++;
        src += 7;  /* skip "Source=" */

        size_t prop_len = (size_t)(prop_end - content);
        size_t src_len = (size_t)(content_end - src);

        path_t internal = {0};
        if (src_len == 4 && !strncmp(src, "Root", 4)) {
          snprintf(internal, sizeof(internal), "##Root/%.*s", (int)prop_len, content);
        } else if (src_len == 11 && !strncmp(src, "DataContext", 11)) {
          snprintf(internal, sizeof(internal), "DataContext/%.*s", (int)prop_len, content);
        } else if (*src == '.' || *src == '#') {
          /* Relative path or #-prefixed — append prop, adding '/' if needed */
          if (src_len > 0 && src[src_len - 1] == '/') {
            snprintf(internal, sizeof(internal), "%.*s%.*s", (int)src_len, src, (int)prop_len, content);
          } else {
            snprintf(internal, sizeof(internal), "%.*s/%.*s", (int)src_len, src, (int)prop_len, content);
          }
        } else {
          /* Named element: Source=Body → #Body/PropPath */
          snprintf(internal, sizeof(internal), "#%.*s/%.*s", (int)src_len, src, (int)prop_len, content);
        }
        tok = _TokenMake(internal, strchr(internal, 0), tok_argument);
      } else {
        tok = _TokenMake(content, content_end, tok_argument);
      }
    }
    if (eat_token(str, '.')) {
      lpcString_t name = *str;
      while (isalpha(**str)||**str=='_')
        (*str)++;
      tok = _TokenMaybeExtract(tok, name, *str);
    }
    tok->rootnode = root_node;
    return tok;
  }

  if (eat_token(str, '(')) {
    struct token* expr = parse_expression(str, lex);
    if (!eat_token(str, ')')) {
      Con_Error("Expected )");
      Token_Release(expr);
      return NULL;
    }
    expr->inbrackets = TRUE;
    return expr;
  }

  if (eat_token(str, '"')) {
    beg = *str;
    *str = strchr(*str, '"');
    if (!strncmp(beg, "kzb://", 6))
      beg += 6;
    if (!strncmp(beg, "kzb:\\\\", 6))
      beg += 6;
    return _TokenMake(beg, (*str)++, tok_string);
  }

  if (isalpha(**str) || **str == '_') {
    for (; isalnum(**str) || **str == '_'; (*str)++)
      ;
    struct token* ident = _TokenMake(beg, *str, tok_identifier);
    ident->reg = lex_get_register(ident->text, lex);
    if (eat_token(str, '(')) {
      ident->type = tok_call;
      ident->inbrackets = TRUE;
      FOR_LOOP(i, (int)strlen(ident->text))
      {
        ident->text[i] = toupper(ident->text[i]);
      }
      struct token** ch = ident->args;
      do {
        *ch = parse_expression(str, lex);
        assert(ch - ident->args < TOKEN_MAX_ARGS);
        ch++;
      } while (eat_token(str, ','));
      if (!eat_token(str, ')')) {
        Con_Error("Expected )");
        Token_Release(ident);
        return NULL;
      }
    }
    return ident;
  }
  if (isdigit(**str) || (**str == '.' && isdigit((*str)[1]))) {
    for (; isdigit(**str) || **str == '.'; (*str)++)
      ;
    return _TokenMake(beg, *str, tok_constant);
  }

  if (**str) {
    Con_Error("Unexpected token '%c' (0x%02X) in binding expression near: %s",
              **str, (unsigned char)**str, *str);
  } else {
    Con_Error("Unexpected end of binding expression");
  }
  return NULL;
}

#define is_mul(ch)                                                             \
  (!strcmp(ch, "MUL") || !strcmp(ch, "DIV") || !strcmp(ch, "POW"))
#define is_add(ch) (!strcmp(ch, "ADD") || !strcmp(ch, "SUB"))

struct token*
parse_expression(lpcString_t* str, struct lexer_state* lex)
{
  struct token* left = _TokenParse(str, lex);
  if (!left) {
    return NULL;
  }
  bool_t neg = FALSE;
  lpcString_t s = *skip_space(str);
  char op = **str;
  if (*s == '-') {
    op = '+';
    neg = TRUE;
  }
  if (isoperator(*s)) {
    if (*s == '=' && left->type != tok_identifier) {
      Con_Error("Expected identifier on left of = sign");
      return NULL;
    }
    s++;
    struct token* expr = _TokenMakeOperator(op);
    struct token* right = parse_expression(&s, lex);
    if (!right) {
      Token_Release(left);
      return NULL;
    }
    expr->args[0] = left;
    if (is_mul(expr->text) && is_add(right->text) && !right->inbrackets) {
      expr->args[1] = right->args[0];
      right->args[0] = expr;
      *str = s;
      return right;
    } else {
      expr->args[1] = right;
      if (neg) {
        if (right->args[0] && !right->inbrackets) {
          struct token* unm = _TokenMakeCall("UNM");
          unm->args[0] = right->args[0];
          right->args[0] = unm;
          expr->args[1] = right;
        } else {
          struct token* unm = _TokenMakeCall("UNM");
          unm->args[0] = right;
          expr->args[1] = unm;
        }
      } else {
        expr->args[1] = right;
      }
      *str = s;
      return expr;
    }
  } else {
    *str = s;
    return left;
  }
}

ORCA_API struct token*
_compile(lpcString_t code, lpcString_t filename)
{
  struct token *token = NULL, **tail = &token;
  struct lexer_state lex = { .filename = filename };
  for (lpcString_t str = strtok((LPSTR)code, "\n"); str;
       str = strtok(NULL, "\n")) {
    while (**skip_space(&str)) {
      *tail = parse_expression(&str, &lex);
      if (*tail) {
        tail = &(*tail)->next;
      } else {
        break;
      }
    }
  }
  return token;
}

struct _lisp_buf {
  char *s;
  size_t n;
  size_t cap;
};

static bool_t
_lisp_buf_putn(struct _lisp_buf *b, lpcString_t s, size_t len)
{
  if (!len) return TRUE;
  if (b->n + len + 1 > b->cap) {
    size_t need = b->n + len + 1;
    size_t cap = b->cap ? b->cap * 2 : 128;
    if (cap < need) cap = need;
    char *next = realloc(b->s, cap);
    if (!next) return FALSE;
    b->s = next;
    b->cap = cap;
  }
  memcpy(b->s + b->n, s, len);
  b->n += len;
  b->s[b->n] = '\0';
  return TRUE;
}

static bool_t
_lisp_buf_put(struct _lisp_buf *b, lpcString_t s)
{
  return _lisp_buf_putn(b, s, strlen(s));
}

static bool_t
_lisp_buf_putc(struct _lisp_buf *b, char c)
{
  return _lisp_buf_putn(b, &c, 1);
}

static lpcString_t
_lisp_skip(lpcString_t p)
{
  while (*p && isspace((unsigned char)*p)) p++;
  return p;
}

static char *
_lisp_read_atom(lpcString_t *pp)
{
  lpcString_t p = _lisp_skip(*pp);
  lpcString_t start = p;
  while (*p && !isspace((unsigned char)*p) && *p != '(' && *p != ')') p++;
  size_t len = (size_t)(p - start);
  char *out = malloc(len + 1);
  if (!out) return NULL;
  memcpy(out, start, len);
  out[len] = '\0';
  *pp = p;
  return out;
}

static char *
_lisp_parse_expr(lpcString_t *pp, bool_t *ok);

static char *
_lisp_parse_string(lpcString_t *pp, bool_t *ok)
{
  lpcString_t p = _lisp_skip(*pp);
  struct _lisp_buf out = {0};
  if (*p != '"') {
    *ok = FALSE;
    return NULL;
  }
  p++;
  if (!_lisp_buf_putc(&out, '"')) {
    *ok = FALSE;
    free(out.s);
    return NULL;
  }
  while (*p && *p != '"') {
    char c = *p++;
    if (c == '\\' && *p) {
      char esc = *p++;
      if (esc == '"' || esc == '\\') {
        if (!_lisp_buf_putc(&out, '\\') || !_lisp_buf_putc(&out, esc)) {
          *ok = FALSE;
          free(out.s);
          return NULL;
        }
      } else if (esc == 'n') {
        if (!_lisp_buf_put(&out, "\\n")) {
          *ok = FALSE;
          free(out.s);
          return NULL;
        }
      } else if (esc == 't') {
        if (!_lisp_buf_put(&out, "\\t")) {
          *ok = FALSE;
          free(out.s);
          return NULL;
        }
      } else {
        if (!_lisp_buf_putc(&out, '\\') || !_lisp_buf_putc(&out, esc)) {
          *ok = FALSE;
          free(out.s);
          return NULL;
        }
      }
    } else {
      if (c == '"' && !_lisp_buf_putc(&out, '\\')) {
        *ok = FALSE;
        free(out.s);
        return NULL;
      }
      if (!_lisp_buf_putc(&out, c)) {
        *ok = FALSE;
        free(out.s);
        return NULL;
      }
    }
  }
  if (*p != '"') {
    *ok = FALSE;
    free(out.s);
    return NULL;
  }
  p++;
  if (!_lisp_buf_putc(&out, '"')) {
    *ok = FALSE;
    free(out.s);
    return NULL;
  }
  *pp = p;
  return out.s ? out.s : strdup("\"\"");
}

static char *
_lisp_parse_list(lpcString_t *pp, bool_t *ok)
{
  lpcString_t p = _lisp_skip(*pp);
  struct _lisp_buf out = {0};
  if (*p != '(') {
    *ok = FALSE;
    return NULL;
  }
  p++;

  char *fn = _lisp_read_atom(&p);
  if (!fn || !*fn) {
    free(fn);
    *ok = FALSE;
    return NULL;
  }

  if (!strcasecmp(fn, "bind")) {
    free(fn);
    char *arg = _lisp_parse_expr(&p, ok);
    if (!*ok || !arg) {
      free(arg);
      return NULL;
    }
    size_t len = strlen(arg);
    bool_t quoted = len >= 2 && arg[0] == '"' && arg[len - 1] == '"';
    if (!quoted) {
      free(arg);
      *ok = FALSE;
      return NULL;
    }
    if (!_lisp_buf_putc(&out, '{') ||
        !_lisp_buf_putn(&out, arg + 1, len - 2) ||
        !_lisp_buf_putc(&out, '}')) {
      free(arg);
      free(out.s);
      *ok = FALSE;
      return NULL;
    }
    free(arg);
    p = _lisp_skip(p);
    if (*p != ')') {
      *ok = FALSE;
      free(out.s);
      return NULL;
    }
    p++;
    *pp = p;
    return out.s;
  }

  for (char *c = fn; *c; c++) {
    *c = (char)toupper((unsigned char)*c);
  }
  if (!_lisp_buf_put(&out, fn) || !_lisp_buf_putc(&out, '(')) {
    free(fn);
    free(out.s);
    *ok = FALSE;
    return NULL;
  }
  free(fn);

  bool_t first = TRUE;
  for (;;) {
    p = _lisp_skip(p);
    if (*p == ')') {
      p++;
      break;
    }
    if (!*p) {
      free(out.s);
      *ok = FALSE;
      return NULL;
    }
    char *arg = _lisp_parse_expr(&p, ok);
    if (!*ok || !arg) {
      free(arg);
      free(out.s);
      return NULL;
    }
    if (!first && !_lisp_buf_put(&out, ", ")) {
      free(arg);
      free(out.s);
      *ok = FALSE;
      return NULL;
    }
    if (!_lisp_buf_put(&out, arg)) {
      free(arg);
      free(out.s);
      *ok = FALSE;
      return NULL;
    }
    free(arg);
    first = FALSE;
  }

  if (!_lisp_buf_putc(&out, ')')) {
    free(out.s);
    *ok = FALSE;
    return NULL;
  }
  *pp = p;
  return out.s;
}

static char *
_lisp_parse_expr(lpcString_t *pp, bool_t *ok)
{
  lpcString_t p = _lisp_skip(*pp);
  if (!*p) {
    *ok = FALSE;
    return NULL;
  }
  if (*p == '(') {
    char *list = _lisp_parse_list(&p, ok);
    if (!*ok) return NULL;
    *pp = p;
    return list;
  }
  if (*p == '"') {
    char *s = _lisp_parse_string(&p, ok);
    if (!*ok) return NULL;
    *pp = p;
    return s;
  }
  char *atom = _lisp_read_atom(&p);
  if (!atom) {
    *ok = FALSE;
    return NULL;
  }
  *pp = p;
  return atom;
}

ORCA_API char *
PROP_TranslateLispBinding(lpcString_t code)
{
  lpcString_t p = _lisp_skip(code);
  bool_t ok = TRUE;
  if (*p != '(') {
    return NULL;
  }
  char *translated = _lisp_parse_expr(&p, &ok);
  p = _lisp_skip(p);
  if (!ok || !translated || *p) {
    free(translated);
    return NULL;
  }
  return translated;
}

ORCA_API struct token*
Token_Create(lpcString_t code)
{
  struct token* _compile(lpcString_t code, lpcString_t filename);
  lpcString_t p = _lisp_skip(code);
  if (*p == '(') {
    char *lisp = PROP_TranslateLispBinding(code);
    if (lisp) {
      struct token *result = _compile(lisp, "binding");
      free(lisp);
      return result;
    }
  }
  return _compile(code, "binding");
}
