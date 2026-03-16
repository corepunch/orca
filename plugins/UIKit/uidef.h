#ifndef __UIDEF_H__
#define __UIDEF_H__

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
#define MARGIN_TOP(node2d, axis) (NODE2D_FRAME(node2d, Margin, axis)._RIGHT + NODE2D_FRAME(node2d, Border.Width, axis)._RIGHT)
#define MARGIN_BOTTOM(node2d, axis) (NODE2D_FRAME(node2d, Margin, axis)._LEFT + NODE2D_FRAME(node2d, Border.Width, axis)._LEFT)
#define PADDING_TOP(node2d, axis) (NODE2D_FRAME(node2d, Padding, axis)._RIGHT)
#define PADDING_BOTTOM(node2d, axis) (NODE2D_FRAME(node2d, Padding, axis)._LEFT)
#define TOTAL_PADDING(node2d, axis) (PADDING_TOP(node2d, axis) + PADDING_BOTTOM(node2d, axis))
#define TOTAL_MARGIN(node2d, axis) (MARGIN_TOP(node2d, axis) + MARGIN_BOTTOM(node2d, axis))

#endif
