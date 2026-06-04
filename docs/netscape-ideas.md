# Netscape Style System Ideas For ORCA Properties

This note records useful ideas from the Netscape source tree and maps them to
ORCA's property, style, shorthand, and binding systems.

The important lesson is not to copy Netscape's implementation directly. Its
style layer is intentionally loose: declarations are mostly string or number
pairs, selectors write into temporary style structs, and layout interprets many
values late. ORCA should keep its stronger generated property model, but borrow
the separation between authored declarations, shorthand expansion, cascade
priority, contextual resolution, and hot runtime storage.

## 1. Shorthands Fan Out To Atomic Properties

Netscape has grouping helpers such as margins, paddings, and border widths.
Those helpers do not store a grouped margin or padding object as the final
style value. They immediately write the component properties:

```text
margins(1, 2, 3, 4)
  -> marginTop
  -> marginRight
  -> marginBottom
  -> marginLeft
```

The same principle should apply in ORCA.

For text, `Font` should be an authoring shorthand, not a stored property:

```xml
<TextBlock Font="bold italic 22 Inter"/>
```

should lower to atomic property writes:

```text
TextRun.FontWeight
TextRun.FontStyle
TextRun.FontSize
TextRun.FontFamily
```

There should be no real `TextRun.Font` property in the runtime property table,
and generated component structs should not expose nested storage such as:

```c
text.Font.Weight
```

Hot code should read effective atomic properties:

```c
TextRun_GetProperty(object, offsetof(struct TextRun, Font.Weight))
PROP_GetValue(property)
```

The shorthand type is still useful. `FontShorthand` can remain a convenience
structure for parsing, constructing, passing around, and reading a grouped view
of the atomic properties. It is transient data, not property storage.

Codegen should provide helpers in this shape:

```c
struct FontShorthand TextRun_GetFontShorthand(struct TextRun const *text);
void TextRun_ApplyFontShorthand(struct Object *object,
                                struct FontShorthand const *font,
                                uint32_t presentMask);
bool_t TextRun_ParseFontShorthand(lpcString_t source,
                                  struct FontShorthand *font,
                                  uint32_t *presentMask);
```

The `presentMask` is essential. A partial shorthand must only write the fields
that appeared in the authored value:

```xml
<TextBlock Font="bold"/>
```

sets `FontWeight`, but does not clear or overwrite inherited `FontSize`,
`FontStyle`, or `FontFamily`.

The generated getter gives C code pleasant local ergonomics:

```c
struct FontShorthand font = TextRun_GetFontShorthand(pTextRun);
```

while preserving flat property storage underneath.

## 2. Declarations Carry Priority And Specificity

Netscape stores style declarations as name/value pairs with a priority. When a
second declaration targets the same property, the higher priority wins. If
priorities are equal, the later declaration can replace the earlier one.

That is a good fit for ORCA's style problem.

Today ORCA has several property sources:

- Component defaults.
- Inherited parent values.
- Theme and class styles.
- State-specific styles.
- Trigger and animation writes.
- Local XML or Lua values.
- Bindings.

These sources should be modeled explicitly as a cascade of declarations before
they become final effective property values.

A useful shape is:

```c
struct PropertyDeclaration {
  uint32_t propertyID;
  uint32_t sourcePriority;
  uint32_t order;
  uint32_t flags;
  void const *value;
};
```

`sourcePriority` encodes the broad source of the declaration. `order` preserves
stable ordering inside a source, such as stylesheet order. `flags` can represent
state like important, inherited, animated, or specialized.

For example:

```text
default < inherited < type style < class style < id style < state style < local < animation
```

The exact ordering can change, but it should be documented and centralized.
The important bit is that precedence is no longer an accidental side effect of
which subsystem happened to write last.

This also makes shorthands easier: shorthand parsing produces several
`PropertyDeclaration` records with the same priority and order, one per atomic
target property.

## 3. Specified Values Can Become Bindings

Netscape separates authored values from resolved values. A declaration can keep
something like `1.2em`, `50%`, or `12pt`, and layout resolves it later using
context.

ORCA does not need a parallel permanent "specified value" storage model for
every property. Contextual values can be converted into bindings or generated
property programs.

The pipeline should be:

```text
authored value
  -> parse
  -> concrete value or generated binding
  -> binding evaluates to concrete atomic value
  -> property stores local value or resolves an inherited effective value
```

Simple values write directly:

```xml
<TextBlock FontSize="18"/>
```

Contextual values install a binding:

```xml
<TextBlock FontSize="1.2em"/>
```

This can lower to a generated binding that depends on the inherited or current
font context.

Likewise:

```xml
<Node Width="50%"/>
```

can become a binding that depends on the parent layout size.

