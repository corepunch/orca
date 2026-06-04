# CSS Compatibility

ORCA stylesheets parse a CSS-like subset and map supported CSS declaration names to ORCA properties before value parsing. This table is extracted from `plugins/UIKit/Css.c` (`k_css_prop_map`).

Values use ORCA property parsers. Numeric values are bare numbers; browser units such as `px`, `rem`, `vh`, and `%` are not converted. Unsupported properties are ignored when the stylesheet is converted into `StyleRule` objects.

| CSS property | ORCA property | Value type |
|--------------|---------------|------------|
| `background-color` | `Node2D.BackgroundColor` | color |
| `color` | `Node2D.ForegroundColor` | color |
| `opacity` | `Node.Opacity` | float |
| `width` | `Node.Width` | float |
| `height` | `Node.Height` | float |
| `min-width` | `Node.MinWidth` | float |
| `min-height` | `Node.MinHeight` | float |
| `horizontal-align` | `Node.HorizontalAlignment` | enum: HorizontalAlignment |
| `horizontal-alignment` | `Node.HorizontalAlignment` | enum: HorizontalAlignment |
| `vertical-align` | `Node.VerticalAlignment` | enum: VerticalAlignment |
| `vertical-alignment` | `Node.VerticalAlignment` | enum: VerticalAlignment |
| `align-self` | `Node.HorizontalAlignment` | enum: HorizontalAlignment |
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
| `clip-children` | `Node2D.ClipChildren` | bool |
| `content-stretch` | `Node2D.ContentStretch` | bool |
| `ignore-hit-test` | `Node2D.IgnoreHitTest` | bool; `pointer-events: none` maps to `true`, `auto` maps to `false` |
| `pointer-events` | `Node2D.IgnoreHitTest` | bool; `pointer-events: none` maps to `true`, `auto` maps to `false` |
| `size-to-content` | `Node2D.SizeToContent` | bool |
| `snap-to-pixel` | `Node2D.SnapToPixel` | bool |
| `align-items` | `StackView.AlignItems` | enum: AlignItems |
| `justify-content` | `StackView.JustifyContent` | enum: JustifyContent |
| `flex-direction` | `StackView.Direction` | enum: Direction |
| `direction` | `StackView.Direction` | enum: Direction |
| `gap` | `StackView.Spacing` | float |
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
| `text-wrapping` | `TextBlockConcept.TextWrapping` | enum: TextWrapping |
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
- `pointer-events` maps to `Node2D.IgnoreHitTest`: `none` becomes `true`; `auto` becomes `false`.
