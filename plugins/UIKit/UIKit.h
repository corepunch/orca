// Auto-generated from UIKit.xml by tools/templates/header.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __UIKIT_H__
#define __UIKIT_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;

struct Object;
struct ViewEntity;
struct ViewText;
struct ViewDef;
struct text_info;
struct lua_State;

#include <include/renderer.h>
#include <source/core/core.h>
#include <plugins/UIKit/uidef.h>

#include "UIKit_properties.h"
#include "../../source/renderer/api/renderer.h"

ORCA_API extern struct MessageType UpdateMatrixMessage;
ORCA_API extern struct MessageType HitTestMessage;
ORCA_API extern struct MessageType GetSizeMessage;
ORCA_API extern struct MessageType SubmitMessage;
ORCA_API extern struct MessageType MeasureMessage;
ORCA_API extern struct MessageType ArrangeMessage;
ORCA_API extern struct MessageType MeasureOverrideMessage;
ORCA_API extern struct MessageType ArrangeOverrideMessage;
ORCA_API extern struct MessageType PushPropertyMessage;
ORCA_API extern struct MessageType UpdateGeometryMessage;
ORCA_API extern struct MessageType DrawBrushMessage;
ORCA_API extern struct MessageType HandleMessageMessage;
ORCA_API extern struct MessageType LoadViewMessage;
ORCA_API extern struct MessageType MakeTextMessage;
ORCA_API extern struct MessageType TriggeredMessage;
ORCA_API extern struct MessageType ForegroundContentMessage;
ORCA_API extern struct MessageType UpdateLayoutMessage;
ORCA_API extern struct MessageType NavigateToPageMessage;
ORCA_API extern struct MessageType NavigateBackMessage;

typedef struct UpdateMatrixMsgArgs UpdateMatrixMsg_t,* UpdateMatrixMsgPtr;
typedef struct HitTestMsgArgs HitTestMsg_t,* HitTestMsgPtr;
typedef struct GetSizeMsgArgs GetSizeMsg_t,* GetSizeMsgPtr;
typedef struct SubmitMsgArgs SubmitMsg_t,* SubmitMsgPtr;
typedef struct MeasureMsgArgs MeasureMsg_t,* MeasureMsgPtr;
typedef struct ArrangeMsgArgs ArrangeMsg_t,* ArrangeMsgPtr;
typedef struct MeasureMsgArgs MeasureOverrideMsg_t,* MeasureOverrideMsgPtr;
typedef struct ArrangeMsgArgs ArrangeOverrideMsg_t,* ArrangeOverrideMsgPtr;
typedef struct PushPropertyMsgArgs PushPropertyMsg_t,* PushPropertyMsgPtr;
typedef struct UpdateGeometryMsgArgs UpdateGeometryMsg_t,* UpdateGeometryMsgPtr;
typedef struct DrawBrushMsgArgs DrawBrushMsg_t,* DrawBrushMsgPtr;
typedef struct HandleMessageMsgArgs HandleMessageMsg_t,* HandleMessageMsgPtr;
typedef struct LoadViewMsgArgs LoadViewMsg_t,* LoadViewMsgPtr;
typedef struct MakeTextMsgArgs MakeTextMsg_t,* MakeTextMsgPtr;
typedef struct TriggeredMsgArgs TriggeredMsg_t,* TriggeredMsgPtr;
typedef struct ForegroundContentMsgArgs ForegroundContentMsg_t,* ForegroundContentMsgPtr;
typedef struct UpdateLayoutMsgArgs UpdateLayoutMsg_t,* UpdateLayoutMsgPtr;
typedef struct NavigateToPageMsgArgs NavigateToPageMsg_t,* NavigateToPageMsgPtr;
typedef struct NavigateBackMsgArgs NavigateBackMsg_t,* NavigateBackMsgPtr;


/// @brief Defines the primary axis for layout operations
/** Direction enum */
typedef enum Direction {
	kDirectionHorizontal, ///< Left-to-right horizontal layout
	kDirectionVertical, ///< Top-to-bottom vertical layout
	kDirectionDepth, ///< Front-to-back depth layout (3D)
} eDirection_t;
#define Direction_Count 3
ORCA_API const char *DirectionToString(enum Direction value);
ORCA_API enum Direction luaX_checkDirection(lua_State *L, int idx);
ORCA_API void luaX_pushDirection(lua_State *L, enum Direction value);

/// @brief Fields of a 3D box, used to describe position and size components
/** Box3Field enum */
typedef enum Box3Field {
	kBox3FieldX, ///< X-coordinate of the box origin
	kBox3FieldY, ///< Y-coordinate of the box origin
	kBox3FieldZ, ///< Z-coordinate of the box origin
	kBox3FieldWidth, ///< Width of the box along the X axis
	kBox3FieldHeight, ///< Height of the box along the Y axis
	kBox3FieldDepth, ///< Depth of the box along the Z axis
} eBox3Field_t;
#define Box3Field_Count 6
ORCA_API const char *Box3FieldToString(enum Box3Field value);
ORCA_API enum Box3Field luaX_checkBox3Field(lua_State *L, int idx);
ORCA_API void luaX_pushBox3Field(lua_State *L, enum Box3Field value);

/// @brief Controls how overflowing text is displayed, similar to the CSS text-overflow property
/** TextOverflow enum */
typedef enum TextOverflow {
	kTextOverflowClip, ///< Clip overflowing text at the container boundary
	kTextOverflowEllipsis, ///< Truncate overflowing text and append an ellipsis (…)
} eTextOverflow_t;
#define TextOverflow_Count 2
ORCA_API const char *TextOverflowToString(enum TextOverflow value);
ORCA_API enum TextOverflow luaX_checkTextOverflow(lua_State *L, int idx);
ORCA_API void luaX_pushTextOverflow(lua_State *L, enum TextOverflow value);

/// @brief Controls text wrapping behavior
/** TextWrapping enum */
typedef enum TextWrapping {
	kTextWrappingWrapWithOverflow, ///< Wrap at word boundaries; a word that exceeds the available width is allowed to overflow rather than being broken
	kTextWrappingNoWrap, ///< No automatic line wrapping; text extends on a single line and may overflow the container
	kTextWrappingWrap, ///< Wrap at word boundaries; a word that exceeds the available width is broken at the character level to prevent overflow
} eTextWrapping_t;
#define TextWrapping_Count 3
ORCA_API const char *TextWrappingToString(enum TextWrapping value);
ORCA_API enum TextWrapping luaX_checkTextWrapping(lua_State *L, int idx);
ORCA_API void luaX_pushTextWrapping(lua_State *L, enum TextWrapping value);

/// @brief Horizontal text alignment options within text bounds
/** TextHorizontalAlignment enum */
typedef enum TextHorizontalAlignment {
	kTextHorizontalAlignmentLeft, ///< Align text to the left edge
	kTextHorizontalAlignmentCenter, ///< Center text horizontally
	kTextHorizontalAlignmentRight, ///< Align text to the right edge
} eTextHorizontalAlignment_t;
#define TextHorizontalAlignment_Count 3
ORCA_API const char *TextHorizontalAlignmentToString(enum TextHorizontalAlignment value);
ORCA_API enum TextHorizontalAlignment luaX_checkTextHorizontalAlignment(lua_State *L, int idx);
ORCA_API void luaX_pushTextHorizontalAlignment(lua_State *L, enum TextHorizontalAlignment value);

/// @brief Vertical text alignment options within text bounds
/** TextVerticalAlignment enum */
typedef enum TextVerticalAlignment {
	kTextVerticalAlignmentTop, ///< Align text to the top
	kTextVerticalAlignmentCenter, ///< Center text vertically
	kTextVerticalAlignmentBottom, ///< Align text to the bottom
	kTextVerticalAlignmentBaseline, ///< Align to text baseline
} eTextVerticalAlignment_t;
#define TextVerticalAlignment_Count 4
ORCA_API const char *TextVerticalAlignmentToString(enum TextVerticalAlignment value);
ORCA_API enum TextVerticalAlignment luaX_checkTextVerticalAlignment(lua_State *L, int idx);
ORCA_API void luaX_pushTextVerticalAlignment(lua_State *L, enum TextVerticalAlignment value);

