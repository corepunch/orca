// Auto-generated from core.xml by tools/templates/header.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __CORE_H__
#define __CORE_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;

struct localization;
struct game;
struct Property;
struct lua_State;
struct style_class_selector;


#include "core_properties.h"
#include "../geometry/geometry.h"

typedef struct Object_CreateEventArgs Object_CreateMsg_t,* Object_CreateMsgPtr;
typedef struct Object_StartEventArgs Object_StartMsg_t,* Object_StartMsgPtr;
typedef struct Object_AnimateEventArgs Object_AnimateMsg_t,* Object_AnimateMsgPtr;
typedef struct StyleController_ThemeChangedEventArgs StyleController_ThemeChangedMsg_t,* StyleController_ThemeChangedMsgPtr;
typedef struct Object_PropertyChangedEventArgs Object_PropertyChangedMsg_t,* Object_PropertyChangedMsgPtr;
typedef struct Object_AttachedEventArgs Object_AttachedMsg_t,* Object_AttachedMsgPtr;
typedef struct Object_ReleaseEventArgs Object_ReleaseMsg_t,* Object_ReleaseMsgPtr;
typedef struct Object_DestroyEventArgs Object_DestroyMsg_t,* Object_DestroyMsgPtr;
typedef struct Object_TimerEventArgs Object_TimerMsg_t,* Object_TimerMsgPtr;
typedef struct AnimationPlayer_PlayEventArgs AnimationPlayer_PlayMsg_t,* AnimationPlayer_PlayMsgPtr;
typedef struct AnimationPlayer_ResumeEventArgs AnimationPlayer_ResumeMsg_t,* AnimationPlayer_ResumeMsgPtr;
typedef struct AnimationPlayer_StopEventArgs AnimationPlayer_StopMsg_t,* AnimationPlayer_StopMsgPtr;
typedef struct AnimationPlayer_PauseEventArgs AnimationPlayer_PauseMsg_t,* AnimationPlayer_PauseMsgPtr;
typedef struct AnimationPlayer_StartedEventArgs AnimationPlayer_StartedMsg_t,* AnimationPlayer_StartedMsgPtr;
typedef struct AnimationPlayer_StoppedEventArgs AnimationPlayer_StoppedMsg_t,* AnimationPlayer_StoppedMsgPtr;
typedef struct AnimationPlayer_CompletedEventArgs AnimationPlayer_CompletedMsg_t,* AnimationPlayer_CompletedMsgPtr;
typedef struct StyleController_AddClassEventArgs StyleController_AddClassMsg_t,* StyleController_AddClassMsgPtr;
typedef struct StyleController_AddClassesEventArgs StyleController_AddClassesMsg_t,* StyleController_AddClassesMsgPtr;
typedef struct StateManagerController_ControllerChangedEventArgs StateManagerController_ControllerChangedMsg_t,* StateManagerController_ControllerChangedMsgPtr;
typedef struct Trigger_TriggeredEventArgs Trigger_TriggeredMsg_t,* Trigger_TriggeredMsgPtr;
typedef struct Node_UpdateMatrixEventArgs Node_UpdateMatrixMsg_t,* Node_UpdateMatrixMsgPtr;
typedef struct Node_LoadViewEventArgs Node_LoadViewMsg_t,* Node_LoadViewMsgPtr;
typedef struct Node_HitTestEventArgs Node_HitTestMsg_t,* Node_HitTestMsgPtr;
typedef struct Node_IsVisibleEventArgs Node_IsVisibleMsg_t,* Node_IsVisibleMsgPtr;
typedef struct Node_ViewDidLoadEventArgs Node_ViewDidLoadMsg_t,* Node_ViewDidLoadMsgPtr;
typedef struct Node_KillFocusEventArgs Node_KillFocusMsg_t,* Node_KillFocusMsgPtr;
typedef struct Node_SetFocusEventArgs Node_SetFocusMsg_t,* Node_SetFocusMsgPtr;
typedef struct Node_GetSizeEventArgs Node_GetSizeMsg_t,* Node_GetSizeMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_MouseMessageMsg_t,* Node_MouseMessageMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_LeftButtonDownMsg_t,* Node_LeftButtonDownMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_RightButtonDownMsg_t,* Node_RightButtonDownMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_OtherButtonDownMsg_t,* Node_OtherButtonDownMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_LeftButtonUpMsg_t,* Node_LeftButtonUpMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_RightButtonUpMsg_t,* Node_RightButtonUpMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_OtherButtonUpMsg_t,* Node_OtherButtonUpMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_LeftButtonDraggedMsg_t,* Node_LeftButtonDraggedMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_RightButtonDraggedMsg_t,* Node_RightButtonDraggedMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_OtherButtonDraggedMsg_t,* Node_OtherButtonDraggedMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_LeftDoubleClickMsg_t,* Node_LeftDoubleClickMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_RightDoubleClickMsg_t,* Node_RightDoubleClickMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_OtherDoubleClickMsg_t,* Node_OtherDoubleClickMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_MouseMovedMsg_t,* Node_MouseMovedMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_ScrollWheelMsg_t,* Node_ScrollWheelMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_DragDropMsg_t,* Node_DragDropMsgPtr;
typedef struct Node_MouseMessageEventArgs Node_DragEnterMsg_t,* Node_DragEnterMsgPtr;
typedef struct Node_KeyMessageEventArgs Node_KeyMessageMsg_t,* Node_KeyMessageMsgPtr;
typedef struct Node_KeyMessageEventArgs Node_KeyDownMsg_t,* Node_KeyDownMsgPtr;
typedef struct Node_KeyMessageEventArgs Node_KeyUpMsg_t,* Node_KeyUpMsgPtr;
typedef struct Node_KeyMessageEventArgs Node_TextInputMsg_t,* Node_TextInputMsgPtr;


/// @brief Defines the routing strategy for messages sent to objects. This determines how messages propagate through the object hierarchy and which handlers are invoked.
/** MessageRouting enum */
typedef enum MessageRouting {
	kMessageRoutingBubbling, ///< Messages are bubbled up the hierarchy.
	kMessageRoutingTunnelingBubbling, ///< Messages are first tunneled down the hierarchy and then bubbled up.
	kMessageRoutingTunneling, ///< Messages are tunneled down the hierarchy.
	kMessageRoutingDirect, ///< Messages are sent directly to the target object.
} eMessageRouting_t;
#define MessageRouting_Count 4
ORCA_API const char *MessageRoutingToString(enum MessageRouting value);
ORCA_API enum MessageRouting luaX_checkMessageRouting(lua_State *L, int idx);
ORCA_API void luaX_pushMessageRouting(lua_State *L, enum MessageRouting value);

