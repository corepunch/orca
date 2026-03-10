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
#include <source/UIKit/uidef.h>

#include "UIKit_properties.h"
#include "../renderer/api/renderer.h"

typedef void* GetSizeEventPtr;
typedef void* SubmitEventPtr;
typedef struct Size* MeasureEventPtr;
typedef struct rect* ArrangeEventPtr;
typedef struct Size* MeasureOverrideEventPtr;
typedef struct rect* ArrangeOverrideEventPtr;
typedef struct ForegroundContentEventArgs* ForegroundContentEventPtr;
typedef struct PushPropertyEventArgs* PushPropertyEventPtr;
typedef void* UpdateGeometryEventPtr;
typedef struct DrawBrushEventArgs* DrawBrushEventPtr;
typedef struct HandleMessageEventArgs* HandleMessageEventPtr;
typedef struct LoadViewEventArgs* LoadViewEventPtr;
typedef struct MakeTextEventArgs* MakeTextEventPtr;
typedef struct TriggeredEventArgs* TriggeredEventPtr;
typedef struct NavigateToPageArguments* NavigateToPageEventPtr;
typedef struct NavigateBackArguments* NavigateBackEventPtr;

/** Direction enum */
typedef enum Direction {
	kDirectionHorizontal, // Left-to-right horizontal layout
	kDirectionVertical, // Top-to-bottom vertical layout
	kDirectionDepth, // Front-to-back depth layout (3D)
} eDirection_t;
#define Direction_Count 3
ORCA_API const char *DirectionToString(enum Direction value);
ORCA_API enum Direction luaX_checkDirection(lua_State *L, int idx);
ORCA_API void luaX_pushDirection(lua_State *L, enum Direction value);
/** Box3Field enum */
typedef enum Box3Field {
	kBox3FieldX, // X-coordinate of the box origin
	kBox3FieldY, // Y-coordinate of the box origin
	kBox3FieldZ, // Z-coordinate of the box origin
	kBox3FieldWidth, // Width of the box along the X axis
	kBox3FieldHeight, // Height of the box along the Y axis
	kBox3FieldDepth, // Depth of the box along the Z axis
} eBox3Field_t;
#define Box3Field_Count 6
ORCA_API const char *Box3FieldToString(enum Box3Field value);
ORCA_API enum Box3Field luaX_checkBox3Field(lua_State *L, int idx);
ORCA_API void luaX_pushBox3Field(lua_State *L, enum Box3Field value);
/** TextOverflow enum */
typedef enum TextOverflow {
	kTextOverflowClip, // Clip overflowing text at the container boundary
	kTextOverflowEllipsis, // Truncate overflowing text and append an ellipsis (…)
} eTextOverflow_t;
#define TextOverflow_Count 2
ORCA_API const char *TextOverflowToString(enum TextOverflow value);
ORCA_API enum TextOverflow luaX_checkTextOverflow(lua_State *L, int idx);
ORCA_API void luaX_pushTextOverflow(lua_State *L, enum TextOverflow value);
/** TextWrapping enum */
typedef enum TextWrapping {
	kTextWrappingWrapWithOverflow, // Wrap at word boundaries; a word that exceeds the available width is allowed to overflow rather than being broken
	kTextWrappingNoWrap, // No automatic line wrapping; text extends on a single line and may overflow the container
	kTextWrappingWrap, // Wrap at word boundaries; a word that exceeds the available width is broken at the character level to prevent overflow
} eTextWrapping_t;
#define TextWrapping_Count 3
ORCA_API const char *TextWrappingToString(enum TextWrapping value);
ORCA_API enum TextWrapping luaX_checkTextWrapping(lua_State *L, int idx);
ORCA_API void luaX_pushTextWrapping(lua_State *L, enum TextWrapping value);
/** TextHorizontalAlignment enum */
typedef enum TextHorizontalAlignment {
	kTextHorizontalAlignmentLeft, // Align text to the left edge
	kTextHorizontalAlignmentCenter, // Center text horizontally
	kTextHorizontalAlignmentRight, // Align text to the right edge
} eTextHorizontalAlignment_t;
#define TextHorizontalAlignment_Count 3
ORCA_API const char *TextHorizontalAlignmentToString(enum TextHorizontalAlignment value);
ORCA_API enum TextHorizontalAlignment luaX_checkTextHorizontalAlignment(lua_State *L, int idx);
ORCA_API void luaX_pushTextHorizontalAlignment(lua_State *L, enum TextHorizontalAlignment value);
/** TextVerticalAlignment enum */
typedef enum TextVerticalAlignment {
	kTextVerticalAlignmentTop, // Align text to the top
	kTextVerticalAlignmentCenter, // Center text vertically
	kTextVerticalAlignmentBottom, // Align text to the bottom
	kTextVerticalAlignmentBaseline, // Align to text baseline
} eTextVerticalAlignment_t;
#define TextVerticalAlignment_Count 4
ORCA_API const char *TextVerticalAlignmentToString(enum TextVerticalAlignment value);
ORCA_API enum TextVerticalAlignment luaX_checkTextVerticalAlignment(lua_State *L, int idx);
ORCA_API void luaX_pushTextVerticalAlignment(lua_State *L, enum TextVerticalAlignment value);
/** HorizontalAlignment enum */
typedef enum HorizontalAlignment {
	kHorizontalAlignmentStretch, // Stretch to fill container width
	kHorizontalAlignmentLeft, // Align to left edge of container
	kHorizontalAlignmentCenter, // Center within container
	kHorizontalAlignmentRight, // Align to right edge of container
} eHorizontalAlignment_t;
#define HorizontalAlignment_Count 4
ORCA_API const char *HorizontalAlignmentToString(enum HorizontalAlignment value);
ORCA_API enum HorizontalAlignment luaX_checkHorizontalAlignment(lua_State *L, int idx);
ORCA_API void luaX_pushHorizontalAlignment(lua_State *L, enum HorizontalAlignment value);
/** VerticalAlignment enum */
typedef enum VerticalAlignment {
	kVerticalAlignmentStretch, // Stretch to fill container height
	kVerticalAlignmentTop, // Align to top edge of container
	kVerticalAlignmentCenter, // Center within container
	kVerticalAlignmentBottom, // Align to bottom edge of container
} eVerticalAlignment_t;
#define VerticalAlignment_Count 4
ORCA_API const char *VerticalAlignmentToString(enum VerticalAlignment value);
ORCA_API enum VerticalAlignment luaX_checkVerticalAlignment(lua_State *L, int idx);
ORCA_API void luaX_pushVerticalAlignment(lua_State *L, enum VerticalAlignment value);
/** AlignItems enum */
typedef enum AlignItems {
	kAlignItemsStart, // Align to start of cross axis
	kAlignItemsCenter, // Center on cross axis
	kAlignItemsEnd, // Align to end of cross axis
	kAlignItemsStretch, // Stretch across cross axis
	kAlignItemsBaseline, // Align to text baseline
} eAlignItems_t;
#define AlignItems_Count 5
ORCA_API const char *AlignItemsToString(enum AlignItems value);
ORCA_API enum AlignItems luaX_checkAlignItems(lua_State *L, int idx);
ORCA_API void luaX_pushAlignItems(lua_State *L, enum AlignItems value);
/** DepthAlignment enum */
typedef enum DepthAlignment {
	kDepthAlignmentNear, // Align to near clipping plane
	kDepthAlignmentCenter, // Center in depth
	kDepthAlignmentFar, // Align to far clipping plane
	kDepthAlignmentStretch, // Stretch across depth
} eDepthAlignment_t;
#define DepthAlignment_Count 4
ORCA_API const char *DepthAlignmentToString(enum DepthAlignment value);
ORCA_API enum DepthAlignment luaX_checkDepthAlignment(lua_State *L, int idx);
ORCA_API void luaX_pushDepthAlignment(lua_State *L, enum DepthAlignment value);
/** Overflow enum */
typedef enum Overflow {
	kOverflowVisible, // Content overflows container bounds
	kOverflowHidden, // Clip overflowing content
	kOverflowClip, // Hard clipping without scrollbars
	kOverflowScroll, // Show scrollbars for overflow
	kOverflowAuto, // Automatic scrollbar behavior
} eOverflow_t;
#define Overflow_Count 5
ORCA_API const char *OverflowToString(enum Overflow value);
ORCA_API enum Overflow luaX_checkOverflow(lua_State *L, int idx);
ORCA_API void luaX_pushOverflow(lua_State *L, enum Overflow value);
/** ForegroundHint enum */
typedef enum ForegroundHint {
	kForegroundHintNone, // No special foreground treatment
	kForegroundHintTranslucent, // Hint for translucent foreground
	kForegroundHintOpaque, // Hint for opaque foreground optimization
} eForegroundHint_t;
#define ForegroundHint_Count 3
ORCA_API const char *ForegroundHintToString(enum ForegroundHint value);
ORCA_API enum ForegroundHint luaX_checkForegroundHint(lua_State *L, int idx);
ORCA_API void luaX_pushForegroundHint(lua_State *L, enum ForegroundHint value);
/** JustifyContent enum */
typedef enum JustifyContent {
	kJustifyContentStart, // Pack items to start
	kJustifyContentEnd, // Pack items to end
	kJustifyContentCenter, // Center items
	kJustifyContentSpaceBetween, // Distribute with space between
	kJustifyContentSpaceAround, // Distribute with space around
	kJustifyContentSpaceEvenly, // Distribute with even spacing
} eJustifyContent_t;
#define JustifyContent_Count 6
ORCA_API const char *JustifyContentToString(enum JustifyContent value);
ORCA_API enum JustifyContent luaX_checkJustifyContent(lua_State *L, int idx);
ORCA_API void luaX_pushJustifyContent(lua_State *L, enum JustifyContent value);
/** FontWeight enum */
typedef enum FontWeight {
	kFontWeightNormal, // Standard font weight
	kFontWeightBold, // Bold font weight
} eFontWeight_t;
#define FontWeight_Count 2
ORCA_API const char *FontWeightToString(enum FontWeight value);
ORCA_API enum FontWeight luaX_checkFontWeight(lua_State *L, int idx);
ORCA_API void luaX_pushFontWeight(lua_State *L, enum FontWeight value);
/** FontStyle enum */
typedef enum FontStyle {
	kFontStyleNormal, // Regular font style
	kFontStyleItalic, // Italic font style
} eFontStyle_t;
#define FontStyle_Count 2
ORCA_API const char *FontStyleToString(enum FontStyle value);
ORCA_API enum FontStyle luaX_checkFontStyle(lua_State *L, int idx);
ORCA_API void luaX_pushFontStyle(lua_State *L, enum FontStyle value);
/** BorderStyle enum */
typedef enum BorderStyle {
	kBorderStyleNone, // No border
	kBorderStyleHidden, // Hidden border (takes space but invisible)
	kBorderStyleDotted, // Dotted border pattern
	kBorderStyleDashed, // Dashed border pattern
	kBorderStyleSolid, // Solid continuous border
	kBorderStyleDouble, // Double line border
	kBorderStyleGroove, // 3D grooved border effect
	kBorderStyleRidge, // 3D ridged border effect
	kBorderStyleInset, // 3D inset border effect
	kBorderStyleOutset, // 3D outset border effect
} eBorderStyle_t;
#define BorderStyle_Count 10
ORCA_API const char *BorderStyleToString(enum BorderStyle value);
ORCA_API enum BorderStyle luaX_checkBorderStyle(lua_State *L, int idx);
ORCA_API void luaX_pushBorderStyle(lua_State *L, enum BorderStyle value);
/** PlaybackMode enum */
typedef enum PlaybackMode {
	kPlaybackModeNormal, // Standard forward playback
	kPlaybackModeReverse, // Reverse playback from end to start
	kPlaybackModePingpong, // Alternating forward/reverse playback
} ePlaybackMode_t;
#define PlaybackMode_Count 3
ORCA_API const char *PlaybackModeToString(enum PlaybackMode value);
ORCA_API enum PlaybackMode luaX_checkPlaybackMode(lua_State *L, int idx);
ORCA_API void luaX_pushPlaybackMode(lua_State *L, enum PlaybackMode value);
/** ButtonType enum */
typedef enum ButtonType {
	kButtonTypeNormal, // Standard interactive button
	kButtonTypeSubmit, // Form submission button with special handling
} eButtonType_t;
#define ButtonType_Count 2
ORCA_API const char *ButtonTypeToString(enum ButtonType value);
ORCA_API enum ButtonType luaX_checkButtonType(lua_State *L, int idx);
ORCA_API void luaX_pushButtonType(lua_State *L, enum ButtonType value);
/** InputType enum */
typedef enum InputType {
	kInputTypeText, // Single or multi-line text input 
	kInputTypePassword, // Masked password input for security 
	kInputTypeNumber, // Numeric input with validation 
	kInputTypeCheckbox, // Boolean checkbox control 
	kInputTypeRadio, // Radio button for exclusive selection 
} eInputType_t;
#define InputType_Count 5
ORCA_API const char *InputTypeToString(enum InputType value);
ORCA_API enum InputType luaX_checkInputType(lua_State *L, int idx);
ORCA_API void luaX_pushInputType(lua_State *L, enum InputType value);
/** Sizing enum */
typedef enum Sizing {
	kSizingPlusMargin, // Include margins in size calculation
	kSizingSizeOnly, // Return only the base size
	kSizingMinusPadding, // Subtract padding from size
} eSizing_t;
#define Sizing_Count 3
ORCA_API const char *SizingToString(enum Sizing value);
ORCA_API enum Sizing luaX_checkSizing(lua_State *L, int idx);
ORCA_API void luaX_pushSizing(lua_State *L, enum Sizing value);
/** ResizeMode enum */
typedef enum ResizeMode {
	kResizeModeNoResize, // The window size is fixed. The user cannot resize or maximize the window.
	kResizeModeCanMinimize, // The user can minimize the window, but resizing and maximizing are disabled.
	kResizeModeCanResize, // The user can freely resize the window and maximize it.
	kResizeModeCanResizeWithGrip, // The user can resize and maximize the window. A resize grip is displayed in the lower-right corner if the platform supports it.
} eResizeMode_t;
#define ResizeMode_Count 4
ORCA_API const char *ResizeModeToString(enum ResizeMode value);
ORCA_API enum ResizeMode luaX_checkResizeMode(lua_State *L, int idx);
ORCA_API void luaX_pushResizeMode(lua_State *L, enum ResizeMode value);
/** Stretch enum */
typedef enum Stretch {
	kStretchUniform, // Scale content uniformly to fit within the area, preserving aspect ratio
	kStretchNone, // Do not scale the content; display at its native size
	kStretchFill, // Scale content to fill both width and height, ignoring aspect ratio
	kStretchUniformToFill, // Scale content uniformly to fill the area, preserving aspect ratio but cropping if necessary
} eStretch_t;
#define Stretch_Count 4
ORCA_API const char *StretchToString(enum Stretch value);
ORCA_API enum Stretch luaX_checkStretch(lua_State *L, int idx);
ORCA_API void luaX_pushStretch(lua_State *L, enum Stretch value);
/** TransitionType enum */
typedef enum TransitionType {
	kTransitionTypeNone, // No transition
	kTransitionTypeSlide, // Sliding transition
	kTransitionTypeFade, // Fading transition
} eTransitionType_t;
#define TransitionType_Count 3
ORCA_API const char *TransitionTypeToString(enum TransitionType value);
ORCA_API enum TransitionType luaX_checkTransitionType(lua_State *L, int idx);
ORCA_API void luaX_pushTransitionType(lua_State *L, enum TransitionType value);
/** StyleType enum */
typedef enum StyleType {
	kStyleTypeGeneric, // Base style applied to all components of the specified target type. Serves as a default visual definition.
	kStyleTypeNamed, // Style associated with a specific theme or visual skin. Used to switch appearance dynamically based on active theme.
} eStyleType_t;
#define StyleType_Count 2
ORCA_API const char *StyleTypeToString(enum StyleType value);
ORCA_API enum StyleType luaX_checkStyleType(lua_State *L, int idx);
ORCA_API void luaX_pushStyleType(lua_State *L, enum StyleType value);