/// @brief Horizontal alignment of elements within their parent container
/** HorizontalAlignment enum */
typedef enum HorizontalAlignment {
	kHorizontalAlignmentStretch, ///< Stretch to fill container width
	kHorizontalAlignmentLeft, ///< Align to left edge of container
	kHorizontalAlignmentCenter, ///< Center within container
	kHorizontalAlignmentRight, ///< Align to right edge of container
} eHorizontalAlignment_t;
#define HorizontalAlignment_Count 4
ORCA_API const char *HorizontalAlignmentToString(enum HorizontalAlignment value);
ORCA_API enum HorizontalAlignment luaX_checkHorizontalAlignment(lua_State *L, int idx);
ORCA_API void luaX_pushHorizontalAlignment(lua_State *L, enum HorizontalAlignment value);

/// @brief Vertical alignment of elements within their parent container
/** VerticalAlignment enum */
typedef enum VerticalAlignment {
	kVerticalAlignmentStretch, ///< Stretch to fill container height
	kVerticalAlignmentTop, ///< Align to top edge of container
	kVerticalAlignmentCenter, ///< Center within container
	kVerticalAlignmentBottom, ///< Align to bottom edge of container
} eVerticalAlignment_t;
#define VerticalAlignment_Count 4
ORCA_API const char *VerticalAlignmentToString(enum VerticalAlignment value);
ORCA_API enum VerticalAlignment luaX_checkVerticalAlignment(lua_State *L, int idx);
ORCA_API void luaX_pushVerticalAlignment(lua_State *L, enum VerticalAlignment value);

/// @brief Cross-axis alignment in flexbox-style layout containers
/** AlignItems enum */
typedef enum AlignItems {
	kAlignItemsStart, ///< Align to start of cross axis
	kAlignItemsCenter, ///< Center on cross axis
	kAlignItemsEnd, ///< Align to end of cross axis
	kAlignItemsStretch, ///< Stretch across cross axis
	kAlignItemsBaseline, ///< Align to text baseline
} eAlignItems_t;
#define AlignItems_Count 5
ORCA_API const char *AlignItemsToString(enum AlignItems value);
ORCA_API enum AlignItems luaX_checkAlignItems(lua_State *L, int idx);
ORCA_API void luaX_pushAlignItems(lua_State *L, enum AlignItems value);

/// @brief 3D depth alignment options for positioning along Z-axis
/** DepthAlignment enum */
typedef enum DepthAlignment {
	kDepthAlignmentNear, ///< Align to near clipping plane
	kDepthAlignmentCenter, ///< Center in depth
	kDepthAlignmentFar, ///< Align to far clipping plane
	kDepthAlignmentStretch, ///< Stretch across depth
} eDepthAlignment_t;
#define DepthAlignment_Count 4
ORCA_API const char *DepthAlignmentToString(enum DepthAlignment value);
ORCA_API enum DepthAlignment luaX_checkDepthAlignment(lua_State *L, int idx);
ORCA_API void luaX_pushDepthAlignment(lua_State *L, enum DepthAlignment value);

/// @brief Content overflow handling strategies
/** Overflow enum */
typedef enum Overflow {
	kOverflowVisible, ///< Content overflows container bounds
	kOverflowHidden, ///< Clip overflowing content
	kOverflowClip, ///< Hard clipping without scrollbars
	kOverflowScroll, ///< Show scrollbars for overflow
	kOverflowAuto, ///< Automatic scrollbar behavior
} eOverflow_t;
#define Overflow_Count 5
ORCA_API const char *OverflowToString(enum Overflow value);
ORCA_API enum Overflow luaX_checkOverflow(lua_State *L, int idx);
ORCA_API void luaX_pushOverflow(lua_State *L, enum Overflow value);
/** ForegroundHint enum */
typedef enum ForegroundHint {
	kForegroundHintNone, ///< No special foreground treatment
	kForegroundHintTranslucent, ///< Hint for translucent foreground
	kForegroundHintOpaque, ///< Hint for opaque foreground optimization
} eForegroundHint_t;
#define ForegroundHint_Count 3
ORCA_API const char *ForegroundHintToString(enum ForegroundHint value);
ORCA_API enum ForegroundHint luaX_checkForegroundHint(lua_State *L, int idx);
ORCA_API void luaX_pushForegroundHint(lua_State *L, enum ForegroundHint value);

/// @brief Main-axis content distribution in flexbox-style layouts
/** JustifyContent enum */
typedef enum JustifyContent {
	kJustifyContentStart, ///< Pack items to start
	kJustifyContentEnd, ///< Pack items to end
	kJustifyContentCenter, ///< Center items
	kJustifyContentSpaceBetween, ///< Distribute with space between
	kJustifyContentSpaceAround, ///< Distribute with space around
	kJustifyContentSpaceEvenly, ///< Distribute with even spacing
} eJustifyContent_t;
#define JustifyContent_Count 6
ORCA_API const char *JustifyContentToString(enum JustifyContent value);
ORCA_API enum JustifyContent luaX_checkJustifyContent(lua_State *L, int idx);
ORCA_API void luaX_pushJustifyContent(lua_State *L, enum JustifyContent value);

/// @brief Font weight options for typography
/** FontWeight enum */
typedef enum FontWeight {
	kFontWeightNormal, ///< Standard font weight
	kFontWeightBold, ///< Bold font weight
} eFontWeight_t;
#define FontWeight_Count 2
ORCA_API const char *FontWeightToString(enum FontWeight value);
ORCA_API enum FontWeight luaX_checkFontWeight(lua_State *L, int idx);
ORCA_API void luaX_pushFontWeight(lua_State *L, enum FontWeight value);

/// @brief Font style options for typography
/** FontStyle enum */
typedef enum FontStyle {
	kFontStyleNormal, ///< Regular font style
	kFontStyleItalic, ///< Italic font style
} eFontStyle_t;
#define FontStyle_Count 2
ORCA_API const char *FontStyleToString(enum FontStyle value);
ORCA_API enum FontStyle luaX_checkFontStyle(lua_State *L, int idx);
ORCA_API void luaX_pushFontStyle(lua_State *L, enum FontStyle value);

/// @brief Visual styles for element borders
/** BorderStyle enum */
typedef enum BorderStyle {
	kBorderStyleNone, ///< No border
	kBorderStyleHidden, ///< Hidden border (takes space but invisible)
	kBorderStyleDotted, ///< Dotted border pattern
	kBorderStyleDashed, ///< Dashed border pattern
	kBorderStyleSolid, ///< Solid continuous border
	kBorderStyleDouble, ///< Double line border
	kBorderStyleGroove, ///< 3D grooved border effect
	kBorderStyleRidge, ///< 3D ridged border effect
	kBorderStyleInset, ///< 3D inset border effect
	kBorderStyleOutset, ///< 3D outset border effect
} eBorderStyle_t;
#define BorderStyle_Count 10
ORCA_API const char *BorderStyleToString(enum BorderStyle value);
ORCA_API enum BorderStyle luaX_checkBorderStyle(lua_State *L, int idx);
ORCA_API void luaX_pushBorderStyle(lua_State *L, enum BorderStyle value);

/// @brief Animation playback behavior modes
/** PlaybackMode enum */
typedef enum PlaybackMode {
	kPlaybackModeNormal, ///< Standard forward playback
	kPlaybackModeReverse, ///< Reverse playback from end to start
	kPlaybackModePingpong, ///< Alternating forward/reverse playback
} ePlaybackMode_t;
#define PlaybackMode_Count 3
ORCA_API const char *PlaybackModeToString(enum PlaybackMode value);
ORCA_API enum PlaybackMode luaX_checkPlaybackMode(lua_State *L, int idx);
ORCA_API void luaX_pushPlaybackMode(lua_State *L, enum PlaybackMode value);

/// @brief Button behavior types for different interaction patterns
/** ButtonType enum */
typedef enum ButtonType {
	kButtonTypeNormal, ///< Standard interactive button
	kButtonTypeSubmit, ///< Form submission button with special handling
} eButtonType_t;
#define ButtonType_Count 2
ORCA_API const char *ButtonTypeToString(enum ButtonType value);
ORCA_API enum ButtonType luaX_checkButtonType(lua_State *L, int idx);
ORCA_API void luaX_pushButtonType(lua_State *L, enum ButtonType value);

