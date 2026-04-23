//#ifdef __QNX__
//#define LUA_USE_LINUX
//#define l_likely(x) (x)
//#define l_unlikely(x) (x)
//#include "lua-master/onelua.c"
//#endif

#include <include/api.h>
#include <include/renderer.h>
#include <include/version.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#ifdef _WIN32
#include <windows.h>
#elif __EMSCRIPTEN__
#include <emscripten.h>
#elif __linux__
#include <unistd.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#endif

#define ORCA_FEATURE_DEBUG
#define DEFAULT_WINDOW_SIZE 1024, 768

struct args {
  lpcString_t plugins;
  lpcString_t server;
  lpcString_t test;
} args = {0};

struct { lpcString_t name, *target; } argsmap[] = {
  { "-plugins=", &args.plugins },
  { "-server=", &args.server },
  { "-test=", &args.test },
};

static void
get_exe_filename(LPSTR buf, uint32_t sz)
{
#if _WIN32
  uint32_t len = GetModuleFileName(NULL, buf, sz - 1);
  buf[len] = '\0';
#elif __linux__
  path_t path;
  sprintf(path, "/proc/%d/exe", getpid());
  uint32_t len = readlink(path, buf, sz - 1);
  buf[len] = '\0';
#elif __APPLE__
  uint32_t size = sz;
  _NSGetExecutablePath(buf, &size);
#else
  strcpy(buf, "./orca");
#endif
}

lpcString_t requires[] = {
  "orca",
  "orca.core",
  "orca.geometry",
  "orca.filesystem",
  "orca.renderer",
  "orca.parsers.xml",
  "orca.UIKit",
  "orca.SceneKit",
  "orca.SpriteKit",
};

#if __EMSCRIPTEN__
/* ── Emscripten callback-driven main loop ────────────────────────────────────
 * Without ASYNCIFY the Lua main loop runs as a coroutine.  RunProject loads
 * the project script into g_co and calls emscripten_set_main_loop so the
 * browser invokes orca_main_loop_iter() on every requestAnimationFrame.
 * Each frame we inject a WindowPaint event and resume the coroutine, which
 * drains the event queue and yields via coroutine.yield() when the queue is
 * empty.  Input events posted by the platform callbacks accumulate between
 * frames and are processed on the next resume.
 * --------------------------------------------------------------------------*/
static lua_State *g_L   = NULL;
static lua_State *g_co  = NULL;
static int g_co_ref = LUA_NOREF;