typedef struct BorderRadiusShorthand BorderRadiusShorthand_t, *lpBorderRadiusShorthand_t;
typedef struct BorderRadiusShorthand const cBorderRadiusShorthand_t, *lpcBorderRadiusShorthand_t;
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
typedef struct MarginShorthand MarginShorthand_t, *lpMarginShorthand_t;
typedef struct MarginShorthand const cMarginShorthand_t, *lpcMarginShorthand_t;
typedef struct BorderShorthand BorderShorthand_t, *lpBorderShorthand_t;
typedef struct BorderShorthand const cBorderShorthand_t, *lpcBorderShorthand_t;
typedef struct SizeAxisShorthand SizeAxisShorthand_t, *lpSizeAxisShorthand_t;
typedef struct SizeAxisShorthand const cSizeAxisShorthand_t, *lpcSizeAxisShorthand_t;
typedef struct SizeShorthand SizeShorthand_t, *lpSizeShorthand_t;
typedef struct SizeShorthand const cSizeShorthand_t, *lpcSizeShorthand_t;
typedef struct PushPropertyEventArgs PushPropertyEventArgs_t, *lpPushPropertyEventArgs_t;
typedef struct PushPropertyEventArgs const cPushPropertyEventArgs_t, *lpcPushPropertyEventArgs_t;
typedef struct ForegroundContentEventArgs ForegroundContentEventArgs_t, *lpForegroundContentEventArgs_t;
typedef struct ForegroundContentEventArgs const cForegroundContentEventArgs_t, *lpcForegroundContentEventArgs_t;
typedef struct HandleMessageEventArgs HandleMessageEventArgs_t, *lpHandleMessageEventArgs_t;
typedef struct HandleMessageEventArgs const cHandleMessageEventArgs_t, *lpcHandleMessageEventArgs_t;
typedef struct MakeTextEventArgs MakeTextEventArgs_t, *lpMakeTextEventArgs_t;
typedef struct MakeTextEventArgs const cMakeTextEventArgs_t, *lpcMakeTextEventArgs_t;
typedef struct DrawBrushEventArgs DrawBrushEventArgs_t, *lpDrawBrushEventArgs_t;
typedef struct DrawBrushEventArgs const cDrawBrushEventArgs_t, *lpcDrawBrushEventArgs_t;
typedef struct LoadViewEventArgs LoadViewEventArgs_t, *lpLoadViewEventArgs_t;
typedef struct LoadViewEventArgs const cLoadViewEventArgs_t, *lpcLoadViewEventArgs_t;
typedef struct TriggeredEventArgs TriggeredEventArgs_t, *lpTriggeredEventArgs_t;
typedef struct TriggeredEventArgs const cTriggeredEventArgs_t, *lpcTriggeredEventArgs_t;
typedef struct NavigateToPageArguments NavigateToPageArguments_t, *lpNavigateToPageArguments_t;
typedef struct NavigateToPageArguments const cNavigateToPageArguments_t, *lpcNavigateToPageArguments_t;
typedef struct NavigateBackArguments NavigateBackArguments_t, *lpNavigateBackArguments_t;
typedef struct NavigateBackArguments const cNavigateBackArguments_t, *lpcNavigateBackArguments_t;