/// @brief Input control behavior types for different data entry needs
/** InputType enum */
typedef enum InputType {
	kInputTypeText, ///< Single or multi-line text input 
	kInputTypePassword, ///< Masked password input for security 
	kInputTypeNumber, ///< Numeric input with validation 
	kInputTypeCheckbox, ///< Boolean checkbox control 
	kInputTypeRadio, ///< Radio button for exclusive selection 
} eInputType_t;
#define InputType_Count 5
ORCA_API const char *InputTypeToString(enum InputType value);
ORCA_API enum InputType luaX_checkInputType(lua_State *L, int idx);
ORCA_API void luaX_pushInputType(lua_State *L, enum InputType value);

/// @brief Size calculation modes for layout operations
/** Sizing enum */
typedef enum Sizing {
	kSizingPlusMargin, ///< Include margins in size calculation
	kSizingSizeOnly, ///< Return only the base size
	kSizingMinusPadding, ///< Subtract padding from size
} eSizing_t;
#define Sizing_Count 3
ORCA_API const char *SizingToString(enum Sizing value);
ORCA_API enum Sizing luaX_checkSizing(lua_State *L, int idx);
ORCA_API void luaX_pushSizing(lua_State *L, enum Sizing value);

/// @brief Defines how the window can be resized by the user or system.
/** ResizeMode enum */
typedef enum ResizeMode {
	kResizeModeCanResize, ///< The user can freely resize the window and maximize it.
	kResizeModeCanMinimize, ///< The user can minimize the window, but resizing and maximizing are disabled.
	kResizeModeCanResizeWithGrip, ///< The user can resize and maximize the window. A resize grip is displayed in the lower-right corner if the platform supports it.
	kResizeModeNoResize, ///< The window size is fixed. The user cannot resize or maximize the window.
} eResizeMode_t;
#define ResizeMode_Count 4
ORCA_API const char *ResizeModeToString(enum ResizeMode value);
ORCA_API enum ResizeMode luaX_checkResizeMode(lua_State *L, int idx);
ORCA_API void luaX_pushResizeMode(lua_State *L, enum ResizeMode value);

/// @brief Specifies how the content of an element is resized to fill its layout area
/** Stretch enum */
typedef enum Stretch {
	kStretchUniform, ///< Scale content uniformly to fit within the area, preserving aspect ratio
	kStretchNone, ///< Do not scale the content; display at its native size
	kStretchFill, ///< Scale content to fill both width and height, ignoring aspect ratio
	kStretchUniformToFill, ///< Scale content uniformly to fill the area, preserving aspect ratio but cropping if necessary
} eStretch_t;
#define Stretch_Count 4
ORCA_API const char *StretchToString(enum Stretch value);
ORCA_API enum Stretch luaX_checkStretch(lua_State *L, int idx);
ORCA_API void luaX_pushStretch(lua_State *L, enum Stretch value);

/// @brief Specifies the type of transition animation to use when navigating between pages.
/** TransitionType enum */
typedef enum TransitionType {
	kTransitionTypeNone, ///< No transition
	kTransitionTypeSlide, ///< Sliding transition
	kTransitionTypeFade, ///< Fading transition
} eTransitionType_t;
#define TransitionType_Count 3
ORCA_API const char *TransitionTypeToString(enum TransitionType value);
ORCA_API enum TransitionType luaX_checkTransitionType(lua_State *L, int idx);
ORCA_API void luaX_pushTransitionType(lua_State *L, enum TransitionType value);

/// @brief Specifies the classification of a style and how it is applied to components.
/** StyleType enum */
typedef enum StyleType {
	kStyleTypeGeneric, ///< Base style applied to all components of the specified target type. Serves as a default visual definition.
	kStyleTypeNamed, ///< Style associated with a specific theme or visual skin. Used to switch appearance dynamically based on active theme.
} eStyleType_t;
#define StyleType_Count 2
ORCA_API const char *StyleTypeToString(enum StyleType value);
ORCA_API enum StyleType luaX_checkStyleType(lua_State *L, int idx);
ORCA_API void luaX_pushStyleType(lua_State *L, enum StyleType value);

typedef struct CornerRadius CornerRadius_t, *lpCornerRadius_t;
typedef struct CornerRadius const cCornerRadius_t, *lpcCornerRadius_t;
typedef struct EdgeShorthand EdgeShorthand_t, *lpEdgeShorthand_t;
typedef struct EdgeShorthand const cEdgeShorthand_t, *lpcEdgeShorthand_t;
typedef struct AlignmentShorthand AlignmentShorthand_t, *lpAlignmentShorthand_t;
typedef struct AlignmentShorthand const cAlignmentShorthand_t, *lpcAlignmentShorthand_t;
typedef struct FontShorthand FontShorthand_t, *lpFontShorthand_t;
typedef struct FontShorthand const cFontShorthand_t, *lpcFontShorthand_t;
typedef struct BrushShorthand BrushShorthand_t, *lpBrushShorthand_t;
typedef struct BrushShorthand const cBrushShorthand_t, *lpcBrushShorthand_t;
typedef struct ShadowShorthand ShadowShorthand_t, *lpShadowShorthand_t;
typedef struct ShadowShorthand const cShadowShorthand_t, *lpcShadowShorthand_t;
typedef struct RingShorthand RingShorthand_t, *lpRingShorthand_t;
typedef struct RingShorthand const cRingShorthand_t, *lpcRingShorthand_t;
typedef struct OverflowShorthand OverflowShorthand_t, *lpOverflowShorthand_t;
typedef struct OverflowShorthand const cOverflowShorthand_t, *lpcOverflowShorthand_t;
typedef struct UnderlineShorthand UnderlineShorthand_t, *lpUnderlineShorthand_t;
typedef struct UnderlineShorthand const cUnderlineShorthand_t, *lpcUnderlineShorthand_t;
typedef struct Thickness Thickness_t, *lpThickness_t;
typedef struct Thickness const cThickness_t, *lpcThickness_t;
typedef struct BorderShorthand BorderShorthand_t, *lpBorderShorthand_t;
typedef struct BorderShorthand const cBorderShorthand_t, *lpcBorderShorthand_t;
typedef struct SizeAxisShorthand SizeAxisShorthand_t, *lpSizeAxisShorthand_t;
typedef struct SizeAxisShorthand const cSizeAxisShorthand_t, *lpcSizeAxisShorthand_t;
typedef struct SizeShorthand SizeShorthand_t, *lpSizeShorthand_t;
typedef struct SizeShorthand const cSizeShorthand_t, *lpcSizeShorthand_t;



