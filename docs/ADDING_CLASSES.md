# Adding Classes

Use this file when adding or changing a component/class in core or a plugin.

## Mandatory workflow

1. Edit the module `.cgen`.
2. Run `make modules`.
3. Implement or update the C handlers.
4. Confirm registration happens at module startup.
5. Add the handwritten `.c` file to Xcode if it is new.
6. Add or update focused tests.

## Where to edit

- Core classes: `source/core/core.cgen` and `source/core/components/`
- UIKit classes: `plugins/UIKit/UIKit.cgen` and `plugins/UIKit/*.c`
- Other plugins: `<plugin>/<Plugin>.cgen` and nearby handwritten `.c` files

## What the `.cgen` must contain

At minimum, document and declare:

- `<class name="...">`
- `<summary>`
- `<handles>` for every message the class responds to
- `<properties>` for public state
- `<messages>` when the class emits messages
- `<details>` when lifecycle or usage rules are non-obvious

## Local-model documentation checklist for `.cgen`

Treat `.cgen` docs as compressed context.
For every public class or interface, prefer to include:

- what the type is for
- when it is created or attached
- what messages it handles or emits
- what each property means, including units or expected values
- what state changes or side effects matter to callers
- topic separators for large interfaces so method groups stay readable

## Validation

- `make modules`
- the narrowest relevant test target
- broader build/test commands only if the change touches shared behavior

## Common failure signals

- Empty generated `*Proc` switch: missing `<handle>` entries
- Lua API missing: the `.cgen` declaration does not match the intended export
- Make build works but Xcode fails: new `.c` file was not added to `orca.xcodeproj/project.pbxproj`
