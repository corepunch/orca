# Codegen — Agent Instructions

## Documentation map

| Doc | Contents |
|---|---|
| `ARCHITECTURE.md` | Pipeline, file layout, plugin inventory |
| `MODEL.md` | `cg_node`, `cg_model`, `cg_kind`, `CG_FLAG_*`, iteration helpers |
| `PLUGINS.md` | Plugin ABI, host interface, writing a plugin |
| `USAGE.md` | CLI flags and exit codes |

Read the relevant doc before touching code. Avoid opening source files just to orient yourself.

## Navigation — ctags REQUIRED

**Do NOT open source files to locate symbols. Do NOT use Glob or Grep on source files.**
Always use the tag index instead.

### Step 0 — generate the index (do this first, every session)

    ctags -R \
      --languages=C,C++ \
      --fields=+nksStia \
      --extras=+q \
      --exclude=.git \
      --exclude=build \
      --exclude=.venv \
      -o /tmp/tags \
      tools/codegen

If `/tmp/tags` already exists, skip this step.

### Step 1 — query the tag file

    grep -P '^symbol_name\t' /tmp/tags

This returns the file path and line number. Then open that file at that exact line only.

### Prohibited before querying /tmp/tags

- Glob on source files
- Grep on source files  
- Reading a file without a line number

## Key symbols (quick reference)

These are the most frequently needed entry points — look them up in `/tmp/tags` for exact line numbers:

| Symbol | File | What it is |
|---|---|---|
| `cg_model_load` | `src/model.c` | Parses a `.cgen` file into a `cg_model` |
| `walk` | `src/model.c` | Recursive XML tree walker |
| `elem_kind` | `src/model.c` | Maps XML element name → `cg_kind` |
| `cg_model_free` | `src/model.c` | Frees a `cg_model` |
| `cg_first` | `include/cg_api.h` | First child by kind |
| `cg_next` | `include/cg_api.h` | Next sibling by kind |
| `cg_foreach` | `include/cg_api.h` | Iteration macro |
| `cg_plugin_init_v1` | plugin `.so` | Plugin entry point (exported symbol) |
| `ob_printf` | `include/outbuf.h` | Printf into a dynamic output buffer |

## Codebase conventions

- Model nodes are a flat array; navigate by `id`/`parent` integers, never pointer arithmetic across nodes.
- All string fields on `cg_node` (`name`, `type`, `extra`, `aux`, `aux2`, `doc`) may be `NULL` — check before use.
- `cg_kind` semantics for each field vary by kind; consult `MODEL.md` before reading `node->type` etc.
- Plugins must not store pointers into `cg_model` past the `emit` call — the model is freed immediately after.
- `host->write_file()` creates parent directories; never call `mkdir` manually in a plugin.
- Output goes under `out/` mirroring the source module path; do not hardcode paths in plugins.

## Making changes

1. Identify the symbol to change using `/tmp/tags` (regenerate if stale).
2. Read `ARCHITECTURE.md` if the change touches the pipeline, `MODEL.md` if it touches node fields or kinds, `PLUGINS.md` if it touches the ABI.
3. Edit the source. If you add/rename a public symbol, update the relevant `.md` doc.
4. Re-run ctags after edits.
5. Build with `make codegen-host` and verify with `make run-sample`.
