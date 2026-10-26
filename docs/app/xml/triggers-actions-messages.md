# Triggers, Actions, and Message Types

Orca's trigger/action system lets you wire UI behaviour — show/hide elements, dispatch messages, navigate screens — entirely in XML, without any Lua controller code. The core idea mirrors WPF's trigger model and NextStep's responder chain: **triggers** watch for conditions, and when met they fire **actions** against targets.

---

## Concepts

| Term | Role |
|---|---|
| **Trigger** | Watches for a condition (an event, a property change, object attachment) and fires when it is met |
| **Action** | Executes when a trigger fires — hides a node, sends a message, sets a property |
| **`Node.Triggers`** | An array property on every node that holds that node's trigger list |
| **Routed event** | A typed UI event (mouse, keyboard, custom) that bubbles up the object tree |

Triggers are children of `Node.Triggers`; actions are children of their trigger.

---

## Form bindings

Define the draft's data layout in a schema, independently of its controls:

```xml
<Schema>
  <Entity Name="ProfileDraft">
    <Column Name="DisplayName" Type="string"/>
    <Column Name="Email" Type="string"/>
    <Column Name="OptIn" Type="bool"/>
  </Entity>
</Schema>
```

Place a draft record in the form's `DataContext`, directly or through a managed
XML datasource. Descendants inherit the nearest context:

```xml
<StackView>
  <Input Text="{Binding DataContext/DisplayName, Mode=TwoWay}"/>
  <Input Type="Email" Text="{Binding Path=DataContext/Email, Mode=TwoWay}"/>
  <Input Type="Checkbox" Checked="{Binding DataContext/OptIn, Mode=TwoWay}"/>
  <TextBlock Text="{Binding DataContext/DisplayName}"/>
</StackView>
```

The equivalent explicit form is:

```xml
<Input>
  <Binding Target="TextRun.Text" Mode="TwoWay" Expression="DataContext/Email"/>
</Input>
```

| Mode | Behavior |
|---|---|
| `OneWay` | Default for XML binding markup. Reads the model; control edits do not write back. |
| `TwoWay` | Initializes the control from the model, then writes actual control edits back. |
| `OneWayToSource` | Writes control changes without reading the model or transferring the control's initial default. |
| `Expression` | Evaluates a forward-only program, including calculations and concatenation. |

Writable modes require one property path and matching source/target types.
They reject computed reverse expressions, read-only properties and arrays.
Use string draft fields for text entry; perform numeric parsing and domain
validation in the validation/save layer. Password controls bind the underlying
value while masking its display. Native text, clear and checkbox operations
all use the property system, so controllers do not copy values from controls.

Replacing `DataContext`, overriding it on a subtree, or reparenting a form
re-resolves the path. Synchronization waits if its context is unavailable;
edits made without a context are not queued into a later model. A missing field
in the nearest context cannot write into a more distant context. Forward
initialization and refresh are guarded against feedback into the source.

`Enabled="false"` disables an explicit binding. XML serialization preserves
mode and enabled state. See [the complete sample](../../../apps/MvvmForms/README.md)
for separate schema, initial data, view, controller and committed snapshot.

## Trigger types

### `EventTrigger` — react to a routed event

Fires when the named event reaches the node.

```xml
<TextBlock Name="OpenButton" Text="Open Settings">
  <Node.Triggers>
    <EventTrigger RoutedEvent="Node.LeftButtonUp">
      <Screen.ShowModal Path="MyApp/Screens/SettingsPopup"/>
    </EventTrigger>
  </Node.Triggers>
</TextBlock>
```

`RoutedEvent` is a dotted name: `Category.EventName`. Common values:

| Value | When |
|---|---|
| `Node.LeftButtonUp` | Left mouse / touch released |
| `Node.RightButtonUp` | Right mouse released |
| `Node.LeftButtonDown` | Left mouse / touch pressed |
| `Node.KeyDown` | Key pressed while node has focus |
| `Node.ScrollWheel` | Scroll wheel |
| `Input.Submit` | User submits an `<Input>` (Enter key or submit button) |

### `OnPropertyChangedTrigger` — react to a property value change

Fires whenever the named property changes. Optionally watches a different node via `SourceNode`.

```xml
<Node2D Name="Panel">
  <Node.Triggers>
    <OnPropertyChangedTrigger Property="Visible">
      <HideAction Path="../Overlay"/>
    </OnPropertyChangedTrigger>
  </Node.Triggers>
</Node2D>
```

With `SourceNode` to watch a sibling:

```xml
<OnPropertyChangedTrigger SourceNode="../StatusLabel" Property="Text">
  <HideAction Path="../LoadingSpinner"/>
</OnPropertyChangedTrigger>
```

### `OnAttachedTrigger` — run once when attached to the tree

Fires immediately when the object is added to the live scene. Useful for one-shot initialisation that can't be done with static property values.

```xml
<Node2D Name="Banner">
  <Node.Triggers>
    <OnAttachedTrigger>
      <HideAction Path="."/>
    </OnAttachedTrigger>
  </Node.Triggers>
</Node2D>
```

---

## Action types

### `HideAction` — hide a node

