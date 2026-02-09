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
#elif __linux__
#include <unistd.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#endif

struct args {
  lpcString_t plugins;
  lpcString_t server;
} args = {0};

struct { lpcString_t name, *target; } argsmap[] = {
  { "-plugins=", &args.plugins },
  { "-server=", &args.server },
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

lpcString_t RunProject(lua_State *L, lpcString_t szDirname) {
  lpcString_t requires[] = {
    "orca",
    "orca.core",
    "orca.geometry",
    "orca.filesystem",
    "orca.renderer",
    "orca.parsers.xml",
    "orca.ui",
#ifdef ORCA_FEATURE_3D
    "orca.SceneKit",
#endif
  };
  size_t size = 0;
  LPSTR buf = NULL;
  path_t path={0};
  snprintf(path, sizeof(path), "%s/%s", szDirname, ORCA_PACKAGE_NAME);
  xmlWith(xmlDoc, doc, xmlReadFile(path, NULL, 0), xmlFreeDoc) {
    FILE *mem = open_memstream(&buf, &size);
    FOR_LOOP(i, sizeof(requires)/sizeof(*requires)) {
      fprintf(mem, "require '%s'\n", requires[i]);
    }
    fprintf(mem, "local orca = require 'orca'\n");
    fprintf(mem, "local sys = require 'orca.system'\n");
    fprintf(mem, "local fs = require 'orca.filesystem'\n");
    fprintf(mem, "local loc = require 'orca.localization'\n");
    fprintf(mem, "local ref = require 'orca.renderer'\n");
    fprintf(mem, "local s = require 'orca.backend'\n");

    xmlNodePtr root = xmlDocGetRootElement(doc);

//    lua_getglobal(L, "DATADIR");
//    lpcString_t DATADIR = luaL_checkstring(L, -1);

    xmlWith(xmlChar, Name, xmlGetProp(root, XMLSTR("Name")), xmlFree) {
      lua_pushstring(L, (lpcString_t)Name);
      lua_setglobal(L, "PROJECTNAME");
    }

    fprintf(mem, "fs.init '%s'\n", szDirname);

    xmlFindAll(loc, root, XMLSTR("LocaleLibrary"))
    xmlFindAllText(path, content, loc, XMLSTR("LocaleReferenceItem")) {
      fprintf(mem, "loc.load '%s'\n", content);
    }

    xmlWith(xmlChar, StartupScreen, xmlGetProp(root, XMLSTR("StartupScreen")), xmlFree) {
      fprintf(mem, "local Screen = require '%s'\n", StartupScreen);
      fprintf(mem, "local screen = Screen()\n");
    }

    fprintf(mem, "local editor = require 'orca.editor'\n");
    fprintf(mem, "editor.setScreen(screen)\n");

//    fprintf(mem, "print 'Project %s was initialized'\n", szProjectPath);
    fprintf(mem, "while true do for msg in s.getMessage(screen) do\n");
    //    fprintf(mem, "if m:is'KeyDown' and m.key=='q' then\nreturn 'closed'\nelse");
    //    fprintf(mem, "if m:is'WindowClosed' then\nreturn 'closed'\nelse");
    //    fprintf(mem, "if m:is'RequestReload' then\nwindow:refresh()\nelse");
    bool_t has_written = FALSE;
    fprintf(mem, "fs.trackChangedFiles()\n");
    fprintf(mem, "if RELOAD then return RELOAD end\n");
    xmlFindAll(messages, root, XMLSTR("MessageLibrary")) {
      xmlFindAll(message, messages, XMLSTR("SystemMessage")) {
        fprintf(mem, "if");
        bool written = FALSE;
        FOR_EACH_LIST(xmlAttr, attr, message->properties) {
          if (!xmlStrcmp(attr->name, XMLSTR("Name"))) {
            continue;
          }
          xmlWith(xmlChar, value, xmlNodeGetContent((xmlNode*)attr), xmlFree) {
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
        xmlWith(xmlChar, content, xmlNodeGetContent(message), xmlFree) {
          fprintf(mem, " then\n%s\nelse", content);
        }
        has_written = TRUE;
      }
    }
//    fprintf(mem, "\ns.dispatchMessage(msg)");
    fprintf(mem, "\nif s.dispatchMessage(msg) then screen:paint(ref.getSize()) end \n");
    if (has_written) {
      fprintf(mem, "end\n");
    }
    fprintf(mem, "end end\n");
    fclose(mem);
  }
  if (luaL_loadbuffer(L, buf, size, "@main") || lua_pcall(L, 0, 1, 0)) {
    Con_Error("Uncaught exception: %s", lua_tostring(L, -1));
    lua_pop(L, 1);
    return NULL;
  }
  if (lua_type(L, -1) == LUA_TSTRING) {
    static path_t result;
    strncpy(result, luaL_checkstring(L, -1), sizeof(result));
    return result;
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
  
  if (!szProject) {
    Con_Error("Usage: orca [options] <project_dir>\n");
    Con_Error("Options:");
    Con_Error("\t-editor=true/false       Open in Editor mode");
    Con_Error("\t-plugins=<plugins_dir>   Set the plugins directory");
    return 1;
  }
  
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
    
#if __linux__
    lua_pushfstring(L, "%s/lib", exename);
    lua_setglobal(L, "LIBDIR");
    lua_pushfstring(L, "%s/share", exename);
    lua_setglobal(L, "SHAREDIR");
#elif __APPLE__
#ifdef XCODE
    lua_pushfstring(L, "%s/Debug", exename);
    lua_setglobal(L, "LIBDIR");
#else
//    lua_pushfstring(L, "%s/lib/darwin", exename);
    lua_pushfstring(L, "%s/lib", exename);
    lua_setglobal(L, "LIBDIR");
#endif
    lua_pushfstring(L, "%s/share", exename);
    lua_setglobal(L, "SHAREDIR");
#else
    lua_pushfstring(L, ".");
    lua_setglobal(L, "LIBDIR");
    lua_pushfstring(L, ".");
    lua_setglobal(L, "SHAREDIR");
#endif

    if (args.plugins) {
      lua_pushstring(L, args.plugins);
    } else {
      lua_pushfstring(L, "%s/plugins", exename);
    }
    lua_setglobal(L, "PLUGDIR");
    
    lua_pushstring(L, szProject);
    lua_setglobal(L, "PROJECTDIR");
    
    if (args.server) {
      lua_pushboolean(L, TRUE);
      lua_setglobal(L, "SERVER");
    }
    
    luaL_dostring(L, "package.path = PLUGDIR..'/?.lua;'..package.path\n");
    luaL_dostring(L, "package.cpath = LIBDIR..'/lib?.so;'..package.cpath\n");

    lua_pushstring(L, szProject);
    lua_setglobal(L, "DATADIR");
    
    szProject = RunProject(L, szProject);

    lua_getglobal(L, "SERVER");
    if (lua_toboolean(L, -1)) {
      args.server = "true";
    }

    lua_close(L);
  } while (szProject);
  
  return 0;
}
