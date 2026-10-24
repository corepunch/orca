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

## Authoritative commands

```bash
make modules
make unite
make test-headless
xvfb-run make test
```

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
