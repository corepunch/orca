# Object Typedata Model

This is the planned replacement direction for ORCA's current linked component
storage. The goal is to keep the WinAPI/WPF-style message and property model,
but make object memory direct, predictable, and easy to inherit through.

Today an `Object` owns a linked list of component allocations. A `Button`
object is assembled from separate `Button`, `TextBlock`, `TextRun`, `Node2D`,
`StyleController`, and `Node` component blocks. That made dynamic composition
possible, but most engine classes already behave like fixed inheritance chains.
The list adds allocation, lookup, and property wiring overhead to the common
case.

The new direction is:

```c
struct Object {
  ...
  uint32_t class_id;          /* Button, ImageView, Grid, Mesh, Texture, ... */
  uint32_t super_id;          /* Node2D, Node3D, Resource, Action, Project, ... */
  struct TypeDesc const *type;
  char typedata[];
};
```

`class_id` identifies what the object is. `super_id` identifies which backing
storage family is present and which broad engine rules apply. The storage family
is not the same thing as the concrete class.

## Storage Families

A storage family is a monolithic typedata struct for a coherent object world.
For UIKit, most objects already share the same base behavior, so a single UI
storage block is reasonable:

```c
struct UIData {
  struct Node Node;
  struct Node2D Node2D;
  struct StyleController StyleController;

  struct TextRun TextRun;
  struct TextBlock TextBlock;
  struct ImageView ImageView;
  struct Grid Grid;
  struct StackView StackView;
  struct Button Button;
  struct Input Input;
  ...
};
```

Every UI object has this storage, but every UI object does not semantically
become every UI class. A `Button` can have memory for `Grid.Column` while still
being a `Button`; the parent `Grid` can read the attached layout fields from the
child's UI storage.

Likely storage families:

| `super_id` | Backing storage | Examples |
|---|---|---|
| `Node2D` | `struct UIData` | `Button`, `TextBlock`, `Grid`, `ImageView`, `Screen` |
| `Node3D` | `struct SceneData` | `Node3D`, `Model3D`, `Light3D`, `Viewport3D` |
| `Resource` | resource typedata | `Texture`, `Shader`, `FontFamily`, `Mesh` |
| `Action` | action/message typedata | `Setter`, generated message actions |
| `Project` | project/package typedata | filesystem packages, project roots |

The exact list can evolve. The important rule is that hot-path storage is chosen
once when the object is created.

## Message Dispatch

Each object has one concrete class proc instead of one proc per attached
component. That proc delegates to default/base procs in the same spirit as
WinAPI's `DefWindowProc`:

```c
LRESULT
ButtonProc(struct Object *object, uint32_t message, wParam_t w, lParam_t l)
{
  struct UIData *data = Object_UIData(object);

  switch (message) {
    case ID_Node_LeftButtonUp:
      return Button_LeftButtonUp(object, data, w, l);
  }

  if (TextBlockProc(object, message, w, l)) return TRUE;
  if (Node2DProc(object, message, w, l)) return TRUE;
  if (StyleControllerProc(object, message, w, l)) return TRUE;
  return NodeProc(object, message, w, l);
}
```

Some lifecycle messages, such as create/destroy/release, may need broadcast
semantics through the default chain rather than "first handled wins". The
generated proc should make that explicit per message.

## Properties

Property descriptors point into the storage family, not into separately
allocated component blocks. For a UI object:

```c
TextRun.Font.Style  -> offsetof(struct UIData, TextRun.Font.Style)
ImageView.Source    -> offsetof(struct UIData, ImageView.Source)
Grid.Columns        -> offsetof(struct UIData, Grid.Columns)
Grid.Column         -> offsetof(struct UIData, Grid.Column)
```

The descriptor still records the public category/name (`TextRun.FontStyle`,
`ImageView.Source`, `Grid.Column`) and the concrete class metadata still decides
which properties are valid to author on a given class.

This makes attached properties simple. A `Button` inside a `Grid` can store
`Grid.Column` in `data->Grid.Column`; this does not activate `GridProc` on the
button.

## Shorthands

Shorthands should become structured authoring again rather than storage tricks:

```c
data->TextRun.Font.Style
data->TextRun.Font.Weight
data->TextRun.Font.Size
data->TextRun.Font.Family
```

Setting `TextRun.Font` writes several leaf properties. Setting
`TextRun.Font.Style` writes one leaf. The backing fields are normal values.

## Inherited Properties

The typedata model separates value storage from property state.

Inherited fields should be plain fields in the storage family:

```c
data->TextRun.Font.Family
data->Node2D.Foreground.Color
```

Local/inherited/default/bound/animated state belongs in compact property state,
not in the field type. Effective lookup can be:

```c
if (state says local) return &data->TextRun.Font.Family;
if (parent has same storage family) return inherited parent value;
return default value;
```

For UIKit this means inherited text properties can flow through any UI object,
even if the intermediate object is not itself a `TextBlock`, because the UI
storage guarantees that the relevant field exists.

## Migration Notes

- Keep class identity and storage family identity separate.
- Move fake "concept" components into their owning class or storage family.
- Preserve property metadata for tools, XML, Lua, CSS, and diagnostics.
- Do not make universal storage imply universal behavior.
- Runtime attachable behavior can remain as a smaller extension mechanism if it
  is genuinely dynamic, but fixed class inheritance should not require a linked
  component list.

