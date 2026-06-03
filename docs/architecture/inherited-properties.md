# Inherited Property Slots

Inherited properties are ORCA's WPF-style mechanism for values such as
`Node2D.ForegroundColor` and `TextRun.FontFamily`. A parent can define the
value once and descendants read it as their effective value unless they set a
local override.

The important rule is that inherited values are not repeatedly looked up while
drawing or measuring. Hot code still reads directly from component structs. The
property system does the slower wiring work when objects are attached, local
values change, or inherited values are cleared.

## Goals

- Keep render and layout reads as close to direct struct access as possible.
- Allow attached inherited properties on objects that do not own the component.
- Avoid copying inherited values through the whole subtree when a parent value
  changes.
- Preserve the normal local-value precedence rule: local values beat inherited
  values.

## Terminology

An inherited property has an effective slot. The slot stores the current
effective value. Descendants point at the nearest inherited slot instead of
copying the value.

For a normal scalar property:

```c
float FontSize;
```

the slot type is:

```c
float *
```

For an object-valued property:

```c
struct FontFamily *FontFamily;
```

the slot type is:

```c
struct FontFamily **
```

The extra pointer is not an accident. The inherited field points at the slot,
and dereferencing the slot gives the current value.

## Component Field Shape

Inherited component fields should store pointers to effective slots:

```c
struct TextRun {
  float *FontSize;
  struct FontFamily **FontFamily;
};
```

Hot reads use `IVALUE`:

```c
#define IVALUE(PTR, FALLBACK) ((PTR) ? *(PTR) : (FALLBACK))

float size = IVALUE(textRun->FontSize, DEFAULT_FONT_SIZE);
struct FontFamily *family = IVALUE(textRun->FontFamily, NULL);
```

This gives a compact null-safe read:

```c
fontFamily = IVALUE(textRun->FontFamily, NULL);
```

## Property Storage

For inherited slot-backed fields, `property->value` is the stable address of the
component field, not the inherited value itself.

For:

```c
struct TextRun {
  struct FontFamily **FontFamily;
};
```

the matching property stores:

```c
property->value == &textRun->FontFamily;
```

Changing the effective source means changing the pointer stored in that field:

```c
#define ISET_SLOT(PROP, SLOT) (*(void **)(PROP)->value = (SLOT))
```

Examples:

```c
ISET_SLOT(property, parentProperty->localSlot);  // inherited
ISET_SLOT(property, property->localSlot);        // local
ISET_SLOT(property, NULL);                       // no value found
```

The exact name of `localSlot` is implementation detail. The key contract is:
`property->value` remains the address of the component field, and
`*(void **)property->value` is the effective slot pointer used by hot code.

## Attached Inherited Properties

Attached properties are sparse properties stored in `Object.properties`. The
owner object does not need the component struct.

Example:

```xml
<Screen id="screen">
  <Stack id="stack" TextRun.FontFamily="...">
    <TextBlock id="text"/>
  </Stack>
</Screen>
```

`Stack` does not own `TextRun`, but it can own an attached property:

```c
stack.properties += Property(TextRun.FontFamily);
```

That property owns a stable local slot containing:

```c
struct FontFamily *;
```

When inheritance reaches `text`, the `TextRun` component field is wired to the
nearest ancestor slot:

```c
textRun->FontFamily = stackFontFamilyProperty->slot;
```

Then hot code reads:

```c
struct FontFamily *family = IVALUE(textRun->FontFamily, NULL);
```

If the stack changes its font family later, it updates the value inside its
slot. The child already points at that slot, so the next read sees the new
value without copying through descendants.

## No Ancestor Value

If no ancestor defines an inherited value, the component field points nowhere:

```c
textRun->FontFamily = NULL;
```

Reading remains null-safe:

```c
struct FontFamily *family = IVALUE(textRun->FontFamily, NULL);
```

For the scene:

```xml
<Screen id="screen">
  <Stack id="stack">
    <TextBlock id="text"/>
  </Stack>
</Screen>
```

`text` has no inherited `TextRun.FontFamily`, so its effective value is `NULL`.

## Local Override

When a descendant sets a local value, it owns a local slot and points at it:

```xml
<TextBlock id="text" TextRun.FontFamily="local"/>
```

Runtime wiring:

```c
textFontFamilyProperty->slot = localFont;
textRun->FontFamily = textFontFamilyProperty->slot;
```

Parent changes no longer affect this text run, because it points at its own
slot rather than the ancestor slot.

## Flags

The existing local/inherited distinction still matters:

- `PF_MODIFIED`: this property has a local value.
- `PF_INHERITED`: this property is wired to an ancestor slot.

Slot ownership should be represented separately from value precedence. A local
attached inherited property may need heap storage for its slot because it has no
component field to store the value. Component-backed inherited properties can
use component-local slot storage generated into the component struct.

Do not use `PF_MODIFIED` as an ownership flag. It answers "does this property
have local precedence?", not "should this memory be freed?"

## Implementation Notes

The implementation should distinguish three addresses:

```text
property->value
  Address of the component field that stores the effective slot pointer.

*(void **)property->value
  The effective slot pointer currently used by the component.

*effective_slot
  The actual property value read by hot code.
```

For object-valued inherited properties:

```c
property->value                    == &textRun->FontFamily
*(struct FontFamily ***)property->value == textRun->FontFamily
**(struct FontFamily ***)property->value == current FontFamily
```

Most code should not spell those casts directly. Use `ISET_SLOT` when changing
the effective source, and `IVALUE` when reading from component structs.

## Suggested Macros

```c
#define IVALUE(PTR, FALLBACK) ((PTR) ? *(PTR) : (FALLBACK))
#define ISET_SLOT(PROP, SLOT) (*(void **)(PROP)->value = (SLOT))
```

`IVALUE` is for hot reads from component structs.

`ISET_SLOT` is for property-system wiring only.

## Precedence

Effective value resolution remains:

```text
local value > inherited ancestor value > default/fallback
```

The pointer model changes how values are wired, not the user-facing semantics.