/// @brief Corner rounding configuration for rectangular elements
/** CornerRadius struct */
struct CornerRadius {
	float TopLeftRadius; ///< Top-left corner radius
	float TopRightRadius; ///< Top-right corner radius
	float BottomRightRadius; ///< Bottom-right corner radius
	float BottomLeftRadius; ///< Bottom-left corner radius
};
ORCA_API void luaX_pushCornerRadius(lua_State *L, struct CornerRadius const* CornerRadius);
ORCA_API struct CornerRadius* luaX_checkCornerRadius(lua_State *L, int idx);
/// @brief Spacing values for opposite edges of an axis
/** EdgeShorthand struct */
struct EdgeShorthand {
	float Left; ///< Leading edge value (left for horizontal, top for vertical, near for depth)
	float Right; ///< Trailing edge value (right for horizontal, bottom for vertical, far for depth)
};
ORCA_API void luaX_pushEdgeShorthand(lua_State *L, struct EdgeShorthand const* EdgeShorthand);
ORCA_API struct EdgeShorthand* luaX_checkEdgeShorthand(lua_State *L, int idx);
/// @brief Axis-specific alignment parameters.
/** AlignmentShorthand struct */
struct AlignmentShorthand {
	int32_t Axis[3]; ///< Alignment of the element within its parent (similar to HorizontalAlignment/VerticalAlignment in XAML).
};
ORCA_API void luaX_pushAlignmentShorthand(lua_State *L, struct AlignmentShorthand const* AlignmentShorthand);
ORCA_API struct AlignmentShorthand* luaX_checkAlignmentShorthand(lua_State *L, int idx);
/// @brief Complete font specification including weight, style, size, and family
/** FontShorthand struct */
struct FontShorthand {
	enum FontWeight Weight; ///< Font weight (normal or bold)
	enum FontStyle Style; ///< Font style (normal or italic)
	float Size; ///< Font size in points
	struct FontFamily* Family; ///< Font family reference
};
ORCA_API void luaX_pushFontShorthand(lua_State *L, struct FontShorthand const* FontShorthand);
ORCA_API struct FontShorthand* luaX_checkFontShorthand(lua_State *L, int idx);
/// @brief Represents a generic brush used for filling UI elements.
/** BrushShorthand struct */
struct BrushShorthand {
	struct color Color; ///< Solid color fill.
	struct Texture* Image; ///< Texture/image fill.
	struct Material* Material; ///< Custom material or shader fill. Advanced fill using shader/material settings for effects beyond simple color or image.
};
ORCA_API void luaX_pushBrushShorthand(lua_State *L, struct BrushShorthand const* BrushShorthand);
ORCA_API struct BrushShorthand* luaX_checkBrushShorthand(lua_State *L, int idx);
/// @brief Drop shadow effect configuration
/** ShadowShorthand struct */
struct ShadowShorthand {
	struct vec2 Offset; ///< Shadow offset from element (x, y)
	float BlurRadius; ///< Blur amount for shadow softness
	float SpreadRadius; ///< Shadow expansion beyond element bounds
	struct color Color; ///< Shadow color and opacity
};
ORCA_API void luaX_pushShadowShorthand(lua_State *L, struct ShadowShorthand const* ShadowShorthand);
ORCA_API struct ShadowShorthand* luaX_checkShadowShorthand(lua_State *L, int idx);
/// @brief Decorative ring/outline effect around elements
/** RingShorthand struct */
struct RingShorthand {
	float Offset; ///< Distance from element edge to ring
	float Width; ///< Ring thickness in pixels
	struct color Color; ///< Ring color and opacity
};
ORCA_API void luaX_pushRingShorthand(lua_State *L, struct RingShorthand const* RingShorthand);
ORCA_API struct RingShorthand* luaX_checkRingShorthand(lua_State *L, int idx);
/// @brief Per-axis overflow behavior configuration
/** OverflowShorthand struct */
struct OverflowShorthand {
	enum Overflow x; ///< Horizontal overflow handling
	enum Overflow y; ///< Vertical overflow handling
};
ORCA_API void luaX_pushOverflowShorthand(lua_State *L, struct OverflowShorthand const* OverflowShorthand);
ORCA_API struct OverflowShorthand* luaX_checkOverflowShorthand(lua_State *L, int idx);
/// @brief Defines shorthand underline properties (offset, width, and color).
/** UnderlineShorthand struct */
struct UnderlineShorthand {
	float Offset; ///< Vertical offset of the underline from the baseline.
	float Width; ///< Line thickness of the underline.
	struct color Color; ///< Color of the underline.
};
ORCA_API void luaX_pushUnderlineShorthand(lua_State *L, struct UnderlineShorthand const* UnderlineShorthand);
ORCA_API struct UnderlineShorthand* luaX_checkUnderlineShorthand(lua_State *L, int idx);
/// @brief External spacing configuration for rectangular elements
/** Thickness struct */
struct Thickness {
	struct EdgeShorthand Axis[3]; ///< External spacing per axis
};
ORCA_API void luaX_pushThickness(lua_State *L, struct Thickness const* Thickness);
ORCA_API struct Thickness* luaX_checkThickness(lua_State *L, int idx);
/// @brief Complete border definition for rectangular element
/** BorderShorthand struct */
struct BorderShorthand {
	struct Thickness Width; ///< Border thickness in pixels, specified for each edge
	struct color Color; ///< Border color
	enum BorderStyle Style; ///< Visual style of the border
	struct CornerRadius Radius; ///< Border radius definition per corner
};
ORCA_API void luaX_pushBorderShorthand(lua_State *L, struct BorderShorthand const* BorderShorthand);
ORCA_API struct BorderShorthand* luaX_checkBorderShorthand(lua_State *L, int idx);
/// @brief Complete sizing definition for one axis
/** SizeAxisShorthand struct */
struct SizeAxisShorthand {
	float Requested; ///< Requested {0} size in pixels
	float Desired; ///< Desired {0} size in pixels
	float Min; ///< Minimum allowable {0} size along
	float Actual; ///< Actual (calculated) {0} size after layout resolution
	float Scroll; ///< Total {0} size required for scrolling content
};
ORCA_API void luaX_pushSizeAxisShorthand(lua_State *L, struct SizeAxisShorthand const* SizeAxisShorthand);
ORCA_API struct SizeAxisShorthand* luaX_checkSizeAxisShorthand(lua_State *L, int idx);
/// @brief Sizing configuration of elements (e.g., Width, Height, Depth)
/** SizeShorthand struct */
struct SizeShorthand {
	struct SizeAxisShorthand Axis[3]; ///< A number value per axis
};
ORCA_API void luaX_pushSizeShorthand(lua_State *L, struct SizeShorthand const* SizeShorthand);
ORCA_API struct SizeShorthand* luaX_checkSizeShorthand(lua_State *L, int idx);

