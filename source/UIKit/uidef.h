#ifndef __UIDEF_H__
#define __UIDEF_H__

/// @typedef fixedString_t
/// @brief Fixed-length string type for UI properties
//typedef char fixedString_t[MAX_PROPERTY_STRING];

/// @typedef uiLabelSize_t
/// @brief 2D size array for UI label dimensions [width, height]
typedef int uiLabelSize_t[2];

/// @typedef uiLabelSteps_t
/// @brief Internal step-based rendering parameters for text labels
typedef longTime_t uiLabelSteps_t[4];

/// @typedef uiGridViewBuffer_t
/// @brief Internal buffer for grid layout calculations
typedef char uiGridViewBuffer_t[256];

/// @typedef actualPos_t
/// @brief 3D position array [x, y, z] for actual computed positions
typedef float actualPos_t[3];

#ifndef KANZI_SUPPORT
#define _RIGHT Left
#define _LEFT Right
#else
#define _RIGHT Right
#define _LEFT Left
#endif

#define NODE2D_FRAME(node2d, param, num) node2d->_node->param.Axis[num]
#define MARGIN_TOP(node2d, axis) (NODE2D_FRAME(node2d, Margin, axis)._RIGHT + NODE2D_FRAME(node2d, Border, axis)._RIGHT.Width)
#define MARGIN_BOTTOM(node2d, axis) (NODE2D_FRAME(node2d, Margin, axis)._LEFT + NODE2D_FRAME(node2d, Border, axis)._LEFT.Width)
#define PADDING_TOP(node2d, axis) (NODE2D_FRAME(node2d, Padding, axis)._RIGHT)
#define PADDING_BOTTOM(node2d, axis) (NODE2D_FRAME(node2d, Padding, axis)._LEFT)
#define TOTAL_PADDING(node2d, axis) (PADDING_TOP(node2d, axis) + PADDING_BOTTOM(node2d, axis))
#define TOTAL_MARGIN(node2d, axis) (MARGIN_TOP(node2d, axis) + MARGIN_BOTTOM(node2d, axis))

/// @typedef DrawBrush
/// @brief Event data structure for brush drawing operations
///
/// Contains all necessary information for rendering brush-based
/// content including projection matrix, target image, and rendering parameters.
typedef struct DrawBrush
{
  /// Projection matrix for 3D to 2D transformation
  struct mat4 const* projection;
  /// Target image handle for rendering
  handle_t image;
  /// Brush configuration to draw with
  struct BrushShorthand* brush;
  /// Border offset adjustment for rendering
  float borderOffset;
  /// Border width for each edge (top, right, bottom, left)
  struct vec4 borderWidth;
  /// True if drawing foreground, false for background
  bool_t foreground;
  /// View definition context for rendering
  struct ViewDef* viewdef;
} DRAWBRUSHSTRUCT, *EVENT_PTR(DrawBrush);

/// @def kEventMakeText
/// @brief Event constant for text generation requests
#define kEventMakeText 0x73a47798

/// @typedef MAKETEXTSTRUCT
/// @brief Event data structure for text rendering requests
typedef struct {
  /// Text view to render
  struct ViewText* text;
  /// Available space for text layout
  uint32_t availableSpace;
} MAKETEXTSTRUCT, *EVENT_PTR(MakeText);

/// @brief Create a view entity for rendering operations
/// @param object Target object for view creation
/// @param ViewEntity Output view entity structure
/// @param image Image resource for the view
/// @param brush Brush configuration for rendering
///
/// Prepares a view entity that can be used in the rendering pipeline
/// with the specified image and brush configuration.
void
Node2D_GetViewEntity(lpObject_t object,
                     struct ViewEntity* ViewEntity,
                     struct Texture const* image,
                     struct BrushShorthand const* brush);

#endif
