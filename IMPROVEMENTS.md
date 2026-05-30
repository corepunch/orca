# Improvements and Guardrails

This document records the lessons from the `Screen.SetModalObject` work and
the follow-up review. The short version is: the architecture is coherent, but it
has sharp edges. Small API changes can cross `.cgen`, generated C, generated
message-action classes, component dispatch, Lua wrappers, tests, and compiled
runtime shared libraries. When one layer is stale, the source can look correct
while runtime behavior still looks like nothing changed.

This is not a sign that the system is rotten. It is a sign that the workflow
needs stronger guardrails.

See also `docs/improvements.md` for older component, codegen, and Lua bridge
gotchas.

---

## Assessment

The assessment from the thread is broadly correct:

- The code is not horrible; it is systems code with generated glue.
- Adding one message can legitimately touch several layers:
  - `.cgen` model
  - generated headers
  - generated property IDs
  - generated Lua export bindings
  - generated message-action metadata
  - object/component dispatch
  - Lua method wrappers
  - tests
  - runtime shared libraries
- A stale generated artifact or shared library can make source review
  misleading.
- That is exactly what happened with `Screen.SetModalObject`: the model and
  export path were basically right, but the loaded UIKit runtime was stale, so
  Lua could not build the message payload and the handler did not fire.

The system has a coherent shape. `screen:SetModalObject(popup)` fits it well:
the message has a payload, the generated message-action class exposes that
payload as properties, the Lua bridge fills the action from positional or table
arguments, and the Screen component handler attaches the object. The roughness
is in the workflow and in a few implementation details.

The right label is "sharp-edged", not "rotten".

---

## Why `Screen.SetModalObject` Crosses So Many Layers

`Screen.SetModalObject` is a useful example because it looks small:

```lua
screen:SetModalObject(popup)
```

But this shorthand depends on a full path through the engine:

1. `plugins/UIKit/UIKit.cgen` declares `Screen.SetModalObject` and its `Target`
   payload field.
2. Codegen emits:
   - `generated/UIKit/UIKit.h`
   - `generated/UIKit/UIKit_properties.h`
   - `generated/UIKit/UIKit_export.c`
3. Codegen synthesizes the `Screen.SetModalObject` action class with payload
   properties plus `Target` and `Mode` dispatch properties.
4. `source/core/property/property_lua.c` turns event properties into Lua
   callable closures.
5. `source/core/object/object_lua_msg.c` creates the generated action object
   and copies Lua arguments into its payload properties.
6. `ScreenProc` dispatches `ID_Screen_SetModalObject` to the handler.
7. `plugins/UIKit/Screen.c` attaches the popup and marks it visible.
8. The compiled `build/lib/liborca/UIKit.so` must actually contain the updated
   generated export and handler code.

If any generated file or compiled shared library is stale, source can look
correct while runtime dispatch behaves as if the message has no payload or no
handler.

---

## What Went Right

The `SetModalObject` change follows the intended architecture:

- The message is declared in `plugins/UIKit/UIKit.cgen`.
- The generated `Screen_SetModalObjectEventArgs` struct exists.
- The generated property ID exists as `ID_Screen_SetModalObject`.
- `ScreenProperties` includes `SetModalObject` as a `kDataTypeEvent`.
- `ScreenProc` has a dispatch case for `ID_Screen_SetModalObject`.
- `UIKit_export.c` registers the generated `Screen.SetModalObject` action class.
- Lua method shorthand resolves through the normal event property path.
- `tests/test_layout.lua` now includes a direct regression test:
  `test_lua_set_modal_object_dispatches_message`.

The targeted test passed after rebuilding:

```text
PASS: test_lua_set_modal_object_dispatches_message
All layout tests passed.
```

`make test-message-registry` also passed. The target name is historical now;
the useful coverage is that message metadata is reachable through generated
classes:

```text
3 test(s) run, 0 failure(s)
```

These are good signs. The source design is not the problem.

---

## What Went Wrong

The failure mode was a stale runtime shared library.

