/**
 * @file textrun_example.c
 * @brief Example implementation showing how to use multi-run text support
 * 
 * This example demonstrates how to implement rich text with multiple text runs,
 * each with different fonts, sizes, and styles, similar to WPF's TextBlock/Run pattern.
 */

#include <include/renderer.h>

// Example: A simple TextBlock structure that holds multiple runs
typedef struct {
  struct ViewTextRun* runs;  // Array of text runs
  int runCount;              // Number of runs
  int runCapacity;           // Allocated capacity
} TextBlock;

/**
 * Enumeration callback implementation for TextBlock.
 * This is called by the renderer (r_font) to process each text run.
 * 
 * @param callback The renderer's callback (T_MeasureRunCallback or similar)
 * @param callbackData Data for the callback (MeasureContext or RenderContext)
 * @param userData Pointer to the TextBlock instance
 * @return TRUE if all runs were enumerated successfully
 */
static bool_t
TextBlock_EnumRuns(EnumTextRunProc callback, void* callbackData, void* userData)
{
  TextBlock* block = (TextBlock*)userData;
  
  // Call the renderer's callback for each run in our text block
  for (int i = 0; i < block->runCount; i++) {
    if (!callback(&block->runs[i], callbackData)) {
      return FALSE;  // Renderer requested stop (e.g., error occurred)
    }
  }
  
  return TRUE;  // All runs enumerated successfully
}

/**
 * Example usage: Creating rich text with multiple runs.
 * Shows how to create text like: "Hello bold world" where "bold" uses different formatting.
 */
void example_multirun_text(void)
{
  // Example 1: Simple single-run text (backward compatible)
  struct ViewText simpleText = {
    .run = {
      .string = "Simple text",
      .fontSize = 12,
      .fontStyle = FS_NORMAL,
      .font = NULL  // Use default font
    },
    .availableWidth = 800,
    .backingScale = 1.0f,
    .enumRunsFunc = NULL,  // NULL = single run mode
    .enumRunsData = NULL
  };
  
  // Use simpleText with Text_GetInfo, Text_Print, etc.
  
  // Example 2: Rich text with multiple runs
  TextBlock richTextBlock = {
    .runs = (struct ViewTextRun[]){
      {
        .string = "Hello ",
        .fontSize = 12,
        .fontStyle = FS_NORMAL,
        .font = NULL
      },
      {
        .string = "bold",
        .fontSize = 12,
        .fontStyle = FS_BOLD,  // Bold style
        .font = NULL
      },
      {
        .string = " world",
        .fontSize = 12,
        .fontStyle = FS_NORMAL,
        .font = NULL
      }
    },
    .runCount = 3,
    .runCapacity = 3
  };
  
  struct ViewText richText = {
    .run = richTextBlock.runs[0],  // First run is the primary run
    .availableWidth = 800,
    .backingScale = 1.0f,
    .enumRunsFunc = TextBlock_EnumRuns,  // Our enumeration function
    .enumRunsData = &richTextBlock       // Pass our TextBlock as user data
  };
  
  // Now richText can be used with Text_GetInfo, Text_Print, etc.
  // The renderer will call TextBlock_EnumRuns to get all three runs
  
  // Example 3: Rich text with mixed fonts and sizes
  extern struct font* myCustomFont;  // Assume this exists
  
  TextBlock mixedBlock = {
    .runs = (struct ViewTextRun[]){
      {
        .string = "Regular text ",
        .fontSize = 12,
        .fontStyle = FS_NORMAL,
        .font = NULL  // Default font
      },
      {
        .string = "Large heading",
        .fontSize = 24,  // Larger size
        .fontStyle = FS_BOLD,
        .font = NULL
      },
      {
        .string = " with custom font",
        .fontSize = 14,
        .fontStyle = FS_ITALIC,
        .font = myCustomFont  // Different font
      }
    },
    .runCount = 3,
    .runCapacity = 3
  };
  
  struct ViewText mixedText = {
    .run = mixedBlock.runs[0],
    .availableWidth = 800,
    .backingScale = 1.0f,
    .enumRunsFunc = TextBlock_EnumRuns,
    .enumRunsData = &mixedBlock
  };
}

/**
 * Example: Dynamic TextBlock with run management
 */
typedef struct {
  struct ViewTextRun* runs;
  int runCount;
  int runCapacity;
} DynamicTextBlock;

static void
DynamicTextBlock_Init(DynamicTextBlock* block)
{
  block->runs = NULL;
  block->runCount = 0;
  block->runCapacity = 0;
}

static void
DynamicTextBlock_AddRun(DynamicTextBlock* block, struct ViewTextRun const* run)
{
  if (block->runCount >= block->runCapacity) {
    int newCapacity = block->runCapacity == 0 ? 4 : block->runCapacity * 2;
    block->runs = realloc(block->runs, newCapacity * sizeof(struct ViewTextRun));
    block->runCapacity = newCapacity;
  }
  block->runs[block->runCount++] = *run;
}

static void
DynamicTextBlock_Free(DynamicTextBlock* block)
{
  free(block->runs);
  block->runs = NULL;
  block->runCount = 0;
  block->runCapacity = 0;
}

void example_dynamic_textblock(void)
{
  DynamicTextBlock block;
  DynamicTextBlock_Init(&block);
  
  // Build rich text dynamically
  DynamicTextBlock_AddRun(&block, &(struct ViewTextRun){
    .string = "First run ",
    .fontSize = 12,
    .fontStyle = FS_NORMAL
  });
  
  DynamicTextBlock_AddRun(&block, &(struct ViewTextRun){
    .string = "Second run",
    .fontSize = 16,
    .fontStyle = FS_BOLD
  });
  
  // Use the dynamic block for rendering
  struct ViewText text = {
    .run = block.runs[0],
    .availableWidth = 800,
    .backingScale = 1.0f,
    .enumRunsFunc = TextBlock_EnumRuns,
    .enumRunsData = &block
  };
  
  // Render the text...
  
  DynamicTextBlock_Free(&block);
}
