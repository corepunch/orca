// Auto-generated from core.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "core.h"

#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }

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
// style_class
extern void luaX_pushstyle_class(lua_State *L, struct style_class const* value);
extern struct style_class* luaX_checkstyle_class(lua_State *L, int index);
// style_sheet
extern void luaX_pushstyle_sheet(lua_State *L, struct style_sheet const* value);
extern struct style_sheet* luaX_checkstyle_sheet(lua_State *L, int index);

#define ENUM(NAME, ...) \
ORCA_API const char *_##NAME[] = {__VA_ARGS__, NULL}; \
const char *NAME##ToString(enum NAME value) { \
	return (assert(value >= 0 && value < sizeof(_##NAME) / sizeof(*_##NAME) - 1), _##NAME[value]); \
} \
enum NAME luaX_check##NAME(lua_State *L, int idx) { \
	return luaL_checkoption(L, idx, NULL, _##NAME); \
} \
void luaX_push##NAME(lua_State *L, enum NAME value) { \
	lua_pushstring(L, (assert(value >= 0 && value < sizeof(_##NAME) / sizeof(*_##NAME) - 1), _##NAME[value])); \
}
ENUM(MessageRouting, "Bubbling", "TunnelingBubbling", "Tunneling", "Direct")
ENUM(PropertyState, "Normal", "Hover", "Focus", "Select", "Disable", "OldValue")
ENUM(BindingMode, "OneWay", "TwoWay", "OneWayToSource", "Expression")
ENUM(PropertyAttribute, "WholeProperty", "ColorR", "ColorG", "ColorB", "ColorA", "VectorX", "VectorY", "VectorZ", "VectorW")
ENUM(AnimationMode, "PlayOnce", "Loop", "PingPong")
ENUM(PlaybackMode, "Normal", "Reverse", "PingPong")
ENUM(InterpolationMode, "Linear", "Const", "Back", "Bounce", "Circ", "Cubic", "Elastic", "Expo", "Quad", "Quart", "Quint", "Sine")
ENUM(Easing, "InOut", "In", "Out")

int f_OBJ_CreateFromLuaState(lua_State *L) {
	return OBJ_CreateFromLuaState(L);
}
int f_OBJ_Awake(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_Awake(L, this_ );
	return 0;
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
int f_OBJ_PostMessage(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* message = luaL_checkstring(L, 2);
	OBJ_PostMessage(L, this_, message );
	return 0;
}
int f_OBJ_MsgSend(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* message = luaL_checkstring(L, 2);
	OBJ_MsgSend(L, this_, message );
	return 0;
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
int f_OBJ_ApplyStyles(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	bool_t recursive = lua_toboolean(L, 2);
	OBJ_ApplyStyles(this_, recursive );
	return 0;
}
int f_OBJ_AddStyleSheet(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* source = luaL_checkstring(L, 2);
	OBJ_AddStyleSheet(L, this_, source );
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
int f_OBJ_GetLuaObject(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	uint32_t result_ = OBJ_GetLuaObject(this_);
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
int f_OBJ_ParseClassAttribute(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* classString = luaL_checkstring(L, 2);
	OBJ_ParseClassAttribute(this_, classString );
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
	OBJ_LoadPrefabs(L, this_ );
	return 0;
}
int f_OBJ_IsPrefabView(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	bool_t result_ = OBJ_IsPrefabView(this_);
	lua_pushboolean(L, result_);
	return 1;
}
int f_OBJ_AddAlias(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* name = luaL_checkstring(L, 2);
	const char* path = luaL_checkstring(L, 3);
	OBJ_AddAlias(this_, name, path );
	return 0;
}
int f_OBJ_AssignAliases(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* pathToObject = luaL_checkstring(L, 2);
	OBJ_AssignAliases(this_, pathToObject );
	return 0;
}

int luaopen_orca_Object(lua_State *L) {
	luaL_newmetatable(L, "Object");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_OBJ_CreateFromLuaState },
		{ "awake", f_OBJ_Awake },
		{ "animate", f_OBJ_Animate },
		{ "clear", f_OBJ_Clear },
		{ "__gc", f_OBJ_Release },
		{ "__eq", f_OBJ_Equals },
		{ "rebuild", f_OBJ_Rebuild },
		{ "addChild", f_OBJ_AddChild },
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
		{ "postMessage", f_OBJ_PostMessage },
		{ "msgSend", f_OBJ_MsgSend },
		{ "__setproperty", f_OBJ_SetProperty },
		{ "__getproperty", f_OBJ_GetProperty },
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
		{ "applyStyles", f_OBJ_ApplyStyles },
		{ "addStyleSheet", f_OBJ_AddStyleSheet },
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
		{ "getLuaObject", f_OBJ_GetLuaObject },
		{ "getDomain", f_OBJ_GetDomain },
		{ "__setcontext", f_OBJ_SetContext },
		{ "parseClassAttribute", f_OBJ_ParseClassAttribute },
		{ "instantiate", f_OBJ_Instantiate },
		{ "loadPrefabs", f_OBJ_LoadPrefabs },
		{ "isPrefabView", f_OBJ_IsPrefabView },
		{ "addAlias", f_OBJ_AddAlias },
		{ "assignAliases", f_OBJ_AssignAliases },
		{ NULL, NULL },
	}), 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	return 1;
}
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(lua_State *L, const char* str, struct PropertyType const* prop, void* struct_ptr);

#define STRUCT(NAME, EXPORT) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
	if (data == NULL) { lua_pushnil(L); return; } \
	memcpy(lua_newuserdata(L, sizeof(struct NAME)), data, sizeof(struct NAME)); \
	luaL_setmetatable(L, #EXPORT); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { return luaL_checkudata(L, idx, #EXPORT); } \
static int f_new_##NAME(lua_State *L) { \
	struct NAME self; \
	memset(&self, 0, sizeof(struct NAME)); \
	if (lua_islightuserdata(L, 1)) { \
		memcpy(&self, lua_touserdata(L, 1), sizeof(struct NAME)); \
	} else if (lua_istable(L, 1)) \
		for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); lua_pop(L, 1), i++) { \
			if (lua_getfield(L, 1, _##NAME[i].Name)) \
				read_property(L, -1, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); } \
	else for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		read_property(L, i + 1, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); \
	luaX_push##NAME(L, &self); \
	return 1; \
} \
static int f_##NAME##___index(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (write_property(L, &_##NAME[i], ((char*)luaX_check##NAME(L, 1))+_##NAME[i].Offset), 1); \
	for (uint32_t i = 0; i < sizeof(_##NAME##_Methods) / sizeof(*_##NAME##_Methods); i++) { \
		if (_##NAME##_Methods[i].name && strcmp(_##NAME##_Methods[i].name, luaL_checkstring(L, 2)) == 0) { \
			lua_pushcfunction(L, _##NAME##_Methods[i].func); \
			return 1; \
		} \
	} \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___newindex(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (read_property(L, 3, &_##NAME[i], ((char*)luaX_check##NAME(L, 1))+_##NAME[i].Offset), 0); \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___call(lua_State *L) { \
  lua_insert(L, (lua_getfield(L, 1, "new"), 2)); \
  lua_call(L, lua_gettop(L) - 2, 1); \
	return 1; \
} \
static int f_##NAME##___fromstring(lua_State *L) { \
	char* tmp = strdup(luaL_checkstring(L, 1)),* tok = strtok(tmp, " "); \
	struct NAME self; \
	memset(&self, 0, sizeof(struct NAME)); \
	for (uint32_t i = 0; tok && i < sizeof(_##NAME) / sizeof(*_##NAME); i++, tok = strtok(NULL, " ")) \
		if (_##NAME[i].DataType != kDataTypeStruct) \
			parse_property(L, tok, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); \
	free(tmp); \
	return (luaX_push##NAME(L, &self), 1); \
} \
int luaopen_orca_##NAME(lua_State *L) { \
	luaL_newmetatable(L, #EXPORT); \
	luaL_setfuncs(L, ((luaL_Reg[]) { \
		{ "new", f_new_##NAME }, \
		{ "fromstring", f_##NAME##___fromstring }, \
		{ "__newindex", f_##NAME##___newindex }, \
		{ "__index", f_##NAME##___index }, \
		{ NULL, NULL }, \
	}), 0); \
	luaL_setfuncs(L, _##NAME##_Methods, 0); \
	/* Make struct creatable via constructor-like syntax */ \
	lua_newtable(L); \
	lua_pushcfunction(L, f_##NAME##___call); \
	lua_setfield(L, -2, "__call"); \
	lua_setmetatable(L, -2); \
	return 1; \
}
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

STRUCT(Keyframe, Keyframe);

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
#define REGISTER_CLASS(NAME, ...) \
ORCA_API struct ClassDesc _##NAME = { \
	.ClassName = #NAME, \
	.DefaultName = #NAME, \
	.ContentType = #NAME, \
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
	.ParentClasses = { __VA_ARGS__ }, \
	.ClassID = ID_##NAME, \
	.ClassSize = sizeof(struct NAME), \
	.Properties = NAME##Properties, \
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
};
#define REGISTER_ATTACH_ONLY_CLASS(NAME, ...) \
ORCA_API struct ClassDesc _##NAME = { \
	.ClassName = #NAME, \
	.DefaultName = #NAME, \
	.ContentType = #NAME, \
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
	.ParentClasses = { __VA_ARGS__ }, \
	.ClassID = ID_##NAME, \
	.ClassSize = sizeof(struct NAME), \
	.Properties = NAME##Properties, \
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
	.IsAttachOnly = TRUE, \
};
static struct PropertyType const AnimationCurveProperties[kAnimationCurveNumProperties] = {
	DECL(0xeb66e456, AnimationCurve, Path, Path, kDataTypeString), // AnimationCurve.Path
	DECL(0x5221f9e8, AnimationCurve, Property, Property, kDataTypeString), // AnimationCurve.Property
	ARRAY_DECL(0xf893ff8e, AnimationCurve, Keyframes, Keyframes, kDataTypeStruct, .TypeString = "Keyframe"), // AnimationCurve.Keyframes
	DECL(0x33ee8bf8, AnimationCurve, NumKeyframes, NumKeyframes, kDataTypeInt), // AnimationCurve.NumKeyframes
};
static struct AnimationCurve AnimationCurveDefaults = {
};
LRESULT AnimationCurveProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
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
	switch (message&MSG_DATA_MASK) {
		case ID_Object_Start&MSG_DATA_MASK: return AnimationClip_Start(object, cmp, wparm, lparm); // Object.Start
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
	switch (message&MSG_DATA_MASK) {
		case ID_Object_Start&MSG_DATA_MASK: return AnimationPlayer_Start(object, cmp, wparm, lparm); // Object.Start
		case ID_Object_Animate&MSG_DATA_MASK: return AnimationPlayer_Animate(object, cmp, wparm, lparm); // Object.Animate
		case ID_AnimationPlayer_Play&MSG_DATA_MASK: return AnimationPlayer_Play(object, cmp, wparm, lparm); // AnimationPlayer.Play
		case ID_AnimationPlayer_Resume&MSG_DATA_MASK: return AnimationPlayer_Resume(object, cmp, wparm, lparm); // AnimationPlayer.Resume
		case ID_AnimationPlayer_Stop&MSG_DATA_MASK: return AnimationPlayer_Stop(object, cmp, wparm, lparm); // AnimationPlayer.Stop
		case ID_AnimationPlayer_Pause&MSG_DATA_MASK: return AnimationPlayer_Pause(object, cmp, wparm, lparm); // AnimationPlayer.Pause
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
	switch (message&MSG_DATA_MASK) {
		case ID_Object_Animate&MSG_DATA_MASK: return PropertyAnimation_Animate(object, cmp, wparm, lparm); // Object.Animate
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
HANDLER(StyleController, Object, Create);
HANDLER(StyleController, Object, Release);
static struct PropertyType const StyleControllerProperties[kStyleControllerNumProperties] = {
};
static struct StyleController StyleControllerDefaults = {
};
LRESULT StyleControllerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Object_Create&MSG_DATA_MASK: return StyleController_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Object_Release&MSG_DATA_MASK: return StyleController_Release(object, cmp, wparm, lparm); // Object.Release
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

ORCA_API int luaopen_orca_core(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ "getFocus", f_core_GetFocus },
		{ "getHover", f_core_GetHover },
		{ NULL, NULL } 
	}));
	lua_setfield(L, ((void)luaopen_orca_Keyframe(L), -2), "Keyframe");
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
	lua_setfield(L, ((void)luaopen_orca_Object(L), -2), "Object");
	lua_setfield(L, ((void)lua_pushclass(L, &_AnimationCurve), -2), "AnimationCurve");
	lua_setfield(L, ((void)lua_pushclass(L, &_AnimationClip), -2), "AnimationClip");
	lua_setfield(L, ((void)lua_pushclass(L, &_AnimationPlayer), -2), "AnimationPlayer");
	lua_setfield(L, ((void)lua_pushclass(L, &_PropertyAnimation), -2), "PropertyAnimation");
	lua_setfield(L, ((void)lua_pushclass(L, &_StyleController), -2), "StyleController");
	void on_core_module_registered(lua_State *L);
	on_core_module_registered(L);
	return 1;
}