static void orca_main_loop_iter(void) {
  /* Drive rendering: inject a WindowPaint event every rAF tick. */
  axPostMessageW(NULL, kEventWindowPaint, axGetSize(NULL), 0);
  int nres = 0;
  int status = lua_resume(g_co, g_L, 0, &nres);
  lua_pop(g_co, nres);
  if (status == LUA_YIELD) {
    return; /* coroutine will be resumed next frame */
  }
  /* Script returned (e.g. hot-reload) or errored. */
  if (status != LUA_OK) {
    fprintf(stderr, "orca: Lua coroutine error: %s\n", lua_tostring(g_co, -1));
  }
  luaL_unref(g_L, LUA_REGISTRYINDEX, g_co_ref);
  g_co = NULL;
  g_co_ref = LUA_NOREF;
  emscripten_cancel_main_loop();
}
#endif
lpcString_t RunTest(lua_State *L, lpcString_t szFileName) {
  size_t size = 0;
  char *buf = NULL;
  FILE *mem = open_memstream(&buf, &size);
  fprintf(mem, "local orca = require 'orca'\n");
  fprintf(mem, "local filesystem = require 'orca.filesystem'\n");
  fprintf(mem, "orca.init()\n");
  FOR_LOOP(i, sizeof(requires)/sizeof(*requires)) {
    fprintf(mem, "require '%s'\n", requires[i]);
  }
  fprintf(mem, "filesystem.loadObject('%s')\n", szFileName);
  fclose(mem);
  if (luaL_loadbuffer(L, buf, size, "@main") || lua_pcall(L, 0, 1, 0)) {
    fprintf(stderr, "Uncaught exception: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    free(buf);
    exit(1);
    return NULL;
  }
  free(buf);
  return NULL;
}

int main (int argc, LPSTR *argv)
{
  lpcString_t szProject = NULL;
  
  for (uint32_t i = 1; i < argc; i++) {
    FOR_LOOP(j, sizeof(argsmap)/sizeof(*argsmap)) {
      if (!strncmp(argsmap[j].name, argv[i], strlen(argsmap[j].name))) {
        *argsmap[j].target = argv[i]+strlen(argsmap[j].name);
        goto next_arg;
      }
    }
    szProject = argv[i];
    next_arg:;
  }
  
#ifndef PROJECTDIR
  if (!szProject && !args.test) {
    Con_Error("Usage: orca [options] <project_dir>\n");
    Con_Error("Options:");
    Con_Error("\t-test=true/false         Open in Test mode");
    Con_Error("\t-server=true/false       Open in Server mode (no window)");
    Con_Error("\t-editor=true/false       Open in Editor mode");
    Con_Error("\t-plugins=<plugins_dir>   Set the plugins directory");
    return 1;
  }
#else
  if (!szProject && !args.test) {
    szProject = PROJECTDIR;
  }
#endif
  
  do {
    lua_State* L = luaL_newstate();
    
    // it seems we need to clear extraspace ourselves
    *(void**)lua_getextraspace(L) = NULL;
    
    luaL_openlibs(L);

    path_t exename = { 0 };
    get_exe_filename(exename, sizeof(exename));

    lua_pushstring(L, exename);
    lua_setglobal(L, "EXENAME");

#if _WIN32
    for (int i = 0; strrchr(exename, '\\') && i < 2; i++)
      *strrchr(exename, '\\') = '\0';
#else
    for (int i = 0; strrchr(exename, '/') && i < 2; i++)
      *strrchr(exename, '/') = '\0';
#endif

#if __EMSCRIPTEN__
    int luaopen_orca(lua_State*);
    int luaL_preload(lua_State*, lpcString_t, lua_CFunction);
    luaL_preload(L, "orca", luaopen_orca);
#endif
    
#ifdef LIBDIR
    lua_pushstring(L, LIBDIR);
    lua_setglobal(L, "LIBDIR");
#elif __EMSCRIPTEN__
    lua_pushstring(L, axLibDirectory());
    lua_setglobal(L, "LIBDIR");
#elif __linux__ || __APPLE__
#ifdef XCODE
    lua_pushfstring(L, "%s/Debug", exename);
    lua_setglobal(L, "LIBDIR");
#else
    lua_pushfstring(L, "%s/lib", exename);
    lua_setglobal(L, "LIBDIR");
#endif
#else
    lua_pushstring(L, ".");
    lua_setglobal(L, "LIBDIR");
#endif

#ifdef SHAREDIR
    lua_pushstring(L, SHAREDIR);
    lua_setglobal(L, "SHAREDIR");
#elif __EMSCRIPTEN__
    lua_pushstring(L, axShareDirectory());
    lua_setglobal(L, "SHAREDIR");
#elif __linux__ || __APPLE__
    lua_pushfstring(L, "%s/share", exename);
    lua_setglobal(L, "SHAREDIR");
#else
    lua_pushstring(L, ".");
    lua_setglobal(L, "SHAREDIR");
#endif

    if (args.plugins) {
      lua_pushstring(L, args.plugins);
    } else {
      lua_pushfstring(L, "%s/plugins", exename);
    }
    lua_setglobal(L, "PLUGDIR");
    
    if (szProject) {
      lua_pushstring(L, szProject);
      lua_setglobal(L, "PROJECTDIR");
    }
    
    if (args.server) {
      lua_pushboolean(L, TRUE);
      lua_setglobal(L, "SERVER");
    }
    
    luaL_dostring(L, "package.path = SHAREDIR..'/?.lua;'..package.path\n");
    luaL_dostring(L, "package.path = SHAREDIR..'/?/init.lua;'..package.path\n");
#ifndef __EMSCRIPTEN__
    luaL_dostring(L, "package.cpath = LIBDIR..'/lib?.so;'..package.cpath\n");
#endif

    lua_pushstring(L, szProject);
    lua_setglobal(L, "DATADIR");
    
    if (!args.test) {
      const char* bootstrap =
      "local Application = require 'orca.core.application'\n"
      "local app = assert(Application.open(DATADIR))\n"
      "return app:run()\n";
//      lua_getglobal(L, "require");
//      lua_pushstring(L, "orca.core.application");
//      lua_pcall(L, 1, 1, 0);
      if (luaL_dostring(L, bootstrap) != LUA_OK) {
        fprintf(stderr, "%s\n", luaL_checkstring(L, -1));
        lua_close(L);
        break;
      }
      if (lua_type(L, -1) == LUA_TSTRING) {
        static path_t result;
        strncpy(result, luaL_checkstring(L, -1), sizeof(result));
        szProject = result;
      } else {
        szProject = NULL;
      }
    } else if (strstr(args.test, ".xml")) {
      RunTest(L, args.test);
    } else {
      if (strstr(args.test, ".moon")) {
        const char *moon_bootstrap =
          "local ms = require 'moonscript'\n"
          "local chunk, err = ms.loadfile(...)\n"
          "if not chunk then error(err) end\n"
          "chunk()\n";
        luaL_loadstring(L, moon_bootstrap);
        lua_pushstring(L, args.test);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
          fprintf(stderr, "%s\n", lua_tostring(L, -1));
          exit(1);
        }
      } else if (luaL_dofile(L, args.test) != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        exit(1);
      }
      szProject = NULL;
    }

    lua_getglobal(L, "SERVER");
    if (lua_toboolean(L, -1)) {
      args.server = "true";
    }

    lua_close(L);
  } while (szProject);
  
  return 0;
}