/// @brief Represents the various states a property can be in.
/** PropertyState enum */
typedef enum PropertyState {
	kPropertyStateNormal, ///< Normal property state when no triggers are active
	kPropertyStateHover, ///< Property state when the mouse is hovering over it
	kPropertyStateFocus, ///< Property state when it has focus
	kPropertyStateSelect, ///< Property state when it is selected
	kPropertyStateDisable, ///< Property state when it is disabled
	kPropertyStateOldValue, ///< Property state representing the previous value
} ePropertyState_t;
#define PropertyState_Count 6
ORCA_API const char *PropertyStateToString(enum PropertyState value);
ORCA_API enum PropertyState luaX_checkPropertyState(lua_State *L, int idx);
ORCA_API void luaX_pushPropertyState(lua_State *L, enum PropertyState value);

/// @brief Various modes to bind properties
/** BindingMode enum */
typedef enum BindingMode {
	kBindingModeOneWay, ///< Default mode, updates target property when source changes
	kBindingModeTwoWay, ///< Updates target property when source changes and updates source when target changes
	kBindingModeOneWayToSource, ///< Updates source property when target changes, but not the other way around
	kBindingModeExpression, ///< Allows binding to a custom expression
} eBindingMode_t;
#define BindingMode_Count 4
ORCA_API const char *BindingModeToString(enum BindingMode value);
ORCA_API enum BindingMode luaX_checkBindingMode(lua_State *L, int idx);
ORCA_API void luaX_pushBindingMode(lua_State *L, enum BindingMode value);

/// @brief Attributes that can be applied to properties for binding purposes.
/** PropertyAttribute enum */
typedef enum PropertyAttribute {
	kPropertyAttributeWholeProperty, ///< Default binding to the whole property
	kPropertyAttributeColorR, ///< Bind to the red channel of a color property
	kPropertyAttributeColorG, ///< Bind to the green channel of a color property
	kPropertyAttributeColorB, ///< Bind to the blue channel of a color property
	kPropertyAttributeColorA, ///< Bind to the alpha channel of a color property
	kPropertyAttributeVectorX, ///< Bind to the X component of a vector property
	kPropertyAttributeVectorY, ///< Bind to the Y component of a vector property
	kPropertyAttributeVectorZ, ///< Bind to the Z component of a vector property
	kPropertyAttributeVectorW, ///< Bind to the W component of a vector property
} ePropertyAttribute_t;
#define PropertyAttribute_Count 9
ORCA_API const char *PropertyAttributeToString(enum PropertyAttribute value);
ORCA_API enum PropertyAttribute luaX_checkPropertyAttribute(lua_State *L, int idx);
ORCA_API void luaX_pushPropertyAttribute(lua_State *L, enum PropertyAttribute value);

/// @brief Defines how an animation clip repeats after reaching its end.
/** AnimationMode enum */
typedef enum AnimationMode {
	kAnimationModePlayOnce, ///< Play the animation once and stop at the last frame
	kAnimationModeLoop, ///< Restart from the beginning when the animation reaches the end
	kAnimationModePingPong, ///< Alternate between playing forward and backward
} eAnimationMode_t;
#define AnimationMode_Count 3
ORCA_API const char *AnimationModeToString(enum AnimationMode value);
ORCA_API enum AnimationMode luaX_checkAnimationMode(lua_State *L, int idx);
ORCA_API void luaX_pushAnimationMode(lua_State *L, enum AnimationMode value);

/// @brief Animation playback behavior modes
/** PlaybackMode enum */
typedef enum PlaybackMode {
	kPlaybackModeNormal, ///< Standard forward playback
	kPlaybackModeReverse, ///< Reverse playback from end to start
	kPlaybackModePingPong, ///< Alternating forward/reverse playback
} ePlaybackMode_t;
#define PlaybackMode_Count 3
ORCA_API const char *PlaybackModeToString(enum PlaybackMode value);
ORCA_API enum PlaybackMode luaX_checkPlaybackMode(lua_State *L, int idx);
ORCA_API void luaX_pushPlaybackMode(lua_State *L, enum PlaybackMode value);
/** InterpolationMode enum */
typedef enum InterpolationMode {
	kInterpolationModeLinear, ///< Linear interpolation
	kInterpolationModeConst, ///< Constant interpolation
	kInterpolationModeBack, ///< Back interpolation
	kInterpolationModeBounce, ///< Bounce interpolation
	kInterpolationModeCirc, ///< Circular interpolation
	kInterpolationModeCubic, ///< Cubic interpolation
	kInterpolationModeElastic, ///< Elastic interpolation
	kInterpolationModeExpo, ///< Exponential interpolation
	kInterpolationModeQuad, ///< Quadratic interpolation
	kInterpolationModeQuart, ///< Quartic interpolation
	kInterpolationModeQuint, ///< Quintic interpolation
	kInterpolationModeSine, ///< Sine interpolation
} eInterpolationMode_t;
#define InterpolationMode_Count 12
ORCA_API const char *InterpolationModeToString(enum InterpolationMode value);
ORCA_API enum InterpolationMode luaX_checkInterpolationMode(lua_State *L, int idx);
ORCA_API void luaX_pushInterpolationMode(lua_State *L, enum InterpolationMode value);

/// @brief Discriminates how the string value of a LocaleEntry should be interpreted.
/** LocaleEntryType enum */
typedef enum LocaleEntryType {
	kLocaleEntryTypeUndefined, ///< Unrecognised or unset type
	kLocaleEntryTypeText, ///< A display string (may contain format tags)
	kLocaleEntryTypeResource, ///< An asset path (returned verbatim)
} eLocaleEntryType_t;
#define LocaleEntryType_Count 3
ORCA_API const char *LocaleEntryTypeToString(enum LocaleEntryType value);
ORCA_API enum LocaleEntryType luaX_checkLocaleEntryType(lua_State *L, int idx);
ORCA_API void luaX_pushLocaleEntryType(lua_State *L, enum LocaleEntryType value);

/// @brief Defines easing functions for animation interpolation.
/** Easing enum */
typedef enum Easing {
	kEasingInOut, ///< Easing function that starts slow, speeds up in the middle, and slows down at the end
	kEasingIn, ///< Easing function that starts slow and speeds up towards the end
	kEasingOut, ///< Easing function that starts fast and slows down towards the end
} eEasing_t;
#define Easing_Count 3
ORCA_API const char *EasingToString(enum Easing value);
ORCA_API enum Easing luaX_checkEasing(lua_State *L, int idx);
ORCA_API void luaX_pushEasing(lua_State *L, enum Easing value);

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

/// @brief Identifies which mouse button was involved in a mouse button event.
/** MouseButton enum */
typedef enum MouseButton {
	kMouseButtonNone, ///< No mouse button used for mouse move and scroll events
	kMouseButtonLeft, ///< The left (primary) mouse button
	kMouseButtonRight, ///< The right (secondary) mouse button
	kMouseButtonMiddle, ///< The middle mouse button
} eMouseButton_t;
#define MouseButton_Count 4
ORCA_API const char *MouseButtonToString(enum MouseButton value);
ORCA_API enum MouseButton luaX_checkMouseButton(lua_State *L, int idx);
ORCA_API void luaX_pushMouseButton(lua_State *L, enum MouseButton value);

