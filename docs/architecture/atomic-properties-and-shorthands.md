# Atomic Properties And Authoring Shorthands

This is a design draft for separating real engine properties from authoring
shorthands.

The immediate problem is inherited shorthand storage. If `TextRun.Font` is a
real generated property, and `FontShorthand` is marked inherited, generated C
starts exposing this shape:

```c
struct TextRun {
  struct {
    enum FontWeight *Weight;
    enum FontStyle *Style;
    float *Size;
    struct FontFamily **Family;
  } Font;
};
```

That is technically usable, but it is the wrong public API. It exposes parser
grouping as engine storage and makes hot code read `text.Font.Size` even though
`TextRun.FontSize` is the real semantic property authors and style systems talk
about.

The proposed rule is:

> Component structs contain only atomic properties. Shorthands are metadata that
> lower authoring syntax into writes to those atomic properties.

## Goals

- Keep hot C reads flat and predictable.
- Keep inherited slots attached to atomic properties only.
- Allow WPF/CSS-style grouped authoring syntax such as `Font="..."`,
  `Foreground="..."`, and `RenderTransform="..."`.
- Avoid generating fake group fields like `TextRun.Font` or
  `Node2D.RenderTransform`.
- Keep Lua, XML, styles, and bindings speaking the same public property names.

## Definitions

An atomic property is a real property with storage, metadata, property ID,
offset, default value, inheritance flag, binding support, Lua export, and C field.

Examples:

```xml
<property name="FontWeight" type="FontWeight" inherited="true"/>
<property name="FontStyle" type="FontStyle" inherited="true"/>
<property name="FontSize" type="float" inherited="true" default="DEFAULT_FONT_SIZE"/>
<property name="FontFamily" type="FontFamily" pointer="true" inherited="true"/>
```

Generated C:

```c
struct TextRun {
  enum FontWeight *FontWeight;
  enum FontStyle *FontStyle;
  float *FontSize;
  struct FontFamily **FontFamily;
};
```

A shorthand is an authoring alias. It has no generated C field and no effective
property value of its own. It is parsed into a set of atomic property writes.

Example:

```xml
<shorthand name="Font" type="FontShorthand">
  <target field="Weight" property="FontWeight"/>
  <target field="Style" property="FontStyle"/>
  <target field="Size" property="FontSize"/>
  <target field="Family" property="FontFamily"/>
</shorthand>
```

`TextRun.Font="..."` is accepted by XML/Lua/style loaders, but it lowers to
`TextRun.FontWeight`, `TextRun.FontStyle`, `TextRun.FontSize`, and
`TextRun.FontFamily`.

## Non-Goals

- Do not create a real `TextRun.Font` property.
- Do not create a generated `struct TextRun::Font` group.
- Do not make shorthand objects participate in inheritance.
- Do not make render/layout code ask the property system for values on every
  draw or measure pass.

## Example: TextRun Font

The current declaration should move away from this:

```xml
<property name="Font" type="FontShorthand" default=".Size=DEFAULT_FONT_SIZE" inherited="true">
  Font definition used for text rendering.
</property>
```

And become:

```xml
<property name="FontWeight" type="FontWeight" inherited="true">
  Font weight (normal or bold).
</property>
<property name="FontStyle" type="FontStyle" inherited="true">
  Font style (normal or italic).
</property>
<property name="FontSize" type="float" default="DEFAULT_FONT_SIZE" inherited="true">
  Font size in points.
</property>
<property name="FontFamily" type="FontFamily" pointer="true" inherited="true">
  Font family reference.
</property>
<shorthand name="Font" type="FontShorthand">
  <target field="Weight" property="FontWeight"/>
  <target field="Style" property="FontStyle"/>
  <target field="Size" property="FontSize"/>
  <target field="Family" property="FontFamily"/>
</shorthand>
```

Hot code reads:

```c
struct FontShorthand font = {
  .Weight = IVALUE(text.FontWeight, kFontWeightNormal),
  .Style = IVALUE(text.FontStyle, kFontStyleNormal),
  .Size = IVALUE(text.FontSize, DEFAULT_FONT_SIZE),
  .Family = IVALUE(text.FontFamily, NULL),
};
```

There is no `text.Font.Weight`. The group exists only while parsing authoring
syntax.

## Example: Node2D Foreground

`Node2D.Foreground` should not be a real property if its only purpose is to
write one of these:

```xml
<property name="ForegroundColor" type="color" inherited="true"/>
<property name="ForegroundImage" type="Texture" pointer="true" inherited="true"/>
<property name="ForegroundMaterial" type="Material" pointer="true" inherited="true"/>
```

A shorthand maps the grouped input:

```xml
<shorthand name="Foreground" type="BrushShorthand">
  <target field="Color" property="ForegroundColor"/>
  <target field="Image" property="ForegroundImage"/>
  <target field="Material" property="ForegroundMaterial"/>
</shorthand>
```

