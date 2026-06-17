# ORCA Agent Guide

## Source priority

Use sources in this order — stop as soon as the task is clear:

1. `docs/` — architecture, conventions, workflows
2. `docs/generated/*.md` — API documentation: classes, structs, methods, properties, messages, Lua exports
3. `.cgen` — authoritative source to access only when adding new classes, not for reference
4. `ctags` — symbol lookup, ownership, relevant implementation files
4. `grep` — string literals, macro expansions, patterns not represented as symbols
5. Source files — only when implementation details are required

## Task read order

| Task | Read |
|------|------|
| Build / test | `docs/BUILD.md`, `docs/TESTING.md` |
| Architecture | `docs/PROJECT_MAP.md`, `docs/ARCHITECTURE.md` |
| Class / component | `docs/generated/` for the relevant module, then related `docs/`, then implementation |
| Lua API | `docs/generated/` for the relevant module, then `docs/LUA_API.md` |
| UI / layout / XML / style | `docs/UI_SYSTEM.md` |
| C style / conventions | `docs/CODING_STYLE.md` |

## Clarification policy

For ambiguous tasks, ask before exploring. Ask for the relevant subsystem, document, expected behavior, or reference implementation. A short question beats reading dozens of files.

## Narrow-context rules

- Read at most one similar implementation or test for reference.
- If more than 3–5 files are needed to understand a task, stop and ask.
- Do not scan entire directories or the whole repository unless explicitly required.
- Expand investigation only when compilation errors, failing tests, or diagnostics require it.

## Test edits

Read `docs/TESTING.md`, the target test file, and `tests/test_local.h` if shared macros are involved. At most one neighboring test for reference. Do not scan all `tests/*.c` unless explicitly refactoring all tests.

## Generated code

Never edit `generated/` by hand — modify `.cgen` or source files and rerun `make modules`.

## Anti-exploration

Avoid `Glob "**/*"`, reading every file in a directory, re-running searches for known facts, spawning subagents for small edits, or inferring conventions from many examples.

For local edits: read target → nearest helper if needed → `ctags`/narrow grep → edit → build/test.

## Planning mode

- Ask clarifying questions before proposing a plan.
- Never assume design, tech stack, or features.
- Use sub-agents for research and to review the plan before presenting it.

## Change / edit mode

- Coordinate sub-agents rather than implementing directly.
- Parallelize independent changes across sub-agents.
- Use premium models for complex tasks, mid-tier for simpler ones.
- After each feature, run lint, type check, and build.

## walk_prop — properties.c recursive property walker

`walk_prop` (`tools/codegen/plugins/properties.c:295-340`) is the core recursive function that traverses property/field trees and emits C header definitions. It is called from `emit_class` (line 442), `emit_struct` (line 498), and recursively from itself for nested non-sealed structs.

### Signature

```c
static int walk_prop(walk_ctx *ctx, char segs[MAX_DEPTH][64], int n_segs, char const *type_name);
```

- `ctx` — shared context containing output buffer (`ob *b`), model (`cg_model const *m`), struct map (`smap`/`scount`), host callbacks (`h`), class name, property index pointer (`pidx`), enum-recording flag (`record_enum`), and a dynamic array of emitted property `cases` (for accessor switch generation).
- `segs` — stack-allocated array of `MAX_DEPTH` (16) segments, each up to 64 bytes. `segs[0..n_segs-1]` holds the path components from root property down to the current field.
- `n_segs` — current depth (number of populated segments).
- `type_name` — the C type of the current property/field; used to look up whether the type is a struct and whether it is sealed.

### Step-by-step execution

**1. Build dot-joined ucfirsted path** (lines 299-305)

Each segment `segs[i]` is ucfirstized (first character uppercased via `ucfirst_into`) and joined with `.` into a single `path` string (up to 512 bytes). This path represents the hierarchical property path excluding the class name prefix.

Example: for a property `Layout` with a field `Axis[0].Left`, `segs = ["Layout", "Axis[0]", "Left"]` produces `path = "Layout.Axis[0].Left"`.

**2. Axis transform → leaf name** (line 307)

`axis_transform(path, leaf, sizeof(leaf))` converts the dot-joined path into a final C property name. The transform applies a fixed set of pattern-matching rules (lines 77-182) in priority order:

- **Rules 7-9**: `Size.Axis[N].{Sub}` — maps to `{Width,Height,Depth}{Requested,Desired,Min,Actual,Scroll}` (e.g. `Size.Axis[0].Requested` → `WidthRequested`, `Size.Axis[1].Min` → `MinHeight`).
- **Rules 1-6**: `(.+).Axis[N].Left/Right` — replaces `.Axis[N].Left`/`.Right` with the axis-specific directional name (`Left`→`Left`, `Top`, `Front`; `Right`→`Right`, `Bottom`, `Back`), then strips `[`, `]`, `.` characters. E.g. `Layout.Axis[0].Left` → `LayoutLeft`.
- **Rules 10-12**: `(.+).Axis[N]` at end — appends axis name (`Horizontal`→`Width`, `Vertical`→`Height`, `Depth`→`Depth`). E.g. `Layout.Axis[1]` → `HeightLayout`.
- **Rule 13**: `Border.Radius.(.+)Radius` — special case for border radii. E.g. `Border.Radius.TopLeftRadius` → `BorderTopLeftRadius`.
- **Rules 14-19**: `Axis[N].Left/Right` without leading segment — same directional mapping without prefix.
- **Rules 20-22**: Bare `Axis[N]` anywhere in path — same axis name substitution.
- **Fallback**: If no pattern matches, strip `[`, `]`, `.` characters from the path.

