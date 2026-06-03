# Typedata Migration Plan

Detailed step-by-step plan for migrating from the linked component storage model
to the monolithic `typedata[]` model described in
[object-typedata-model.md](object-typedata-model.md).

## Background

The current `Object` holds a linked list of `component` allocations. A `Button`
is assembled from separate `Button`, `TextBlock`, `TextRun`, `Node2D`,
`StyleController`, and `Node` component blocks. The goal is to replace that with
a single monolithic typedata block embedded directly in `Object`, one block per
storage family.

The branch `codex/property-unfolding-shorthands` has already completed:

- Shorthand properties unfolded into atomic leaf properties
- `TextBlockConcept` removed; text storage moved into `TextBlock` directly
- Per-property dynamic value storage allocated at the `Property` level
- Inherited property slots hardened and propagation tested

What remains is the structural migration described below.

## What stays

**Everything-is-an-Object stays.** The engine's scene graph, XML parser,
bindings, Lua scripting, CSS, and diagnostics all speak the Object/property
language. That stays intact.

**The `struct Property` linked list stays — but only for properties that need
it.** The key insight is that there are two fundamentally different kinds of
properties:

1. **UIData properties** — `FontSize`, `BackgroundColor`, `Width`, `Source`, etc.
   These are declared in `.cgen`, always present on every UI object, live in
   fixed offsets in `struct UIData`. They do *not* need a heap-allocated
   `Property` node for storage — the value already lives in `UIData`. The
   `Property` node is only needed when the property has an active binding,
   animation, or change callback.

2. **User-defined / project properties** — `Card.Title`, `Card.Subtitle`,
   `Card.PrimaryColor`, etc. Declared in `package.lua` under `PropertyTypes`.
   Set as XML attributes (`Card.Title="2.4 M"`), bound via `{Binding Card.Title}`
   in prefabs. These have *no* fixed storage slot — they are genuinely dynamic
   bags attached to whichever node the author puts them on. The `Property` linked
   list is the right model for these: sparse, per-instance, runtime-typed.

The migration narrows `struct Property` to its legitimate use: dynamic,
user-defined, or binding-active properties. UIData fields read directly from the
struct; a Property node is only allocated when a binding or callback is attached.

---

## Phase 1 — Add `class_id`, `super_id`, and `type` to Object [Done]

**Goal:** Give every object an explicit concrete-class identity and a
storage-family identity, without breaking anything yet.

**Files:**
`source/core/object/object_internal.h`,
`source/core/object/object_core.c`,
`include/orca.h`

**Steps:**

1. Add three fields to `struct Object` after the existing identity fields:
   ```c
   uint32_t class_id;             /* concrete class — Button, TextBlock, … */
   uint32_t super_id;             /* storage family — Node2D, Node3D, Resource, … */
   struct ClassDesc const *type;  /* pointer to concrete ClassDesc */
   ```

2. In `OBJ_Create(uint32_t class_id)`, after looking up the `ClassDesc`:
   - Set `obj->class_id = class_id`.
   - Set `obj->type = cls`.
   - Determine `super_id` by walking `cls->ParentClasses` and checking against
     the known storage-family roots: `ID_Node2D`, `ID_Node3D`, `ID_Resource`,
     `ID_Action`, `ID_Project`. Use the first match; fall back to `ID_Node`.
   - Set `obj->super_id = super_id`.

3. No behavior changes. Component list untouched.

**Verification:** Build passes. Existing tests unchanged.

---

## Phase 2 — Define `struct UIData` [Done]

**Goal:** Create the monolithic storage struct for all UIKit objects and an
accessor. No allocation changes yet.

**Files:**
`plugins/UIKit/ui_data.h` *(new)*,
`plugins/UIKit/UIKit.c`

**Steps:**

1. Create `plugins/UIKit/ui_data.h`:
   ```c
   struct UIData {
     struct Node            Node;
     struct Node2D          Node2D;
     struct StyleController StyleController;
     struct TextRun         TextRun;
     struct TextBlock       TextBlock;
     struct ImageView       ImageView;
     struct Grid            Grid;
     struct StackView       StackView;
     struct Button          Button;
     struct Input           Input;
     /* extend as remaining UIKit classes are migrated */
   };
   ```
   Base classes first so offsets are stable as the struct grows.

2. Add stub accessor:
   ```c
   struct UIData *Object_UIData(struct Object *object);  /* returns NULL for now */
   ```

