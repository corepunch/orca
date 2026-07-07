# Remove Lua from Orca Engine — Master Plan

**Branch:** `feature/remove-lua`
**Goal:** Replace all Lua scripting with pure C — XML config, C controller registry, C function pointer dispatch.

---

## Phase 0 — Headers & Type Signatures (foundation)

All later phases depend on these signature changes. Do these first.

### 0.1 `include/orca.h`
- [ ] Remove `typedef struct lua_State lua_State;`
- [ ] `PROP_Create`: remove `lua_State*` first param
- [ ] `SV_RegisterMessageProc` / `SV_UnregisterMessageProc`: change proc type from `LRESULT (*)(lua_State*, struct AXmessage*)` → `LRESULT (*)(struct AXmessage*)`
- [ ] `SV_DispatchMessage`: remove `lua_State*` param
- [ ] Remove `OBJ_SetShorthandValueFromLua` declaration
- [ ] Remove `OBJ_PushShorthandValue` declaration
- [ ] `propertyParser_t`: remove `lua_State*` param
- [ ] Remove `kEventResumeCoroutine`, `kEventStopCoroutine` defines
- [ ] Remove `OBJ_GetScriptHandle` macro

### 0.2 `include/api.h`
- [ ] Replace entire file with passthrough to `orca.h`:
  ```c
  #ifndef __api_h__
  #define __api_h__
  #include <include/orca.h>
  #endif
  ```

### 0.3 `include/codegen.h`
- [ ] `ENUM(NAME, ...)`: remove `luaX_checkNAME` / `luaX_pushNAME`, keep string array + `ToString` only
- [ ] `STRUCT(NAME, EXPORT)`: remove Lua metatable/luaopen/push/check machinery, keep struct descriptor registration
- [ ] `STRUCT_PARSER(NAME, EXPORT, PARSER)`: same treatment, add `.Parser` field
- [ ] `REGISTER_CLASS(NAME, ...)`: remove `luaX_pushNAME` / `luaX_checkNAME`, keep `ClassDesc` only
- [ ] `REGISTER_MESSAGE_ACTION`: same treatment

### 0.4 `source/core/core_local.h`
- [ ] `#include <include/api.h>` → `#include <include/orca.h>`
- [ ] Remove `lua_safecall` macro
- [ ] Remove `CORE_COMMANDS`, `CORE_KEMAP` defines
- [ ] Remove `struct lua_State;` forward decl
- [ ] Update `PROP_Create`, `OBJ_AddComponentProperty` signatures (remove `lua_State*`)
- [ ] Remove `PROP_RegisterChangedCallback` declaration
- [ ] Remove `UI_ProcessCommands` declaration
- [ ] Remove `API_PrintStackTrace` declaration

### 0.5 `source/core/object/object_internal.h`
- [ ] Remove `body_ref` and `controller_ref` fields from `struct Object`
- [ ] Update includes if circular dependency exists

---

## Phase 1 — Core Engine Files

### 1.1 `source/orca.c` — Bootstrap rewrite
- [ ] Remove all `luaL_newstate`, `luaL_openlibs`, `lua_resume`, `lua_pcall`, `lua_newthread`, `lua_close`
- [ ] Remove all `lua_setglobal`, `luaL_dostring`, `luaL_loadstring`, package.path manipulation
- [ ] Remove `RunTest()` Lua test runner entirely
- [ ] Remove moonscript support, Emscripten coroutine, `-test=` handling
- [ ] Remove the `do-while` hot-reload loop
- [ ] New run logic: call `FS_LoadProject(projectpath)`, then loop `axGetMessage(&msg)` → `SV_DispatchMessage(&msg)`
- [ ] Keep: arg parsing, `scaffold()`, `get_exe()`, `path_is_abs()`, `chdir_project()`

### 1.2 `source/core/object/object_core.c` — Controller registry
- [ ] Remove `wire_controller` function entirely
- [ ] Remove `OBJ_Rebuild` (Lua body re-execution)
- [ ] Remove `OBJ_SetContext` (Lua extraspace)
- [ ] Replace `OBJ_LoadController` with C controller registry:
  - `OBJ_RegisterController(name, factory_fn)`
  - `OBJ_FindControllerFactory(name)`
  - `OBJ_LoadController(self, path)` — looks up and calls factory
