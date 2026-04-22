// Auto-generated from core.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>
#include <include/codegen.h>

#include "core.h"

// localization
extern void luaX_pushlocalization(lua_State *L, struct localization const* value);
extern struct localization* luaX_checklocalization(lua_State *L, int index);
// game
extern void luaX_pushgame(lua_State *L, struct game const* value);
extern struct game* luaX_checkgame(lua_State *L, int index);
// Property
extern void luaX_pushProperty(lua_State *L, struct Property const* value);
extern struct Property* luaX_checkProperty(lua_State *L, int index);
// lua_State
extern void luaX_pushlua_State(lua_State *L, struct lua_State const* value);
extern struct lua_State* luaX_checklua_State(lua_State *L, int index);
// style_class_selector
extern void luaX_pushstyle_class_selector(lua_State *L, struct style_class_selector const* value);
extern struct style_class_selector* luaX_checkstyle_class_selector(lua_State *L, int index);

ENUM(MessageRouting, "Bubbling", "TunnelingBubbling", "Tunneling", "Direct")
ENUM(PropertyState, "Normal", "Hover", "Focus", "Select", "Disable", "OldValue")
ENUM(BindingMode, "OneWay", "TwoWay", "OneWayToSource", "Expression")
ENUM(PropertyAttribute, "WholeProperty", "ColorR", "ColorG", "ColorB", "ColorA", "VectorX", "VectorY", "VectorZ", "VectorW")
ENUM(AnimationMode, "PlayOnce", "Loop", "PingPong")
ENUM(PlaybackMode, "Normal", "Reverse", "PingPong")
ENUM(InterpolationMode, "Linear", "Const", "Back", "Bounce", "Circ", "Cubic", "Elastic", "Expo", "Quad", "Quart", "Quint", "Sine")
ENUM(LocaleEntryType, "Undefined", "Text", "Resource")
ENUM(Easing, "InOut", "In", "Out")
ENUM(HorizontalAlignment, "Stretch", "Left", "Center", "Right")
ENUM(VerticalAlignment, "Stretch", "Top", "Center", "Bottom")
ENUM(DepthAlignment, "Near", "Center", "Far", "Stretch")
ENUM(Sizing, "PlusMargin", "SizeOnly", "MinusPadding")
ENUM(BorderStyle, "None", "Hidden", "Dotted", "Dashed", "Solid", "Double", "Groove", "Ridge", "Inset", "Outset")
ENUM(MouseButton, "None", "Left", "Right", "Middle")
ENUM(ResourceEntryType, "Undefined", "Text", "Resource")

