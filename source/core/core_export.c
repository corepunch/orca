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
extern int64_t OBJ_GetObjectCount(void);

ENUM(MessageRouting, "Bubbling", "TunnelingBubbling", "Tunneling", "Direct")
ENUM(PropertyState, "Normal", "Hover", "Focus", "Select", "Disable")
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
int f_OBJ_Clear(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_Clear(this_ );
	return 0;
}
int f_OBJ_ReleaseOrphan(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_ReleaseRef(this_ );
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
	OBJ_Rebuild(L, this_ );
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
	OBJ_RemoveFromParent(this_ );
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
int f_OBJ_IsPrefabView(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	bool_t result_ = OBJ_IsPrefabView(this_);
	lua_pushboolean(L, result_);
	return 1;
}

int luaopen_orca_Object(lua_State *L) {
	luaL_newmetatable(L, "Object");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_OBJ_CreateFromLuaState },
		{ "clear", f_OBJ_Clear },
		{ "__gc", f_OBJ_ReleaseOrphan },
		{ "__eq", f_OBJ_Equals },
		{ "rebuild", f_OBJ_Rebuild },
		{ "addChild", f_OBJ_AddChild },
		{ "__add", f_OBJ_AddChild },
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
		{ "__setcontext", f_OBJ_SetContext },
		{ "instantiate", f_OBJ_Instantiate },
		{ "isPrefabView", f_OBJ_IsPrefabView },
		{ NULL, NULL },
	}), 0);
	lua_getfield(L, -1, "__index");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	} else {
		lua_pop(L, 1);
	}
	return 1;
}
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);
static struct PropertyType _AnimationClipReference[] = {
	DECL_STRING(0x0fe07306, AnimationClipReference, Name, Name), // AnimationClipReference.Name
	DECL_OBJECT(0xd33ddb1b, AnimationClipReference, Clip, Clip, AnimationClip), // AnimationClipReference.Clip
};
static luaL_Reg _AnimationClipReference_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _Keyframe[] = {
	DECL_FLOAT(0xdfe4e404, Keyframe, Time, time), // Keyframe.Time
	DECL_STRUCT_TYPE(0xd147f96a, Keyframe, Value, value, Vector4D), // Keyframe.Value
	DECL_STRUCT_TYPE(0xbe5410b1, Keyframe, InSlope, inSlope, Vector4D), // Keyframe.InSlope
	DECL_STRUCT_TYPE(0xc3a6b78e, Keyframe, OutSlope, outSlope, Vector4D), // Keyframe.OutSlope
	DECL_STRUCT_TYPE(0xfd369606, Keyframe, InWeight, inWeight, Vector4D), // Keyframe.InWeight
	DECL_STRUCT_TYPE(0xdac7b09b, Keyframe, OutWeight, outWeight, Vector4D), // Keyframe.OutWeight
	DECL_INT(0x153369e9, Keyframe, TangentMode, tangentMode), // Keyframe.TangentMode
	DECL_INT(0xf0555d63, Keyframe, WeightedMode, weightedMode), // Keyframe.WeightedMode
};
static luaL_Reg _Keyframe_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _LocaleEntry[] = {
	DECL_STRING(0xcd1ac90c, LocaleEntry, Key, Key), // LocaleEntry.Key
	DECL_STRING(0xd147f96a, LocaleEntry, Value, Value), // LocaleEntry.Value
	DECL_ENUM(0xd155d06d, LocaleEntry, Type, Type, LocaleEntryType), // LocaleEntry.Type
};
static luaL_Reg _LocaleEntry_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _ResourceEntry[] = {
	DECL_STRING(0xcd1ac90c, ResourceEntry, Key, Key), // ResourceEntry.Key
	DECL_STRING(0xd147f96a, ResourceEntry, Value, Value), // ResourceEntry.Value
	DECL_ENUM(0xd155d06d, ResourceEntry, Type, Type, ResourceEntryType), // ResourceEntry.Type
};
static luaL_Reg _ResourceEntry_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _CornerRadius[] = {
	DECL_FLOAT(0xd5ac3a0b, CornerRadius, TopLeftRadius, TopLeftRadius), // CornerRadius.TopLeftRadius
	DECL_FLOAT(0xdbe5a724, CornerRadius, TopRightRadius, TopRightRadius), // CornerRadius.TopRightRadius
	DECL_FLOAT(0xf6ae40ce, CornerRadius, BottomRightRadius, BottomRightRadius), // CornerRadius.BottomRightRadius
	DECL_FLOAT(0x7f5fe235, CornerRadius, BottomLeftRadius, BottomLeftRadius), // CornerRadius.BottomLeftRadius
};
static luaL_Reg _CornerRadius_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _EdgeShorthand[] = {
	DECL_FLOAT(0x92773890, EdgeShorthand, Left, Left), // EdgeShorthand.Left
	DECL_FLOAT(0x1e9e9f85, EdgeShorthand, Right, Right), // EdgeShorthand.Right
};
static luaL_Reg _EdgeShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _AlignmentShorthand[] = {
	DECL_INT(0x6bf39489, AlignmentShorthand, Horizontal, Axis[0]), // AlignmentShorthand.Horizontal
	DECL_INT(0x80fb2d5b, AlignmentShorthand, Vertical, Axis[1]), // AlignmentShorthand.Vertical
	DECL_INT(0xd070218a, AlignmentShorthand, Depth, Axis[2]), // AlignmentShorthand.Depth
};
static luaL_Reg _AlignmentShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _Thickness[] = {
	DECL_STRUCT_TYPE(0x6bf39489, Thickness, Horizontal, Axis[0], EdgeShorthand), // Thickness.Horizontal
	DECL_FLOAT(0x92773890, Thickness, Left, Axis[0].Left), // Thickness.Left
	DECL_FLOAT(0x1e9e9f85, Thickness, Right, Axis[0].Right), // Thickness.Right
	DECL_STRUCT_TYPE(0x80fb2d5b, Thickness, Vertical, Axis[1], EdgeShorthand), // Thickness.Vertical
	DECL_FLOAT(0x099b73dc, Thickness, Top, Axis[1].Left), // Thickness.Top
	DECL_FLOAT(0x22b5f34a, Thickness, Bottom, Axis[1].Right), // Thickness.Bottom
	DECL_STRUCT_TYPE(0xd070218a, Thickness, Depth, Axis[2], EdgeShorthand), // Thickness.Depth
	DECL_FLOAT(0x6de89878, Thickness, Front, Axis[2].Left), // Thickness.Front
	DECL_FLOAT(0xc2954bc2, Thickness, Back, Axis[2].Right), // Thickness.Back
};
static luaL_Reg _Thickness_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _BorderShorthand[] = {
	DECL_STRUCT_TYPE(0x3b42dfbf, BorderShorthand, Width, Width, Thickness), // BorderShorthand.Width
	DECL_STRUCT_TYPE(0x9bd0d683, BorderShorthand, HorizontalWidth, Width.Axis[0], EdgeShorthand), // BorderShorthand.HorizontalWidth
	DECL_FLOAT(0x0671d536, BorderShorthand, WidthLeft, Width.Axis[0].Left), // BorderShorthand.WidthLeft
	DECL_FLOAT(0x30532a6b, BorderShorthand, WidthRight, Width.Axis[0].Right), // BorderShorthand.WidthRight
	DECL_STRUCT_TYPE(0xa1c213dd, BorderShorthand, VerticalWidth, Width.Axis[1], EdgeShorthand), // BorderShorthand.VerticalWidth
	DECL_FLOAT(0xf37d53fe, BorderShorthand, WidthTop, Width.Axis[1].Left), // BorderShorthand.WidthTop
	DECL_FLOAT(0xddffd3a0, BorderShorthand, WidthBottom, Width.Axis[1].Right), // BorderShorthand.WidthBottom
	DECL_STRUCT_TYPE(0x67356c56, BorderShorthand, DepthWidth, Width.Axis[2], EdgeShorthand), // BorderShorthand.DepthWidth
	DECL_FLOAT(0x03261636, BorderShorthand, WidthFront, Width.Axis[2].Left), // BorderShorthand.WidthFront
	DECL_FLOAT(0x16fd78d8, BorderShorthand, WidthBack, Width.Axis[2].Right), // BorderShorthand.WidthBack
	DECL_COLOR(0xe5b43cf8, BorderShorthand, Color, Color), // BorderShorthand.Color
	DECL_ENUM(0x5467ec76, BorderShorthand, Style, Style, BorderStyle), // BorderShorthand.Style
	DECL_STRUCT_TYPE(0x3a8111d3, BorderShorthand, Radius, Radius, CornerRadius), // BorderShorthand.Radius
	DECL_FLOAT(0x1c0b6355, BorderShorthand, RadiusTopLeftRadius, Radius.TopLeftRadius), // BorderShorthand.RadiusTopLeftRadius
	DECL_FLOAT(0xe0a577ee, BorderShorthand, RadiusTopRightRadius, Radius.TopRightRadius), // BorderShorthand.RadiusTopRightRadius
	DECL_FLOAT(0x494aff04, BorderShorthand, RadiusBottomRightRadius, Radius.BottomRightRadius), // BorderShorthand.RadiusBottomRightRadius
	DECL_FLOAT(0xfe16d36b, BorderShorthand, RadiusBottomLeftRadius, Radius.BottomLeftRadius), // BorderShorthand.RadiusBottomLeftRadius
};
static luaL_Reg _BorderShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _SizeAxisShorthand[] = {
	DECL_FLOAT(0x77ea8663, SizeAxisShorthand, Requested, Requested), // SizeAxisShorthand.Requested
	DECL_FLOAT(0x28adf5d5, SizeAxisShorthand, Desired, Desired), // SizeAxisShorthand.Desired
	DECL_FLOAT(0x2e9445f7, SizeAxisShorthand, Min, Min), // SizeAxisShorthand.Min
	DECL_FLOAT(0xf1aed197, SizeAxisShorthand, Actual, Actual), // SizeAxisShorthand.Actual
	DECL_FLOAT(0x25dca54c, SizeAxisShorthand, Scroll, Scroll), // SizeAxisShorthand.Scroll
};
static luaL_Reg _SizeAxisShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _SizeShorthand[] = {
	DECL_STRUCT_TYPE(0x6bf39489, SizeShorthand, Horizontal, Axis[0], SizeAxisShorthand), // SizeShorthand.Horizontal
	DECL_FLOAT(0x9463344f, SizeShorthand, HorizontalRequested, Axis[0].Requested), // SizeShorthand.HorizontalRequested
	DECL_FLOAT(0x1c7b4471, SizeShorthand, HorizontalDesired, Axis[0].Desired), // SizeShorthand.HorizontalDesired
	DECL_FLOAT(0xda5582c3, SizeShorthand, HorizontalMin, Axis[0].Min), // SizeShorthand.HorizontalMin
	DECL_FLOAT(0x42284603, SizeShorthand, HorizontalActual, Axis[0].Actual), // SizeShorthand.HorizontalActual
	DECL_FLOAT(0x8c65db48, SizeShorthand, HorizontalScroll, Axis[0].Scroll), // SizeShorthand.HorizontalScroll
	DECL_STRUCT_TYPE(0x80fb2d5b, SizeShorthand, Vertical, Axis[1], SizeAxisShorthand), // SizeShorthand.Vertical
	DECL_FLOAT(0x4d64cad5, SizeShorthand, VerticalRequested, Axis[1].Requested), // SizeShorthand.VerticalRequested
	DECL_FLOAT(0x9b2653af, SizeShorthand, VerticalDesired, Axis[1].Desired), // SizeShorthand.VerticalDesired
	DECL_FLOAT(0xc446df6d, SizeShorthand, VerticalMin, Axis[1].Min), // SizeShorthand.VerticalMin
	DECL_FLOAT(0x2a77ca09, SizeShorthand, VerticalActual, Axis[1].Actual), // SizeShorthand.VerticalActual
	DECL_FLOAT(0xe04d52a2, SizeShorthand, VerticalScroll, Axis[1].Scroll), // SizeShorthand.VerticalScroll
	DECL_STRUCT_TYPE(0xd070218a, SizeShorthand, Depth, Axis[2], SizeAxisShorthand), // SizeShorthand.Depth
	DECL_FLOAT(0xe455f006, SizeShorthand, DepthRequested, Axis[2].Requested), // SizeShorthand.DepthRequested
	DECL_FLOAT(0x5c288608, SizeShorthand, DepthDesired, Axis[2].Desired), // SizeShorthand.DepthDesired
	DECL_FLOAT(0x3303ed1e, SizeShorthand, DepthMin, Axis[2].Min), // SizeShorthand.DepthMin
	DECL_FLOAT(0xf04276dc, SizeShorthand, DepthActual, Axis[2].Actual), // SizeShorthand.DepthActual
	DECL_FLOAT(0xd8e0c913, SizeShorthand, DepthScroll, Axis[2].Scroll), // SizeShorthand.DepthScroll
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
static luaL_Reg _Object_ThemeChangedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_ThemeChangedEventArgs[] = {
	DECL_BOOL(0x5bee3c77, Object_ThemeChangedEventArgs, recursive, recursive), // Object_ThemeChangedEventArgs.recursive
};
static luaL_Reg _Object_PropertyChangedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_PropertyChangedEventArgs[] = {
	DECL_STRUCT_TYPE(0x5221f9e8, Object_PropertyChangedEventArgs, Property, Property, Property), // Object_PropertyChangedEventArgs.Property
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
	DECL_INT(0x4903ce0f, Object_TimerEventArgs, timerId, timerId), // Object_TimerEventArgs.timerId
};
static luaL_Reg _AnimationPlayer_PlayEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _AnimationPlayer_PlayEventArgs[] = {
	DECL_STRING(0x0fe07306, AnimationPlayer_PlayEventArgs, Name, Name), // AnimationPlayer_PlayEventArgs.Name
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
static luaL_Reg _StyleController_ThemeChangedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _StyleController_ThemeChangedEventArgs[] = {
	DECL_BOOL(0x5bee3c77, StyleController_ThemeChangedEventArgs, recursive, recursive), // StyleController_ThemeChangedEventArgs.recursive
};
static luaL_Reg _StyleController_AddClassEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _StyleController_AddClassEventArgs[] = {
	DECL_STRING(0x2ecda0bc, StyleController_AddClassEventArgs, ClassName, ClassName), // StyleController_AddClassEventArgs.ClassName
};
static luaL_Reg _StyleController_AddClassesEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _StyleController_AddClassesEventArgs[] = {
	DECL_STRING(0x7cb425dd, StyleController_AddClassesEventArgs, ClassNames, ClassNames), // StyleController_AddClassesEventArgs.ClassNames
};
static luaL_Reg _StateManagerController_ControllerChangedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _StateManagerController_ControllerChangedEventArgs[] = {
	DECL_STRUCT_TYPE(0x5221f9e8, StateManagerController_ControllerChangedEventArgs, Property, Property, Property), // StateManagerController_ControllerChangedEventArgs.Property
};
static luaL_Reg _Trigger_TriggeredEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Trigger_TriggeredEventArgs[] = {
	DECL_OBJECT(0xa5ea0da3, Trigger_TriggeredEventArgs, Trigger, Trigger, Trigger), // Trigger_TriggeredEventArgs.Trigger
};
static luaL_Reg _Node_UpdateMatrixEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_UpdateMatrixEventArgs[] = {
	DECL_STRUCT_TYPE(0xeacdfcfd, Node_UpdateMatrixEventArgs, parent, parent, Matrix3D), // Node_UpdateMatrixEventArgs.parent
	DECL_FLOAT(0xc6c2dd66, Node_UpdateMatrixEventArgs, opacity, opacity), // Node_UpdateMatrixEventArgs.opacity
	DECL_BOOL(0x79a98884, Node_UpdateMatrixEventArgs, force, force), // Node_UpdateMatrixEventArgs.force
};
static luaL_Reg _Node_LoadViewEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LoadViewEventArgs[] = {
	DECL_STRUCT_TYPE(0x187f5b0f, Node_LoadViewEventArgs, lua_state, lua_state, lua_State), // Node_LoadViewEventArgs.lua_state
};
static luaL_Reg _Node_HitTestEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_HitTestEventArgs[] = {
	DECL_INT(0xfd0c5087, Node_HitTestEventArgs, x, x), // Node_HitTestEventArgs.x
	DECL_INT(0xfc0c4ef4, Node_HitTestEventArgs, y, y), // Node_HitTestEventArgs.y
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
	DECL_FLOAT(0xfd0c5087, Node_MouseMessageEventArgs, x, x), // Node_MouseMessageEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_MouseMessageEventArgs, y, y), // Node_MouseMessageEventArgs.y
	DECL_INT(0xec56f27b, Node_MouseMessageEventArgs, deltaX, deltaX), // Node_MouseMessageEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_MouseMessageEventArgs, deltaY, deltaY), // Node_MouseMessageEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_MouseMessageEventArgs, button, button, MouseButton), // Node_MouseMessageEventArgs.button
	DECL_INT(0xd9747336, Node_MouseMessageEventArgs, clickCount, clickCount), // Node_MouseMessageEventArgs.clickCount
};
static luaL_Reg _Node_LeftButtonDownEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftButtonDownEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_LeftButtonDownEventArgs, x, x), // Node_LeftButtonDownEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_LeftButtonDownEventArgs, y, y), // Node_LeftButtonDownEventArgs.y
	DECL_INT(0xec56f27b, Node_LeftButtonDownEventArgs, deltaX, deltaX), // Node_LeftButtonDownEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_LeftButtonDownEventArgs, deltaY, deltaY), // Node_LeftButtonDownEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_LeftButtonDownEventArgs, button, button, MouseButton), // Node_LeftButtonDownEventArgs.button
	DECL_INT(0xd9747336, Node_LeftButtonDownEventArgs, clickCount, clickCount), // Node_LeftButtonDownEventArgs.clickCount
};
static luaL_Reg _Node_RightButtonDownEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightButtonDownEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_RightButtonDownEventArgs, x, x), // Node_RightButtonDownEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_RightButtonDownEventArgs, y, y), // Node_RightButtonDownEventArgs.y
	DECL_INT(0xec56f27b, Node_RightButtonDownEventArgs, deltaX, deltaX), // Node_RightButtonDownEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_RightButtonDownEventArgs, deltaY, deltaY), // Node_RightButtonDownEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_RightButtonDownEventArgs, button, button, MouseButton), // Node_RightButtonDownEventArgs.button
	DECL_INT(0xd9747336, Node_RightButtonDownEventArgs, clickCount, clickCount), // Node_RightButtonDownEventArgs.clickCount
};
static luaL_Reg _Node_OtherButtonDownEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherButtonDownEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_OtherButtonDownEventArgs, x, x), // Node_OtherButtonDownEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_OtherButtonDownEventArgs, y, y), // Node_OtherButtonDownEventArgs.y
	DECL_INT(0xec56f27b, Node_OtherButtonDownEventArgs, deltaX, deltaX), // Node_OtherButtonDownEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_OtherButtonDownEventArgs, deltaY, deltaY), // Node_OtherButtonDownEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_OtherButtonDownEventArgs, button, button, MouseButton), // Node_OtherButtonDownEventArgs.button
	DECL_INT(0xd9747336, Node_OtherButtonDownEventArgs, clickCount, clickCount), // Node_OtherButtonDownEventArgs.clickCount
};
static luaL_Reg _Node_LeftButtonUpEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftButtonUpEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_LeftButtonUpEventArgs, x, x), // Node_LeftButtonUpEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_LeftButtonUpEventArgs, y, y), // Node_LeftButtonUpEventArgs.y
	DECL_INT(0xec56f27b, Node_LeftButtonUpEventArgs, deltaX, deltaX), // Node_LeftButtonUpEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_LeftButtonUpEventArgs, deltaY, deltaY), // Node_LeftButtonUpEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_LeftButtonUpEventArgs, button, button, MouseButton), // Node_LeftButtonUpEventArgs.button
	DECL_INT(0xd9747336, Node_LeftButtonUpEventArgs, clickCount, clickCount), // Node_LeftButtonUpEventArgs.clickCount
};
static luaL_Reg _Node_RightButtonUpEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightButtonUpEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_RightButtonUpEventArgs, x, x), // Node_RightButtonUpEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_RightButtonUpEventArgs, y, y), // Node_RightButtonUpEventArgs.y
	DECL_INT(0xec56f27b, Node_RightButtonUpEventArgs, deltaX, deltaX), // Node_RightButtonUpEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_RightButtonUpEventArgs, deltaY, deltaY), // Node_RightButtonUpEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_RightButtonUpEventArgs, button, button, MouseButton), // Node_RightButtonUpEventArgs.button
	DECL_INT(0xd9747336, Node_RightButtonUpEventArgs, clickCount, clickCount), // Node_RightButtonUpEventArgs.clickCount
};
static luaL_Reg _Node_OtherButtonUpEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherButtonUpEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_OtherButtonUpEventArgs, x, x), // Node_OtherButtonUpEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_OtherButtonUpEventArgs, y, y), // Node_OtherButtonUpEventArgs.y
	DECL_INT(0xec56f27b, Node_OtherButtonUpEventArgs, deltaX, deltaX), // Node_OtherButtonUpEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_OtherButtonUpEventArgs, deltaY, deltaY), // Node_OtherButtonUpEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_OtherButtonUpEventArgs, button, button, MouseButton), // Node_OtherButtonUpEventArgs.button
	DECL_INT(0xd9747336, Node_OtherButtonUpEventArgs, clickCount, clickCount), // Node_OtherButtonUpEventArgs.clickCount
};
static luaL_Reg _Node_LeftButtonDraggedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftButtonDraggedEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_LeftButtonDraggedEventArgs, x, x), // Node_LeftButtonDraggedEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_LeftButtonDraggedEventArgs, y, y), // Node_LeftButtonDraggedEventArgs.y
	DECL_INT(0xec56f27b, Node_LeftButtonDraggedEventArgs, deltaX, deltaX), // Node_LeftButtonDraggedEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_LeftButtonDraggedEventArgs, deltaY, deltaY), // Node_LeftButtonDraggedEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_LeftButtonDraggedEventArgs, button, button, MouseButton), // Node_LeftButtonDraggedEventArgs.button
	DECL_INT(0xd9747336, Node_LeftButtonDraggedEventArgs, clickCount, clickCount), // Node_LeftButtonDraggedEventArgs.clickCount
};
static luaL_Reg _Node_RightButtonDraggedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightButtonDraggedEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_RightButtonDraggedEventArgs, x, x), // Node_RightButtonDraggedEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_RightButtonDraggedEventArgs, y, y), // Node_RightButtonDraggedEventArgs.y
	DECL_INT(0xec56f27b, Node_RightButtonDraggedEventArgs, deltaX, deltaX), // Node_RightButtonDraggedEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_RightButtonDraggedEventArgs, deltaY, deltaY), // Node_RightButtonDraggedEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_RightButtonDraggedEventArgs, button, button, MouseButton), // Node_RightButtonDraggedEventArgs.button
	DECL_INT(0xd9747336, Node_RightButtonDraggedEventArgs, clickCount, clickCount), // Node_RightButtonDraggedEventArgs.clickCount
};
static luaL_Reg _Node_OtherButtonDraggedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherButtonDraggedEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_OtherButtonDraggedEventArgs, x, x), // Node_OtherButtonDraggedEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_OtherButtonDraggedEventArgs, y, y), // Node_OtherButtonDraggedEventArgs.y
	DECL_INT(0xec56f27b, Node_OtherButtonDraggedEventArgs, deltaX, deltaX), // Node_OtherButtonDraggedEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_OtherButtonDraggedEventArgs, deltaY, deltaY), // Node_OtherButtonDraggedEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_OtherButtonDraggedEventArgs, button, button, MouseButton), // Node_OtherButtonDraggedEventArgs.button
	DECL_INT(0xd9747336, Node_OtherButtonDraggedEventArgs, clickCount, clickCount), // Node_OtherButtonDraggedEventArgs.clickCount
};
static luaL_Reg _Node_LeftDoubleClickEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftDoubleClickEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_LeftDoubleClickEventArgs, x, x), // Node_LeftDoubleClickEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_LeftDoubleClickEventArgs, y, y), // Node_LeftDoubleClickEventArgs.y
	DECL_INT(0xec56f27b, Node_LeftDoubleClickEventArgs, deltaX, deltaX), // Node_LeftDoubleClickEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_LeftDoubleClickEventArgs, deltaY, deltaY), // Node_LeftDoubleClickEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_LeftDoubleClickEventArgs, button, button, MouseButton), // Node_LeftDoubleClickEventArgs.button
	DECL_INT(0xd9747336, Node_LeftDoubleClickEventArgs, clickCount, clickCount), // Node_LeftDoubleClickEventArgs.clickCount
};
static luaL_Reg _Node_RightDoubleClickEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightDoubleClickEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_RightDoubleClickEventArgs, x, x), // Node_RightDoubleClickEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_RightDoubleClickEventArgs, y, y), // Node_RightDoubleClickEventArgs.y
	DECL_INT(0xec56f27b, Node_RightDoubleClickEventArgs, deltaX, deltaX), // Node_RightDoubleClickEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_RightDoubleClickEventArgs, deltaY, deltaY), // Node_RightDoubleClickEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_RightDoubleClickEventArgs, button, button, MouseButton), // Node_RightDoubleClickEventArgs.button
	DECL_INT(0xd9747336, Node_RightDoubleClickEventArgs, clickCount, clickCount), // Node_RightDoubleClickEventArgs.clickCount
};
static luaL_Reg _Node_OtherDoubleClickEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherDoubleClickEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_OtherDoubleClickEventArgs, x, x), // Node_OtherDoubleClickEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_OtherDoubleClickEventArgs, y, y), // Node_OtherDoubleClickEventArgs.y
	DECL_INT(0xec56f27b, Node_OtherDoubleClickEventArgs, deltaX, deltaX), // Node_OtherDoubleClickEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_OtherDoubleClickEventArgs, deltaY, deltaY), // Node_OtherDoubleClickEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_OtherDoubleClickEventArgs, button, button, MouseButton), // Node_OtherDoubleClickEventArgs.button
	DECL_INT(0xd9747336, Node_OtherDoubleClickEventArgs, clickCount, clickCount), // Node_OtherDoubleClickEventArgs.clickCount
};
static luaL_Reg _Node_MouseMovedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_MouseMovedEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_MouseMovedEventArgs, x, x), // Node_MouseMovedEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_MouseMovedEventArgs, y, y), // Node_MouseMovedEventArgs.y
	DECL_INT(0xec56f27b, Node_MouseMovedEventArgs, deltaX, deltaX), // Node_MouseMovedEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_MouseMovedEventArgs, deltaY, deltaY), // Node_MouseMovedEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_MouseMovedEventArgs, button, button, MouseButton), // Node_MouseMovedEventArgs.button
	DECL_INT(0xd9747336, Node_MouseMovedEventArgs, clickCount, clickCount), // Node_MouseMovedEventArgs.clickCount
};
static luaL_Reg _Node_ScrollWheelEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_ScrollWheelEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_ScrollWheelEventArgs, x, x), // Node_ScrollWheelEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_ScrollWheelEventArgs, y, y), // Node_ScrollWheelEventArgs.y
	DECL_INT(0xec56f27b, Node_ScrollWheelEventArgs, deltaX, deltaX), // Node_ScrollWheelEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_ScrollWheelEventArgs, deltaY, deltaY), // Node_ScrollWheelEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_ScrollWheelEventArgs, button, button, MouseButton), // Node_ScrollWheelEventArgs.button
	DECL_INT(0xd9747336, Node_ScrollWheelEventArgs, clickCount, clickCount), // Node_ScrollWheelEventArgs.clickCount
};
static luaL_Reg _Node_DragDropEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_DragDropEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_DragDropEventArgs, x, x), // Node_DragDropEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_DragDropEventArgs, y, y), // Node_DragDropEventArgs.y
	DECL_INT(0xec56f27b, Node_DragDropEventArgs, deltaX, deltaX), // Node_DragDropEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_DragDropEventArgs, deltaY, deltaY), // Node_DragDropEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_DragDropEventArgs, button, button, MouseButton), // Node_DragDropEventArgs.button
	DECL_INT(0xd9747336, Node_DragDropEventArgs, clickCount, clickCount), // Node_DragDropEventArgs.clickCount
};
static luaL_Reg _Node_DragEnterEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_DragEnterEventArgs[] = {
	DECL_FLOAT(0xfd0c5087, Node_DragEnterEventArgs, x, x), // Node_DragEnterEventArgs.x
	DECL_FLOAT(0xfc0c4ef4, Node_DragEnterEventArgs, y, y), // Node_DragEnterEventArgs.y
	DECL_INT(0xec56f27b, Node_DragEnterEventArgs, deltaX, deltaX), // Node_DragEnterEventArgs.deltaX
	DECL_INT(0xeb56f0e8, Node_DragEnterEventArgs, deltaY, deltaY), // Node_DragEnterEventArgs.deltaY
	DECL_ENUM(0x43b27471, Node_DragEnterEventArgs, button, button, MouseButton), // Node_DragEnterEventArgs.button
	DECL_INT(0xd9747336, Node_DragEnterEventArgs, clickCount, clickCount), // Node_DragEnterEventArgs.clickCount
};
static luaL_Reg _Node_KeyMessageEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KeyMessageEventArgs[] = {
	DECL_INT(0xd803e2a9, Node_KeyMessageEventArgs, keyCode, keyCode), // Node_KeyMessageEventArgs.keyCode
	DECL_INT(0x8b3aa710, Node_KeyMessageEventArgs, character, character), // Node_KeyMessageEventArgs.character
	DECL_INT(0x411b1019, Node_KeyMessageEventArgs, modifiers, modifiers), // Node_KeyMessageEventArgs.modifiers
	DECL_STRING(0xbde64e3e, Node_KeyMessageEventArgs, text, text), // Node_KeyMessageEventArgs.text
	DECL_STRING(0x8f82be34, Node_KeyMessageEventArgs, modifiersString, modifiersString), // Node_KeyMessageEventArgs.modifiersString
	DECL_STRING(0x9703dc13, Node_KeyMessageEventArgs, hotKey, hotKey), // Node_KeyMessageEventArgs.hotKey
};
static luaL_Reg _Node_KeyDownEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KeyDownEventArgs[] = {
	DECL_INT(0xd803e2a9, Node_KeyDownEventArgs, keyCode, keyCode), // Node_KeyDownEventArgs.keyCode
	DECL_INT(0x8b3aa710, Node_KeyDownEventArgs, character, character), // Node_KeyDownEventArgs.character
	DECL_INT(0x411b1019, Node_KeyDownEventArgs, modifiers, modifiers), // Node_KeyDownEventArgs.modifiers
	DECL_STRING(0xbde64e3e, Node_KeyDownEventArgs, text, text), // Node_KeyDownEventArgs.text
	DECL_STRING(0x8f82be34, Node_KeyDownEventArgs, modifiersString, modifiersString), // Node_KeyDownEventArgs.modifiersString
	DECL_STRING(0x9703dc13, Node_KeyDownEventArgs, hotKey, hotKey), // Node_KeyDownEventArgs.hotKey
};
static luaL_Reg _Node_KeyUpEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KeyUpEventArgs[] = {
	DECL_INT(0xd803e2a9, Node_KeyUpEventArgs, keyCode, keyCode), // Node_KeyUpEventArgs.keyCode
	DECL_INT(0x8b3aa710, Node_KeyUpEventArgs, character, character), // Node_KeyUpEventArgs.character
	DECL_INT(0x411b1019, Node_KeyUpEventArgs, modifiers, modifiers), // Node_KeyUpEventArgs.modifiers
	DECL_STRING(0xbde64e3e, Node_KeyUpEventArgs, text, text), // Node_KeyUpEventArgs.text
	DECL_STRING(0x8f82be34, Node_KeyUpEventArgs, modifiersString, modifiersString), // Node_KeyUpEventArgs.modifiersString
	DECL_STRING(0x9703dc13, Node_KeyUpEventArgs, hotKey, hotKey), // Node_KeyUpEventArgs.hotKey
};
static luaL_Reg _Node_TextInputEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_TextInputEventArgs[] = {
	DECL_INT(0xd803e2a9, Node_TextInputEventArgs, keyCode, keyCode), // Node_TextInputEventArgs.keyCode
	DECL_INT(0x8b3aa710, Node_TextInputEventArgs, character, character), // Node_TextInputEventArgs.character
	DECL_INT(0x411b1019, Node_TextInputEventArgs, modifiers, modifiers), // Node_TextInputEventArgs.modifiers
	DECL_STRING(0xbde64e3e, Node_TextInputEventArgs, text, text), // Node_TextInputEventArgs.text
	DECL_STRING(0x8f82be34, Node_TextInputEventArgs, modifiersString, modifiersString), // Node_TextInputEventArgs.modifiersString
	DECL_STRING(0x9703dc13, Node_TextInputEventArgs, hotKey, hotKey), // Node_TextInputEventArgs.hotKey
};

