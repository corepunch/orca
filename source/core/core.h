// Auto-generated from core.xml by tools/templates/header.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __CORE_H__
#define __CORE_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;

struct WI_Message;
struct localization;
struct game;
struct Property;
struct KeyframeAnim;
struct lua_State;


#include "core_properties.h"
#include "../geometry/geometry.h"

typedef struct WI_Message MouseMessageMsg_t,* MouseMessageMsgPtr;
typedef struct WI_Message KeyMessageMsg_t,* KeyMessageMsgPtr;
typedef struct WI_Message LeftMouseDownMsg_t,* LeftMouseDownMsgPtr;
typedef struct WI_Message RightMouseDownMsg_t,* RightMouseDownMsgPtr;
typedef struct WI_Message OtherMouseDownMsg_t,* OtherMouseDownMsgPtr;
typedef struct WI_Message LeftMouseUpMsg_t,* LeftMouseUpMsgPtr;
typedef struct WI_Message RightMouseUpMsg_t,* RightMouseUpMsgPtr;
typedef struct WI_Message OtherMouseUpMsg_t,* OtherMouseUpMsgPtr;
typedef struct WI_Message LeftMouseDraggedMsg_t,* LeftMouseDraggedMsgPtr;
typedef struct WI_Message RightMouseDraggedMsg_t,* RightMouseDraggedMsgPtr;
typedef struct WI_Message OtherMouseDraggedMsg_t,* OtherMouseDraggedMsgPtr;
typedef struct WI_Message LeftDoubleClickMsg_t,* LeftDoubleClickMsgPtr;
typedef struct WI_Message RightDoubleClickMsg_t,* RightDoubleClickMsgPtr;
typedef struct WI_Message OtherDoubleClickMsg_t,* OtherDoubleClickMsgPtr;
typedef struct WI_Message MouseMovedMsg_t,* MouseMovedMsgPtr;
typedef struct WI_Message ScrollWheelMsg_t,* ScrollWheelMsgPtr;
typedef struct WI_Message DragDropMsg_t,* DragDropMsgPtr;
typedef struct WI_Message DragEnterMsg_t,* DragEnterMsgPtr;
typedef struct WI_Message KeyDownMsg_t,* KeyDownMsgPtr;
typedef struct WI_Message KeyUpMsg_t,* KeyUpMsgPtr;
typedef struct WI_Message CharMsg_t,* CharMsgPtr;
typedef struct WindowPaintMsgArgs WindowPaintMsg_t,* WindowPaintMsgPtr;
typedef struct WindowPaintMsgArgs WindowResizedMsg_t,* WindowResizedMsgPtr;
typedef void WindowClosedMsg_t,* WindowClosedMsgPtr;
typedef void WindowChangedScreenMsg_t,* WindowChangedScreenMsgPtr;
typedef void KillFocusMsg_t,* KillFocusMsgPtr;
typedef void SetFocusMsg_t,* SetFocusMsgPtr;
typedef void TimerMsg_t,* TimerMsgPtr;
typedef struct UpdateMatrixMsgArgs UpdateMatrixMsg_t,* UpdateMatrixMsgPtr;
typedef void HitTestMsg_t,* HitTestMsgPtr;
typedef void IsVisibleMsg_t,* IsVisibleMsgPtr;
typedef void CreateMsg_t,* CreateMsgPtr;
typedef void StartMsg_t,* StartMsgPtr;
typedef void AwakeMsg_t,* AwakeMsgPtr;
typedef void ThemeChangedMsg_t,* ThemeChangedMsgPtr;
typedef struct PropertyChangedMsgArgs PropertyChangedMsg_t,* PropertyChangedMsgPtr;
typedef void AttachedMsg_t,* AttachedMsgPtr;
typedef void ReleaseMsg_t,* ReleaseMsgPtr;
typedef void DestroyMsg_t,* DestroyMsgPtr;
typedef void ResumeCoroutineMsg_t,* ResumeCoroutineMsgPtr;
typedef void StopCoroutineMsg_t,* StopCoroutineMsgPtr;
typedef void ViewDidLoadMsg_t,* ViewDidLoadMsgPtr;

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
/// @brief Specifies the underlying data type of a property.
/** DataType enum */
typedef enum DataType {
	kDataTypeNone, ///< No data type specified.
	kDataTypeBool, ///< Boolean value representing true or false.
	kDataTypeInt, ///< Signed integer value.
	kDataTypeEnum, ///< Enumeration type represented by integer values mapped to named constants.
	kDataTypeFloat, ///< Floating-point numeric value.
	kDataTypeString, ///< String data, heap-allocated to support arbitrary length content.
	kDataTypeEvent, ///< Event reference used to bind triggers or callbacks.
	kDataTypeStruct, ///< Composite data structure containing multiple fields used for packaging related geometric, visual, and layout properties together.
	kDataTypeColor, ///< RGBA color value represented as four floating-point components (red, green, blue, alpha) in the range 0.0 to 1.0.
	kDataTypeObject, ///< Reference to a complex object instance.
} eDataType_t;
#define DataType_Count 10
ORCA_API const char *DataTypeToString(enum DataType value);
ORCA_API enum DataType luaX_checkDataType(lua_State *L, int idx);
ORCA_API void luaX_pushDataType(lua_State *L, enum DataType value);