/** UpdateMatrixMsgArgs struct */
struct UpdateMatrixMsgArgs {
	struct mat4 parent; ///< The parent matrix
	float opacity; ///< The opacity value
	bool_t force; ///< Indicates if the update is forced
};
ORCA_API void luaX_pushUpdateMatrixMsgArgs(lua_State *L, struct UpdateMatrixMsgArgs const* data);
ORCA_API struct UpdateMatrixMsgArgs* luaX_checkUpdateMatrixMsgArgs(lua_State *L, int idx);
/** HitTestMsgArgs struct */
struct HitTestMsgArgs {
	int32_t x; ///< X coordinate of the hit test point
	int32_t y; ///< Y coordinate of the hit test point
};
ORCA_API void luaX_pushHitTestMsgArgs(lua_State *L, struct HitTestMsgArgs const* data);
ORCA_API struct HitTestMsgArgs* luaX_checkHitTestMsgArgs(lua_State *L, int idx);
/** GetSizeMsgArgs struct */
struct GetSizeMsgArgs {
};
ORCA_API void luaX_pushGetSizeMsgArgs(lua_State *L, struct GetSizeMsgArgs const* data);
ORCA_API struct GetSizeMsgArgs* luaX_checkGetSizeMsgArgs(lua_State *L, int idx);
/** SubmitMsgArgs struct */
struct SubmitMsgArgs {
};
ORCA_API void luaX_pushSubmitMsgArgs(lua_State *L, struct SubmitMsgArgs const* data);
ORCA_API struct SubmitMsgArgs* luaX_checkSubmitMsgArgs(lua_State *L, int idx);
/** MeasureMsgArgs struct */
struct MeasureMsgArgs {
	float Width;
	float Height;
};
ORCA_API void luaX_pushMeasureMsgArgs(lua_State *L, struct MeasureMsgArgs const* data);
ORCA_API struct MeasureMsgArgs* luaX_checkMeasureMsgArgs(lua_State *L, int idx);
/** ArrangeMsgArgs struct */
struct ArrangeMsgArgs {
	float X;
	float Y;
	float Width;
	float Height;
};
ORCA_API void luaX_pushArrangeMsgArgs(lua_State *L, struct ArrangeMsgArgs const* data);
ORCA_API struct ArrangeMsgArgs* luaX_checkArrangeMsgArgs(lua_State *L, int idx);
/** PushPropertyMsgArgs struct */
struct PushPropertyMsgArgs {
	int32_t Placeholder;
};
ORCA_API void luaX_pushPushPropertyMsgArgs(lua_State *L, struct PushPropertyMsgArgs const* data);
ORCA_API struct PushPropertyMsgArgs* luaX_checkPushPropertyMsgArgs(lua_State *L, int idx);
/** UpdateGeometryMsgArgs struct */
struct UpdateGeometryMsgArgs {
};
ORCA_API void luaX_pushUpdateGeometryMsgArgs(lua_State *L, struct UpdateGeometryMsgArgs const* data);
ORCA_API struct UpdateGeometryMsgArgs* luaX_checkUpdateGeometryMsgArgs(lua_State *L, int idx);
/** DrawBrushMsgArgs struct */
struct DrawBrushMsgArgs {
	struct mat4 projection; ///< Projection matrix for 3D to 2D transformation
	struct Texture* image; ///< Target image handle for rendering
	struct BrushShorthand brush; ///< Brush configuration to draw with
	float borderOffset; ///< Border offset adjustment for rendering
	struct vec4 borderWidth; ///< Border width for each edge (top, right, bottom, left)
	bool_t foreground; ///< True if drawing foreground, false for background
	struct ViewDef* viewdef; ///< View definition context for rendering
};
ORCA_API void luaX_pushDrawBrushMsgArgs(lua_State *L, struct DrawBrushMsgArgs const* data);
ORCA_API struct DrawBrushMsgArgs* luaX_checkDrawBrushMsgArgs(lua_State *L, int idx);
/** HandleMessageMsgArgs struct */
struct HandleMessageMsgArgs {
	const char* EventName;
	uint32_t FirstArg;
	uint32_t NumArgs;
};
ORCA_API void luaX_pushHandleMessageMsgArgs(lua_State *L, struct HandleMessageMsgArgs const* data);
ORCA_API struct HandleMessageMsgArgs* luaX_checkHandleMessageMsgArgs(lua_State *L, int idx);
/** LoadViewMsgArgs struct */
struct LoadViewMsgArgs {
	struct lua_State* lua_state;
};
ORCA_API void luaX_pushLoadViewMsgArgs(lua_State *L, struct LoadViewMsgArgs const* data);
ORCA_API struct LoadViewMsgArgs* luaX_checkLoadViewMsgArgs(lua_State *L, int idx);
/** MakeTextMsgArgs struct */
struct MakeTextMsgArgs {
	struct ViewText* text; ///< Text view to render
	uint32_t availableSpace; ///< Available space for text layout
};
ORCA_API void luaX_pushMakeTextMsgArgs(lua_State *L, struct MakeTextMsgArgs const* data);
ORCA_API struct MakeTextMsgArgs* luaX_checkMakeTextMsgArgs(lua_State *L, int idx);
/** TriggeredMsgArgs struct */
struct TriggeredMsgArgs {
	struct Trigger* Trigger;
	struct HandleMessageMsgArgs message;
};
ORCA_API void luaX_pushTriggeredMsgArgs(lua_State *L, struct TriggeredMsgArgs const* data);
ORCA_API struct TriggeredMsgArgs* luaX_checkTriggeredMsgArgs(lua_State *L, int idx);
/** ForegroundContentMsgArgs struct */
struct ForegroundContentMsgArgs {
};
ORCA_API void luaX_pushForegroundContentMsgArgs(lua_State *L, struct ForegroundContentMsgArgs const* data);
ORCA_API struct ForegroundContentMsgArgs* luaX_checkForegroundContentMsgArgs(lua_State *L, int idx);
/** UpdateLayoutMsgArgs struct */
struct UpdateLayoutMsgArgs {
	float Width;
	float Height;
};
ORCA_API void luaX_pushUpdateLayoutMsgArgs(lua_State *L, struct UpdateLayoutMsgArgs const* data);
ORCA_API struct UpdateLayoutMsgArgs* luaX_checkUpdateLayoutMsgArgs(lua_State *L, int idx);
/** NavigateToPageMsgArgs struct */
struct NavigateToPageMsgArgs {
	const char* URL; ///< The URL of the page to navigate to.
	enum TransitionType TransitionType; ///< The type of transition animation to use during navigation.
};
ORCA_API void luaX_pushNavigateToPageMsgArgs(lua_State *L, struct NavigateToPageMsgArgs const* data);
ORCA_API struct NavigateToPageMsgArgs* luaX_checkNavigateToPageMsgArgs(lua_State *L, int idx);
/** NavigateBackMsgArgs struct */
struct NavigateBackMsgArgs {
	enum TransitionType TransitionType; ///< The type of transition animation to use during navigation.
};
ORCA_API void luaX_pushNavigateBackMsgArgs(lua_State *L, struct NavigateBackMsgArgs const* data);
ORCA_API struct NavigateBackMsgArgs* luaX_checkNavigateBackMsgArgs(lua_State *L, int idx);


/// @brief Base class for data-holding objects within the framework
/** DataObject component */
typedef struct DataObject DataObject_t, *DataObjectPtr, *lpDataObject_t;
typedef struct DataObject const *DataObjectCPtr, *lpcDataObject_t;
struct DataObject {
};
ORCA_API void luaX_pushDataObject(lua_State *L, struct DataObject const* DataObject);
ORCA_API struct DataObject* luaX_checkDataObject(lua_State *L, int idx);

/// @brief Controls playback of animations within the UI engine.
/** AnimationPlayer component */
typedef struct AnimationPlayer AnimationPlayer_t, *AnimationPlayerPtr, *lpAnimationPlayer_t;
typedef struct AnimationPlayer const *AnimationPlayerCPtr, *lpcAnimationPlayer_t;
struct AnimationPlayer {
	bool_t AutoplayEnabled; ///< Enables or disables automatic playback on initialization.
	float DurationScale; ///< Scales the playback speed of the animation. A value greater than 1.0 speeds up playback, while values between 0.0 and 1.0 slow it down.
	enum PlaybackMode PlaybackMode; ///< Defines how the animation should be played.
	bool_t RelativePlayback; ///< When true, playback is relative to the current state rather than starting from an absolute base state.
	int32_t RepeatCount; ///< Number of times the animation should repeat. Use -1 (or another sentinel value, depending on implementation) to indicate infinite looping.
	bool_t RestoreOriginalValuesAfterPlayback; ///< Restores the original property values after the animation ends.
	struct Timeline* Timeline; ///< Reference to the animation timeline that defines keyframes and transitions.
};
ORCA_API void luaX_pushAnimationPlayer(lua_State *L, struct AnimationPlayer const* AnimationPlayer);
ORCA_API struct AnimationPlayer* luaX_checkAnimationPlayer(lua_State *L, int idx);

/// @brief Base class for event-driven actions within the UI system
/** Trigger component */
typedef struct Trigger Trigger_t, *TriggerPtr, *lpTrigger_t;
typedef struct Trigger const *TriggerCPtr, *lpcTrigger_t;
struct Trigger {
	const char* Property; ///< Target property name to monitor or modify
	int32_t Value; ///< Associated value for the trigger condition
};
ORCA_API void luaX_pushTrigger(lua_State *L, struct Trigger const* Trigger);
ORCA_API struct Trigger* luaX_checkTrigger(lua_State *L, int idx);

/// @brief Responds to property value changes on specified nodes
/** OnPropertyChangedTrigger component */
typedef struct OnPropertyChangedTrigger OnPropertyChangedTrigger_t, *OnPropertyChangedTriggerPtr, *lpOnPropertyChangedTrigger_t;
typedef struct OnPropertyChangedTrigger const *OnPropertyChangedTriggerCPtr, *lpcOnPropertyChangedTrigger_t;
struct OnPropertyChangedTrigger {
	const char* SourceNode; ///< Name or identifier of the node to monitor
	const char* Property; ///< Property name to watch for changes
};
ORCA_API void luaX_pushOnPropertyChangedTrigger(lua_State *L, struct OnPropertyChangedTrigger const* OnPropertyChangedTrigger);
ORCA_API struct OnPropertyChangedTrigger* luaX_checkOnPropertyChangedTrigger(lua_State *L, int idx);

/// @brief Triggers when an element is attached to the visual tree
/** OnAttachedTrigger component */
typedef struct OnAttachedTrigger OnAttachedTrigger_t, *OnAttachedTriggerPtr, *lpOnAttachedTrigger_t;
typedef struct OnAttachedTrigger const *OnAttachedTriggerCPtr, *lpcOnAttachedTrigger_t;
struct OnAttachedTrigger {
};
ORCA_API void luaX_pushOnAttachedTrigger(lua_State *L, struct OnAttachedTrigger const* OnAttachedTrigger);
ORCA_API struct OnAttachedTrigger* luaX_checkOnAttachedTrigger(lua_State *L, int idx);