Authoring:

```xml
<Stack Foreground="#336699">
  <TextBlock Text="Inherited"/>
</Stack>
```

Runtime:

- `Foreground="#336699"` is parsed as `ForegroundColor`.
- The attached inherited property is `Node2D.ForegroundColor`.
- Descendant text reads `IVALUE(node.ForegroundColor, fallback)`.
- There is no `node.Foreground.Color` generated field.

## Example: RenderTransform

The same model applies to transforms. `RenderTransform` is syntax sugar for
atomic transform properties:

```xml
<property name="RenderTransformTranslation" type="vec2"/>
<property name="RenderTransformRotation" type="float"/>
<property name="RenderTransformScale" type="vec2"/>
<property name="RenderTransformOrigin" type="vec2"/>
<shorthand name="RenderTransform" type="TransformShorthand">
  <target field="Translation" property="RenderTransformTranslation"/>
  <target field="Rotation" property="RenderTransformRotation"/>
  <target field="Scale" property="RenderTransformScale"/>
  <target field="Origin" property="RenderTransformOrigin"/>
</shorthand>
```

Setting:

```xml
<TextBlock RenderTransform="translate(10 20) rotate(15) scale(2)"/>
```

Does not set `Node2D.RenderTransform`. There is no such property. The loader
parses the shorthand and writes the relevant atomic properties.

## Property Metadata

`ClassDesc` should expose two related but separate lists:

```c
struct ClassDesc {
  struct PropertyType const *Properties;
  int PropertyCount;
  struct PropertyShorthand const *Shorthands;
  int ShorthandCount;
};
```

`PropertyType` remains the source of truth for real properties. It should not
know whether a shorthand points at it, except perhaps for documentation links.

`PropertyShorthand` describes authoring aliases:

```c
struct PropertyShorthand {
  const char *Name;          /* "Font" */
  const char *Type;          /* "FontShorthand" */
  uint32_t ShortIdentifier;  /* fnv1a32("Font") */
  uint32_t FullIdentifier;   /* fnv1a32("TextRun.Font") */
  const struct ShorthandTarget *Targets;
  int TargetCount;
};

struct ShorthandTarget {
  const char *Field;         /* "Size" */
  const char *Property;      /* "FontSize" */
  uint32_t PropertyID;       /* fnv1a32("TextRun.FontSize") */
};
```

The exact names can change. The important split is that shorthand metadata is
looked up by authoring systems, while `PropertyType` remains the real runtime
property list.

## Cgen Syntax

The current module schema already has a `<shorthand>` tag, but it describes
struct-field expansion with attributes such as `physical` and `userdata`. That
is too close to generated storage.

The new shorthand declaration should live beside properties and describe a
lowering rule:

```xml
<properties>
  <property name="FontWeight" type="FontWeight" inherited="true"/>
  <property name="FontStyle" type="FontStyle" inherited="true"/>
  <property name="FontSize" type="float" default="DEFAULT_FONT_SIZE" inherited="true"/>
  <property name="FontFamily" type="FontFamily" pointer="true" inherited="true"/>
</properties>
<shorthands>
  <shorthand name="Font" type="FontShorthand">
    <target field="Weight" property="FontWeight"/>
    <target field="Style" property="FontStyle"/>
    <target field="Size" property="FontSize"/>
    <target field="Family" property="FontFamily"/>
  </shorthand>
</shorthands>
```

This keeps shorthand data close to the class that owns the atomic properties,
without making the shorthand itself a property.

The generated representation can be explicit:

```c
static const struct ShorthandTarget _TextRun_FontTargets[] = {
  { "Weight", "FontWeight", TextRun_FontWeight },
  { "Style", "FontStyle", TextRun_FontStyle },
  { "Size", "FontSize", TextRun_FontSize },
  { "Family", "FontFamily", TextRun_FontFamily },
};
```

Or compact when the target properties are contiguous in the generated property
table:

```c
struct PropertyShorthand {
  const char *Name;
  const char *Type;
  uint32_t FullIdentifier;
  int FirstPropertyIndex;
  int PropertyCount;
};
```

The compact form is attractive for generated code, but the design should not
depend on contiguity. The semantic model is still "this shorthand maps parsed
fields to these atomic properties".

## Lookup Pipeline

When setting a property by name:

1. Try to find an atomic property by full name or contextual short name.
2. If found, set it normally.
3. If not found, try to find a shorthand by full name or contextual short name.
4. Parse the input value using the shorthand type.
5. For each parsed field that is present, set the mapped atomic property.
6. If no atomic property or shorthand is found, report an unknown property.

This keeps regular property sets fast and makes shorthand parsing an authoring
edge, not a rendering edge.

## Inheritance

Inheritance applies only to atomic properties.

Good:

```xml
<Stack TextRun.FontSize="22">
  <TextBlock Text="large"/>
</Stack>
```

Also good:

