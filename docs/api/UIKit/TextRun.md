# TextRun

**Class** | `orca.UIKit`

Shared concept for text rendering in 2D and 3D nodes.

## Overview

This class is not instantiated directly. Instead, it provides the common properties and functionality for text rendering, used internally by higher-level components such as 2D and 3D text nodes. The concept encapsulates text content, resources, formatting options, alignment, and font configuration.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `Text` | `fixed` | The short text string displayed in the block, limited to 64 bytes. If you need more space consider using `TextResourceID`. |
| `Font` | `FontShorthand` | Font definition used for text rendering. Contains typeface, size, and style information. |
| `Underline` | `UnderlineShorthand` | Underline style applied to the text. |
| `LetterSpacing` | `float` | Additional spacing applied between letters. |
| `LineHeight` | `float` | Line height multiplier for multi-line text layout. |
| `CharacterSpacing` | `float` | Extra spacing between characters, beyond kerning. |
| `FixedCharacterWidth` | `float` | Forces a fixed width per character (monospace effect). |
| `RemoveSideBearingsProperty` | `bool` | Removes side bearings (spacing) defined by the font. |