/// @brief Discriminates how the string value of a ResourceEntry should be interpreted.
/** ResourceEntryType enum */
typedef enum ResourceEntryType {
	kResourceEntryTypeUndefined, ///< Unrecognised or unset type
	kResourceEntryTypeText, ///< A display string (may contain format tags)
	kResourceEntryTypeResource, ///< An asset path (returned verbatim)
} eResourceEntryType_t;
#define ResourceEntryType_Count 3
ORCA_API const char *ResourceEntryTypeToString(enum ResourceEntryType value);
ORCA_API enum ResourceEntryType luaX_checkResourceEntryType(lua_State *L, int idx);
ORCA_API void luaX_pushResourceEntryType(lua_State *L, enum ResourceEntryType value);

typedef struct AnimationClipReference AnimationClipReference_t, *lpAnimationClipReference_t;
typedef struct AnimationClipReference const cAnimationClipReference_t, *lpcAnimationClipReference_t;
typedef struct Keyframe Keyframe_t, *lpKeyframe_t;
typedef struct Keyframe const cKeyframe_t, *lpcKeyframe_t;
typedef struct LocaleEntry LocaleEntry_t, *lpLocaleEntry_t;
typedef struct LocaleEntry const cLocaleEntry_t, *lpcLocaleEntry_t;
typedef struct ResourceEntry ResourceEntry_t, *lpResourceEntry_t;
typedef struct ResourceEntry const cResourceEntry_t, *lpcResourceEntry_t;
typedef struct CornerRadius CornerRadius_t, *lpCornerRadius_t;
typedef struct CornerRadius const cCornerRadius_t, *lpcCornerRadius_t;
typedef struct EdgeShorthand EdgeShorthand_t, *lpEdgeShorthand_t;
typedef struct EdgeShorthand const cEdgeShorthand_t, *lpcEdgeShorthand_t;
typedef struct AlignmentShorthand AlignmentShorthand_t, *lpAlignmentShorthand_t;
typedef struct AlignmentShorthand const cAlignmentShorthand_t, *lpcAlignmentShorthand_t;
typedef struct Thickness Thickness_t, *lpThickness_t;
typedef struct Thickness const cThickness_t, *lpcThickness_t;
typedef struct BorderShorthand BorderShorthand_t, *lpBorderShorthand_t;
typedef struct BorderShorthand const cBorderShorthand_t, *lpcBorderShorthand_t;
typedef struct SizeAxisShorthand SizeAxisShorthand_t, *lpSizeAxisShorthand_t;
typedef struct SizeAxisShorthand const cSizeAxisShorthand_t, *lpcSizeAxisShorthand_t;
typedef struct SizeShorthand SizeShorthand_t, *lpSizeShorthand_t;
typedef struct SizeShorthand const cSizeShorthand_t, *lpcSizeShorthand_t;


/// @brief Retrieves currently active object.
ORCA_API struct Object*
core_GetFocus(void);

/// @brief Gets the currently hovered object
ORCA_API struct Object*
core_GetHover(void);

/// @brief Adds global style rules that gets applied even without any stylesheet present.
ORCA_API void
core_AddGlobalStyleRule(struct lua_State*, struct Object*);

/// @brief Advances the engine by one frame, processing all pending updates and rendering.
ORCA_API void
core_AdvanceFrame(void);

/// @brief Drains the pending platform message queue and dispatches each event immediately.
ORCA_API void
core_FlushQueue(struct lua_State*);


/// @name Lifecycle
/// Manages object creation, initialization, update cycles, and destruction.

/// @brief Create new object.
ORCA_API int
OBJ_CreateFromLuaState(struct lua_State*);

/// @brief Clear all children of the object.
ORCA_API void
OBJ_Clear(struct lua_State*, struct Object*);

/// @brief Garbage-collect an object (clear and release).
ORCA_API void
OBJ_Release(struct lua_State*, struct Object*);

/// @brief Compare two objects for equality.
ORCA_API bool_t
OBJ_Equals(struct Object const*, struct Object const*);

/// @brief Rebuilds the object's body content asynchronously
ORCA_API void
OBJ_Rebuild(struct lua_State*, struct Object*);

/// @name Hierarchy
/// Navigates and manipulates the parent-child relationship tree.

/// @brief Add a child object.
ORCA_API struct Object*
OBJ_AddChild(struct Object*, struct Object*, bool_t);

/// @brief Destroys an object.
ORCA_API void
OBJ_RemoveFromParent(struct lua_State*, struct Object*);

/// @brief Gets the parent object in the hierarchy
ORCA_API struct Object*
OBJ_GetParent(struct Object const*);

/// @brief Gets the first child object
ORCA_API struct Object*
OBJ_GetFirstChild(struct Object const*);

/// @brief Gets the next sibling object
ORCA_API struct Object*
OBJ_GetNext(struct Object const*);

/// @brief Gets the root object of the hierarchy
ORCA_API struct Object*
OBJ_GetRoot(struct Object*);

/// @brief Find a child object by name.
ORCA_API struct Object*
OBJ_FindChild(struct Object*, const char*, bool_t);

/// @brief Finds child object by hierarchical path
ORCA_API struct Object*
OBJ_FindByPath(struct Object*, const char*);

/// @brief Finds a child object by its unique identifier
ORCA_API struct Object*
OBJ_FindChildByID(struct Object*, uint32_t);

/// @brief Finds a child object by its alias identifier
ORCA_API struct Object*
OBJ_FindChildByAlias(struct Object*, uint32_t);

/// @brief Finds a child object of a specific class
ORCA_API struct Object*
OBJ_FindChildOfClass(struct Object*, uint32_t);

/// @brief Finds the nearest parent object of a specific class
ORCA_API struct Object*
OBJ_FindParentOfClass(struct Object*, uint32_t);

/// @name Messaging
/// Dispatches events and sends messages between objects.

/// @brief Posts a message asynchronously, bubbling through the object hierarchy.
ORCA_API void
OBJ_post(struct lua_State*, struct Object*, const char*);

/// @brief Sends a message synchronously to the current object and returns the result.
ORCA_API int
OBJ_send(struct lua_State*, struct Object*, const char*);

/// @name Properties
/// Reads and writes typed properties, applying and observing changes.

/// @brief Set a property on the object.
ORCA_API bool_t
OBJ_SetProperty(struct lua_State*, struct Object*, const char*);

/// @brief Get a property value from an object.
ORCA_API int
OBJ_GetProperty(struct lua_State*, struct Object*, const char*);

/// @brief Updates object properties.
ORCA_API void
OBJ_UpdateProperties(struct Object*);

/// @brief Emits onPropertyChanged events by comparing to previous values.
ORCA_API void
OBJ_EmitPropertyChangedEvents(struct lua_State*, struct Object*);

