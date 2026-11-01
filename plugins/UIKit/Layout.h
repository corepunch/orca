// Full-range synchronous layout results, independent of pointer width. Keep
// the existing whole-point rounding and legacy packed returns (saturated above
// 65535) for source compatibility with native controls.
#ifndef ORCA_UIKIT_LAYOUT_H
#define ORCA_UIKIT_LAYOUT_H

#include <math.h>
#include <UIKit/UIKit.h>

static inline LRESULT
Layout_Result(float *resultWidth, float *resultHeight, float width, float height)
{
  bool_t outputRequested = isnan(*resultWidth) && isnan(*resultHeight);
  *resultWidth = isfinite(width) ? floorf(fmaxf(0, width)) : 0;
  *resultHeight = isfinite(height) ? floorf(fmaxf(0, height)) : 0;
  uint16_t low = (uint16_t)fminf(*resultWidth, UINT16_MAX);
  uint16_t high = (uint16_t)fminf(*resultHeight, UINT16_MAX);
  LRESULT packed = MAKEDWORD(low, high);
  // A zero-sized result is still handled; do not dispatch the base override.
  return packed || !outputRequested ? packed : TRUE;
}

#define Layout_Return(args, width, height) \
  Layout_Result(&(args)->ResultWidth, &(args)->ResultHeight, (width), (height))

static inline struct Size
Layout_SendMeasure(struct Object *object, uint32_t message, Node2D_MeasureMsgPtr args)
{
  args->ResultWidth = args->ResultHeight = NAN;
  LRESULT result = OBJ_SendMessageW(object, message, 0, args);
  return (struct Size){ isnan(args->ResultWidth) ? LOWORD(result) : args->ResultWidth,
                        isnan(args->ResultHeight) ? HIWORD(result) : args->ResultHeight };
}

static inline struct Size
Layout_SendArrange(struct Object *object, uint32_t message, Node2D_ArrangeMsgPtr args)
{
  args->ResultWidth = args->ResultHeight = NAN;
  LRESULT result = OBJ_SendMessageW(object, message, 0, args);
  return (struct Size){ isnan(args->ResultWidth) ? LOWORD(result) : args->ResultWidth,
                        isnan(args->ResultHeight) ? HIWORD(result) : args->ResultHeight };
}

#define Layout_Measure(object, ...) \
  Layout_SendMeasure((object), ID_Node2D_Measure, &(Node2D_MeasureMsg_t){ __VA_ARGS__ })
#define Layout_MeasureOverride(object, ...) \
  Layout_SendMeasure((object), ID_Node2D_MeasureOverride, &(Node2D_MeasureMsg_t){ __VA_ARGS__ })
#define Layout_Arrange(object, ...) \
  Layout_SendArrange((object), ID_Node2D_Arrange, &(Node2D_ArrangeMsg_t){ __VA_ARGS__ })
#define Layout_ArrangeOverride(object, ...) \
  Layout_SendArrange((object), ID_Node2D_ArrangeOverride, &(Node2D_ArrangeMsg_t){ __VA_ARGS__ })

#endif