`Screen.SetModalObject` existed in source and generated exports, but the runtime
loaded an old `UIKit.so`. Because UIKit is a plugin shared library loaded at
runtime, a stale file can leave Lua unable to see the new message payload or
leave the component dispatch table without the new handler.

The particularly sharp edge is that focused Lua test targets can allow this.
For example, `make test-layout` depends on:

```make
app copyshare
```

It does not force `unite` or `buildplugins`. That means it can relink the app
and copy Lua files while still using an old plugin shared library.

`make test-headless` is safer because it starts with `unite`, which rebuilds
plugins, but the focused test path remains a trap.

---

## Current Concrete Findings

### P1: Focused Lua tests can use stale plugin libraries

Most focused Lua tests in `Makefile` use the `LUA_TEST_RULE` with dependencies
such as:

```make
$(eval $(call LUA_TEST_RULE,test-layout,tests/test_layout.lua,app copyshare))
```

That builds the app and copies shared Lua files, but it does not necessarily
rebuild `build/lib/liborca/*.so` plugin libraries.

This exactly matches the `Screen.SetModalObject` failure mode:

- source says the message exists
- generated C says the message exists
- Lua test starts a fresh app binary
- stale `UIKit.so` is still loaded
- runtime behavior looks like the message did not change

Recommended fix:

- Make all Lua tests that can load plugins depend on `unite`, or at least
  `buildplugins app copyshare`.
- Keep the faster path only for tests that provably do not load plugins.
- Consider splitting test rules into `LUA_CORE_TEST_RULE` and
  `LUA_PLUGIN_TEST_RULE` so the dependency choice is explicit.

### P2: `_AttachModalObject` can over-retain moved objects

`plugins/UIKit/Screen.c` manually unlinks a modal target from its old parent:

```c
if (target->parent) {
  REMOVE_FROM_LIST(struct Object, target, target->parent->children);
  REMOVE_FROM_LIST(struct Object, target, target->parent);
}
```

Then it adds a new reference:

```c
OBJ_AddRef(target);
```

This differs from `OBJ_AddChild`, which releases the old parent-held reference
when moving a child between parents. Re-showing or moving an existing popup can
therefore over-retain it.

Recommended fix:

- Prefer reusing `OBJ_AddChild` if modal objects should follow normal child
  ownership semantics.
- If modal objects must live in the `next` chain rather than `children`, factor
  a helper that mirrors `OBJ_AddChild`'s refcount behavior.
- Add a regression test that moves an already-parented popup into modal
  position and verifies object retention after close/clear.

### P2: `SetModalObject` accepts non-`Popup` targets

`Screen.ShowModal` validates that the loaded object is a `Popup`:

```c
if (!GetPopup(target)) {
  Con_Error("Screen.ShowModal template '%s' is not a Popup", pShowModal->Path);
  OBJ_ReleaseRef(target);
  return FALSE;
}
```

`Screen.SetModalObject` does not perform the equivalent validation. It calls
`_AttachModalObject` directly. `_AttachModalObject` attempts to reset
`DialogResult`, logs if that property is missing, and still returns `TRUE`.

That makes `SetModalObject` a looser API than `ShowModal`, even though the
documentation describes it as the in-memory counterpart.

Recommended fix:

- Reject non-`Popup` targets before attachment.
- Return `FALSE` if `DialogResult` cannot be reset.
- Add a Lua test that `screen:SetModalObject(ui.TextBlock {})` fails and does
  not attach the object.

---

## Recommended Workflow for New Messages

Every new message should follow this workflow:

1. Declare the message in the owning module `.cgen`.
2. Add the owning class `<handle message="..."/>` if a component handler should
   receive it.
3. Run `make modules`.
4. Inspect generated files:
   - the event args struct exists
   - the property ID exists
   - the event property exists
   - the generated dotted message-action class exists
   - the Proc switch has a dispatch case
   - payload fields appear as generated action properties
5. Implement the C `HANDLER(...)`.
6. Rebuild the runtime plugin or full `unite` target.
7. Add a focused regression test that calls the Lua shorthand or dispatch path.
8. Run the focused test through a target that rebuilds plugins.
9. Run `make test-headless` before merging.