- [ ] `OBJ_AddComponentByName`: remove `lua_State*`, replace `luaL_error` → `Con_Error`
- [ ] In `OBJ_Release`: remove `body_ref` / `controller_ref` cleanup blocks

### 1.3 `source/core/core_main.c` — Message loop cleanup
- [ ] `CORE_ProcessMessage`: remove `lua_State*` param
- [ ] Remove `kEventResumeCoroutine` case (lua_resume)
- [ ] Remove `kEventStopCoroutine` case (luaL_unref)
- [ ] Remove `kEventKeyDown` `CORE_KEMAP` lookup (lua registry hotkey dispatch)
- [ ] `PROP_FireNotification` call: remove `lua_State*` arg
- [ ] Remove `lua_pushclass` function
- [ ] Remove `f_addCommands`, `f_executeCommand`, `f_addHotKeys`
- [ ] Remove `f_registerPropertyType`
- [ ] Remove `before_core_module_registered`, `after_core_module_registered`
- [ ] Remove `API_PrintStackTrace`

### 1.4 `plugins/UIKit/UIKit_message.c` — Event dispatch rewrite
- [ ] Remove `luaX_getobjectcallback` extern and all Lua callback dispatch
- [ ] Remove `push_object_message_arg` (Lua userdata construction)
- [ ] Remove `f_beginDraggingSession` (Lua API)
- [ ] Define `ControllerCallback` typedef for C function pointer dispatch
- [ ] `CORE_HandleObjectMessage`: replace Lua `orca.async` dispatch with direct C callback lookup via event property
- [ ] `process_dragndrop`: replace Lua registry drag session with static `g_drag_session` struct
- [ ] `UI_HandleMouseEvent`, `UI_HandleKeyEvent`, `ui_handle_event`: remove `lua_State*`
- [ ] Remove `kEventResumeCoroutine` / `kEventStopCoroutine` cases

### 1.5 `source/core/property/property_core.c`
- [ ] `_PropertyAlloc`: remove `lua_State*`
- [ ] `PROP_Create`: remove `lua_State*`
- [ ] `OBJ_AddComponentProperty`: remove `lua_State*`
- [ ] Replace `luaL_error` → `Con_Error`

### 1.6 `source/core/property/property_events.c`
- [ ] Remove `invoke_changed_callback_from_property_events` (Lua function ref invocation)
- [ ] `PROP_FireNotification`: remove `lua_State*`, keep only `ID_PropertyChangedMessage` posting
- [ ] Remove `PROP_RegisterChangedCallback`

---

## Phase 2 — Delete Lua Bridge Files

### 2.1 Delete bridge source files
- [ ] `source/core/object/object_lua.c`
- [ ] `source/core/object/object_lua_msg.c`
- [ ] `source/core/object/object_lua_props.c`
- [ ] `source/core/property/property_lua.c`
- [ ] `source/orca/orcalib.c`

### 2.2 Delete Lua framework files
- [ ] `share/orca/core/application.lua`
- [ ] `share/orca/core/startup.lua`
- [ ] `share/orca/core/router.lua`
- [ ] `share/orca/core/async.lua`
- [ ] `share/orca/core/widget.lua`
- [ ] `share/orca/core/object.lua`
- [ ] `share/orca/core/project.lua`
- [ ] `share/orca/application.lua` (if exists)
- [ ] `share/orca/async.lua` (if exists)
- [ ] `share/orca/object.lua` (if exists)
- [ ] `share/orca/router.lua` (if exists)
- [ ] `share/orca/startup.lua` (if exists)
- [ ] `share/orca/widget.lua` (if exists)
- [ ] `share/orca/project.lua` (if exists)

---

## Phase 3 — Strip Remaining Source Files

### 3.1 Filesystem
- [ ] `source/filesystem/w_filesystem.c` — remove `f_find_module`, `luaopen_orca_filesystem`, all Lua wrappers
- [ ] `source/filesystem/io_open.c` — remove entirely (Lua `io.open` override, dead without Lua)
- [ ] `source/filesystem/popen2.c` — remove `luaopen_orca_pipe`
- [ ] `source/filesystem/Package.c` — rewrite `Package_LoadProject` to read `package.xml` via libxml2
- [ ] `source/filesystem/Directory.c` — rewrite `Directory_LoadProject` to use `FS_LoadProjectFromXml()`

