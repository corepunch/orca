#include "../platform.h"
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#define APPNAME "highperf"

static char documents[1024] = { 0 };

static PATHSTR g_share = { 0 };
static PATHSTR g_local = { 0 };
static PATHSTR g_lib = { 0 };

char const*
WI_GetPlatform(void)
{
  return "linux (wayland)";
}

char const*
WI_SettingsDirectory()
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
  int len = readlink(path, buf, sz - 1);
  buf[len] = '\0';
  char* c = buf + len - 1;
  for (int i = 1;; c--) {
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
WI_ShareDirectory()
{
  if (g_share[0] == 0) {
    WI_BundleDirectory(g_share, sizeof(g_share), "/share/" APPNAME);
  }
  return g_share;
}

char const*
WI_LibDirectory()
{
  if (g_lib[0] == 0) {
    WI_BundleDirectory(g_lib, sizeof(g_lib), "/lib/" APPNAME);
  }
  return g_lib;
}

char const*
KEY_GetKeyName(uint32_t keycode)
{
  // for (keymap_t const *km = darwin_scancode_table; km->keycode != -1;
  // km++) {
  //     if (keycode == km->keycode) {
  //         return km->keyname;
  //     }
  // }
  return "";
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
  // Try to detect dark theme from environment variables
  // This is a simplified implementation - a full implementation would
  // require checking desktop environment settings via D-Bus
  char const* gtk_theme = getenv("GTK_THEME");
  if (gtk_theme && strstr(gtk_theme, "dark")) {
    return TRUE;
  }
  
  // Check if running in a known dark theme
  char const* theme = getenv("QT_STYLE_OVERRIDE");
  if (theme && strstr(theme, "dark")) {
    return TRUE;
  }
  
  return FALSE;
}

bool_t
WI_GetOpenFileName(struct _WI_OpenFileName const* ofn)
{
  // File dialogs require GTK or Qt integration
  // For now, this is a stub that returns FALSE
  // A full implementation would use gtk_file_chooser_dialog_new() or zenity
  (void)ofn;
  return FALSE;
}

bool_t
WI_GetSaveFileName(struct _WI_OpenFileName const* ofn)
{
  // File dialogs require GTK or Qt integration
  // For now, this is a stub that returns FALSE
  (void)ofn;
  return FALSE;
}

bool_t
WI_GetFolderName(struct _WI_OpenFileName const* ofn)
{
  // File dialogs require GTK or Qt integration
  // For now, this is a stub that returns FALSE
  (void)ofn;
  return FALSE;
}