/// @brief Looks up a property by context-driven syntax, like "Column" instead of "Grid.Column"
ORCA_API struct PropertyType const*
OBJ_FindImplicitProperty(struct Object*, const char*);

/// @brief Looks up a property by full syntax, like "Grid.Column" instead of "Column"
ORCA_API struct PropertyType const*
OBJ_FindExplicitProperty(struct Object*, const char*);

/// @brief Attaches a property program to the specified property
ORCA_API bool_t
OBJ_AttachPropertyProgram(struct Object*, const char*, const char*, enum PropertyAttribute, enum BindingMode, bool_t);

/// @brief Finds a property by navigating a hierarchical path
ORCA_API struct Property*
OBJ_FindPropertyByPath(struct Object*, const char*);

/// @brief Gets the properties collection
ORCA_API struct Property*
OBJ_GetProperties(struct Object const*);

/// @brief Gets an integer property value by identifier
ORCA_API int32_t
OBJ_GetInteger(struct Object const*, uint32_t, int32_t);

/// @name Layout
/// Controls position, size, and dirty-flag propagation for layout passes.

/// @brief Sets object dirty and queues it for recalculation
ORCA_API void
OBJ_SetDirty(struct Object*);

/// @brief Clears dirty flags, marks object as recalculated
ORCA_API void
OBJ_ClearDirtyFlags(struct Object*);

/// @name Style
/// Manages style sheets and resolves computed style values.

/// @brief Retrieves object style flags
ORCA_API uint32_t
OBJ_GetStyle(struct Object const*);

/// @brief Sets object style flags
ORCA_API void
OBJ_SetStyle(struct Object*, uint32_t);

/// @name Animation
/// Attaches, plays, and transitions keyframe animations on this object.

/// @brief Tween an object property over time.
ORCA_API void
OBJ_DoTween(struct lua_State*, struct Object*, const char*, int32_t, enum InterpolationMode, enum Easing);

/// @brief Attaches a component class to this object.
ORCA_API void
OBJ_AddComponentByName(struct lua_State*, struct Object*, const char*);

/// @name Focus and Input
/// Controls focus state, hover, modal presentation, and timers.

/// @brief Set focus on the object.
ORCA_API void
OBJ_SetFocus(struct Object*);

/// @brief Checks if this object currently has focus
ORCA_API bool_t
OBJ_IsFocused(struct Object const*);

/// @brief Sets the hover state for an object
ORCA_API void
OBJ_SetHover(struct Object*);

/// @brief Sets or clears the modal child object
ORCA_API int
OBJ_ShowModal(struct lua_State*, struct Object*, struct Object*);

/// @brief Set a timer on the object.
ORCA_API int32_t
OBJ_SetTimer(struct Object*, int32_t);

/// @name Identity and State
/// Accesses name, class, flags, aliases, text content, and Lua state.

/// @brief Retrieves the object's name identifier
ORCA_API const char*
OBJ_GetName(struct Object const*);

/// @brief Sets the object's name identifier
ORCA_API void
OBJ_SetName(struct Object*, const char*);

/// @brief Returns the object's class type name
ORCA_API const char*
OBJ_GetClassName(struct Object const*);

/// @brief Sets the class name of the object
ORCA_API void
OBJ_SetClassName(struct Object*, const char*);

/// @brief Checks if object has a specific name
ORCA_API bool_t
OBJ_CheckName(struct Object const*, const char*);

/// @brief Gets the object flags
ORCA_API uint32_t
OBJ_GetFlags(struct Object const*);

/// @brief Sets the object flags
ORCA_API void
OBJ_SetFlags(struct Object*, uint32_t);

/// @brief Gets the identifier of the object
ORCA_API uint32_t
OBJ_GetIdentifier(struct Object const*);

/// @brief Gets the alias identifier
ORCA_API uint32_t
OBJ_GetAlias(struct Object const*);

/// @brief Gets the source file path
ORCA_API const char*
OBJ_GetSourceFile(struct Object const*);

/// @brief Sets the source file path of the object
ORCA_API void
OBJ_SetSourceFile(struct Object*, const char*);

/// @brief Gets the text content of the object
ORCA_API const char*
OBJ_GetTextContent(struct Object const*);

/// @brief Sets the text content of the object
ORCA_API void
OBJ_SetTextContent(struct Object*, const char*);

/// @brief Gets the last modified timestamp
ORCA_API long
OBJ_GetTimestamp(struct Object const*);

/// @brief Gets the domain of the object
ORCA_API struct lua_State*
OBJ_GetDomain(struct Object*);

/// @brief Set the current context object where newly created objects will be parented.
ORCA_API void
OBJ_SetContext(struct lua_State*, struct Object*);

/// @name Prefabs and Aliases
/// Loads prefab templates and manages named child aliases.

/// @brief Instantiates a new object from this prefab.
ORCA_API struct Object*
OBJ_Instantiate(struct lua_State*, struct Object*);

/// @brief Loads and instantiates prefabs.
ORCA_API void
OBJ_LoadPrefabs(struct lua_State*, struct Object*);

/// @brief Checks if this object is a prefab view container
ORCA_API bool_t
OBJ_IsPrefabView(struct Object const*);

/// @brief A named animation clip entry used in an AnimationPlayer's Clips array.
/** AnimationClipReference struct */
struct AnimationClipReference {
	const char* Name; ///< Short name used to identify and select this clip (e.g. "idle", "walk")
	struct AnimationClip* Clip; ///< The AnimationClip object for this entry
};
ORCA_API void luaX_pushAnimationClipReference(lua_State *L, struct AnimationClipReference const* AnimationClipReference);
ORCA_API struct AnimationClipReference* luaX_checkAnimationClipReference(lua_State *L, int idx);
/// @brief A single keyframe in an animation curve.
/** Keyframe struct */
struct Keyframe {
	float time; ///< Time position of this keyframe in seconds
	struct vec4 value; ///< Animated value (up to 4 components for vectors/colors)
	struct vec4 inSlope; ///< Incoming tangent slope for bezier interpolation
	struct vec4 outSlope; ///< Outgoing tangent slope for bezier interpolation
	struct vec4 inWeight; ///< Incoming tangent weight for weighted bezier interpolation
	struct vec4 outWeight; ///< Outgoing tangent weight for weighted bezier interpolation
	int32_t tangentMode; ///< Interpolation mode for this keyframe (free, auto, linear, constant)
	int32_t weightedMode; ///< Whether tangent weights are used for this keyframe
};
ORCA_API void luaX_pushKeyframe(lua_State *L, struct Keyframe const* Keyframe);
ORCA_API struct Keyframe* luaX_checkKeyframe(lua_State *L, int idx);
/// @brief A single entry in a Locale resource, representing a localized string or asset reference.
/** LocaleEntry struct */
struct LocaleEntry {
	const char* Key; ///< The unique key identifying this locale entry (e.g. "MainMenu.StartButton")
	const char* Value; ///< The localized string value or asset path for this entry
	enum LocaleEntryType Type; ///< Indicates how to interpret the Value field (e.g. Text, Resource)
};
ORCA_API void luaX_pushLocaleEntry(lua_State *L, struct LocaleEntry const* LocaleEntry);
ORCA_API struct LocaleEntry* luaX_checkLocaleEntry(lua_State *L, int idx);
/// @brief A single entry resource, representing an asset reference.
/** ResourceEntry struct */
struct ResourceEntry {
	const char* Key; ///< The unique key identifying this resource entry
	const char* Value; ///< The localized string value or asset path for this entry
	enum ResourceEntryType Type; ///< Indicates how to interpret the Value field (e.g. Text, Resource)
};
ORCA_API void luaX_pushResourceEntry(lua_State *L, struct ResourceEntry const* ResourceEntry);
ORCA_API struct ResourceEntry* luaX_checkResourceEntry(lua_State *L, int idx);
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

