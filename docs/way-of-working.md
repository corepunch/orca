# Way of Working

This guide captures the development conventions used in ORCA and explains *why* each step in the component workflow matters.  It is the companion to the technical reference in [Object + Component System](architecture/object-component-system.md).

---

## The component workflow

When you add any component to ORCA — whether refactoring an existing system or building something new — follow this order without exception:

```
1. XML declaration   →   2. Code generation   →   3. C handlers
     →   4. Registration   →   5. Xcode project   →   6. Tests
```

Each step depends on the previous one.  The steps are described in detail in [Object + Component System → How to Add a New Component Type](architecture/object-component-system.md#how-to-add-a-new-component-type).

---

## Why this order is mandatory

### Step 1: XML is the single source of truth

The `.xml` file drives everything else.  It declares:
- The `struct` that the component's data is stored in
- The FNV1a property hash IDs used in switch statements
- The Proc switch — the dispatch table that routes messages to C handlers
- The Lua bindings
- The `REGISTER_CLASS` macro that registers the class with the engine

**If you skip or delay the XML step**, you are writing C code that references symbols that don't exist yet (`struct MyComponent`, `ID_MyComponent_Foo`, `pMyComponent->field`).  The code will not compile, and there is no partial-working state to test against.

### Step 2: Verify codegen output

After running `cd tools && make`, open `<module>_export.c` and check that:
1. `struct MyComponent { ... }` has the expected fields
2. `MyComponentProc` has a `case` entry for every `<handle message="..."/>` you declared
3. `REGISTER_CLASS(MyComponent, ...)` appears at the bottom

An **empty `switch {}`** in the Proc is the single most common sign that you forgot a `<handle>` element in XML.  The component will attach and appear to work, but will silently ignore every message.

### Step 3: C handlers must match the XML exactly

A `HANDLER(MyComponent, NS, Event)` function in C corresponds to a `<handle message="NS.Event"/>` in XML.  The code generator forward-declares the function and adds it to the switch.  If the XML declaration is missing, the handler is an unreachable function with no caller.

### Step 4: Registration at module init

`OBJ_RegisterClass` must be called exactly once per class, at module startup.  If the call is missing, `obj:addComponent("MyComponent")` will succeed silently (it returns without attaching anything useful) — another class of invisible failure.

### Step 5: Xcode project

The GNU Makefile and the Xcode project are independent build systems.  A `.c` file added to a `source/*/components/` directory is picked up by `Makefile` automatically but **is invisible to Xcode** until manually added to `project.pbxproj`.  This means the GNU build may succeed while the macOS/Xcode build silently excludes the file and gets a link error.

### Step 6: Tests are not optional

A test file is the only way to catch the "component attaches but does nothing" failure mode that results from any missed step above.  A minimal test should:

- Attach the component to an object
- Set and get each property
- Trigger at least one message and assert a side-effect

Look at `tests/test_state_manager.lua` and `tests/test_styles_lua.lua` as templates.

---

## Use case walkthroughs

### "I want to add a new component to core"

1. Open `source/core/core.xml`
2. Add a `<class name="MyComponent">` with `<handles>`, `<properties>`, and optionally `<messages>`
3. Run `cd tools && make` — inspect `source/core/core_export.c` to confirm the struct and Proc switch look correct
4. Create `source/core/components/MyComponent.c` with `HANDLER(...)` implementations
5. Confirm `on_core_registered` in `core_export.c` calls `OBJ_RegisterClass(&_MyComponent)` (or add the call manually if the callback is hand-written)
6. Add `source/core/components/MyComponent.c` to `orca.xcodeproj/project.pbxproj` in all four required sections
7. Create `tests/test_mycomponent.lua`
8. Run `make unite && xvfb-run make test`

### "I want to add a component to a plugin (e.g. UIKit)"

Same as above, but replace `source/core/` with `plugins/UIKit/` (or the relevant plugin directory) and `core.xml` with `UIKit.xml`.

### "I want to migrate a legacy embedded Object field into a component"

This is the hardest use case.  In addition to the standard steps above:

1. **Do not break the existing API** — keep any public `OBJ_GetXxx` / `OBJ_SetXxx` functions working during the transition by forwarding to the component
2. **Remove the field from `struct Object`** only after the component is fully functional and tested
3. **Remove the `kCompXxx` enum entry** and the `_GetXxx(obj)` accessor macro from `core_local.h`
4. **Remove the manual release call** from `OBJ_Release`
5. **Delete the old `.c` file** (e.g. `stateman.c`, `alias.c`) once the component handles all the same behaviour

Do not leave both the old global implementation and the new component active at the same time — the dual-path state is invisible from the outside and leads to subtle bugs.

---

## Dependency rules

| Direction | Allowed? |
|---|---|
| Plugin → core | ✅ Yes |
| Core → plugin | ❌ No |
| Plugin A → plugin B | ⚠️ Avoid — use messages |

`source/core/` must never `#include` a header from `plugins/`.  This rule exists because the core is meant to be buildable and testable without any plugins loaded.  A core file that includes `<plugins/UIKit/UIKit.h>` creates a hard compile-time dependency on UIKit, breaks cross-platform builds that exclude UIKit, and makes the engine impossible to use without all plugins present.

If core code needs behaviour from a plugin (e.g. looking up a `struct Node` field), the correct approach is:
- Define a message (e.g. `Object.FindResourceByKey`) and have the plugin handle it
- Register a callback or accessor function pointer at startup via a core API
- Pass the data through a generic property lookup instead of a typed struct field

---

## Checklist for every new component MR

Use this as a pull request self-review checklist:

```
[ ] <class> declared in module XML with <handles>, <properties>, <messages>
[ ] cd tools && make run — no errors
[ ] Generated *_export.c has non-empty MyComponentProc switch
[ ] HANDLER(...) implementations in source/*/components/MyComponent.c
[ ] OBJ_RegisterClass(&_MyComponent) called at module init
[ ] MyComponent.c added to orca.xcodeproj/project.pbxproj (4 places)
[ ] tests/test_mycomponent.lua covers attach + properties + at least one message
[ ] make unite passes (C-only build)
[ ] xvfb-run make test passes (full test suite)
[ ] No #include <plugins/...> from source/core/
[ ] If migrating: old code path deleted, not running alongside the new component
```
