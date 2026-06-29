# Supported CSS Properties

ORCA parses a CSS-like subset and maps declaration names to ORCA properties. This is the authoritative reference for what CSS you can write.

## Key Differences from Browser CSS

1. **Browser units are converted to bindings.** `rem`/`em` → parent Font.Size binding, `vh`/`vw` → viewport binding.
2. **Font shorthand works.** `font: 14px/1.5 sans-serif` expands to `font-size`, `line-height`, `font-family`.
3. **No `!important`.** Use specificity instead.
4. **No media queries.** Use ORCA's layout system.
5. **No animations/keyframes.** Use ORCA's AnimationPlayer.
6. **No custom properties.** Use theme variables `$token-name` instead.

## Supported Shorthands

ORCA supports these CSS shorthands:

| Shorthand | Example | Notes |
|-----------|---------|-------|
| `margin` | `margin: 10 20;` | Top/bottom 10, left/right 20 |
| `padding` | `padding: 10 20 30 40;` | Top, right, bottom, left |
| `border` | `border: 1 solid red;` | Width, style, color |
| `border-radius` | `border-radius: 8;` | All corners |
| `box-shadow` | `box-shadow: 0 2 8 rgba(0,0,0,0.1);` | Offset, blur, spread, color |
| `background` | `background: #333;` | Color shorthand |
| `overflow` | `overflow: hidden;` | Both axes |
| `font` | `font: 14px/1.5 sans-serif` | Expands to font-size, line-height, font-family |
| `text-decoration` | `text-decoration: underline;` | Use separate properties |

Browser units are converted to binding expressions:

| Unit | Binding | Example |
|------|---------|---------|
| `rem` | `{../Font.Size} × n` | `1.5rem` → `{../Font.Size} * 1.5` |
| `em` | `{../Font.Size} × n` | `2em` → `{../Font.Size} * 2` |
| `vh` | `{Screen.Height} × n/100` | `100vh` → `{Screen.Height}` |
| `vw` | `{Screen.Width} × n/100` | `50vw` → `{Screen.Width} * 0.5` |
| `vmin` | min(`{Screen.Width}`, `{Screen.Height}`) × n/100 | `vmin` → viewport minimum |
| `vmax` | max(`{Screen.Width}`, `{Screen.Height}`) × n/100 | `vmax` → viewport maximum |
| `px`, `pt` | Bare number (no binding) | `120px` → `120` |

Bare numbers (no unit) are passed as literal values.

## Layout Properties

| CSS Property | ORCA Property | Value Type |
|--------------|---------------|------------|
| `width` | `Node.Width` | float |
| `height` | `Node.Height` | float |
| `min-width` | `Node.MinWidth` | float |
| `min-height` | `Node.MinHeight` | float |
| `margin` | `Node.Margin` | Thickness |
| `margin-top` | `Node.MarginTop` | float |
| `margin-right` | `Node.MarginRight` | float |
| `margin-bottom` | `Node.MarginBottom` | float |
| `margin-left` | `Node.MarginLeft` | float |
| `margin-inline` | `Node.HorizontalMargin` | EdgeShorthand |
| `margin-block` | `Node.VerticalMargin` | EdgeShorthand |
| `padding` | `Node.Padding` | Thickness |
| `padding-top` | `Node.PaddingTop` | float |
| `padding-right` | `Node.PaddingRight` | float |
| `padding-bottom` | `Node.PaddingBottom` | float |
| `padding-left` | `Node.PaddingLeft` | float |
| `padding-inline` | `Node.HorizontalPadding` | EdgeShorthand |
| `padding-block` | `Node.VerticalPadding` | EdgeShorthand |

### Layout Alignment

- `width: auto` — Stretches in finite layout space (default)
- `width: 100%` — Compatibility shorthand for stretch
- `margin-left: auto; margin-right: auto;` — Center horizontally
- `margin-top: auto; margin-bottom: auto;` — Center vertically

## Visual Properties

| CSS Property | ORCA Property | Value Type |
|--------------|---------------|------------|
| `background-color` | `Node2D.BackgroundColor` | color |
| `color` | `Node2D.ForegroundColor` | color |
| `opacity` | `Node.Opacity` | float |
| `visibility` | `Node.Visible` | bool |
| `background` | `Node2D.Background` | BrushShorthand |
| `background-image` | `Node2D.BackgroundImage` | Texture |
| `foreground` | `Node2D.Foreground` | BrushShorthand |
| `foreground-color` | `Node2D.ForegroundColor` | color |

### Visibility Values

- `visibility: visible` → `true`
- `visibility: hidden` → `false`
- `visibility: collapse` → `false`

## Border Properties