typedef struct PropertyEnumValue PropertyEnumValue_t, *lpPropertyEnumValue_t;
typedef struct PropertyEnumValue const cPropertyEnumValue_t, *lpcPropertyEnumValue_t;
typedef struct PropertyType PropertyType_t, *lpPropertyType_t;
typedef struct PropertyType const cPropertyType_t, *lpcPropertyType_t;

/// @brief Retrieves currently active object.
ORCA_API struct Object*
core_GetFocus(void);
/// @brief Gets the currently hovered object
ORCA_API struct Object*
core_GetHover(void);

/// @name Lifecycle

/// Manages object creation, initialization, update cycles, and destruction.

/// @brief Create new object.
ORCA_API int
OBJ_CreateFromLuaState(struct lua_State*);
/// @brief Initializes the core component when it is loaded, an essential lifecycle method.
ORCA_API void
OBJ_Awake(struct lua_State*, struct Object*);
/// @brief Runs object animations.
ORCA_API void
OBJ_Animate(struct lua_State*, struct Object*);
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

/// @brief Dispatch an event starting from this object and bubbling up parents.
ORCA_API struct Object*
OBJ_DispatchEvent(struct lua_State*, struct Object*, const char*);
/// @brief Posts a message to the global message queue.
ORCA_API void
OBJ_PostMessage(struct lua_State*, struct Object*, const char*);
/// @brief Send a message to directly to the object ignoring queue.
ORCA_API void
OBJ_SendMessage2(struct lua_State*, struct Object*, const char*);
/// @brief Retrieves callback function name for event ID
ORCA_API const char*
OBJ_FindCallbackForID(struct Object*, uint32_t);
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

/// @brief Updates object layout.
ORCA_API void
OBJ_UpdateLayout(struct Object*, int32_t, int32_t);
/// @brief Sets object dirty and queues it for recalculation
ORCA_API void
OBJ_SetDirty(struct Object*);
/// @brief Clears dirty flags, marks object as recalculated
ORCA_API void
OBJ_ClearDirtyFlags(struct Object*);
/// @brief Applies style changes to object hierarchy
ORCA_API void
OBJ_ApplyStyles(struct Object*, bool_t);
/// @name Style

/// Manages style sheets and resolves computed style values.

/// @brief Add a stylesheet to the object.
ORCA_API void
OBJ_AddStyleSheet(struct lua_State*, struct Object*, const char*);
/// @brief Retrieves object style flags
ORCA_API uint32_t
OBJ_GetStyle(struct Object const*);
/// @brief Sets object style flags
ORCA_API void
OBJ_SetStyle(struct Object*, uint32_t);
/// @name Animation

/// Attaches, plays, and transitions keyframe animations on this object.

/// @brief Play an animation or resource on the object.
ORCA_API void
OBJ_Play(struct Object*, const char*);
/// @brief Tween an object property over time.
ORCA_API void
OBJ_DoTween(struct lua_State*, struct Object*);
/// @brief Sets the active animation by name
ORCA_API void
OBJ_SetAnimation(struct Object*, const char*);
/// @brief Gets the currently active animation
ORCA_API struct KeyframeAnim const*
OBJ_GetAnimation(struct Object const*);
/// @brief Adds a keyframe animation to the object's animation library
ORCA_API void
OBJ_AddAnimation(struct Object*, struct KeyframeAnim*);
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
ORCA_API int
OBJ_SetTimer(struct lua_State*, struct Object*);
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
/// @brief Gets the Lua object reference
ORCA_API uint32_t
OBJ_GetLuaObject(struct Object const*);
/// @brief Gets the domain of the object
ORCA_API struct lua_State*
OBJ_GetDomain(struct Object*);
/// @brief Set the current context object where newly created objects will be parented.
ORCA_API void
OBJ_SetContext(struct lua_State*, struct Object*);
/// @brief Parses and applies multiple class names from a class attribute string
ORCA_API void
OBJ_ParseClassAttribute(struct Object*, const char*);
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
/// @brief Registers an alias for a child object path
ORCA_API void
OBJ_AddAlias(struct Object*, const char*, const char*);
/// @brief Resolves and assigns all registered aliases for an object
ORCA_API void
OBJ_AssignAliases(struct Object*, const char*);