### 3.2 Sysutil
- [ ] `source/sysutil/backend.c` — remove `push_event_message`, `f_get_message`, `f_peek_message`, `f_dispatch_message`, `f_translate_message`, `f_event_index/message/is/new`, Lua Event metatable; update `SV_DispatchMessage` and `message_proc_t` signatures; remove `kEventClearReference`
- [ ] `source/sysutil/queue.c` — remove any `lua_State*` usage
- [ ] `source/sysutil/w_system.c` — remove `luaopen_orca_system` and all Lua wrappers

### 3.3 Parsers
- [ ] `source/parsers/p_xml.c` — remove `luaopen_orca_parsers_xml` and Lua wrappers, keep C XML parsing
- [ ] `source/parsers/p_json.c` — remove `luaopen_orca_parsers_json` and all Lua wrappers

### 3.4 Renderer
- [ ] `source/renderer/r_main.c` — remove `renderer_gc`, `on_renderer_module_registered`, `luaX_pushViewDef`
- [ ] `source/renderer/r_shape.c` — remove `API_LoadLineTrajectory` (Lua table reader)
- [ ] `source/renderer/api/api_image.c` — remove `luaopen_renderer_image` and Lua wrappers
- [ ] `source/renderer/api/api_userdata.c` — remove `API_UserdateNew`, `API_UserdateGc`, `luaopen_renderer_userdata`
- [ ] `source/renderer/api/api_renderer.c` — remove `lua_State*` params and Lua calls
- [ ] `source/renderer/r_local.h` — remove `struct lua_State;` forward decl

### 3.5 Components & Objects
- [ ] `source/core/components/PropertyAnimation.c` — remove `OBJ_DoTween` Lua arg parsing or the function entirely
- [ ] `source/core/components/AnimationPlayer.c` — remove `lua_State*` params
- [ ] `source/core/components/StyleController.c` — remove `lua_State*` params
- [ ] `source/core/object/object_properties.c` — remove `lua_State*` params and `lua_*` calls
- [ ] `source/core/object/object_component.c` — remove `lua_State*` params
- [ ] `source/core/property/property_runtime.c` — remove `lua_State*` params
- [ ] `source/core/debug.c` — remove Lua debug functions

---

## Phase 4 — Code Generation

### 4.1 `tools/codegen/plugins/export.c`
- [ ] Remove `emit_luaopen` → replace with `emit_module_init` (void init function, no `lua_State*`)
- [ ] Remove method generation pattern `int f_method(lua_State *L)` — generate direct C signatures
- [ ] Remove `extern void read_property(lua_State *L, ...)` / `write_property(lua_State *L, ...)` declarations
- [ ] Remove `extern void luaX_push*/check*` declarations
- [ ] Remove all `luaL_newlib`, `luaL_setfuncs`, `luaL_newmetatable`, `lua_setfield`, `lua_pushcfunction` patterns

### 4.2 `tools/codegen/plugins/header.c`
- [ ] Remove any `luaX_*` declarations it generates in headers

### 4.3 `tools/codegen/include/cg_api.h`
- [ ] Remove `lua_State` from codegen API

### 4.4 `tools/codegen/src/model.c`
- [ ] Remove any `lua_State` usage

---

## Phase 5 — Build System & Config

### 5.1 `Makefile`
- [ ] Remove `lua5.4` / `lua` from `pkg-config` CFLAGS and LDFLAGS
- [ ] Remove `INST_LIBDIR` / `INST_LUADIR` (LuaRocks paths)
- [ ] Remove `HEADLESS_LUA_TESTS` variable
- [ ] Remove all `LUA_TEST_RULE` invocations (~25 lines)
- [ ] Remove `LUA_TEST_RULE` macro definition
- [ ] Update `test-headless` target: remove Lua test dependencies
- [ ] Update `test` target if it references Lua tests
- [ ] Remove `.lua` install rules (lines installing `behaviour.lua`, `main.lua`)
- [ ] Update `.PHONY`: remove Lua test names
- [ ] Keep: `copyshare`, all C tests, all non-Lua targets

