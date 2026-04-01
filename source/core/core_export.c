// Auto-generated from core.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "core.h"

#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }

// WI_Message
extern void luaX_pushWI_Message(lua_State *L, struct WI_Message const* value);
extern struct WI_Message* luaX_checkWI_Message(lua_State *L, int index);
// localization
extern void luaX_pushlocalization(lua_State *L, struct localization const* value);
extern struct localization* luaX_checklocalization(lua_State *L, int index);
// game
extern void luaX_pushgame(lua_State *L, struct game const* value);
extern struct game* luaX_checkgame(lua_State *L, int index);
// Property
extern void luaX_pushProperty(lua_State *L, struct Property const* value);
extern struct Property* luaX_checkProperty(lua_State *L, int index);
// KeyframeAnim
extern void luaX_pushKeyframeAnim(lua_State *L, struct KeyframeAnim const* value);
extern struct KeyframeAnim* luaX_checkKeyframeAnim(lua_State *L, int index);
// lua_State
extern void luaX_pushlua_State(lua_State *L, struct lua_State const* value);
extern struct lua_State* luaX_checklua_State(lua_State *L, int index);

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


int luaopen_orca_Input(lua_State *L) {
	luaL_newmetatable(L, "Input");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ NULL, NULL },
	}), 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	return 1;
}

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
int f_OBJ_DispatchEvent(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* event = luaL_checkstring(L, 2);
	struct Object* result_ = OBJ_DispatchEvent(L, this_, event);
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
int f_OBJ_FindCallbackForID(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	uint32_t event = (uint32_t)luaL_checkinteger(L, 2);
	const char* result_ = OBJ_FindCallbackForID(this_, event);
	lua_pushstring(L, result_);
	return 1;
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
int f_OBJ_Play(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* animation = luaL_checkstring(L, 2);
	OBJ_Play(this_, animation );
	return 0;
}
int f_OBJ_DoTween(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	OBJ_DoTween(L, this_ );
	return 0;
}
int f_OBJ_SetAnimation(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* name = luaL_checkstring(L, 2);
	OBJ_SetAnimation(this_, name );
	return 0;
}
int f_OBJ_GetAnimation(lua_State *L) {
	struct Object const* this_ = luaX_checkObject(L, 1);
	struct KeyframeAnim const* result_ = OBJ_GetAnimation(this_);
	luaX_pushKeyframeAnim(L, result_);
	return 1;
}
int f_OBJ_AddAnimation(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	struct KeyframeAnim* animation = NULL;
	OBJ_AddAnimation(this_, animation );
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
	return OBJ_SetTimer(L, this_);
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
		{ "dispatchEvent", f_OBJ_DispatchEvent },
		{ "postMessage", f_OBJ_PostMessage },
		{ "msgSend", f_OBJ_MsgSend },
		{ "findCallbackForID", f_OBJ_FindCallbackForID },
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
		{ "play", f_OBJ_Play },
		{ "doTween", f_OBJ_DoTween },
		{ "setAnimation", f_OBJ_SetAnimation },
		{ "getAnimation", f_OBJ_GetAnimation },
		{ "addAnimation", f_OBJ_AddAnimation },
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
extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);

#define STRUCT(NAME, EXPORT) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
	if (data == NULL) { lua_pushnil(L); return; } \
	struct NAME* ud = lua_newuserdata(L, sizeof(struct NAME)); \
	luaL_setmetatable(L, #EXPORT); \
	memcpy(ud, data, sizeof(struct NAME)); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { return luaL_checkudata(L, idx, #EXPORT); } \
static int f_new_##NAME(lua_State *L) { \
	struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
	luaL_setmetatable(L, #EXPORT); \
	memset(self, 0, sizeof(struct NAME)); \
	if (lua_istable(L, 1)) \
    for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
			lua_pop(L, (lua_getfield(L, 1, _##NAME[i].Name), read_property(L, -1, &_##NAME[i], ((char*)self)+_##NAME[i].Offset), 1)); \
	else for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		read_property(L, i + 1, &_##NAME[i], ((char*)self)+_##NAME[i].Offset); \
	return 1; \
} \
static int f_##NAME##___index(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (write_property(L, &_##NAME[i], ((char*)luaX_check##NAME(L, 1))+_##NAME[i].Offset), 1); \
	for (uint32_t i = 0; i < sizeof(_##NAME##_Methods) / sizeof(*_##NAME##_Methods); i++) { \
		if (strcmp(_##NAME##_Methods[i].name, luaL_checkstring(L, 2)) == 0) { \
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
			parse_property(tok, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); \
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
static struct PropertyType _MessageType[] = {
	DECL(0x0fe07306, MessageType, Name, name, kDataTypeString), // MessageType.Name
	DECL(0x36e8b900, MessageType, Id, id, kDataTypeInt), // MessageType.Id
	DECL(0xce213309, MessageType, Routing, routing, kDataTypeEnum, .EnumValues = _MessageRouting), // MessageType.Routing
	DECL(0xa6478e7c, MessageType, Size, size, kDataTypeInt), // MessageType.Size
};
static luaL_Reg _MessageType_Methods[] = {
	{ NULL, NULL }
};

STRUCT(MessageType, MessageType);
//struct MessageType MouseMessageMessage = {
//	.name = "MouseMessage",
//	.id = kMsgMouseMessage,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
//struct MessageType KeyMessageMessage = {
//	.name = "KeyMessage",
//	.id = kMsgKeyMessage,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define LeftMouseDownMsgArgs MouseMessageMsgArgs
//struct MessageType LeftMouseDownMessage = {
//	.name = "LeftMouseDown",
//	.id = kMsgLeftMouseDown,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define RightMouseDownMsgArgs MouseMessageMsgArgs
//struct MessageType RightMouseDownMessage = {
//	.name = "RightMouseDown",
//	.id = kMsgRightMouseDown,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define OtherMouseDownMsgArgs MouseMessageMsgArgs
//struct MessageType OtherMouseDownMessage = {
//	.name = "OtherMouseDown",
//	.id = kMsgOtherMouseDown,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define LeftMouseUpMsgArgs MouseMessageMsgArgs
//struct MessageType LeftMouseUpMessage = {
//	.name = "LeftMouseUp",
//	.id = kMsgLeftMouseUp,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define RightMouseUpMsgArgs MouseMessageMsgArgs
//struct MessageType RightMouseUpMessage = {
//	.name = "RightMouseUp",
//	.id = kMsgRightMouseUp,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define OtherMouseUpMsgArgs MouseMessageMsgArgs
//struct MessageType OtherMouseUpMessage = {
//	.name = "OtherMouseUp",
//	.id = kMsgOtherMouseUp,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define LeftMouseDraggedMsgArgs MouseMessageMsgArgs
//struct MessageType LeftMouseDraggedMessage = {
//	.name = "LeftMouseDragged",
//	.id = kMsgLeftMouseDragged,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define RightMouseDraggedMsgArgs MouseMessageMsgArgs
//struct MessageType RightMouseDraggedMessage = {
//	.name = "RightMouseDragged",
//	.id = kMsgRightMouseDragged,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define OtherMouseDraggedMsgArgs MouseMessageMsgArgs
//struct MessageType OtherMouseDraggedMessage = {
//	.name = "OtherMouseDragged",
//	.id = kMsgOtherMouseDragged,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define LeftDoubleClickMsgArgs MouseMessageMsgArgs
//struct MessageType LeftDoubleClickMessage = {
//	.name = "LeftDoubleClick",
//	.id = kMsgLeftDoubleClick,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define RightDoubleClickMsgArgs MouseMessageMsgArgs
//struct MessageType RightDoubleClickMessage = {
//	.name = "RightDoubleClick",
//	.id = kMsgRightDoubleClick,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define OtherDoubleClickMsgArgs MouseMessageMsgArgs
//struct MessageType OtherDoubleClickMessage = {
//	.name = "OtherDoubleClick",
//	.id = kMsgOtherDoubleClick,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define MouseMovedMsgArgs MouseMessageMsgArgs
//struct MessageType MouseMovedMessage = {
//	.name = "MouseMoved",
//	.id = kMsgMouseMoved,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define ScrollWheelMsgArgs MouseMessageMsgArgs
//struct MessageType ScrollWheelMessage = {
//	.name = "ScrollWheel",
//	.id = kMsgScrollWheel,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define DragDropMsgArgs MouseMessageMsgArgs
//struct MessageType DragDropMessage = {
//	.name = "DragDrop",
//	.id = kMsgDragDrop,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define DragEnterMsgArgs MouseMessageMsgArgs
//struct MessageType DragEnterMessage = {
//	.name = "DragEnter",
//	.id = kMsgDragEnter,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define KeyDownMsgArgs KeyMessageMsgArgs
//struct MessageType KeyDownMessage = {
//	.name = "KeyDown",
//	.id = kMsgKeyDown,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define KeyUpMsgArgs KeyMessageMsgArgs
//struct MessageType KeyUpMessage = {
//	.name = "KeyUp",
//	.id = kMsgKeyUp,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
#define CharMsgArgs KeyMessageMsgArgs
//struct MessageType CharMessage = {
//	.name = "Char",
//	.id = kMsgChar,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct WI_Message),
//};
//struct MessageType CreateMessage = {
//	.name = "Create",
//	.id = kMsgCreate,
//	.routing = kMessageRoutingDirect,
//	.size = sizeof(struct CreateMsgArgs),
//};
//struct MessageType StartMessage = {
//	.name = "Start",
//	.id = kMsgStart,
//	.routing = kMessageRoutingDirect,
//	.size = sizeof(struct StartMsgArgs),
//};
//struct MessageType AwakeMessage = {
//	.name = "Awake",
//	.id = kMsgAwake,
//	.routing = kMessageRoutingDirect,
//	.size = sizeof(struct AwakeMsgArgs),
//};
//struct MessageType ThemeChangedMessage = {
//	.name = "ThemeChanged",
//	.id = kMsgThemeChanged,
//	.routing = kMessageRoutingDirect,
//	.size = sizeof(struct ThemeChangedMsgArgs),
//};
//struct MessageType PropertyChangedMessage = {
//	.name = "PropertyChanged",
//	.id = kMsgPropertyChanged,
//	.routing = kMessageRoutingDirect,
//	.size = sizeof(struct PropertyChangedMsgArgs),
//};
//struct MessageType AttachedMessage = {
//	.name = "Attached",
//	.id = kMsgAttached,
//	.routing = kMessageRoutingDirect,
//	.size = sizeof(struct AttachedMsgArgs),
//};
//struct MessageType ReleaseMessage = {
//	.name = "Release",
//	.id = kMsgRelease,
//	.routing = kMessageRoutingDirect,
//	.size = sizeof(struct ReleaseMsgArgs),
//};
//struct MessageType DestroyMessage = {
//	.name = "Destroy",
//	.id = kMsgDestroy,
//	.routing = kMessageRoutingDirect,
//	.size = sizeof(struct DestroyMsgArgs),
//};
//struct MessageType TimerMessage = {
//	.name = "Timer",
//	.id = kMsgTimer,
//	.routing = kMessageRoutingTunnelingBubbling,
//	.size = sizeof(struct TimerMsgArgs),
//};
//struct MessageType ResumeCoroutineMessage = {
//	.name = "ResumeCoroutine",
//	.id = kMsgResumeCoroutine,
//	.routing = kMessageRoutingDirect,
//	.size = sizeof(struct ResumeCoroutineMsgArgs),
//};
//struct MessageType StopCoroutineMessage = {
//	.name = "StopCoroutine",
//	.id = kMsgStopCoroutine,
//	.routing = kMessageRoutingDirect,
//	.size = sizeof(struct StopCoroutineMsgArgs),
//};

static luaL_Reg _MouseMessageMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _MouseMessageMsgArgs[] = {
};
static luaL_Reg _KeyMessageMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _KeyMessageMsgArgs[] = {
};
static luaL_Reg _LeftMouseDownMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _LeftMouseDownMsgArgs[] = {
};
static luaL_Reg _RightMouseDownMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _RightMouseDownMsgArgs[] = {
};
static luaL_Reg _OtherMouseDownMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _OtherMouseDownMsgArgs[] = {
};
static luaL_Reg _LeftMouseUpMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _LeftMouseUpMsgArgs[] = {
};
static luaL_Reg _RightMouseUpMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _RightMouseUpMsgArgs[] = {
};
static luaL_Reg _OtherMouseUpMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _OtherMouseUpMsgArgs[] = {
};
static luaL_Reg _LeftMouseDraggedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _LeftMouseDraggedMsgArgs[] = {
};
static luaL_Reg _RightMouseDraggedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _RightMouseDraggedMsgArgs[] = {
};
static luaL_Reg _OtherMouseDraggedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _OtherMouseDraggedMsgArgs[] = {
};
static luaL_Reg _LeftDoubleClickMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _LeftDoubleClickMsgArgs[] = {
};
static luaL_Reg _RightDoubleClickMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _RightDoubleClickMsgArgs[] = {
};
static luaL_Reg _OtherDoubleClickMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _OtherDoubleClickMsgArgs[] = {
};
static luaL_Reg _MouseMovedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _MouseMovedMsgArgs[] = {
};
static luaL_Reg _ScrollWheelMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _ScrollWheelMsgArgs[] = {
};
static luaL_Reg _DragDropMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _DragDropMsgArgs[] = {
};
static luaL_Reg _DragEnterMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _DragEnterMsgArgs[] = {
};
static luaL_Reg _KeyDownMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _KeyDownMsgArgs[] = {
};
static luaL_Reg _KeyUpMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _KeyUpMsgArgs[] = {
};
static luaL_Reg _CharMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _CharMsgArgs[] = {
};
static luaL_Reg _CreateMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _CreateMsgArgs[] = {
};
static luaL_Reg _StartMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _StartMsgArgs[] = {
};
static luaL_Reg _AwakeMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _AwakeMsgArgs[] = {
};
static luaL_Reg _ThemeChangedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _ThemeChangedMsgArgs[] = {
};
static luaL_Reg _PropertyChangedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _PropertyChangedMsgArgs[] = {
	DECL(0x5221f9e8, PropertyChangedMsgArgs, Property, Property, kDataTypeStruct, .TypeString = "Property"), // PropertyChangedMsgArgs.Property
};
static luaL_Reg _AttachedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _AttachedMsgArgs[] = {
};
static luaL_Reg _ReleaseMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _ReleaseMsgArgs[] = {
};
static luaL_Reg _DestroyMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _DestroyMsgArgs[] = {
};
static luaL_Reg _TimerMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _TimerMsgArgs[] = {
};
static luaL_Reg _ResumeCoroutineMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _ResumeCoroutineMsgArgs[] = {
};
static luaL_Reg _StopCoroutineMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _StopCoroutineMsgArgs[] = {
};

STRUCT(MouseMessageMsgArgs, MouseMessageMsgArgs);
STRUCT(KeyMessageMsgArgs, KeyMessageMsgArgs);
STRUCT(LeftMouseDownMsgArgs, LeftMouseDownMsgArgs);
STRUCT(RightMouseDownMsgArgs, RightMouseDownMsgArgs);
STRUCT(OtherMouseDownMsgArgs, OtherMouseDownMsgArgs);
STRUCT(LeftMouseUpMsgArgs, LeftMouseUpMsgArgs);
STRUCT(RightMouseUpMsgArgs, RightMouseUpMsgArgs);
STRUCT(OtherMouseUpMsgArgs, OtherMouseUpMsgArgs);
STRUCT(LeftMouseDraggedMsgArgs, LeftMouseDraggedMsgArgs);
STRUCT(RightMouseDraggedMsgArgs, RightMouseDraggedMsgArgs);
STRUCT(OtherMouseDraggedMsgArgs, OtherMouseDraggedMsgArgs);
STRUCT(LeftDoubleClickMsgArgs, LeftDoubleClickMsgArgs);
STRUCT(RightDoubleClickMsgArgs, RightDoubleClickMsgArgs);
STRUCT(OtherDoubleClickMsgArgs, OtherDoubleClickMsgArgs);
STRUCT(MouseMovedMsgArgs, MouseMovedMsgArgs);
STRUCT(ScrollWheelMsgArgs, ScrollWheelMsgArgs);
STRUCT(DragDropMsgArgs, DragDropMsgArgs);
STRUCT(DragEnterMsgArgs, DragEnterMsgArgs);
STRUCT(KeyDownMsgArgs, KeyDownMsgArgs);
STRUCT(KeyUpMsgArgs, KeyUpMsgArgs);
STRUCT(CharMsgArgs, CharMsgArgs);
STRUCT(CreateMsgArgs, CreateMsgArgs);
STRUCT(StartMsgArgs, StartMsgArgs);
STRUCT(AwakeMsgArgs, AwakeMsgArgs);
STRUCT(ThemeChangedMsgArgs, ThemeChangedMsgArgs);
STRUCT(PropertyChangedMsgArgs, PropertyChangedMsgArgs);
STRUCT(AttachedMsgArgs, AttachedMsgArgs);
STRUCT(ReleaseMsgArgs, ReleaseMsgArgs);
STRUCT(DestroyMsgArgs, DestroyMsgArgs);
STRUCT(TimerMsgArgs, TimerMsgArgs);
STRUCT(ResumeCoroutineMsgArgs, ResumeCoroutineMsgArgs);
STRUCT(StopCoroutineMsgArgs, StopCoroutineMsgArgs);
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
	.MessageTypes = NAME##MessageTypes, \
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
	.NumMessageTypes = k##NAME##NumMessageTypes, \
};

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
	lua_setfield(L, ((void)luaopen_orca_MessageType(L), -2), "MessageType");
	lua_setfield(L, ((void)luaopen_orca_MouseMessageMsgArgs(L), -2), "MouseMessageMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_KeyMessageMsgArgs(L), -2), "KeyMessageMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_LeftMouseDownMsgArgs(L), -2), "LeftMouseDownMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_RightMouseDownMsgArgs(L), -2), "RightMouseDownMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_OtherMouseDownMsgArgs(L), -2), "OtherMouseDownMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_LeftMouseUpMsgArgs(L), -2), "LeftMouseUpMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_RightMouseUpMsgArgs(L), -2), "RightMouseUpMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_OtherMouseUpMsgArgs(L), -2), "OtherMouseUpMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_LeftMouseDraggedMsgArgs(L), -2), "LeftMouseDraggedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_RightMouseDraggedMsgArgs(L), -2), "RightMouseDraggedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_OtherMouseDraggedMsgArgs(L), -2), "OtherMouseDraggedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_LeftDoubleClickMsgArgs(L), -2), "LeftDoubleClickMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_RightDoubleClickMsgArgs(L), -2), "RightDoubleClickMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_OtherDoubleClickMsgArgs(L), -2), "OtherDoubleClickMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_MouseMovedMsgArgs(L), -2), "MouseMovedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_ScrollWheelMsgArgs(L), -2), "ScrollWheelMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_DragDropMsgArgs(L), -2), "DragDropMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_DragEnterMsgArgs(L), -2), "DragEnterMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_KeyDownMsgArgs(L), -2), "KeyDownMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_KeyUpMsgArgs(L), -2), "KeyUpMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_CharMsgArgs(L), -2), "CharMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_CreateMsgArgs(L), -2), "CreateMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_StartMsgArgs(L), -2), "StartMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_AwakeMsgArgs(L), -2), "AwakeMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_ThemeChangedMsgArgs(L), -2), "ThemeChangedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_PropertyChangedMsgArgs(L), -2), "PropertyChangedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_AttachedMsgArgs(L), -2), "AttachedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_ReleaseMsgArgs(L), -2), "ReleaseMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_DestroyMsgArgs(L), -2), "DestroyMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_TimerMsgArgs(L), -2), "TimerMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_ResumeCoroutineMsgArgs(L), -2), "ResumeCoroutineMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_StopCoroutineMsgArgs(L), -2), "StopCoroutineMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Input(L), -2), "Input");
	lua_setfield(L, ((void)luaopen_orca_Object(L), -2), "Object");
	void on_core_module_registered(lua_State *L);
	on_core_module_registered(L);
	return 1;
}