Sets `Visible="false"` on the node at `Path`. The path is resolved relative to the dispatch sender (the node that owns the trigger).

```xml
<EventTrigger RoutedEvent="Node.LeftButtonUp">
  <HideAction Path="../Overlay"/>
</EventTrigger>
```

`Path` supports relative segments: `..` goes to the parent, names descend into children. `../Sibling/Child` is a valid path.

### `Setter` — set an arbitrary property

Sets any property on the sender to a literal value.

```xml
<EventTrigger RoutedEvent="Node.LeftButtonUp">
  <Setter Property="Visible" Value="false"/>
</EventTrigger>
```

### Generated message actions — `Category.MessageName`

Any registered message can be used as an action directly by its dotted name. The codegen produces a typed action class for every message. When used as an action, its properties become the message payload.

**Show a modal screen:**

```xml
<EventTrigger RoutedEvent="Node.LeftButtonUp">
  <Screen.ShowModal Path="MyApp/Screens/SettingsPopup"/>
</EventTrigger>
```

**Forward an event to another node** (`Target` resolves the receiver):

```xml
<!-- Source fires LeftButtonUp → sends RightButtonUp to Receiver -->
<EventTrigger RoutedEvent="Node.LeftButtonUp">
  <Node.RightButtonUp Target="../Receiver"/>
</EventTrigger>
```

`Target` is optional. When omitted the message bubbles from the sender through its ancestors, just like a real event — useful for action chains that any ancestor can handle without hard-coding a path.

---

## Inline shorthand — event attribute syntax

For single-action triggers on mouse and other event properties, you can skip the `<Node.Triggers>` block entirely. Assign an XML fragment as the attribute value:

```xml
<!-- Longhand -->
<TextBlock Name="CloseButton" Text="Close">
  <Node.Triggers>
    <EventTrigger RoutedEvent="Node.LeftButtonUp">
      <HideAction Path="../Overlay"/>
    </EventTrigger>
  </Node.Triggers>
</TextBlock>

<!-- Inline shorthand — identical behaviour -->
<TextBlock Name="CloseButton" Text="Close"
    LeftButtonUp="{HideAction Path=../Overlay}"/>
```

The parser wraps the fragment in an `EventTrigger` whose `RoutedEvent` is derived from the attribute name (`LeftButtonUp` → `Node.LeftButtonUp`). The `{}` delimiters mark the value as an object fragment rather than a plain string.

The shorthand works for any event-typed property, including custom ones:

```xml
<Input Name="CommandInput" PlaceholderText="Enter command..."
    Submit="{Screen.ShowModal Path=MyApp/Screens/Help}"/>
```

---

## How dispatch works

When a trigger fires it sends `Action.Dispatch` to each of its children in order. `SendMessageAction` (the base of all generated message actions) handles `Action.Dispatch` as follows:

1. **Explicit `Target`** — resolves the path relative to the sender and calls `OBJ_SendMessageW` on it directly.
2. **No `Target`** — walks the sender's ancestor chain until a handler claims the message, then stops. This is the responder-chain behaviour.
3. **`Mode="Post"`** — queues the message for deferred dispatch instead of sending synchronously. Useful when the action would mutate the tree mid-event.

---

## Full example — modal popup with close button

```xml
<!-- MainScreen.xml -->
<Screen Name="MainScreen" Width="800" Height="600">
  <StackView class="toolbar">
    <TextBlock Name="SettingsButton" Text="Settings"
        LeftButtonUp="{Screen.ShowModal Path=MyApp/Screens/SettingsPopup}"/>
  </StackView>
</Screen>
```

```xml
<!-- SettingsPopup.xml -->
<Popup Name="SettingsPopup">
  <Grid Rows="auto 1fr auto">
    <TextBlock Text="Settings" class="title"/>
    <!-- content -->
    <TextBlock Name="SettingsPopupClose" Text="Done"
        LeftButtonUp="{Popup.ClosePopup}"/>
  </Grid>
</Popup>
```

The close button sends `Popup.ClosePopup` up the responder chain — no `Target` needed because `Popup` handles it on itself. No Lua required.

---

## Chaining triggers

Multiple triggers on the same node fire independently. Multiple actions inside one trigger fire in document order.

```xml
<TextBlock Name="DestructiveButton" Text="Delete">
  <Node.Triggers>
    <EventTrigger RoutedEvent="Node.LeftButtonUp">
      <HideAction Path="../ConfirmPanel"/>
      <HideAction Path="../ItemList"/>
      <Screen.ShowModal Path="MyApp/Screens/DeleteConfirm"/>
    </EventTrigger>
    <EventTrigger RoutedEvent="Node.RightButtonUp">
      <Screen.ShowModal Path="MyApp/Screens/ContextMenu"/>
    </EventTrigger>
  </Node.Triggers>
</TextBlock>
```

---

## When to use a Lua controller instead

Triggers and actions cover stateless, declarative reactions. Reach for a Lua controller when:

- The handler needs to read or compute values (e.g. validate input before submitting)
- The action depends on runtime state not available as a property binding
- You need to mutate a DataSource (until `DataSource.Add` / `DataSource.Remove` actions land)

See [Controller authoring](../../lua-scene-loading.md) for the controller pattern.