/** Object_CreateEventArgs struct */
struct Object_CreateEventArgs {
};
ORCA_API void luaX_pushObject_CreateEventArgs(lua_State *L, struct Object_CreateEventArgs const* data);
ORCA_API struct Object_CreateEventArgs* luaX_checkObject_CreateEventArgs(lua_State *L, int idx);
/** Object_StartEventArgs struct */
struct Object_StartEventArgs {
};
ORCA_API void luaX_pushObject_StartEventArgs(lua_State *L, struct Object_StartEventArgs const* data);
ORCA_API struct Object_StartEventArgs* luaX_checkObject_StartEventArgs(lua_State *L, int idx);
/** Object_AnimateEventArgs struct */
struct Object_AnimateEventArgs {
};
ORCA_API void luaX_pushObject_AnimateEventArgs(lua_State *L, struct Object_AnimateEventArgs const* data);
ORCA_API struct Object_AnimateEventArgs* luaX_checkObject_AnimateEventArgs(lua_State *L, int idx);
/** StyleController_ThemeChangedEventArgs struct */
struct StyleController_ThemeChangedEventArgs {
	bool_t recursive; ///< When TRUE, also sends ThemeChanged to every direct child
};
ORCA_API void luaX_pushStyleController_ThemeChangedEventArgs(lua_State *L, struct StyleController_ThemeChangedEventArgs const* data);
ORCA_API struct StyleController_ThemeChangedEventArgs* luaX_checkStyleController_ThemeChangedEventArgs(lua_State *L, int idx);
/** Object_PropertyChangedEventArgs struct */
struct Object_PropertyChangedEventArgs {
	struct Property* Property; ///< The property that changed
};
ORCA_API void luaX_pushObject_PropertyChangedEventArgs(lua_State *L, struct Object_PropertyChangedEventArgs const* data);
ORCA_API struct Object_PropertyChangedEventArgs* luaX_checkObject_PropertyChangedEventArgs(lua_State *L, int idx);
/** Object_AttachedEventArgs struct */
struct Object_AttachedEventArgs {
};
ORCA_API void luaX_pushObject_AttachedEventArgs(lua_State *L, struct Object_AttachedEventArgs const* data);
ORCA_API struct Object_AttachedEventArgs* luaX_checkObject_AttachedEventArgs(lua_State *L, int idx);
/** Object_ReleaseEventArgs struct */
struct Object_ReleaseEventArgs {
};
ORCA_API void luaX_pushObject_ReleaseEventArgs(lua_State *L, struct Object_ReleaseEventArgs const* data);
ORCA_API struct Object_ReleaseEventArgs* luaX_checkObject_ReleaseEventArgs(lua_State *L, int idx);
/** Object_DestroyEventArgs struct */
struct Object_DestroyEventArgs {
};
ORCA_API void luaX_pushObject_DestroyEventArgs(lua_State *L, struct Object_DestroyEventArgs const* data);
ORCA_API struct Object_DestroyEventArgs* luaX_checkObject_DestroyEventArgs(lua_State *L, int idx);
/** Object_TimerEventArgs struct */
struct Object_TimerEventArgs {
	int32_t timerId; ///< Timer identifier returned by setTimer
};
ORCA_API void luaX_pushObject_TimerEventArgs(lua_State *L, struct Object_TimerEventArgs const* data);
ORCA_API struct Object_TimerEventArgs* luaX_checkObject_TimerEventArgs(lua_State *L, int idx);
/** AnimationPlayer_PlayEventArgs struct */
struct AnimationPlayer_PlayEventArgs {
	const char* Name; ///< Name of the clip to play (optional; if set, selects a clip from the Clips array before starting)
};
ORCA_API void luaX_pushAnimationPlayer_PlayEventArgs(lua_State *L, struct AnimationPlayer_PlayEventArgs const* data);
ORCA_API struct AnimationPlayer_PlayEventArgs* luaX_checkAnimationPlayer_PlayEventArgs(lua_State *L, int idx);
/** AnimationPlayer_ResumeEventArgs struct */
struct AnimationPlayer_ResumeEventArgs {
};
ORCA_API void luaX_pushAnimationPlayer_ResumeEventArgs(lua_State *L, struct AnimationPlayer_ResumeEventArgs const* data);
ORCA_API struct AnimationPlayer_ResumeEventArgs* luaX_checkAnimationPlayer_ResumeEventArgs(lua_State *L, int idx);
/** AnimationPlayer_StopEventArgs struct */
struct AnimationPlayer_StopEventArgs {
};
ORCA_API void luaX_pushAnimationPlayer_StopEventArgs(lua_State *L, struct AnimationPlayer_StopEventArgs const* data);
ORCA_API struct AnimationPlayer_StopEventArgs* luaX_checkAnimationPlayer_StopEventArgs(lua_State *L, int idx);
/** AnimationPlayer_PauseEventArgs struct */
struct AnimationPlayer_PauseEventArgs {
};
ORCA_API void luaX_pushAnimationPlayer_PauseEventArgs(lua_State *L, struct AnimationPlayer_PauseEventArgs const* data);
ORCA_API struct AnimationPlayer_PauseEventArgs* luaX_checkAnimationPlayer_PauseEventArgs(lua_State *L, int idx);
/** AnimationPlayer_StartedEventArgs struct */
struct AnimationPlayer_StartedEventArgs {
};
ORCA_API void luaX_pushAnimationPlayer_StartedEventArgs(lua_State *L, struct AnimationPlayer_StartedEventArgs const* data);
ORCA_API struct AnimationPlayer_StartedEventArgs* luaX_checkAnimationPlayer_StartedEventArgs(lua_State *L, int idx);
/** AnimationPlayer_StoppedEventArgs struct */
struct AnimationPlayer_StoppedEventArgs {
};
ORCA_API void luaX_pushAnimationPlayer_StoppedEventArgs(lua_State *L, struct AnimationPlayer_StoppedEventArgs const* data);
ORCA_API struct AnimationPlayer_StoppedEventArgs* luaX_checkAnimationPlayer_StoppedEventArgs(lua_State *L, int idx);
/** AnimationPlayer_CompletedEventArgs struct */
struct AnimationPlayer_CompletedEventArgs {
};
ORCA_API void luaX_pushAnimationPlayer_CompletedEventArgs(lua_State *L, struct AnimationPlayer_CompletedEventArgs const* data);
ORCA_API struct AnimationPlayer_CompletedEventArgs* luaX_checkAnimationPlayer_CompletedEventArgs(lua_State *L, int idx);
/** StyleController_AddClassEventArgs struct */
struct StyleController_AddClassEventArgs {
	const char* ClassName; ///< Name of the class to add
};
ORCA_API void luaX_pushStyleController_AddClassEventArgs(lua_State *L, struct StyleController_AddClassEventArgs const* data);
ORCA_API struct StyleController_AddClassEventArgs* luaX_checkStyleController_AddClassEventArgs(lua_State *L, int idx);
/** StyleController_AddClassesEventArgs struct */
struct StyleController_AddClassesEventArgs {
	const char* ClassNames; ///< Space-separated list of class names to add
};
ORCA_API void luaX_pushStyleController_AddClassesEventArgs(lua_State *L, struct StyleController_AddClassesEventArgs const* data);
ORCA_API struct StyleController_AddClassesEventArgs* luaX_checkStyleController_AddClassesEventArgs(lua_State *L, int idx);
/** StateManagerController_ControllerChangedEventArgs struct */
struct StateManagerController_ControllerChangedEventArgs {
	struct Property* Property; ///< The property whose value changed, triggering state group evaluation
};
ORCA_API void luaX_pushStateManagerController_ControllerChangedEventArgs(lua_State *L, struct StateManagerController_ControllerChangedEventArgs const* data);
ORCA_API struct StateManagerController_ControllerChangedEventArgs* luaX_checkStateManagerController_ControllerChangedEventArgs(lua_State *L, int idx);
/** Trigger_TriggeredEventArgs struct */
struct Trigger_TriggeredEventArgs {
	struct Trigger* Trigger;
};
ORCA_API void luaX_pushTrigger_TriggeredEventArgs(lua_State *L, struct Trigger_TriggeredEventArgs const* data);
ORCA_API struct Trigger_TriggeredEventArgs* luaX_checkTrigger_TriggeredEventArgs(lua_State *L, int idx);
/** Node_UpdateMatrixEventArgs struct */
struct Node_UpdateMatrixEventArgs {
	struct mat4 parent; ///< The parent matrix
	float opacity; ///< The opacity value
	bool_t force; ///< Indicates if the update is forced
};
ORCA_API void luaX_pushNode_UpdateMatrixEventArgs(lua_State *L, struct Node_UpdateMatrixEventArgs const* data);
ORCA_API struct Node_UpdateMatrixEventArgs* luaX_checkNode_UpdateMatrixEventArgs(lua_State *L, int idx);
/** Node_LoadViewEventArgs struct */
struct Node_LoadViewEventArgs {
	struct lua_State* lua_state;
};
ORCA_API void luaX_pushNode_LoadViewEventArgs(lua_State *L, struct Node_LoadViewEventArgs const* data);
ORCA_API struct Node_LoadViewEventArgs* luaX_checkNode_LoadViewEventArgs(lua_State *L, int idx);
/** Node_HitTestEventArgs struct */
struct Node_HitTestEventArgs {
	int32_t x; ///< X coordinate of the hit test point
	int32_t y; ///< Y coordinate of the hit test point
};
ORCA_API void luaX_pushNode_HitTestEventArgs(lua_State *L, struct Node_HitTestEventArgs const* data);
ORCA_API struct Node_HitTestEventArgs* luaX_checkNode_HitTestEventArgs(lua_State *L, int idx);
/** Node_IsVisibleEventArgs struct */
struct Node_IsVisibleEventArgs {
};
ORCA_API void luaX_pushNode_IsVisibleEventArgs(lua_State *L, struct Node_IsVisibleEventArgs const* data);
ORCA_API struct Node_IsVisibleEventArgs* luaX_checkNode_IsVisibleEventArgs(lua_State *L, int idx);
/** Node_ViewDidLoadEventArgs struct */
struct Node_ViewDidLoadEventArgs {
};
ORCA_API void luaX_pushNode_ViewDidLoadEventArgs(lua_State *L, struct Node_ViewDidLoadEventArgs const* data);
ORCA_API struct Node_ViewDidLoadEventArgs* luaX_checkNode_ViewDidLoadEventArgs(lua_State *L, int idx);
/** Node_KillFocusEventArgs struct */
struct Node_KillFocusEventArgs {
};
ORCA_API void luaX_pushNode_KillFocusEventArgs(lua_State *L, struct Node_KillFocusEventArgs const* data);
ORCA_API struct Node_KillFocusEventArgs* luaX_checkNode_KillFocusEventArgs(lua_State *L, int idx);
/** Node_SetFocusEventArgs struct */
struct Node_SetFocusEventArgs {
};
ORCA_API void luaX_pushNode_SetFocusEventArgs(lua_State *L, struct Node_SetFocusEventArgs const* data);
ORCA_API struct Node_SetFocusEventArgs* luaX_checkNode_SetFocusEventArgs(lua_State *L, int idx);
/** Node_GetSizeEventArgs struct */
struct Node_GetSizeEventArgs {
};
ORCA_API void luaX_pushNode_GetSizeEventArgs(lua_State *L, struct Node_GetSizeEventArgs const* data);
ORCA_API struct Node_GetSizeEventArgs* luaX_checkNode_GetSizeEventArgs(lua_State *L, int idx);
/** Node_MouseMessageEventArgs struct */
struct Node_MouseMessageEventArgs {
	float x; ///< Horizontal pointer position in local space
	float y; ///< Vertical pointer position in local space
	int32_t deltaX; ///< Scroll wheel rotation amount along the X axis; positive values scroll right/forward
	int32_t deltaY; ///< Scroll wheel rotation amount along the Y axis; positive values scroll up/forward
	enum MouseButton button; ///< The mouse button involved in this event
	int32_t clickCount; ///< Number of consecutive clicks (1 for single click, 2 for double click)
};
ORCA_API void luaX_pushNode_MouseMessageEventArgs(lua_State *L, struct Node_MouseMessageEventArgs const* data);
ORCA_API struct Node_MouseMessageEventArgs* luaX_checkNode_MouseMessageEventArgs(lua_State *L, int idx);
/** Node_KeyMessageEventArgs struct */
struct Node_KeyMessageEventArgs {
	int32_t keyCode; ///< The key involved in this event
	int32_t character; ///< The character produced by this key event
	int32_t modifiers; ///< Active modifier keys during the event (bitmask of AX_MOD_* flags)
	const char* text; ///< The text produced by this key event as a UTF-8 string (empty for non-printable keys)
	const char* modifiersString; ///< Active modifier keys as a human-readable prefix string (e.g. "ctrl+shift+")
	const char* hotKey; ///< Combined modifier+key string for hotkey matching (e.g. "ctrl+alt+w")
};
ORCA_API void luaX_pushNode_KeyMessageEventArgs(lua_State *L, struct Node_KeyMessageEventArgs const* data);
ORCA_API struct Node_KeyMessageEventArgs* luaX_checkNode_KeyMessageEventArgs(lua_State *L, int idx);


