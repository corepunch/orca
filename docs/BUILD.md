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

On macOS, install the desktop build dependencies with Homebrew:

```bash
brew install pkgconf lua lz4 libxml2
```

The build uses `pkg-config` to find headers and libraries. It prefers the
`lua5.4` package when available, then falls back to `lua`. Missing packages
are reported by name; having the `lua` executable alone does not establish
that all build dependencies are available.

To build without LZ4-compressed package support (and without installing `lz4`):

```bash
make no-lz4
```

## Authoritative commands

```bash
make modules
make unite
make test-headless
xvfb-run make test
```

## Scene format selection

Scene readers are selected at compile time. XML is the default; Lisp is enabled with `-DORCA_USE_LISP`, an
alternative build using the same object construction and lifetime handling:

```bash
make unite
make CPPFLAGS=-DORCA_USE_LISP BUILDDIR=build-lisp unite
make test-headless
make CPPFLAGS=-DORCA_USE_LISP BUILDDIR=build-lisp test-headless
```

Use separate output directories to keep both binaries. `test-headless` runs the
common engine tests and the selected-format loader regression in both builds;
Lisp fixture tests run only in the Lisp build, and application tests that load
pages (`XML_APP_LUA_TESTS`) run only in the XML build. `test-scene-format`
is the focused selection test.

Define `ORCA_USE_LISP` in any build system to choose Lisp, as declared in
`include/scene_format.h`. Make also accepts `SCENE_FORMAT=lisp` as a shorthand
for adding `-DORCA_USE_LISP`; raw flags in `CPPFLAGS` or `CFLAGS` select the same
mode. WebGL builds receive the same selection. The unselected scene reader is not
compiled and its Lua file/string loader APIs are absent; C entry points remain
as unavailable stubs for the generated ABI. The generic loader, library/font
lookup, cloning, editor scene saves and project scaffolding follow the selection.

The public I/O API is format-neutral: `loadObjectFromDisk(path)`,
`loadObjectFromString(source)`, and `serializeObjectToString(object)`.
Legacy format-specific names remain compatibility aliases for the active
reader. Parser selection lives in one `fs_scene.c` block; the configuration
header contains the other format-selection block. The readers share object
construction and registration through one immutable descriptor.

Applications are authored in XML. Lisp is an experimental format: only
`apps/Example` includes Lisp scenes, and its extensionless startup paths select
the matching assets. Every other application requires the default XML build;
do not add `.lisp` twins to them. This switch concerns scene
reading only. XML data/schema parsing, libxml2, and both serialization utilities
remain available independently.

## What each command is for

- `make modules` — regenerate code from every `.cgen` file.
- `make unite` — build the core library, plugins, and app without running tests.
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
`apps/cluster-demo/cluster-demo.xcodeproj` and select `cluster-demo-ios`.
They support iPhone, iPad and Simulator, with shared schemes for Xcode Cloud.
Regenerate them offline using `sh tools/xcode/generate.sh --all`.

Build standalone iOS apps with Make and the Xcode command-line tools; no
Xcode project is required. Start with `make ios-run` for the chat app in an
iPhone simulator, or `make ios-run DEVICE_TYPE=iPad` for an iPad simulator.
`make ios` builds an arm64 device bundle; `make ios-install` signs, installs,
and launches it using an installed development profile and identity.
See [iOS build and deployment](IOS.md) for prerequisites, outputs and options.

## Common failure meaning

- `cgen: dlopen failed ... invalid ELF header` — rebuild artifacts for codegen plugins are stale or incompatible.
- Test compile errors in `tests/test_*.c` usually indicate the test is calling an internal API with an outdated signature.

## For local models

If the task is primarily about build or validation:

1. Read this file.
2. Read `docs/TESTING.md`.
3. Read only the touched source file and one nearby reference if needed.
