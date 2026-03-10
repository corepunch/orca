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

typedef struct WI_Message* LeftMouseDownEventPtr;
typedef struct WI_Message* RightMouseDownEventPtr;
typedef struct WI_Message* OtherMouseDownEventPtr;
typedef struct WI_Message* LeftMouseUpEventPtr;
typedef struct WI_Message* RightMouseUpEventPtr;
typedef struct WI_Message* OtherMouseUpEventPtr;
typedef struct WI_Message* LeftMouseDraggedEventPtr;
typedef struct WI_Message* RightMouseDraggedEventPtr;
typedef struct WI_Message* OtherMouseDraggedEventPtr;
typedef struct WI_Message* LeftDoubleClickEventPtr;
typedef struct WI_Message* RightDoubleClickEventPtr;
typedef struct WI_Message* OtherDoubleClickEventPtr;
typedef struct WI_Message* MouseMovedEventPtr;
typedef struct WI_Message* ScrollWheelEventPtr;
typedef struct WI_Message* DragDropEventPtr;
typedef struct WI_Message* DragEnterEventPtr;
typedef struct WI_Message* KeyDownEventPtr;
typedef struct WI_Message* KeyUpEventPtr;
typedef struct WI_Message* CharEventPtr;
typedef void* WindowPaintEventPtr;
typedef void* WindowClosedEventPtr;
typedef void* WindowResizedEventPtr;
typedef void* WindowChangedScreenEventPtr;
typedef void* KillFocusEventPtr;
typedef void* SetFocusEventPtr;
typedef void* TimerEventPtr;
typedef struct UpdateMatrixEventArgs* UpdateMatrixEventPtr;
typedef void* HitTestEventPtr;
typedef void* IsVisibleEventPtr;
typedef void* CreateEventPtr;
typedef void* StartEventPtr;
typedef void* AwakeEventPtr;
typedef void* ThemeChangedEventPtr;
typedef struct PropertyChangedEventArgs* PropertyChangedEventPtr;
typedef void* AttachedEventPtr;
typedef void* ReleaseEventPtr;
typedef void* DestroyEventPtr;
typedef void* ResumeCoroutineEventPtr;
typedef void* StopCoroutineEventPtr;
typedef void* ViewDidLoadEventPtr;

/** BindingMode enum */
typedef enum BindingMode {
	kBindingModeOneWay, // Default mode, updates target property when source changes
	kBindingModeTwoWay, // Updates target property when source changes and updates source when target changes
	kBindingModeOneWayToSource, // Updates source property when target changes, but not the other way around
	kBindingModeExpression, // Allows binding to a custom expression
} eBindingMode_t;
#define BindingMode_Count 4
ORCA_API const char *BindingModeToString(enum BindingMode value);
ORCA_API enum BindingMode luaX_checkBindingMode(lua_State *L, int idx);
ORCA_API void luaX_pushBindingMode(lua_State *L, enum BindingMode value);
/** PropertyAttribute enum */
typedef enum PropertyAttribute {
	kPropertyAttributeWholeProperty, // Default binding to the whole property
	kPropertyAttributeColorR, // Bind to the red channel of a color property
	kPropertyAttributeColorG, // Bind to the green channel of a color property
	kPropertyAttributeColorB, // Bind to the blue channel of a color property
	kPropertyAttributeColorA, // Bind to the alpha channel of a color property
	kPropertyAttributeVectorX, // Bind to the X component of a vector property
	kPropertyAttributeVectorY, // Bind to the Y component of a vector property
	kPropertyAttributeVectorZ, // Bind to the Z component of a vector property
	kPropertyAttributeVectorW, // Bind to the W component of a vector property
} ePropertyAttribute_t;
#define PropertyAttribute_Count 9
ORCA_API const char *PropertyAttributeToString(enum PropertyAttribute value);
ORCA_API enum PropertyAttribute luaX_checkPropertyAttribute(lua_State *L, int idx);
ORCA_API void luaX_pushPropertyAttribute(lua_State *L, enum PropertyAttribute value);
/** DataType enum */
typedef enum DataType {
	kDataTypeNone, // No data type specified.
	kDataTypeBool, // Boolean value representing true or false.
	kDataTypeInt, // Signed integer value.
	kDataTypeEnum, // Enumeration type represented by integer values mapped to named constants.
	kDataTypeFloat, // Floating-point numeric value.
	kDataTypeFixed, // Fixed-length string or symbolic value.
	kDataTypeLongString, // Extended string data, intended for larger text content.
	kDataTypeObjectTags, // List of tag identifiers associated with an object.
	kDataTypeEvent, // Event reference used to bind triggers or callbacks.
	kDataTypeStruct, // Composite data structure containing multiple fields used for packaging related geometric, visual, and layout properties together.
	kDataTypeObject, // Reference to a complex object instance.
} eDataType_t;
#define DataType_Count 11
ORCA_API const char *DataTypeToString(enum DataType value);
ORCA_API enum DataType luaX_checkDataType(lua_State *L, int idx);
ORCA_API void luaX_pushDataType(lua_State *L, enum DataType value);

