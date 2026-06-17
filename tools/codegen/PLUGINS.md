# Codegen — Plugin System

Plugins are shared libraries (`.so`) that export a single symbol:

```c
cg_plugin_v1 const *cg_plugin_init_v1(void);
```

## Plugin ABI (`cg_api.h:118`)

```c
typedef struct cg_plugin_v1 {
    uint32_t   abi;        // must match CG_API_VERSION
    char const *name;      // plugin name for logging
    int        (*emit)(cg_host_v1 const *host, cg_model const *model, char const *output);
} cg_plugin_v1;
```

The `emit` callback receives:
- `host` — callbacks provided by the host (logging, file writing, hashing)
- `model` — the parsed module (read-only)
- `output` — output path (`"-"` means stdout)

## Host interface (`cg_api.h:111`)

```c
typedef struct cg_host_v1 {
    uint32_t abi;
    void     (*logf)(char const *fmt, ...);
    int      (*write_file)(char const *path, char const *data, size_t len);
    uint32_t (*fnv1a32)(char const *s);
} cg_host_v1;
```

- `logf` — log to stderr
- `write_file` — write a file, creating parent directories as needed
- `fnv1a32` — FNV-1a32 hash for hash-based naming

## Writing a plugin

1. Create a `.so` that exports `cg_plugin_init_v1`
2. Return a `cg_plugin_v1` with `abi = CG_API_VERSION` and an `emit` function
3. In `emit`, iterate the model using `cg_first`/`cg_next`/`cg_foreach` (see `MODEL.md`)
4. Use `host->write_file()` to write output
5. Use `host->logf()` for diagnostics
6. Use `host->fnv1a32()` for hash-based naming