/// @brief A single animated property curve, consisting of keyframes for one property on one target object.
/** AnimationCurve component */
typedef struct AnimationCurve AnimationCurve_t, *AnimationCurvePtr, *lpAnimationCurve_t;
typedef struct AnimationCurve const *AnimationCurveCPtr, *lpcAnimationCurve_t;
struct AnimationCurve {
	const char* Path; ///< Relative path from the AnimationClip's host object to the target object (empty string means the host object itself)
	const char* Property; ///< Short name of the property to animate (e.g. "Opacity", "Position")
	struct Keyframe* Keyframes; ///< Array of keyframes that define this curve
	int32_t NumKeyframes;
};
ORCA_API void luaX_pushAnimationCurve(lua_State *L, struct AnimationCurve const* AnimationCurve);
ORCA_API struct AnimationCurve* luaX_checkAnimationCurve(lua_State *L, int idx);

/// @brief A reusable, self-loading animation asset containing one or more AnimationCurve components.
/** AnimationClip component */
typedef struct AnimationClip AnimationClip_t, *AnimationClipPtr, *lpAnimationClip_t;
typedef struct AnimationClip const *AnimationClipCPtr, *lpcAnimationClip_t;
struct AnimationClip {
	enum AnimationMode Mode; ///< Playback mode: PlayOnce stops at end, Loop restarts, PingPong alternates direction
	float StartTime; ///< Start time of the active region in seconds (default 0)
	float StopTime; ///< End time of the active region in seconds
};
ORCA_API void luaX_pushAnimationClip(lua_State *L, struct AnimationClip const* AnimationClip);
ORCA_API struct AnimationClip* luaX_checkAnimationClip(lua_State *L, int idx);

