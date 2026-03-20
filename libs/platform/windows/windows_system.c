#include "windows_local.h"
#include "../platform.h"
#include <commdlg.h>
#include <shlobj.h>

static char g_documents[MAX_PATH] = { 0 };
static PATHSTR g_share = { 0 };
static PATHSTR g_local = { 0 };
static PATHSTR g_lib   = { 0 };

char const *
WI_GetPlatform(void)
{
  return "windows";
}

char const *
WI_SettingsDirectory(void)
{
  if (g_local[0] == 0) {
    char appdata[MAX_PATH] = { 0 };
    SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appdata);
    snprintf(g_local, sizeof(g_local), "%s\\" APPNAME, appdata);
    CreateDirectoryA(g_local, NULL);
  }
  return g_local;
}

static void
get_exe_dir(char *buf, int sz)
{
  GetModuleFileNameA(NULL, buf, sz);
  char *sep = strrchr(buf, '\\');
  if (sep) {
    *sep = '\0';
  }
}

char const *
WI_ShareDirectory(void)
{
  if (g_share[0] == 0) {
    get_exe_dir(g_share, sizeof(g_share));
  }
  return g_share;
}

char const *
WI_LibDirectory(void)
{
  if (g_lib[0] == 0) {
    get_exe_dir(g_lib, sizeof(g_lib));
  }
  return g_lib;
}

char const *
WI_DocumentsDirectory(void)
{
  if (g_documents[0] == 0) {
    SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, g_documents);
  }
  return g_documents;
}

bool_t
WI_IsDarkTheme(void)
{
  HKEY hkey;
  DWORD value = 1; /* default: light theme */
  DWORD size = sizeof(DWORD);
  if (RegOpenKeyExA(HKEY_CURRENT_USER,
                    "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                    0, KEY_READ, &hkey) == ERROR_SUCCESS) {
    RegQueryValueExA(hkey, "AppsUseLightTheme", NULL, NULL,
                     (LPBYTE)&value, &size);
    RegCloseKey(hkey);
  }
  /* AppsUseLightTheme == 0 means dark mode is active */
  return (value == 0) ? TRUE : FALSE;
}

bool_t
WI_GetOpenFileName(struct _WI_OpenFileName const *ofn)
{
  OPENFILENAMEA fn;
  ZeroMemory(&fn, sizeof(fn));
  fn.lStructSize = sizeof(fn);
  fn.hwndOwner   = g_hwnd;
  fn.lpstrFile   = ofn->lpstrFile;
  fn.nMaxFile    = ofn->nMaxFile;
  fn.lpstrFilter = ofn->lpstrFilter;
  fn.lpstrTitle  = ofn->lpstrTitle;
  fn.Flags       = OFN_EXPLORER;
  if (ofn->Flags & OFN_FILEMUSTEXIST) fn.Flags |= OFN_FILEMUSTEXIST;
  if (ofn->Flags & OFN_PATHMUSTEXIST) fn.Flags |= OFN_PATHMUSTEXIST;
  fn.lpstrFile[0] = '\0';
  return GetOpenFileNameA(&fn) ? TRUE : FALSE;
}

bool_t
WI_GetSaveFileName(struct _WI_OpenFileName const *ofn)
{
  OPENFILENAMEA fn;
  ZeroMemory(&fn, sizeof(fn));
  fn.lStructSize = sizeof(fn);
  fn.hwndOwner   = g_hwnd;
  fn.lpstrFile   = ofn->lpstrFile;
  fn.nMaxFile    = ofn->nMaxFile;
  fn.lpstrFilter = ofn->lpstrFilter;
  fn.lpstrTitle  = ofn->lpstrTitle;
  fn.Flags       = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
  fn.lpstrFile[0] = '\0';
  return GetSaveFileNameA(&fn) ? TRUE : FALSE;
}

bool_t
WI_GetFolderName(struct _WI_OpenFileName const *ofn)
{
  BROWSEINFOA bi;
  ZeroMemory(&bi, sizeof(bi));
  bi.hwndOwner = g_hwnd;
  bi.lpszTitle = ofn->lpstrTitle ? ofn->lpstrTitle : "Select Folder";
  bi.ulFlags   = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
  if (pidl) {
    BOOL ok = SHGetPathFromIDListA(pidl, ofn->lpstrFile);
    CoTaskMemFree(pidl);
    return ok ? TRUE : FALSE;
  }
  return FALSE;
}

char const *
KEY_GetKeyName(uint32_t keycode)
{
  (void)keycode;
  return "";
}
