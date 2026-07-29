# ORCA Agent Guide

## Agents and skills

Agents are classic implementation roles under `.opencode/agents/`; skills are
task playbooks under `.opencode/skills/`. Choose the task skill first, then use
an agent when delegation or specialist ownership is useful.

| Typical task | Skill | Agent |
|---|---|---|
| Add a class, property, field, method, message, struct, or enum | `orca-api-codegen` | `engine-developer` |
| Implement a cross-cutting engine feature | `orca-feature-implementation` | `engine-developer` |
| Expose an existing or new API to Lua | `orca-lua-exposure` | `lua-developer` |
| Write Lua or MoonScript programs | `orca-lua-authoring` | `lua-developer` |
| Write UI/package XML | `orca-xml-authoring` | `xml-ui-developer` |
| Define images, textures, shaders, font families, or scenes | `orca-xml-assets-scenes` | `renderer-developer` |
| Bind Lua to XML with controllers, events, or data bindings | `orca-controller-binding` | `lua-developer` or `xml-ui-developer` |
| Add or update tests | `orca-testing` | `test-writer` |
| Apply C or CSS conventions | `orca-c-style` or `orca-css` | relevant implementation agent |

## Source priority

Stop as soon as the task is clear:

1. The applicable `.opencode/skills/<name>/SKILL.md` for workflow.
2. Handwritten `docs/` for architecture, conventions, and system behavior.
3. `.opencode/generated/{c,lua,xml}/<module>/` for generated API reference.
4. `docs/generated/{c,lua,xml}/<module>/`, the developer-facing mirror.
5. The relevant `.cgen` when changing API shape or generated docs are absent.
6. `ctags`, then narrow `rg`, then implementation files.

The generated Markdown is API documentation produced from `.cgen`; it is not a
generated skill. Skills consume that neutral reference rather than owning
copies of it.

### Generated API documentation

`.cgen` is the authoritative source for public API shape. This includes
classes, inheritance, properties, fields, methods, messages, structs, enums,
interfaces, functions, and Lua export metadata. Every API-shape change must be
made in the relevant `.cgen`, including exposing an existing implementation to
Lua.

```bash
# Generate C code and canonical C/Lua/XML API docs
make modules

# Generate canonical docs and copy them to docs/generated
make -f tools/Makefile MODULE_ROOT= documentation
```

Canonical output is
`.opencode/generated/{c,lua,xml}/<module>/`; `documentation` mirrors it to
`docs/generated/{c,lua,xml}/<module>/`. Both trees are ignored build outputs.
On a fresh checkout, generate them before API lookup. If generation is
unavailable, read only the relevant `.cgen` as the fallback.

Never edit `generated/`, `.opencode/generated/`, or `docs/generated/` by hand.
After `.cgen` changes, run `make modules`.

## Task read order

| Task | Read after its skill |
|---|---|
| Build / test | `docs/BUILD.md`, `docs/TESTING.md` |
| Architecture | `docs/PROJECT_MAP.md`, `docs/ARCHITECTURE.md` |
| Class / component | generated reference for its module, then related docs |
| Lua API | generated Lua reference, then `docs/LUA_API.md` |
| UI / layout / XML / style | `docs/UI_SYSTEM.md` |
| C style | `docs/CODING_STYLE.md` |

## Working rules

- For ambiguous tasks, ask for the subsystem, expected behavior, document, or
  reference implementation before exploring.
- Read at most one similar implementation or neighboring test.
- If understanding requires more than 3–5 files, stop and ask.
- Do not scan entire directories or the repository.
- For tests, read `docs/TESTING.md`, the target test, and `tests/test_local.h`
  only when shared macros are involved.
- For local edits: target → nearest helper → `ctags`/narrow `rg` → edit →
  focused build/test.
- In planning mode, clarify before planning and use sub-agents for research and
  plan review.
- In change mode, coordinate agents for independent work and run the relevant
  lint, type check, build, and tests after each feature.

## Lua API naming

`.cgen` methods use PascalCase (`FindChild`, `AddChild`). Codegen lowercases the
first letter for Lua (`findChild`, `addChild`). The generated
`*_export.c` registration tables are authoritative for actual exported names.

## Icons

Use [Lucide Icons](https://lucide.dev/icons) for SVG icons. Import raw,
stroke-based SVG with `stroke="currentColor"` so CSS `color` and
`-orca-tint-mode: template` control tint consistently.

## `walk_prop` invariants

`walk_prop` in `tools/codegen/plugins/properties.c` recursively emits property
definitions for classes and structs:

- It ucfirsts and dot-joins path segments, then `axis_transform` converts the
  path to the public leaf name.
- It always emits the FNV-1a32 `ID_{CLASS}_{LEAF}` for
  `{class_name}.{leaf}`.
- With `record_enum=1`, a non-struct or sealed leaf also emits its `offsetof`
  macro, records the accessor case, and increments `pidx`.
- Non-sealed structs recurse through fields; fixed arrays recurse once per
  indexed element.
- `MAX_DEPTH` is 16. Class emission has `record_enum=1` and a valid `pidx`;
  struct emission has `record_enum=0` and `pidx=NULL`.
- Sealed structs are leaves. `emit_class` owns and frees every name/address
  allocated in `cases`, including error paths.

## C coding style

Keep functions compact without hiding complex control flow:

```c
if (!ic->ItemsSource) return FALSE;                 // guard clause
return Con_Printf("error"), FALSE;                  // side effect + return
struct Object *o = tpl ? OBJ_Instantiate(tpl)       // initializer ternary
                       : OBJ_Create(ID_TextBlock);
n && (n->DataContext = OBJ_GetComponent(data, ID_DataObject));
FOR_EACH_OBJECT(c, hObject) { first = c; break; }
```

Use short-circuit assignment only for a simple pointer/truthiness guard and one
expression. Use a plain `if` for complex conditions, multiple statements, or
long calls. Comment intent only when the code does not make it obvious.