int f_OBJ_CreateFromLuaState(lua_State *L) {
	return OBJ_CreateFromLuaState(L);
}
int f_OBJ_Animate(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_Animate(L, this_ );
	return 0;
}
int f_OBJ_Clear(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_Clear(L, this_ );
	return 0;
}
int f_OBJ_Release(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_Release(L, this_ );
	return 0;
}
int f_OBJ_Equals(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	struct Object const* other = luaX_checkObject(L, 2);
	bool_t result_ = OBJ_Equals(this_, other);
	lua_pushboolean(L, result_);
	return 1;
}
int f_OBJ_Rebuild(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_Rebuild(L, this_);
	return 0;
}
int f_OBJ_AddChild(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	struct Object* child = luaX_checkObject(L, 2);
	bool_t is_template = lua_toboolean(L, 3);
	struct Object* result_ = OBJ_AddChild(this_, child, is_template);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_RemoveFromParent(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_RemoveFromParent(L, this_ );
	return 0;
}
int f_OBJ_GetParent(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	struct Object* result_ = OBJ_GetParent(this_);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_GetFirstChild(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	struct Object* result_ = OBJ_GetFirstChild(this_);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_GetNext(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	struct Object* result_ = OBJ_GetNext(this_);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_GetRoot(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	struct Object* result_ = OBJ_GetRoot(this_);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_FindChild(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* name = luaL_checkstring(L, 2);
	bool_t recursive = lua_toboolean(L, 3);
	struct Object* result_ = OBJ_FindChild(this_, name, recursive);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_FindByPath(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* name = luaL_checkstring(L, 2);
	struct Object* result_ = OBJ_FindByPath(this_, name);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_FindChildByID(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	uint32_t id = (uint32_t)luaL_checkinteger(L, 2);
	struct Object* result_ = OBJ_FindChildByID(this_, id);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_FindChildByAlias(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	uint32_t id = (uint32_t)luaL_checkinteger(L, 2);
	struct Object* result_ = OBJ_FindChildByAlias(this_, id);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_FindChildOfClass(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	uint32_t classID = (uint32_t)luaL_checkinteger(L, 2);
	struct Object* result_ = OBJ_FindChildOfClass(this_, classID);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_FindParentOfClass(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	uint32_t classID = (uint32_t)luaL_checkinteger(L, 2);
	struct Object* result_ = OBJ_FindParentOfClass(this_, classID);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_post(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* message = luaL_checkstring(L, 2);
	OBJ_post(L, this_, message );
	return 0;
}
int f_OBJ_send(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* message = luaL_checkstring(L, 2);
	return OBJ_send(L, this_, message);
}
int f_OBJ_SetProperty(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* Property = luaL_checkstring(L, 2);
	bool_t result_ = OBJ_SetProperty(L, this_, Property);
	lua_pushboolean(L, result_);
	return 1;
}
int f_OBJ_GetProperty(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* Property = luaL_checkstring(L, 2);
	return OBJ_GetProperty(L, this_, Property);
}
int f_OBJ_UpdateProperties(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_UpdateProperties(this_ );
	return 0;
}
int f_OBJ_EmitPropertyChangedEvents(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_EmitPropertyChangedEvents(L, this_ );
	return 0;
}
int f_OBJ_FindImplicitProperty(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* name = luaL_checkstring(L, 2);
	struct PropertyType const* result_ = OBJ_FindImplicitProperty(this_, name);
	luaX_pushPropertyType(L, &*result_);
	return 1;
}
int f_OBJ_FindExplicitProperty(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* name = luaL_checkstring(L, 2);
	struct PropertyType const* result_ = OBJ_FindExplicitProperty(this_, name);
	luaX_pushPropertyType(L, &*result_);
	return 1;
}
int f_OBJ_AttachPropertyProgram(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* name = luaL_checkstring(L, 2);
	const char* program = luaL_checkstring(L, 3);
	enum PropertyAttribute attribute = luaX_checkPropertyAttribute(L, 4);
	enum BindingMode mode = luaX_checkBindingMode(L, 5);
	bool_t enabled = lua_toboolean(L, 6);
	bool_t result_ = OBJ_AttachPropertyProgram(this_, name, program, attribute, mode, enabled);
	lua_pushboolean(L, result_);
	return 1;
}
int f_OBJ_FindPropertyByPath(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* path = luaL_checkstring(L, 2);
	struct Property* result_ = OBJ_FindPropertyByPath(this_, path);
	luaX_pushProperty(L, result_);
	return 1;
}
int f_OBJ_GetProperties(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	struct Property* result_ = OBJ_GetProperties(this_);
	luaX_pushProperty(L, result_);
	return 1;
}
int f_OBJ_GetInteger(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	uint32_t ident = (uint32_t)luaL_checkinteger(L, 2);
	int32_t fallback = (int32_t)luaL_checkinteger(L, 3);
	int32_t result_ = OBJ_GetInteger(this_, ident, fallback);
	lua_pushinteger(L, result_);
	return 1;
}
int f_OBJ_SetDirty(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_SetDirty(this_ );
	return 0;
}
int f_OBJ_ClearDirtyFlags(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_ClearDirtyFlags(this_ );
	return 0;
}
int f_OBJ_GetStyle(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	uint32_t result_ = OBJ_GetStyle(this_);
	lua_pushinteger(L, result_);
	return 1;
}
int f_OBJ_SetStyle(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	uint32_t style = (uint32_t)luaL_checkinteger(L, 2);
	OBJ_SetStyle(this_, style );
	return 0;
}
int f_OBJ_DoTween(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* property = luaL_checkstring(L, 2);
	int32_t duration = (int32_t)luaL_checkinteger(L, 3);
	enum InterpolationMode ipo = luaX_checkInterpolationMode(L, 4);
	enum Easing easing = luaX_checkEasing(L, 5);
	OBJ_DoTween(L, this_, property, duration, ipo, easing );
	return 0;
}
int f_OBJ_AddComponentByName(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* className = luaL_checkstring(L, 2);
	OBJ_AddComponentByName(L, this_, className );
	return 0;
}
int f_OBJ_SetFocus(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_SetFocus(this_ );
	return 0;
}
int f_OBJ_IsFocused(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	bool_t result_ = OBJ_IsFocused(this_);
	lua_pushboolean(L, result_);
	return 1;
}
int f_OBJ_SetHover(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_SetHover(this_ );
	return 0;
}
int f_OBJ_ShowModal(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	struct Object* modal = luaX_checkObject(L, 2);
	return OBJ_ShowModal(L, this_, modal);
}
int f_OBJ_SetTimer(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	int32_t duration = (int32_t)luaL_checkinteger(L, 2);
	int32_t result_ = OBJ_SetTimer(this_, duration);
	lua_pushinteger(L, result_);
	return 1;
}
int f_OBJ_GetName(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	const char* result_ = OBJ_GetName(this_);
	lua_pushstring(L, result_);
	return 1;
}
int f_OBJ_SetName(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* name = luaL_checkstring(L, 2);
	OBJ_SetName(this_, name );
	return 0;
}
int f_OBJ_GetClassName(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	const char* result_ = OBJ_GetClassName(this_);
	lua_pushstring(L, result_);
	return 1;
}
int f_OBJ_SetClassName(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* classID = luaL_checkstring(L, 2);
	OBJ_SetClassName(this_, classID );
	return 0;
}
int f_OBJ_CheckName(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	const char* name = luaL_checkstring(L, 2);
	bool_t result_ = OBJ_CheckName(this_, name);
	lua_pushboolean(L, result_);
	return 1;
}
int f_OBJ_GetFlags(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	uint32_t result_ = OBJ_GetFlags(this_);
	lua_pushinteger(L, result_);
	return 1;
}
int f_OBJ_SetFlags(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	uint32_t flags = (uint32_t)luaL_checkinteger(L, 2);
	OBJ_SetFlags(this_, flags );
	return 0;
}
int f_OBJ_GetIdentifier(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	uint32_t result_ = OBJ_GetIdentifier(this_);
	lua_pushinteger(L, result_);
	return 1;
}
int f_OBJ_GetAlias(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	uint32_t result_ = OBJ_GetAlias(this_);
	lua_pushinteger(L, result_);
	return 1;
}
int f_OBJ_GetSourceFile(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	const char* result_ = OBJ_GetSourceFile(this_);
	lua_pushstring(L, result_);
	return 1;
}
int f_OBJ_SetSourceFile(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* path = luaL_checkstring(L, 2);
	OBJ_SetSourceFile(this_, path );
	return 0;
}
int f_OBJ_GetTextContent(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	const char* result_ = OBJ_GetTextContent(this_);
	lua_pushstring(L, result_);
	return 1;
}
int f_OBJ_SetTextContent(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* text = luaL_checkstring(L, 2);
	OBJ_SetTextContent(this_, text );
	return 0;
}
int f_OBJ_GetTimestamp(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	long result_ = OBJ_GetTimestamp(this_);
	lua_pushinteger(L, result_);
	return 1;
}
int f_OBJ_GetDomain(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	struct lua_State* result_ = OBJ_GetDomain(this_);
	luaX_pushlua_State(L, result_);
	return 1;
}
int f_OBJ_SetContext(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_SetContext(L, this_ );
	return 0;
}
int f_OBJ_Instantiate(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	struct Object* result_ = OBJ_Instantiate(L, this_);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_LoadPrefabs(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_LoadPrefabs(this_);
	return 0;
}
int f_OBJ_IsPrefabView(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	bool_t result_ = OBJ_IsPrefabView(this_);
	lua_pushboolean(L, result_);
	return 1;
}

int f_object_index(lua_State* L);

int luaopen_orca_Object(lua_State *L) {
	luaL_newmetatable(L, "Object");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_OBJ_CreateFromLuaState },
		{ "animate", f_OBJ_Animate },
		{ "clear", f_OBJ_Clear },
		{ "__eq", f_OBJ_Equals },
		{ "rebuild", f_OBJ_Rebuild },
		{ "addChild", f_OBJ_AddChild },
		{ "__add", f_OBJ_AddChild },
		{ "release", f_OBJ_Release },
		{ "removeFromParent", f_OBJ_RemoveFromParent },
		{ "getParent", f_OBJ_GetParent },
		{ "getFirstChild", f_OBJ_GetFirstChild },
		{ "getNext", f_OBJ_GetNext },
		{ "getRoot", f_OBJ_GetRoot },
		{ "findChild", f_OBJ_FindChild },
		{ "findByPath", f_OBJ_FindByPath },
		{ "findChildByID", f_OBJ_FindChildByID },
		{ "findChildByAlias", f_OBJ_FindChildByAlias },
		{ "findChildOfClass", f_OBJ_FindChildOfClass },
		{ "findParentOfClass", f_OBJ_FindParentOfClass },
		{ "post", f_OBJ_post },
		{ "send", f_OBJ_send },
		{ "__newindex", f_OBJ_SetProperty },
		{ "__index", f_OBJ_GetProperty },
		{ "updateProperties", f_OBJ_UpdateProperties },
		{ "emitPropertyChangedEvents", f_OBJ_EmitPropertyChangedEvents },
		{ "findImplicitProperty", f_OBJ_FindImplicitProperty },
		{ "findExplicitProperty", f_OBJ_FindExplicitProperty },
		{ "attachPropertyProgram", f_OBJ_AttachPropertyProgram },
		{ "findPropertyByPath", f_OBJ_FindPropertyByPath },
		{ "getProperties", f_OBJ_GetProperties },
		{ "getInteger", f_OBJ_GetInteger },
		{ "setDirty", f_OBJ_SetDirty },
		{ "clearDirtyFlags", f_OBJ_ClearDirtyFlags },
		{ "getStyle", f_OBJ_GetStyle },
		{ "setStyle", f_OBJ_SetStyle },
		{ "doTween", f_OBJ_DoTween },
		{ "addComponentByName", f_OBJ_AddComponentByName },
		{ "setFocus", f_OBJ_SetFocus },
		{ "isFocused", f_OBJ_IsFocused },
		{ "setHover", f_OBJ_SetHover },
		{ "showModal", f_OBJ_ShowModal },
		{ "setTimer", f_OBJ_SetTimer },
		{ "getName", f_OBJ_GetName },
		{ "setName", f_OBJ_SetName },
		{ "getClassName", f_OBJ_GetClassName },
		{ "setClassName", f_OBJ_SetClassName },
		{ "checkName", f_OBJ_CheckName },
		{ "getFlags", f_OBJ_GetFlags },
		{ "setFlags", f_OBJ_SetFlags },
		{ "getIdentifier", f_OBJ_GetIdentifier },
		{ "getAlias", f_OBJ_GetAlias },
		{ "getSourceFile", f_OBJ_GetSourceFile },
		{ "setSourceFile", f_OBJ_SetSourceFile },
		{ "getTextContent", f_OBJ_GetTextContent },
		{ "setTextContent", f_OBJ_SetTextContent },
		{ "getTimestamp", f_OBJ_GetTimestamp },
		{ "getDomain", f_OBJ_GetDomain },
		{ "instantiate", f_OBJ_Instantiate },
		{ "loadPrefabs", f_OBJ_LoadPrefabs },
		{ "isPrefabView", f_OBJ_IsPrefabView },
		{ NULL, NULL },
	}), 0);
	int f_OBJ_newindex(lua_State* L);
	lua_pushcfunction(L, f_OBJ_newindex);
	lua_setfield(L, -2, "__newindex");
	lua_pushcfunction(L, f_object_index);
	lua_setfield(L, -2, "__index");
	return 1;
}
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(lua_State *L, const char* str, struct PropertyType const* prop, void* struct_ptr);
static struct PropertyType _AnimationClipReference[] = {
	DECL(0x0fe07306, AnimationClipReference, Name, Name, kDataTypeString), // AnimationClipReference.Name
	DECL(0xd33ddb1b, AnimationClipReference, Clip, Clip, kDataTypeObject, .TypeString = "AnimationClip"), // AnimationClipReference.Clip
};
static luaL_Reg _AnimationClipReference_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _Keyframe[] = {
	DECL(0xdfe4e404, Keyframe, Time, time, kDataTypeFloat), // Keyframe.Time
	DECL(0xd147f96a, Keyframe, Value, value, kDataTypeStruct, .TypeString = "Vector4D"), // Keyframe.Value
	DECL(0xbe5410b1, Keyframe, InSlope, inSlope, kDataTypeStruct, .TypeString = "Vector4D"), // Keyframe.InSlope
	DECL(0xc3a6b78e, Keyframe, OutSlope, outSlope, kDataTypeStruct, .TypeString = "Vector4D"), // Keyframe.OutSlope
	DECL(0xfd369606, Keyframe, InWeight, inWeight, kDataTypeStruct, .TypeString = "Vector4D"), // Keyframe.InWeight
	DECL(0xdac7b09b, Keyframe, OutWeight, outWeight, kDataTypeStruct, .TypeString = "Vector4D"), // Keyframe.OutWeight
	DECL(0x153369e9, Keyframe, TangentMode, tangentMode, kDataTypeInt), // Keyframe.TangentMode
	DECL(0xf0555d63, Keyframe, WeightedMode, weightedMode, kDataTypeInt), // Keyframe.WeightedMode
};
static luaL_Reg _Keyframe_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _LocaleEntry[] = {
	DECL(0xcd1ac90c, LocaleEntry, Key, Key, kDataTypeString), // LocaleEntry.Key
	DECL(0xd147f96a, LocaleEntry, Value, Value, kDataTypeString), // LocaleEntry.Value
	DECL(0xd155d06d, LocaleEntry, Type, Type, kDataTypeEnum, .EnumValues = _LocaleEntryType), // LocaleEntry.Type
};
static luaL_Reg _LocaleEntry_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _ResourceEntry[] = {
	DECL(0xcd1ac90c, ResourceEntry, Key, Key, kDataTypeString), // ResourceEntry.Key
	DECL(0xd147f96a, ResourceEntry, Value, Value, kDataTypeString), // ResourceEntry.Value
	DECL(0xd155d06d, ResourceEntry, Type, Type, kDataTypeEnum, .EnumValues = _ResourceEntryType), // ResourceEntry.Type
};
static luaL_Reg _ResourceEntry_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _CornerRadius[] = {
	DECL(0xd5ac3a0b, CornerRadius, TopLeftRadius, TopLeftRadius, kDataTypeFloat), // CornerRadius.TopLeftRadius
	DECL(0xdbe5a724, CornerRadius, TopRightRadius, TopRightRadius, kDataTypeFloat), // CornerRadius.TopRightRadius
	DECL(0xf6ae40ce, CornerRadius, BottomRightRadius, BottomRightRadius, kDataTypeFloat), // CornerRadius.BottomRightRadius
	DECL(0x7f5fe235, CornerRadius, BottomLeftRadius, BottomLeftRadius, kDataTypeFloat), // CornerRadius.BottomLeftRadius
};
static luaL_Reg _CornerRadius_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _EdgeShorthand[] = {
	DECL(0x92773890, EdgeShorthand, Left, Left, kDataTypeFloat), // EdgeShorthand.Left
	DECL(0x1e9e9f85, EdgeShorthand, Right, Right, kDataTypeFloat), // EdgeShorthand.Right
};
static luaL_Reg _EdgeShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _AlignmentShorthand[] = {
	DECL(0x6bf39489, AlignmentShorthand, Horizontal, Axis[0], kDataTypeInt), // AlignmentShorthand.Horizontal
	DECL(0x80fb2d5b, AlignmentShorthand, Vertical, Axis[1], kDataTypeInt), // AlignmentShorthand.Vertical
	DECL(0xd070218a, AlignmentShorthand, Depth, Axis[2], kDataTypeInt), // AlignmentShorthand.Depth
};
static luaL_Reg _AlignmentShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _Thickness[] = {
	DECL(0x6bf39489, Thickness, Horizontal, Axis[0], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Thickness.Horizontal
	DECL(0x92773890, Thickness, Left, Axis[0].Left, kDataTypeFloat), // Thickness.Left
	DECL(0x1e9e9f85, Thickness, Right, Axis[0].Right, kDataTypeFloat), // Thickness.Right
	DECL(0x80fb2d5b, Thickness, Vertical, Axis[1], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Thickness.Vertical
	DECL(0x099b73dc, Thickness, Top, Axis[1].Left, kDataTypeFloat), // Thickness.Top
	DECL(0x22b5f34a, Thickness, Bottom, Axis[1].Right, kDataTypeFloat), // Thickness.Bottom
	DECL(0xd070218a, Thickness, Depth, Axis[2], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Thickness.Depth
	DECL(0x6de89878, Thickness, Front, Axis[2].Left, kDataTypeFloat), // Thickness.Front
	DECL(0xc2954bc2, Thickness, Back, Axis[2].Right, kDataTypeFloat), // Thickness.Back
};
static luaL_Reg _Thickness_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _BorderShorthand[] = {
	DECL(0x3b42dfbf, BorderShorthand, Width, Width, kDataTypeStruct, .TypeString = "Thickness"), // BorderShorthand.Width
	DECL(0x9bd0d683, BorderShorthand, HorizontalWidth, Width.Axis[0], kDataTypeStruct, .TypeString = "EdgeShorthand"), // BorderShorthand.HorizontalWidth
	DECL(0x0671d536, BorderShorthand, WidthLeft, Width.Axis[0].Left, kDataTypeFloat), // BorderShorthand.WidthLeft
	DECL(0x30532a6b, BorderShorthand, WidthRight, Width.Axis[0].Right, kDataTypeFloat), // BorderShorthand.WidthRight
	DECL(0xa1c213dd, BorderShorthand, VerticalWidth, Width.Axis[1], kDataTypeStruct, .TypeString = "EdgeShorthand"), // BorderShorthand.VerticalWidth
	DECL(0xf37d53fe, BorderShorthand, WidthTop, Width.Axis[1].Left, kDataTypeFloat), // BorderShorthand.WidthTop
	DECL(0xddffd3a0, BorderShorthand, WidthBottom, Width.Axis[1].Right, kDataTypeFloat), // BorderShorthand.WidthBottom
	DECL(0x67356c56, BorderShorthand, DepthWidth, Width.Axis[2], kDataTypeStruct, .TypeString = "EdgeShorthand"), // BorderShorthand.DepthWidth
	DECL(0x03261636, BorderShorthand, WidthFront, Width.Axis[2].Left, kDataTypeFloat), // BorderShorthand.WidthFront
	DECL(0x16fd78d8, BorderShorthand, WidthBack, Width.Axis[2].Right, kDataTypeFloat), // BorderShorthand.WidthBack
	DECL(0xe5b43cf8, BorderShorthand, Color, Color, kDataTypeColor), // BorderShorthand.Color
	DECL(0x5467ec76, BorderShorthand, Style, Style, kDataTypeEnum, .EnumValues = _BorderStyle), // BorderShorthand.Style
	DECL(0x3a8111d3, BorderShorthand, Radius, Radius, kDataTypeStruct, .TypeString = "CornerRadius"), // BorderShorthand.Radius
	DECL(0x1c0b6355, BorderShorthand, RadiusTopLeftRadius, Radius.TopLeftRadius, kDataTypeFloat), // BorderShorthand.RadiusTopLeftRadius
	DECL(0xe0a577ee, BorderShorthand, RadiusTopRightRadius, Radius.TopRightRadius, kDataTypeFloat), // BorderShorthand.RadiusTopRightRadius
	DECL(0x494aff04, BorderShorthand, RadiusBottomRightRadius, Radius.BottomRightRadius, kDataTypeFloat), // BorderShorthand.RadiusBottomRightRadius
	DECL(0xfe16d36b, BorderShorthand, RadiusBottomLeftRadius, Radius.BottomLeftRadius, kDataTypeFloat), // BorderShorthand.RadiusBottomLeftRadius
};
static luaL_Reg _BorderShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _SizeAxisShorthand[] = {
	DECL(0x77ea8663, SizeAxisShorthand, Requested, Requested, kDataTypeFloat), // SizeAxisShorthand.Requested
	DECL(0x28adf5d5, SizeAxisShorthand, Desired, Desired, kDataTypeFloat), // SizeAxisShorthand.Desired
	DECL(0x2e9445f7, SizeAxisShorthand, Min, Min, kDataTypeFloat), // SizeAxisShorthand.Min
	DECL(0xf1aed197, SizeAxisShorthand, Actual, Actual, kDataTypeFloat), // SizeAxisShorthand.Actual
	DECL(0x25dca54c, SizeAxisShorthand, Scroll, Scroll, kDataTypeFloat), // SizeAxisShorthand.Scroll
};
static luaL_Reg _SizeAxisShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _SizeShorthand[] = {
	DECL(0x6bf39489, SizeShorthand, Horizontal, Axis[0], kDataTypeStruct, .TypeString = "SizeAxisShorthand"), // SizeShorthand.Horizontal
	DECL(0x9463344f, SizeShorthand, HorizontalRequested, Axis[0].Requested, kDataTypeFloat), // SizeShorthand.HorizontalRequested
	DECL(0x1c7b4471, SizeShorthand, HorizontalDesired, Axis[0].Desired, kDataTypeFloat), // SizeShorthand.HorizontalDesired
	DECL(0xda5582c3, SizeShorthand, HorizontalMin, Axis[0].Min, kDataTypeFloat), // SizeShorthand.HorizontalMin
	DECL(0x42284603, SizeShorthand, HorizontalActual, Axis[0].Actual, kDataTypeFloat), // SizeShorthand.HorizontalActual
	DECL(0x8c65db48, SizeShorthand, HorizontalScroll, Axis[0].Scroll, kDataTypeFloat), // SizeShorthand.HorizontalScroll
	DECL(0x80fb2d5b, SizeShorthand, Vertical, Axis[1], kDataTypeStruct, .TypeString = "SizeAxisShorthand"), // SizeShorthand.Vertical
	DECL(0x4d64cad5, SizeShorthand, VerticalRequested, Axis[1].Requested, kDataTypeFloat), // SizeShorthand.VerticalRequested
	DECL(0x9b2653af, SizeShorthand, VerticalDesired, Axis[1].Desired, kDataTypeFloat), // SizeShorthand.VerticalDesired
	DECL(0xc446df6d, SizeShorthand, VerticalMin, Axis[1].Min, kDataTypeFloat), // SizeShorthand.VerticalMin
	DECL(0x2a77ca09, SizeShorthand, VerticalActual, Axis[1].Actual, kDataTypeFloat), // SizeShorthand.VerticalActual
	DECL(0xe04d52a2, SizeShorthand, VerticalScroll, Axis[1].Scroll, kDataTypeFloat), // SizeShorthand.VerticalScroll
	DECL(0xd070218a, SizeShorthand, Depth, Axis[2], kDataTypeStruct, .TypeString = "SizeAxisShorthand"), // SizeShorthand.Depth
	DECL(0xe455f006, SizeShorthand, DepthRequested, Axis[2].Requested, kDataTypeFloat), // SizeShorthand.DepthRequested
	DECL(0x5c288608, SizeShorthand, DepthDesired, Axis[2].Desired, kDataTypeFloat), // SizeShorthand.DepthDesired
	DECL(0x3303ed1e, SizeShorthand, DepthMin, Axis[2].Min, kDataTypeFloat), // SizeShorthand.DepthMin
	DECL(0xf04276dc, SizeShorthand, DepthActual, Axis[2].Actual, kDataTypeFloat), // SizeShorthand.DepthActual
	DECL(0xd8e0c913, SizeShorthand, DepthScroll, Axis[2].Scroll, kDataTypeFloat), // SizeShorthand.DepthScroll
};
static luaL_Reg _SizeShorthand_Methods[] = {
	{ NULL, NULL }
};

STRUCT(AnimationClipReference, AnimationClipReference);
STRUCT(Keyframe, Keyframe);
STRUCT(LocaleEntry, LocaleEntry);
STRUCT(ResourceEntry, ResourceEntry);
STRUCT(CornerRadius, CornerRadius);
STRUCT(EdgeShorthand, EdgeShorthand);
STRUCT(AlignmentShorthand, AlignmentShorthand);
STRUCT(Thickness, Thickness);
STRUCT(BorderShorthand, BorderShorthand);
STRUCT(SizeAxisShorthand, SizeAxisShorthand);
STRUCT(SizeShorthand, SizeShorthand);
#define Node_LeftButtonDownEventArgs Node_MouseMessageEventArgs
#define Node_RightButtonDownEventArgs Node_MouseMessageEventArgs
#define Node_OtherButtonDownEventArgs Node_MouseMessageEventArgs
#define Node_LeftButtonUpEventArgs Node_MouseMessageEventArgs
#define Node_RightButtonUpEventArgs Node_MouseMessageEventArgs
#define Node_OtherButtonUpEventArgs Node_MouseMessageEventArgs
#define Node_LeftButtonDraggedEventArgs Node_MouseMessageEventArgs
#define Node_RightButtonDraggedEventArgs Node_MouseMessageEventArgs
#define Node_OtherButtonDraggedEventArgs Node_MouseMessageEventArgs
#define Node_LeftDoubleClickEventArgs Node_MouseMessageEventArgs
#define Node_RightDoubleClickEventArgs Node_MouseMessageEventArgs
#define Node_OtherDoubleClickEventArgs Node_MouseMessageEventArgs
#define Node_MouseMovedEventArgs Node_MouseMessageEventArgs
#define Node_ScrollWheelEventArgs Node_MouseMessageEventArgs
#define Node_DragDropEventArgs Node_MouseMessageEventArgs
#define Node_DragEnterEventArgs Node_MouseMessageEventArgs
#define Node_KeyDownEventArgs Node_KeyMessageEventArgs
#define Node_KeyUpEventArgs Node_KeyMessageEventArgs
#define Node_TextInputEventArgs Node_KeyMessageEventArgs

static luaL_Reg _Object_CreateEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_CreateEventArgs[] = {
};
static luaL_Reg _Object_StartEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_StartEventArgs[] = {
};
static luaL_Reg _Object_AnimateEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_AnimateEventArgs[] = {
};
static luaL_Reg _StyleController_ThemeChangedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _StyleController_ThemeChangedEventArgs[] = {
	DECL(0x5bee3c77, StyleController_ThemeChangedEventArgs, recursive, recursive, kDataTypeBool), // StyleController_ThemeChangedEventArgs.recursive
};
static luaL_Reg _Object_PropertyChangedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_PropertyChangedEventArgs[] = {
	DECL(0x5221f9e8, Object_PropertyChangedEventArgs, Property, Property, kDataTypeStruct, .TypeString = "Property"), // Object_PropertyChangedEventArgs.Property
};
static luaL_Reg _Object_AttachedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_AttachedEventArgs[] = {
};
static luaL_Reg _Object_ReleaseEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_ReleaseEventArgs[] = {
};
static luaL_Reg _Object_DestroyEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_DestroyEventArgs[] = {
};
static luaL_Reg _Object_TimerEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_TimerEventArgs[] = {
	DECL(0x4903ce0f, Object_TimerEventArgs, timerId, timerId, kDataTypeInt), // Object_TimerEventArgs.timerId
};
static luaL_Reg _AnimationPlayer_PlayEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _AnimationPlayer_PlayEventArgs[] = {
	DECL(0x0fe07306, AnimationPlayer_PlayEventArgs, Name, Name, kDataTypeString), // AnimationPlayer_PlayEventArgs.Name
};
static luaL_Reg _AnimationPlayer_ResumeEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _AnimationPlayer_ResumeEventArgs[] = {
};
static luaL_Reg _AnimationPlayer_StopEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _AnimationPlayer_StopEventArgs[] = {
};
static luaL_Reg _AnimationPlayer_PauseEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _AnimationPlayer_PauseEventArgs[] = {
};
static luaL_Reg _AnimationPlayer_StartedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _AnimationPlayer_StartedEventArgs[] = {
};
static luaL_Reg _AnimationPlayer_StoppedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _AnimationPlayer_StoppedEventArgs[] = {
};
static luaL_Reg _AnimationPlayer_CompletedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _AnimationPlayer_CompletedEventArgs[] = {
};
static luaL_Reg _StyleController_AddClassEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _StyleController_AddClassEventArgs[] = {
	DECL(0x2ecda0bc, StyleController_AddClassEventArgs, ClassName, ClassName, kDataTypeString), // StyleController_AddClassEventArgs.ClassName
};
static luaL_Reg _StyleController_AddClassesEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _StyleController_AddClassesEventArgs[] = {
	DECL(0x7cb425dd, StyleController_AddClassesEventArgs, ClassNames, ClassNames, kDataTypeString), // StyleController_AddClassesEventArgs.ClassNames
};
static luaL_Reg _StateManagerController_ControllerChangedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _StateManagerController_ControllerChangedEventArgs[] = {
	DECL(0x5221f9e8, StateManagerController_ControllerChangedEventArgs, Property, Property, kDataTypeStruct, .TypeString = "Property"), // StateManagerController_ControllerChangedEventArgs.Property
};
static luaL_Reg _Trigger_TriggeredEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Trigger_TriggeredEventArgs[] = {
	DECL(0xa5ea0da3, Trigger_TriggeredEventArgs, Trigger, Trigger, kDataTypeObject, .TypeString = "Trigger"), // Trigger_TriggeredEventArgs.Trigger
};
static luaL_Reg _Node_UpdateMatrixEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_UpdateMatrixEventArgs[] = {
	DECL(0xeacdfcfd, Node_UpdateMatrixEventArgs, parent, parent, kDataTypeStruct, .TypeString = "Matrix3D"), // Node_UpdateMatrixEventArgs.parent
	DECL(0xc6c2dd66, Node_UpdateMatrixEventArgs, opacity, opacity, kDataTypeFloat), // Node_UpdateMatrixEventArgs.opacity
	DECL(0x79a98884, Node_UpdateMatrixEventArgs, force, force, kDataTypeBool), // Node_UpdateMatrixEventArgs.force
};
static luaL_Reg _Node_LoadViewEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LoadViewEventArgs[] = {
	DECL(0x187f5b0f, Node_LoadViewEventArgs, lua_state, lua_state, kDataTypeStruct, .TypeString = "lua_State"), // Node_LoadViewEventArgs.lua_state
};
static luaL_Reg _Node_HitTestEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_HitTestEventArgs[] = {
	DECL(0xfd0c5087, Node_HitTestEventArgs, x, x, kDataTypeInt), // Node_HitTestEventArgs.x
	DECL(0xfc0c4ef4, Node_HitTestEventArgs, y, y, kDataTypeInt), // Node_HitTestEventArgs.y
};
static luaL_Reg _Node_IsVisibleEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_IsVisibleEventArgs[] = {
};
static luaL_Reg _Node_ViewDidLoadEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_ViewDidLoadEventArgs[] = {
};
static luaL_Reg _Node_KillFocusEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KillFocusEventArgs[] = {
};
static luaL_Reg _Node_SetFocusEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_SetFocusEventArgs[] = {
};
static luaL_Reg _Node_GetSizeEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_GetSizeEventArgs[] = {
};
static luaL_Reg _Node_MouseMessageEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_MouseMessageEventArgs[] = {
	DECL(0xfd0c5087, Node_MouseMessageEventArgs, x, x, kDataTypeFloat), // Node_MouseMessageEventArgs.x
	DECL(0xfc0c4ef4, Node_MouseMessageEventArgs, y, y, kDataTypeFloat), // Node_MouseMessageEventArgs.y
	DECL(0xec56f27b, Node_MouseMessageEventArgs, deltaX, deltaX, kDataTypeInt), // Node_MouseMessageEventArgs.deltaX
	DECL(0xeb56f0e8, Node_MouseMessageEventArgs, deltaY, deltaY, kDataTypeInt), // Node_MouseMessageEventArgs.deltaY
	DECL(0x43b27471, Node_MouseMessageEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_MouseMessageEventArgs.button
	DECL(0xd9747336, Node_MouseMessageEventArgs, clickCount, clickCount, kDataTypeInt), // Node_MouseMessageEventArgs.clickCount
};
static luaL_Reg _Node_LeftButtonDownEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftButtonDownEventArgs[] = {
	DECL(0xfd0c5087, Node_LeftButtonDownEventArgs, x, x, kDataTypeFloat), // Node_LeftButtonDownEventArgs.x
	DECL(0xfc0c4ef4, Node_LeftButtonDownEventArgs, y, y, kDataTypeFloat), // Node_LeftButtonDownEventArgs.y
	DECL(0xec56f27b, Node_LeftButtonDownEventArgs, deltaX, deltaX, kDataTypeInt), // Node_LeftButtonDownEventArgs.deltaX
	DECL(0xeb56f0e8, Node_LeftButtonDownEventArgs, deltaY, deltaY, kDataTypeInt), // Node_LeftButtonDownEventArgs.deltaY
	DECL(0x43b27471, Node_LeftButtonDownEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_LeftButtonDownEventArgs.button
	DECL(0xd9747336, Node_LeftButtonDownEventArgs, clickCount, clickCount, kDataTypeInt), // Node_LeftButtonDownEventArgs.clickCount
};
static luaL_Reg _Node_RightButtonDownEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightButtonDownEventArgs[] = {
	DECL(0xfd0c5087, Node_RightButtonDownEventArgs, x, x, kDataTypeFloat), // Node_RightButtonDownEventArgs.x
	DECL(0xfc0c4ef4, Node_RightButtonDownEventArgs, y, y, kDataTypeFloat), // Node_RightButtonDownEventArgs.y
	DECL(0xec56f27b, Node_RightButtonDownEventArgs, deltaX, deltaX, kDataTypeInt), // Node_RightButtonDownEventArgs.deltaX
	DECL(0xeb56f0e8, Node_RightButtonDownEventArgs, deltaY, deltaY, kDataTypeInt), // Node_RightButtonDownEventArgs.deltaY
	DECL(0x43b27471, Node_RightButtonDownEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_RightButtonDownEventArgs.button
	DECL(0xd9747336, Node_RightButtonDownEventArgs, clickCount, clickCount, kDataTypeInt), // Node_RightButtonDownEventArgs.clickCount
};
static luaL_Reg _Node_OtherButtonDownEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherButtonDownEventArgs[] = {
	DECL(0xfd0c5087, Node_OtherButtonDownEventArgs, x, x, kDataTypeFloat), // Node_OtherButtonDownEventArgs.x
	DECL(0xfc0c4ef4, Node_OtherButtonDownEventArgs, y, y, kDataTypeFloat), // Node_OtherButtonDownEventArgs.y
	DECL(0xec56f27b, Node_OtherButtonDownEventArgs, deltaX, deltaX, kDataTypeInt), // Node_OtherButtonDownEventArgs.deltaX
	DECL(0xeb56f0e8, Node_OtherButtonDownEventArgs, deltaY, deltaY, kDataTypeInt), // Node_OtherButtonDownEventArgs.deltaY
	DECL(0x43b27471, Node_OtherButtonDownEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_OtherButtonDownEventArgs.button
	DECL(0xd9747336, Node_OtherButtonDownEventArgs, clickCount, clickCount, kDataTypeInt), // Node_OtherButtonDownEventArgs.clickCount
};
static luaL_Reg _Node_LeftButtonUpEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftButtonUpEventArgs[] = {
	DECL(0xfd0c5087, Node_LeftButtonUpEventArgs, x, x, kDataTypeFloat), // Node_LeftButtonUpEventArgs.x
	DECL(0xfc0c4ef4, Node_LeftButtonUpEventArgs, y, y, kDataTypeFloat), // Node_LeftButtonUpEventArgs.y
	DECL(0xec56f27b, Node_LeftButtonUpEventArgs, deltaX, deltaX, kDataTypeInt), // Node_LeftButtonUpEventArgs.deltaX
	DECL(0xeb56f0e8, Node_LeftButtonUpEventArgs, deltaY, deltaY, kDataTypeInt), // Node_LeftButtonUpEventArgs.deltaY
	DECL(0x43b27471, Node_LeftButtonUpEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_LeftButtonUpEventArgs.button
	DECL(0xd9747336, Node_LeftButtonUpEventArgs, clickCount, clickCount, kDataTypeInt), // Node_LeftButtonUpEventArgs.clickCount
};
static luaL_Reg _Node_RightButtonUpEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightButtonUpEventArgs[] = {
	DECL(0xfd0c5087, Node_RightButtonUpEventArgs, x, x, kDataTypeFloat), // Node_RightButtonUpEventArgs.x
	DECL(0xfc0c4ef4, Node_RightButtonUpEventArgs, y, y, kDataTypeFloat), // Node_RightButtonUpEventArgs.y
	DECL(0xec56f27b, Node_RightButtonUpEventArgs, deltaX, deltaX, kDataTypeInt), // Node_RightButtonUpEventArgs.deltaX
	DECL(0xeb56f0e8, Node_RightButtonUpEventArgs, deltaY, deltaY, kDataTypeInt), // Node_RightButtonUpEventArgs.deltaY
	DECL(0x43b27471, Node_RightButtonUpEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_RightButtonUpEventArgs.button
	DECL(0xd9747336, Node_RightButtonUpEventArgs, clickCount, clickCount, kDataTypeInt), // Node_RightButtonUpEventArgs.clickCount
};
static luaL_Reg _Node_OtherButtonUpEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherButtonUpEventArgs[] = {
	DECL(0xfd0c5087, Node_OtherButtonUpEventArgs, x, x, kDataTypeFloat), // Node_OtherButtonUpEventArgs.x
	DECL(0xfc0c4ef4, Node_OtherButtonUpEventArgs, y, y, kDataTypeFloat), // Node_OtherButtonUpEventArgs.y
	DECL(0xec56f27b, Node_OtherButtonUpEventArgs, deltaX, deltaX, kDataTypeInt), // Node_OtherButtonUpEventArgs.deltaX
	DECL(0xeb56f0e8, Node_OtherButtonUpEventArgs, deltaY, deltaY, kDataTypeInt), // Node_OtherButtonUpEventArgs.deltaY
	DECL(0x43b27471, Node_OtherButtonUpEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_OtherButtonUpEventArgs.button
	DECL(0xd9747336, Node_OtherButtonUpEventArgs, clickCount, clickCount, kDataTypeInt), // Node_OtherButtonUpEventArgs.clickCount
};
static luaL_Reg _Node_LeftButtonDraggedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftButtonDraggedEventArgs[] = {
	DECL(0xfd0c5087, Node_LeftButtonDraggedEventArgs, x, x, kDataTypeFloat), // Node_LeftButtonDraggedEventArgs.x
	DECL(0xfc0c4ef4, Node_LeftButtonDraggedEventArgs, y, y, kDataTypeFloat), // Node_LeftButtonDraggedEventArgs.y
	DECL(0xec56f27b, Node_LeftButtonDraggedEventArgs, deltaX, deltaX, kDataTypeInt), // Node_LeftButtonDraggedEventArgs.deltaX
	DECL(0xeb56f0e8, Node_LeftButtonDraggedEventArgs, deltaY, deltaY, kDataTypeInt), // Node_LeftButtonDraggedEventArgs.deltaY
	DECL(0x43b27471, Node_LeftButtonDraggedEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_LeftButtonDraggedEventArgs.button
	DECL(0xd9747336, Node_LeftButtonDraggedEventArgs, clickCount, clickCount, kDataTypeInt), // Node_LeftButtonDraggedEventArgs.clickCount
};
static luaL_Reg _Node_RightButtonDraggedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightButtonDraggedEventArgs[] = {
	DECL(0xfd0c5087, Node_RightButtonDraggedEventArgs, x, x, kDataTypeFloat), // Node_RightButtonDraggedEventArgs.x
	DECL(0xfc0c4ef4, Node_RightButtonDraggedEventArgs, y, y, kDataTypeFloat), // Node_RightButtonDraggedEventArgs.y
	DECL(0xec56f27b, Node_RightButtonDraggedEventArgs, deltaX, deltaX, kDataTypeInt), // Node_RightButtonDraggedEventArgs.deltaX
	DECL(0xeb56f0e8, Node_RightButtonDraggedEventArgs, deltaY, deltaY, kDataTypeInt), // Node_RightButtonDraggedEventArgs.deltaY
	DECL(0x43b27471, Node_RightButtonDraggedEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_RightButtonDraggedEventArgs.button
	DECL(0xd9747336, Node_RightButtonDraggedEventArgs, clickCount, clickCount, kDataTypeInt), // Node_RightButtonDraggedEventArgs.clickCount
};
static luaL_Reg _Node_OtherButtonDraggedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherButtonDraggedEventArgs[] = {
	DECL(0xfd0c5087, Node_OtherButtonDraggedEventArgs, x, x, kDataTypeFloat), // Node_OtherButtonDraggedEventArgs.x
	DECL(0xfc0c4ef4, Node_OtherButtonDraggedEventArgs, y, y, kDataTypeFloat), // Node_OtherButtonDraggedEventArgs.y
	DECL(0xec56f27b, Node_OtherButtonDraggedEventArgs, deltaX, deltaX, kDataTypeInt), // Node_OtherButtonDraggedEventArgs.deltaX
	DECL(0xeb56f0e8, Node_OtherButtonDraggedEventArgs, deltaY, deltaY, kDataTypeInt), // Node_OtherButtonDraggedEventArgs.deltaY
	DECL(0x43b27471, Node_OtherButtonDraggedEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_OtherButtonDraggedEventArgs.button
	DECL(0xd9747336, Node_OtherButtonDraggedEventArgs, clickCount, clickCount, kDataTypeInt), // Node_OtherButtonDraggedEventArgs.clickCount
};
static luaL_Reg _Node_LeftDoubleClickEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftDoubleClickEventArgs[] = {
	DECL(0xfd0c5087, Node_LeftDoubleClickEventArgs, x, x, kDataTypeFloat), // Node_LeftDoubleClickEventArgs.x
	DECL(0xfc0c4ef4, Node_LeftDoubleClickEventArgs, y, y, kDataTypeFloat), // Node_LeftDoubleClickEventArgs.y
	DECL(0xec56f27b, Node_LeftDoubleClickEventArgs, deltaX, deltaX, kDataTypeInt), // Node_LeftDoubleClickEventArgs.deltaX
	DECL(0xeb56f0e8, Node_LeftDoubleClickEventArgs, deltaY, deltaY, kDataTypeInt), // Node_LeftDoubleClickEventArgs.deltaY
	DECL(0x43b27471, Node_LeftDoubleClickEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_LeftDoubleClickEventArgs.button
	DECL(0xd9747336, Node_LeftDoubleClickEventArgs, clickCount, clickCount, kDataTypeInt), // Node_LeftDoubleClickEventArgs.clickCount
};
static luaL_Reg _Node_RightDoubleClickEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightDoubleClickEventArgs[] = {
	DECL(0xfd0c5087, Node_RightDoubleClickEventArgs, x, x, kDataTypeFloat), // Node_RightDoubleClickEventArgs.x
	DECL(0xfc0c4ef4, Node_RightDoubleClickEventArgs, y, y, kDataTypeFloat), // Node_RightDoubleClickEventArgs.y
	DECL(0xec56f27b, Node_RightDoubleClickEventArgs, deltaX, deltaX, kDataTypeInt), // Node_RightDoubleClickEventArgs.deltaX
	DECL(0xeb56f0e8, Node_RightDoubleClickEventArgs, deltaY, deltaY, kDataTypeInt), // Node_RightDoubleClickEventArgs.deltaY
	DECL(0x43b27471, Node_RightDoubleClickEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_RightDoubleClickEventArgs.button
	DECL(0xd9747336, Node_RightDoubleClickEventArgs, clickCount, clickCount, kDataTypeInt), // Node_RightDoubleClickEventArgs.clickCount
};
static luaL_Reg _Node_OtherDoubleClickEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherDoubleClickEventArgs[] = {
	DECL(0xfd0c5087, Node_OtherDoubleClickEventArgs, x, x, kDataTypeFloat), // Node_OtherDoubleClickEventArgs.x
	DECL(0xfc0c4ef4, Node_OtherDoubleClickEventArgs, y, y, kDataTypeFloat), // Node_OtherDoubleClickEventArgs.y
	DECL(0xec56f27b, Node_OtherDoubleClickEventArgs, deltaX, deltaX, kDataTypeInt), // Node_OtherDoubleClickEventArgs.deltaX
	DECL(0xeb56f0e8, Node_OtherDoubleClickEventArgs, deltaY, deltaY, kDataTypeInt), // Node_OtherDoubleClickEventArgs.deltaY
	DECL(0x43b27471, Node_OtherDoubleClickEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_OtherDoubleClickEventArgs.button
	DECL(0xd9747336, Node_OtherDoubleClickEventArgs, clickCount, clickCount, kDataTypeInt), // Node_OtherDoubleClickEventArgs.clickCount
};
static luaL_Reg _Node_MouseMovedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_MouseMovedEventArgs[] = {
	DECL(0xfd0c5087, Node_MouseMovedEventArgs, x, x, kDataTypeFloat), // Node_MouseMovedEventArgs.x
	DECL(0xfc0c4ef4, Node_MouseMovedEventArgs, y, y, kDataTypeFloat), // Node_MouseMovedEventArgs.y
	DECL(0xec56f27b, Node_MouseMovedEventArgs, deltaX, deltaX, kDataTypeInt), // Node_MouseMovedEventArgs.deltaX
	DECL(0xeb56f0e8, Node_MouseMovedEventArgs, deltaY, deltaY, kDataTypeInt), // Node_MouseMovedEventArgs.deltaY
	DECL(0x43b27471, Node_MouseMovedEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_MouseMovedEventArgs.button
	DECL(0xd9747336, Node_MouseMovedEventArgs, clickCount, clickCount, kDataTypeInt), // Node_MouseMovedEventArgs.clickCount
};
static luaL_Reg _Node_ScrollWheelEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_ScrollWheelEventArgs[] = {
	DECL(0xfd0c5087, Node_ScrollWheelEventArgs, x, x, kDataTypeFloat), // Node_ScrollWheelEventArgs.x
	DECL(0xfc0c4ef4, Node_ScrollWheelEventArgs, y, y, kDataTypeFloat), // Node_ScrollWheelEventArgs.y
	DECL(0xec56f27b, Node_ScrollWheelEventArgs, deltaX, deltaX, kDataTypeInt), // Node_ScrollWheelEventArgs.deltaX
	DECL(0xeb56f0e8, Node_ScrollWheelEventArgs, deltaY, deltaY, kDataTypeInt), // Node_ScrollWheelEventArgs.deltaY
	DECL(0x43b27471, Node_ScrollWheelEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_ScrollWheelEventArgs.button
	DECL(0xd9747336, Node_ScrollWheelEventArgs, clickCount, clickCount, kDataTypeInt), // Node_ScrollWheelEventArgs.clickCount
};
static luaL_Reg _Node_DragDropEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_DragDropEventArgs[] = {
	DECL(0xfd0c5087, Node_DragDropEventArgs, x, x, kDataTypeFloat), // Node_DragDropEventArgs.x
	DECL(0xfc0c4ef4, Node_DragDropEventArgs, y, y, kDataTypeFloat), // Node_DragDropEventArgs.y
	DECL(0xec56f27b, Node_DragDropEventArgs, deltaX, deltaX, kDataTypeInt), // Node_DragDropEventArgs.deltaX
	DECL(0xeb56f0e8, Node_DragDropEventArgs, deltaY, deltaY, kDataTypeInt), // Node_DragDropEventArgs.deltaY
	DECL(0x43b27471, Node_DragDropEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_DragDropEventArgs.button
	DECL(0xd9747336, Node_DragDropEventArgs, clickCount, clickCount, kDataTypeInt), // Node_DragDropEventArgs.clickCount
};
static luaL_Reg _Node_DragEnterEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_DragEnterEventArgs[] = {
	DECL(0xfd0c5087, Node_DragEnterEventArgs, x, x, kDataTypeFloat), // Node_DragEnterEventArgs.x
	DECL(0xfc0c4ef4, Node_DragEnterEventArgs, y, y, kDataTypeFloat), // Node_DragEnterEventArgs.y
	DECL(0xec56f27b, Node_DragEnterEventArgs, deltaX, deltaX, kDataTypeInt), // Node_DragEnterEventArgs.deltaX
	DECL(0xeb56f0e8, Node_DragEnterEventArgs, deltaY, deltaY, kDataTypeInt), // Node_DragEnterEventArgs.deltaY
	DECL(0x43b27471, Node_DragEnterEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_DragEnterEventArgs.button
	DECL(0xd9747336, Node_DragEnterEventArgs, clickCount, clickCount, kDataTypeInt), // Node_DragEnterEventArgs.clickCount
};
static luaL_Reg _Node_KeyMessageEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KeyMessageEventArgs[] = {
	DECL(0xd803e2a9, Node_KeyMessageEventArgs, keyCode, keyCode, kDataTypeInt), // Node_KeyMessageEventArgs.keyCode
	DECL(0x8b3aa710, Node_KeyMessageEventArgs, character, character, kDataTypeInt), // Node_KeyMessageEventArgs.character
	DECL(0x411b1019, Node_KeyMessageEventArgs, modifiers, modifiers, kDataTypeInt), // Node_KeyMessageEventArgs.modifiers
	DECL(0xbde64e3e, Node_KeyMessageEventArgs, text, text, kDataTypeString), // Node_KeyMessageEventArgs.text
	DECL(0x8f82be34, Node_KeyMessageEventArgs, modifiersString, modifiersString, kDataTypeString), // Node_KeyMessageEventArgs.modifiersString
	DECL(0x9703dc13, Node_KeyMessageEventArgs, hotKey, hotKey, kDataTypeString), // Node_KeyMessageEventArgs.hotKey
};
static luaL_Reg _Node_KeyDownEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KeyDownEventArgs[] = {
	DECL(0xd803e2a9, Node_KeyDownEventArgs, keyCode, keyCode, kDataTypeInt), // Node_KeyDownEventArgs.keyCode
	DECL(0x8b3aa710, Node_KeyDownEventArgs, character, character, kDataTypeInt), // Node_KeyDownEventArgs.character
	DECL(0x411b1019, Node_KeyDownEventArgs, modifiers, modifiers, kDataTypeInt), // Node_KeyDownEventArgs.modifiers
	DECL(0xbde64e3e, Node_KeyDownEventArgs, text, text, kDataTypeString), // Node_KeyDownEventArgs.text
	DECL(0x8f82be34, Node_KeyDownEventArgs, modifiersString, modifiersString, kDataTypeString), // Node_KeyDownEventArgs.modifiersString
	DECL(0x9703dc13, Node_KeyDownEventArgs, hotKey, hotKey, kDataTypeString), // Node_KeyDownEventArgs.hotKey
};
static luaL_Reg _Node_KeyUpEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KeyUpEventArgs[] = {
	DECL(0xd803e2a9, Node_KeyUpEventArgs, keyCode, keyCode, kDataTypeInt), // Node_KeyUpEventArgs.keyCode
	DECL(0x8b3aa710, Node_KeyUpEventArgs, character, character, kDataTypeInt), // Node_KeyUpEventArgs.character
	DECL(0x411b1019, Node_KeyUpEventArgs, modifiers, modifiers, kDataTypeInt), // Node_KeyUpEventArgs.modifiers
	DECL(0xbde64e3e, Node_KeyUpEventArgs, text, text, kDataTypeString), // Node_KeyUpEventArgs.text
	DECL(0x8f82be34, Node_KeyUpEventArgs, modifiersString, modifiersString, kDataTypeString), // Node_KeyUpEventArgs.modifiersString
	DECL(0x9703dc13, Node_KeyUpEventArgs, hotKey, hotKey, kDataTypeString), // Node_KeyUpEventArgs.hotKey
};
static luaL_Reg _Node_TextInputEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_TextInputEventArgs[] = {
	DECL(0xd803e2a9, Node_TextInputEventArgs, keyCode, keyCode, kDataTypeInt), // Node_TextInputEventArgs.keyCode
	DECL(0x8b3aa710, Node_TextInputEventArgs, character, character, kDataTypeInt), // Node_TextInputEventArgs.character
	DECL(0x411b1019, Node_TextInputEventArgs, modifiers, modifiers, kDataTypeInt), // Node_TextInputEventArgs.modifiers
	DECL(0xbde64e3e, Node_TextInputEventArgs, text, text, kDataTypeString), // Node_TextInputEventArgs.text
	DECL(0x8f82be34, Node_TextInputEventArgs, modifiersString, modifiersString, kDataTypeString), // Node_TextInputEventArgs.modifiersString
	DECL(0x9703dc13, Node_TextInputEventArgs, hotKey, hotKey, kDataTypeString), // Node_TextInputEventArgs.hotKey
};

STRUCT(Object_CreateEventArgs, Object_CreateEventArgs);
STRUCT(Object_StartEventArgs, Object_StartEventArgs);
STRUCT(Object_AnimateEventArgs, Object_AnimateEventArgs);
STRUCT(StyleController_ThemeChangedEventArgs, StyleController_ThemeChangedEventArgs);
STRUCT(Object_PropertyChangedEventArgs, Object_PropertyChangedEventArgs);
STRUCT(Object_AttachedEventArgs, Object_AttachedEventArgs);
STRUCT(Object_ReleaseEventArgs, Object_ReleaseEventArgs);
STRUCT(Object_DestroyEventArgs, Object_DestroyEventArgs);
STRUCT(Object_TimerEventArgs, Object_TimerEventArgs);
STRUCT(AnimationPlayer_PlayEventArgs, AnimationPlayer_PlayEventArgs);
STRUCT(AnimationPlayer_ResumeEventArgs, AnimationPlayer_ResumeEventArgs);
STRUCT(AnimationPlayer_StopEventArgs, AnimationPlayer_StopEventArgs);
STRUCT(AnimationPlayer_PauseEventArgs, AnimationPlayer_PauseEventArgs);
STRUCT(AnimationPlayer_StartedEventArgs, AnimationPlayer_StartedEventArgs);
STRUCT(AnimationPlayer_StoppedEventArgs, AnimationPlayer_StoppedEventArgs);
STRUCT(AnimationPlayer_CompletedEventArgs, AnimationPlayer_CompletedEventArgs);
STRUCT(StyleController_AddClassEventArgs, StyleController_AddClassEventArgs);
STRUCT(StyleController_AddClassesEventArgs, StyleController_AddClassesEventArgs);
STRUCT(StateManagerController_ControllerChangedEventArgs, StateManagerController_ControllerChangedEventArgs);
STRUCT(Trigger_TriggeredEventArgs, Trigger_TriggeredEventArgs);
STRUCT(Node_UpdateMatrixEventArgs, Node_UpdateMatrixEventArgs);
STRUCT(Node_LoadViewEventArgs, Node_LoadViewEventArgs);
STRUCT(Node_HitTestEventArgs, Node_HitTestEventArgs);
STRUCT(Node_IsVisibleEventArgs, Node_IsVisibleEventArgs);
STRUCT(Node_ViewDidLoadEventArgs, Node_ViewDidLoadEventArgs);
STRUCT(Node_KillFocusEventArgs, Node_KillFocusEventArgs);
STRUCT(Node_SetFocusEventArgs, Node_SetFocusEventArgs);
STRUCT(Node_GetSizeEventArgs, Node_GetSizeEventArgs);
STRUCT(Node_MouseMessageEventArgs, Node_MouseMessageEventArgs);
STRUCT(Node_LeftButtonDownEventArgs, Node_LeftButtonDownEventArgs);
STRUCT(Node_RightButtonDownEventArgs, Node_RightButtonDownEventArgs);
STRUCT(Node_OtherButtonDownEventArgs, Node_OtherButtonDownEventArgs);
STRUCT(Node_LeftButtonUpEventArgs, Node_LeftButtonUpEventArgs);
STRUCT(Node_RightButtonUpEventArgs, Node_RightButtonUpEventArgs);
STRUCT(Node_OtherButtonUpEventArgs, Node_OtherButtonUpEventArgs);
STRUCT(Node_LeftButtonDraggedEventArgs, Node_LeftButtonDraggedEventArgs);
STRUCT(Node_RightButtonDraggedEventArgs, Node_RightButtonDraggedEventArgs);
STRUCT(Node_OtherButtonDraggedEventArgs, Node_OtherButtonDraggedEventArgs);
STRUCT(Node_LeftDoubleClickEventArgs, Node_LeftDoubleClickEventArgs);
STRUCT(Node_RightDoubleClickEventArgs, Node_RightDoubleClickEventArgs);
STRUCT(Node_OtherDoubleClickEventArgs, Node_OtherDoubleClickEventArgs);
STRUCT(Node_MouseMovedEventArgs, Node_MouseMovedEventArgs);
STRUCT(Node_ScrollWheelEventArgs, Node_ScrollWheelEventArgs);
STRUCT(Node_DragDropEventArgs, Node_DragDropEventArgs);
STRUCT(Node_DragEnterEventArgs, Node_DragEnterEventArgs);
STRUCT(Node_KeyMessageEventArgs, Node_KeyMessageEventArgs);
STRUCT(Node_KeyDownEventArgs, Node_KeyDownEventArgs);
STRUCT(Node_KeyUpEventArgs, Node_KeyUpEventArgs);
STRUCT(Node_TextInputEventArgs, Node_TextInputEventArgs);
static struct PropertyType const AnimationCurveProperties[kAnimationCurveNumProperties] = {
	DECL(0xeb66e456, AnimationCurve, Path, Path, kDataTypeString), // AnimationCurve.Path
	DECL(0x5221f9e8, AnimationCurve, Property, Property, kDataTypeString), // AnimationCurve.Property
	ARRAY_DECL(0xf893ff8e, AnimationCurve, Keyframes, Keyframes, kDataTypeStruct, .TypeString = "Keyframe"), // AnimationCurve.Keyframes
	DECL(0x33ee8bf8, AnimationCurve, NumKeyframes, NumKeyframes, kDataTypeInt), // AnimationCurve.NumKeyframes
};
static struct AnimationCurve AnimationCurveDefaults = {
};
LRESULT AnimationCurveProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushAnimationCurve(lua_State *L, struct AnimationCurve const* AnimationCurve) {
	luaX_pushObject(L, CMP_GetObject(AnimationCurve));
}
struct AnimationCurve* luaX_checkAnimationCurve(lua_State *L, int idx) {
	return GetAnimationCurve(luaX_checkObject(L, idx));
}
REGISTER_CLASS(AnimationCurve, 0);
HANDLER(AnimationClip, Object, Start);
static struct PropertyType const AnimationClipProperties[kAnimationClipNumProperties] = {
	DECL(0x534e7732, AnimationClip, Mode, Mode, kDataTypeEnum, .EnumValues = _AnimationMode), // AnimationClip.Mode
	DECL(0xd6195a6e, AnimationClip, StartTime, StartTime, kDataTypeFloat), // AnimationClip.StartTime
	DECL(0x03274144, AnimationClip, StopTime, StopTime, kDataTypeFloat), // AnimationClip.StopTime
};
static struct AnimationClip AnimationClipDefaults = {
};
LRESULT AnimationClipProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Start: return AnimationClip_Start(object, cmp, wparm, lparm); // Object.Start
	}
	return FALSE;
}
void luaX_pushAnimationClip(lua_State *L, struct AnimationClip const* AnimationClip) {
	luaX_pushObject(L, CMP_GetObject(AnimationClip));
}
struct AnimationClip* luaX_checkAnimationClip(lua_State *L, int idx) {
	return GetAnimationClip(luaX_checkObject(L, idx));
}
REGISTER_CLASS(AnimationClip, 0);
HANDLER(AnimationPlayer, Object, Start);
HANDLER(AnimationPlayer, Object, Animate);
HANDLER(AnimationPlayer, AnimationPlayer, Play);
HANDLER(AnimationPlayer, AnimationPlayer, Resume);
HANDLER(AnimationPlayer, AnimationPlayer, Stop);
HANDLER(AnimationPlayer, AnimationPlayer, Pause);
static struct PropertyType const AnimationPlayerProperties[kAnimationPlayerNumProperties] = {
	DECL(0xd33ddb1b, AnimationPlayer, Clip, Clip, kDataTypeObject, .TypeString = "AnimationClip"), // AnimationPlayer.Clip
	ARRAY_DECL(0xf26064b8, AnimationPlayer, Clips, Clips, kDataTypeStruct, .TypeString = "AnimationClipReference"), // AnimationPlayer.Clips
	DECL(0x090c1a52, AnimationPlayer, NumClips, NumClips, kDataTypeInt), // AnimationPlayer.NumClips
	DECL(0xdf450ad5, AnimationPlayer, Playing, Playing, kDataTypeBool), // AnimationPlayer.Playing
	DECL(0x343782cd, AnimationPlayer, Looping, Looping, kDataTypeBool), // AnimationPlayer.Looping
	DECL(0x0a6b8020, AnimationPlayer, Speed, Speed, kDataTypeFloat), // AnimationPlayer.Speed
	DECL(0x0d3e3b9b, AnimationPlayer, CurrentTime, CurrentTime, kDataTypeFloat), // AnimationPlayer.CurrentTime
	DECL(0x706b62d9, AnimationPlayer, AutoplayEnabled, AutoplayEnabled, kDataTypeBool), // AnimationPlayer.AutoplayEnabled
	DECL(0x9b01fbb4, AnimationPlayer, RelativePlayback, RelativePlayback, kDataTypeBool), // AnimationPlayer.RelativePlayback
	DECL(0x280cbcbb, AnimationPlayer, RestoreOriginalValuesAfterPlayback, RestoreOriginalValuesAfterPlayback, kDataTypeBool), // AnimationPlayer.RestoreOriginalValuesAfterPlayback
	DECL(0x234c71cf, AnimationPlayer, PlaybackMode, PlaybackMode, kDataTypeEnum, .EnumValues = _PlaybackMode), // AnimationPlayer.PlaybackMode
	DECL(0x9bcd7639, AnimationPlayer, DurationScale, DurationScale, kDataTypeFloat), // AnimationPlayer.DurationScale
	DECL(0xa3a5f0a1, AnimationPlayer, RepeatCount, RepeatCount, kDataTypeInt), // AnimationPlayer.RepeatCount
	DECL(0x29ab6f83, AnimationPlayer, Play, Play, kDataTypeEvent, .TypeString = "AnimationPlayer_PlayEventArgs"), // AnimationPlayer.Play
	DECL(0x454d1d8a, AnimationPlayer, Resume, Resume, kDataTypeEvent, .TypeString = "AnimationPlayer_ResumeEventArgs"), // AnimationPlayer.Resume
	DECL(0x4b7f7705, AnimationPlayer, Stop, Stop, kDataTypeEvent, .TypeString = "AnimationPlayer_StopEventArgs"), // AnimationPlayer.Stop
	DECL(0x44f9bf2d, AnimationPlayer, Pause, Pause, kDataTypeEvent, .TypeString = "AnimationPlayer_PauseEventArgs"), // AnimationPlayer.Pause
	DECL(0x97b5823e, AnimationPlayer, Started, Started, kDataTypeEvent, .TypeString = "AnimationPlayer_StartedEventArgs"), // AnimationPlayer.Started
	DECL(0x863f19ee, AnimationPlayer, Stopped, Stopped, kDataTypeEvent, .TypeString = "AnimationPlayer_StoppedEventArgs"), // AnimationPlayer.Stopped
	DECL(0xec88aab2, AnimationPlayer, Completed, Completed, kDataTypeEvent, .TypeString = "AnimationPlayer_CompletedEventArgs"), // AnimationPlayer.Completed
};
static struct AnimationPlayer AnimationPlayerDefaults = {
		
  .Speed = 1.0,
};
LRESULT AnimationPlayerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Start: return AnimationPlayer_Start(object, cmp, wparm, lparm); // Object.Start
		case ID_Object_Animate: return AnimationPlayer_Animate(object, cmp, wparm, lparm); // Object.Animate
		case ID_AnimationPlayer_Play: return AnimationPlayer_Play(object, cmp, wparm, lparm); // AnimationPlayer.Play
		case ID_AnimationPlayer_Resume: return AnimationPlayer_Resume(object, cmp, wparm, lparm); // AnimationPlayer.Resume
		case ID_AnimationPlayer_Stop: return AnimationPlayer_Stop(object, cmp, wparm, lparm); // AnimationPlayer.Stop
		case ID_AnimationPlayer_Pause: return AnimationPlayer_Pause(object, cmp, wparm, lparm); // AnimationPlayer.Pause
	}
	return FALSE;
}
void luaX_pushAnimationPlayer(lua_State *L, struct AnimationPlayer const* AnimationPlayer) {
	luaX_pushObject(L, CMP_GetObject(AnimationPlayer));
}
struct AnimationPlayer* luaX_checkAnimationPlayer(lua_State *L, int idx) {
	return GetAnimationPlayer(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(AnimationPlayer, 0);
HANDLER(PropertyAnimation, Object, Animate);
HANDLER(PropertyAnimation, Object, Release);
static struct PropertyType const PropertyAnimationProperties[kPropertyAnimationNumProperties] = {
	DECL(0x5221f9e8, PropertyAnimation, Property, Property, kDataTypeString), // PropertyAnimation.Property
	DECL(0x18743595, PropertyAnimation, From, From, kDataTypeString), // PropertyAnimation.From
	DECL(0x41f59644, PropertyAnimation, To, To, kDataTypeString), // PropertyAnimation.To
	DECL(0x77818ebd, PropertyAnimation, Interpolation, Interpolation, kDataTypeEnum, .EnumValues = _InterpolationMode), // PropertyAnimation.Interpolation
	DECL(0xa3116148, PropertyAnimation, Easing, Easing, kDataTypeEnum, .EnumValues = _Easing), // PropertyAnimation.Easing
	DECL(0x0ae8097f, PropertyAnimation, Start, Start, kDataTypeInt), // PropertyAnimation.Start
	DECL(0xd4c7492d, PropertyAnimation, Duration, Duration, kDataTypeInt), // PropertyAnimation.Duration
};
static struct PropertyAnimation PropertyAnimationDefaults = {
};
LRESULT PropertyAnimationProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Animate: return PropertyAnimation_Animate(object, cmp, wparm, lparm); // Object.Animate
		case ID_Object_Release: return PropertyAnimation_Release(object, cmp, wparm, lparm); // Object.Release
	}
	return FALSE;
}
void luaX_pushPropertyAnimation(lua_State *L, struct PropertyAnimation const* PropertyAnimation) {
	luaX_pushObject(L, CMP_GetObject(PropertyAnimation));
}
struct PropertyAnimation* luaX_checkPropertyAnimation(lua_State *L, int idx) {
	return GetPropertyAnimation(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(PropertyAnimation, 0);
static struct PropertyType const StyleSheetProperties[kStyleSheetNumProperties] = {
};
static struct StyleSheet StyleSheetDefaults = {
};
LRESULT StyleSheetProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushStyleSheet(lua_State *L, struct StyleSheet const* StyleSheet) {
	luaX_pushObject(L, CMP_GetObject(StyleSheet));
}
struct StyleSheet* luaX_checkStyleSheet(lua_State *L, int idx) {
	return GetStyleSheet(luaX_checkObject(L, idx));
}
REGISTER_CLASS(StyleSheet, 0);
HANDLER(StyleRule, Object, PropertyChanged);
static struct PropertyType const StyleRuleProperties[kStyleRuleNumProperties] = {
	DECL(0x2ecda0bc, StyleRule, ClassName, ClassName, kDataTypeString), // StyleRule.ClassName
	DECL(0xc0e9c7e9, StyleRule, PseudoClass, PseudoClass, kDataTypeString), // StyleRule.PseudoClass
};
static struct StyleRule StyleRuleDefaults = {
};
LRESULT StyleRuleProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_PropertyChanged: return StyleRule_PropertyChanged(object, cmp, wparm, lparm); // Object.PropertyChanged
	}
	return FALSE;
}
void luaX_pushStyleRule(lua_State *L, struct StyleRule const* StyleRule) {
	luaX_pushObject(L, CMP_GetObject(StyleRule));
}
struct StyleRule* luaX_checkStyleRule(lua_State *L, int idx) {
	return GetStyleRule(luaX_checkObject(L, idx));
}
REGISTER_CLASS(StyleRule, 0);
HANDLER(StyleController, Object, Create);
HANDLER(StyleController, Object, Release);
HANDLER(StyleController, StyleController, ThemeChanged);
HANDLER(StyleController, StyleController, AddClass);
HANDLER(StyleController, StyleController, AddClasses);
static struct PropertyType const StyleControllerProperties[kStyleControllerNumProperties] = {
	DECL(0x6546e1e1, StyleController, StyleSheet, StyleSheet, kDataTypeObject, .TypeString = "StyleSheet"), // StyleController.StyleSheet
	DECL(0x064087a6, StyleController, ThemeChanged, ThemeChanged, kDataTypeEvent, .TypeString = "StyleController_ThemeChangedEventArgs"), // StyleController.ThemeChanged
	DECL(0x543ca51c, StyleController, AddClass, AddClass, kDataTypeEvent, .TypeString = "StyleController_AddClassEventArgs"), // StyleController.AddClass
	DECL(0x41acd398, StyleController, AddClasses, AddClasses, kDataTypeEvent, .TypeString = "StyleController_AddClassesEventArgs"), // StyleController.AddClasses
};
static struct StyleController StyleControllerDefaults = {
};
LRESULT StyleControllerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Create: return StyleController_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Object_Release: return StyleController_Release(object, cmp, wparm, lparm); // Object.Release
		case ID_StyleController_ThemeChanged: return StyleController_ThemeChanged(object, cmp, wparm, lparm); // StyleController.ThemeChanged
		case ID_StyleController_AddClass: return StyleController_AddClass(object, cmp, wparm, lparm); // StyleController.AddClass
		case ID_StyleController_AddClasses: return StyleController_AddClasses(object, cmp, wparm, lparm); // StyleController.AddClasses
	}
	return FALSE;
}
void luaX_pushStyleController(lua_State *L, struct StyleController const* StyleController) {
	luaX_pushObject(L, CMP_GetObject(StyleController));
}
struct StyleController* luaX_checkStyleController(lua_State *L, int idx) {
	return GetStyleController(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(StyleController, 0);
static struct PropertyType const StateManagerProperties[kStateManagerNumProperties] = {
};
static struct StateManager StateManagerDefaults = {
};
LRESULT StateManagerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushStateManager(lua_State *L, struct StateManager const* StateManager) {
	luaX_pushObject(L, CMP_GetObject(StateManager));
}
struct StateManager* luaX_checkStateManager(lua_State *L, int idx) {
	return GetStateManager(luaX_checkObject(L, idx));
}
REGISTER_CLASS(StateManager, 0);
static struct PropertyType const StateGroupProperties[kStateGroupNumProperties] = {
	DECL(0xd9dc005c, StateGroup, ControllerProperty, ControllerProperty, kDataTypeString), // StateGroup.ControllerProperty
};
static struct StateGroup StateGroupDefaults = {
};
LRESULT StateGroupProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushStateGroup(lua_State *L, struct StateGroup const* StateGroup) {
	luaX_pushObject(L, CMP_GetObject(StateGroup));
}
struct StateGroup* luaX_checkStateGroup(lua_State *L, int idx) {
	return GetStateGroup(luaX_checkObject(L, idx));
}
REGISTER_CLASS(StateGroup, 0);
static struct PropertyType const StateProperties[kStateNumProperties] = {
	DECL(0xd147f96a, State, Value, Value, kDataTypeString), // State.Value
	DECL(0xeb66e456, State, Path, Path, kDataTypeString), // State.Path
};
static struct State StateDefaults = {
};
LRESULT StateProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushState(lua_State *L, struct State const* State) {
	luaX_pushObject(L, CMP_GetObject(State));
}
struct State* luaX_checkState(lua_State *L, int idx) {
	return GetState(luaX_checkObject(L, idx));
}
REGISTER_CLASS(State, 0);
HANDLER(StateManagerController, Object, Start);
HANDLER(StateManagerController, Object, PropertyChanged);
HANDLER(StateManagerController, StateManagerController, ControllerChanged);
static struct PropertyType const StateManagerControllerProperties[kStateManagerControllerNumProperties] = {
	DECL(0xe76f2815, StateManagerController, StateManager, StateManager, kDataTypeObject, .TypeString = "StateManager"), // StateManagerController.StateManager
	DECL(0xda0795ff, StateManagerController, ControllerChanged, ControllerChanged, kDataTypeEvent, .TypeString = "StateManagerController_ControllerChangedEventArgs"), // StateManagerController.ControllerChanged
};
static struct StateManagerController StateManagerControllerDefaults = {
};
LRESULT StateManagerControllerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Start: return StateManagerController_Start(object, cmp, wparm, lparm); // Object.Start
		case ID_Object_PropertyChanged: return StateManagerController_PropertyChanged(object, cmp, wparm, lparm); // Object.PropertyChanged
		case ID_StateManagerController_ControllerChanged: return StateManagerController_ControllerChanged(object, cmp, wparm, lparm); // StateManagerController.ControllerChanged
	}
	return FALSE;
}
void luaX_pushStateManagerController(lua_State *L, struct StateManagerController const* StateManagerController) {
	luaX_pushObject(L, CMP_GetObject(StateManagerController));
}
struct StateManagerController* luaX_checkStateManagerController(lua_State *L, int idx) {
	return GetStateManagerController(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(StateManagerController, 0);
static struct PropertyType const ResourceDictionaryProperties[kResourceDictionaryNumProperties] = {
};
static struct ResourceDictionary ResourceDictionaryDefaults = {
};
LRESULT ResourceDictionaryProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushResourceDictionary(lua_State *L, struct ResourceDictionary const* ResourceDictionary) {
	luaX_pushObject(L, CMP_GetObject(ResourceDictionary));
}
struct ResourceDictionary* luaX_checkResourceDictionary(lua_State *L, int idx) {
	return GetResourceDictionary(luaX_checkObject(L, idx));
}
REGISTER_CLASS(ResourceDictionary, 0);
static struct PropertyType const LocaleProperties[kLocaleNumProperties] = {
	DECL(0x9a73db9b, Locale, Language, Language, kDataTypeString), // Locale.Language
	ARRAY_DECL(0xdcaca293, Locale, Entries, Entries, kDataTypeStruct, .TypeString = "LocaleEntry"), // Locale.Entries
	DECL(0x67d575c1, Locale, NumEntries, NumEntries, kDataTypeInt), // Locale.NumEntries
};
static struct Locale LocaleDefaults = {
};
LRESULT LocaleProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushLocale(lua_State *L, struct Locale const* Locale) {
	luaX_pushObject(L, CMP_GetObject(Locale));
}
struct Locale* luaX_checkLocale(lua_State *L, int idx) {
	return GetLocale(luaX_checkObject(L, idx));
}
#define ID_ResourceDictionary 0xde7febc5
REGISTER_CLASS(Locale, ID_ResourceDictionary, 0);
static struct PropertyType const DataObjectProperties[kDataObjectNumProperties] = {
};
static struct DataObject DataObjectDefaults = {
};
LRESULT DataObjectProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushDataObject(lua_State *L, struct DataObject const* DataObject) {
	luaX_pushObject(L, CMP_GetObject(DataObject));
}
struct DataObject* luaX_checkDataObject(lua_State *L, int idx) {
	return GetDataObject(luaX_checkObject(L, idx));
}
REGISTER_CLASS(DataObject, 0);
HANDLER(Trigger, Object, PropertyChanged);
HANDLER(Trigger, Object, Attached);
static struct PropertyType const TriggerProperties[kTriggerNumProperties] = {
	DECL(0x5221f9e8, Trigger, Property, Property, kDataTypeString), // Trigger.Property
	DECL(0xd147f96a, Trigger, Value, Value, kDataTypeInt), // Trigger.Value
	DECL(0x3b1c3ae2, Trigger, Triggered, Triggered, kDataTypeEvent, .TypeString = "Trigger_TriggeredEventArgs"), // Trigger.Triggered
};
static struct Trigger TriggerDefaults = {
};
LRESULT TriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_PropertyChanged: return Trigger_PropertyChanged(object, cmp, wparm, lparm); // Object.PropertyChanged
		case ID_Object_Attached: return Trigger_Attached(object, cmp, wparm, lparm); // Object.Attached
	}
	return FALSE;
}
void luaX_pushTrigger(lua_State *L, struct Trigger const* Trigger) {
	luaX_pushObject(L, CMP_GetObject(Trigger));
}
struct Trigger* luaX_checkTrigger(lua_State *L, int idx) {
	return GetTrigger(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(Trigger, 0);
HANDLER(OnPropertyChangedTrigger, Object, PropertyChanged);
HANDLER(OnPropertyChangedTrigger, Object, Attached);
static struct PropertyType const OnPropertyChangedTriggerProperties[kOnPropertyChangedTriggerNumProperties] = {
	DECL(0x9ff03304, OnPropertyChangedTrigger, SourceNode, SourceNode, kDataTypeString), // OnPropertyChangedTrigger.SourceNode
	DECL(0x5221f9e8, OnPropertyChangedTrigger, Property, Property, kDataTypeString), // OnPropertyChangedTrigger.Property
};
static struct OnPropertyChangedTrigger OnPropertyChangedTriggerDefaults = {
};
LRESULT OnPropertyChangedTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_PropertyChanged: return OnPropertyChangedTrigger_PropertyChanged(object, cmp, wparm, lparm); // Object.PropertyChanged
		case ID_Object_Attached: return OnPropertyChangedTrigger_Attached(object, cmp, wparm, lparm); // Object.Attached
	}
	return FALSE;
}
void luaX_pushOnPropertyChangedTrigger(lua_State *L, struct OnPropertyChangedTrigger const* OnPropertyChangedTrigger) {
	luaX_pushObject(L, CMP_GetObject(OnPropertyChangedTrigger));
}
struct OnPropertyChangedTrigger* luaX_checkOnPropertyChangedTrigger(lua_State *L, int idx) {
	return GetOnPropertyChangedTrigger(luaX_checkObject(L, idx));
}
#define ID_Trigger 0xa5ea0da3
REGISTER_ATTACH_ONLY_CLASS(OnPropertyChangedTrigger, ID_Trigger, 0);
HANDLER(OnAttachedTrigger, Object, Attached);
static struct PropertyType const OnAttachedTriggerProperties[kOnAttachedTriggerNumProperties] = {
};
static struct OnAttachedTrigger OnAttachedTriggerDefaults = {
};
LRESULT OnAttachedTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Attached: return OnAttachedTrigger_Attached(object, cmp, wparm, lparm); // Object.Attached
	}
	return FALSE;
}
void luaX_pushOnAttachedTrigger(lua_State *L, struct OnAttachedTrigger const* OnAttachedTrigger) {
	luaX_pushObject(L, CMP_GetObject(OnAttachedTrigger));
}
struct OnAttachedTrigger* luaX_checkOnAttachedTrigger(lua_State *L, int idx) {
	return GetOnAttachedTrigger(luaX_checkObject(L, idx));
}
#define ID_Trigger 0xa5ea0da3
REGISTER_ATTACH_ONLY_CLASS(OnAttachedTrigger, ID_Trigger, 0);
static struct PropertyType const EventTriggerProperties[kEventTriggerNumProperties] = {
	DECL(0x30d77e1a, EventTrigger, RoutedEvent, RoutedEvent, kDataTypeString), // EventTrigger.RoutedEvent
};
static struct EventTrigger EventTriggerDefaults = {
};
LRESULT EventTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushEventTrigger(lua_State *L, struct EventTrigger const* EventTrigger) {
	luaX_pushObject(L, CMP_GetObject(EventTrigger));
}
struct EventTrigger* luaX_checkEventTrigger(lua_State *L, int idx) {
	return GetEventTrigger(luaX_checkObject(L, idx));
}
#define ID_Trigger 0xa5ea0da3
REGISTER_ATTACH_ONLY_CLASS(EventTrigger, ID_Trigger, 0);
HANDLER(Setter, Trigger, Triggered);
static struct PropertyType const SetterProperties[kSetterNumProperties] = {
	DECL(0xa5ea0da3, Setter, Trigger, Trigger, kDataTypeObject, .TypeString = "Trigger"), // Setter.Trigger
	DECL(0x5221f9e8, Setter, Property, Property, kDataTypeString), // Setter.Property
	DECL(0xd147f96a, Setter, Value, Value, kDataTypeString), // Setter.Value
};
static struct Setter SetterDefaults = {
};
LRESULT SetterProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Trigger_Triggered: return Setter_Triggered(object, cmp, wparm, lparm); // Trigger.Triggered
	}
	return FALSE;
}
void luaX_pushSetter(lua_State *L, struct Setter const* Setter) {
	luaX_pushObject(L, CMP_GetObject(Setter));
}
struct Setter* luaX_checkSetter(lua_State *L, int idx) {
	return GetSetter(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(Setter, 0);
HANDLER(Handler, Trigger, Triggered);
static struct PropertyType const HandlerProperties[kHandlerNumProperties] = {
	DECL(0xa5ea0da3, Handler, Trigger, Trigger, kDataTypeObject, .TypeString = "Trigger"), // Handler.Trigger
	DECL(0x8b67f168, Handler, Target, Target, kDataTypeObject, .TypeString = "Node"), // Handler.Target
	DECL(0x98a79a69, Handler, Function, Function, kDataTypeString), // Handler.Function
};
static struct Handler HandlerDefaults = {
};
LRESULT HandlerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Trigger_Triggered: return Handler_Triggered(object, cmp, wparm, lparm); // Trigger.Triggered
	}
	return FALSE;
}
void luaX_pushHandler(lua_State *L, struct Handler const* Handler) {
	luaX_pushObject(L, CMP_GetObject(Handler));
}
struct Handler* luaX_checkHandler(lua_State *L, int idx) {
	return GetHandler(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(Handler, 0);
HANDLER(Node, Node, GetSize);
HANDLER(Node, Node, IsVisible);
HANDLER(Node, Object, Start);
static struct PropertyType const NodeProperties[kNodeNumProperties] = {
	DECL(0xa6478e7c, Node, Size, Size, kDataTypeStruct, .TypeString = "SizeShorthand"), // Node.Size
	DECL(0x2dbf56d8, Node, HorizontalSize, Size.Axis[0], kDataTypeStruct, .TypeString = "SizeAxisShorthand"), // Node.HorizontalSize
	DECL(0x3b42dfbf, Node, Width, Size.Axis[0].Requested, kDataTypeFloat), // Node.Width
	DECL(0xbe3d446f, Node, DesiredWidth, Size.Axis[0].Desired, kDataTypeFloat), // Node.DesiredWidth
	DECL(0xfe87ddd9, Node, MinWidth, Size.Axis[0].Min, kDataTypeFloat), // Node.MinWidth
	DECL(0xa6d77d39, Node, ActualWidth, Size.Axis[0].Actual, kDataTypeFloat), // Node.ActualWidth
	DECL(0x29366f18, Node, ScrollWidth, Size.Axis[0].Scroll, kDataTypeFloat), // Node.ScrollWidth
	DECL(0x629fba9a, Node, VerticalSize, Size.Axis[1], kDataTypeStruct, .TypeString = "SizeAxisShorthand"), // Node.VerticalSize
	DECL(0x1bd13562, Node, Height, Size.Axis[1].Requested, kDataTypeFloat), // Node.Height
	DECL(0xf5943e12, Node, DesiredHeight, Size.Axis[1].Desired, kDataTypeFloat), // Node.DesiredHeight
	DECL(0x9f6bc248, Node, MinHeight, Size.Axis[1].Min, kDataTypeFloat), // Node.MinHeight
	DECL(0x34c40b28, Node, ActualHeight, Size.Axis[1].Actual, kDataTypeFloat), // Node.ActualHeight
	DECL(0x5320b4af, Node, ScrollHeight, Size.Axis[1].Scroll, kDataTypeFloat), // Node.ScrollHeight
	DECL(0x51724993, Node, DepthSize, Size.Axis[2], kDataTypeStruct, .TypeString = "SizeAxisShorthand"), // Node.DepthSize
	DECL(0xd070218a, Node, Depth, Size.Axis[2].Requested, kDataTypeFloat), // Node.Depth
	DECL(0xe3fa9b3a, Node, DesiredDepth, Size.Axis[2].Desired, kDataTypeFloat), // Node.DesiredDepth
	DECL(0xe01ae1b0, Node, MinDepth, Size.Axis[2].Min, kDataTypeFloat), // Node.MinDepth
	DECL(0x886c1410, Node, ActualDepth, Size.Axis[2].Actual, kDataTypeFloat), // Node.ActualDepth
	DECL(0x04d53041, Node, ScrollDepth, Size.Axis[2].Scroll, kDataTypeFloat), // Node.ScrollDepth
	DECL(0xc4cc799b, Node, Margin, Margin, kDataTypeStruct, .TypeString = "Thickness"), // Node.Margin
	DECL(0x1d32e627, Node, HorizontalMargin, Margin.Axis[0], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.HorizontalMargin
	DECL(0x0bdcf7a2, Node, MarginLeft, Margin.Axis[0].Left, kDataTypeFloat), // Node.MarginLeft
	DECL(0xe9999c8f, Node, MarginRight, Margin.Axis[0].Right, kDataTypeFloat), // Node.MarginRight
	DECL(0xb859064d, Node, VerticalMargin, Margin.Axis[1], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.VerticalMargin
	DECL(0xe0a2f632, Node, MarginTop, Margin.Axis[1].Left, kDataTypeFloat), // Node.MarginTop
	DECL(0x4231df34, Node, MarginBottom, Margin.Axis[1].Right, kDataTypeFloat), // Node.MarginBottom
	DECL(0x606034ac, Node, DepthMargin, Margin.Axis[2], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.DepthMargin
	DECL(0x8bfc919a, Node, MarginFront, Margin.Axis[2].Left, kDataTypeFloat), // Node.MarginFront
	DECL(0xfc651334, Node, MarginBack, Margin.Axis[2].Right, kDataTypeFloat), // Node.MarginBack
	DECL(0x0736dd56, Node, Padding, Padding, kDataTypeStruct, .TypeString = "Thickness"), // Node.Padding
	DECL(0x96255a02, Node, HorizontalPadding, Padding.Axis[0], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.HorizontalPadding
	DECL(0x380ec7b3, Node, PaddingLeft, Padding.Axis[0].Left, kDataTypeFloat), // Node.PaddingLeft
	DECL(0x2e56c2bc, Node, PaddingRight, Padding.Axis[0].Right, kDataTypeFloat), // Node.PaddingRight
	DECL(0x147a27e0, Node, VerticalPadding, Padding.Axis[1], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.VerticalPadding
	DECL(0x67bd6401, Node, PaddingTop, Padding.Axis[1].Left, kDataTypeFloat), // Node.PaddingTop
	DECL(0x8e7f220d, Node, PaddingBottom, Padding.Axis[1].Right, kDataTypeFloat), // Node.PaddingBottom
	DECL(0xcba5b053, Node, DepthPadding, Padding.Axis[2], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.DepthPadding
	DECL(0x3d180671, Node, PaddingFront, Padding.Axis[2].Left, kDataTypeFloat), // Node.PaddingFront
	DECL(0x8c287ea5, Node, PaddingBack, Padding.Axis[2].Right, kDataTypeFloat), // Node.PaddingBack
	DECL(0x0cad6f57, Node, Border, Border, kDataTypeStruct, .TypeString = "BorderShorthand"), // Node.Border
	DECL(0x0aaf7cf9, Node, BorderWidth, Border.Width, kDataTypeStruct, .TypeString = "Thickness"), // Node.BorderWidth
	DECL(0x19c21815, Node, HorizontalBorderWidth, Border.Width.Axis[0], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.HorizontalBorderWidth
	DECL(0x63b03bb4, Node, BorderWidthLeft, Border.Width.Axis[0].Left, kDataTypeFloat), // Node.BorderWidthLeft
	DECL(0xe6977c61, Node, BorderWidthRight, Border.Width.Axis[0].Right, kDataTypeFloat), // Node.BorderWidthRight
	DECL(0x6b40a6c7, Node, VerticalBorderWidth, Border.Width.Axis[1], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.VerticalBorderWidth
	DECL(0x54b7f688, Node, BorderWidthTop, Border.Width.Axis[1].Left, kDataTypeFloat), // Node.BorderWidthTop
	DECL(0xa44eab36, Node, BorderWidthBottom, Border.Width.Axis[1].Right, kDataTypeFloat), // Node.BorderWidthBottom
	DECL(0x3170de3c, Node, DepthBorderWidth, Border.Width.Axis[2], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.DepthBorderWidth
	DECL(0x14d0d564, Node, BorderWidthFront, Border.Width.Axis[2].Left, kDataTypeFloat), // Node.BorderWidthFront
	DECL(0x7c087736, Node, BorderWidthBack, Border.Width.Axis[2].Right, kDataTypeFloat), // Node.BorderWidthBack
	DECL(0x933e48c6, Node, BorderColor, Border.Color, kDataTypeColor), // Node.BorderColor
	DECL(0x390b4488, Node, BorderStyle, Border.Style, kDataTypeEnum, .EnumValues = _BorderStyle), // Node.BorderStyle
	DECL(0xb8e9fe05, Node, BorderRadius, Border.Radius, kDataTypeStruct, .TypeString = "CornerRadius"), // Node.BorderRadius
	DECL(0x22252041, Node, BorderTopLeftRadius, Border.Radius.TopLeftRadius, kDataTypeFloat), // Node.BorderTopLeftRadius
	DECL(0x789f0d82, Node, BorderTopRightRadius, Border.Radius.TopRightRadius, kDataTypeFloat), // Node.BorderTopRightRadius
	DECL(0xc321a1f8, Node, BorderBottomRightRadius, Border.Radius.BottomRightRadius, kDataTypeFloat), // Node.BorderBottomRightRadius
	DECL(0x700e8e07, Node, BorderBottomLeftRadius, Border.Radius.BottomLeftRadius, kDataTypeFloat), // Node.BorderBottomLeftRadius
	DECL(0xd66abafe, Node, Alignment, Alignment, kDataTypeStruct, .TypeString = "AlignmentShorthand"), // Node.Alignment
	DECL(0x1b8d5152, Node, HorizontalAlignment, Alignment.Axis[0], kDataTypeEnum, .EnumValues = _HorizontalAlignment), // Node.HorizontalAlignment
	DECL(0x94b01054, Node, VerticalAlignment, Alignment.Axis[1], kDataTypeEnum, .EnumValues = _VerticalAlignment), // Node.VerticalAlignment
	DECL(0x7ef540ff, Node, DepthAlignment, Alignment.Axis[2], kDataTypeEnum, .EnumValues = _DepthAlignment), // Node.DepthAlignment
	DECL(0x592a4941, Node, Visible, Visible, kDataTypeBool), // Node.Visible
	DECL(0x20d9ba7c, Node, QuickHide, QuickHide, kDataTypeBool), // Node.QuickHide
	DECL(0xa0b06d26, Node, VisibleAmountInParent, VisibleAmountInParent, kDataTypeFloat), // Node.VisibleAmountInParent
	DECL(0xde1f0406, Node, Opacity, Opacity, kDataTypeFloat), // Node.Opacity
	DECL(0x76bda0c0, Node, Tags, Tags, kDataTypeString), // Node.Tags
	DECL(0xa310331c, Node, DataContext, DataContext, kDataTypeObject, .TypeString = "DataObject"), // Node.DataContext
	ARRAY_DECL(0x9564a892, Node, Resources, Resources, kDataTypeStruct, .TypeString = "ResourceEntry"), // Node.Resources
	DECL(0x25139ae4, Node, NumResources, NumResources, kDataTypeInt), // Node.NumResources
	DECL(0x5dbe404d, Node, UpdateMatrix, UpdateMatrix, kDataTypeEvent, .TypeString = "Node_UpdateMatrixEventArgs"), // Node.UpdateMatrix
	DECL(0xa3650e54, Node, LoadView, LoadView, kDataTypeEvent, .TypeString = "Node_LoadViewEventArgs"), // Node.LoadView
	DECL(0x898160ea, Node, HitTest, HitTest, kDataTypeEvent, .TypeString = "Node_HitTestEventArgs"), // Node.HitTest
	DECL(0x608d20d1, Node, IsVisible, IsVisible, kDataTypeEvent, .TypeString = "Node_IsVisibleEventArgs"), // Node.IsVisible
	DECL(0x03e93095, Node, ViewDidLoad, ViewDidLoad, kDataTypeEvent, .TypeString = "Node_ViewDidLoadEventArgs"), // Node.ViewDidLoad
	DECL(0xa7c0f8d7, Node, KillFocus, KillFocus, kDataTypeEvent, .TypeString = "Node_KillFocusEventArgs"), // Node.KillFocus
	DECL(0xc399d265, Node, SetFocus, SetFocus, kDataTypeEvent, .TypeString = "Node_SetFocusEventArgs"), // Node.SetFocus
	DECL(0x80d9e0ee, Node, GetSize, GetSize, kDataTypeEvent, .TypeString = "Node_GetSizeEventArgs"), // Node.GetSize
	DECL(0x0d589383, Node, MouseMessage, MouseMessage, kDataTypeEvent, .TypeString = "Node_MouseMessageEventArgs"), // Node.MouseMessage
	DECL(0x6cc782b0, Node, LeftButtonDown, LeftButtonDown, kDataTypeEvent, .TypeString = "Node_LeftButtonDownEventArgs"), // Node.LeftButtonDown
	DECL(0x13a69401, Node, RightButtonDown, RightButtonDown, kDataTypeEvent, .TypeString = "Node_RightButtonDownEventArgs"), // Node.RightButtonDown
	DECL(0x39ed21d1, Node, OtherButtonDown, OtherButtonDown, kDataTypeEvent, .TypeString = "Node_OtherButtonDownEventArgs"), // Node.OtherButtonDown
	DECL(0x0bb30ac9, Node, LeftButtonUp, LeftButtonUp, kDataTypeEvent, .TypeString = "Node_LeftButtonUpEventArgs"), // Node.LeftButtonUp
	DECL(0x05e78e74, Node, RightButtonUp, RightButtonUp, kDataTypeEvent, .TypeString = "Node_RightButtonUpEventArgs"), // Node.RightButtonUp
	DECL(0xc86953a4, Node, OtherButtonUp, OtherButtonUp, kDataTypeEvent, .TypeString = "Node_OtherButtonUpEventArgs"), // Node.OtherButtonUp
	DECL(0x4884ec22, Node, LeftButtonDragged, LeftButtonDragged, kDataTypeEvent, .TypeString = "Node_LeftButtonDraggedEventArgs"), // Node.LeftButtonDragged
	DECL(0x651f7219, Node, RightButtonDragged, RightButtonDragged, kDataTypeEvent, .TypeString = "Node_RightButtonDraggedEventArgs"), // Node.RightButtonDragged
	DECL(0xf4cda889, Node, OtherButtonDragged, OtherButtonDragged, kDataTypeEvent, .TypeString = "Node_OtherButtonDraggedEventArgs"), // Node.OtherButtonDragged
	DECL(0x5a92bc67, Node, LeftDoubleClick, LeftDoubleClick, kDataTypeEvent, .TypeString = "Node_LeftDoubleClickEventArgs"), // Node.LeftDoubleClick
	DECL(0xeeebbe60, Node, RightDoubleClick, RightDoubleClick, kDataTypeEvent, .TypeString = "Node_RightDoubleClickEventArgs"), // Node.RightDoubleClick
	DECL(0xf6c60630, Node, OtherDoubleClick, OtherDoubleClick, kDataTypeEvent, .TypeString = "Node_OtherDoubleClickEventArgs"), // Node.OtherDoubleClick
	DECL(0x65db8b6f, Node, MouseMoved, MouseMoved, kDataTypeEvent, .TypeString = "Node_MouseMovedEventArgs"), // Node.MouseMoved
	DECL(0x626f90e3, Node, ScrollWheel, ScrollWheel, kDataTypeEvent, .TypeString = "Node_ScrollWheelEventArgs"), // Node.ScrollWheel
	DECL(0x25989e7a, Node, DragDrop, DragDrop, kDataTypeEvent, .TypeString = "Node_DragDropEventArgs"), // Node.DragDrop
	DECL(0xc0e97a77, Node, DragEnter, DragEnter, kDataTypeEvent, .TypeString = "Node_DragEnterEventArgs"), // Node.DragEnter
	DECL(0xee4b252b, Node, KeyMessage, KeyMessage, kDataTypeEvent, .TypeString = "Node_KeyMessageEventArgs"), // Node.KeyMessage
	DECL(0x83b19b78, Node, KeyDown, KeyDown, kDataTypeEvent, .TypeString = "Node_KeyDownEventArgs"), // Node.KeyDown
	DECL(0xfca37d71, Node, KeyUp, KeyUp, kDataTypeEvent, .TypeString = "Node_KeyUpEventArgs"), // Node.KeyUp
	DECL(0xd484cca2, Node, TextInput, TextInput, kDataTypeEvent, .TypeString = "Node_TextInputEventArgs"), // Node.TextInput
};
static struct Node NodeDefaults = {
		
  .Visible = TRUE,
		
  .VisibleAmountInParent = 1,
		
  .Opacity = 1,
};
LRESULT NodeProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node_GetSize: return Node_GetSize(object, cmp, wparm, lparm); // Node.GetSize
		case ID_Node_IsVisible: return Node_IsVisible(object, cmp, wparm, lparm); // Node.IsVisible
		case ID_Object_Start: return Node_Start(object, cmp, wparm, lparm); // Object.Start
	}
	return FALSE;
}
void luaX_pushNode(lua_State *L, struct Node const* Node) {
	luaX_pushObject(L, CMP_GetObject(Node));
}
struct Node* luaX_checkNode(lua_State *L, int idx) {
	return GetNode(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Node, 0);
int f_core_GetFocus(lua_State *L) {
	struct Object* result_ = core_GetFocus();
	luaX_pushObject(L, result_);
	return 1;
}
int f_core_GetHover(lua_State *L) {
	struct Object* result_ = core_GetHover();
	luaX_pushObject(L, result_);
	return 1;
}
int f_core_AddGlobalStyleRule(lua_State *L) {
	struct Object* rule = luaX_checkObject(L, 1);
	core_AddGlobalStyleRule(L, rule );
	return 0;
}
int f_core_AdvanceFrame(lua_State *L) {
	core_AdvanceFrame( );
	return 0;
}

ORCA_API int luaopen_orca_core(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ "getFocus", f_core_GetFocus },
		{ "getHover", f_core_GetHover },
		{ "addGlobalStyleRule", f_core_AddGlobalStyleRule },
		{ "advanceFrame", f_core_AdvanceFrame },
		{ NULL, NULL } 
	}));
	void before_core_module_registered(lua_State *L);
	before_core_module_registered(L);
	lua_setfield(L, ((void)luaopen_orca_AnimationClipReference(L), -2), "AnimationClipReference");
	lua_setfield(L, ((void)luaopen_orca_Keyframe(L), -2), "Keyframe");
	lua_setfield(L, ((void)luaopen_orca_LocaleEntry(L), -2), "LocaleEntry");
	lua_setfield(L, ((void)luaopen_orca_ResourceEntry(L), -2), "ResourceEntry");
	lua_setfield(L, ((void)luaopen_orca_CornerRadius(L), -2), "CornerRadius");
	lua_setfield(L, ((void)luaopen_orca_EdgeShorthand(L), -2), "EdgeShorthand");
	lua_setfield(L, ((void)luaopen_orca_AlignmentShorthand(L), -2), "AlignmentShorthand");
	lua_setfield(L, ((void)luaopen_orca_Thickness(L), -2), "Thickness");
	lua_setfield(L, ((void)luaopen_orca_BorderShorthand(L), -2), "BorderShorthand");
	lua_setfield(L, ((void)luaopen_orca_SizeAxisShorthand(L), -2), "SizeAxisShorthand");
	lua_setfield(L, ((void)luaopen_orca_SizeShorthand(L), -2), "SizeShorthand");
	lua_setfield(L, ((void)luaopen_orca_Object_CreateEventArgs(L), -2), "Object_CreateEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_StartEventArgs(L), -2), "Object_StartEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_AnimateEventArgs(L), -2), "Object_AnimateEventArgs");
	lua_setfield(L, ((void)luaopen_orca_StyleController_ThemeChangedEventArgs(L), -2), "StyleController_ThemeChangedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_PropertyChangedEventArgs(L), -2), "Object_PropertyChangedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_AttachedEventArgs(L), -2), "Object_AttachedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_ReleaseEventArgs(L), -2), "Object_ReleaseEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_DestroyEventArgs(L), -2), "Object_DestroyEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_TimerEventArgs(L), -2), "Object_TimerEventArgs");
	lua_setfield(L, ((void)luaopen_orca_AnimationPlayer_PlayEventArgs(L), -2), "AnimationPlayer_PlayEventArgs");
	lua_setfield(L, ((void)luaopen_orca_AnimationPlayer_ResumeEventArgs(L), -2), "AnimationPlayer_ResumeEventArgs");
	lua_setfield(L, ((void)luaopen_orca_AnimationPlayer_StopEventArgs(L), -2), "AnimationPlayer_StopEventArgs");
	lua_setfield(L, ((void)luaopen_orca_AnimationPlayer_PauseEventArgs(L), -2), "AnimationPlayer_PauseEventArgs");
	lua_setfield(L, ((void)luaopen_orca_AnimationPlayer_StartedEventArgs(L), -2), "AnimationPlayer_StartedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_AnimationPlayer_StoppedEventArgs(L), -2), "AnimationPlayer_StoppedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_AnimationPlayer_CompletedEventArgs(L), -2), "AnimationPlayer_CompletedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_StyleController_AddClassEventArgs(L), -2), "StyleController_AddClassEventArgs");
	lua_setfield(L, ((void)luaopen_orca_StyleController_AddClassesEventArgs(L), -2), "StyleController_AddClassesEventArgs");
	lua_setfield(L, ((void)luaopen_orca_StateManagerController_ControllerChangedEventArgs(L), -2), "StateManagerController_ControllerChangedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Trigger_TriggeredEventArgs(L), -2), "Trigger_TriggeredEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_UpdateMatrixEventArgs(L), -2), "Node_UpdateMatrixEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LoadViewEventArgs(L), -2), "Node_LoadViewEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_HitTestEventArgs(L), -2), "Node_HitTestEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_IsVisibleEventArgs(L), -2), "Node_IsVisibleEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_ViewDidLoadEventArgs(L), -2), "Node_ViewDidLoadEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_KillFocusEventArgs(L), -2), "Node_KillFocusEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_SetFocusEventArgs(L), -2), "Node_SetFocusEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_GetSizeEventArgs(L), -2), "Node_GetSizeEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_MouseMessageEventArgs(L), -2), "Node_MouseMessageEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LeftButtonDownEventArgs(L), -2), "Node_LeftButtonDownEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_RightButtonDownEventArgs(L), -2), "Node_RightButtonDownEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_OtherButtonDownEventArgs(L), -2), "Node_OtherButtonDownEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LeftButtonUpEventArgs(L), -2), "Node_LeftButtonUpEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_RightButtonUpEventArgs(L), -2), "Node_RightButtonUpEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_OtherButtonUpEventArgs(L), -2), "Node_OtherButtonUpEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LeftButtonDraggedEventArgs(L), -2), "Node_LeftButtonDraggedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_RightButtonDraggedEventArgs(L), -2), "Node_RightButtonDraggedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_OtherButtonDraggedEventArgs(L), -2), "Node_OtherButtonDraggedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LeftDoubleClickEventArgs(L), -2), "Node_LeftDoubleClickEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_RightDoubleClickEventArgs(L), -2), "Node_RightDoubleClickEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_OtherDoubleClickEventArgs(L), -2), "Node_OtherDoubleClickEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_MouseMovedEventArgs(L), -2), "Node_MouseMovedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_ScrollWheelEventArgs(L), -2), "Node_ScrollWheelEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_DragDropEventArgs(L), -2), "Node_DragDropEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_DragEnterEventArgs(L), -2), "Node_DragEnterEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_KeyMessageEventArgs(L), -2), "Node_KeyMessageEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_KeyDownEventArgs(L), -2), "Node_KeyDownEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_KeyUpEventArgs(L), -2), "Node_KeyUpEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_TextInputEventArgs(L), -2), "Node_TextInputEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Object(L), -2), "Object");
	lua_setfield(L, ((void)lua_pushclass(L, &_AnimationCurve), -2), "AnimationCurve");
	lua_setfield(L, ((void)lua_pushclass(L, &_AnimationClip), -2), "AnimationClip");
	lua_setfield(L, ((void)lua_pushclass(L, &_AnimationPlayer), -2), "AnimationPlayer");
	lua_setfield(L, ((void)lua_pushclass(L, &_PropertyAnimation), -2), "PropertyAnimation");
	lua_setfield(L, ((void)lua_pushclass(L, &_StyleSheet), -2), "StyleSheet");
	lua_setfield(L, ((void)lua_pushclass(L, &_StyleRule), -2), "StyleRule");
	lua_setfield(L, ((void)lua_pushclass(L, &_StyleController), -2), "StyleController");
	lua_setfield(L, ((void)lua_pushclass(L, &_StateManager), -2), "StateManager");
	lua_setfield(L, ((void)lua_pushclass(L, &_StateGroup), -2), "StateGroup");
	lua_setfield(L, ((void)lua_pushclass(L, &_State), -2), "State");
	lua_setfield(L, ((void)lua_pushclass(L, &_StateManagerController), -2), "StateManagerController");
	lua_setfield(L, ((void)lua_pushclass(L, &_ResourceDictionary), -2), "ResourceDictionary");
	lua_setfield(L, ((void)lua_pushclass(L, &_Locale), -2), "Locale");
	lua_setfield(L, ((void)lua_pushclass(L, &_DataObject), -2), "DataObject");
	lua_setfield(L, ((void)lua_pushclass(L, &_Trigger), -2), "Trigger");
	lua_setfield(L, ((void)lua_pushclass(L, &_OnPropertyChangedTrigger), -2), "OnPropertyChangedTrigger");
	lua_setfield(L, ((void)lua_pushclass(L, &_OnAttachedTrigger), -2), "OnAttachedTrigger");
	lua_setfield(L, ((void)lua_pushclass(L, &_EventTrigger), -2), "EventTrigger");
	lua_setfield(L, ((void)lua_pushclass(L, &_Setter), -2), "Setter");
	lua_setfield(L, ((void)lua_pushclass(L, &_Handler), -2), "Handler");
	lua_setfield(L, ((void)lua_pushclass(L, &_Node), -2), "Node");
	void after_core_module_registered(lua_State *L);
	after_core_module_registered(L);
	return 1;
}