/** BorderRadiusShorthand struct */
struct BorderRadiusShorthand {
	float TopLeftRadius;
	float TopRightRadius;
	float BottomRightRadius;
	float BottomLeftRadius;
};
ORCA_API void luaX_pushBorderRadiusShorthand(lua_State *L, struct BorderRadiusShorthand const* BorderRadiusShorthand);
ORCA_API struct BorderRadiusShorthand* luaX_checkBorderRadiusShorthand(lua_State *L, int idx);
/** EdgeShorthand struct */
struct EdgeShorthand {
	float Left;
	float Right;
};
ORCA_API void luaX_pushEdgeShorthand(lua_State *L, struct EdgeShorthand const* EdgeShorthand);
ORCA_API struct EdgeShorthand* luaX_checkEdgeShorthand(lua_State *L, int idx);
/** AlignmentShorthand struct */
struct AlignmentShorthand {
	int32_t Axis[3];
};
ORCA_API void luaX_pushAlignmentShorthand(lua_State *L, struct AlignmentShorthand const* AlignmentShorthand);
ORCA_API struct AlignmentShorthand* luaX_checkAlignmentShorthand(lua_State *L, int idx);
/** FontShorthand struct */
struct FontShorthand {
	enum FontWeight Weight;
	enum FontStyle Style;
	float Size;
	struct FontFamily* Family;
};
ORCA_API void luaX_pushFontShorthand(lua_State *L, struct FontShorthand const* FontShorthand);
ORCA_API struct FontShorthand* luaX_checkFontShorthand(lua_State *L, int idx);
/** BrushShorthand struct */
struct BrushShorthand {
	struct color Color;
	struct Texture* Image;
	struct Material* Material;
};
ORCA_API void luaX_pushBrushShorthand(lua_State *L, struct BrushShorthand const* BrushShorthand);
ORCA_API struct BrushShorthand* luaX_checkBrushShorthand(lua_State *L, int idx);
/** ShadowShorthand struct */
struct ShadowShorthand {
	struct vec2 Offset;
	float BlurRadius;
	float SpreadRadius;
	struct color Color;
};
ORCA_API void luaX_pushShadowShorthand(lua_State *L, struct ShadowShorthand const* ShadowShorthand);
ORCA_API struct ShadowShorthand* luaX_checkShadowShorthand(lua_State *L, int idx);
/** RingShorthand struct */
struct RingShorthand {
	float Offset;
	float Width;
	struct color Color;
};
ORCA_API void luaX_pushRingShorthand(lua_State *L, struct RingShorthand const* RingShorthand);
ORCA_API struct RingShorthand* luaX_checkRingShorthand(lua_State *L, int idx);
/** OverflowShorthand struct */
struct OverflowShorthand {
	enum Overflow x;
	enum Overflow y;
};
ORCA_API void luaX_pushOverflowShorthand(lua_State *L, struct OverflowShorthand const* OverflowShorthand);
ORCA_API struct OverflowShorthand* luaX_checkOverflowShorthand(lua_State *L, int idx);
/** UnderlineShorthand struct */
struct UnderlineShorthand {
	float Offset;
	float Width;
	struct color Color;
};
ORCA_API void luaX_pushUnderlineShorthand(lua_State *L, struct UnderlineShorthand const* UnderlineShorthand);
ORCA_API struct UnderlineShorthand* luaX_checkUnderlineShorthand(lua_State *L, int idx);
/** MarginShorthand struct */
struct MarginShorthand {
	struct EdgeShorthand Axis[3];
};
ORCA_API void luaX_pushMarginShorthand(lua_State *L, struct MarginShorthand const* MarginShorthand);
ORCA_API struct MarginShorthand* luaX_checkMarginShorthand(lua_State *L, int idx);
/** BorderShorthand struct */
struct BorderShorthand {
	struct MarginShorthand Width;
	struct color Color;
	enum BorderStyle Style;
	struct BorderRadiusShorthand Radius;
};
ORCA_API void luaX_pushBorderShorthand(lua_State *L, struct BorderShorthand const* BorderShorthand);
ORCA_API struct BorderShorthand* luaX_checkBorderShorthand(lua_State *L, int idx);
/** SizeAxisShorthand struct */
struct SizeAxisShorthand {
	float Requested;
	float Desired;
	float Min;
	float Actual;
	float Scroll;
};
ORCA_API void luaX_pushSizeAxisShorthand(lua_State *L, struct SizeAxisShorthand const* SizeAxisShorthand);
ORCA_API struct SizeAxisShorthand* luaX_checkSizeAxisShorthand(lua_State *L, int idx);
/** SizeShorthand struct */
struct SizeShorthand {
	struct SizeAxisShorthand Axis[3];
};
ORCA_API void luaX_pushSizeShorthand(lua_State *L, struct SizeShorthand const* SizeShorthand);
ORCA_API struct SizeShorthand* luaX_checkSizeShorthand(lua_State *L, int idx);
/** PushPropertyEventArgs struct */
struct PushPropertyEventArgs {
	int32_t Placeholder;
};
ORCA_API void luaX_pushPushPropertyEventArgs(lua_State *L, struct PushPropertyEventArgs const* PushPropertyEventArgs);
ORCA_API struct PushPropertyEventArgs* luaX_checkPushPropertyEventArgs(lua_State *L, int idx);
/** ForegroundContentEventArgs struct */
struct ForegroundContentEventArgs {
	struct Texture* result;
};
ORCA_API void luaX_pushForegroundContentEventArgs(lua_State *L, struct ForegroundContentEventArgs const* ForegroundContentEventArgs);
ORCA_API struct ForegroundContentEventArgs* luaX_checkForegroundContentEventArgs(lua_State *L, int idx);
/** HandleMessageEventArgs struct */
struct HandleMessageEventArgs {
	fixedString_t EventName;
	uint32_t FirstArg;
	uint32_t NumArgs;
};
ORCA_API void luaX_pushHandleMessageEventArgs(lua_State *L, struct HandleMessageEventArgs const* HandleMessageEventArgs);
ORCA_API struct HandleMessageEventArgs* luaX_checkHandleMessageEventArgs(lua_State *L, int idx);
/** MakeTextEventArgs struct */
struct MakeTextEventArgs {
	struct ViewText* text;
	uint32_t availableSpace;
};
ORCA_API void luaX_pushMakeTextEventArgs(lua_State *L, struct MakeTextEventArgs const* MakeTextEventArgs);
ORCA_API struct MakeTextEventArgs* luaX_checkMakeTextEventArgs(lua_State *L, int idx);
/** DrawBrushEventArgs struct */
struct DrawBrushEventArgs {
	struct mat4 projection;
	struct Texture* image;
	struct BrushShorthand brush;
	float borderOffset;
	struct vec4 borderWidth;
	bool_t foreground;
	struct ViewDef* viewdef;
};
ORCA_API void luaX_pushDrawBrushEventArgs(lua_State *L, struct DrawBrushEventArgs const* DrawBrushEventArgs);
ORCA_API struct DrawBrushEventArgs* luaX_checkDrawBrushEventArgs(lua_State *L, int idx);
/** LoadViewEventArgs struct */
struct LoadViewEventArgs {
	struct lua_State* lua_state;
};
ORCA_API void luaX_pushLoadViewEventArgs(lua_State *L, struct LoadViewEventArgs const* LoadViewEventArgs);
ORCA_API struct LoadViewEventArgs* luaX_checkLoadViewEventArgs(lua_State *L, int idx);
/** TriggeredEventArgs struct */
struct TriggeredEventArgs {
	struct Trigger* Trigger;
	struct HandleMessageEventArgs message;
};
ORCA_API void luaX_pushTriggeredEventArgs(lua_State *L, struct TriggeredEventArgs const* TriggeredEventArgs);
ORCA_API struct TriggeredEventArgs* luaX_checkTriggeredEventArgs(lua_State *L, int idx);
/** NavigateToPageArguments struct */
struct NavigateToPageArguments {
	fixedString_t URL;
	enum TransitionType TransitionType;
};
ORCA_API void luaX_pushNavigateToPageArguments(lua_State *L, struct NavigateToPageArguments const* NavigateToPageArguments);
ORCA_API struct NavigateToPageArguments* luaX_checkNavigateToPageArguments(lua_State *L, int idx);
/** NavigateBackArguments struct */
struct NavigateBackArguments {
	enum TransitionType TransitionType;
};
ORCA_API void luaX_pushNavigateBackArguments(lua_State *L, struct NavigateBackArguments const* NavigateBackArguments);
ORCA_API struct NavigateBackArguments* luaX_checkNavigateBackArguments(lua_State *L, int idx);