/// @brief Enum value descriptor for a property.
/** PropertyEnumValue struct */
struct PropertyEnumValue {
	fixedString_t Name; ///< Unique name identifier for the value.
	int32_t Value; ///< Integer value representing the enum.
};
ORCA_API void luaX_pushPropertyEnumValue(lua_State *L, struct PropertyEnumValue const* PropertyEnumValue);
ORCA_API struct PropertyEnumValue* luaX_checkPropertyEnumValue(lua_State *L, int idx);
/// @brief Defines a custom property type that can be attached to engine objects.
/** PropertyType struct */
struct PropertyType {
	fixedString_t Name; ///< Unique name identifier for the property type.
	fixedString_t Category; ///< Organizational category for this property, used for grouping in editors and UIs.
	enum DataType DataType; ///< Underlying data type that determines how the property value is interpreted and stored.
	fixedString_t DefaultValue; ///< Default value assigned when the property is not explicitly set.
	fixedString_t TypeString; ///< String representation of the property type, used to store struct/object type names.
	lpcString_t const* EnumValues; ///< Null-terminated array of enum value name strings for kDataTypeEnum properties.
	bool_t AffectLayout; ///< Indicates whether this property affects element layout (e.g., size or alignment).
	bool_t AffectRender; ///< Indicates whether this property influences the rendering output.
	bool_t IsReadOnly; ///< If true, the property value cannot be modified at runtime or through the editor.
	bool_t IsHidden; ///< If true, the property is excluded from the UI or inspector views.
	bool_t IsInherited; ///< Specifies whether the property value can be inherited from parent components.
	fixedString_t Key; ///< Internal key name used for property identification and lookup.
	fixedString_t Value; ///< Runtime value stored in this property instance.
	float Step; ///< Increment step used for numeric adjustments in UI controls.
	float UpperBound; ///< Maximum allowed value for numeric properties.
	float LowerBound; ///< Minimum allowed value for numeric properties.
	uint32_t ShortIdentifier; ///< Unique short identifier for the property type, automatically generated from implicit property name.
	uint32_t FullIdentifier; ///< Unique full identifier for the property type, automatically generated from explicit (ie. Grid.Columns) property name.
	uint32_t Offset; ///< Byte offset of the property within the structure.
	uint32_t DataSize; ///< Size of the property data in bytes.
	bool_t IsArray; ///< Indicates whether the property is an array type, will generate Num* property to indicate the number of elements.
};
ORCA_API void luaX_pushPropertyType(lua_State *L, struct PropertyType const* PropertyType);
ORCA_API struct PropertyType* luaX_checkPropertyType(lua_State *L, int idx);

/** WindowPaintMsgArgs struct */
struct WindowPaintMsgArgs {
	uint32_t WindowWidth;
	uint32_t WindowHeight;
};
ORCA_API void luaX_pushWindowPaintMsgArgs(lua_State *L, struct WindowPaintMsgArgs const* data);
ORCA_API struct WindowPaintMsgArgs* luaX_checkWindowPaintMsgArgs(lua_State *L, int idx);
/** UpdateMatrixMsgArgs struct */
struct UpdateMatrixMsgArgs {
	struct mat4 parent; ///< The parent matrix
	float opacity; ///< The opacity value
	bool_t force; ///< Indicates if the update is forced
};
ORCA_API void luaX_pushUpdateMatrixMsgArgs(lua_State *L, struct UpdateMatrixMsgArgs const* data);
ORCA_API struct UpdateMatrixMsgArgs* luaX_checkUpdateMatrixMsgArgs(lua_State *L, int idx);
/** PropertyChangedMsgArgs struct */
struct PropertyChangedMsgArgs {
	struct Property* Property; ///< The property that changed
};
ORCA_API void luaX_pushPropertyChangedMsgArgs(lua_State *L, struct PropertyChangedMsgArgs const* data);
ORCA_API struct PropertyChangedMsgArgs* luaX_checkPropertyChangedMsgArgs(lua_State *L, int idx);


#endif