The important rule: source review is not enough for message work. Generated
class metadata and loaded shared libraries must be verified.

---

## Regression Tests to Add for Every Message

For every new message, add a tiny test that checks the real runtime path:

- Lua shorthand resolves:
  - example: `root:SetModalObject(popup)`
- Payload fields arrive in C:
  - example: `Target` is not `NULL`
- The handler fires:
  - example: `root:getNext() == popup`
- Side effects happen:
  - example: popup becomes visible and `DialogResult` resets
- Bad input fails predictably:
  - missing payload
  - wrong object type
  - unknown path or target

For generated message-action metadata coverage, add C tests where practical:

- `OBJ_FindClass("Screen.SetModalObject")` returns the generated action class.
- Field names and count match the `.cgen` declaration.
- Dispatch meta-properties such as `Target` and `Mode` are present.
- Unknown message-action classes return `NULL`.

---

## Build Guardrails

### Rebuild plugins for plugin-dependent Lua tests

Change plugin-dependent Lua tests to depend on `unite` or `buildplugins`.

Candidate direction:

```make
define LUA_PLUGIN_TEST_RULE
$(1): unite
	$(Q)echo "Running $(1)"
	$(Q)$(TARGET) -test=$(2)
endef
```

Use this for tests that load UIKit, SceneKit, SpriteKit, or other plugin
classes. Keep a separate lightweight rule only for tests that exercise core Lua
modules without plugin shared libraries.

### Add a runtime message-action verification command

Add a small dev command that verifies the loaded runtime, not just generated
source. It should:

- start the built app or a small test binary
- load `orca.UIKit`
- query `OBJ_FindClass` for generated dotted message-action classes
- verify known message payload properties
- optionally print the loaded plugin path and modification time

Example checks:

- `Screen.SetModalObject` exists as a generated class
- payload count is `1`
- payload field is `Target`
- payload field type is `Object`
- dispatch `Mode` exists and defaults to `Send`
- `ScreenProc` dispatches the message

### Make stale plugin diagnosis obvious

When Lua tries to dispatch a message but the generated message-action class is
missing from the loaded runtime, the error can appear far from the real cause.

Possible improvements:

- Log when `_CreateMessageAction` cannot find a generated class for a message
  that has an event property.
- Include the message name and loaded module path in the warning.
- Add a debug-only check that generated event property metadata agrees with the
  generated message-action class metadata.

---

## Design Notes About Objects, Components, and Messages

The thread originally called out that components are classes while Lua often
sees everything through generic `Object`. That is not inherently a problem. It
is a useful boundary: C can keep component storage, generated structs, and
dispatch tables as implementation detail, while Lua works with object identity
and capabilities.

This is exactly why the Lua surface can stay pleasant:

```lua
screen:SetModalObject(popup)
```

The problem was not that Lua sees an `Object`. The problem was that messages
used to live partly outside the class/property/object system. Lua had to build
payload buffers by hand, XML needed a generic adapter, and payload metadata
lived in a separate runtime registry. That made messages feel like a parallel
abstraction instead of another generated class with properties.

Generated message actions improve the shape:

- message metadata now lives in the generated `ClassDesc`
- XML can use dotted message classes directly, such as `<Screen.ShowModal/>`
- Lua dispatch creates the generated action and fills its payload properties
- `SendMessageAction` provides one generic `Action.Dispatch` handler for all
  generated message actions
- C handlers still receive the same typed `EventArgs` payload structs

So the practical improvement is not to make Lua expose component internals. The
practical improvement is to keep messages inside the same class/property/object
machinery that objects and actions already use.

The remaining guardrail work is to verify the boundaries:

- `.cgen` agrees with generated files
- generated files agree with committed source
- committed source agrees with compiled libraries
- compiled libraries expose the generated message-action classes Lua uses at
  runtime
- focused tests rebuild the libraries they depend on

---

## Specific `Screen.SetModalObject` Cleanup Plan

### 1. Tighten target validation