Theme references fit naturally:

```xml
<TextBlock ForegroundColor="$text-muted"/>
```

can become either a concrete theme lookup at parse time or a theme-dependent
binding that refreshes when the active theme changes. The latter is more
powerful and closer to CSS custom property behavior.

Shorthands can mix direct writes and generated bindings:

```xml
<TextBlock Font="bold 1.2em Inter"/>
```

lowers to:

```text
FontWeight = Bold
FontSize   = generated binding for 1.2em
FontFamily = Inter
```

The binding layer must know how to invalidate itself. Some common dependency
families are:

- `em`, `rem`, and font-relative values: font context changes.
- Percent sizes: parent or containing layout size changes.
- Theme variables: active theme or theme token changes.
- Viewport units: screen or viewport size changes.
- Object references: target object lifetime or relevant target property changes.

This keeps hot component fields concrete and fast while still letting authoring
syntax remain CSS-like.

## 4. Style Matching Produces A Temporary Computed Declaration Set

Netscape uses a style stack. As tags are pushed and popped, it computes the
style that applies to the current top tag. Applicable rules write declarations
into a temporary style struct; priority decides which declarations survive.

ORCA can adapt this idea without becoming stringly typed.

For a given object, style resolution should produce a temporary computed
declaration set:

```text
object
  -> collect matching style rules
  -> expand shorthands
  -> sort or merge by priority/order
  -> produce winning declarations
  -> apply changed atomic properties
```

This computed declaration set should be typed as early as practical. ORCA has
generated `PropertyType` metadata, so the style system can parse property values
into the same binary representation used by XML and Lua.

The final effective properties should not need to know whether a value came from:

- XML attribute.
- Lua assignment.
- CSS-like style rule.
- Shorthand expansion.
- State style.
- Generated contextual binding.

They only need to know the effective atomic value and whether it is local,
inherited, bound, animated, or otherwise controlled by a higher-priority source.

This also suggests a cleaner lifecycle for style updates:

1. Mark style context dirty when class, id, state, theme, or parent changes.
2. Recompute declarations for the affected object.
3. Compare against the previous winning declarations.
4. Apply only changed atomic properties.
5. Resolve inherited atomic properties through the property accessor path.

That fits the pull-inheritance design: inheritance remains attached to atomic
properties only, and `PROP_GetValue` resolves the effective value. Shorthands
disappear before inheritance runs.

## 5. Normalize Names Across CSS, XML, Lua, And C

Netscape includes a CSS-to-JavaScript name conversion layer. It handles names
like `font-size` becoming JavaScript-compatible names such as `fontSize`.

ORCA needs a stronger, generated version of this idea.

Each atomic property should have one canonical identity:

```text
TextRun.FontSize
```

But several authoring spellings can map to it:

```text
FontSize
font-size
fontSize
TextRun.FontSize
text-run.font-size
```

The same applies to shorthands:

```text
Font
font
TextRun.Font
text-run.font
```

Codegen should produce alias metadata instead of leaving each subsystem to
invent its own name normalization rules.

A possible generated descriptor is:

```c
struct PropertyAlias {
  uint32_t aliasID;
  uint32_t canonicalPropertyID;
  uint32_t flags;
};
```

For shorthands:

```c
struct ShorthandAlias {
  uint32_t aliasID;
  uint32_t canonicalShorthandID;
  uint32_t flags;
};
```

This gives every authoring surface the same lookup behavior:

- XML attributes.
- XML property elements.
- Lua field assignment.
- MoonScript wrappers.
- CSS-like style sheets.
- Inline style strings.
- Bindings and animations.

It also helps with attached properties. An object that does not own `TextRun`
should still be able to accept:

```xml
<Stack TextRun.Font="bold 22 Inter"/>
```

That attached shorthand lookup needs global shorthand metadata, just as attached
atomic properties need globally registered property metadata.

## ORCA Design Summary

The combined model is:

```text
authoring input
  -> normalize name
  -> find atomic property or shorthand
  -> parse direct value, contextual value, or shorthand
  -> produce atomic declarations
  -> cascade by priority/order
  -> direct write or generated binding
  -> final effective atomic property
  -> optional generated convenience struct for C hot code
```

For `Font`, this means:

```text
Font shorthand
  -> FontWeight declaration
  -> FontStyle declaration
  -> FontSize declaration or binding
  -> FontFamily declaration
```

Then C can still use:

```c
struct FontShorthand font = TextRun_GetFontShorthand(text);
```

but that value is synthesized from effective atomic properties on request.

The big rule stays simple:

> Store atomic properties. Treat shorthands as authoring and convenience APIs.

Netscape's style system points toward a useful layered architecture, but ORCA's
version should remain generated, typed, and property-driven.