**Verification:** `UIKit.c` compiles. No runtime changes.

---

## Phase 3 — Embed `typedata[]` in Object and allocate UIData [Done]

**Goal:** Objects with `super_id == ID_Node2D` carry a live `UIData` block.
`Object_UIData` returns a real pointer. The `data[MAX_OBJECT_DATA]` blob is
removed.

**Files:**
`source/core/object/object_internal.h`,
`source/core/object/object_core.c`,
`plugins/UIKit/UIKit.c`

**Steps:**

1. Replace `byte_t data[MAX_OBJECT_DATA]` with `char typedata[]` as the last
   field of `struct Object`:
   ```c
   struct Object {
     ...
     longTime_t dirty;
     char typedata[];
   };
   ```

2. In `OBJ_Create`, size the allocation by storage family:
   ```c
   size_t typedata_size = 0;
   if (super_id == ID_Node2D) typedata_size = sizeof(struct UIData);
   if (super_id == ID_Node3D) typedata_size = sizeof(struct SceneData);

   struct Object *obj = calloc(1, sizeof(struct Object) + typedata_size);
   ```

3. Apply per-family defaults after zero-fill:
   ```c
   if (super_id == ID_Node2D) {
     struct UIData *data = (struct UIData *)obj->typedata;
     Node_ApplyDefaults(&data->Node);
     Node2D_ApplyDefaults(&data->Node2D);
     /* etc. */
   }
   ```

4. Wire `Object_UIData`:
   ```c
   struct UIData *Object_UIData(struct Object *object) {
     if (!object || object->super_id != ID_Node2D) return NULL;
     return (struct UIData *)object->typedata;
   }
   ```

5. Update `GetNode`, `GetNode2D`, etc. to prefer typedata, fall back to
   component list during transition:
   ```c
   #define GetNode2D(_P) \
     ((_P) && (_P)->super_id == ID_Node2D \
       ? &((struct UIData *)(_P)->typedata)->Node2D \
       : (struct Node2D *)OBJ_GetComponent(_P, ID_Node2D))
   ```

**Verification:** `tests/test_layout.lua`, `tests/test_properties.c`. UI renders
identically.

---

## Phase 4 — Redirect property descriptor offsets into UIData [Done]

**Goal:** `PropertyType.Offset` values reference `offsetof(struct UIData, …)`
so property reads/writes go directly into the embedded typedata. The `Property`
heap node is only needed for bindings and user-defined properties — not for plain
UIData field storage.

**Files:**
`tools/codegen/plugins/export.c`,
`tools/codegen/plugins/header.c`

**Steps:**

1. In codegen, tag UIKit classes with their storage family struct (`UIData`).

2. Change emitted offsets from component-relative:
   ```c
   offsetof(struct TextRun, Font.Family)
   ```
   to UIData-relative:
   ```c
   offsetof(struct UIData, TextRun.Font.Family)
   ```
   Same change for `PropertyShorthandTarget.Offset`.

3. In `OBJ_GetPropertyValue` and the XML/CSS parser: when reading or writing a
   UIData property on a Node2D object, apply the offset against `obj->typedata`
   directly — no `Property` node allocation required. Only allocate a `Property`
   node when:
   - a binding (`{Binding …}`) is attached to the property, or
   - a Lua change callback is registered, or
   - the property is a user-defined project property (not in UIData at all).

4. Regenerate all `.cgen` outputs and rebuild.

**Verification:** Full `tests/test_properties.c`. Bindings (`{Binding Card.Title}`
in prefabs) still work. Plain XML attribute reads (`FontSize="20"`) no longer
allocate a `Property` node — verify with a malloc count or ASAN.

---

## Phase 5 — Flatten message dispatch to single class proc per UIKit class

**Goal:** Each concrete UIKit class proc calls explicit base procs rather than
walking the component list. Lifecycle messages use broadcast semantics.

**Files:**
`plugins/UIKit/Button.c`,
`plugins/UIKit/TextBlock.c`,
`plugins/UIKit/Screen.c`,
`plugins/UIKit/Input.c`,
`plugins/UIKit/RadioButton.c`,
`plugins/UIKit/Tab.c`,
remaining UIKit `.c` files with a proc.

**Steps:**

