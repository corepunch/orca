#include <include/api.h>
#include <include/renderer.h>

#include "wim_local.h"

IRENDERER lpRenderer;
IFILESYSTEM lpFileSystem;
ICONSOLE lpConsole;

#define IDWIMSHEADER MAKE_FOURCC('W', 'I', 'M', 'S')

typedef enum _WIMPATTERNTYPE
{
  WPT_UNDEFINED,
  WPT_VARIABLE,
  WPT_UNIT,
  WPT_ICON,
} WIMPATTERNTYPE;

typedef struct _WIMPATTERN
{
  WIMPATTERNTYPE Type;
  DWORD PatternOffset;
  DWORD ReferenceOffset;
}* LPWIMPATTERN;

typedef struct _WIMTEXT
{
  BYTE L;
  BYTE E;
  BYTE Line;
  BYTE Unused;
  DWORD StringOffset;
}* LPWIMTEXT;

typedef struct _WIMLANGUAGE
{
  DWORD Identifier;
  DWORD Grid;
  DWORD StartText;
  DWORD NumTexts;
}* LPWIMLANGUAGE;

typedef struct _WIMSCREEN
{
  DWORD Hil_ID;
  DWORD StartPattern;
  DWORD NumPatterns;
  DWORD StartLanguage;
  DWORD NumLanguages;
  DWORD NameOffset;
  DWORD ClassificationOffset;
  struct
  {
    BYTE Warning_Message : 1;
    BYTE Acknowledgement : 1;
    BYTE Sound_Only : 1;
    BYTE Urgent : 1;
    BYTE Premature_Confirmation_by_System : 1;
    BYTE Confirmation_by_Driver : 1;
    BYTE Confirmation_by_System : 1;
    BYTE Retrigger_after_System_Confirmation : 1;
    BYTE Retrigger_after_Driver_Confirmation : 1;
    BYTE Warning_Memory_Diagnostics : 1;
    BYTE Warning_Memory : 1;
    BYTE Deletable_From_Warning_Memory : 1;
    BYTE Telltale_Synchronously : 1;
    BYTE Reserved : 3;
    BYTE Colour : 2;
    BYTE Colour_SW : 2;
    BYTE Main_Priority : 4;
  } Flags;
  DWORD Sub_Priority;
}* LPWIMSCREEN;

typedef struct _WIMHEADER
{
  DWORD Magic;
  DWORD Version;
  DWORD ScreensOffset;
  DWORD PatternsOffset;
  DWORD LanguagesOffset;
  DWORD TextsOffset;
  DWORD NumScreens;
}* LPWIMHEADER;

static LPFILE wims;

#define LANG_US 0x5ce8ec36 // eng_Latn_US

LPWIMHEADER header;

LPVOID
WIM_GetData(DWORD dwOffset)
{
  return wims->data + dwOffset;
}