/// @brief Component for managing and playing animations on UI elements.
/** AnimationPlayer component */
typedef struct AnimationPlayer AnimationPlayer_t, *AnimationPlayerPtr, *lpAnimationPlayer_t;
typedef struct AnimationPlayer const *AnimationPlayerCPtr, *lpcAnimationPlayer_t;
struct AnimationPlayer {
	struct AnimationClip* Clip; ///< The AnimationClip object that supplies the curves to animate
	struct AnimationClipReference* Clips; ///< Array of named animation clips available to this player; use the Play message with a Name to select one by name
	int32_t NumClips;
	bool_t Playing; ///< Whether the animation is currently advancing each frame
	bool_t Looping; ///< If true, the animation restarts from StartTime when it reaches StopTime
	float Speed; ///< Playback speed multiplier (1.0 = normal speed)
	float CurrentTime; ///< Current playback position within the animation in seconds
	bool_t AutoplayEnabled; ///< Enables or disables automatic playback on initialization.
	bool_t RelativePlayback; ///< When true, playback is relative to the current state rather than starting from an absolute base state.
	bool_t RestoreOriginalValuesAfterPlayback; ///< Restores the original property values after the animation ends.
	enum PlaybackMode PlaybackMode; ///< Defines how the animation should be played.
	float DurationScale; ///< Scales the playback speed of the animation. A value greater than 1.0 speeds up playback, while values between 0.0 and 1.0 slow it down.
	int32_t RepeatCount; ///< Number of times the animation should repeat. Use -1 to indicate infinite looping.
	long _prevRealtime; ///< The last real time when the animation was updated, used to calculate delta time for advancing CurrentTime
	event_t Play;
	event_t Resume;
	event_t Stop;
	event_t Pause;
	event_t Started;
	event_t Stopped;
	event_t Completed;
};
ORCA_API void luaX_pushAnimationPlayer(lua_State *L, struct AnimationPlayer const* AnimationPlayer);
ORCA_API struct AnimationPlayer* luaX_checkAnimationPlayer(lua_State *L, int idx);

/// @brief Attach-only component that tweens one property value to a target, then self-destructs.
/** PropertyAnimation component */
typedef struct PropertyAnimation PropertyAnimation_t, *PropertyAnimationPtr, *lpPropertyAnimation_t;
typedef struct PropertyAnimation const *PropertyAnimationCPtr, *lpcPropertyAnimation_t;
struct PropertyAnimation {
	const char* Property; ///< Property to animate
	const char* From; ///< Start value (raw bytes)
	const char* To; ///< End value (raw bytes)
	enum InterpolationMode Interpolation; ///< Interpolation method
	enum Easing Easing; ///< Easing function
	int32_t Start; ///< Timestamp when animation started
	int32_t Duration; ///< Duration in milliseconds
	struct Property* _property; ///< The property being animated
};
ORCA_API void luaX_pushPropertyAnimation(lua_State *L, struct PropertyAnimation const* PropertyAnimation);
ORCA_API struct PropertyAnimation* luaX_checkPropertyAnimation(lua_State *L, int idx);

/// @brief Container object for StyleRule definitions.
/** StyleSheet component */
typedef struct StyleSheet StyleSheet_t, *StyleSheetPtr, *lpStyleSheet_t;
typedef struct StyleSheet const *StyleSheetCPtr, *lpcStyleSheet_t;
struct StyleSheet {
};
ORCA_API void luaX_pushStyleSheet(lua_State *L, struct StyleSheet const* StyleSheet);
ORCA_API struct StyleSheet* luaX_checkStyleSheet(lua_State *L, int idx);

/// @brief A single style rule that maps a class name (with optional pseudo-class) to property overrides.
/** StyleRule component */
typedef struct StyleRule StyleRule_t, *StyleRulePtr, *lpStyleRule_t;
typedef struct StyleRule const *StyleRuleCPtr, *lpcStyleRule_t;
struct StyleRule {
	const char* ClassName; ///< Base class name without leading dot (e.g., "button" for a rule matching nodes with class "button").
	const char* PseudoClass; ///< Colon-separated pseudo-state qualifiers (e.g., "hover" or "hover:focus"). Empty string means the rule applies to all states.
	uint32_t class_id; ///< FNV1a hash of ClassName (cached for fast matching)
	uint32_t flags; ///< Pseudo-state bitmask derived from PseudoClass (STYLE_HOVER, STYLE_FOCUS, etc.)
};
ORCA_API void luaX_pushStyleRule(lua_State *L, struct StyleRule const* StyleRule);
ORCA_API struct StyleRule* luaX_checkStyleRule(lua_State *L, int idx);

