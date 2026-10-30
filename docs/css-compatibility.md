# CSS Compatibility

ORCA stylesheets parse a CSS-like subset and map supported CSS declaration names to ORCA properties before value parsing. This table is extracted from `plugins/UIKit/Css.c` (`k_css_prop_map`).

Values use ORCA property parsers. Numeric values are bare numbers; browser units such as `px`, `rem`, `vh`, and `%` are not converted, except for the special `width: 100%` stretch shorthand below. Unsupported properties are ignored when the stylesheet is converted into `StyleRule` objects.

CSS edge shorthands use CSS ordering before being passed to ORCA's WPF-like `Thickness` parser. For example, `padding: 10 32;` means top/bottom `10` and left/right `32`; `margin: 1 2 3 4;` means top `1`, right `2`, bottom `3`, left `4`.

For layout alignment, prefer CSS-native sizing and auto margins:

- `width: auto` maps to `Node.Width = NaN`, which is also the default and stretches in finite layout space unless that axis has an auto margin.
- `width: 100%` also maps to `Node.Width = NaN` as a compatibility stretch shorthand.
- `margin-left: auto; margin-right: auto;` centers a node horizontally; auto-width nodes use their desired content width.
- `margin-left: auto;` aligns a node to the trailing/right edge; auto-width nodes use their desired content width.
- `margin-top: auto; margin-bottom: auto;` centers a node vertically; auto-height nodes use their desired content height.
- `margin-top: auto;` aligns a node to the trailing/bottom edge; auto-height nodes use their desired content height.

For text alignment, use the CSS-native `text-align` property for horizontal text placement. ORCA also supports `text-vertical-align` as a small extension for vertical text placement inside text bounds; `TextBlock` defaults to `top`, while `Input` defaults to `center`.

## Selectors and interactive states

Selectors support types, classes, IDs, descendants, direct children, and
compound pseudo-states. Supported pseudo-states are `:hover`, `:active`,
`:focus`, `:focus-visible`, `:dark`, `:checked`, `:indeterminate`, `:disabled`,
and `:enabled`. An unknown pseudo-state fails to match; it does not turn its
declarations into unconditional styling.

`:checked` follows `CheckBox.IsChecked`, `RadioButton.IsChecked`, or
`ToggleSwitch.IsOn`. `:indeterminate` follows `CheckBox.IsIndeterminate`;
`:disabled` and `:enabled` follow the selection control's `IsEnabled`.
Changing a state restyles its descendants, so a rule such as
`CheckBox:checked .control-mark` updates the generated mark. State declarations
are removed when the state stops matching, restoring the normal cascade.
Combine states when needed, for example `CheckBox:checked:disabled`.

Stylesheets apply from global defaults to scoped application sheets. Within a
sheet, matching state rules override matching base rules in source order. A
later application sheet can override a shared default even when that default
uses `:dark` or `:checked`. This is sheet/state priority, not full browser CSS
specificity.

### Selection-control parts

`CheckBox`, `RadioButton`, and `ToggleSwitch` retain their native input,
selection, and binding behavior while exposing ordinary child nodes for CSS.
The shared stylesheet is installed by normal application startup. Applications
can override its declarations without constructing their own visual tree.

| Control | Constructor classes | Generated part classes |
| --- | --- | --- |
| `CheckBox` | `selection-control checkbox` | `control-indicator`, `control-check control-mark` (SVG), `control-indeterminate`, `control-label` |
| `RadioButton` | `selection-control radio-button` | `control-indicator`, `control-mark` (filled circle), `control-label` |
| `ToggleSwitch` | `selection-control toggle-switch` | `control-indicator control-track`, `control-thumb`, `control-label` |

All controls expose `Text`, `IsEnabled`, `IndicatorSize`, and
`IndicatorSpacing`. Checkboxes expose `IsChecked` and `IsIndeterminate`;
radio buttons expose `IsChecked` and `Value` inside a `RadioGroup` with
`SelectedValue`; switches expose `IsOn`. The control owns its generated parts
and label placement; use CSS for their fill, border, radius, mark visibility,
and colors. Keep application bindings on the owning control. Use type selectors
for shared skins: XML loading or assigning an application `class` can replace
the root classes installed during construction, while the control's type remains
stable. Type matching is exact: list `CheckBox`, `RadioButton`, and `ToggleSwitch`
instead of selecting their `SelectionControl` base type. Generated part classes
remain available for descendant selectors.

