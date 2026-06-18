#include <include/api.h>
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

/* ── args ─────────────────────────────────────────────────────────────────── */
typedef struct {
  char const *plugins, *server, *test, *url, *new_proj;
} cli_args;

static cli_args g_args = {0};

static int parse_args(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    char *a = argv[i];
    if (!strcmp(a, "new") && i + 1 < argc) { g_args.new_proj = argv[++i]; continue; }
    if (!strncmp(a, "-plugins=", 9)) { g_args.plugins = a + 9; continue; }
    if (!strncmp(a, "-server=", 8))  { g_args.server = a + 8;  continue; }
    if (!strncmp(a, "-test=", 6))    { g_args.test = a + 6;    continue; }
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

/* ── requires ─────────────────────────────────────────────────────────────── */
static char const *requires[] = {
  "orca", "orca.core", "orca.geometry", "orca.filesystem",
  "orca.renderer", "orca.parsers.xml", "orca.UIKit", "orca.SceneKit", "orca.SpriteKit",
};

/* ── test runner ──────────────────────────────────────────────────────────── */
static void RunTest(lua_State *L, char const *file) {
  size_t sz = 0; char *buf = NULL;
  FILE *mem = open_memstream(&buf, &sz);
  fprintf(mem, "local orca = require 'orca'\nrequire 'orca.core'\n"
               "local filesystem = require 'orca.filesystem'\norca.init()\n");
  FOR_LOOP(i, sizeof(requires) / sizeof(*requires))
    fprintf(mem, "require '%s'\n", requires[i]);
  fprintf(mem, "filesystem.loadObject('%s')\n", file);
  fclose(mem);
  if (luaL_loadbuffer(L, buf, sz, "@main") || lua_pcall(L, 0, 1, 0)) {
    fprintf(stderr, "Uncaught exception: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1); free(buf); exit(1);
  }
  free(buf);
}

/* ── emscripten main loop ─────────────────────────────────────────────────── */
#if __EMSCRIPTEN__
static lua_State *g_L = NULL, *g_co = NULL;
static int g_co_ref = LUA_NOREF;

static void orca_main_loop_iter(void) {
  axPostMessageW(NULL, kEventWindowPaint, axGetSize(NULL), 0);
  int nres = 0, status = lua_resume(g_co, g_L, 0, &nres);
  lua_pop(g_co, nres);
  if (status == LUA_YIELD) return;
  if (status != LUA_OK)
    fprintf(stderr, "orca: Lua coroutine error: %s\n", lua_tostring(g_co, -1));
  luaL_unref(g_L, LUA_REGISTRYINDEX, g_co_ref);
  g_co = NULL; g_co_ref = LUA_NOREF;
  emscripten_cancel_main_loop();
}
#endif

/* ── main ─────────────────────────────────────────────────────────────────── */
int main(int argc, LPSTR *argv) {
  int proj_idx = parse_args(argc, argv);

  /* scaffold: orca new <name> */
  if (g_args.new_proj) return scaffold(g_args.new_proj);

  lpcString_t szProject = proj_idx ? argv[proj_idx] : NULL;
#ifndef PROJECTDIR
  if (!szProject && !g_args.test) {
    fprintf(stderr,
      "Usage: orca <project_dir>\n"
      "       orca new <project_name>\n"
      "       orca -test=<file> [options]\n"
      "Options:\n"
      "  -server=true/false    Server mode (no window)\n"
      "  -plugins=<dir>       Plugins directory\n"
      "  -url=<url>           Startup URL\n");
    return 1;
  }
#else
  if (!szProject && !g_args.test) szProject = PROJECTDIR;
#endif

  path_t projectdir = {0};
  if (szProject) {
    if (chdir_project(szProject, projectdir) < 0) {
      fprintf(stderr, "Failed to resolve/chdir to project: %s\n", szProject);
      return 1;
    }
    szProject = projectdir;
  }

  do {
    lua_State *L = luaL_newstate();
    *(void **)lua_getextraspace(L) = NULL;
    luaL_openlibs(L);

    path_t exename = {0}; get_exe(exename, sizeof(exename));
    lua_pushstring(L, exename); lua_setglobal(L, "EXENAME");
#ifdef _WIN32
    for (int i = 0; strrchr(exename, '\\') && i < 2; i++) *strrchr(exename, '\\') = '\0';
#else
    for (int i = 0; strrchr(exename, '/') && i < 2; i++) *strrchr(exename, '/') = '\0';
#endif

#if __EMSCRIPTEN__
    int luaopen_orca(lua_State *), luaL_preload(lua_State *, char const *, lua_CFunction);
    luaL_preload(L, "orca", luaopen_orca);
#endif

    /* LIBDIR */
#ifdef LIBDIR
    lua_pushstring(L, LIBDIR);
#elif __EMSCRIPTEN__
    lua_pushstring(L, axLibDirectory());
#elif defined(XCODE)
    lua_pushfstring(L, "%s/Debug", exename);
#else
    lua_pushfstring(L, "%s/lib", exename);
#endif
    lua_setglobal(L, "LIBDIR");

    /* SHAREDIR */
#ifdef SHAREDIR
    lua_pushstring(L, SHAREDIR);
#elif __EMSCRIPTEN__
    lua_pushstring(L, axShareDirectory());
#else
    lua_pushfstring(L, "%s/share", exename);
#endif
    lua_setglobal(L, "SHAREDIR");

    /* PLUGDIR */
    if (g_args.plugins) lua_pushstring(L, g_args.plugins);
    else lua_pushfstring(L, "%s/plugins", exename);
    lua_setglobal(L, "PLUGDIR");

    if (szProject) { lua_pushstring(L, szProject); lua_setglobal(L, "PROJECTDIR"); }
    if (g_args.server) { lua_pushboolean(L, 1); lua_setglobal(L, "SERVER"); }

    luaL_dostring(L, "package.path = SHAREDIR..'/?.lua;'..package.path\n"
                     "package.path = SHAREDIR..'/?/init.lua;'..package.path\n");
#ifndef __EMSCRIPTEN__
    luaL_dostring(L, "package.cpath = LIBDIR..'/lib?.so;'..package.cpath\n");
#endif

    if (szProject) { lua_pushstring(L, szProject); lua_setglobal(L, "DATADIR"); }

    if (g_args.url) { lua_pushstring(L, g_args.url); lua_setglobal(L, "STARTUP_URL"); }

    if (!g_args.test) {
      char const *bootstrap =
        "local Application = require 'orca.core.application'\n"
        "local Startup = require 'orca.core.startup'\n"
        "local app = assert(Startup.open(Application, DATADIR))\n"
        "return app:run()\n";
      if (luaL_loadstring(L, bootstrap) != LUA_OK) {
        fprintf(stderr, "%s\n", luaL_checkstring(L, -1));
        lua_close(L); break;
      }
      lua_State *co = lua_newthread(L);
      lua_rotate(L, -2, 1); lua_xmove(L, co, 1);
      int nres = 0, status;
      while ((status = lua_resume(co, L, 0, &nres)) == LUA_YIELD) lua_pop(co, nres);
      if (status != LUA_OK) { fprintf(stderr, "%s\n", lua_tostring(co, -1)); lua_close(L); break; }
      if (lua_type(co, -1) == LUA_TSTRING) {
        static path_t result; strncpy(result, lua_tostring(co, -1), sizeof(result));
        szProject = result;
      } else szProject = NULL;
    } else if (strstr(g_args.test, ".xml")) {
      RunTest(L, g_args.test);
    } else if (strstr(g_args.test, ".moon")) {
      char const *moon =
        "local ms = require 'moonscript'\n"
        "local chunk, err = ms.loadfile(...)\n"
        "if not chunk then error(err) end\n"
        "chunk()\n";
      luaL_loadstring(L, moon); lua_pushstring(L, g_args.test);
      if (lua_pcall(L, 1, 0, 0) != LUA_OK) { fprintf(stderr, "%s\n", lua_tostring(L, -1)); exit(1); }
      szProject = NULL;
    } else {
      if (luaL_dofile(L, g_args.test) != LUA_OK) { fprintf(stderr, "%s\n", lua_tostring(L, -1)); exit(1); }
      szProject = NULL;
    }

    lua_getglobal(L, "SERVER");
    if (lua_toboolean(L, -1)) g_args.server = "true";
    lua_close(L);
  } while (szProject);

  return 0;
}
