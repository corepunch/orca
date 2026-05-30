# Message Actions

This note describes a proposed cleanup for Orca's trigger/action/message path.
It is intentionally a design document first; implementation should follow only
after the shape is agreed on.

## Problem

Today, XML-triggered message dispatch goes through `SendMessageAction`:

```xml
<EventTrigger RoutedEvent="Node.LeftButtonUp">
  <SendMessageAction Message="Screen.ShowModal"
                     Screen_ShowModalEventArgs.Path="Example/Screens/GetStartedPopup"/>
</EventTrigger>
```

The inline shorthand:

```xml
LeftButtonUp="{Screen.ShowModal Path=Example/Screens/GetStartedPopup}"
```

currently lowers to that same generic action. This works, but it creates a
hybrid model:

- messages are generated C payload structs
- Lua sees messages as event properties
- XML sees message payload fields as fake properties on `SendMessageAction`
- the XML parser has special handling for `SendMessageAction`
- the object property system has special lookup logic for message fields

The goal is to make XML message dispatch look like normal object creation and
normal action dispatch.

## Proposed Model

Add a base `Action` class.

`Action` defines the dispatch contract:

```xml
<class name="Action">
  <messages>
    <message name="Dispatch">
      <field name="Sender" type="Object" pointer="true"/>
    </message>
  </messages>
</class>
```

Important: `Action` does **not** need to handle `Action.Dispatch` itself. It is
an abstract base/contract. Concrete action classes handle `Action.Dispatch`.

Every generated message type also gets a generated message-action class. For a
message declaration such as:

```xml
<class name="Screen">
  <messages>
    <message name="ShowModal">
      <field name="Path" type="string"/>
    </message>
  </messages>
</class>
```

codegen would synthesize an action class with an XML-facing name equivalent to:

```xml
<Screen.ShowModal Path="Example/Screens/GetStartedPopup"/>
```

Internally, the generated C symbol can use a sanitized name:

```c
struct Screen_ShowModalAction {
  char *Target;
  const char *Path;
};
```

The generated class automatically derives from `Action` and handles
`Action.Dispatch`.

## Dispatch Flow

The trigger owns trigger semantics. Actions own action execution.

```text
EventTrigger handles Trigger.Triggered
  -> for each child Action
  -> send Action.Dispatch { Sender = original sender }

Screen.ShowModal handles Action.Dispatch
  -> resolve Target, or use Sender
  -> materialize Screen_ShowModalEventArgs
  -> send/post ID_Screen_ShowModal to the target
```

This means message-action classes do not handle `Trigger.Triggered` directly.
That message belongs to triggers. `Action.Dispatch` is the bridge between a
trigger firing and an action executing.

## Generated Message-Action Classes

Each `<message>` declaration should imply a generated action class. Authors
should not need to write `parent="Action"` on every message. Requiring that
would create another easy way to make inert half-wired messages.

Generated message-action classes should have:

- automatic parent: `Action`
- automatic handler: `Action.Dispatch`
- payload properties copied from the message fields
- optional action metadata such as `Target`
- a generated dispatch function that builds or aliases the payload struct

For example:

```xml
<Screen.ShowModal Target="../Root" Path="Example/Screens/GetStartedPopup"/>
<Popup.ClosePopup ReturnValue="1"/>
<Node.RightButtonUp Target="../Receiver"/>
```

## Naming

Use the message name as the XML-facing class name:

```xml
<Screen.ShowModal .../>
```

Use sanitized C names for generated symbols:

```c
struct Screen_ShowModalAction;
LRESULT Screen_ShowModalAction_Dispatch(...);
```

The docs can call these **message actions**: generated `Action` subclasses that
dispatch a concrete message.

## Payload Representation

Keep the existing C message payload structs.

They are still the best shape for C handlers:

```c
HANDLER(Screen, Screen, ShowModal) {
  if (!pShowModal || !pShowModal->Path) return FALSE;
}
```

The generated action object can either:

- duplicate the payload layout and copy/alias it when dispatching, or
- store normal component properties and materialize the payload struct before
  sending.

If the generated action component memory layout can match the corresponding
`EventArgs` struct, dispatch can be cheap:

```c
axPostMessageDataW(target,
                   ID_Screen_ShowModal,
                   0,
                   payload_component,
                   sizeof(struct Screen_ShowModalEventArgs));
```

The important point is that C handlers keep receiving typed payload structs,
not generic objects.

## What This Removes

This should allow removal of:

- `SendMessageAction` as a public generic adapter
- fake message-field properties on `SendMessageAction`
- parser ordering special cases for `Message=...`
- inline lowering from `{Screen.ShowModal ...}` to `<SendMessageAction ...>`
- parts of the runtime message-field lookup path used only to support
  `SendMessageAction`

The raw line-count reduction may be modest because codegen grows, but the
hand-written runtime becomes simpler and less magical.

## Migration Plan

1. Add `Action` with `Action.Dispatch`.
2. Update `EventTrigger` so it sends `Action.Dispatch` to child actions when
   it receives `Trigger.Triggered`.
3. Teach codegen to generate message-action classes for each `<message>`.
4. Teach XML class lookup to resolve dotted generated class names such as
   `Screen.ShowModal`.
5. Update inline shorthand to emit the generated message-action element
   directly.
6. Keep `SendMessageAction` as a compatibility shim for one release window.
7. Convert samples/tests to generated message-action syntax.
8. Remove `SendMessageAction` and its parser/property special cases.

## Open Questions

- Should message actions dispatch synchronously or post asynchronously by
  default?
- Should `Target` be a string path, an object property, or both?
- Should no-payload messages generate zero-field action classes?
- Should message actions expose a `Mode` property, such as `Post` vs `Send`?
- Should generated message actions be visible in Lua constructors, or only XML?

## Preferred Direction

Use `Action` as the base concept, `Action.Dispatch` as the execution message,
and generated message-action classes as the XML-facing replacement for
`SendMessageAction`.

This keeps the fast typed C message path while making XML authoring and parser
behavior feel like the rest of the object system.
