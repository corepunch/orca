# TextBlockConcept

**Class** | `orca.UIKit` | extends [TextRun](TextRun.md)

Shared concept for text rendering in 2D and 3D nodes.

## Overview

This class is not instantiated directly. Instead, it provides the common properties and functionality for text rendering, used internally by higher-level components such as 2D and 3D text nodes. The concept encapsulates text content, resources, formatting options, alignment, and font configuration.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `TextResourceID` | `fixed` | Resource identifier for localized text lookup. |
| `TextResourceConfiguration` | `fixed` | Configuration key used when resolving text resources. |
| `PlaceholderText` | `fixed` | Placeholder text displayed when no main text is set. |
| `TextOverflow` | `TextOverflow` | Defines how overflowing text should be handled. |
| `Placeholder` | `BrushShorthand` | Brush definition for rendering placeholder text. |
| `UseFullFontHeight` | `bool` | When true, uses the font's full height for layout calculations. |
| `ConstrainContentHeight` | `bool` | Constrains the content height to the text's bounding box. |
| `WordWrap` | `bool` | Enables automatic word wrapping of the text. |
| `TextWrapping` | `TextWrapping` | Controls text wrapping behavior. |
| `TextHorizontalAlignment` | `TextHorizontalAlignment` | Horizontal alignment of the text within its bounds. |
| `TextVerticalAlignment` | `TextVerticalAlignment` | Vertical alignment of the text within its bounds. |

