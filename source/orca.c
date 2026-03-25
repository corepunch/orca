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
  WI_PostMessageW(NULL, kEventWindowPaint, WI_GetSize(NULL), 0);
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
  fprintf(mem, "orca.init()\n");
  FOR_LOOP(i, sizeof(requires)/sizeof(*requires)) {
    fprintf(mem, "require '%s'\n", requires[i]);
  }
  fprintf(mem, "doxmlfile('%s')\n", szFileName);
  fclose(mem);
  if (luaL_loadbuffer(L, buf, size, "@main") || lua_pcall(L, 0, 1, 0)) {
    fprintf(stderr, "Uncaught exception: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    free(buf);
    return NULL;
  }
  free(buf);
  return NULL;
}

lpcString_t RunProject(lua_State *L, lpcString_t szDirName) {
  size_t size = 0;
  char *buf = NULL;
  path_t path={0};
  int windowsize[] = { DEFAULT_WINDOW_SIZE };
  snprintf(path, sizeof(path), "%s/%s", szDirName, ORCA_PACKAGE_NAME);
  WITH(xmlDoc, doc, xmlReadFile(path, NULL, 0), xmlFreeDoc) {
    FILE *mem = open_memstream(&buf, &size);

    fprintf(mem, "local orca = require 'orca'\n");
    
    // load plugins
    fprintf(mem, "orca.init()\n");

    FOR_LOOP(i, sizeof(requires)/sizeof(*requires)) {
      fprintf(mem, "require '%s'\n", requires[i]);
    }
    fprintf(mem, "local core = require 'orca.core'\n");
    fprintf(mem, "local fs = require 'orca.filesystem'\n");
    fprintf(mem, "local loc = require 'orca.localization'\n");
    fprintf(mem, "local ref = require 'orca.renderer'\n");
    fprintf(mem, "local s = require 'orca.backend'\n");
    fprintf(mem, "local ui = require 'orca.UIKit'\n");

    xmlNodePtr root = xmlDocGetRootElement(doc);

//    lua_getglobal(L, "DATADIR");
//    lpcString_t DATADIR = luaL_checkstring(L, -1);

    WITH(xmlChar, Name, xmlGetProp(root, XMLSTR("Name")), xmlFree) {
      lua_pushstring(L, (lpcString_t)Name);
      lua_setglobal(L, "PROJECTNAME");
    }

    WITH(xmlChar, WindowWidth, xmlGetProp(root, XMLSTR("WindowWidth")), xmlFree) {
      windowsize[0] = atoi((char*)WindowWidth);
    }

    WITH(xmlChar, WindowHeight, xmlGetProp(root, XMLSTR("WindowHeight")), xmlFree) {
      windowsize[1] = atoi((char*)WindowHeight);
    }

    // initialize
    fprintf(mem, "ref.init(%d, %d, %s)\n", windowsize[0], windowsize[1], args.server ? args.server : "false");
    fprintf(mem, "fs.init('%s')\n", szDirName);
    
    xmlFindAll(loc, root, XMLSTR("LocaleLibrary"))
    xmlFindAllText(path, content, loc, XMLSTR("LocaleReferenceItem")) {
      fprintf(mem, "loc.load '%s'\n", content);
    }

    WITH(xmlChar, StartupScreen, xmlGetProp(root, XMLSTR("StartupScreen")), xmlFree) {
//      fprintf(mem, "local Screen = require '%s'\n", StartupScreen);
//      fprintf(mem, "local screen = Screen()\n");
      fprintf(mem, "local ok, screen = pcall(require, '%s')\n", StartupScreen);
      fprintf(mem, "if not ok then\n");
      fprintf(mem, "\tlocal err = screen\n");
      fprintf(mem, "\tscreen = ui.Screen { Width = %d, Height = %d }\n", windowsize[0], windowsize[1]);
      fprintf(mem, "\tlocal term = ui.TerminalView {\n");
      fprintf(mem, "\tBufferWidth=screen.Width/8,\n");
      fprintf(mem, "\tBufferHeight=screen.Height/16}\n");
      fprintf(mem, "\tterm:println(nil, err)\n");
      fprintf(mem, "\tscreen:addChild(term)\n");
      fprintf(mem, "\tprint(err)\n");
//      fprintf(mem, "screen:addChild(ui.TextBlock(err))\n");
//      fprintf(mem, "else screen = screen.__userdata and screen or screen()\n");
      fprintf(mem, "else\n");
      fprintf(mem, "screen = screen()\n");
      fprintf(mem, "for k, v in pairs(orca.styles) do screen:addStyleSheet(k, v) end\n");
      fprintf(mem, "end\n");
  }
#if defined(ORCA_FEATURE_DEBUG) && !defined(__EMSCRIPTEN__)
    fprintf(mem, "local editor = require 'orca.editor'\n");
    fprintf(mem, "editor.setScreen(screen)\n");
#endif
//    fprintf(mem, "print 'Project %s was initialized'\n", szProjectPath);
    fprintf(mem, "while true do for msg in s.getMessage(screen) do\n");
    //    fprintf(mem, "if m:is'KeyDown' and m.key=='q' then\nreturn 'closed'\nelse");
    //    fprintf(mem, "if m:is'WindowClosed' then\nreturn 'closed'\nelse");
    //    fprintf(mem, "if m:is'RequestReload' then\nwindow:refresh()\nelse");
    bool_t has_written = FALSE;
    fprintf(mem, "fs.trackChangedFiles()\n");
    fprintf(mem, "if RELOAD then return RELOAD end\n");
    xmlFindAll(messages, root, XMLSTR("Project.SystemMessages")) {
      xmlFindAll(message, messages, XMLSTR("SystemMessage")) {
        fprintf(mem, "if");
        bool written = FALSE;
        FOR_EACH_LIST(xmlAttr, attr, message->properties) {
          if (!xmlStrcmp(attr->name, XMLSTR("Name"))) continue;
          if (!xmlStrcmp(attr->name, XMLSTR("Command"))) continue;
          WITH(xmlChar, value, xmlNodeGetContent((xmlNode*)attr), xmlFree) {
            if (written) {
              fprintf(mem, " and");
            }
            char tmp[64];
            strncpy(tmp, (char*)attr->name, sizeof(tmp));
            *tmp=tolower(*tmp);
            if (!xmlStrcmp(attr->name, XMLSTR("Message"))) {
              fprintf(mem, " msg:is '%s'", value);
            } else if (isdigit(*value)) {
              fprintf(mem, " msg.%s == %s", tmp, value);
            } else {
              fprintf(mem, " msg.%s == '%s'", tmp, value);
            }
            written = TRUE;
          }
        }
        WITH(xmlChar, content, xmlGetProp(message, XMLSTR("Command")), xmlFree) {
          fprintf(mem, " then\n%s\nelse", content);
        }
        has_written = TRUE;
      }
    }
    fprintf(mem, "\nlocal ok, res = pcall(s.dispatchMessage, msg)\n");
    fprintf(mem, "if not ok then print(res) screen:clear() screen:addChild(ui.TextBlock(res))\n");
//    fprintf(mem, "\nif s.dispatchMessage(msg) then screen:paint(ref.getSize()) end \n");
    fprintf(mem, "\nelseif res and not msg:is 'WindowPaint' then\n");
    fprintf(mem, "\t\tscreen:postMessage 'WindowPaint' end \n");
    if (has_written) {
      fprintf(mem, "end\n");
    }
#if __EMSCRIPTEN__
    /* On Emscripten the main loop is driven by orca_main_loop_iter() via
       emscripten_set_main_loop.  After draining all queued events the
       coroutine yields back to the browser so input callbacks can fire. */
    fprintf(mem, "end\ncoroutine.yield()\nend\n");
#else
    fprintf(mem, "end end\n");
#endif
    fprintf(mem, "ref.shutdown()\n");
    fclose(mem);
  }
#if __EMSCRIPTEN__
  if (luaL_loadbuffer(L, buf, size, "@main")) {
    fprintf(stderr, "Uncaught exception: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    free(buf);
    return NULL;
  }
  free(buf);
  g_L     = L;
  g_co    = lua_newthread(L);
  g_co_ref = luaL_ref(L, LUA_REGISTRYINDEX); /* pin coroutine against GC */
  lua_xmove(L, g_co, 1);                     /* move compiled chunk to co */
  emscripten_set_main_loop(orca_main_loop_iter, 0, 1); /* never returns */
  return NULL;
#else
  if (luaL_loadbuffer(L, buf, size, "@main") || lua_pcall(L, 0, 1, 0)) {
    fprintf(stderr, "Uncaught exception: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    free(buf);
    return NULL;
  }
  if (lua_type(L, -1) == LUA_TSTRING) {
    static path_t result;
    strncpy(result, luaL_checkstring(L, -1), sizeof(result));
    return result;
  }
  free(buf);
  return NULL;
#endif
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
    lua_pushstring(L, WI_LibDirectory());
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
    lua_pushstring(L, WI_ShareDirectory());
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
      "local core = require 'orca.core2'\n"
      "core.init()\n";
//      lua_getglobal(L, "require");
//      lua_pushstring(L, "orca.core2");
//      lua_pcall(L, 1, 1, 0);
      if (luaL_dostring(L, bootstrap) != LUA_OK) {
        fprintf(stderr, "%s", luaL_checkstring(L, -1));
      }
//      szProject = RunProject(L, szProject);
      exit(0);
    } else if (strstr(args.test, ".xml")) {
      RunTest(L, args.test);
    } else if (luaL_dofile(L, args.test) != LUA_OK) {
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
      szProject = NULL;
    } else {
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
