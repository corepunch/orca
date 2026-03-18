# WebGL / Emscripten Build

Orca can be compiled to WebAssembly and run in a browser using the
[Emscripten](https://emscripten.org/) toolchain.  The `libs/platform`
submodule already contains a complete WebGL backend (`libs/platform/webgl/`)
so only the engine's Makefile and a few small source-level guards needed to be
added.

## Quick Start

```bash
# 1. Install the Emscripten SDK (one-time setup)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source ./emsdk_env.sh   # adds emcc/emmake to PATH

# 2. Build the platform library for WebGL
emmake make -C libs/platform OUTDIR=../../build/lib

# 3. Build Orca as orca.html + orca.js + orca.wasm
make webgl

# 4. Serve the output directory from a local HTTP server
#    (browsers block direct file:// access to WebAssembly)
python3 -m http.server --directory build/webgl
# then open http://localhost:8000/orca.html
```

The `share/` assets are always packed into the WASM virtual file system (they
contain fonts and Lua plugins that the engine requires at runtime).

To bundle a project's data directory into the build as well:

```bash
make webgl WEBGL_DATA=samples/Example
```

This additionally packs the project directory and compiles `PROJECTDIR` into
the binary so the engine opens the bundled project automatically without
needing a command-line argument.

The build uses `-Oz --closure 1` for minimum output size.

---

## Debug Build

When a production build produces an unhelpful error like:

```
RuntimeError: Out of bounds memory access (evaluating 'a(...c)')
```

the underlying C source location is hidden by JS minification (`--closure 1`)
and disabled runtime assertions (`-sASSERTIONS=0`).  Use the debug target to
get C file names and line numbers instead:

```bash
emmake make webgl-debug                          # share/ assets only
emmake make webgl-debug WEBGL_DATA=samples/Example  # bundle a project
# or use the convenience alias:
emmake make webgl-debug-demo
```

Serve the output directory (Chrome/Firefox block `file://` WebAssembly access):

```bash
python3 -m http.server --directory build/webgl
# Open http://localhost:8000/orca.html
```

The debug target differs from the production build in the following ways:

| Flag | Effect |
|------|--------|
| `-g` | Embed DWARF debug information in `orca.wasm` |
| `-gsource-map` | Emit `orca.wasm.map` — maps every WASM byte back to the original C file and line number.  Chrome DevTools shows the C source location automatically when the map file is served alongside `orca.wasm`. |
| `-sASSERTIONS=2` | Enable strict runtime checks: out-of-bounds memory accesses, null-pointer dereferences, type mismatches, and stack overflows all produce descriptive error messages that name the offending C symbol. |
| `-sSAFE_HEAP=1` | Validate every heap read and write for alignment and bounds.  This catches the class of errors that appear as `"Out of bounds memory access"` in production builds. |
| `-sSTACK_SIZE=1048576` | Set the C stack size to 1 MB (up from the 64 KB Emscripten default).  The renderer initialization requires deep call chains that exceed the default stack; this prevents `"stack overflow"` aborts at startup. |
| `-sSTACK_OVERFLOW_CHECK=2` | Detect stack overflows with precise per-call checking.  Reports the offending function when the C stack is exhausted. |
| `-sDEMANGLE_SUPPORT=1` | Demangle C/C++ symbols in stack traces so function names are human-readable instead of mangled identifiers. |
| `-sEXCEPTION_STACK_TRACES=1` | Attach a full stack trace to every thrown exception, visible in the browser console. |
| `-O1` (not `-Oz`) | Minimal optimisation — code structure is preserved and stack traces are readable. |
| *(no `--closure 1`)* | JavaScript output is not minified, so DevTools stack traces show real function names instead of single-letter identifiers like `a`, `b`, `c`. |
| *(no `-flto`)* | Link-time optimisation is disabled for faster iteration. |

> **Tip:** Open DevTools → **Sources** tab after loading the debug build.
> If `orca.wasm.map` is served correctly, Emscripten errors will include a
> clickable link to the exact C file and line that triggered the fault.

---

## Prerequisites

### Emscripten ports (automatic)

These libraries are fetched and compiled automatically by Emscripten via
`-sUSE_*` linker flags — no manual work required:

| Library   | Flag              |
|-----------|-------------------|
| zlib      | `-sUSE_ZLIB=1`    |
| libpng    | `-sUSE_LIBPNG=1`  |
| libjpeg   | `-sUSE_LIBJPEG=1` |
| freetype  | `-sUSE_FREETYPE=1`|

### WASM libraries (built from source with emmake)

The following libraries must be compiled for WASM before running `make webgl`.
Run `emmake make wasm-deps` once (with `emcc` in PATH) to build and install
them into `build/wasm-deps/`:

| Library  | Version | Source                                          |
|----------|---------|-------------------------------------------------|
| lua 5.4  | 5.4.7   | https://www.lua.org/download.html               |
| libxml2  | 2.9.14  | https://gitlab.gnome.org/GNOME/libxml2          |
| liblz4   | 1.10.0  | https://github.com/lz4/lz4                      |

```bash
source /path/to/emsdk/emsdk_env.sh
emmake make wasm-deps
# then: emmake make webgl
```

## How the WebGL Backend Works

The `libs/platform/webgl/` directory contains three source files:

| File                      | Purpose |
|---------------------------|---------|
| `webgl_system.c`          | `WI_Init` / `WI_Shutdown`, timing, platform strings |
| `webgl_window.c`          | Canvas sizing, WebGL context, `WI_BeginPaint`/`WI_EndPaint` |
| `webgl_event.c`           | Emscripten input callbacks → internal event queue; `WI_PollEvent` |

The platform Makefile detects `EMSCRIPTEN` automatically when invoked with
`emmake` and selects only the `webgl/` sources.

### Main Loop and ASYNCIFY

The engine's main loop is a Lua `while true do … end` that polls for events
via `WI_PollEvent`.  Browsers cannot run a blocking loop on the main thread;
the JavaScript event loop must be allowed to yield periodically.

**Solution:** Compile with `-sASYNCIFY=1`.  In
`source/backend/queue.c::f_peek_iterator`, an `emscripten_sleep(0)` call is
inserted whenever `WI_PollEvent` returns zero (queue empty).  With ASYNCIFY
this suspends the C stack, yields control back to the browser (so input
callbacks fire and `requestAnimationFrame` can schedule a repaint), then
resumes the Lua loop on the next tick.

### GLSL Shader Version

The renderer dynamically prepends a version preamble to every shader.  On
desktop it uses `#version 330 core`; on WebGL 2 it must use
`#version 300 es`.  The guard in `source/renderer/r_shader.c` was extended to
cover `__EMSCRIPTEN__` in addition to the existing `__QNX__` case.

---

## Modules Excluded from the WebGL Build

Some engine modules cannot work inside a browser and are excluded from the
`WEBGL_MODULES` list in the Makefile:

| Module    | Reason |
|-----------|--------|
| `network` | Uses BSD sockets / libcurl; use `fetch()` or WebSockets instead |
| `vsomeip` | C++ SOME/IP service-discovery library, host-only |
| `server`  | Server-mode listener; host-only |
| `editor`  | Native desktop file-dialog and font-rendering features |

---

## Findings: What Should Move to `libs/platform`

While wiring up the WebGL build the following platform-specific concerns in
the main engine were identified.  They are candidates to be abstracted into
`libs/platform` so that the engine core stays portable:

### 1. Executable / Resource Path Discovery

`source/orca.c::get_exe_filename()` uses `readlink("/proc/<pid>/exe")`,
`_NSGetExecutablePath`, or `GetModuleFileName` to locate the binary, then
derives `LIBDIR`, `SHAREDIR`, and `PLUGDIR` from it.

In WebGL there is no executable on a filesystem.  The engine currently falls
back to the `WI_LibDirectory()` / `WI_ShareDirectory()` helpers already
provided by `platform.h`, but the path-stripping logic (`strrchr(exename, '/')
 strip two segments`) still runs.

**Recommendation:** Add a `WI_GetExecutablePath(char *buf, size_t sz)` to
`platform.h` and implement it per-platform (including a no-op for WebGL that
fills `buf` with `"."`) so `orca.c` has a single call for all platforms.

### 2. Lua C-Module Loading (`.so` Path)

`orca.c` appends `LIBDIR/lib?.so` to `package.cpath` so Lua's `require` can
find shared libraries.  This works on Linux/macOS but is a no-op in WebGL
because all C modules are statically linked into the binary.

For WebGL the `.so` path line is skipped (`#ifndef __EMSCRIPTEN__`), but the
engine still calls `require 'orca'`, `require 'orca.core'`, etc. which will
fail unless the C functions are pre-registered before the Lua state starts.

**Status: resolved.** All built-in C modules are pre-registered by
`luaopen_orca` via `luaL_preload` before Lua code runs; no `.so` discovery is
needed for WebGL.

### 3. Dynamic Plugin Loading

Plugins (`build/lib/liborca/*.so`) are discovered and loaded at runtime via
`dlopen`.  Emscripten supports `SIDE_MODULE` / `MAIN_MODULE` dynamic linking
but it requires all side modules to be listed at link time; fully runtime
dlopen is not supported.

**Status: resolved.** The Makefile `webgl` target now:

1. Lists the plugins to bundle in `WEBGL_PLUGINS` (UIKit, SceneKit, SpriteKit,
   DarkReign — vsomeip is excluded because it is C++ / SOME/IP).
2. Compiles all plugin `.c` sources directly into the single WASM binary
   alongside the engine modules.
3. Auto-generates `build/webgl/plugins_luaopen.h` at build time by scanning
   the plugin sources for `ORCA_API int luaopen_orca_*` symbols and building a
   `plugin_modules[]` registration table.  The Lua module name is derived from
   the C symbol name by stripping the `luaopen_` prefix and replacing `_` with
   `.` (e.g. `luaopen_orca_UIKit` → `"orca.UIKit"`).
4. Passes `-DPLUGINS_LUAOPEN` when compiling `orcalib.c`.  Under that guard the
   generated header is included and each plugin module is registered via
   `luaL_preload` in `luaopen_orca`, exactly like the built-in modules.

### 4. Thread / Blocking I/O

`source/network/*.c` uses blocking POSIX sockets and `source/sysutil/w_system.c`
uses `opendir`/`readdir` for directory listing.  Directory listing works via
Emscripten's VFS; network operations require replacing curl/sockets with
Emscripten's `emscripten_fetch` API.

**Recommendation:** Move the network abstraction into `libs/platform`
(`WI_FetchURL`, `WI_OpenSocket`) so the engine calls a single API and each
platform provides the appropriate implementation (POSIX vs. Fetch API).

### 5. `WI_Sleep` / Frame Pacing

`WI_Sleep(ms)` is currently a `nanosleep` on POSIX and a no-op on WebGL.
The comment in `webgl_system.c` suggests using `emscripten_sleep(msec)` (with
ASYNCIFY) for a proper sleep.  This is already partially handled by the
`emscripten_sleep(0)` yield in `queue.c`, but a non-zero duration sleep may
be useful for CPU throttling.

**Recommendation:** Implement `WI_Sleep` in `webgl_system.c` as
`emscripten_sleep(msec)` once ASYNCIFY is confirmed stable.  The ASYNCIFY
build flag is already set in the `webgl` Makefile target.
