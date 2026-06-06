# CSS Compatibility

ORCA stylesheets parse a CSS-like subset and map supported CSS declaration names to ORCA properties before value parsing. This table is extracted from `plugins/UIKit/Css.c` (`k_css_prop_map`).

Values use ORCA property parsers. Numeric values are bare numbers; browser units such as `px`, `rem`, `vh`, and `%` are not converted, except for the special `width: 100%` stretch shorthand below. Unsupported properties are ignored when the stylesheet is converted into `StyleRule` objects.

CSS edge shorthands use CSS ordering before being passed to ORCA's WPF-like `Thickness` parser. For example, `padding: 10 32;` means top/bottom `10` and left/right `32`; `margin: 1 2 3 4;` means top `1`, right `2`, bottom `3`, left `4`.

For layout alignment, prefer CSS-native sizing and auto margins:

- `width: auto` maps to `Node.Width = NaN`, which is also the default and stretches in finite layout space.
- `width: 100%` also maps to `Node.Width = NaN` as a compatibility stretch shorthand.
- `margin-left: auto; margin-right: auto;` centers an explicit-width node horizontally.
- `margin-left: auto;` aligns an explicit-width node to the trailing/right edge.
- `margin-top: auto; margin-bottom: auto;` centers an explicit-height node vertically.
- `margin-top: auto;` aligns an explicit-height node to the trailing/bottom edge.

| CSS property | ORCA property | Value type |
|--------------|---------------|------------|
| `background-color` | `Node2D.BackgroundColor` | color |
| `color` | `Node2D.ForegroundColor` | color |
| `opacity` | `Node.Opacity` | float |
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
| `border` | `Node.Border` | BorderShorthand |
| `border-color` | `Node.BorderColor` | color |
| `border-style` | `Node.BorderStyle` | enum: BorderStyle |
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
| `visibility` | `Node.Visible` | bool; `visible` maps to `true`, `hidden` and `collapse` map to `false` |
| `background` | `Node2D.Background` | BrushShorthand |
| `background-image` | `Node2D.BackgroundImage` | Texture |
| `foreground` | `Node2D.Foreground` | BrushShorthand |
| `foreground-color` | `Node2D.ForegroundColor` | color |
| `box-shadow` | `Node2D.BoxShadow` | ShadowShorthand |
| `box-shadow-color` | `Node2D.BoxShadowColor` | color |
| `box-shadow-blur` | `Node2D.BoxShadowBlurRadius` | float |
| `box-shadow-blur-radius` | `Node2D.BoxShadowBlurRadius` | float |
| `box-shadow-spread` | `Node2D.BoxShadowSpreadRadius` | float |
| `box-shadow-spread-radius` | `Node2D.BoxShadowSpreadRadius` | float |
| `overflow` | `Node2D.Overflow` | OverflowShorthand |
| `overflow-x` | `Node2D.OverflowX` | enum: Overflow |
| `overflow-y` | `Node2D.OverflowY` | enum: Overflow |
| `ring` | `Node2D.Ring` | RingShorthand |
| `ring-color` | `Node2D.RingColor` | color |
| `ring-offset` | `Node2D.RingOffset` | float |
| `ring-width` | `Node2D.RingWidth` | float |
| `content-stretch` | `Node2D.ContentStretch` | bool |
| `ignore-hit-test` | `Node2D.IgnoreHitTest` | bool; `pointer-events: none` maps to `true`, `auto` maps to `false` |
| `pointer-events` | `Node2D.IgnoreHitTest` | bool; `pointer-events: none` maps to `true`, `auto` maps to `false` |
| `size-to-content` | `Node2D.SizeToContent` | bool |
| `snap-to-pixel` | `Node2D.SnapToPixel` | bool |
| `align-items` | `StackView.AlignItems` | enum: AlignItems |
| `justify-content` | `StackView.JustifyContent` | enum: JustifyContent |
| `flex-direction` | `StackView.Direction` | enum: Direction |
| `direction` | `StackView.Direction` | enum: Direction |
| `gap` | `StackView.Spacing`, `Grid.Spacing` | float |
| `spacing` | `StackView.Spacing` | float |
| `reversed` | `StackView.Reversed` | bool |
| `font` | `TextRun.Font` | FontShorthand |
| `font-size` | `TextRun.FontSize` | float |
| `font-family` | `TextRun.FontFamily` | registered family list or object path |
| `font-weight` | `TextRun.FontWeight` | enum: FontWeight |
| `font-style` | `TextRun.FontStyle` | enum: FontStyle |
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
| `text-wrap` | `TextBlockConcept.TextWrapping` | enum: TextWrapping |
| `text-overflow` | `TextBlockConcept.TextOverflow` | enum: TextOverflow |
| `text-align` | `TextBlockConcept.TextHorizontalAlignment` | enum: TextHorizontalAlignment |
| `text-horizontal-align` | `TextBlockConcept.TextHorizontalAlignment` | enum: TextHorizontalAlignment |
| `text-horizontal-alignment` | `TextBlockConcept.TextHorizontalAlignment` | enum: TextHorizontalAlignment |
| `text-vertical-align` | `TextBlockConcept.TextVerticalAlignment` | enum: TextVerticalAlignment |
| `text-vertical-alignment` | `TextBlockConcept.TextVerticalAlignment` | enum: TextVerticalAlignment |
| `placeholder-color` | `TextBlockConcept.PlaceholderColor` | color |

## Special Cases

- CSS property names are matched case-insensitively.
- Enum values are matched case-insensitively and may use CSS-style separators; for example `text-overflow: ellipsis;` maps to `TextOverflow = "Ellipsis"`.
- `font-family` accepts a comma-separated CSS family list. Registered family names and generic aliases such as `serif`, `sans-serif`, and `monospace` are resolved before falling back to explicit object paths.
- `visibility` is normalized before parsing: `visible` becomes `true`; `hidden` and `collapse` become `false`.
- `width: auto` and `width: 100%` normalize to `NaN`, which stretches in finite layout space.
- `margin-* : auto` normalizes to `NaN` and is used for alignment.
- `pointer-events` maps to `Node2D.IgnoreHitTest`: `none` becomes `true`; `auto` becomes `false`.
- Child clipping behavior is controlled through `overflow`, `overflow-x`, and `overflow-y`.