typedef struct PropertyChangedEventArgs PropertyChangedEventArgs_t, *lpPropertyChangedEventArgs_t;
typedef struct PropertyChangedEventArgs const cPropertyChangedEventArgs_t, *lpcPropertyChangedEventArgs_t;
typedef struct UpdateMatrixEventArgs UpdateMatrixEventArgs_t, *lpUpdateMatrixEventArgs_t;
typedef struct UpdateMatrixEventArgs const cUpdateMatrixEventArgs_t, *lpcUpdateMatrixEventArgs_t;
typedef struct PropertyEnumValue PropertyEnumValue_t, *lpPropertyEnumValue_t;
typedef struct PropertyEnumValue const cPropertyEnumValue_t, *lpcPropertyEnumValue_t;
typedef struct PropertyType PropertyType_t, *lpPropertyType_t;
typedef struct PropertyType const cPropertyType_t, *lpcPropertyType_t;

ORCA_API struct Object*
core_GetFocus(void);
ORCA_API struct Object*
core_GetHover(void);

ORCA_API void
OBJ_Clear(struct lua_State*, struct Object*);
ORCA_API void
OBJ_Release(struct lua_State*, struct Object*);
ORCA_API bool_t
OBJ_Equals(struct Object const*, struct Object const*);
ORCA_API void
OBJ_Awake(struct lua_State*, struct Object*);
ORCA_API void
OBJ_Animate(struct lua_State*, struct Object*);
ORCA_API void
OBJ_LoadPrefabs(struct lua_State*, struct Object*);
ORCA_API void
OBJ_EmitPropertyChangedEvents(struct lua_State*, struct Object*);
ORCA_API void
OBJ_UpdateProperties(struct Object*);
ORCA_API void
OBJ_UpdateLayout(struct Object*, int32_t, int32_t);
ORCA_API void
OBJ_AddChild(struct Object*, struct Object*, bool_t);
ORCA_API void
OBJ_RemoveFromParent(struct lua_State*, struct Object*, bool_t);
ORCA_API bool_t
OBJ_SetProperty(struct lua_State*, struct Object*, const char*);
ORCA_API int
OBJ_GetProperty(struct lua_State*, struct Object*, const char*);
ORCA_API struct Object*
OBJ_FindChild(struct Object*, const char*, bool_t);
ORCA_API struct Object*
OBJ_DispatchEvent(struct lua_State*, struct Object*, const char*);
ORCA_API void
OBJ_PostMessage(struct lua_State*, struct Object*, const char*);
ORCA_API void
OBJ_Play(struct Object*, const char*);
ORCA_API void
OBJ_SetFocus(struct Object*);
ORCA_API void
OBJ_DoTween(struct lua_State*, struct Object*);
ORCA_API void
OBJ_AddStyleSheet(struct lua_State*, struct Object*, const char*);
ORCA_API int
OBJ_SetTimer(struct lua_State*, struct Object*);
ORCA_API void
OBJ_SetContext(struct lua_State*, struct Object*);
ORCA_API const char*
OBJ_GetName(struct Object const*);
ORCA_API void
OBJ_SetName(struct Object*, const char*);
ORCA_API const char*
OBJ_GetClassName(struct Object const*);
ORCA_API bool_t
OBJ_CheckName(struct Object const*, const char*);
ORCA_API struct Object*
OBJ_FindByPath(struct Object*, const char*);
ORCA_API uint32_t
OBJ_GetStyle(struct Object const*);
ORCA_API void
OBJ_SetStyle(struct Object*, uint32_t);
ORCA_API const char*
OBJ_FindCallbackForID(struct Object*, uint32_t);
ORCA_API void
OBJ_ApplyStyles(struct Object*, bool_t);
ORCA_API void
OBJ_SetDirty(struct Object*);
ORCA_API struct Object*
OBJ_GetParent(struct Object const*);
ORCA_API struct Object*
OBJ_GetFirstChild(struct Object const*);
ORCA_API struct Object*
OBJ_GetNext(struct Object const*);
ORCA_API struct Object*
OBJ_GetRoot(struct Object*);
ORCA_API struct Object*
OBJ_FindChildByID(struct Object*, uint32_t);
ORCA_API struct Object*
OBJ_FindChildByAlias(struct Object*, uint32_t);
ORCA_API const char*
OBJ_GetSourceFile(struct Object const*);
ORCA_API const char*
OBJ_GetTextContent(struct Object const*);
ORCA_API long
OBJ_GetTimestamp(struct Object const*);
ORCA_API uint32_t
OBJ_GetLuaObject(struct Object const*);
ORCA_API uint32_t
OBJ_GetAlias(struct Object const*);
ORCA_API uint32_t
OBJ_GetFlags(struct Object const*);
ORCA_API void
OBJ_SetFlags(struct Object*, uint32_t);
ORCA_API uint32_t
OBJ_GetIdentifier(struct Object const*);
ORCA_API struct Object*
OBJ_GetModal(struct Object const*);
ORCA_API void
OBJ_SetModal(struct Object*, struct Object*);
ORCA_API bool_t
OBJ_IsFocused(struct Object const*);
ORCA_API void
OBJ_SetHover(struct Object*);
ORCA_API void
OBJ_SetTextContent(struct Object*, const char*);
ORCA_API void
OBJ_SetSourceFile(struct Object*, const char*);
ORCA_API void
OBJ_SetClassName(struct Object*, const char*);
ORCA_API struct Object*
OBJ_FindParentOfClass(struct Object*, uint32_t);
ORCA_API struct Object*
OBJ_FindChildOfClass(struct Object*, uint32_t);
ORCA_API bool_t
OBJ_IsPrefabView(struct Object const*);
ORCA_API void
OBJ_AddAlias(struct Object*, const char*, const char*);
ORCA_API void
OBJ_AssignAliases(struct Object*, const char*);
ORCA_API void
OBJ_ParseClassAttribute(struct Object*, const char*);
ORCA_API void
OBJ_SetAnimation(struct Object*, const char*);
ORCA_API struct KeyframeAnim const*
OBJ_GetAnimation(struct Object const*);
ORCA_API void
OBJ_AddAnimation(struct Object*, struct KeyframeAnim*);
ORCA_API int32_t
OBJ_GetInteger(struct Object const*, uint32_t, int32_t);
ORCA_API struct Property*
OBJ_GetProperties(struct Object const*);
ORCA_API struct PropertyType*
OBJ_FindImplicitProperty(struct Object*, const char*);
ORCA_API struct PropertyType*
OBJ_FindExplicitProperty(struct Object*, const char*);
ORCA_API bool_t
OBJ_AttachPropertyProgram(struct Object*, const char*, const char*, enum PropertyAttribute, enum BindingMode, bool_t);
ORCA_API struct Property*
OBJ_FindPropertyByPath(struct Object*, const char*);
ORCA_API void
OBJ_Rebuild(struct lua_State*, struct Object*);
ORCA_API struct lua_State*
OBJ_GetDomain(struct Object*);