1. For each class proc, replace component-walk dispatch with an explicit ordered
   call chain:
   ```c
   LRESULT ButtonProc(struct Object *obj, uint32_t msg, wParam_t w, lParam_t l)
   {
     struct UIData *data = Object_UIData(obj);

     switch (msg) {
       case ID_Node_LeftButtonUp:
         return Button_LeftButtonUp(obj, data, w, l);
     }

     if (TextBlockProc(obj, msg, w, l)) return TRUE;
     if (Node2DProc(obj, msg, w, l))   return TRUE;
     if (StyleControllerProc(obj, msg, w, l)) return TRUE;
     return NodeProc(obj, msg, w, l);
   }
   ```

2. Lifecycle messages (create, destroy, release) broadcast unconditionally — all
   base procs called regardless of return value.

3. Remove any `OBJ_SendMessageToComponents` calls from UIKit procs.

**Verification:** App smoke test. `tests/test_layout.lua` passes.

---

## Phase 6 — Remove component infrastructure for fixed classes

**Goal:** `Object.components` linked list and `struct component` are gone for
objects covered by a typedata family.

**Files:**
`source/core/object/object_internal.h`,
`source/core/object/object_component.c`,
`source/core/object/object_core.c`,
all `GetXxx` accessor macros.

**Steps:**

1. Audit every `OBJ_AddComponent` / `OBJ_GetComponent` call site. For fixed
   UIKit classes, remove the call — storage is in typedata. For genuinely
   runtime-attached behavior, keep a single optional `extension` pointer on
   `Object`.

2. Remove `struct component` and `Object.components` once no fixed-class call
   sites remain.

3. Simplify accessors to direct casts:
   ```c
   #define GetNode2D(_P) \
     ((_P) ? &((struct UIData *)(_P)->typedata)->Node2D : NULL)
   ```

4. Remove `object_component.c` or reduce it to extension-slot logic only.

**Verification:** Full test suite + ASAN. No dangling pointers. Binary size
decreases.

---

## Phase 7 — Migrate remaining concept components

**Goal:** Any surviving pseudo-class "concept" components (like the already-
removed `TextBlockConcept`) are folded into the owning struct or class proc.

**Steps:**

1. Grep for classes whose `ClassSize == 0` or that exist solely to attach
   properties to other classes.

2. For each: fold data fields into the storage family struct, fold message
   handlers into the concrete class proc's call chain, delete the concept class.

3. Verify `Grid.Column` as an attached property: memory lives in
   `UIData.Grid.Column`, but `GridProc` is never dispatched on the child `Button`.

**Verification:** XML/Lua round-trips for attached properties. Editor property
introspection still resolves `Category.Name` correctly.

---

## What the Property list is for after this migration

After Phase 4, `struct Property` nodes exist **only** for:

| Case | Example |
|---|---|
| Active binding on a UIData field | `Text="{Binding Card.Title}"` — the Text field has a binding |
| Lua change callback registered | `obj:onPropertyChanged("FontSize", fn)` |
| User-defined project property | `Card.Title`, `Card.PrimaryColor` from `package.lua` |
| Animated property | a running animation targets `Node2D.Opacity` |

Plain XML attribute writes like `FontSize="20"` or `Width="120"` write directly
into `UIData` with no heap allocation. The `Property` list becomes sparse — only
nodes that are *active* in some dynamic sense.

This keeps the full binding and project-property system (`Card.*` in prefabs,
`{Binding …}` expressions, CSS, Lua callbacks) without the overhead of
allocating a `Property` node for every static field on every object.

---

## Cross-cutting constraints

- **Class identity ≠ storage family.** A `Button` with `Grid.Column` memory is
  still a `Button`; `GridProc` is never dispatched on it.
- **Property metadata preserved.** `Category.Name`, type, and tooling metadata
  survive. Only `Offset` values change (component-relative → UIData-relative).
- **User-defined properties are unaffected.** `Card.Title` and friends never had
  a UIData slot and never will. They live in `Property` nodes as before.
- **Phases 1–3 are non-breaking.** Component list coexists until Phase 6. Do not
  remove component infrastructure before Phases 4 and 5 are tested.

---

## Test checkpoints

| After phase | Test to run |
|---|---|
| 1 | Build passes; existing tests unchanged |
| 2 | `UIKit.c` compiles; no runtime regressions |
| 3 | `tests/test_layout.lua`, `tests/test_properties.c` |
| 4 | Full property suite; verify bindings still work; check malloc count for static attrs |
| 5 | App smoke test; `tests/test_layout.lua` |
| 6 | Full suite + ASAN; binary size check |
| 7 | XML/Lua round-trip; prefab `{Binding Card.*}` works; editor introspection correct |