`Screen.SetModalObject` should reject invalid targets before attachment:

```c
if (!pSetModalObject || !pSetModalObject->Target) {
  Con_Error("Screen.SetModalObject missing Target");
  return FALSE;
}

if (!GetPopup(pSetModalObject->Target)) {
  Con_Error("Screen.SetModalObject target is not a Popup");
  return FALSE;
}
```

This makes the API match `Screen.ShowModal`.

### 2. Fix ownership when moving an existing object

The modal attach path should not leak references when moving an object from an
existing parent. Either route through `OBJ_AddChild` or duplicate its ownership
semantics exactly.

Questions to answer before patching:

- Should modal objects belong in `children` or only in the `next` chain?
- Is `next` being used as a modal overlay chain separate from normal children?
- Should `Object.Attached` and `StyleController.ThemeChanged` fire for modal
  attachment?

If the answer is yes, `OBJ_AddChild` may be the right primitive. If the answer
is no, create a dedicated modal-chain helper that still handles refcounts.

### 3. Add negative tests

Add tests for:

- `SetModalObject(nil)` fails
- `SetModalObject(nonPopup)` fails
- moving an already-parented popup does not leak or leave stale parent links
- closing a modal after a move detaches and releases correctly

### 4. Add generated message-action coverage

Extend `tests/test_message_registry.c` with:

- `Screen.SetModalObject` exists as a generated action class
- count is `1`
- field name is `Target`
- field type is `kDataTypeObject`
- type string is `Object`
- `Target` and `Mode` dispatch properties exist

This test should load UIKit, not only core, otherwise it will miss plugin
message-action metadata problems.

---

## Documentation Improvements

The docs already say generated files are committed and that `.cgen` changes
require `make modules`. They should also spell out the runtime shared library
trap:

- A generated source diff is not enough.
- A relinked app is not enough.
- Plugin changes require rebuilding the plugin `.so`.
- Focused Lua tests must be run through a target that rebuilds the plugin.

Suggested wording:

> If a message is declared in `.cgen` and visible in generated source but Lua
> still cannot send its payload, suspect a stale plugin shared library. Rebuild
> with `make unite` or run a plugin-aware test target before debugging the
> bridge.

---

## Checklist for Future Contributors

When adding a message:

- [ ] Add the message to `.cgen`.
- [ ] Add the handler declaration in `<handles>` if needed.
- [ ] Run `make modules`.
- [ ] Confirm generated event args exist.
- [ ] Confirm generated property IDs exist.
- [ ] Confirm generated `Proc` dispatch exists.
- [ ] Confirm the generated dotted message-action class exists.
- [ ] Implement `HANDLER(...)`.
- [ ] Rebuild plugins with `make unite` or equivalent.
- [ ] Add a Lua test for shorthand dispatch.
- [ ] Add a generated-action metadata test for payload fields.
- [ ] Run a plugin-aware focused test.
- [ ] Run `make test-headless`.

When debugging a message that "does nothing":

- [ ] Check the `.cgen` message declaration.
- [ ] Check `<handle message="..."/>`.
- [ ] Check generated IDs and dispatch switch.
- [ ] Check the generated dotted message-action class.
- [ ] Check the handler signature and payload struct name.
- [ ] Check Lua event property exposure.
- [ ] Check loaded plugin `.so` timestamp.
- [ ] Rebuild with `make unite`.
- [ ] Re-run the focused regression test.

---

## Bottom Line

The architecture can support this style of API nicely. `screen:SetModalObject`
is a good fit for the system. The pain came from weak guardrails between source,
generated code, and loaded runtime artifacts.

The highest-value improvements are:

1. Make plugin-dependent Lua tests rebuild plugins.
2. Add generated message-action metadata tests for every new payload message.
3. Add a dev command that verifies loaded runtime message-action classes.
4. Tighten `Screen.SetModalObject` validation.
5. Fix modal attachment ownership when moving existing objects.

Those changes would turn this from a spelunking exercise into a boring,
repeatable workflow. Boring is exactly what we want here.