#define WIM_GETTER(TYPE, Type)                                                 \
  LPWIM##TYPE WIM_Get##Type(DWORD dwIndex)                                     \
  {                                                                            \
    return (LPWIM##TYPE)WIM_GetData(header->Type##sOffset) + dwIndex;          \
  }

WIM_GETTER(SCREEN, Screen);
WIM_GETTER(LANGUAGE, Language);
WIM_GETTER(PATTERN, Pattern);
WIM_GETTER(TEXT, Text);

LPWIMSCREEN
WIM_FindScreen(DWORD id)
{
  FOR_LOOP(i, header->NumScreens)
  {
    LPWIMSCREEN screen = WIM_GetScreen(i);
    //        printf("%06x\n", screen->Hil_ID);
    if (screen->Hil_ID == id) {
      return screen;
    }
  }
  return NULL;
}

LPWIMLANGUAGE
WIM_LocalizeScreen(LPWIMSCREEN screen, DWORD language)
{
  FOR_LOOP(i, screen->NumLanguages)
  {
    LPWIMLANGUAGE lang = WIM_GetLanguage(i + screen->StartLanguage);
    if (lang->Identifier == language) {
      return lang;
    }
  }
  return NULL;
}

HRESULT
WIM_Init(LPCSTR szFileName)
{
  HRESULT hr = lpFileSystem->LoadFile(szFileName, &wims);
  if (FAILED(hr)) {
    return hr;
  }
  header = (LPVOID)wims->data;
  if (header->Magic != IDWIMSHEADER) {
    free(wims);
    wims = NULL;
    return E_INVALIDARG;
  }

  //    LPWIMSCREEN screen = WIM_FindScreen(0x200072);
  //    LPWIMLANGUAGE lang = WIM_LocalizeScreen(screen, LANG_US);
  //    LPWIMTEXT text = WIM_GetText(lang->StartText);
  //    LPCSTR line = WIM_GetData(text->StringOffset);
  //    printf("%s\n", line);

  //    FOR_LOOP(i, header->NumScreens) {
  //        LPWIMSCREEN scr = WIM_GetScreen(i);
  //        LPWIMLANGUAGE lang = WIM_LocalizeScreen(scr, LANG_US);
  //        if (lang && lang->Grid == 200) {
  //            printf("0x%08x,\n", scr->Hil_ID);
  //        }
  //    }

  return NOERROR;
}

void
WIM_Shutdown(void)
{
  free(wims);
}

void
WIM_EnumScreens(WIMSCREENPROC fnProc, LPVOID lpData)
{
  FOR_LOOP(i, header->NumScreens)
  {
    LPWIMSCREEN screen = WIM_GetScreen(i);
    LPWIMLANGUAGE lang = WIM_LocalizeScreen(screen, LANG_US);
    if (!lang) {
      fnProc(
        &(struct _WIMSCREENVIEW){
          .id = screen->Hil_ID,
          .string = "<NO TEXT>",
        },
        lpData);
    } else {
      LPWIMTEXT text = WIM_GetText(lang->StartText);
      fnProc(
        &(struct _WIMSCREENVIEW){
          .id = screen->Hil_ID,
          .string = WIM_GetData(text->StringOffset),
        },
        lpData);
    }
  }
}

static int
f_init(lua_State* L)
{
  LPCSTR szFileName = luaL_checkstring(L, 1);
  WIM_Init(szFileName);
  return 0;
}

static int
f_shutdown(lua_State* L)
{
  WIM_Shutdown();
  return 0;
}

static int
hack_icon_color(int color)
{
  switch (color) {
    case 1:
      return 2;
    case 2:
      return 1;
    default:
      return color;
  }
}

static int
f_find_screen(lua_State* L)
{
  DWORD hil_id = (DWORD)lua_tointeger(L, 1);
  LPCSTR langauge = luaL_checkstring(L, 2);
  LPWIMSCREEN lpScreen = WIM_FindScreen(hil_id);
  if (!lpScreen) {
    lua_pushnil(L);
    lpConsole->Printf("Can't find screen 0x%06x", hil_id);
    return 1;
  }
  LPWIMLANGUAGE lpLanguage = WIM_LocalizeScreen(lpScreen, fnv1a32(langauge));
  if (!lpLanguage) {
    lua_pushnil(L);
    lpConsole->Printf(
      "Can't find language %s in screen %08x", langauge, hil_id);
    return 1;
  }
  lua_newtable(L); // screen table

  lua_pushboolean(L, lpScreen->Flags.Warning_Message);
  lua_setfield(L, -2, "Warning_Message");
  lua_pushboolean(L, lpScreen->Flags.Acknowledgement);
  lua_setfield(L, -2, "Acknowledgement");
  lua_pushboolean(L, lpScreen->Flags.Sound_Only);
  lua_setfield(L, -2, "Sound_Only");
  lua_pushboolean(L, lpScreen->Flags.Urgent);
  lua_setfield(L, -2, "Urgent");
  lua_pushboolean(L, lpScreen->Flags.Premature_Confirmation_by_System);
  lua_setfield(L, -2, "Premature_Confirmation_by_System");
  lua_pushboolean(L, lpScreen->Flags.Confirmation_by_Driver);
  lua_setfield(L, -2, "Confirmation_by_Driver");
  lua_pushboolean(L, lpScreen->Flags.Confirmation_by_System);
  lua_setfield(L, -2, "Confirmation_by_System");
  lua_pushboolean(L, lpScreen->Flags.Retrigger_after_System_Confirmation);
  lua_setfield(L, -2, "Retrigger_after_System_Confirmation");
  lua_pushboolean(L, lpScreen->Flags.Retrigger_after_Driver_Confirmation);
  lua_setfield(L, -2, "Retrigger_after_Driver_Confirmation");
  lua_pushboolean(L, lpScreen->Flags.Warning_Memory_Diagnostics);
  lua_setfield(L, -2, "Warning_Memory_Diagnostics");
  lua_pushboolean(L, lpScreen->Flags.Warning_Memory);
  lua_setfield(L, -2, "Warning_Memory");
  lua_pushboolean(L, lpScreen->Flags.Deletable_From_Warning_Memory);
  lua_setfield(L, -2, "Deletable_From_Warning_Memory");
  lua_pushboolean(L, lpScreen->Flags.Telltale_Synchronously);
  lua_setfield(L, -2, "Telltale_Synchronously");
  lua_pushinteger(L, lpScreen->Flags.Colour);
  lua_setfield(L, -2, "FontColor");
  lua_pushinteger(L, hack_icon_color(lpScreen->Flags.Colour));
  lua_setfield(L, -2, "IconColor");
  lua_pushinteger(L, lpScreen->Flags.Colour_SW);
  lua_setfield(L, -2, "Colour_SW");
  lua_pushinteger(L, lpScreen->Flags.Main_Priority);
  lua_setfield(L, -2, "Main_Priority");
  lua_pushinteger(L, lpScreen->Sub_Priority);
  lua_setfield(L, -2, "Sub_Priority");

  if (lpLanguage->Grid == 2001) {
    lua_pushfstring(L, "IC_200_1");
  } else {
    lua_pushfstring(L, "IC_%d", lpLanguage->Grid / 10);
  }
  lua_setfield(L, -2, "grid");
  FOR_LOOP(i, lpScreen->NumPatterns)
  {
    LPWIMPATTERN lpPattern = WIM_GetPattern(lpScreen->StartPattern + i);
    lua_newtable(L); // pattern
    lua_pushinteger(L, lpPattern->Type);
    lua_setfield(L, -2, "type");
    lua_pushstring(L, WIM_GetData(lpPattern->ReferenceOffset));
    lua_setfield(L, -2, "value");
    lua_setfield(L, -2, WIM_GetData(lpPattern->PatternOffset));
  }
  FOR_LOOP(i, lpLanguage->NumTexts)
  {
    LPWIMTEXT lpText = WIM_GetText(lpLanguage->StartText + i);
    CHAR fmt[32];
    if (lpText->Line > 0) {
      snprintf(
        fmt, sizeof(fmt), "L%d_E%d_L%d", lpText->E, lpText->L, lpText->Line);
    } else {
      snprintf(fmt, sizeof(fmt), "L%d_E%d", lpText->E, lpText->L);
    }
    lua_pushstring(L, WIM_GetData(lpText->StringOffset));
    lua_setfield(L, -2, fmt);
  }
  return 1;
}

static luaL_Reg const lib_wim[] = { { "init", f_init },
                                    { "find_screen", f_find_screen },
                                    { "shutdown", f_shutdown },
                                    { NULL, NULL } };

ORCA_API int
luaopen_wim(lua_State* L)
{
  lpRenderer = API_GetModule(L, IID_RENDERER);
  lpFileSystem = API_GetModule(L, IID_FILESYSTEM);
  lpConsole = API_GetModule(L, IID_CONSOLE);

  luaL_newlib(L, lib_wim);
  return 1;
}
