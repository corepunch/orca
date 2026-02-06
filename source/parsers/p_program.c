#include "xml_local.h"

#define MAX_REGISTERS 64
#define MAX_ARGS 16

lpObject_t root_node = NULL;

typedef struct
{
  path_t path;
  enum property_attribute attr;
} argument_t;

struct lexer_state
{
  lpcString_t filename;
  shortStr_t registers[MAX_REGISTERS];
  argument_t arguments[MAX_ARGS];
  uint32_t numargs;
};

enum property_attribute
parsePropertyAttrInCode(lpcString_t s)
{
  if (!strcmp(s, "COLORR")) return ATTR_COLOR_R;
  if (!strcmp(s, "COLORG")) return ATTR_COLOR_G;
  if (!strcmp(s, "COLORB")) return ATTR_COLOR_B;
  if (!strcmp(s, "COLORA")) return ATTR_COLOR_A;
  if (!strcmp(s, "VECTORX")||!strcmp(s, "VECTOR_X")) return ATTR_VECTOR_X;
  if (!strcmp(s, "VECTORY")||!strcmp(s, "VECTOR_Y")) return ATTR_VECTOR_Y;
  if (!strcmp(s, "VECTORZ")||!strcmp(s, "VECTOR_Z")) return ATTR_VECTOR_Z;
  if (!strcmp(s, "VECTORW")||!strcmp(s, "VECTOR_W")) return ATTR_VECTOR_W;
  assert(0);
  return ATTR_WHOLE_PROPERTY;
}

bool_t
isoperator(int c)
{
  return c == '+' || c == '-' || c == '%' || c == '*' || c == '/' || c == '=' ||
         c == '<' || c == '>' || c == '&' || c == '|' || c == '^' || c == '!' ||
         c == '~';
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
      tok->attr = a->attr;
      *str = strchr(*str, '}') + 1;
    } else {
      if (**str == '@')
        (*str)++;
      *str = strchr(*str, '}');
      tok = _TokenMake(beg + 1, (*str)++, tok_argument);
    }
    if (eat_token(str, '.')) {
      lpcString_t s = *str;
      while (isalpha(**str)||**str=='_')
        (*str)++;
      shortStr_t attr = { 0 };
      for (LPSTR a = attr; s != *str; *(a++) = toupper(*(s++)))
        ;
      tok->attr = parsePropertyAttrInCode(attr);
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
    if (!strcmp(ident->text, "ANIMATE") && ident->args[1]) {
      ident->userdata = FS_LoadXML;
    }
    return ident;
  }
  if (isdigit(**str) || (**str == '.' && isdigit((*str)[1]))) {
    for (; isdigit(**str) || **str == '.'; (*str)++)
      ;
    return _TokenMake(beg, *str, tok_constant);
  }

  assert(0);
  Con_Warning("%s", *str);
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