```css
CheckBox .control-indicator {
    border: 2 solid #FFFFFF;
    border-radius: 0;
    background-color: transparent;
}
CheckBox:checked .control-indicator { background-color: #0084FF; }
CheckBox:disabled { opacity: 0.45; }
CheckBox:focus-visible, RadioButton:focus-visible, ToggleSwitch:focus-visible { ring: 0 2 #0084FF; }
```

The checkbox checkmark is a shared Lucide SVG rendered by `VectorView` with
the part's `color`. CSS `-orca-source` maps to `VectorView.Source` when a skin
needs a different SVG resource. Radio circles and switch geometry use styled
nodes. See the deterministic dark/light [selection gallery](../apps/Controls/README.md)
for native visual verification.

## Declaration mappings

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
| `border-image-source` | `Node2D.BorderImageSource` | Texture |
| `border-image-slice` | `Node2D.BorderImageSlice` | Thickness: left top right bottom pixels |
| `border-image-repeat` | `Node2D.BorderImageRepeat` | enum: BorderImageRepeat |
| `foreground` | `Node2D.Foreground` | BrushShorthand |
| `foreground-color` | `Node2D.ForegroundColor` | color |
| `box-shadow` | `Node2D.BoxShadow` | ShadowShorthand |
| `box-shadow-color` | `Node2D.BoxShadowColor` | color |
| `box-shadow-blur` | `Node2D.BoxShadowBlurRadius` | float |
| `box-shadow-blur-radius` | `Node2D.BoxShadowBlurRadius` | float |
| `box-shadow-spread` | `Node2D.BoxShadowSpreadRadius` | float |
| `box-shadow-spread-radius` | `Node2D.BoxShadowSpreadRadius` | float |
| `backdrop-filter` | `Node2D.BackdropFilter` | BackdropFilterShorthand |
| `-webkit-backdrop-filter` | `Node2D.BackdropFilter` | BackdropFilterShorthand |
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
| `text-vertical-align` | `TextBlockConcept.TextVerticalAlignment` | enum: TextVerticalAlignment |
| `placeholder-color` | `TextBlockConcept.PlaceholderColor` | color |
| `-orca-source` | `VectorView.Source` | SVG resource path |
| `-orca-indicator-spacing` | `SelectionControl.IndicatorSpacing` | float |

## Special Cases

- CSS property names are matched case-insensitively.
- Enum values are matched case-insensitively and may use CSS-style separators; for example `text-overflow: ellipsis;` maps to `TextOverflow = "Ellipsis"`.
- `font-family` accepts a comma-separated CSS family list. Registered family names and generic aliases such as `serif`, `sans-serif`, and `monospace` are resolved before falling back to explicit object paths.
- `box-shadow` uses `x-offset y-offset blur-radius spread-radius color` with bare numeric values, for example `box-shadow: 2 2 0 0 black`. On `TextBlock`, the offset and color are rendered through the text glyph mask; blur and spread are parsed but are not yet rendered.
- `backdrop-filter` accepts `none` or a list of `blur(<length>)` and `saturate(<number or percentage>)`, for example `backdrop-filter: blur(20) saturate(180%)`. The blur radius is a standard deviation, as in CSS. Other filter functions make the declaration invalid. The backdrop is blurred within the element's background box with mirrored edges, and a translucent `background-color` tints it.
- `visibility` is normalized before parsing: `visible` becomes `true`; `hidden` and `collapse` become `false`.
- `width: auto` and `width: 100%` normalize to `NaN`, which stretches in finite layout space unless that axis has an auto margin.
- `margin-* : auto` normalizes to `NaN` and is used for alignment.
- `text-vertical-align` is an ORCA extension; use it only when text must be vertically positioned inside a `TextBlock` or `Input`.
- `pointer-events` maps to `Node2D.IgnoreHitTest`: `none` becomes `true`; `auto` becomes `false`.
- Child clipping behavior is controlled through `overflow`, `overflow-x`, and `overflow-y`.
