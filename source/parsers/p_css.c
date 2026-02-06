#include <ctype.h>
#include <include/api.h>
#include <string.h>

lpcString_t
skip_whitespace(lpcString_t* s);
int eat_char(lpcString_t* s, char c);
void parse_single_class(lua_State* L, lpcString_t prefix, lpcString_t* s);

void
kebab_to_pascal(lpcString_t a, LPSTR b)
{
  if (!strcmp(a, "color")) {
    strcpy(b, "ForegroundColor");
    return;
  }
  for (int c = 1; *a; a++)
    if (*a == '-')
      c = 1;
    else
      *b++ = c ? ((void)(c = 0), toupper(*a)) : *a;
  *b = '\0';
}

void
push_css_value(lua_State* L, lpcString_t* s)
{
  lpcString_t start = *s;
  skip_whitespace(s);
  while (**s && **s != ';' && **s != '}')
    (*s)++;
  lua_pushlstring(L, start, *s - start);
}

void
push_css_properties(lua_State* L, lpcString_t classname, lpcString_t* s)
{
  if (!eat_char(s, '{'))
    return;

  while (**s && **s != '}') {
    skip_whitespace(s);
    while (**s == '-')
      (*s)++;
    lpcString_t key_start = *s;
    switch (**s) {
      case '@':
        while (**s && !isspace(**s))
          (*s)++;
        break;
      case '&':
        (*s)++;
        parse_single_class(L, classname, s);
        continue;
      default:
        while (**s && **s != ':')
          (*s)++;
    }
    shortStr_t tmp = { 0 };
    strncpy(tmp, key_start, MIN(sizeof(tmp), *s - key_start));
    kebab_to_pascal(tmp, tmp);
    lua_pushstring(L, tmp);
    //		lua_pushlstring(L, key_start, *s - key_start);
    if (*key_start != '@') {
      eat_char(s, ':');
    }
    push_css_value(L, s);
    lua_settable(L, -3);
    eat_char(s, ';');
  }
  eat_char(s, '}');
}

int push_css_selectors(lua_State* L, LPSTR s, int table_idx)
{
  while (*s) {
    skip_whitespace((lpcString_t*)&s);
    lpcString_t sel_start = s;
    while (*s && !isspace(*s) && *s != ',')
      s++;
    if (*s)
      *(s++) = '\0';
    SkipSpace(sel_start);
    if (!*sel_start) {
      return luaL_error(L, "Expected identifier");
    }
    int length = 0;
    for (lpcString_t a = sel_start; a != s && *a; a++, length++)
      ;
    lua_pushlstring(L, sel_start, length);
    lua_pushvalue(L, -2);
    lua_settable(L, table_idx);

    if (*s == ',')
      s++;
  }
  return 0;
}

void
parse_single_class(lua_State* L, lpcString_t prefix, lpcString_t* s)
{
  path_t classname = { 0 };
  lpcString_t sel_start = *s;
  if ((*s = strchr(*s, '{'))) {
    int length = 0;
    for (lpcString_t a = prefix; *a && !isspace(*a); a++, length++)
      ;
    strncpy(classname, prefix, length);
    memcpy(classname + length, sel_start, *s - sel_start);
    lua_newtable(L);
    push_css_properties(L, classname, s);
    push_css_selectors(L, classname, 2);
    lua_pop(L, 1);
  }
}

int lua_css_parse(lua_State* L)
{
  lpcString_t css = luaL_checkstring(L, 1);
  lua_newtable(L);

  while (*css) {
    skip_whitespace(&css);
    if (!*css)
      break;
    parse_single_class(L, "", &css);
  }

  return 1;
}

ORCA_API int luaopen_orca_parsers_css(lua_State* L)
{
  luaL_newlib(L, ((luaL_Reg[]){ { "parse", lua_css_parse }, { NULL, NULL } }));
  return 1;
}