STRUCT(Object_CreateEventArgs, Object_CreateEventArgs);
STRUCT(Object_StartEventArgs, Object_StartEventArgs);
STRUCT(Object_AnimateEventArgs, Object_AnimateEventArgs);
STRUCT(Object_ThemeChangedEventArgs, Object_ThemeChangedEventArgs);
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
STRUCT(StyleController_ThemeChangedEventArgs, StyleController_ThemeChangedEventArgs);
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
COMPONENT_PROPERTIES(AnimationCurve) = {
	DECL_STRING(0xeb66e456, AnimationCurve, Path, Path), // AnimationCurve.Path
	DECL_STRING(0x5221f9e8, AnimationCurve, Property, Property), // AnimationCurve.Property
	ARRAY_DECL_STRUCT_TYPE(0xf893ff8e, AnimationCurve, Keyframes, Keyframes, Keyframe), // AnimationCurve.Keyframes
	DECL_INT(0x33ee8bf8, AnimationCurve, NumKeyframes, NumKeyframes), // AnimationCurve.NumKeyframes
};
COMPONENT_DEFAULTS(AnimationCurve) = {
};
COMPONENT_PROC_BEGIN(AnimationCurve)
COMPONENT_PROC_END()
void luaX_pushAnimationCurve(lua_State *L, struct AnimationCurve const* AnimationCurve) {
	luaX_pushObject(L, CMP_GetObject(AnimationCurve));
}
struct AnimationCurve* luaX_checkAnimationCurve(lua_State *L, int idx) {
	return GetAnimationCurve(luaX_checkObject(L, idx));
}
REGISTER_CLASS(AnimationCurve, 0);
HANDLER(AnimationClip, Object, Start);
COMPONENT_PROPERTIES(AnimationClip) = {
	DECL_ENUM(0x534e7732, AnimationClip, Mode, Mode, AnimationMode), // AnimationClip.Mode
	DECL_FLOAT(0xd6195a6e, AnimationClip, StartTime, StartTime), // AnimationClip.StartTime
	DECL_FLOAT(0x03274144, AnimationClip, StopTime, StopTime), // AnimationClip.StopTime
};
COMPONENT_DEFAULTS(AnimationClip) = {
};
COMPONENT_PROC_BEGIN(AnimationClip)
		DISPATCH(AnimationClip, Object, Start) // Object.Start
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(AnimationPlayer) = {
	DECL_OBJECT(0xd33ddb1b, AnimationPlayer, Clip, Clip, AnimationClip), // AnimationPlayer.Clip
	ARRAY_DECL_STRUCT_TYPE(0xf26064b8, AnimationPlayer, Clips, Clips, AnimationClipReference), // AnimationPlayer.Clips
	DECL_INT(0x090c1a52, AnimationPlayer, NumClips, NumClips), // AnimationPlayer.NumClips
	DECL_BOOL(0xdf450ad5, AnimationPlayer, Playing, Playing), // AnimationPlayer.Playing
	DECL_BOOL(0x343782cd, AnimationPlayer, Looping, Looping), // AnimationPlayer.Looping
	DECL_FLOAT(0x0a6b8020, AnimationPlayer, Speed, Speed), // AnimationPlayer.Speed
	DECL_FLOAT(0x0d3e3b9b, AnimationPlayer, CurrentTime, CurrentTime), // AnimationPlayer.CurrentTime
	DECL_BOOL(0x706b62d9, AnimationPlayer, AutoplayEnabled, AutoplayEnabled), // AnimationPlayer.AutoplayEnabled
	DECL_BOOL(0x9b01fbb4, AnimationPlayer, RelativePlayback, RelativePlayback), // AnimationPlayer.RelativePlayback
	DECL_BOOL(0x280cbcbb, AnimationPlayer, RestoreOriginalValuesAfterPlayback, RestoreOriginalValuesAfterPlayback), // AnimationPlayer.RestoreOriginalValuesAfterPlayback
	DECL_ENUM(0x234c71cf, AnimationPlayer, PlaybackMode, PlaybackMode, PlaybackMode), // AnimationPlayer.PlaybackMode
	DECL_FLOAT(0x9bcd7639, AnimationPlayer, DurationScale, DurationScale), // AnimationPlayer.DurationScale
	DECL_INT(0xa3a5f0a1, AnimationPlayer, RepeatCount, RepeatCount), // AnimationPlayer.RepeatCount
	DECL_EVENT(0x29ab6f83, AnimationPlayer, Play), // AnimationPlayer.Play
	DECL_EVENT(0x454d1d8a, AnimationPlayer, Resume), // AnimationPlayer.Resume
	DECL_EVENT(0x4b7f7705, AnimationPlayer, Stop), // AnimationPlayer.Stop
	DECL_EVENT(0x44f9bf2d, AnimationPlayer, Pause), // AnimationPlayer.Pause
	DECL_EVENT(0x97b5823e, AnimationPlayer, Started), // AnimationPlayer.Started
	DECL_EVENT(0x863f19ee, AnimationPlayer, Stopped), // AnimationPlayer.Stopped
	DECL_EVENT(0xec88aab2, AnimationPlayer, Completed), // AnimationPlayer.Completed
};
COMPONENT_DEFAULTS(AnimationPlayer) = {
		
  .Speed = 1.0,
};
COMPONENT_PROC_BEGIN(AnimationPlayer)
		DISPATCH(AnimationPlayer, Object, Start) // Object.Start
		DISPATCH(AnimationPlayer, Object, Animate) // Object.Animate
		DISPATCH(AnimationPlayer, AnimationPlayer, Play) // AnimationPlayer.Play
		DISPATCH(AnimationPlayer, AnimationPlayer, Resume) // AnimationPlayer.Resume
		DISPATCH(AnimationPlayer, AnimationPlayer, Stop) // AnimationPlayer.Stop
		DISPATCH(AnimationPlayer, AnimationPlayer, Pause) // AnimationPlayer.Pause