The result is the `leaf` name used in the emitted `#define` identifiers.

**3. Emit ID macro** (lines 309-315)

A full name is constructed as `{class_name}.{leaf}`. An FNV-1a32 hash is computed via `ctx->h->fnv1a32(full)`. The following macro is emitted to the output buffer:

```c
#define ID_{CLASS}_{LEAF} 0x{HASH} // {class}.{leaf} ({type})
```

The type is included as a comment; if `type_name` is empty/NULL, `"void"` is emitted instead.

**4. Emit offset constant and record property (if `record_enum` is true)** (lines 316-323)

The struct for `type_name` is looked up via `find_struct(ctx->smap, ctx->scount, type_name)`. If `ctx->record_enum` is true AND the struct is either not found or is sealed, the following macro is emitted:

```c
#define k{CLASS}{LEAF} offsetof(struct {CLASS}, {addr})
```

where `addr` is the dot-joined path of segments (built by `property_addr`), e.g. `Layout.Axis[0].Left`.

Then `wctx_push(ctx, leaf, addr, *ctx->pidx)` records the property: it appends `{name, addr, index}` to `ctx->cases` (a dynamically array used to generate an accessor switch statement later). The property index `*ctx->pidx` is then incremented.

**5. Recurse into non-sealed struct fields** (lines 325-338)

If the struct for `type_name` exists AND is NOT sealed AND `n_segs < MAX_DEPTH - 1` (depth guard), the function iterates all fields of that struct via `cg_foreach(ctx->m, s->id, CG_KIND_FIELD)`:

- For each field, `fixed = atoi(f->extra)` is checked. If `fixed > 0`, the field is a fixed-size array and `walk_fixed_array` is called, which iterates `fixed` times with appended indices (e.g. `Fields[0]`, `Fields[1]`, ...).
- Otherwise, a new segment is appended and `walk_prop` is called recursively with `n_segs + 1`.

If `walk_prop` returns -1 (error), the outer caller must free `ctx->cases` before returning -1.

### walk_ctx structure (lines 219-235)

```c
typedef struct {
    ob                *b;            /* output buffer for emitted text */
    cg_model const    *m;            /* parsed model (read-only) */
    sentry const      *smap;         /* struct name→id→sealed lookup table */
    size_t             scount;       /* number of entries in smap */
    cg_host_v1 const  *h;            /* host callbacks (fnv1a32, write_file, logf) */
    char const        *class_name;   /* current class/struct name */
    uint32_t          *pidx;         /* pointer to property index counter (NULL for structs) */
    int                record_enum;  /* 1 = emit k macros and record properties; 0 = ID-only */
    struct { char *name; char *addr; uint32_t index; } *cases;
                                     /* accumulated property entries for accessor switch */
    size_t             case_n, case_cap;
                                     /* current/allocated count of entries */
} walk_ctx;
```

### Key invariants

- `MAX_DEPTH` is 16; recursion beyond `MAX_DEPTH - 1` is prevented in step 5.
- `pidx` is `NULL` when called from `emit_struct` (line 490), meaning offset constants and property recording are skipped for struct field emission — only ID macros are emitted.
- `record_enum` is `1` when called from `emit_class` (line 423), meaning full property emission with offset constants and index recording. `record_enum` is `0` when called from `emit_struct` (line 491), meaning only ID macros are produced.
- The `sealed` flag on a struct (from `CG_FLAG_SEALED`) prevents recursion into its fields. Sealed structs are leaf types with no nested properties.
- `wctx_push` dynamically grows the `cases` array (doubling from 64 up as needed). Each entry's `name` and `addr` are heap-allocated copies; the caller (`emit_class`) is responsible for freeing them (lines 465-469).
- FNV-1a32 hashing is done on `{class_name}.{leaf}` to produce deterministic 32-bit property IDs.

### Call graph

```
emit_properties (line 527)
  └── emit_class (line 418)
        └── walk_prop (line 442) — root property, record_enum=1, pidx set
              └── walk_fixed_array (line 330) — iterates fixed-size array elements
              └── walk_prop (line 335) — recursive for nested struct fields
  └── emit_struct (line 482)
        └── walk_prop (line 498) — field, record_enum=0, pidx=NULL
              └── walk_fixed_array (line 495)
              └── walk_prop (line 498)
```