### 5.2 `Makefile.webgl`
- [ ] Remove Lua references (if any)

### 5.3 `premake5.lua`
- [ ] Remove Lua dependency references (if any)

---

## Phase 6 — Sample Project Conversion

### 6.1 `samples/Example/package.lua` → `package.xml`
- [ ] Read `package.lua` to capture all values
- [ ] Write `package.xml` with equivalent XML structure:
  - `<Project>` root element with `Name`, `StartupScreen`, `WindowWidth`, `WindowHeight`
  - `<ScreenLibrary>`, `<ImageLibrary>`, `<PrefabLibrary>` child elements
  - `<Project.ThemeLibrary>` with `<ThemeEntry>` children
  - `<Project.SystemMessages>` with `<SystemMessage>` children
  - `<Project.DataSourceLibrary>` with `<XmlDataSource>` children
  - `<Project.EnginePlugins>` with `<EnginePlugin>` children
- [ ] Delete `package.lua`

---

## Phase 7 — Build & Fix

- [ ] Run `make` — capture all compile errors
- [ ] Fix header include errors (missing declarations, wrong signatures)
- [ ] Fix function signature mismatches (callers still passing `lua_State*`)
- [ ] Fix undefined symbol errors (removed functions still referenced)
- [ ] Iterate until `make` produces zero errors
- [ ] Run `make test-headless` — fix any C test failures
- [ ] Run `samples/Example` — verify it loads and displays

---

## Phase 8 — Cleanup

- [ ] Remove any remaining `#include <lua.h>`, `<lauxlib.h>`, `<lualib.h>` from any file
- [ ] `grep -rn "lua_State" source/ include/ plugins/ tools/codegen/` — zero matches
- [ ] `grep -rn "luaL_" source/ include/ plugins/` — zero matches
- [ ] `grep -rn "luaopen_" source/ include/ plugins/` — zero matches
- [ ] Remove `LUAROCKS.md` and `orca-dev-1.rockspec` (LuaRocks packaging)
- [ ] Update `AGENTS.md` to remove Lua references
- [ ] Remove Lua-related docs from `docs/` if any
- [ ] Final `make clean && make` — clean build succeeds
- [ ] Commit all changes

---

## Dependency Graph

```
Phase 0 (Headers)
  ↓
Phase 1 (Core Engine)  ← depends on Phase 0 signatures
  ↓
Phase 2 (Delete Bridges)  ← can start after Phase 1
  ↓
Phase 3 (Strip Remaining)  ← can run in parallel with Phase 2
  ↓
Phase 4 (Codegen)  ← can run in parallel with Phase 3
  ↓
Phase 5 (Build System)  ← can run in parallel with Phase 4
  ↓
Phase 6 (Sample Conversion)  ← independent, can run anytime
  ↓
Phase 7 (Build & Fix)  ← after all other phases
  ↓
Phase 8 (Cleanup)  ← after Phase 7
```

## Key Design Decisions

| Old (Lua) | New (C) |
|-----------|---------|
| `package.lua` project config | `package.xml` parsed with libxml2 |
| `luaL_newstate()` bootstrap | Direct `FS_LoadProject()` + message loop |
| Lua coroutine event loop | `axGetMessage` / `SV_DispatchMessage` loop |
| `luaX_getobjectcallback` | `ControllerCallback` function pointer via event property |
| `wire_controller(L, tbl, obj)` | `OBJ_RegisterController(name, fn)` static registry |
| `luaL_ref` for change callbacks | Removed (notification via `ID_PropertyChangedMessage` only) |
| `luaopen_orca_*` module init | `ORCA_*_Init(void)` void functions |
| `luaX_push*/check*` in codegen | Direct C struct pointer passing |
| `CORE_KEMAP` hotkey Lua registry | Removed (key handling elsewhere) |
| `UI_ProcessCommands` Lua command queue | Removed |
| `lua_openfile_with_env` proxy _ENV | `FS_LoadProjectFromXml()` XML parsing |