/// @brief Manages CSS-style classes and stylesheet rules for an object.
/** StyleController component */
typedef struct StyleController StyleController_t, *StyleControllerPtr, *lpStyleController_t;
typedef struct StyleController const *StyleControllerCPtr, *lpcStyleController_t;
struct StyleController {
	struct StyleSheet* StyleSheet; ///< The StyleSheet object that supplies style rules for this object. Rules are matched by selector against the object's class list. If NULL, only the global static stylesheet is used.
	struct style_class_selector* classes; ///< Linked list of parsed style classes with flags (hover, focus, dark mode, etc.)
	bool_t owned_sheet; ///< TRUE when StyleSheet was created internally and must be released on clear. FALSE when it was assigned externally (e.g. from Lua).
	event_t ThemeChanged;
	event_t AddClass;
	event_t AddClasses;
};
ORCA_API void luaX_pushStyleController(lua_State *L, struct StyleController const* StyleController);
ORCA_API struct StyleController* luaX_checkStyleController(lua_State *L, int idx);

/// @brief Container object for StateGroup definitions.
/** StateManager component */
typedef struct StateManager StateManager_t, *StateManagerPtr, *lpStateManager_t;
typedef struct StateManager const *StateManagerCPtr, *lpcStateManager_t;
struct StateManager {
};
ORCA_API void luaX_pushStateManager(lua_State *L, struct StateManager const* StateManager);
ORCA_API struct StateManager* luaX_checkStateManager(lua_State *L, int idx);

/// @brief Tracks one controller property and maps its values to State objects.
/** StateGroup component */
typedef struct StateGroup StateGroup_t, *StateGroupPtr, *lpStateGroup_t;
typedef struct StateGroup const *StateGroupCPtr, *lpcStateGroup_t;
struct StateGroup {
	const char* ControllerProperty; ///< Short name of the property on the host object to watch (e.g. "IsActive")
};
ORCA_API void luaX_pushStateGroup(lua_State *L, struct StateGroup const* StateGroup);
ORCA_API struct StateGroup* luaX_checkStateGroup(lua_State *L, int idx);

/// @brief Defines property overrides to apply when the parent StateGroup's controller property matches Value.
/** State component */
typedef struct State State_t, *StatePtr, *lpState_t;
typedef struct State const *StateCPtr, *lpcState_t;
struct State {
	const char* Value; ///< The value of the controller property that activates this state (compared as a number; for booleans "true"/"false" are also accepted)
	const char* Path; ///< Optional path from the host object to the target object. Empty string means the host object itself.
};
ORCA_API void luaX_pushState(lua_State *L, struct State const* State);
ORCA_API struct State* luaX_checkState(lua_State *L, int idx);

/// @brief Attach-only component that applies state-driven property overrides from a StateManager object.
/** StateManagerController component */
typedef struct StateManagerController StateManagerController_t, *StateManagerControllerPtr, *lpStateManagerController_t;
typedef struct StateManagerController const *StateManagerControllerCPtr, *lpcStateManagerController_t;
struct StateManagerController {
	struct StateManager* StateManager; ///< The StateManager object that supplies the StateGroup definitions. Setting this property automatically marks all tracked controller properties.
	event_t ControllerChanged;
};
ORCA_API void luaX_pushStateManagerController(lua_State *L, struct StateManagerController const* StateManagerController);
ORCA_API struct StateManagerController* luaX_checkStateManagerController(lua_State *L, int idx);

/// @brief Base class for resource containers such as aliases and locale entries.
/** ResourceDictionary component */
typedef struct ResourceDictionary ResourceDictionary_t, *ResourceDictionaryPtr, *lpResourceDictionary_t;
typedef struct ResourceDictionary const *ResourceDictionaryCPtr, *lpcResourceDictionary_t;
struct ResourceDictionary {
};
ORCA_API void luaX_pushResourceDictionary(lua_State *L, struct ResourceDictionary const* ResourceDictionary);
ORCA_API struct ResourceDictionary* luaX_checkResourceDictionary(lua_State *L, int idx);

/// @brief Localization resource component that provides text and resource strings for a given language.
/** Locale component */
typedef struct Locale Locale_t, *LocalePtr, *lpLocale_t;
typedef struct Locale const *LocaleCPtr, *lpcLocale_t;
struct Locale {
	const char* Language; ///< BCP-47 language code for this locale (e.g. "en", "fr-CA")
	struct LocaleEntry* Entries; ///< Array of locale entries contained in this locale
	int32_t NumEntries;
};
ORCA_API void luaX_pushLocale(lua_State *L, struct Locale const* Locale);
ORCA_API struct Locale* luaX_checkLocale(lua_State *L, int idx);

/// @brief Base class for data-holding objects within the framework
/** DataObject component */
typedef struct DataObject DataObject_t, *DataObjectPtr, *lpDataObject_t;
typedef struct DataObject const *DataObjectCPtr, *lpcDataObject_t;
struct DataObject {
};
ORCA_API void luaX_pushDataObject(lua_State *L, struct DataObject const* DataObject);
ORCA_API struct DataObject* luaX_checkDataObject(lua_State *L, int idx);

/// @brief Base class for event-driven actions within the UI system
/** Trigger component */
typedef struct Trigger Trigger_t, *TriggerPtr, *lpTrigger_t;
typedef struct Trigger const *TriggerCPtr, *lpcTrigger_t;
struct Trigger {
	const char* Property; ///< Target property name to monitor or modify
	int32_t Value; ///< Associated value for the trigger condition
	event_t Triggered;
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
	struct ResourceEntry* Resources; ///< Array of resources associated with this node. Can be aliases to objects or other resources.
	int32_t NumResources;
	long _tags; ///< Calculated tags value
	event_t UpdateMatrix;
	event_t LoadView;
	event_t HitTest;
	event_t IsVisible;
	event_t ViewDidLoad;
	event_t KillFocus;
	event_t SetFocus;
	event_t GetSize;
	event_t MouseMessage;
	event_t LeftButtonDown;
	event_t RightButtonDown;
	event_t OtherButtonDown;
	event_t LeftButtonUp;
	event_t RightButtonUp;
	event_t OtherButtonUp;
	event_t LeftButtonDragged;
	event_t RightButtonDragged;
	event_t OtherButtonDragged;
	event_t LeftDoubleClick;
	event_t RightDoubleClick;
	event_t OtherDoubleClick;
	event_t MouseMoved;
	event_t ScrollWheel;
	event_t DragDrop;
	event_t DragEnter;
	event_t KeyMessage;
	event_t KeyDown;
	event_t KeyUp;
	event_t TextInput;
};
ORCA_API void luaX_pushNode(lua_State *L, struct Node const* Node);
ORCA_API struct Node* luaX_checkNode(lua_State *L, int idx);

#endif