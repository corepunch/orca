# Pre-built WASM Libraries

This directory contains static libraries compiled for WebAssembly (WASM) with
Emscripten.  They are checked into the repository so that `make webgl` can
link against them without requiring a separate download-and-compile step.

## Contents

```
libs/wasm/
├── lib/
│   ├── liblua5.4.a      — Lua 5.4 compiled with emcc -O2 -DLUA_USE_POSIX
│   ├── liblz4.a         — LZ4 1.10.0 compiled with emcc -O2
│   └── libxml2.a        — libxml2 2.9.14 compiled with emconfigure/emmake
└── include/
    ├── lua5.4/          — Lua public headers (lua.h, luaconf.h, lauxlib.h, lualib.h)
    ├── lz4.h            — LZ4 public headers
    ├── lz4hc.h
    ├── lz4frame.h
    └── libxml2/libxml/  — libxml2 public headers
```

## Rebuilding

If you need to rebuild these libraries (e.g. after an Emscripten update or a
library version bump), run:

```bash
# Requires emcc in PATH — activate your emsdk first:
source /path/to/emsdk/emsdk_env.sh

emmake make wasm-deps WASM_DEPS_DIR=libs/wasm
```

Then commit the updated `.a` files and headers.

## Library Versions

| Library  | Version |
|----------|---------|
| lua      | 5.4.7   |
| lz4      | 1.10.0  |
| libxml2  | 2.9.14  |
