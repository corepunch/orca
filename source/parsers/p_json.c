#include <include/api.h>

#include <ctype.h>

lpcString_t
skip_whitespace(lpcString_t* s)
{
  SkipSpace(*s);
  return *s;
}

int eat_char(lpcString_t* s, char c)
{
  skip_whitespace(s);
  if (**s == c) {
    (*s)++;
    skip_whitespace(s);
    return 1;
  }
  return 0;
}

void
push_json_value(lua_State* L, lpcString_t* s);

void
push_json_object(lua_State* L, lpcString_t* s)
{
  lua_newtable(L);
  if (!eat_char(s, '{'))
    return;

  while (**s && **s != '}') {
    if (!eat_char(s, '"'))
      return;
    lpcString_t key_start = *s;
    while (eat_char(s, '"') == 0)
      (*s)++; // Move until closing quote
    lua_pushlstring(L, key_start, *s - key_start - 1);
    if (!eat_char(s, ':'))
      return;
    push_json_value(L, s);
    lua_settable(L, -3);
    eat_char(s, ',');
  }
  eat_char(s, '}');
}

void
push_json_array(lua_State* L, lpcString_t* s)
{
  lua_newtable(L);
  int index = 1;
  if (!eat_char(s, '['))
    return;

  while (**s && **s != ']') {
    push_json_value(L, s);
    lua_rawseti(L, -2, index++);
    eat_char(s, ',');
  }
  eat_char(s, ']');
}

void
push_json_string(lua_State* L, lpcString_t* s)
{
  LPSTR write_ptr = (LPSTR)*s; // Writing position
  LPSTR read_ptr = (LPSTR)*s;  // Reading position
  while (*read_ptr && *read_ptr != '"') {
    if (*read_ptr == '\\' && read_ptr[1] == '"') {
      *write_ptr++ = '"'; // Replace \" with "
      read_ptr += 2;      // Skip the backslash and quote
    } else {
      *write_ptr++ = *read_ptr++; // Copy normally
    }
  }
  // Push modified string to Lua
  lua_pushlstring(L, *s, write_ptr - *s);
  *s = read_ptr;
  eat_char(s, '"');
}

ORCA_API void
push_json_value(lua_State* L, lpcString_t* s)
{
  skip_whitespace(s);
  if (eat_char(s, '"')) { // String
    push_json_string(L, s);
  } else if (**s == '{') {
    push_json_object(L, s);
  } else if (**s == '[') {
    push_json_array(L, s);
  } else { // Number, `true`, `false`, `null`
    lpcString_t start = *s;
    while (!isspace(**s) && **s != ',' && **s != ']' && **s != '}')
      (*s)++;
    if (!strncmp(start, "true", 4))
      lua_pushboolean(L, 1);
    else if (!strncmp(start, "false", 5))
      lua_pushboolean(L, 0);
    else if (!strncmp(start, "null", 4))
      lua_pushnil(L);
    else
      lua_pushnumber(L, atof(start));
  }
}

int lua_json_parse(lua_State* L)
{
  lpcString_t json = luaL_checkstring(L, 1);
  push_json_value(L, &json);
  return 1;
}

ORCA_API int luaopen_orca_parsers_json(lua_State* L)
{
  luaL_newlib(L, ((luaL_Reg[]){ { "parse", lua_json_parse }, { NULL, NULL } }));
  return 1;
}
