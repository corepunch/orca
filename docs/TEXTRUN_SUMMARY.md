# TextRun Multi-Run Implementation Summary

## Changes Implemented

### 1. Core Data Structures (include/renderer.h)

Added support for multi-run text via callback enumeration pattern:

```c
// Individual text run with formatting
struct ViewTextRun {
  lpcString_t string;
  struct font* font;
  uint32_t fontSize;
  FONTSTYLE fontStyle;
  float letterSpacing;
  uint32_t fixedCharacterWidth;
};

// Callback to process each run (renderer side)
typedef bool_t (*EnumTextRunProc)(
    struct ViewTextRun const* run, 
    void* userData
);

// Callback to enumerate runs (caller side)
typedef bool_t (*EnumTextRunsProc)(
    EnumTextRunProc callback,
    void* callbackData,
    void* userData
);

// Extended ViewText for multi-run support
struct ViewText {
  struct ViewTextRun run;           // Primary run
  // ... other fields ...
  EnumTextRunsProc enumRunsFunc;   // Optional multi-run callback
  void* enumRunsData;               // Caller data
};
```

### 2. Font Measurement Logic (source/renderer/r_font.c)

Refactored `T_GetSize()` to support measuring multiple text runs:

```c
// Context to track state across runs
typedef struct {
  struct ViewText const* text;
  struct WI_Size textSize;
  uint32_t textwidth, wordwidth;
  FT_UInt prev_glyph_index;
  FT_Pos maxLineHeight;
  // ...
} MeasureContext;

// Measures a single run
static bool_t T_MeasureRunCallback(
    struct ViewTextRun const* run, 
    void* userData
);

// Measures text (single or multi-run)
static struct WI_Size T_GetSize(
    FT_Face face, 
    struct ViewText const* text, 
    struct rect* rcursor
);
```

**Key Features:**
- Per-run font face and size switching
- Maximum line height tracking across runs
- Kerning applied within runs
- Word wrapping across runs
- Backward compatible (NULL callback = single run)

### 3. Documentation

- **TEXTRUN_DESIGN.md** - Complete design rationale
- **textrun_example.c** - Practical usage examples
- Inline code documentation with examples

## Answers to Original Questions

### Question 1: How do we provide text runs?

**Answer: Callback Enumeration Pattern (WinAPI-style)**

```c
// Caller implements this
bool_t MyEnumRuns(EnumTextRunProc callback, void* callbackData, void* userData) {
  MyTextBlock* block = (MyTextBlock*)userData;
  for (int i = 0; i < block->runCount; i++) {
    if (!callback(&block->runs[i], callbackData))
      return FALSE;
  }
  return TRUE;
}

// Use it
struct ViewText text = {
  .run = block->runs[0],
  .enumRunsFunc = MyEnumRuns,
  .enumRunsData = &block
};
```

**Why not arrays?**
- More flexible (any storage: array, list, generated)
- No pre-allocation/copying required
- Memory efficient
- Follows existing codebase patterns (EnumSpreadheetProc, etc.)

### Question 2: What changes in r_font for measurements?

**Answer: Per-run font measurement with state accumulation**

Changes in `T_GetSize()`:
1. **Font face switching per run** - Each run can use different font/size
2. **Line height tracking** - Uses max height from all runs on a line
3. **State accumulation** - MeasureContext tracks width, kerning across runs
4. **Word wrapping** - Works across runs using accumulated width

Example flow:
```
Run 1: "Hello " (12pt, Normal)  -> measure with Font A, size 12
Run 2: "bold" (12pt, Bold)      -> measure with Font A Bold, size 12
Run 3: " world" (12pt, Normal)  -> measure with Font A, size 12
Final: Use max line height, total width
```

## Usage Example

```c
// Setup runs
TextBlock block = {
  .runs = (struct ViewTextRun[]){
    { .string = "Normal ", .fontSize = 12, .fontStyle = FS_NORMAL },
    { .string = "Bold", .fontSize = 12, .fontStyle = FS_BOLD },
    { .string = " text", .fontSize = 12, .fontStyle = FS_NORMAL }
  },
  .runCount = 3
};

// Create ViewText
struct ViewText text = {
  .run = block.runs[0],
  .availableWidth = 800,
  .backingScale = 1.0f,
  .enumRunsFunc = TextBlock_EnumRuns,
  .enumRunsData = &block
};

// Use with existing APIs
Text_GetInfo(&text, &info);    // Measures with multi-run support
Text_Print(&text, &img, FALSE); // TODO: Add multi-run rendering
```

## Build Verification

✅ Code compiles successfully
✅ Functions present in liborca.so:
  - `T_MeasureRunCallback`
  - `T_GetSize.isra.0`

## Status

**Completed:**
- ✅ ViewTextRun and ViewText structures
- ✅ Callback typedef system
- ✅ T_GetSize() measurement for multiple runs
- ✅ Comprehensive documentation
- ✅ Example code
- ✅ Build verification

**Remaining:**
- ⏳ Text_Print() rendering for multiple runs (TODO added in code)
- ⏳ Integration testing with UIKit TextBlock
- ⏳ Real-world usage examples

## Files Modified

1. `include/renderer.h` - Data structures and typedefs
2. `source/renderer/r_font.c` - Measurement implementation
3. `docs/TEXTRUN_DESIGN.md` - Design documentation
4. `docs/textrun_example.c` - Usage examples

## Backward Compatibility

✅ Fully backward compatible
- Single-run text works unchanged (enumRunsFunc = NULL)
- No breaking changes to existing APIs
- Optional feature - use only when needed

## Next Steps

1. Implement Text_Print() multi-run rendering
2. Add UIKit TextBlock component support
3. Create integration tests
4. Performance testing with complex layouts
