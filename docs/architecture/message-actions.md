# Message Actions

Orca represents every generated message as both:

- a typed C payload struct for handlers
- a generated `SendMessageAction` subclass for XML and Lua construction

That keeps C handlers fast and typed while making message dispatch look like
ordinary object/action dispatch in authoring surfaces.

## Model

`Action` is the base executable object. It defines the dispatch contract:

```xml
<class name="Action">
  <messages>
    <message name="Dispatch">
      <field name="Sender" type="Object" pointer="true"/>
    </message>
  </messages>
</class>
```

`Action` itself is only the contract. Concrete actions handle
`Action.Dispatch`.

`SendMessageAction` is the generic base for generated message actions. It owns
the common dispatch properties and the single shared dispatch handler:

```xml
<class name="SendMessageAction" parent="Action">
  <property name="Target" type="string"/>
  <property name="Mode" type="DispatchMode" default="Send"/>
  <handle message="Action.Dispatch"/>
</class>
```

Every `.cgen` `<message>` automatically gets a generated action class:

```xml
<Screen.ShowModal Path="Example/Screens/GetStartedPopup"/>
<Popup.ClosePopup ReturnValue="1"/>
<Node.RightButtonUp Target="../Receiver"/>
```

The XML-facing class name is the message name. The generated C symbol is
sanitized, for example `Screen_ShowModalAction`.

## Dispatch Flow

Triggers still own trigger semantics:

```text
EventTrigger receives Trigger.Triggered
  -> base Trigger sends Action.Dispatch { Sender = original sender }
  -> each child action executes itself
```

Generated message actions inherit the `SendMessageAction` handler. That handler
finds the concrete generated payload component on the action object, uses that
component's class ID as the message ID, and sends the component data as the
typed payload:

```text
Screen.ShowModal action
  -> SendMessageAction calls OBJ_FindByPath(Sender, Target)
     Target is a slash path: "Body/NavBar", "../Sibling", "./Self"
     If Target is empty, Sender is used directly.
  -> finds the Screen_ShowModalAction payload component
  -> send/post ID_Screen_ShowModal with Screen_ShowModalEventArgs payload
```

Default `Mode` is synchronous `Send` (calls `OBJ_SendMessageW` immediately).
`Mode="Post"` defers through the message queue (`axPostMessageDataW`).

## Runtime Metadata

There is no separate message registry. Message metadata lives in the generated
message-action `ClassDesc`:

- `ClassName` is the dotted message name, such as `Screen.ShowModal`
- `ClassID` is the message hash
- direct properties are only payload fields
- `Target` and `Mode` are inherited from `SendMessageAction`

Lua `object:send("Message.Name", ...)` creates the generated message-action
object, fills its payload fields, and dispatches `Action.Dispatch`. It no
longer builds an ad-hoc message payload buffer or consults a separate
`core.message_types` table.

## C Handlers

C handlers continue to receive typed payload structs:

```c
HANDLER(Screen, Screen, ShowModal) {
  if (!pShowModal || !pShowModal->Path) return FALSE;
}
```

The generated action struct keeps the payload fields first so the component can
be passed as the corresponding `EventArgs` payload. The generic
`SendMessageAction` handler passes the generated component data directly to
`OBJ_SendMessageW` or `axPostMessageDataW`.

## Removed Baggage

This model removes:

- the legacy XML-facing generic message-dispatch adapter
- adapter-specific fake message-field properties
- XML parser ordering special cases for `Message=...`
- inline lowering to a generic adapter
- per-message generated `Action.Dispatch` handlers
- the runtime `core.message_types` registry
- Lua's hand-built message payload buffer path

Generated message actions are now data-only message payload components that
inherit one generic dispatch implementation.
