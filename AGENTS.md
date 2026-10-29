# ORCA Agent Guide

## Application authoring

For creating or reorganizing an app, start with
[`orca-app-authoring`](.opencode/skills/orca-app-authoring/SKILL.md).
The maintained conventions live in [Building ORCA applications](docs/app/index.md)
and [App styling](docs/app/styling.md). They distinguish supported behavior,
recommended defaults, and remaining engine gaps. Preserve an explicit user
choice or an established project architecture when doing local feature work.

### Scene format: XML only

Author application pages, templates and scenes in XML. Lisp
(`-DORCA_USE_LISP`) is an experimental scene format: only `apps/Example` has a
Lisp version, which exercises the alternative reader. Do not add `.lisp` twins
to other apps. Tests that load another app's pages belong in
`XML_APP_LUA_TESTS` in the Makefile, which runs in XML builds only.

## UI design skill

For designing or reviewing screens and components, use the [`orca-ui-design`](.opencode/skills/orca-ui-design/SKILL.md) skill (adapted from Apple HIG principles for ORCA's declarative modern UI).

For `apps/chat`, preserve its current Windows Phone / Metro direction. Read
[Chat design decisions and references](apps/chat/DESIGN.md) for the visual
baseline, historical sources, and footer/photo-message rules.

## Agents and skills

Agents are classic implementation roles under `.opencode/agents/`; skills are
task playbooks under `.opencode/skills/`. The same skills are reachable as
`.claude/skills/` (Claude Code) and `.agents/skills/` (Codex); both are
symlinks, so edit only `.opencode/skills/`. Choose the task skill first, then
use an agent when delegation or specialist ownership is useful.

| Typical task | Skill | Agent |
|---|---|---|
| Create an app or choose its architecture, state, and navigation | `orca-app-authoring` | `lua-developer` or `xml-ui-developer` |
| Add a class, property, field, method, message, struct, or enum | `orca-api-codegen` | `engine-developer` |
| Implement a cross-cutting engine feature | `orca-feature-implementation` | `engine-developer` |
| Expose an existing or new API to Lua | `orca-lua-exposure` | `lua-developer` |
| Write Lua or MoonScript programs | `orca-lua-authoring` | `lua-developer` |
| Write UI/package XML | `orca-xml-authoring` | `xml-ui-developer` |
| Define images, textures, shaders, font families, or scenes | `orca-xml-assets-scenes` | `renderer-developer` |
| Build or debug a 3D app: SceneKit scene XML, shaders/materials, animation players bound to inputs, running it | `orca-3d-scene` | `renderer-developer` |
| Author a Blender scene for ORCA and export it (`tools/blender-export.py`, `orca-fbx`) | `orca-blender-pipeline` | `renderer-developer` |
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
| Architecture / application structure | `docs/PROJECT_MAP.md`, `docs/app/index.md` |
| Class / component | generated reference for its module, then related docs |
| Lua API | generated Lua reference, then `docs/LUA_API.md` |
| UI / layout / XML / style | `docs/UI_SYSTEM.md`, `docs/app/styling.md` |
| Text rendering / fonts / glyphs | `docs/renderer-internals.md` → *Text Rendering Pipeline* |
| 3D scene / Blender export | `apps/cluster-demo/` (reference project), `docs/animations.md` |
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

## Chat: fast Appwrite unit tests

For Appwrite work, run only the smallest affected unit tests by default.
`make test-chat-appwrite` runs the adapter directly in standalone Lua, without
starting or rebuilding ORCA. `make test-chat-appwrite-backend` tests Function
logic with injected REST/repository doubles. For a specific backend change,
use Node's `--test-name-pattern` to select the affected cases. UI-only edits
do not require Appwrite tests. Reuse passing results until relevant code changes.

Do not run integration or end-to-end suites, including the offline live-suite
emulator, unless the user explicitly asks for integration verification.
Appwrite integration is trusted for routine work; do not retest deployments,
accounts, native HTTP, Keychain or devices as part of unit validation.

### Manual integration checks (explicit request only)

Live Appwrite checks are strictly on demand: run them only when the user
explicitly requests live verification. Do not automatically run or propose
them after Chat edits, offline tests, builds, or deployments. This applies to
setup, cleanup, Node tests, native tests, and two-account device checks. Keep
them out of default tests and PR CI. See
[live test instructions](apps/chat/backend/appwrite/live/README.md).

For ordinary `apps/chat` work, run the smallest affected offline checks.
For timeline, photo presentation, or footer changes, `make test-chat-ui` runs
focused offline tests using the existing binary, without rebuilding the
engine. Run a single test file directly when only that behavior is affected.
Rebuild when native code/API changes or the binary is missing; broaden offline
testing only for shared behavior, failures, or unresolved concerns. Do not
repeat successful checks without a relevant change. A UI-only edit does not
require live backend, Keychain, or device verification.

```bash
make test-live-setup                       # once, or reconnect expired sessions
make test-live                             # supported features on real Appwrite
CHAT_LIVE_ONLY=discovery,direct,messages make test-live  # focused iteration
CHAT_LIVE=1 make test-chat-live             # actual Lua/native client, macOS
```

During an explicitly requested live check, select the affected suites. Run the
full suite only when full live verification is requested. Run Node and native live suites
sequentially when they share accounts: normalization, locks and lookup rate
limits can interfere with concurrent runs. Choose native checks for requested
verification of Account authentication, session storage/renewal, native HTTP,
or photo processing/transport. Include a two-account device pass only when
requested. Network and macOS Keychain access must be available. A skipped or
emulated run is not live verification; focused checks do not establish full
end-to-end integration coverage.

Reuse Alice/Bob/Carol/Dave in the ignored
`apps/chat/backend/.live-accounts.json` (mode 0600). The Appwrite plugin can
create the accounts; setup signs them in and initializes usernames. Never print
or commit passwords, cookies, JWTs or server keys, and never bundle the accounts
file in the Function archive. Tests use ordinary user sessions. Provision Dave's
verified-email fixture through the Appwrite plugin; this checks discovery,
not OTP delivery. No server key is needed for these assertions. Use the plugin
for administrative fixture setup/cleanup; never broaden the app's permissions.
Report skipped cases, retained test data, target project, active deployment and
test results. Deployments must contain the routes being tested; record stale
deployment failures rather than substituting mocks. Preserve existing project
permissions and unrelated users/data.

When requested, `make test-live` tests supported features. `CHAT_LIVE_ONLY=parity make test-live`
is an optional exploratory backlog, whose TODO probes do not establish parity.
Reactions/emoji, videos and calls are not current acceptance requirements; see
[the scoped roadmap](apps/chat/PRODUCT.md#current-scope-and-deferred-work).

## Lua API naming

`.cgen` methods use PascalCase (`FindChild`, `AddChild`). Codegen lowercases the
first letter for Lua (`findChild`, `addChild`). The generated
`*_export.c` registration tables are authoritative for actual exported names.

## Icons

Use [Lucide Icons](https://lucide.dev/icons) for SVG icons. Import raw,
stroke-based SVG with `stroke="currentColor"`. `VectorView` automatically
renders an SVG as a template alpha mask tinted by `Foreground` / CSS `color`.
Verify supported properties for other image controls instead of assuming a
`-orca-tint-mode` CSS declaration exists.

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