```xml
<Stack TextRun.Font="22 bold Inter">
  <TextBlock Text="large"/>
</Stack>
```

The second form lowers to atomic property writes. After lowering, inheritance
does not know or care that the writes came from a shorthand.

This avoids recursive inherited group storage and keeps slot wiring simple:

```c
text.FontSize = ancestorFontSizeSlot;
text.FontFamily = ancestorFontFamilySlot;
```

## Defaults

Defaults belong to atomic properties, not shorthands.

For `Font`, the default size should be:

```xml
<property name="FontSize" type="float" default="DEFAULT_FONT_SIZE" inherited="true"/>
```

A shorthand parser may provide temporary parse defaults while interpreting an
input string, but it should only write fields explicitly provided by the author
unless the shorthand definition says otherwise.

That matters for partial shorthands:

```xml
<Stack TextRun.FontFamily="Inter"/>
<TextBlock Font="bold"/>
```

`Font="bold"` should set `FontWeight`. It should not accidentally clear or
overwrite inherited `FontFamily`.

## Binding And Animation

Bindings and animations should bind to atomic properties.

Allowed:

```xml
<TextRun.FontSize>{Binding TitleSize}</TextRun.FontSize>
```

A shorthand binding can exist only if the binding result is parsed and fanned
out into atomic property updates:

```xml
<TextRun.Font>{Binding TitleFont}</TextRun.Font>
```

This is convenient authoring syntax, but internally it is a binding attached to
a shorthand adapter, not a normal property value. The adapter re-runs the
shorthand parser when the binding changes and updates the atomic properties.

If that is too much machinery initially, shorthand bindings can be rejected with
a clear error while normal atomic property bindings remain fully supported.

## Lua API

Lua should expose atomic properties as normal fields:

```lua
text.FontSize = 22
text.FontWeight = "Bold"
```

For shorthand assignment:

```lua
text.Font = "22 Bold Inter"
```

Lua should route through the same shorthand pipeline as XML and styles. It
should not imply that `Font` is stored as a component field.

Reading shorthand values is optional and should not be part of the first pass.
If implemented, it should synthesize a table or shorthand struct from atomic
effective values:

```lua
local font = text.Font
```

But this is convenience. The reliable API is still:

```lua
text.FontSize
text.FontWeight
text.FontStyle
text.FontFamily
```

## Documentation Output

Generated API docs should show atomic properties as properties.

Shorthands should be documented separately as authoring aliases:

```text
Properties
  Text
  FontWeight [inherited]
  FontStyle [inherited]
  FontSize [inherited]
  FontFamily [inherited]

Shorthands
  Font -> FontWeight, FontStyle, FontSize, FontFamily
```

Generated C comments should stay on atomic fields:

```c
enum FontWeight *FontWeight; ///< Font weight (normal or bold) [inherited].
enum FontStyle *FontStyle;   ///< Font style (normal or italic) [inherited].
float *FontSize;             ///< Font size in points [inherited].
struct FontFamily **FontFamily; ///< Font family reference [inherited].
```

No anonymous `Font` struct should appear in the public generated header.

## Migration Plan

1. Extend the cgen model to represent shorthand nodes and shorthand targets.
2. Extend `module.dtd` with the new shorthand shape.
3. Change generated headers to emit only atomic properties as fields.
4. Change generated property tables to include only atomic properties in
   `PropertyType`.
5. Add `PropertyShorthand` metadata to `ClassDesc`.
6. Update property lookup/set paths to try shorthand metadata after atomic
   property lookup fails.
7. Rewrite UIKit shorthand properties as atomic properties plus shorthand
   metadata:
   - `TextRun.Font`
   - `TextRun.Underline`
   - `Node2D.Foreground`
   - `Node2D.Background`
   - `Node2D.RenderTransform`
   - `Node2D.LayoutTransform`
   - `Node2D.BoxShadow`
   - `Node2D.Ring`
   - `Node2D.Overflow`
8. Update render/layout code to use flat atomic inherited fields.
9. Add tests for XML, Lua, styles, attached properties, inheritance, local
   overrides, late parent set, and clear/reparent behavior.

## Open Questions

- Should shorthand names have property IDs in the same namespace as properties,
  or a separate namespace?
- Should reading `text.Font` synthesize a shorthand value, or should shorthand
  names be write-only authoring aliases?
- Should shorthand assignment clear omitted fields, or only write present fields?
  The safer default is "only write present fields".
- Should a shorthand definition allow explicit clear semantics, such as
  `Font=""` clearing all mapped atomic font properties?
- Should style serialization emit shorthand names when possible, or always emit
  atomic properties?

## Decision

Treat shorthands as syntax sugar over atomic properties.

Do not generate group storage for shorthand names. Do not mark shorthand groups
as inherited. Mark the atomic properties as inherited. The property system,
bindings, styles, Lua, XML, docs, and C hot paths should all converge on those
atomic property names.