/// @brief Responds to routed UI events (mouse, keyboard, etc.)
/** EventTrigger component */
typedef struct EventTrigger EventTrigger_t, *EventTriggerPtr, *lpEventTrigger_t;
typedef struct EventTrigger const *EventTriggerCPtr, *lpcEventTrigger_t;
struct EventTrigger {
	const char* RoutedEvent; ///< Name of the routed event to handle (e.g., "MouseDown", "KeyPress")
};
ORCA_API void luaX_pushEventTrigger(lua_State *L, struct EventTrigger const* EventTrigger);
ORCA_API struct EventTrigger* luaX_checkEventTrigger(lua_State *L, int idx);

/// @brief Applies property values when triggered by specified conditions
/** Setter component */
typedef struct Setter Setter_t, *SetterPtr, *lpSetter_t;
typedef struct Setter const *SetterCPtr, *lpcSetter_t;
struct Setter {
	struct Trigger* Trigger; ///< Triggering condition or state image
	const char* Property; ///< Target property name to set
	const char* Value; ///< Value to apply to the property
};
ORCA_API void luaX_pushSetter(lua_State *L, struct Setter const* Setter);
ORCA_API struct Setter* luaX_checkSetter(lua_State *L, int idx);

/// @brief Executes custom functions when triggered by specified conditions
/** Handler component */
typedef struct Handler Handler_t, *HandlerPtr, *lpHandler_t;
typedef struct Handler const *HandlerCPtr, *lpcHandler_t;
struct Handler {
	struct Trigger* Trigger; ///< Triggering condition or state image
	struct Node* Target; ///< Target object to call function on
	const char* Function; ///< Function name to execute on target
};
ORCA_API void luaX_pushHandler(lua_State *L, struct Handler const* Handler);
ORCA_API struct Handler* luaX_checkHandler(lua_State *L, int idx);

/// @brief Base class for all brush types used in rendering.
/** Brush component */
typedef struct Brush Brush_t, *BrushPtr, *lpBrush_t;
typedef struct Brush const *BrushCPtr, *lpcBrush_t;
struct Brush {
};
ORCA_API void luaX_pushBrush(lua_State *L, struct Brush const* Brush);
ORCA_API struct Brush* luaX_checkBrush(lua_State *L, int idx);

/// @brief Brush that fills an area with a solid color.
/** ColorBrush component */
typedef struct ColorBrush ColorBrush_t, *ColorBrushPtr, *lpColorBrush_t;
typedef struct ColorBrush const *ColorBrushCPtr, *lpcColorBrush_t;
struct ColorBrush {
	struct color Color; ///< The color value used to fill the target area.
};
ORCA_API void luaX_pushColorBrush(lua_State *L, struct ColorBrush const* ColorBrush);
ORCA_API struct ColorBrush* luaX_checkColorBrush(lua_State *L, int idx);

/// @brief Base class for all UI engine nodes.
/** Node component */
typedef struct Node Node_t, *NodePtr, *lpNode_t;
typedef struct Node const *NodeCPtr, *lpcNode_t;
struct Node {
	struct SizeShorthand Size; ///< Sizing configuration of this element
	struct Thickness Margin; ///< External spacing around the element
	struct Thickness Padding; ///< Internal spacing between element border and content
	struct BorderShorthand Border; ///< Border definition along each axis
	struct AlignmentShorthand Alignment; ///< Alignment definition for each axis.
	bool_t Visible; ///< Whether this node is visible in the scene.
	bool_t QuickHide; ///< If true, node is quickly hidden without triggering full visibility transitions.
	float VisibleAmountInParent; ///< Specifies how much of the node is visible within its parent container. Value in range [0.0, 1.0].
	float Opacity; ///< Opacity of the node, range [0.0 = transparent, 1.0 = fully opaque].
	const char* Tags; ///< Tag collection for categorizing or querying nodes.
	struct DataObject* DataContext; ///< Data context (used for data binding, similar to XAML's DataContext).
	long _tags; ///< Calculated tags value
};
ORCA_API void luaX_pushNode(lua_State *L, struct Node const* Node);
ORCA_API struct Node* luaX_checkNode(lua_State *L, int idx);

/// @brief Shared concept for text rendering in 2D and 3D nodes.
/** TextRun component */
typedef struct TextRun TextRun_t, *TextRunPtr, *lpTextRun_t;
typedef struct TextRun const *TextRunCPtr, *lpcTextRun_t;
struct TextRun {
	const char* Text; ///< The short text string displayed in the block, limited to 64 bytes. If you need more space consider using `TextResourceID`.
	struct FontShorthand Font; ///< Font definition used for text rendering. Contains typeface, size, and style information.
	struct UnderlineShorthand Underline; ///< Underline style applied to the text.
	float LetterSpacing; ///< Additional spacing applied between letters.
	float LineHeight; ///< Line height multiplier for multi-line text layout.
	float CharacterSpacing; ///< Extra spacing between characters, beyond kerning.
	float FixedCharacterWidth; ///< Forces a fixed width per character (monospace effect).
	bool_t RemoveSideBearingsProperty; ///< Removes side bearings (spacing) defined by the font.
	struct text_info _textinfo; ///< Internal text information structure.
	struct FontShorthand* _font; ///< Reference to the font shorthand configuration.
};
ORCA_API void luaX_pushTextRun(lua_State *L, struct TextRun const* TextRun);
ORCA_API struct TextRun* luaX_checkTextRun(lua_State *L, int idx);

/// @brief Shared concept for text rendering in 2D and 3D nodes.
/** TextBlockConcept component */
typedef struct TextBlockConcept TextBlockConcept_t, *TextBlockConceptPtr, *lpTextBlockConcept_t;
typedef struct TextBlockConcept const *TextBlockConceptCPtr, *lpcTextBlockConcept_t;
struct TextBlockConcept {
	const char* TextResourceID; ///< Resource identifier for localized text lookup.
	const char* TextResourceConfiguration; ///< Configuration key used when resolving text resources.
	const char* PlaceholderText; ///< Placeholder text displayed when no main text is set.
	enum TextOverflow TextOverflow; ///< Defines how overflowing text should be handled.
	struct BrushShorthand Placeholder; ///< Brush definition for rendering placeholder text.
	bool_t UseFullFontHeight; ///< When true, uses the font's full height for layout calculations.
	bool_t ConstrainContentHeight; ///< Constrains the content height to the text's bounding box.
	bool_t WordWrap; ///< Enables automatic word wrapping of the text.
	enum TextWrapping TextWrapping; ///< Controls text wrapping behavior.
	enum TextHorizontalAlignment TextHorizontalAlignment; ///< Horizontal alignment of the text within its bounds.
	enum TextVerticalAlignment TextVerticalAlignment; ///< Vertical alignment of the text within its bounds.
	uiLabelSteps_t _steps; ///< Internal step-based rendering parameters.
	struct Node* _node; ///< Reference to the owning node using this concept.
	struct ViewText* _text; ///< Internal view representation of the text.
};
ORCA_API void luaX_pushTextBlockConcept(lua_State *L, struct TextBlockConcept const* TextBlockConcept);
ORCA_API struct TextBlockConcept* luaX_checkTextBlockConcept(lua_State *L, int idx);

