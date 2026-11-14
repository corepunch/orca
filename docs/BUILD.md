# Build

Use this file as the fast path for local setup and command selection.

## First read

- `Makefile`
- `docs/TESTING.md`
- `docs/PROJECT_MAP.md`

## Initial setup

```bash
git submodule update --init --recursive
```

On macOS, the Xcode Command Line Tools are enough: curl comes from the SDK,
libxml2 and zlib are built from `libs/`, and nothing is needed from Homebrew. On Linux, install the
development packages listed in the [README](../README.md).

Lua, LZ4, FreeType, libpng, libjpeg and zlib are built from submodules:

- Lua 5.4 is built from the `libs/lua` submodule into
  `build/lib/liblua5.4.so` and `build/bin/lua`, which `make lua` builds on its
  own. The engine, plugins and Lua-only tests all use that copy.
- LZ4 is compiled into `liborca` from the `libs/lz4` submodule.
- FreeType, libpng, libjpeg and zlib are built from their `libs/` submodules.

MoonScript is optional and is not bundled. To run `.moon` programs, install it
with LuaRocks for Lua 5.4 and point `LUA_PATH` and `LUA_CPATH` at that tree.

## Authoritative commands

```bash
make modules
make unite
make test-headless
xvfb-run make test
```

## XML scenes

Application pages, templates, resource libraries and scenes use XML on every
platform. Extensionless resource paths resolve to `.xml` files. The loader,
library and font lookup, cloning, editor saves and project scaffolding all use
the same XML scene support.

Use `loadObjectFromDisk(path)`, `loadObjectFromString(source)`, and
`serializeObjectToString(object)` for scene I/O. `make test-headless` includes
the engine, XML fixture and application tests.

## What each command is for

- `make modules` — regenerate code from every `.cgen` file.
- `make unite` — build the core library, plugins, and app without running tests.
- `make editor` — build and launch the editor; add `PROJECT=apps/Weather` to preselect a project.
- `make test-headless` — run the C test binaries plus the headless Lua suite.
- `xvfb-run make test` — run the full suite when display-backed tests matter.

## Narrow command selection

- Changed only docs: validate the docs build if needed; do not run unrelated tests.
- Changed `.cgen`: run `make modules` first, then the narrowest relevant test, then broader validation if the API is shared.
- Changed core or plugins: use `make unite` before the relevant tests.
- Changed layout/UI behavior: prefer `make test-headless` before full display tests.

## Generated-file rule

Never hand-edit files under `generated/`.
Change the module `.cgen` or handwritten C source, then rerun `make modules`.

## iPhone and iPad

Native Xcode projects live in each application folder: open
`apps/chat/chat.xcodeproj` and select `chat-ios`, or
`apps/octopus/octopus.xcodeproj` and select `octopus-ios`.
They support iPhone, iPad and Simulator, with shared schemes for Xcode Cloud.
Regenerate them offline using `sh tools/xcode/generate.sh --all`.

Build standalone iOS apps with Make and the Xcode command-line tools; no
Xcode project is required. Use `make chat-ios` or `make <app>-ios` to build,
sign, install and launch an `apps/<app>` project on a connected iPhone or iPad.
Start with `make ios-run` for the chat app in an
iPhone simulator, or `make ios-run DEVICE_TYPE=iPad` for an iPad simulator.
`make ios` builds an arm64 device bundle; `make ios-install` signs, installs,
and launches it using an installed development profile and identity.
See [iOS build and deployment](IOS.md) for prerequisites, outputs and options.
See [Android](ANDROID.md) for `make chat-android` and the NDK/Gradle requirements.

## Common failure meaning

- `cgen: dlopen failed ... invalid ELF header` — rebuild artifacts for codegen plugins are stale or incompatible.
- Test compile errors in `tests/test_*.c` usually indicate the test is calling an internal API with an outdated signature.

## For local models

If the task is primarily about build or validation:

1. Read this file.
2. Read `docs/TESTING.md`.
3. Read only the touched source file and one nearby reference if needed.