/** DataObject component */
typedef struct DataObject DataObject_t, *DataObjectPtr, *lpDataObject_t;
typedef struct DataObject const *DataObjectCPtr, *lpcDataObject_t;
struct DataObject {
};
ORCA_API void luaX_pushDataObject(lua_State *L, struct DataObject const* DataObject);
ORCA_API struct DataObject* luaX_checkDataObject(lua_State *L, int idx);
/** AnimationPlayer component */
typedef struct AnimationPlayer AnimationPlayer_t, *AnimationPlayerPtr, *lpAnimationPlayer_t;
typedef struct AnimationPlayer const *AnimationPlayerCPtr, *lpcAnimationPlayer_t;
struct AnimationPlayer {
	bool_t AutoplayEnabled;
	float DurationScale;
	enum PlaybackMode PlaybackMode;
	bool_t RelativePlayback;
	int32_t RepeatCount;
	bool_t RestoreOriginalValuesAfterPlayback;
	struct Timeline* Timeline;
};
ORCA_API void luaX_pushAnimationPlayer(lua_State *L, struct AnimationPlayer const* AnimationPlayer);
ORCA_API struct AnimationPlayer* luaX_checkAnimationPlayer(lua_State *L, int idx);
/** Trigger component */
typedef struct Trigger Trigger_t, *TriggerPtr, *lpTrigger_t;
typedef struct Trigger const *TriggerCPtr, *lpcTrigger_t;
struct Trigger {
	fixedString_t Property;
	int32_t Value;
};
ORCA_API void luaX_pushTrigger(lua_State *L, struct Trigger const* Trigger);
ORCA_API struct Trigger* luaX_checkTrigger(lua_State *L, int idx);
/** OnPropertyChangedTrigger component */
typedef struct OnPropertyChangedTrigger OnPropertyChangedTrigger_t, *OnPropertyChangedTriggerPtr, *lpOnPropertyChangedTrigger_t;
typedef struct OnPropertyChangedTrigger const *OnPropertyChangedTriggerCPtr, *lpcOnPropertyChangedTrigger_t;
struct OnPropertyChangedTrigger {
	fixedString_t SourceNode;
	fixedString_t Property;
};
ORCA_API void luaX_pushOnPropertyChangedTrigger(lua_State *L, struct OnPropertyChangedTrigger const* OnPropertyChangedTrigger);
ORCA_API struct OnPropertyChangedTrigger* luaX_checkOnPropertyChangedTrigger(lua_State *L, int idx);
/** OnAttachedTrigger component */
typedef struct OnAttachedTrigger OnAttachedTrigger_t, *OnAttachedTriggerPtr, *lpOnAttachedTrigger_t;
typedef struct OnAttachedTrigger const *OnAttachedTriggerCPtr, *lpcOnAttachedTrigger_t;
struct OnAttachedTrigger {
};
ORCA_API void luaX_pushOnAttachedTrigger(lua_State *L, struct OnAttachedTrigger const* OnAttachedTrigger);
ORCA_API struct OnAttachedTrigger* luaX_checkOnAttachedTrigger(lua_State *L, int idx);
/** EventTrigger component */
typedef struct EventTrigger EventTrigger_t, *EventTriggerPtr, *lpEventTrigger_t;
typedef struct EventTrigger const *EventTriggerCPtr, *lpcEventTrigger_t;
struct EventTrigger {
	fixedString_t RoutedEvent;
};
ORCA_API void luaX_pushEventTrigger(lua_State *L, struct EventTrigger const* EventTrigger);
ORCA_API struct EventTrigger* luaX_checkEventTrigger(lua_State *L, int idx);
/** Setter component */
typedef struct Setter Setter_t, *SetterPtr, *lpSetter_t;
typedef struct Setter const *SetterCPtr, *lpcSetter_t;
struct Setter {
	struct Trigger* Trigger;
	fixedString_t Property;
	fixedString_t Value;
};
ORCA_API void luaX_pushSetter(lua_State *L, struct Setter const* Setter);
ORCA_API struct Setter* luaX_checkSetter(lua_State *L, int idx);
/** Handler component */
typedef struct Handler Handler_t, *HandlerPtr, *lpHandler_t;
typedef struct Handler const *HandlerCPtr, *lpcHandler_t;
struct Handler {
	struct Trigger* Trigger;
	struct Node* Target;
	fixedString_t Function;
};
ORCA_API void luaX_pushHandler(lua_State *L, struct Handler const* Handler);
ORCA_API struct Handler* luaX_checkHandler(lua_State *L, int idx);
/** Brush component */
typedef struct Brush Brush_t, *BrushPtr, *lpBrush_t;
typedef struct Brush const *BrushCPtr, *lpcBrush_t;
struct Brush {
};
ORCA_API void luaX_pushBrush(lua_State *L, struct Brush const* Brush);
ORCA_API struct Brush* luaX_checkBrush(lua_State *L, int idx);
/** ColorBrush component */
typedef struct ColorBrush ColorBrush_t, *ColorBrushPtr, *lpColorBrush_t;
typedef struct ColorBrush const *ColorBrushCPtr, *lpcColorBrush_t;
struct ColorBrush {
	struct color Color;
};
ORCA_API void luaX_pushColorBrush(lua_State *L, struct ColorBrush const* ColorBrush);
ORCA_API struct ColorBrush* luaX_checkColorBrush(lua_State *L, int idx);
/** Node component */
typedef struct Node Node_t, *NodePtr, *lpNode_t;
typedef struct Node const *NodeCPtr, *lpcNode_t;
struct Node {
	struct SizeShorthand Size;
	struct MarginShorthand Margin;
	struct MarginShorthand Padding;
	struct BorderShorthand Border;
	struct AlignmentShorthand Alignment;
	bool_t Visible;
	bool_t QuickHide;
	float VisibleAmountInParent;
	float Opacity;
	objectTags_t Tags;
	struct DataObject* DataContext;
};
ORCA_API void luaX_pushNode(lua_State *L, struct Node const* Node);
ORCA_API struct Node* luaX_checkNode(lua_State *L, int idx);
/** TextRun component */
typedef struct TextRun TextRun_t, *TextRunPtr, *lpTextRun_t;
typedef struct TextRun const *TextRunCPtr, *lpcTextRun_t;
struct TextRun {
	fixedString_t Text;
	struct FontShorthand Font;
	struct UnderlineShorthand Underline;
	float LetterSpacing;
	float LineHeight;
	float CharacterSpacing;
	float FixedCharacterWidth;
	bool_t RemoveSideBearingsProperty;
	struct text_info _textinfo;
	struct FontShorthand* _font;
};
ORCA_API void luaX_pushTextRun(lua_State *L, struct TextRun const* TextRun);
ORCA_API struct TextRun* luaX_checkTextRun(lua_State *L, int idx);
/** TextBlockConcept component */
typedef struct TextBlockConcept TextBlockConcept_t, *TextBlockConceptPtr, *lpTextBlockConcept_t;
typedef struct TextBlockConcept const *TextBlockConceptCPtr, *lpcTextBlockConcept_t;
struct TextBlockConcept {
	fixedString_t TextResourceID;
	fixedString_t TextResourceConfiguration;
	fixedString_t PlaceholderText;
	enum TextOverflow TextOverflow;
	struct BrushShorthand Placeholder;
	bool_t UseFullFontHeight;
	bool_t ConstrainContentHeight;
	bool_t WordWrap;
	enum TextWrapping TextWrapping;
	enum TextHorizontalAlignment TextHorizontalAlignment;
	enum TextVerticalAlignment TextVerticalAlignment;
	uiLabelSteps_t _steps;
	struct Node* _node;
	struct ViewText* _text;
};
ORCA_API void luaX_pushTextBlockConcept(lua_State *L, struct TextBlockConcept const* TextBlockConcept);
ORCA_API struct TextBlockConcept* luaX_checkTextBlockConcept(lua_State *L, int idx);
/** Node2D component */
typedef struct Node2D Node2D_t, *Node2DPtr, *lpNode2D_t;
typedef struct Node2D const *Node2DCPtr, *lpcNode2D_t;
struct Node2D {
	struct transform2 LayoutTransform;
	struct transform2 RenderTransform;
	struct vec2 RenderTransformOrigin;
	struct vec2 ContentOffset;
	struct mat4 Matrix;
	struct Texture* RenderTarget;
	struct BrushShorthand Background;
	struct BrushShorthand Foreground;
	struct ShadowShorthand BoxShadow;
	struct OverflowShorthand Overflow;
	struct RingShorthand Ring;
	struct Material* CompositionBrush;
	struct vec2 CompositionDesignSize;
	bool_t SizeToContent;
	bool_t OffscreenRendering;
	bool_t ForceComposition;
	bool_t CacheResult;
	bool_t SnapToPixel;
	bool_t ClipChildren;
	bool_t ContentStretch;
	bool_t Hovered;
	bool_t IgnoreHitTest;
	enum ForegroundHint ForegroundHint;
	struct Node* _node;
	struct Object* _object;
	int32_t _userdata;
	struct rect _rect;
	actualPos_t _actual_pos;
	float _opacity;
	int32_t _bbox_flags;
};
ORCA_API void luaX_pushNode2D(lua_State *L, struct Node2D const* Node2D);
ORCA_API struct Node2D* luaX_checkNode2D(lua_State *L, int idx);
ORCA_API bool_t
Node2D_IsFrameSet(struct Node2D*, enum Box3Field);
ORCA_API void
Node2D_SetFrame(struct Node2D*, enum Box3Field, float);
ORCA_API float
Node2D_GetFrame(struct Node2D*, enum Box3Field);
ORCA_API struct bounds
Node2D_GetBounds(struct Node2D*, enum Direction);
ORCA_API struct rect
Node2D_GetRect(struct Node2D*);
ORCA_API struct rect
Node2D_GetBackgroundRect(struct Node2D*);
ORCA_API float
Node2D_GetSize(struct Node2D*, enum Direction, enum Sizing);
ORCA_API void
Node2D_GetViewEntity(struct Node2D*, struct ViewEntity*, struct Texture const*, struct BrushShorthand const*);
/** PrefabView2D component */
typedef struct PrefabView2D PrefabView2D_t, *PrefabView2DPtr, *lpPrefabView2D_t;
typedef struct PrefabView2D const *PrefabView2DCPtr, *lpcPrefabView2D_t;
struct PrefabView2D {
	fixedString_t SCA;
	fixedString_t Prefab;
	int32_t _loadedSCA;
	int32_t _loadedPrefab;
};
ORCA_API void luaX_pushPrefabView2D(lua_State *L, struct PrefabView2D const* PrefabView2D);
ORCA_API struct PrefabView2D* luaX_checkPrefabView2D(lua_State *L, int idx);
/** TextBlock component */
typedef struct TextBlock TextBlock_t, *TextBlockPtr, *lpTextBlock_t;
typedef struct TextBlock const *TextBlockCPtr, *lpcTextBlock_t;
struct TextBlock {
	struct Node2D* _node2D;
};
ORCA_API void luaX_pushTextBlock(lua_State *L, struct TextBlock const* TextBlock);
ORCA_API struct TextBlock* luaX_checkTextBlock(lua_State *L, int idx);
/** Input component */
typedef struct Input Input_t, *InputPtr, *lpInput_t;
typedef struct Input const *InputCPtr, *lpcInput_t;
struct Input {
	fixedString_t Name;
	enum InputType Type;
	int32_t Cursor;
	bool_t Multiline;
	bool_t Checked;
	struct Texture* _checkmark;
};
ORCA_API void luaX_pushInput(lua_State *L, struct Input const* Input);
ORCA_API struct Input* luaX_checkInput(lua_State *L, int idx);
/** Button component */
typedef struct Button Button_t, *ButtonPtr, *lpButton_t;
typedef struct Button const *ButtonCPtr, *lpcButton_t;
struct Button {
	enum ButtonType Type;
};
ORCA_API void luaX_pushButton(lua_State *L, struct Button const* Button);
ORCA_API struct Button* luaX_checkButton(lua_State *L, int idx);
/** Label component */
typedef struct Label Label_t, *LabelPtr, *lpLabel_t;
typedef struct Label const *LabelCPtr, *lpcLabel_t;
struct Label {
	fixedString_t For;
};
ORCA_API void luaX_pushLabel(lua_State *L, struct Label const* Label);
ORCA_API struct Label* luaX_checkLabel(lua_State *L, int idx);
/** StackView component */
typedef struct StackView StackView_t, *StackViewPtr, *lpStackView_t;
typedef struct StackView const *StackViewCPtr, *lpcStackView_t;
struct StackView {
	bool_t Reversed;
	enum Direction Direction;
	enum AlignItems AlignItems;
	enum JustifyContent JustifyContent;
	float Spacing;
};
ORCA_API void luaX_pushStackView(lua_State *L, struct StackView const* StackView);
ORCA_API struct StackView* luaX_checkStackView(lua_State *L, int idx);
/** Form component */
typedef struct Form Form_t, *FormPtr, *lpForm_t;
typedef struct Form const *FormCPtr, *lpcForm_t;
struct Form {
};
ORCA_API void luaX_pushForm(lua_State *L, struct Form const* Form);
ORCA_API struct Form* luaX_checkForm(lua_State *L, int idx);
/** Control component */
typedef struct Control Control_t, *ControlPtr, *lpControl_t;
typedef struct Control const *ControlCPtr, *lpcControl_t;
struct Control {
	bool_t Pressed;
	bool_t Disabled;
	bool_t Hovered;
	bool_t Focused;
	bool_t Value;
	float Scale;
};
ORCA_API void luaX_pushControl(lua_State *L, struct Control const* Control);
ORCA_API struct Control* luaX_checkControl(lua_State *L, int idx);
/** Screen component */
typedef struct Screen Screen_t, *ScreenPtr, *lpScreen_t;
typedef struct Screen const *ScreenCPtr, *lpcScreen_t;
struct Screen {
	struct color ClearColor;
	enum ResizeMode ResizeMode;
	struct Texture* _rt;
	uint32_t _size;
	bool_t _initialized;
};
ORCA_API void luaX_pushScreen(lua_State *L, struct Screen const* Screen);
ORCA_API struct Screen* luaX_checkScreen(lua_State *L, int idx);
/** Cinematic component */
typedef struct Cinematic Cinematic_t, *CinematicPtr, *lpCinematic_t;
typedef struct Cinematic const *CinematicCPtr, *lpcCinematic_t;
struct Cinematic {
	fixedString_t FileName;
	float FrameRate;
	float NumFrames;
	float FadeOut;
	bool_t _playing;
	longTime_t _starttime;
};
ORCA_API void luaX_pushCinematic(lua_State *L, struct Cinematic const* Cinematic);
ORCA_API struct Cinematic* luaX_checkCinematic(lua_State *L, int idx);
/** Grid component */
typedef struct Grid Grid_t, *GridPtr, *lpGrid_t;
typedef struct Grid const *GridCPtr, *lpcGrid_t;
struct Grid {
	fixedString_t Columns;
	fixedString_t Rows;
	enum Direction Direction;
	float Spacing;
	float CellWidth;
	float CellHeight;
	uiGridViewBuffer_t _buffer;
};
ORCA_API void luaX_pushGrid(lua_State *L, struct Grid const* Grid);
ORCA_API struct Grid* luaX_checkGrid(lua_State *L, int idx);
/** ImageView component */
typedef struct ImageView ImageView_t, *ImageViewPtr, *lpImageView_t;
typedef struct ImageView const *ImageViewCPtr, *lpcImageView_t;
struct ImageView {
	fixedString_t Src;
	struct Texture* Image;
	struct vec4 Edges;
	struct vec4 Insets;
	struct vec4 Viewbox;
	enum Stretch Stretch;
	int32_t _loadedImage;
};
ORCA_API void luaX_pushImageView(lua_State *L, struct ImageView const* ImageView);
ORCA_API struct ImageView* luaX_checkImageView(lua_State *L, int idx);
/** NinePatchImage component */
typedef struct NinePatchImage NinePatchImage_t, *NinePatchImagePtr, *lpNinePatchImage_t;
typedef struct NinePatchImage const *NinePatchImageCPtr, *lpcNinePatchImage_t;
struct NinePatchImage {
	float StretchTypeTop;
	float StretchTypeBottom;
	float StretchTypeLeft;
	float StretchTypeRight;
	float StretchTypeCenter;
	struct Texture* ImageTopLeft;
	struct Texture* ImageTop;
	struct Texture* ImageTopRight;
	struct Texture* ImageLeft;
	struct Texture* ImageCenter;
	struct Texture* ImageRight;
	struct Texture* ImageBottomLeft;
	struct Texture* ImageBottom;
	struct Texture* ImageBottomRight;
};
ORCA_API void luaX_pushNinePatchImage(lua_State *L, struct NinePatchImage const* NinePatchImage);
ORCA_API struct NinePatchImage* luaX_checkNinePatchImage(lua_State *L, int idx);
/** TerminalView component */
typedef struct TerminalView TerminalView_t, *TerminalViewPtr, *lpTerminalView_t;
typedef struct TerminalView const *TerminalViewCPtr, *lpcTerminalView_t;
struct TerminalView {
	int32_t BufferWidth;
	int32_t BufferHeight;
	int32_t Cursor;
	int32_t SelectedIndex;
	bool_t DropShadow;
	int32_t* _buffer;
	struct vec2 _scroll;
	int32_t _contentHeight;
};
ORCA_API void luaX_pushTerminalView(lua_State *L, struct TerminalView const* TerminalView);
ORCA_API struct TerminalView* luaX_checkTerminalView(lua_State *L, int idx);
/** Page component */
typedef struct Page Page_t, *PagePtr, *lpPage_t;
typedef struct Page const *PageCPtr, *lpcPage_t;
struct Page {
	fixedString_t Title;
	fixedString_t Path;
	float Transition;
	struct Node* _node;
};
ORCA_API void luaX_pushPage(lua_State *L, struct Page const* Page);
ORCA_API struct Page* luaX_checkPage(lua_State *L, int idx);
/** PageHost component */
typedef struct PageHost PageHost_t, *PageHostPtr, *lpPageHost_t;
typedef struct PageHost const *PageHostCPtr, *lpcPageHost_t;
struct PageHost {
	struct Page* ActivePage;
	struct Page* _historyStack[32];
	int32_t _historySize;
};
ORCA_API void luaX_pushPageHost(lua_State *L, struct PageHost const* PageHost);
ORCA_API struct PageHost* luaX_checkPageHost(lua_State *L, int idx);
/** PageViewport component */
typedef struct PageViewport PageViewport_t, *PageViewportPtr, *lpPageViewport_t;
typedef struct PageViewport const *PageViewportCPtr, *lpcPageViewport_t;
struct PageViewport {
};
ORCA_API void luaX_pushPageViewport(lua_State *L, struct PageViewport const* PageViewport);
ORCA_API struct PageViewport* luaX_checkPageViewport(lua_State *L, int idx);
/** Style component */
typedef struct Style Style_t, *StylePtr, *lpStyle_t;
typedef struct Style const *StyleCPtr, *lpcStyle_t;
struct Style {
	fixedString_t TargetType;
	enum StyleType Type;
};
ORCA_API void luaX_pushStyle(lua_State *L, struct Style const* Style);
ORCA_API struct Style* luaX_checkStyle(lua_State *L, int idx);

#endif