COMPONENT_PROC_END()
void luaX_pushAnimationPlayer(lua_State *L, struct AnimationPlayer const* AnimationPlayer) {
	luaX_pushObject(L, CMP_GetObject(AnimationPlayer));
}
struct AnimationPlayer* luaX_checkAnimationPlayer(lua_State *L, int idx) {
	return GetAnimationPlayer(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(AnimationPlayer, 0);
HANDLER(PropertyAnimation, Object, Animate);
HANDLER(PropertyAnimation, Object, Release);
COMPONENT_PROPERTIES(PropertyAnimation) = {
	DECL_STRING(0x5221f9e8, PropertyAnimation, Property, Property), // PropertyAnimation.Property
	DECL_STRING(0x18743595, PropertyAnimation, From, From), // PropertyAnimation.From
	DECL_STRING(0x41f59644, PropertyAnimation, To, To), // PropertyAnimation.To
	DECL_ENUM(0x77818ebd, PropertyAnimation, Interpolation, Interpolation, InterpolationMode), // PropertyAnimation.Interpolation
	DECL_ENUM(0xa3116148, PropertyAnimation, Easing, Easing, Easing), // PropertyAnimation.Easing
	DECL_INT(0x0ae8097f, PropertyAnimation, Start, Start), // PropertyAnimation.Start
	DECL_INT(0xd4c7492d, PropertyAnimation, Duration, Duration), // PropertyAnimation.Duration
};
COMPONENT_DEFAULTS(PropertyAnimation) = {
};
COMPONENT_PROC_BEGIN(PropertyAnimation)
		DISPATCH(PropertyAnimation, Object, Animate) // Object.Animate
		DISPATCH(PropertyAnimation, Object, Release) // Object.Release
COMPONENT_PROC_END()
void luaX_pushPropertyAnimation(lua_State *L, struct PropertyAnimation const* PropertyAnimation) {
	luaX_pushObject(L, CMP_GetObject(PropertyAnimation));
}
struct PropertyAnimation* luaX_checkPropertyAnimation(lua_State *L, int idx) {
	return GetPropertyAnimation(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(PropertyAnimation, 0);
COMPONENT_PROPERTIES(StyleSheet) = {
};
COMPONENT_DEFAULTS(StyleSheet) = {
};
COMPONENT_PROC_BEGIN(StyleSheet)
COMPONENT_PROC_END()
void luaX_pushStyleSheet(lua_State *L, struct StyleSheet const* StyleSheet) {
	luaX_pushObject(L, CMP_GetObject(StyleSheet));
}
struct StyleSheet* luaX_checkStyleSheet(lua_State *L, int idx) {
	return GetStyleSheet(luaX_checkObject(L, idx));
}
REGISTER_CLASS(StyleSheet, 0);
HANDLER(StyleRule, Object, PropertyChanged);
COMPONENT_PROPERTIES(StyleRule) = {
	DECL_STRING(0x2ecda0bc, StyleRule, ClassName, ClassName), // StyleRule.ClassName
	DECL_STRING(0xc0e9c7e9, StyleRule, PseudoClass, PseudoClass), // StyleRule.PseudoClass
};
COMPONENT_DEFAULTS(StyleRule) = {
};
COMPONENT_PROC_BEGIN(StyleRule)
		DISPATCH(StyleRule, Object, PropertyChanged) // Object.PropertyChanged
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(StyleController) = {
	DECL_OBJECT(0x6546e1e1, StyleController, StyleSheet, StyleSheet, StyleSheet), // StyleController.StyleSheet
	DECL_EVENT(0x064087a6, StyleController, ThemeChanged), // StyleController.ThemeChanged
	DECL_EVENT(0x543ca51c, StyleController, AddClass), // StyleController.AddClass
	DECL_EVENT(0x41acd398, StyleController, AddClasses), // StyleController.AddClasses
};
COMPONENT_DEFAULTS(StyleController) = {
};
COMPONENT_PROC_BEGIN(StyleController)
		DISPATCH(StyleController, Object, Create) // Object.Create
		DISPATCH(StyleController, Object, Release) // Object.Release
		DISPATCH(StyleController, StyleController, ThemeChanged) // StyleController.ThemeChanged
		DISPATCH(StyleController, StyleController, AddClass) // StyleController.AddClass
		DISPATCH(StyleController, StyleController, AddClasses) // StyleController.AddClasses
COMPONENT_PROC_END()
void luaX_pushStyleController(lua_State *L, struct StyleController const* StyleController) {
	luaX_pushObject(L, CMP_GetObject(StyleController));
}
struct StyleController* luaX_checkStyleController(lua_State *L, int idx) {
	return GetStyleController(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(StyleController, 0);
COMPONENT_PROPERTIES(StateManager) = {
};
COMPONENT_DEFAULTS(StateManager) = {
};
COMPONENT_PROC_BEGIN(StateManager)
COMPONENT_PROC_END()
void luaX_pushStateManager(lua_State *L, struct StateManager const* StateManager) {
	luaX_pushObject(L, CMP_GetObject(StateManager));
}
struct StateManager* luaX_checkStateManager(lua_State *L, int idx) {
	return GetStateManager(luaX_checkObject(L, idx));
}
REGISTER_CLASS(StateManager, 0);
COMPONENT_PROPERTIES(StateGroup) = {
	DECL_STRING(0xd9dc005c, StateGroup, ControllerProperty, ControllerProperty), // StateGroup.ControllerProperty
};
COMPONENT_DEFAULTS(StateGroup) = {
};
COMPONENT_PROC_BEGIN(StateGroup)
COMPONENT_PROC_END()
void luaX_pushStateGroup(lua_State *L, struct StateGroup const* StateGroup) {
	luaX_pushObject(L, CMP_GetObject(StateGroup));
}
struct StateGroup* luaX_checkStateGroup(lua_State *L, int idx) {
	return GetStateGroup(luaX_checkObject(L, idx));
}
REGISTER_CLASS(StateGroup, 0);
COMPONENT_PROPERTIES(State) = {
	DECL_STRING(0xd147f96a, State, Value, Value), // State.Value
	DECL_STRING(0xeb66e456, State, Path, Path), // State.Path
};
COMPONENT_DEFAULTS(State) = {
};
COMPONENT_PROC_BEGIN(State)
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(StateManagerController) = {
	DECL_OBJECT(0xe76f2815, StateManagerController, StateManager, StateManager, StateManager), // StateManagerController.StateManager
	DECL_EVENT(0xda0795ff, StateManagerController, ControllerChanged), // StateManagerController.ControllerChanged
};
COMPONENT_DEFAULTS(StateManagerController) = {
};
COMPONENT_PROC_BEGIN(StateManagerController)
		DISPATCH(StateManagerController, Object, Start) // Object.Start
		DISPATCH(StateManagerController, Object, PropertyChanged) // Object.PropertyChanged
		DISPATCH(StateManagerController, StateManagerController, ControllerChanged) // StateManagerController.ControllerChanged
COMPONENT_PROC_END()
void luaX_pushStateManagerController(lua_State *L, struct StateManagerController const* StateManagerController) {
	luaX_pushObject(L, CMP_GetObject(StateManagerController));
}
struct StateManagerController* luaX_checkStateManagerController(lua_State *L, int idx) {
	return GetStateManagerController(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(StateManagerController, 0);
COMPONENT_PROPERTIES(ResourceDictionary) = {
};
COMPONENT_DEFAULTS(ResourceDictionary) = {
};
COMPONENT_PROC_BEGIN(ResourceDictionary)
COMPONENT_PROC_END()
void luaX_pushResourceDictionary(lua_State *L, struct ResourceDictionary const* ResourceDictionary) {
	luaX_pushObject(L, CMP_GetObject(ResourceDictionary));
}
struct ResourceDictionary* luaX_checkResourceDictionary(lua_State *L, int idx) {
	return GetResourceDictionary(luaX_checkObject(L, idx));
}
REGISTER_CLASS(ResourceDictionary, 0);
COMPONENT_PROPERTIES(Locale) = {
	DECL_STRING(0x9a73db9b, Locale, Language, Language), // Locale.Language
	ARRAY_DECL_STRUCT_TYPE(0xdcaca293, Locale, Entries, Entries, LocaleEntry), // Locale.Entries
	DECL_INT(0x67d575c1, Locale, NumEntries, NumEntries), // Locale.NumEntries
};
COMPONENT_DEFAULTS(Locale) = {
};
COMPONENT_PROC_BEGIN(Locale)
COMPONENT_PROC_END()
void luaX_pushLocale(lua_State *L, struct Locale const* Locale) {
	luaX_pushObject(L, CMP_GetObject(Locale));
}
struct Locale* luaX_checkLocale(lua_State *L, int idx) {
	return GetLocale(luaX_checkObject(L, idx));
}
#define ID_ResourceDictionary 0xde7febc5
REGISTER_CLASS(Locale, ID_ResourceDictionary, 0);
COMPONENT_PROPERTIES(DataObject) = {
};
COMPONENT_DEFAULTS(DataObject) = {
};
COMPONENT_PROC_BEGIN(DataObject)
COMPONENT_PROC_END()
void luaX_pushDataObject(lua_State *L, struct DataObject const* DataObject) {
	luaX_pushObject(L, CMP_GetObject(DataObject));
}
struct DataObject* luaX_checkDataObject(lua_State *L, int idx) {
	return GetDataObject(luaX_checkObject(L, idx));
}
REGISTER_CLASS(DataObject, 0);
HANDLER(Trigger, Object, PropertyChanged);
HANDLER(Trigger, Object, Attached);
HANDLER(Trigger, Trigger, Triggered);
COMPONENT_PROPERTIES(Trigger) = {
	DECL_STRING(0x5221f9e8, Trigger, Property, Property), // Trigger.Property
	DECL_INT(0xd147f96a, Trigger, Value, Value), // Trigger.Value
	DECL_EVENT(0x3b1c3ae2, Trigger, Triggered), // Trigger.Triggered
};
COMPONENT_DEFAULTS(Trigger) = {
};
COMPONENT_PROC_BEGIN(Trigger)
		DISPATCH(Trigger, Object, PropertyChanged) // Object.PropertyChanged
		DISPATCH(Trigger, Object, Attached) // Object.Attached
		DISPATCH(Trigger, Trigger, Triggered) // Trigger.Triggered
COMPONENT_PROC_END()
void luaX_pushTrigger(lua_State *L, struct Trigger const* Trigger) {
	luaX_pushObject(L, CMP_GetObject(Trigger));
}
struct Trigger* luaX_checkTrigger(lua_State *L, int idx) {
	return GetTrigger(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Trigger, 0);
HANDLER(OnPropertyChangedTrigger, Object, PropertyChanged);
HANDLER(OnPropertyChangedTrigger, Object, Attached);
COMPONENT_PROPERTIES(OnPropertyChangedTrigger) = {
	DECL_STRING(0x9ff03304, OnPropertyChangedTrigger, SourceNode, SourceNode), // OnPropertyChangedTrigger.SourceNode
	DECL_STRING(0x5221f9e8, OnPropertyChangedTrigger, Property, Property), // OnPropertyChangedTrigger.Property
};
COMPONENT_DEFAULTS(OnPropertyChangedTrigger) = {
};
COMPONENT_PROC_BEGIN(OnPropertyChangedTrigger)
		DISPATCH(OnPropertyChangedTrigger, Object, PropertyChanged) // Object.PropertyChanged
		DISPATCH(OnPropertyChangedTrigger, Object, Attached) // Object.Attached
COMPONENT_PROC_END()
void luaX_pushOnPropertyChangedTrigger(lua_State *L, struct OnPropertyChangedTrigger const* OnPropertyChangedTrigger) {
	luaX_pushObject(L, CMP_GetObject(OnPropertyChangedTrigger));
}
struct OnPropertyChangedTrigger* luaX_checkOnPropertyChangedTrigger(lua_State *L, int idx) {
	return GetOnPropertyChangedTrigger(luaX_checkObject(L, idx));
}
#define ID_Trigger 0xa5ea0da3
REGISTER_CLASS(OnPropertyChangedTrigger, ID_Trigger, 0);
HANDLER(OnAttachedTrigger, Object, Attached);
COMPONENT_PROPERTIES(OnAttachedTrigger) = {
};
COMPONENT_DEFAULTS(OnAttachedTrigger) = {
};
COMPONENT_PROC_BEGIN(OnAttachedTrigger)
		DISPATCH(OnAttachedTrigger, Object, Attached) // Object.Attached
COMPONENT_PROC_END()
void luaX_pushOnAttachedTrigger(lua_State *L, struct OnAttachedTrigger const* OnAttachedTrigger) {
	luaX_pushObject(L, CMP_GetObject(OnAttachedTrigger));
}
struct OnAttachedTrigger* luaX_checkOnAttachedTrigger(lua_State *L, int idx) {
	return GetOnAttachedTrigger(luaX_checkObject(L, idx));
}
#define ID_Trigger 0xa5ea0da3
REGISTER_CLASS(OnAttachedTrigger, ID_Trigger, 0);
COMPONENT_PROPERTIES(EventTrigger) = {
	DECL_STRING(0x30d77e1a, EventTrigger, RoutedEvent, RoutedEvent), // EventTrigger.RoutedEvent
};
COMPONENT_DEFAULTS(EventTrigger) = {
};
LRESULT EventTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	return FALSE;
}
void luaX_pushEventTrigger(lua_State *L, struct EventTrigger const* EventTrigger) {
	luaX_pushObject(L, CMP_GetObject(EventTrigger));
}
struct EventTrigger* luaX_checkEventTrigger(lua_State *L, int idx) {
	return GetEventTrigger(luaX_checkObject(L, idx));
}
#define ID_Trigger 0xa5ea0da3
REGISTER_CLASS(EventTrigger, ID_Trigger, 0);
COMPONENT_PROPERTIES(OnEventTrigger) = {
	DECL_STRING(0x30d77e1a, OnEventTrigger, RoutedEvent, RoutedEvent), // OnEventTrigger.RoutedEvent
};
COMPONENT_DEFAULTS(OnEventTrigger) = {
};
LRESULT OnEventTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	return FALSE;
}
void luaX_pushOnEventTrigger(lua_State *L, struct OnEventTrigger const* OnEventTrigger) {
	luaX_pushObject(L, CMP_GetObject(OnEventTrigger));
}
struct OnEventTrigger* luaX_checkOnEventTrigger(lua_State *L, int idx) {
	return GetOnEventTrigger(luaX_checkObject(L, idx));
}
REGISTER_CLASS(OnEventTrigger, ID_Trigger, 0);
COMPONENT_PROPERTIES(OnClickTrigger) = {
	DECL_STRING(0x30d77e1a, OnClickTrigger, RoutedEvent, RoutedEvent), // OnClickTrigger.RoutedEvent
};
COMPONENT_DEFAULTS(OnClickTrigger) = {
	.RoutedEvent = "Node.LeftButtonUp",
};
LRESULT OnClickTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	return FALSE;
}
void luaX_pushOnClickTrigger(lua_State *L, struct OnClickTrigger const* OnClickTrigger) {
	luaX_pushObject(L, CMP_GetObject(OnClickTrigger));
}
struct OnClickTrigger* luaX_checkOnClickTrigger(lua_State *L, int idx) {
	return GetOnClickTrigger(luaX_checkObject(L, idx));
}
REGISTER_CLASS(OnClickTrigger, ID_Trigger, 0);
HANDLER(Setter, Trigger, Triggered);
COMPONENT_PROPERTIES(Setter) = {
	DECL_OBJECT(0xa5ea0da3, Setter, Trigger, Trigger, Trigger), // Setter.Trigger
	DECL_STRING(0x5221f9e8, Setter, Property, Property), // Setter.Property
	DECL_STRING(0xd147f96a, Setter, Value, Value), // Setter.Value
};
COMPONENT_DEFAULTS(Setter) = {
};
COMPONENT_PROC_BEGIN(Setter)
		DISPATCH(Setter, Trigger, Triggered) // Trigger.Triggered
COMPONENT_PROC_END()
void luaX_pushSetter(lua_State *L, struct Setter const* Setter) {
	luaX_pushObject(L, CMP_GetObject(Setter));
}
struct Setter* luaX_checkSetter(lua_State *L, int idx) {
	return GetSetter(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Setter, 0);
HANDLER(ShowModalAction, Object, Attached);
HANDLER(ShowModalAction, Trigger, Triggered);
COMPONENT_PROPERTIES(ShowModalAction) = {
	DECL_OBJECT(0xa5ea0da3, ShowModalAction, Trigger, Trigger, Trigger), // ShowModalAction.Trigger
	DECL_STRING(0xeb66e456, ShowModalAction, Path, Path), // ShowModalAction.Path
};
COMPONENT_DEFAULTS(ShowModalAction) = {
};
COMPONENT_PROC_BEGIN(ShowModalAction)
		DISPATCH(ShowModalAction, Object, Attached) // Object.Attached
		DISPATCH(ShowModalAction, Trigger, Triggered) // Trigger.Triggered
COMPONENT_PROC_END()
void luaX_pushShowModalAction(lua_State *L, struct ShowModalAction const* ShowModalAction) {
	luaX_pushObject(L, CMP_GetObject(ShowModalAction));
}
struct ShowModalAction* luaX_checkShowModalAction(lua_State *L, int idx) {
	return GetShowModalAction(luaX_checkObject(L, idx));
}
REGISTER_CLASS(ShowModalAction, 0);
HANDLER(HideAction, Object, Attached);
HANDLER(HideAction, Trigger, Triggered);
COMPONENT_PROPERTIES(HideAction) = {
	DECL_OBJECT(0xa5ea0da3, HideAction, Trigger, Trigger, Trigger), // HideAction.Trigger
	DECL_STRING(0xeb66e456, HideAction, Path, Path), // HideAction.Path
};
COMPONENT_DEFAULTS(HideAction) = {
};
COMPONENT_PROC_BEGIN(HideAction)
		DISPATCH(HideAction, Object, Attached) // Object.Attached
		DISPATCH(HideAction, Trigger, Triggered) // Trigger.Triggered
COMPONENT_PROC_END()
void luaX_pushHideAction(lua_State *L, struct HideAction const* HideAction) {
	luaX_pushObject(L, CMP_GetObject(HideAction));
}
struct HideAction* luaX_checkHideAction(lua_State *L, int idx) {
	return GetHideAction(luaX_checkObject(L, idx));
}
REGISTER_CLASS(HideAction, 0);
HANDLER(Node, Node, GetSize);
HANDLER(Node, Node, IsVisible);
HANDLER(Node, Object, Start);
HANDLER(Node, Object, PropertyChanged);
HANDLER(Node, Object, Attached);
COMPONENT_PROPERTIES(Node) = {
	DECL_STRUCT_TYPE(0xa6478e7c, Node, Size, Size, SizeShorthand), // Node.Size
	DECL_STRUCT_TYPE(0x2dbf56d8, Node, HorizontalSize, Size.Axis[0], SizeAxisShorthand), // Node.HorizontalSize
	DECL_FLOAT(0x3b42dfbf, Node, Width, Size.Axis[0].Requested), // Node.Width
	DECL_FLOAT(0xbe3d446f, Node, DesiredWidth, Size.Axis[0].Desired), // Node.DesiredWidth
	DECL_FLOAT(0xfe87ddd9, Node, MinWidth, Size.Axis[0].Min), // Node.MinWidth
	DECL_FLOAT(0xa6d77d39, Node, ActualWidth, Size.Axis[0].Actual), // Node.ActualWidth
	DECL_FLOAT(0x29366f18, Node, ScrollWidth, Size.Axis[0].Scroll), // Node.ScrollWidth
	DECL_STRUCT_TYPE(0x629fba9a, Node, VerticalSize, Size.Axis[1], SizeAxisShorthand), // Node.VerticalSize
	DECL_FLOAT(0x1bd13562, Node, Height, Size.Axis[1].Requested), // Node.Height
	DECL_FLOAT(0xf5943e12, Node, DesiredHeight, Size.Axis[1].Desired), // Node.DesiredHeight
	DECL_FLOAT(0x9f6bc248, Node, MinHeight, Size.Axis[1].Min), // Node.MinHeight
	DECL_FLOAT(0x34c40b28, Node, ActualHeight, Size.Axis[1].Actual), // Node.ActualHeight
	DECL_FLOAT(0x5320b4af, Node, ScrollHeight, Size.Axis[1].Scroll), // Node.ScrollHeight
	DECL_STRUCT_TYPE(0x51724993, Node, DepthSize, Size.Axis[2], SizeAxisShorthand), // Node.DepthSize
	DECL_FLOAT(0xd070218a, Node, Depth, Size.Axis[2].Requested), // Node.Depth
	DECL_FLOAT(0xe3fa9b3a, Node, DesiredDepth, Size.Axis[2].Desired), // Node.DesiredDepth
	DECL_FLOAT(0xe01ae1b0, Node, MinDepth, Size.Axis[2].Min), // Node.MinDepth
	DECL_FLOAT(0x886c1410, Node, ActualDepth, Size.Axis[2].Actual), // Node.ActualDepth
	DECL_FLOAT(0x04d53041, Node, ScrollDepth, Size.Axis[2].Scroll), // Node.ScrollDepth
	DECL_STRUCT_TYPE(0xc4cc799b, Node, Margin, Margin, Thickness), // Node.Margin
	DECL_STRUCT_TYPE(0x1d32e627, Node, HorizontalMargin, Margin.Axis[0], EdgeShorthand), // Node.HorizontalMargin
	DECL_FLOAT(0x0bdcf7a2, Node, MarginLeft, Margin.Axis[0].Left), // Node.MarginLeft
	DECL_FLOAT(0xe9999c8f, Node, MarginRight, Margin.Axis[0].Right), // Node.MarginRight
	DECL_STRUCT_TYPE(0xb859064d, Node, VerticalMargin, Margin.Axis[1], EdgeShorthand), // Node.VerticalMargin
	DECL_FLOAT(0xe0a2f632, Node, MarginTop, Margin.Axis[1].Left), // Node.MarginTop
	DECL_FLOAT(0x4231df34, Node, MarginBottom, Margin.Axis[1].Right), // Node.MarginBottom
	DECL_STRUCT_TYPE(0x606034ac, Node, DepthMargin, Margin.Axis[2], EdgeShorthand), // Node.DepthMargin
	DECL_FLOAT(0x8bfc919a, Node, MarginFront, Margin.Axis[2].Left), // Node.MarginFront
	DECL_FLOAT(0xfc651334, Node, MarginBack, Margin.Axis[2].Right), // Node.MarginBack
	DECL_STRUCT_TYPE(0x0736dd56, Node, Padding, Padding, Thickness), // Node.Padding
	DECL_STRUCT_TYPE(0x96255a02, Node, HorizontalPadding, Padding.Axis[0], EdgeShorthand), // Node.HorizontalPadding
	DECL_FLOAT(0x380ec7b3, Node, PaddingLeft, Padding.Axis[0].Left), // Node.PaddingLeft
	DECL_FLOAT(0x2e56c2bc, Node, PaddingRight, Padding.Axis[0].Right), // Node.PaddingRight
	DECL_STRUCT_TYPE(0x147a27e0, Node, VerticalPadding, Padding.Axis[1], EdgeShorthand), // Node.VerticalPadding
	DECL_FLOAT(0x67bd6401, Node, PaddingTop, Padding.Axis[1].Left), // Node.PaddingTop
	DECL_FLOAT(0x8e7f220d, Node, PaddingBottom, Padding.Axis[1].Right), // Node.PaddingBottom
	DECL_STRUCT_TYPE(0xcba5b053, Node, DepthPadding, Padding.Axis[2], EdgeShorthand), // Node.DepthPadding
	DECL_FLOAT(0x3d180671, Node, PaddingFront, Padding.Axis[2].Left), // Node.PaddingFront
	DECL_FLOAT(0x8c287ea5, Node, PaddingBack, Padding.Axis[2].Right), // Node.PaddingBack
	DECL_STRUCT_TYPE(0x0cad6f57, Node, Border, Border, BorderShorthand), // Node.Border
	DECL_STRUCT_TYPE(0x0aaf7cf9, Node, BorderWidth, Border.Width, Thickness), // Node.BorderWidth
	DECL_STRUCT_TYPE(0x19c21815, Node, HorizontalBorderWidth, Border.Width.Axis[0], EdgeShorthand), // Node.HorizontalBorderWidth
	DECL_FLOAT(0x63b03bb4, Node, BorderWidthLeft, Border.Width.Axis[0].Left), // Node.BorderWidthLeft
	DECL_FLOAT(0xe6977c61, Node, BorderWidthRight, Border.Width.Axis[0].Right), // Node.BorderWidthRight
	DECL_STRUCT_TYPE(0x6b40a6c7, Node, VerticalBorderWidth, Border.Width.Axis[1], EdgeShorthand), // Node.VerticalBorderWidth
	DECL_FLOAT(0x54b7f688, Node, BorderWidthTop, Border.Width.Axis[1].Left), // Node.BorderWidthTop
	DECL_FLOAT(0xa44eab36, Node, BorderWidthBottom, Border.Width.Axis[1].Right), // Node.BorderWidthBottom
	DECL_STRUCT_TYPE(0x3170de3c, Node, DepthBorderWidth, Border.Width.Axis[2], EdgeShorthand), // Node.DepthBorderWidth
	DECL_FLOAT(0x14d0d564, Node, BorderWidthFront, Border.Width.Axis[2].Left), // Node.BorderWidthFront
	DECL_FLOAT(0x7c087736, Node, BorderWidthBack, Border.Width.Axis[2].Right), // Node.BorderWidthBack
	DECL_COLOR(0x933e48c6, Node, BorderColor, Border.Color), // Node.BorderColor
	DECL_ENUM(0x390b4488, Node, BorderStyle, Border.Style, BorderStyle), // Node.BorderStyle
	DECL_STRUCT_TYPE(0xb8e9fe05, Node, BorderRadius, Border.Radius, CornerRadius), // Node.BorderRadius
	DECL_FLOAT(0x22252041, Node, BorderTopLeftRadius, Border.Radius.TopLeftRadius), // Node.BorderTopLeftRadius
	DECL_FLOAT(0x789f0d82, Node, BorderTopRightRadius, Border.Radius.TopRightRadius), // Node.BorderTopRightRadius
	DECL_FLOAT(0xc321a1f8, Node, BorderBottomRightRadius, Border.Radius.BottomRightRadius), // Node.BorderBottomRightRadius
	DECL_FLOAT(0x700e8e07, Node, BorderBottomLeftRadius, Border.Radius.BottomLeftRadius), // Node.BorderBottomLeftRadius
	DECL_STRUCT_TYPE(0xd66abafe, Node, Alignment, Alignment, AlignmentShorthand), // Node.Alignment
	DECL_ENUM(0x1b8d5152, Node, HorizontalAlignment, Alignment.Axis[0], HorizontalAlignment), // Node.HorizontalAlignment
	DECL_ENUM(0x94b01054, Node, VerticalAlignment, Alignment.Axis[1], VerticalAlignment), // Node.VerticalAlignment
	DECL_ENUM(0x7ef540ff, Node, DepthAlignment, Alignment.Axis[2], DepthAlignment), // Node.DepthAlignment
	DECL_BOOL(0x592a4941, Node, Visible, Visible), // Node.Visible
	DECL_BOOL(0x20d9ba7c, Node, QuickHide, QuickHide), // Node.QuickHide
	DECL_FLOAT(0xa0b06d26, Node, VisibleAmountInParent, VisibleAmountInParent), // Node.VisibleAmountInParent
	DECL_FLOAT(0xde1f0406, Node, Opacity, Opacity), // Node.Opacity
	DECL_STRING(0x76bda0c0, Node, Tags, Tags), // Node.Tags
	DECL_OBJECT(0xa310331c, Node, DataContext, DataContext, DataObject), // Node.DataContext
	ARRAY_DECL_STRUCT_TYPE(0x9564a892, Node, Resources, Resources, ResourceEntry), // Node.Resources
	DECL_INT(0x25139ae4, Node, NumResources, NumResources), // Node.NumResources
	ARRAY_DECL_OBJECT(0x73803bc4, Node, Triggers, Triggers, Trigger), // Node.Triggers
	DECL_INT(0x0f6d8ade, Node, NumTriggers, NumTriggers), // Node.NumTriggers
	DECL_EVENT(0x5dbe404d, Node, UpdateMatrix), // Node.UpdateMatrix
	DECL_EVENT(0xa3650e54, Node, LoadView), // Node.LoadView
	DECL_EVENT(0x898160ea, Node, HitTest), // Node.HitTest
	DECL_EVENT(0x608d20d1, Node, IsVisible), // Node.IsVisible
	DECL_EVENT(0x03e93095, Node, ViewDidLoad), // Node.ViewDidLoad
	DECL_EVENT(0xa7c0f8d7, Node, KillFocus), // Node.KillFocus
	DECL_EVENT(0xc399d265, Node, SetFocus), // Node.SetFocus
	DECL_EVENT(0x80d9e0ee, Node, GetSize), // Node.GetSize
	DECL_EVENT(0x0d589383, Node, MouseMessage), // Node.MouseMessage
	DECL_EVENT(0x6cc782b0, Node, LeftButtonDown), // Node.LeftButtonDown
	DECL_EVENT(0x13a69401, Node, RightButtonDown), // Node.RightButtonDown
	DECL_EVENT(0x39ed21d1, Node, OtherButtonDown), // Node.OtherButtonDown
	DECL_EVENT(0x0bb30ac9, Node, LeftButtonUp), // Node.LeftButtonUp
	DECL_EVENT(0x05e78e74, Node, RightButtonUp), // Node.RightButtonUp
	DECL_EVENT(0xc86953a4, Node, OtherButtonUp), // Node.OtherButtonUp
	DECL_EVENT(0x4884ec22, Node, LeftButtonDragged), // Node.LeftButtonDragged
	DECL_EVENT(0x651f7219, Node, RightButtonDragged), // Node.RightButtonDragged
	DECL_EVENT(0xf4cda889, Node, OtherButtonDragged), // Node.OtherButtonDragged
	DECL_EVENT(0x5a92bc67, Node, LeftDoubleClick), // Node.LeftDoubleClick
	DECL_EVENT(0xeeebbe60, Node, RightDoubleClick), // Node.RightDoubleClick
	DECL_EVENT(0xf6c60630, Node, OtherDoubleClick), // Node.OtherDoubleClick
	DECL_EVENT(0x65db8b6f, Node, MouseMoved), // Node.MouseMoved
	DECL_EVENT(0x626f90e3, Node, ScrollWheel), // Node.ScrollWheel
	DECL_EVENT(0x25989e7a, Node, DragDrop), // Node.DragDrop
	DECL_EVENT(0xc0e97a77, Node, DragEnter), // Node.DragEnter
	DECL_EVENT(0xee4b252b, Node, KeyMessage), // Node.KeyMessage
	DECL_EVENT(0x83b19b78, Node, KeyDown), // Node.KeyDown
	DECL_EVENT(0xfca37d71, Node, KeyUp), // Node.KeyUp
	DECL_EVENT(0xd484cca2, Node, TextInput), // Node.TextInput
};
COMPONENT_DEFAULTS(Node) = {
		
  .Visible = TRUE,
		
  .VisibleAmountInParent = 1,
		
  .Opacity = 1,
};
COMPONENT_PROC_BEGIN(Node)
		DISPATCH(Node, Node, GetSize) // Node.GetSize
		DISPATCH(Node, Node, IsVisible) // Node.IsVisible
		DISPATCH(Node, Object, Start) // Object.Start
		DISPATCH(Node, Object, PropertyChanged) // Object.PropertyChanged
		DISPATCH(Node, Object, Attached) // Object.Attached
COMPONENT_PROC_END()
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
int f_core_FlushQueue(lua_State *L) {
	core_FlushQueue(L );
	return 0;
}
int f_core_ObjectCount(lua_State *L) {
	lua_pushinteger(L, OBJ_GetObjectCount());
	return 1;
}

ORCA_API int luaopen_orca_core(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ "getFocus", f_core_GetFocus },
		{ "getHover", f_core_GetHover },
		{ "addGlobalStyleRule", f_core_AddGlobalStyleRule },
		{ "advanceFrame", f_core_AdvanceFrame },
		{ "flushQueue", f_core_FlushQueue },
		{ "objectCount", f_core_ObjectCount },
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
		lua_setfield(L, ((void)luaopen_orca_Object_ThemeChangedEventArgs(L), -2), "Object_ThemeChangedEventArgs");
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
		lua_setfield(L, ((void)luaopen_orca_StyleController_ThemeChangedEventArgs(L), -2), "StyleController_ThemeChangedEventArgs");
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
	lua_setfield(L, ((void)lua_pushclass(L, &_OnEventTrigger), -2), "OnEventTrigger");
	lua_setfield(L, ((void)lua_pushclass(L, &_OnClickTrigger), -2), "OnClickTrigger");
	lua_setfield(L, ((void)lua_pushclass(L, &_Setter), -2), "Setter");
	lua_setfield(L, ((void)lua_pushclass(L, &_ShowModalAction), -2), "ShowModalAction");
	lua_setfield(L, ((void)lua_pushclass(L, &_HideAction), -2), "HideAction");
	lua_setfield(L, ((void)lua_pushclass(L, &_Node), -2), "Node");
	void after_core_module_registered(lua_State *L);
	after_core_module_registered(L);
	return 1;
}
