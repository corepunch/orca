# WASM Dependencies

WASM static libraries are built at compile time into `build/wasm-deps/` and
are not stored in this directory.

To build them locally, run:

```bash
# Requires emcc in PATH — activate your emsdk first:
source /path/to/emsdk/emsdk_env.sh
emmake make wasm-deps
```

See `docs/webgl.md` for details.
