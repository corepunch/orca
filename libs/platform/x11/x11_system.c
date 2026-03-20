#include "../platform.h"
#include "x11_local.h"
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#define APPNAME "highperf"

static char documents[1024] = { 0 };
static PATHSTR g_share = { 0 };
static PATHSTR g_local = { 0 };
static PATHSTR g_lib   = { 0 };

char const*
WI_GetPlatform(void)
{
  return "linux (x11)";
}

char const*
WI_SettingsDirectory(void)
{
  if (g_local[0] == 0) {
    snprintf(g_local, sizeof(g_local), "%s/." APPNAME, getenv("HOME"));
  }
  return g_local;
}

static void
WI_BundleDirectory(char* buf, int sz, char const* dir)
{
  char path[512];
  snprintf(path, sizeof(path), "/proc/%d/exe", getpid());
  int len = (int)readlink(path, buf, sz - 1);
  if (len <= 0) {
    buf[0] = '\0';
    return;
  }
  buf[len] = '\0';
  char* c = buf + len - 1;
  for (int i = 1; c >= buf; c--) {
    if (*c == '/') {
      if (i > 0) {
        i--;
      } else {
        break;
      }
    }
  }
  strcpy(c, dir);
}

char const*
WI_ShareDirectory(void)
{
  if (g_share[0] == 0) {
    WI_BundleDirectory(g_share, sizeof(g_share), "/share/" APPNAME);
  }
  return g_share;
}

char const*
WI_LibDirectory(void)
{
  if (g_lib[0] == 0) {
    WI_BundleDirectory(g_lib, sizeof(g_lib), "/lib/" APPNAME);
  }
  return g_lib;
}

char const*
WI_DocumentsDirectory(void)
{
  if (documents[0] == 0) {
    char const* home = getenv("HOME");
    if (home) {
      snprintf(documents, sizeof(documents), "%s/Documents", home);
    }
  }
  return documents;
}

bool_t
WI_IsDarkTheme(void)
{
  char const* gtk_theme = getenv("GTK_THEME");
  if (gtk_theme && strstr(gtk_theme, "dark")) {
    return TRUE;
  }
  char const* theme = getenv("QT_STYLE_OVERRIDE");
  if (theme && strstr(theme, "dark")) {
    return TRUE;
  }
  return FALSE;
}

bool_t
WI_GetOpenFileName(struct _WI_OpenFileName const* ofn)
{
  (void)ofn;
  return FALSE;
}

bool_t
WI_GetSaveFileName(struct _WI_OpenFileName const* ofn)
{
  (void)ofn;
  return FALSE;
}

bool_t
WI_GetFolderName(struct _WI_OpenFileName const* ofn)
{
  (void)ofn;
  return FALSE;
}

char const*
KEY_GetKeyName(uint32_t keycode)
{
  (void)keycode;
  return "";
}