/// @brief Primary class for 2D UI elements with comprehensive rendering capabilities
/** Node2D component */
typedef struct Node2D Node2D_t, *Node2DPtr, *lpNode2D_t;
typedef struct Node2D const *Node2DCPtr, *lpcNode2D_t;
struct Node2D {
	struct transform2 LayoutTransform; ///< Transformation applied during layout calculations. Affects size and positioning but does not influence local rendering.
	struct transform2 RenderTransform; ///< Transformation applied at render time. Affects how the element is drawn (rotation, scale, skew, etc.) without altering layout.
	struct vec2 RenderTransformOrigin; ///< The origin point for applying RenderTransform. Defines the pivot point relative to the element.
	struct vec2 ContentOffset; ///< Pixel offset applied to the element's content. Can be used for subpixel positioning or scrolling adjustments.
	struct mat4 Matrix; ///< Final transformation matrix applied to the node. Computed result of layout, render, and parent transforms.
	struct Texture* RenderTarget; ///< Optional render target for offscreen drawing. When set, the node is rendered into this texture/image instead of directly on screen.
	struct BrushShorthand Background; ///< Background brush for the element. Supports solid color, image, or material.
	struct BrushShorthand Foreground; ///< Foreground brush for the element. Typically used for text or shape fills.
	struct ShadowShorthand BoxShadow; ///< Shadow applied to the element's box.
	struct OverflowShorthand Overflow; ///< Defines overflow behavior for child content. Controls clipping and scroll behavior.
	struct RingShorthand Ring; ///< Decorative ring effect applied around the element.
	struct Material* CompositionBrush; ///< Custom composition brush for advanced rendering. Equivalent to UWP `CompositionBrush`. Can be used for effects or visuals beyond standard brushes.
	struct vec2 CompositionDesignSize; ///< Virtual design size when using composition rendering. Defines the expected size for the composition surface.
	bool_t SizeToContent; ///< Whether the element resizes itself to fit its content.
	bool_t OffscreenRendering; ///< Enables rendering to an offscreen surface before compositing. Useful for caching effects or complex visuals.
	bool_t ForceComposition; ///< Forces use of the composition system even when not required. Equivalent to enabling composition layers for debugging or effects.
	bool_t CacheResult; ///< Enables caching of rendering results. Reduces redraw cost at the expense of memory.
	bool_t SnapToPixel; ///< Snaps rendering to device pixels. Improves text sharpness.
	bool_t ClipChildren; ///< Controls whether children are clipped to the bounds of the element.
	bool_t ContentStretch; ///< Determines if content stretches to fill available space.
	bool_t Hovered; ///< Indicates if the element is currently hovered by pointer/mouse.
	bool_t IgnoreHitTest; ///< Marks object as ignored during hit testing (mouse interaction).
	enum ForegroundHint ForegroundHint; ///< Hint for how foreground should be rendered. May be used for accessibility, high-contrast, or text rendering optimizations.
	struct Node* _node; ///< Internal node reference
	struct Object* _object; ///< Internal object reference
	int32_t _userdata; ///< User-defined data storage
	struct rect _rect; ///< Internal rectangle bounds
	actualPos_t _actual_pos; ///< Internal computed position
	float _opacity; ///< Internal opacity value
	int32_t _bbox_flags; ///< Internal bounding box flags
};
ORCA_API void luaX_pushNode2D(lua_State *L, struct Node2D const* Node2D);
ORCA_API struct Node2D* luaX_checkNode2D(lua_State *L, int idx);

/// @brief Set a frame property value on a Node2D
ORCA_API void
Node2D_SetFrame(struct Node2D*, enum Box3Field, float);

/// @brief Get a frame property value from a Node2D
ORCA_API float
Node2D_GetFrame(struct Node2D*, enum Box3Field);

/// @brief Get the layout bounds of a Node2D along a specific axis
ORCA_API struct bounds
Node2D_GetBounds(struct Node2D*, enum Direction);

/// @brief Get the rectangle bounds of a Node2D including padding and margin
ORCA_API struct rect
Node2D_GetRect(struct Node2D*);

/// @brief Get the background rendering rectangle for a Node2D
ORCA_API struct rect
Node2D_GetBackgroundRect(struct Node2D*);

/// @brief Utility function for calculating node dimensions with different sizing modes
ORCA_API float
Node2D_GetSize(struct Node2D*, enum Direction, enum Sizing);

/// @brief Create a view entity for rendering operations
ORCA_API void
Node2D_GetViewEntity(struct Node2D*, struct ViewEntity*, struct Texture const*, struct BrushShorthand const*);

/// @brief Container for loading and displaying prefabricated UI components
/** PrefabView2D component */
typedef struct PrefabView2D PrefabView2D_t, *PrefabView2DPtr, *lpPrefabView2D_t;
typedef struct PrefabView2D const *PrefabView2DCPtr, *lpcPrefabView2D_t;
struct PrefabView2D {
	const char* SCA; ///< Scene archive identifier or path
	const char* Prefab; ///< Prefab resource name within the archive
	int32_t _loadedSCA; ///< Hashed value of currently loaded SCA
	int32_t _loadedPrefab; ///< Hashed value of loaded prefab instance
};
ORCA_API void luaX_pushPrefabView2D(lua_State *L, struct PrefabView2D const* PrefabView2D);
ORCA_API struct PrefabView2D* luaX_checkPrefabView2D(lua_State *L, int idx);

/// @brief Concrete implementation of text display combining Node2D and TextBlockConcept
/** TextBlock component */
typedef struct TextBlock TextBlock_t, *TextBlockPtr, *lpTextBlock_t;
typedef struct TextBlock const *TextBlockCPtr, *lpcTextBlock_t;
struct TextBlock {
	struct Node2D* _node2D; ///< Internal reference to the underlying Node2D instance
};
ORCA_API void luaX_pushTextBlock(lua_State *L, struct TextBlock const* TextBlock);
ORCA_API struct TextBlock* luaX_checkTextBlock(lua_State *L, int idx);

/// @brief Interactive text input control with multiple input modes
/** Input component */
typedef struct Input Input_t, *InputPtr, *lpInput_t;
typedef struct Input const *InputCPtr, *lpcInput_t;
struct Input {
	const char* Name; ///< Input identifier for forms and data binding
	enum InputType Type; ///< Input behavior type (text, password, etc.)
	int32_t Cursor; ///< Current text cursor position
	bool_t Multiline; ///< Allow multiple lines of text input
	bool_t Checked; ///< Checked state for checkbox/radio types
	struct Texture* _checkmark; ///< Internal checkmark image for checkbox rendering
};
ORCA_API void luaX_pushInput(lua_State *L, struct Input const* Input);
ORCA_API struct Input* luaX_checkInput(lua_State *L, int idx);

/// @brief Interactive button control for user actions
/** Button component */
typedef struct Button Button_t, *ButtonPtr, *lpButton_t;
typedef struct Button const *ButtonCPtr, *lpcButton_t;
struct Button {
	enum ButtonType Type; ///< Button behavior type (normal or submit)
};
ORCA_API void luaX_pushButton(lua_State *L, struct Button const* Button);
ORCA_API struct Button* luaX_checkButton(lua_State *L, int idx);

/// @brief Text label with optional association to form controls
/** Label component */
typedef struct Label Label_t, *LabelPtr, *lpLabel_t;
typedef struct Label const *LabelCPtr, *lpcLabel_t;
struct Label {
	const char* For; ///< Identifier of associated form control
};
ORCA_API void luaX_pushLabel(lua_State *L, struct Label const* Label);
ORCA_API struct Label* luaX_checkLabel(lua_State *L, int idx);

/// @brief Linear layout container arranging children in a single direction
/** StackView component */
typedef struct StackView StackView_t, *StackViewPtr, *lpStackView_t;
typedef struct StackView const *StackViewCPtr, *lpcStackView_t;
struct StackView {
	bool_t Reversed; ///< Reverse the order of child elements
	enum Direction Direction; ///< Layout direction (horizontal or vertical)
	enum AlignItems AlignItems; ///< Cross-axis alignment of children
	enum JustifyContent JustifyContent; ///< Main-axis distribution of children
	float Spacing; ///< Space between adjacent children
};
ORCA_API void luaX_pushStackView(lua_State *L, struct StackView const* StackView);
ORCA_API struct StackView* luaX_checkStackView(lua_State *L, int idx);

/// @brief Specialized stack view for form input collection and submission
/** Form component */
typedef struct Form Form_t, *FormPtr, *lpForm_t;
typedef struct Form const *FormCPtr, *lpcForm_t;
struct Form {
};
ORCA_API void luaX_pushForm(lua_State *L, struct Form const* Form);
ORCA_API struct Form* luaX_checkForm(lua_State *L, int idx);

/// @brief Base class for interactive UI controls with state management
/** Control component */
typedef struct Control Control_t, *ControlPtr, *lpControl_t;
typedef struct Control const *ControlCPtr, *lpcControl_t;
struct Control {
	bool_t Pressed; ///< Currently being pressed by user input
	bool_t Disabled; ///< Interaction disabled, typically grayed out
	bool_t Hovered; ///< Mouse pointer is over the control
	bool_t Focused; ///< Control has keyboard focus
	bool_t Value; ///< Generic boolean value for toggle controls
	float Scale; ///< Visual scale factor for size adjustments
};
ORCA_API void luaX_pushControl(lua_State *L, struct Control const* Control);
ORCA_API struct Control* luaX_checkControl(lua_State *L, int idx);

