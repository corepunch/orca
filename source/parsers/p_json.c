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

// Convert a Unicode codepoint (0x0000-0xFFFF) to UTF-8 bytes
// Returns the number of bytes written
static int
unicode_to_utf8(unsigned int codepoint, char* output)
{
  if (codepoint <= 0x7F) {
    // 1-byte sequence: 0xxxxxxx
    output[0] = (char)codepoint;
    return 1;
  } else if (codepoint <= 0x7FF) {
    // 2-byte sequence: 110xxxxx 10xxxxxx
    output[0] = (char)(0xC0 | (codepoint >> 6));
    output[1] = (char)(0x80 | (codepoint & 0x3F));
    return 2;
  } else {
    // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
    output[0] = (char)(0xE0 | (codepoint >> 12));
    output[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
    output[2] = (char)(0x80 | (codepoint & 0x3F));
    return 3;
  }
}

// Parse a hex digit (0-9, A-F, a-f) to its numeric value
static int
parse_hex_digit(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return -1;
}

void
push_json_string(lua_State* L, lpcString_t* s)
{
  LPSTR write_ptr = (LPSTR)*s; // Writing position
  LPSTR read_ptr = (LPSTR)*s;  // Reading position
  while (*read_ptr && *read_ptr != '"') {
    if (*read_ptr == '\\' && read_ptr[1]) {
      read_ptr++; // Skip backslash
      switch (*read_ptr) {
        case '"':  *write_ptr++ = '"';  read_ptr++; break;
        case '\\': *write_ptr++ = '\\'; read_ptr++; break;
        case '/':  *write_ptr++ = '/';  read_ptr++; break;
        case 'b':  *write_ptr++ = '\b'; read_ptr++; break;
        case 'f':  *write_ptr++ = '\f'; read_ptr++; break;
        case 'n':  *write_ptr++ = '\n'; read_ptr++; break;
        case 'r':  *write_ptr++ = '\r'; read_ptr++; break;
        case 't':  *write_ptr++ = '\t'; read_ptr++; break;
        case 'u': {
          // Parse \uXXXX Unicode escape sequence
          read_ptr++; // Skip 'u'
          unsigned int codepoint = 0;
          int valid_unicode = 1;
          for (int i = 0; i < 4; i++) {
            int hex_val = parse_hex_digit(read_ptr[i]);
            if (hex_val < 0) {
              valid_unicode = 0;
              break;
            }
            codepoint = (codepoint << 4) | hex_val;
          }
          if (valid_unicode) {
            read_ptr += 4; // Skip the 4 hex digits
            // Convert codepoint to UTF-8
            int utf8_len = unicode_to_utf8(codepoint, write_ptr);
            write_ptr += utf8_len;
          } else {
            // Invalid Unicode escape - copy literal backslash and 'u'
            *write_ptr++ = '\\';
            *write_ptr++ = 'u';
            // read_ptr already points past 'u'
          }
          break;
        }
        default:
          // Unknown escape sequence - per JSON spec, this is invalid
          // For robustness, copy the backslash and character literally
          *write_ptr++ = '\\';
          *write_ptr++ = *read_ptr++;
          break;
      }
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