| CSS Property | ORCA Property | Value Type |
|--------------|---------------|------------|
| `border` | `Node.Border` | BorderShorthand |
| `border-color` | `Node.BorderColor` | color |
| `border-style` | `Node.BorderStyle` | enum |
| `border-width` | `Node.BorderWidth` | Thickness |
| `border-left-width` | `Node.BorderWidthLeft` | float |
| `border-right-width` | `Node.BorderWidthRight` | float |
| `border-top-width` | `Node.BorderWidthTop` | float |
| `border-bottom-width` | `Node.BorderWidthBottom` | float |
| `border-radius` | `Node.BorderRadius` | CornerRadius |
| `border-top-left-radius` | `Node.BorderTopLeftRadius` | float |
| `border-top-right-radius` | `Node.BorderTopRightRadius` | float |
| `border-bottom-right-radius` | `Node.BorderBottomRightRadius` | float |
| `border-bottom-left-radius` | `Node.BorderBottomLeftRadius` | float |
| `border-image-source` | `Node2D.BorderImageSource` | Texture |
| `border-image-slice` | `Node2D.BorderImageSlice` | Thickness |
| `border-image-repeat` | `Node2D.BorderImageRepeat` | enum |

## Shadow Properties

| CSS Property | ORCA Property | Value Type |
|--------------|---------------|------------|
| `box-shadow` | `Node2D.BoxShadow` | ShadowShorthand |
| `box-shadow-color` | `Node2D.BoxShadowColor` | color |
| `box-shadow-blur` | `Node2D.BoxShadowBlurRadius` | float |
| `box-shadow-blur-radius` | `Node2D.BoxShadowBlurRadius` | float |
| `box-shadow-spread` | `Node2D.BoxShadowSpreadRadius` | float |
| `box-shadow-spread-radius` | `Node2D.BoxShadowSpreadRadius` | float |

## Ring Properties

| CSS Property | ORCA Property | Value Type |
|--------------|---------------|------------|
| `ring` | `Node2D.Ring` | RingShorthand |
| `ring-color` | `Node2D.RingColor` | color |
| `ring-offset` | `Node2D.RingOffset` | float |
| `ring-width` | `Node2D.RingWidth` | float |

## Overflow Properties

| CSS Property | ORCA Property | Value Type |
|--------------|---------------|------------|
| `overflow` | `Node2D.Overflow` | OverflowShorthand |
| `overflow-x` | `Node2D.OverflowX` | enum |
| `overflow-y` | `Node2D.OverflowY` | enum |

## Layout Component Properties

| CSS Property | ORCA Property | Value Type |
|--------------|---------------|------------|
| `align-items` | `StackView.AlignItems` | enum |
| `justify-content` | `StackView.JustifyContent` | enum |
| `flex-direction` | `StackView.Direction` | enum |
| `direction` | `StackView.Direction` | enum |
| `gap` | `StackView.Spacing`, `Grid.Spacing` | float |
| `spacing` | `StackView.Spacing` | float |
| `reversed` | `StackView.Reversed` | bool |

## Typography Properties

| CSS Property | ORCA Property | Value Type |
|--------------|---------------|------------|
| `font` | `TextRun.Font` | FontShorthand |
| `font-size` | `TextRun.FontSize` | float |
| `font-family` | `TextRun.FontFamily` | family list |
| `font-weight` | `TextRun.FontWeight` | enum |
| `font-style` | `TextRun.FontStyle` | enum |
| `line-height` | `TextRun.LineHeight` | float |
| `letter-spacing` | `TextRun.LetterSpacing` | float |
| `character-spacing` | `TextRun.CharacterSpacing` | float |
| `fixed-character-width` | `TextRun.FixedCharacterWidth` | float |
| `text-decoration` | `TextRun.Underline` | UnderlineShorthand |
| `text-decoration-color` | `TextRun.UnderlineColor` | color |
| `text-decoration-thickness` | `TextRun.UnderlineWidth` | float |
| `underline` | `TextRun.Underline` | UnderlineShorthand |
| `underline-color` | `TextRun.UnderlineColor` | color |
| `underline-width` | `TextRun.UnderlineWidth` | float |
| `underline-offset` | `TextRun.UnderlineOffset` | float |
| `word-wrap` | `TextBlockConcept.WordWrap` | bool |
| `overflow-wrap` | `TextBlockConcept.WordWrap` | bool |
| `text-wrap` | `TextBlockConcept.TextWrapping` | enum |
| `text-overflow` | `TextBlockConcept.TextOverflow` | enum |
| `text-align` | `TextBlockConcept.TextHorizontalAlignment` | enum |
| `text-vertical-align` | `TextBlockConcept.TextVerticalAlignment` | enum |
| `placeholder-color` | `TextBlockConcept.PlaceholderColor` | color |

## Interaction Properties

| CSS Property | ORCA Property | Value Type |
|--------------|---------------|------------|
| `ignore-hit-test` | `Node2D.IgnoreHitTest` | bool |
| `pointer-events` | `Node2D.IgnoreHitTest` | bool |
| `content-stretch` | `Node2D.ContentStretch` | bool |
| `size-to-content` | `Node2D.SizeToContent` | bool |
| `snap-to-pixel` | `Node2D.SnapToPixel` | bool |

### Pointer Events Values

- `pointer-events: none` → `true` (ignore hits)
- `pointer-events: auto` → `false` (accept hits)

## Syntax Rules

- **Case insensitive:** Property names and enum values
- **Case sensitive:** Selector names, pseudo-state names
- **Lowercase enums preferred:** `direction: vertical;` not `direction: Vertical;`
- **Repeated keys:** Last value wins
- **Unsupported properties:** Silently ignored
