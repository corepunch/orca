# XML Triggers, Actions, and Message Types

This guide explains the XML pattern used by Orca screens when you want a node to react to an event, run one or more actions, or dispatch a message payload.

It is the XML-facing version of the engine's message-driven object/component model:

- `EventTrigger` listens for a routed event.
- Actions run when the trigger fires.
- Message types define payload fields for a message.
- Every registered message type also has a generated action element, such as `<Screen.ShowModal/>`.

For a broader discussion of the component system, see [Object + Component System](../orca-c/rules/object-component-system.md).

## The Three Pieces

### 1. Trigger

An `EventTrigger` listens for a routed event such as `Node.LeftButtonUp` or `Node.RightButtonUp`.

### 2. Action

An action runs when the trigger fires. Common actions include:

- `HideAction`
- `ShowAction`
- generated message actions such as `Screen.ShowModal`, `Popup.ClosePopup`, and `Node.RightButtonUp`

### 3. Message Type

A message type describes the payload fields attached to a message. In generated code these appear as a struct such as `Screen_ShowModalEventArgs`.

That is why this shorthand works:

```xml
LeftButtonUp="{Screen.ShowModal Path=Example/Screens/GetStartedPopup}"
```

`Path=` maps to the payload field `Screen_ShowModalEventArgs.Path`.

## EventTrigger Example

Use an `EventTrigger` when you want a node to respond to an input event:

```xml
<TextBlock Name="Close"
           Text="Close"
           FontSize="14"
           BackgroundColor="$accent"
           Padding="8"
           TextHorizontalAlignment="Center"
           TextVerticalAlignment="Center">
  <Node.Triggers>
    <EventTrigger RoutedEvent="Node.LeftButtonUp">
      <HideAction Path="../Popup"/>
    </EventTrigger>
  </Node.Triggers>
</TextBlock>
```

When the text block is clicked, the trigger fires and the action hides the target object.

## Generated Message Action Example

Use a generated message action when you want the XML to be explicit about the message name:

```xml
<TextBlock Name="Source"
           Text="Open popup"
           FontSize="16"
           BackgroundColor="#4444AA"
           Padding="16">
  <Node.Triggers>
    <EventTrigger RoutedEvent="Node.LeftButtonUp">
      <Screen.ShowModal Path="Example/Screens/GetStartedPopup"/>
    </EventTrigger>
  </Node.Triggers>
</TextBlock>
```

The same action can dispatch other messages too:

```xml
<Node.RightButtonUp Target="../Receiver"/>
```

## Inline Message Shorthand

For most XML use, the shorthand is shorter and easier to read:

```xml
<TextBlock Name="OpenPopup"
           Text="Open"
           Width="140"
           Height="44"
           BackgroundColor="#4444AA"
           Padding="16"
           LeftButtonUp="{Screen.ShowModal Path=Example/Screens/GetStartedPopup}"/>
```

This shorthand lowers directly to the generated message action element.

### General Form

The general syntax is:

```xml
{Message.Type Field=Value, OtherField=Value}
```

Examples:

```xml
{Popup.ClosePopup}
{Node.RightButtonUp}
{Screen.ShowModal Path=Example/Screens/GetStartedPopup}
```

### Payload Rules

- Named arguments map directly to message payload fields.
- Short names are accepted when they match the payload struct field names.
- Commas separate arguments, just like WPF-style markup extensions.
- Nested `{...}` expressions are allowed in payload values.

Example with multiple arguments:

```xml
<Example.CustomMessage Name="Header"
                       Value="{Binding ../../Node.ActualWidth}"/>
```

## Recommended Patterns

- Use `EventTrigger` for user input and routed events.
- Use generated message actions when you want an explicit XML element.
- Use inline shorthand when you want a concise message dispatch expression.
- Use `Screen.ShowModal` for modal popup loading.
- Use `Popup.ClosePopup` to close a popup and return a result.

## Example App Pattern

The example application uses the shorthand form directly in XML:

```xml
<TextBlock Name="CtaButtonPrimary"
           Text="Get Started"
           LeftButtonUp="{Screen.ShowModal Path=Example/Screens/GetStartedPopup}"/>
```

The popup itself closes with:

```xml
<TextBlock Name="GetStartedPopupClose"
           Text="Close"
           LeftButtonUp="{Popup.ClosePopup}"/>
```

## Where The Message Types Come From

Registered message types are defined in module `.cgen` files and generated into the runtime message registry.

For example:

- `Screen.ShowModal` defines `Screen_ShowModalEventArgs.Path`
- `Popup.ClosePopup` defines `Popup_ClosePopupEventArgs.ReturnValue`

That is why XML can use the short field names directly when the message type is known.
