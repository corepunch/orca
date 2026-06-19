---
name: orca-xml
description: >
  XML authoring reference for ORCA. Covers triggers, actions, messages,
  styling, and XML component declarations. Use when creating XML screens,
  defining triggers/actions, or working with the style system.
---

# ORCA XML Authoring

A framework for building UI applications with XML declarations.
Screens are defined in XML with triggers, actions, and style classes.

## Getting Started

### Create a new project

```sh
orca new MyProject
```

Creates:
```
MyProject/
├── package.xml        # Project config with startup screen
├── Screens/
│   └── Main.xml       # Hello World screen
└── Images/
```

### Run it

```sh
make run PROJECT=MyProject/package.xml
```

### Project structure

- `package.xml` — entry point, declares libraries and system messages
- `Screens/` — XML screen definitions
- `Images/` — image assets

## Principles

1. **`generated/` is your API reference.** For class definitions, properties, and messages — read the markdown in `generated/`, never `.cgen`.
2. **`.cgen` is the build source of truth.** It defines the API shape for codegen, but agents should not parse it directly.
3. **XML declares structure.** UI hierarchy, properties, and triggers are in XML.
4. **Triggers listen for events.** `EventTrigger` reacts to routed events like `Node.LeftButtonUp`.
5. **Actions run on trigger.** `HideAction`, `ShowAction`, or generated message actions.
6. **Styles are CSS-like.** Classes, selectors, pseudo-states, and properties.
7. **Messages bridge XML and C.** Generated actions dispatch typed message payloads.

## Agent Workflow

**API reference:** Use `generated/` markdown files for fast lookups. These contain class definitions, properties, and messages — all extracted from `.cgen`.

**When to read source:** Only read `*.c` or `*.xml` files when actively implementing or modifying code. Never read source for reference.

**Documentation rule:** If you must read a source file to understand behavior, update the relevant `generated/` or `rules/` markdown with your findings. Future agents should not need to repeat this work.

**Quick lookup priority:**
1. `generated/{module}/classes/` — class APIs, properties, messages
2. `rules/` — patterns, conventions, workflows
3. `.cgen` — only when adding/modifying components
4. `source/` — only when implementing, never for reference

## Critical Rules

### Triggers & Actions → [rules/triggers-actions-messages.md](./rules/triggers-actions-messages.md)

- **Three pieces:** EventTrigger (listener) → Action (handler) → MessageType (payload).
- **Inline shorthand:** `LeftButtonUp="{Screen.ShowModal Path=...}"`.
- **Generated actions:** Each registered message type gets an XML action element.
- **HideAction/ShowAction:** Built-in actions for visibility toggling.

### Styling → [rules/styling.md](./rules/styling.md)

- **StyleController:** Component attached to every `Node2D` automatically.
- **Class syntax:** `ClassName[:pseudo-state...][/opacity]` (e.g., `button:hover/80`).
- **Selectors:** `.class`, `#id`, `Element`, `Parent > Child`, `A + B`.
- **Pseudo-states:** `:hover`, `:focus`, `:active`, `:dark`.
- **Properties:** `color`, `background-color`, `width`, `height`, `padding`, etc.
- **Theme variables:** `$accent`, `$text-primary`, `$background`, etc.

## Key Patterns

### Basic trigger with action

```xml
<Button Name="Close" Text="Close">
  <Node.Triggers>
    <EventTrigger RoutedEvent="Node.LeftButtonUp">
      <HideAction Path="../Popup"/>
    </EventTrigger>
  </Node.Triggers>
</Button>
```

### Generated message action

```xml
<Button Name="Open" Text="Open popup">
  <Node.Triggers>
    <EventTrigger RoutedEvent="Node.LeftButtonUp">
      <Screen.ShowModal Path="Screens/MyPopup"/>
    </EventTrigger>
  </Node.Triggers>
</Button>
```

### Inline shorthand

```xml
<!-- Shorthand for EventTrigger + Action -->
<Button Text="Close" LeftButtonUp="{HideAction Path=../Popup}"/>
<Button Text="Open" LeftButtonUp="{Screen.ShowModal Path=Screens/MyPopup}"/>
```

### Style classes

```xml
<!-- Apply classes at construction -->
<Button class="primary large" Text="Submit"/>
<Button StyleClass="primary:hover" Text="Hover me"/>

<!-- Inline styles (avoid when possible) -->
<Button BackgroundColor="#4444AA" Text="Styled"/>
```

### Custom message type

```xml
<!-- Define a message type with payload fields -->
<MessageType Name="MyAction">
  <Field Name="Url" Type="string"/>
  <Field Name="Count" Type="int"/>
</MessageType>

<!-- Use the generated action -->
<Button LeftButtonUp="{MyAction Url='example.com' Count=42}"/>
```

## Component Selection

| Need | Use |
|------|-----|
| Button | `<Button>` |
| Text | `<TextBlock>` |
| Input | `<TextField>` |
| Image | `<Image>` |
| Container | `<Node2D>`, `<StackPanel>`, `<Grid>` |
| Popup | `<Popup>` via `Screen.ShowModal` |
| Style | CSS-like selectors in `<Style>` or `StyleClass` attribute |

## Quick Reference

```xml
<!-- EventTrigger -->
<Node.Triggers>
  <EventTrigger RoutedEvent="Node.LeftButtonUp">
    <ActionName Param="value"/>
  </EventTrigger>
</Node.Triggers>

<!-- Inline shorthand -->
<Node LeftButtonUp="{ActionName Param=value}"/>

<!-- Style classes -->
<Node class="class-name:pseudo/opacity"/>

<!-- Theme variables -->
<Node BackgroundColor="$accent"/>

<!-- Property inheritance -->
<Node Width="100" Height="50">
  <Node2D Width="200"/>  <!-- Inherits Height=50 -->
</Node>
```

## Detailed References

- [rules/triggers-actions-messages.md](./rules/triggers-actions-messages.md) — Triggers, actions, message types
- [rules/styling.md](./rules/styling.md) — Style system, classes, selectors, properties
- [rules/xml-helpers.md](./rules/xml-helpers.md) — C-side XML helpers and StyleClass API
