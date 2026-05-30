# Orca Language

This is a living design note for the idea that Orca should become its own object-based language and use C as a generated runtime target.

The prompt for this document is the recurring thought:

> But if Orca was a language...

When that thought appears while working on XML, C, MoonScript, properties, messages, or editor metadata, capture it here. The purpose is not to design the full compiler in one pass. The purpose is to keep the direction visible while the current engine continues to evolve.

---

## Thesis

Orca should not feel like a C framework with XML files and scripting attached. Orca should feel like a small language for describing objects, properties, messages, hierarchy, resources, and behavior.

That language can then generate the C code and metadata the runtime needs.

In the current engine, a component or feature is often spread across several representations:

- XML or `.cgen` declares shape, properties, messages, and public API.
- C implements storage, handlers, registration, defaults, and message dispatch.
- MoonScript or Lua provides application behavior and glue.
- Generated headers provide identifiers, property metadata, exports, and binding code.

The language direction is to merge those descriptions into `.orca` files and make them the source of truth.

```text
.orca source
  -> generated C structs and handlers
  -> generated property and message identifiers
  -> generated reflection/editor metadata
  -> generated serialization/import/export glue
  -> generated Lua or compatibility bindings where needed
```

C remains important, but as the runtime substrate and generated form, not the main authoring surface.

---

## Object Model

The language should be object-based.

An object is the common unit for meaningful things in Orca:

- components
- nodes
- images
- shaders
- fonts
- meshes
- scripts
- messages
- properties
- editor-visible definitions

This does not mean every object has the same runtime representation. It means the language has one conceptual model for identity, properties, lifecycle, behavior, and tooling.

At the language level, an object has:

- identity
- type or prototype
- properties
- child or owned objects
- message handlers
- lifecycle
- optional generated storage

At the generated C level, the compiler is free to choose efficient representations: structs, tables, constants, offsets, static metadata, compact arrays, or plain fields.

---

## Properties As Objects

Properties should probably be objects too.

That makes properties part of the same reflected model as components and messages. A property object can describe:

- name
- type
- default value
- constraints
- editor widget
- serialization rules
- whether it affects layout or render
- change notification behavior
- generated storage

For example:

```orca
property Width: Float {
  default: 100
  min: 0
  affects: layout
  editor: number
}
```

This should not force every property value to become a heavyweight heap object at runtime. A useful split is:

- the property definition is an object in the Orca model
- the property value may be a compact generated C field
- reflection/editor systems can access generated metadata when needed

The language can be pure without making the runtime wasteful.

---

## Messages As Functions

Function calls in Orca should be message sends.

Instead of treating methods and messages as separate ideas, behavior can be modeled as receiving and handling typed messages.

```orca
message Layout {
  bounds: Rect
}

object Button : Node {
  on Layout(bounds) {
    parent.Layout(bounds)
    Label.Layout(bounds)
  }
}
```

Calling:

```orca
button.Layout(bounds)
```

means:

```text
send Layout(bounds) to button
```

Message handlers replace the usual method table as the primary behavior surface.

---

## Parent Sends

Messages need a clear parent-send story.

This is the equivalent of calling `super`, but in the language of messages:

```orca
on Draw(ctx) {
  parent.Draw(ctx)
  Icon.Draw(ctx)
}
```

One design question to keep explicit:

- prototype or type parent: where inherited behavior comes from
- tree or owner parent: where containment, routing, layout, and bubbling happen

Both are valuable. The language should avoid using one word for both without a rule.

Possible vocabulary:

```orca
parent.Draw(ctx)  // behavior parent
bubble Click(event)  // containment parent
```

The exact syntax can change, but the distinction should remain visible.

---

## Components In Separate Files

Components should live in separate `.orca` files and describe the whole component in one place.

```text
components/
  Transform.orca
  Image.orca
  Shader.orca
  Button.orca

messages/
  Draw.orca
  Click.orca
  Layout.orca
```

The current `.cgen` and XML role can collapse into these files over time. A component file should be able to declare:

- properties
- default values
- parent components or prototypes
- handled messages
- messages it sends
- child objects
- editor metadata
- serialization behavior
- generated C hooks

The compiler then emits the C and metadata that are currently hand-written or generated from multiple sources.

---

## Replacing XML, C, And MoonScript Duplication

The long-term direction is:

```text
.orca replaces:
  XML structure
  C component/property boilerplate
  MoonScript behavior glue
  registration tables
  editor metadata
```

XML may remain useful as a save/interchange format. Lua or MoonScript may remain useful as a compatibility layer or advanced scripting path. But Orca object definitions should not require all three layers to say the same thing.

The hard rule to test this direction against:

> If an object, property, message, component, or editor-visible concept exists, it should be declared in `.orca` first.

Everything else should either be generated from that declaration or be runtime data saved from it.

---

## Sketch

This is not syntax yet. It is only a pressure test for the idea.

```orca
component Button : Node {
  property Text: String {
    default: "Button"
    editor: text
  }

  property Pressed: Bool {
    default: false
    editor: checkbox
  }

  child Label: Text {
    Text: self.Text
  }

  on Layout(bounds: Rect) {
    parent.Layout(bounds)
    Label.Layout(bounds)
  }

  on Click(event: PointerEvent) {
    Pressed = true
    send PressedChanged(self)
  }

  on Draw(ctx: DrawContext) {
    parent.Draw(ctx)
    Label.Draw(ctx)
  }
}
```

From this, the compiler could generate:

- `struct Button`
- property metadata and stable property identifiers
- default value initialization
- message identifiers and handler dispatch
- editor schema
- serialization schema
- compatibility bindings

---

## Open Questions

- Is Orca class-based, prototype-based, or a hybrid?
- Are component inheritance and scene hierarchy separate relationships?
- Are messages globally declared objects, local object capabilities, or both?
- How much behavior should compile into C versus remain interpreted or hot-reloadable?
- What is the minimal `.orca` subset that can replace one current `.cgen` component?
- How should generated identifiers stay stable across renames?
- Can the editor edit `.orca` source directly, or does it edit a structured model that prints `.orca`?
- What compatibility path lets existing XML, C, and Lua/MoonScript continue working during migration?

---

## Future Notes

Append examples here when a feature feels awkward because Orca is not yet a language.

Use this format:

```text
### YYYY-MM-DD - Short title

Current friction:

If Orca was a language:

Possible generated/runtime form:
```