/// @brief Top-level container representing a display surface or window
/** Screen component */
typedef struct Screen Screen_t, *ScreenPtr, *lpScreen_t;
typedef struct Screen const *ScreenCPtr, *lpcScreen_t;
struct Screen {
	struct color ClearColor; ///< Background clear color for the screen
	enum ResizeMode ResizeMode; ///< Specifies how the window can be resized or displayed
	float DialogResult; ///< Result value for dialog interactions
	struct Texture* _rt; ///< Internal render target for the screen
	uint32_t _size; ///< Currently set size (to resize window when changed).
};
ORCA_API void luaX_pushScreen(lua_State *L, struct Screen const* Screen);
ORCA_API struct Screen* luaX_checkScreen(lua_State *L, int idx);

/// @brief Video or animation playback view for media content
/** Cinematic component */
typedef struct Cinematic Cinematic_t, *CinematicPtr, *lpCinematic_t;
typedef struct Cinematic const *CinematicCPtr, *lpcCinematic_t;
struct Cinematic {
	const char* FileName; ///< Path to media file for playback
	float FrameRate; ///< Playback frame rate (frames per second)
	float NumFrames; ///< Total number of frames in the media
	float FadeOut; ///< Fade-out duration in seconds
	bool_t _playing; ///< Internal playback state flag
	longTime_t _starttime; ///< Internal playback start timestamp
};
ORCA_API void luaX_pushCinematic(lua_State *L, struct Cinematic const* Cinematic);
ORCA_API struct Cinematic* luaX_checkCinematic(lua_State *L, int idx);

/// @brief Grid-based layout container with configurable rows and columns
/** Grid component */
typedef struct Grid Grid_t, *GridPtr, *lpGrid_t;
typedef struct Grid const *GridCPtr, *lpcGrid_t;
struct Grid {
	const char* Columns; ///< Column definition string (e.g., "1fr 2fr 100px")
	const char* Rows; ///< Row definition string (e.g., "auto 1fr auto")
	enum Direction Direction; ///< Primary fill direction for auto-placement
	float Spacing; ///< Space between grid cells
	float CellWidth; ///< Default width for auto-sized cells
	float CellHeight; ///< Default height for auto-sized cells
	uiGridViewBuffer_t _buffer; ///< Internal buffer for grid calculations
};
ORCA_API void luaX_pushGrid(lua_State *L, struct Grid const* Grid);
ORCA_API struct Grid* luaX_checkGrid(lua_State *L, int idx);

/// @brief Displays image content with advanced rendering and scaling options
/** ImageView component */
typedef struct ImageView ImageView_t, *ImageViewPtr, *lpImageView_t;
typedef struct ImageView const *ImageViewCPtr, *lpcImageView_t;
struct ImageView {
	const char* Src; ///< Image source path or resource identifier
	struct Texture* Image; ///< Direct reference to loaded image
	struct vec4 Edges; ///< Edge inset values for nine-patch rendering
	struct vec4 Insets; ///< Content inset values for image positioning
	struct vec4 Viewbox; ///< Viewport rectangle for image cropping/scaling
	enum Stretch Stretch; ///< Determines how the image is scaled to fill its allocated layout space
	int32_t _loadedImage; ///< Hashed value of loaded image resource
};
ORCA_API void luaX_pushImageView(lua_State *L, struct ImageView const* ImageView);
ORCA_API struct ImageView* luaX_checkImageView(lua_State *L, int idx);

/// @brief Specialized image view for nine-patch (stretchable) images
/** NinePatchImage component */
typedef struct NinePatchImage NinePatchImage_t, *NinePatchImagePtr, *lpNinePatchImage_t;
typedef struct NinePatchImage const *NinePatchImageCPtr, *lpcNinePatchImage_t;
struct NinePatchImage {
	float StretchTypeTop; ///< Top edge stretch behavior factor
	float StretchTypeBottom; ///< Bottom edge stretch behavior factor
	float StretchTypeLeft; ///< Left edge stretch behavior factor
	float StretchTypeRight; ///< Right edge stretch behavior factor
	float StretchTypeCenter; ///< Center region stretch behavior factor
	struct Texture* ImageTopLeft; ///< Top-left corner image (non-stretchable)
	struct Texture* ImageTop; ///< Top edge image (horizontally stretchable)
	struct Texture* ImageTopRight; ///< Top-right corner image (non-stretchable)
	struct Texture* ImageLeft; ///< Left edge image (vertically stretchable)
	struct Texture* ImageCenter; ///< Center region image (stretchable in both directions)
	struct Texture* ImageRight; ///< Right edge image (vertically stretchable)
	struct Texture* ImageBottomLeft; ///< Bottom-left corner image (non-stretchable)
	struct Texture* ImageBottom; ///< Bottom edge image (horizontally stretchable)
	struct Texture* ImageBottomRight; ///< Bottom-right corner image (non-stretchable)
};
ORCA_API void luaX_pushNinePatchImage(lua_State *L, struct NinePatchImage const* NinePatchImage);
ORCA_API struct NinePatchImage* luaX_checkNinePatchImage(lua_State *L, int idx);

/// @brief Displays a fixed-size text grid using a monospace layout
/** TerminalView component */
typedef struct TerminalView TerminalView_t, *TerminalViewPtr, *lpTerminalView_t;
typedef struct TerminalView const *TerminalViewCPtr, *lpcTerminalView_t;
struct TerminalView {
	int32_t BufferWidth; ///< Buffer width
	int32_t BufferHeight; ///< Buffer height
	int32_t Cursor; ///< Cursor position
	int32_t SelectedIndex; ///< Selected item index
	bool_t DropShadow; ///< Draw shadow underneath this view
	int32_t* _buffer; ///< Buffer used for characters
	struct vec2 _scroll; ///< Scroll position
	int32_t _contentHeight; ///< Current page length
};
ORCA_API void luaX_pushTerminalView(lua_State *L, struct TerminalView const* TerminalView);
ORCA_API struct TerminalView* luaX_checkTerminalView(lua_State *L, int idx);

/// @brief Represents a single page within a document or UI container.
/** Page component */
typedef struct Page Page_t, *PagePtr, *lpPage_t;
typedef struct Page const *PageCPtr, *lpcPage_t;
struct Page {
	const char* Title; ///< The title of the page.
	const char* Path; ///< The URL path of the page, like "/about".
	float Transition; ///< The transition progress of the page.
	struct Node* _node; ///< Pointer to Node for quick referencing
};
ORCA_API void luaX_pushPage(lua_State *L, struct Page const* Page);
ORCA_API struct Page* luaX_checkPage(lua_State *L, int idx);

/// @brief Container that manages multiple pages and navigation between them.
/** PageHost component */
typedef struct PageHost PageHost_t, *PageHostPtr, *lpPageHost_t;
typedef struct PageHost const *PageHostCPtr, *lpcPageHost_t;
struct PageHost {
	struct Page* ActivePage; ///< The currently active page.
	struct Page* _historyStack[32]; ///< Navigation history stack.
	int32_t _historySize; ///< Number of entries in the navigation history stack.
};
ORCA_API void luaX_pushPageHost(lua_State *L, struct PageHost const* PageHost);
ORCA_API struct PageHost* luaX_checkPageHost(lua_State *L, int idx);

/// @brief Viewport for rendering the active page within a PageHost.
/** PageViewport component */
typedef struct PageViewport PageViewport_t, *PageViewportPtr, *lpPageViewport_t;
typedef struct PageViewport const *PageViewportCPtr, *lpcPageViewport_t;
struct PageViewport {
};
ORCA_API void luaX_pushPageViewport(lua_State *L, struct PageViewport const* PageViewport);
ORCA_API struct PageViewport* luaX_checkPageViewport(lua_State *L, int idx);

/// @brief Defines a reusable set of property values and visual configurations for a target component type.
/** Style component */
typedef struct Style Style_t, *StylePtr, *lpStyle_t;
typedef struct Style const *StyleCPtr, *lpcStyle_t;
struct Style {
	const char* TargetType; ///< Specifies the component type this style targets. Only components of the given type or its derived types will use the style.
	enum StyleType Type; ///< Defines the classification of this style, such as Generic, Theme, Override, Local, or Template, determining its scope and application behavior.
};
ORCA_API void luaX_pushStyle(lua_State *L, struct Style const* Style);
ORCA_API struct Style* luaX_checkStyle(lua_State *L, int idx);

#endif