#include <include/api.h>
#include <libxml/tree.h>

typedef struct _LOCALE
{
  struct _LOCALE* next;
  uint32_t ident;
  LOCALE_TYPE type;
  char text[1];
}* PLOCALE;

static PLOCALE locales;

typedef void (*FORMATPROC)(LPSTR szOutput, uint32_t dwLength);

typedef struct _FORMATARG
{
  lpcString_t szString;
  FORMATPROC fnFormatProc;
} FORMATARG, *PFORMATARG;

#define VARIABLE(NAME) { #NAME, NAME }
#define FMTF(NAME, FORMAT, VALUE)                                              \
  void NAME(LPSTR szOutput, uint32_t dwLength)                                 \
  {                                                                            \
    snprintf(szOutput, dwLength, FORMAT, VALUE);                               \
  }

FMTF(data_FirstName, "%s", "John");
FMTF(data_LastName, "%s", "Doe");
FMTF(data_Age, "%s", "30");
FMTF(data_Email, "%s", "john.doe@example.com");
FMTF(data_Greeting, "%s", "Hello, World!");

static FORMATARG args[] = {
  VARIABLE(data_FirstName),
  VARIABLE(data_LastName),
  VARIABLE(data_Age),
  VARIABLE(data_Email),
  VARIABLE(data_Greeting),

  { NULL }
};

static FORMATPROC
FindFormatProc(lpcString_t szTag, size_t dwLength)
{
  for (PFORMATARG fmt = args; fmt->szString; fmt++) {
    if (!strncmp(fmt->szString, szTag, dwLength)) {
      return fmt->fnFormatProc;
    }
  }
  return NULL;
}

static void
formatstring(lpcString_t szFormat, LPSTR szOutput, uint32_t dwLength)
{
  lpcString_t szTag = strchr(szFormat, '<');
  *szOutput = 0;
  if (!szTag) {
    strncpy(szOutput, szFormat, dwLength);
  } else {
    size_t dwSkip = MIN(dwLength, szTag - szFormat);
    strncpy(szOutput, szFormat, dwSkip);
    szOutput += dwSkip;
    dwLength -= dwSkip;
    if (dwLength == 0) {
      // TODO: ran out of space, use realloc()
      return;
    }
    ++szTag; // skip <
    lpcString_t szEnd = strchr(szTag, '>');
    FORMATPROC fnProc = FindFormatProc(szTag, szEnd - szTag);
    if (fnProc) {
      fnProc(szOutput, dwLength);
      while (*szOutput) {
        szOutput++;
        dwLength--;
      }
    }
    formatstring(++szEnd, szOutput, dwLength);
  }
}

lpcString_t
Loc_GetString(lpcString_t szName, LOCALE_TYPE type)
{
  uint32_t ident = fnv1a32(szName);
  FOR_EACH_LIST(struct _LOCALE, locale, locales)
  {
    if (locale->ident == ident && locale->type == type) {
      if (type == LOC_TEXT) {
        static char tmp[1024];
        formatstring(locale->text, tmp, sizeof(tmp));
        return tmp;
      } else {
        return locale->text;
      }
    }
  }
  return szName;
}

void
Loc_AddEntry(lpcString_t key, lpcString_t value, LOCALE_TYPE type)
{
  if (!key || !value) return;
  PLOCALE locale = ZeroAlloc(sizeof(struct _LOCALE) + strlen(value));
  locale->ident = fnv1a32(key);
  locale->type = type;
  memcpy(locale->text, value, strlen(value));
  ADD_TO_LIST(locale, locales);
}

bool_t
Loc_LoadFromXML(xmlNodePtr xml)
{
  if (!xmlStrcmp(xml->name, XMLSTR("Locale"))) {
    xmlForEach(entry, xml)
    {
      WITH(xmlChar, name, xmlGetProp(entry, XMLSTR("Key")), xmlFree)
      {
        WITH(xmlChar, type, xmlGetProp(entry, XMLSTR("Type")), xmlFree)
        {
          WITH(xmlChar, content, xmlNodeGetContent(entry), xmlFree)
          {
            PLOCALE locale =
              ZeroAlloc(sizeof(struct _LOCALE) + xmlStrlen(content));
            locale->ident = fnv1a32((lpcString_t)name);
            if (!xmlStrcmp(type, XMLSTR("TEXT"))) {
              locale->type = LOC_TEXT;
            } else if (!xmlStrcmp(type, XMLSTR("RESOURCE"))) {
              locale->type = LOC_RESOURCE;
            } else {
              locale->type = LOC_UNDEFINED;
            }
            memcpy(locale->text, content, xmlStrlen(content));
            ADD_TO_LIST(locale, locales);
          }
        }
      }
    }
    return TRUE;
  } else {
    Con_Error("Localization object have tag \"Locale\"");
    return FALSE;
  }
}

static int f_load_localization(lua_State* L)
{
	lpcString_t source = luaL_checkstring(L, 1);
  bool_t loaded = FALSE;
  WITH(struct _xmlDoc, doc, FS_LoadXML(source), xmlFreeDoc)
  {
    loaded = Loc_LoadFromXML(xmlDocGetRootElement(doc));
  }
  lua_pushboolean(L, loaded);
  return 1;
}

int w_localization_shutdown(lua_State* L)
{
  Con_Printf("Shutting down localization");
  FOR_EACH_LIST(struct _LOCALE, loc, locales)
  {
    free(loc);
  }
  return 0;
}

static luaL_Reg const lib_files[] = { { "load", f_load_localization },
                                      { NULL, NULL } };

ORCA_API int luaopen_orca_localization(lua_State* L)
{
  Con_Printf("Initializing localization");
  
  locales = 0;
  
//  ALLOCATE_MODULE(L, IID_LOCALIZATION, struct localization);

  luaL_newlib(L, lib_files);

  API_MODULE_SHUTDOWN(L, w_localization_shutdown);

  return 1;
}
