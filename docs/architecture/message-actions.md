# Message Actions

Orca represents every generated message as both:

- a typed C payload struct for handlers
- a generated `Action` subclass for XML and Lua construction

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

Generated message actions handle `Action.Dispatch` by resolving their
dispatch target and sending the original typed message:

```text
Screen.ShowModal action
  -> resolve Target relative to Sender, or use Sender when unset
  -> send/post ID_Screen_ShowModal with Screen_ShowModalEventArgs payload
```

Default `Mode` is synchronous `Send`. `Mode="Post"` posts through the queue.

## Runtime Metadata

There is no separate message registry. Message metadata lives in the generated
message-action `ClassDesc`:

- `ClassName` is the dotted message name, such as `Screen.ShowModal`
- `ClassID` is the message hash
- payload fields are the first properties
- action bookkeeping fields are generated after payload fields

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
be passed as the corresponding `EventArgs` payload.

## Removed Baggage

This model removes:

- the legacy generic message-dispatch adapter
- adapter-specific fake message-field properties
- XML parser ordering special cases for `Message=...`
- inline lowering to a generic adapter
- the runtime `core.message_types` registry
- Lua's hand-built message payload buffer path

Generated message actions are now the single metadata and dispatch surface.
