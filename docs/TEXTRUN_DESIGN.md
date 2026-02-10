# TextRun Multi-Run Support

This document explains the implementation of multi-run text support in Orca, similar to WPF's TextBlock/Run pattern.

## Overview

The multi-run text system allows rendering rich text with multiple text segments (runs), each with different formatting properties such as font, size, and style. This is similar to WPF's TextBlock containing multiple Run elements.

## Design Questions & Answers

### 1. How do we provide text runs?

**Answer: Callback system (WinAPI-style enumeration)**

We use a callback enumeration pattern similar to WinAPI's `EnumWindows`. This approach was chosen over array/list allocation because:

- **No pre-allocation required**: The caller manages their runs however they want (array, linked list, generated on-the-fly)
- **Flexibility**: Runs can be stored in different data structures
- **Memory efficiency**: No need to copy runs into a temporary array
- **Familiar pattern**: Follows established WinAPI conventions used elsewhere in the codebase

#### Callback Pattern

```c
// Callback to process each run (implemented by renderer)
typedef bool_t (*EnumTextRunProc)(struct ViewTextRun const* run, void* userData);

// Callback to enumerate runs (implemented by caller)
typedef bool_t (*EnumTextRunsProc)(
    EnumTextRunProc callback,    // Renderer's processing callback
    void* callbackData,          // Data for the callback
    void* userData               // Caller's data (e.g., TextBlock)
);
```

#### Flow

1. Caller (TextBlock) implements `EnumTextRunsProc` to provide runs
2. Renderer (r_font) provides `EnumTextRunProc` to process runs
3. Renderer calls the caller's `EnumTextRunsProc`, passing its processing callback
4. Caller's function calls the renderer's callback for each run

### 2. What changes in r_font for recalculating measurements?

**Answer: Font measurements are recalculated for each run**

The `T_GetSize()` function was refactored to:

1. **Support per-run font switching**: Each run can use a different font face and size
2. **Track maximum line height**: Line height is determined by the tallest font on each line
3. **Accumulate measurements**: Measurements are accumulated across runs sequentially
4. **Handle kerning per-run**: Kerning is applied within each run (not across run boundaries)

#### Key Changes in r_font.c

- **MeasureContext structure**: Tracks state across multiple runs (width, height, kerning state, etc.)
- **T_MeasureRunCallback()**: Measures a single run with its specific font properties
- **T_GetSize() refactor**: Enumerates runs and accumulates measurements
- **Font face switching**: `T_GetFontFace()` is called for each run to get the appropriate font

```c
// For each run:
// 1. Get the font face for this run
FT_Face face = T_GetFontFace(run);

// 2. Set pixel size for this run's font
FT_Set_Pixel_Sizes(face, 0, run->fontSize);

// 3. Measure text with this font
// 4. Track maximum line height
ctx->maxLineHeight = MAX(ctx->maxLineHeight, lineHeight);
```

## Usage

### Single-Run Text (Backward Compatible)

```c
struct ViewText text = {
  .run = {
    .string = "Hello World",
    .fontSize = 12,
    .fontStyle = FS_NORMAL
  },
  .enumRunsFunc = NULL  // NULL = single run mode
};
```

### Multi-Run Text (Rich Text)

```c
// 1. Implement enumeration callback
bool_t MyEnumRuns(EnumTextRunProc callback, void* callbackData, void* userData) {
  MyTextBlock* block = (MyTextBlock*)userData;
  for (int i = 0; i < block->runCount; i++) {
    if (!callback(&block->runs[i], callbackData))
      return FALSE;
  }
  return TRUE;
}

// 2. Create text with multiple runs
TextBlock block = {
  .runs = (struct ViewTextRun[]){
    { .string = "Hello ", .fontSize = 12, .fontStyle = FS_NORMAL },
    { .string = "bold", .fontSize = 12, .fontStyle = FS_BOLD },
    { .string = " world", .fontSize = 12, .fontStyle = FS_NORMAL }
  },
  .runCount = 3
};

// 3. Set up ViewText
struct ViewText text = {
  .run = block.runs[0],
  .enumRunsFunc = MyEnumRuns,
  .enumRunsData = &block
};
```

## Implementation Status

### Completed
- ✅ `ViewTextRun` structure for individual runs
- ✅ `EnumTextRunProc` callback type for processing runs
- ✅ `EnumTextRunsProc` callback type for enumerating runs
- ✅ `ViewText` extended with `enumRunsFunc` and `enumRunsData`
- ✅ `T_GetSize()` refactored to measure multiple runs
- ✅ Per-run font face switching
- ✅ Maximum line height tracking across runs
- ✅ Comprehensive documentation
- ✅ Example implementation code

### TODO
- ⏳ `Text_Print()` refactor for rendering multiple runs
- ⏳ Testing with real multi-run text
- ⏳ Integration with UIKit TextBlock component

## Technical Details

### Font Measurements Per Run

Each run is measured independently with these properties:
- **Font face**: Different fonts can be mixed in one text block
- **Font size**: Each run can have a different size
- **Font style**: Normal, bold, italic, or bold-italic
- **Letter spacing**: Per-run letter spacing
- **Fixed character width**: For monospace effects

### Line Height Handling

When multiple runs with different font sizes appear on the same line:
1. Each run's line height is calculated: `FT_MulFix(face->height, face->size->metrics.y_scale)`
2. The maximum line height is used for that line
3. This ensures proper vertical spacing regardless of font mixing

### Word Wrapping

Word wrapping works across runs:
- The `availableWidth` applies to the entire text block
- Words can span run boundaries (though this is typically avoided in practice)
- Wrapping decisions consider accumulated width across all runs

## Benefits

1. **Flexibility**: Supports any storage mechanism for runs (array, list, generated)
2. **Efficiency**: No temporary allocations or copies
3. **Backward compatibility**: Single-run mode works without changes
4. **Extensibility**: Easy to add new formatting properties to runs
5. **Familiar pattern**: Follows WinAPI conventions

## Example: Creating Rich Text

See `docs/textrun_example.c` for complete examples including:
- Simple single-run text
- Multi-run rich text with mixed styles
- Mixed fonts and sizes
- Dynamic TextBlock with run management

## References

- `include/renderer.h` - ViewTextRun and ViewText structures
- `source/renderer/r_font.c` - Font measurement and rendering
- `docs/textrun_example.c` - Usage examples
