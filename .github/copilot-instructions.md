# ORCA Agent Rules

Keep this file tiny: Zed injects it into every model request. Put details in
`.github/agent-context/` and read them only when needed.

## Always

- Convention over configuration. Don't Repeat Yourself.
- ORCA is pre-release: prefer clean architecture over legacy compatibility.
- Do not preload large files or attach broad `@file` context unless requested.
- Search first; then read the smallest relevant line range. Avoid whole-file
  reads after grep has returned useful line numbers.
- Do not edit generated files under `generated/`; edit `.cgen` and rerun
  codegen.

## Before Editing

Read exactly the smallest matching area file; read two only for cross-area work.

| Task | Read |
|---|---|
| UI elements, UIKit, XML screens, styling, popups, layout tests | `.github/agent-context/ui-elements-and-styling.md` |
| Lua APIs, object Lua bridge, bindings, formulas, scripts | `.github/agent-context/lua-bindings-and-programs.md` |
| Core object runtime, filesystem, parsers, localization, network, editor services | `.github/agent-context/core-object-and-filesystem.md` |
| `.cgen`, generated APIs, component classes, messages, properties | `.github/agent-context/codegen-and-components.md` |
| Renderer, textures, palettes, shaders, meshes, WebGL, game assets | `.github/agent-context/renderer-and-assets.md` |
| Build rules, tests, Xcode project, docs site | `.github/agent-context/build-tests-and-docs.md` |
| C formatting/style only | `.github/agent-context/c-style.md` |

Full archived guide: `.github/agent-context/orca-reference.md`. Do not read it
by default.

## Quick Paths

- Core: `source/core/`, `include/orca.h`, `include/orcadef.h`
- Filesystem XML load/save: `source/filesystem/`, `source/parsers/`
- Editor serialization: `plugins/EditorKit/ed_stab_object.h`
- UIKit: `plugins/UIKit/UIKit.cgen`, `plugins/UIKit/Button.c`
- Codegen: `tools/codegen/`, `docs/MODULE_XML_GUIDE.md`
- Tests: `tests/`

## Local-Agent Hygiene

- For casual questions, use a no-tool/minimal profile.
- Avoid repeating the same request text and file mention in one prompt.
- Prefer prompts like: "Find X, then read only relevant lines."