/** PropertyChangedEventArgs struct */
struct PropertyChangedEventArgs {
	struct Property* Property;
};
ORCA_API void luaX_pushPropertyChangedEventArgs(lua_State *L, struct PropertyChangedEventArgs const* PropertyChangedEventArgs);
ORCA_API struct PropertyChangedEventArgs* luaX_checkPropertyChangedEventArgs(lua_State *L, int idx);
/** UpdateMatrixEventArgs struct */
struct UpdateMatrixEventArgs {
	struct mat4 parent;
	float opacity;
	bool_t force;
};
ORCA_API void luaX_pushUpdateMatrixEventArgs(lua_State *L, struct UpdateMatrixEventArgs const* UpdateMatrixEventArgs);
ORCA_API struct UpdateMatrixEventArgs* luaX_checkUpdateMatrixEventArgs(lua_State *L, int idx);
/** PropertyEnumValue struct */
struct PropertyEnumValue {
	fixedString_t Name;
	int32_t Value;
};
ORCA_API void luaX_pushPropertyEnumValue(lua_State *L, struct PropertyEnumValue const* PropertyEnumValue);
ORCA_API struct PropertyEnumValue* luaX_checkPropertyEnumValue(lua_State *L, int idx);
/** PropertyType struct */
struct PropertyType {
	fixedString_t Name;
	fixedString_t Category;
	enum DataType DataType;
	fixedString_t DefaultValue;
	fixedString_t TypeString;
	bool_t AffectLayout;
	bool_t AffectRender;
	bool_t IsReadOnly;
	bool_t IsHidden;
	bool_t IsInherited;
	fixedString_t Key;
	fixedString_t Value;
	float Step;
	float UpperBound;
	float LowerBound;
	uint32_t ShortIdentifier;
	uint32_t FullIdentifier;
	uint32_t Offset;
	uint32_t DataSize;
	bool_t IsArray;
};
ORCA_API void luaX_pushPropertyType(lua_State *L, struct PropertyType const* PropertyType);
ORCA_API struct PropertyType* luaX_checkPropertyType(lua_State *L, int idx);


#endif