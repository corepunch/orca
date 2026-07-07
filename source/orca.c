#include <include/orca.h>
#include <include/renderer.h>
#include <include/version.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#elif __EMSCRIPTEN__
#include <emscripten.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#endif
#if defined(__linux__) || defined(__APPLE__) || defined(__EMSCRIPTEN__)
#include <unistd.h>
#endif

#define DEFAULT_WINDOW_SIZE 1024, 768

/* ── extern declarations ──────────────────────────────────────────────────── */
struct AXmessage;
struct Object *FS_LoadProject(const char *path);
bool_t SV_DispatchMessage(struct AXmessage *msg);

/* ── args ─────────────────────────────────────────────────────────────────── */
typedef struct {
  char const *plugins, *server, *url, *new_proj;
} cli_args;

static cli_args g_args = {0};

static int parse_args(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    char *a = argv[i];
    if (!strcmp(a, "new") && i + 1 < argc) { g_args.new_proj = argv[++i]; continue; }
    if (!strncmp(a, "-plugins=", 9)) { g_args.plugins = a + 9; continue; }
    if (!strncmp(a, "-server=", 8))  { g_args.server = a + 8;  continue; }
    if (!strncmp(a, "-url=", 5))     { g_args.url = a + 5;     continue; }
    if (a[0] != '-') return i;
  }
  return 0;
}

/* ── helpers ──────────────────────────────────────────────────────────────── */
static void get_exe(LPSTR buf, uint32_t sz) {
#ifdef _WIN32
  buf[GetModuleFileName(NULL, buf, sz - 1)] = '\0';
#elif __linux__
  char path[256]; sprintf(path, "/proc/%d/exe", getpid());
  buf[readlink(path, buf, sz - 1)] = '\0';
#elif __APPLE__
  uint32_t size = sz; _NSGetExecutablePath(buf, &size);
#else
  strcpy(buf, "./orca");
#endif
}

static int path_is_abs(char const *p) {
  if (!p || !*p) return 0;
#ifdef _WIN32
  return (isalpha((unsigned char)p[0]) && p[1] == ':') || (p[0] == '\\' && p[1] == '\\');
#else
  return p[0] == '/';
#endif
}

static int chdir_project(char const *input, path_t out) {
  if (!input || !*input) return -1;
  if (path_is_abs(input)) strncpy(out, input, sizeof(path_t) - 1);
  else {
    char cwd[1024] = {0};
#ifdef _WIN32
    if (!_getcwd(cwd, sizeof(cwd))) return -1;
    snprintf(out, sizeof(path_t), "%s\\%s", cwd, input);
#else
    if (!getcwd(cwd, sizeof(cwd))) return -1;
    snprintf(out, sizeof(path_t), "%s/%s", cwd, input);
#endif
  }
#ifdef _WIN32
  return _chdir(out) ? -1 : 0;
#else
  return chdir(out) ? -1 : 0;
#endif
}

/* ── scaffold ─────────────────────────────────────────────────────────────── */
static int scaffold(char const *name) {
  path_t dir; snprintf(dir, sizeof(dir), "%s", name);
  struct stat st;
  if (!stat(dir, &st)) { fprintf(stderr, "error: '%s' already exists\n", dir); return 1; }

  char screens[512], images[512];
  snprintf(screens, sizeof(screens), "%s/Screens", dir);
  snprintf(images, sizeof(images), "%s/Images", dir);

#ifdef _WIN32
  _mkdir(dir); _mkdir(screens); _mkdir(images);
#else
  mkdir(dir, 0755); mkdir(screens, 0755); mkdir(images, 0755);
#endif

  /* package.xml */
  char pkgpath[512]; snprintf(pkgpath, sizeof(pkgpath), "%s/package.xml", dir);
  FILE *f = fopen(pkgpath, "w");
  if (f) {
    fprintf(f,
      "<?xml version=\"1.0\"?>\n"
      "<Project Name=\"%s\" StartupScreen=\"%s/Screens/Main\">\n"
      "  <ScreenLibrary Name=\"Screens\" IsExternal=\"true\"/>\n"
      "  <ImageLibrary Name=\"Images\" IsExternal=\"true\"/>\n"
      "  <Project.SystemMessages>\n"
      "    <SystemMessage Message=\"KeyDown\" Key=\"q\" Command=\"return\"/>\n"
      "    <SystemMessage Message=\"WindowClosed\" Command=\"return\"/>\n"
      "  </Project.SystemMessages>\n"
      "</Project>\n", name, name);
    fclose(f);
  }

  /* Screens/Main.xml */
  char scrpath[512]; snprintf(scrpath, sizeof(scrpath), "%s/Screens/Main.xml", dir);
  f = fopen(scrpath, "w");
  if (f) {
    fprintf(f,
      "<?xml version=\"1.0\"?>\n"
      "<Screen Name=\"Main\" Width=\"1024\" Height=\"768\">\n"
      "  <TextBlock Text=\"Hello, %s!\" FontSize=\"48\" LayoutTransform=\"400 350 0 1 1\"/>\n"
      "</Screen>\n", name);
    fclose(f);
  }

  printf("Created '%s/'\n"
         "  package.xml\n"
         "  Screens/Main.xml\n"
         "  Images/\n\n"
         "Run:  make run PROJECT=%s/package.xml\n", dir, dir);
  return 0;
}

/* ── main ─────────────────────────────────────────────────────────────────── */
int main(int argc, LPSTR *argv) {
  int proj_idx = parse_args(argc, argv);

  /* scaffold: orca new <name> */
  if (g_args.new_proj) return scaffold(g_args.new_proj);

  lpcString_t szProject = proj_idx ? argv[proj_idx] : NULL;
#ifndef PROJECTDIR
  if (!szProject) {
    fprintf(stderr,
      "Usage: orca <project_dir>\n"
      "       orca new <project_name>\n"
      "Options:\n"
      "  -server=true/false    Server mode (no window)\n"
      "  -plugins=<dir>       Plugins directory\n"
      "  -url=<url>           Startup URL\n");
    return 1;
  }
#else
  if (!szProject) szProject = PROJECTDIR;
#endif

  path_t projectdir = {0};
  if (szProject) {
    if (chdir_project(szProject, projectdir) < 0) {
      fprintf(stderr, "Failed to resolve/chdir to project: %s\n", szProject);
      return 1;
    }
    szProject = projectdir;
  }

  if (!FS_LoadProject(szProject)) {
    fprintf(stderr, "Failed to load project: %s\n", szProject);
    return 1;
  }

  struct AXmessage msg;
  while (axGetMessage(&msg))
    SV_DispatchMessage(&msg);

  return 0;
}
