// Auto-generated from core.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "core.h"

#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }

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
ENUM(MouseButton, "None", "Left", "Right", "Middle")


int luaopen_orca_Mouse(lua_State *L) {
	luaL_newmetatable(L, "Input");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ NULL, NULL },
	}), 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	return 1;
}


int luaopen_orca_Keyboard(lua_State *L) {
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
	/* Register the struct in the Lua registry */ \
	lua_pushlightuserdata(L, (void*)(intptr_t)ID_##NAME); \
	lua_pushvalue(L, -2); \
	lua_settable(L, LUA_REGISTRYINDEX); \
	/* Make struct creatable via constructor-like syntax */ \
	lua_newtable(L); \
	lua_pushcfunction(L, f_##NAME##___call); \
	lua_setfield(L, -2, "__call"); \
	lua_setmetatable(L, -2); \
	return 1; \
}
static struct PropertyType _MessageType[] = {
	DECL(0x0fe07306, MessageType, Name, Name, kDataTypeString), // MessageType.Name
	DECL(0x429417cf, MessageType, FullIdentifier, FullIdentifier, kDataTypeInt), // MessageType.FullIdentifier
	DECL(0x0f76864e, MessageType, ShortIdentifier, ShortIdentifier, kDataTypeInt), // MessageType.ShortIdentifier
	DECL(0xce213309, MessageType, Routing, Routing, kDataTypeEnum, .EnumValues = _MessageRouting), // MessageType.Routing
	DECL(0xa6478e7c, MessageType, Size, Size, kDataTypeInt), // MessageType.Size
};
static luaL_Reg _MessageType_Methods[] = {
	{ NULL, NULL }
};

STRUCT(MessageType, MessageType);
#define Mouse_LeftMouseDownMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_RightMouseDownMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_OtherMouseDownMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_LeftMouseUpMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_RightMouseUpMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_OtherMouseUpMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_LeftMouseDraggedMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_RightMouseDraggedMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_OtherMouseDraggedMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_LeftDoubleClickMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_RightDoubleClickMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_OtherDoubleClickMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_MouseMovedMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_ScrollWheelMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_DragDropMsgArgs Mouse_MouseMessageMsgArgs
#define Mouse_DragEnterMsgArgs Mouse_MouseMessageMsgArgs
#define Keyboard_KeyDownMsgArgs Keyboard_KeyMessageMsgArgs
#define Keyboard_KeyUpMsgArgs Keyboard_KeyMessageMsgArgs
#define Keyboard_TextInputMsgArgs Keyboard_KeyMessageMsgArgs

static luaL_Reg _Mouse_MouseMessageMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_MouseMessageMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_MouseMessageMsgArgs, x, x, kDataTypeFloat), // Mouse_MouseMessageMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_MouseMessageMsgArgs, y, y, kDataTypeFloat), // Mouse_MouseMessageMsgArgs.y
	DECL(0xec56f27b, Mouse_MouseMessageMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_MouseMessageMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_MouseMessageMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_MouseMessageMsgArgs.deltaY
	DECL(0x43b27471, Mouse_MouseMessageMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_MouseMessageMsgArgs.button
	DECL(0xd9747336, Mouse_MouseMessageMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_MouseMessageMsgArgs.clickCount
};
static luaL_Reg _Mouse_LeftMouseDownMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_LeftMouseDownMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_LeftMouseDownMsgArgs, x, x, kDataTypeFloat), // Mouse_LeftMouseDownMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_LeftMouseDownMsgArgs, y, y, kDataTypeFloat), // Mouse_LeftMouseDownMsgArgs.y
	DECL(0xec56f27b, Mouse_LeftMouseDownMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_LeftMouseDownMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_LeftMouseDownMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_LeftMouseDownMsgArgs.deltaY
	DECL(0x43b27471, Mouse_LeftMouseDownMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_LeftMouseDownMsgArgs.button
	DECL(0xd9747336, Mouse_LeftMouseDownMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_LeftMouseDownMsgArgs.clickCount
};
static luaL_Reg _Mouse_RightMouseDownMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_RightMouseDownMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_RightMouseDownMsgArgs, x, x, kDataTypeFloat), // Mouse_RightMouseDownMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_RightMouseDownMsgArgs, y, y, kDataTypeFloat), // Mouse_RightMouseDownMsgArgs.y
	DECL(0xec56f27b, Mouse_RightMouseDownMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_RightMouseDownMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_RightMouseDownMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_RightMouseDownMsgArgs.deltaY
	DECL(0x43b27471, Mouse_RightMouseDownMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_RightMouseDownMsgArgs.button
	DECL(0xd9747336, Mouse_RightMouseDownMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_RightMouseDownMsgArgs.clickCount
};
static luaL_Reg _Mouse_OtherMouseDownMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_OtherMouseDownMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_OtherMouseDownMsgArgs, x, x, kDataTypeFloat), // Mouse_OtherMouseDownMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_OtherMouseDownMsgArgs, y, y, kDataTypeFloat), // Mouse_OtherMouseDownMsgArgs.y
	DECL(0xec56f27b, Mouse_OtherMouseDownMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_OtherMouseDownMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_OtherMouseDownMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_OtherMouseDownMsgArgs.deltaY
	DECL(0x43b27471, Mouse_OtherMouseDownMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_OtherMouseDownMsgArgs.button
	DECL(0xd9747336, Mouse_OtherMouseDownMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_OtherMouseDownMsgArgs.clickCount
};
static luaL_Reg _Mouse_LeftMouseUpMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_LeftMouseUpMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_LeftMouseUpMsgArgs, x, x, kDataTypeFloat), // Mouse_LeftMouseUpMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_LeftMouseUpMsgArgs, y, y, kDataTypeFloat), // Mouse_LeftMouseUpMsgArgs.y
	DECL(0xec56f27b, Mouse_LeftMouseUpMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_LeftMouseUpMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_LeftMouseUpMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_LeftMouseUpMsgArgs.deltaY
	DECL(0x43b27471, Mouse_LeftMouseUpMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_LeftMouseUpMsgArgs.button
	DECL(0xd9747336, Mouse_LeftMouseUpMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_LeftMouseUpMsgArgs.clickCount
};
static luaL_Reg _Mouse_RightMouseUpMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_RightMouseUpMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_RightMouseUpMsgArgs, x, x, kDataTypeFloat), // Mouse_RightMouseUpMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_RightMouseUpMsgArgs, y, y, kDataTypeFloat), // Mouse_RightMouseUpMsgArgs.y
	DECL(0xec56f27b, Mouse_RightMouseUpMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_RightMouseUpMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_RightMouseUpMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_RightMouseUpMsgArgs.deltaY
	DECL(0x43b27471, Mouse_RightMouseUpMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_RightMouseUpMsgArgs.button
	DECL(0xd9747336, Mouse_RightMouseUpMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_RightMouseUpMsgArgs.clickCount
};
static luaL_Reg _Mouse_OtherMouseUpMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_OtherMouseUpMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_OtherMouseUpMsgArgs, x, x, kDataTypeFloat), // Mouse_OtherMouseUpMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_OtherMouseUpMsgArgs, y, y, kDataTypeFloat), // Mouse_OtherMouseUpMsgArgs.y
	DECL(0xec56f27b, Mouse_OtherMouseUpMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_OtherMouseUpMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_OtherMouseUpMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_OtherMouseUpMsgArgs.deltaY
	DECL(0x43b27471, Mouse_OtherMouseUpMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_OtherMouseUpMsgArgs.button
	DECL(0xd9747336, Mouse_OtherMouseUpMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_OtherMouseUpMsgArgs.clickCount
};
static luaL_Reg _Mouse_LeftMouseDraggedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_LeftMouseDraggedMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_LeftMouseDraggedMsgArgs, x, x, kDataTypeFloat), // Mouse_LeftMouseDraggedMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_LeftMouseDraggedMsgArgs, y, y, kDataTypeFloat), // Mouse_LeftMouseDraggedMsgArgs.y
	DECL(0xec56f27b, Mouse_LeftMouseDraggedMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_LeftMouseDraggedMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_LeftMouseDraggedMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_LeftMouseDraggedMsgArgs.deltaY
	DECL(0x43b27471, Mouse_LeftMouseDraggedMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_LeftMouseDraggedMsgArgs.button
	DECL(0xd9747336, Mouse_LeftMouseDraggedMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_LeftMouseDraggedMsgArgs.clickCount
};
static luaL_Reg _Mouse_RightMouseDraggedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_RightMouseDraggedMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_RightMouseDraggedMsgArgs, x, x, kDataTypeFloat), // Mouse_RightMouseDraggedMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_RightMouseDraggedMsgArgs, y, y, kDataTypeFloat), // Mouse_RightMouseDraggedMsgArgs.y
	DECL(0xec56f27b, Mouse_RightMouseDraggedMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_RightMouseDraggedMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_RightMouseDraggedMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_RightMouseDraggedMsgArgs.deltaY
	DECL(0x43b27471, Mouse_RightMouseDraggedMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_RightMouseDraggedMsgArgs.button
	DECL(0xd9747336, Mouse_RightMouseDraggedMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_RightMouseDraggedMsgArgs.clickCount
};
static luaL_Reg _Mouse_OtherMouseDraggedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_OtherMouseDraggedMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_OtherMouseDraggedMsgArgs, x, x, kDataTypeFloat), // Mouse_OtherMouseDraggedMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_OtherMouseDraggedMsgArgs, y, y, kDataTypeFloat), // Mouse_OtherMouseDraggedMsgArgs.y
	DECL(0xec56f27b, Mouse_OtherMouseDraggedMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_OtherMouseDraggedMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_OtherMouseDraggedMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_OtherMouseDraggedMsgArgs.deltaY
	DECL(0x43b27471, Mouse_OtherMouseDraggedMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_OtherMouseDraggedMsgArgs.button
	DECL(0xd9747336, Mouse_OtherMouseDraggedMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_OtherMouseDraggedMsgArgs.clickCount
};
static luaL_Reg _Mouse_LeftDoubleClickMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_LeftDoubleClickMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_LeftDoubleClickMsgArgs, x, x, kDataTypeFloat), // Mouse_LeftDoubleClickMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_LeftDoubleClickMsgArgs, y, y, kDataTypeFloat), // Mouse_LeftDoubleClickMsgArgs.y
	DECL(0xec56f27b, Mouse_LeftDoubleClickMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_LeftDoubleClickMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_LeftDoubleClickMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_LeftDoubleClickMsgArgs.deltaY
	DECL(0x43b27471, Mouse_LeftDoubleClickMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_LeftDoubleClickMsgArgs.button
	DECL(0xd9747336, Mouse_LeftDoubleClickMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_LeftDoubleClickMsgArgs.clickCount
};
static luaL_Reg _Mouse_RightDoubleClickMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_RightDoubleClickMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_RightDoubleClickMsgArgs, x, x, kDataTypeFloat), // Mouse_RightDoubleClickMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_RightDoubleClickMsgArgs, y, y, kDataTypeFloat), // Mouse_RightDoubleClickMsgArgs.y
	DECL(0xec56f27b, Mouse_RightDoubleClickMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_RightDoubleClickMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_RightDoubleClickMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_RightDoubleClickMsgArgs.deltaY
	DECL(0x43b27471, Mouse_RightDoubleClickMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_RightDoubleClickMsgArgs.button
	DECL(0xd9747336, Mouse_RightDoubleClickMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_RightDoubleClickMsgArgs.clickCount
};
static luaL_Reg _Mouse_OtherDoubleClickMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_OtherDoubleClickMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_OtherDoubleClickMsgArgs, x, x, kDataTypeFloat), // Mouse_OtherDoubleClickMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_OtherDoubleClickMsgArgs, y, y, kDataTypeFloat), // Mouse_OtherDoubleClickMsgArgs.y
	DECL(0xec56f27b, Mouse_OtherDoubleClickMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_OtherDoubleClickMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_OtherDoubleClickMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_OtherDoubleClickMsgArgs.deltaY
	DECL(0x43b27471, Mouse_OtherDoubleClickMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_OtherDoubleClickMsgArgs.button
	DECL(0xd9747336, Mouse_OtherDoubleClickMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_OtherDoubleClickMsgArgs.clickCount
};
static luaL_Reg _Mouse_MouseMovedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_MouseMovedMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_MouseMovedMsgArgs, x, x, kDataTypeFloat), // Mouse_MouseMovedMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_MouseMovedMsgArgs, y, y, kDataTypeFloat), // Mouse_MouseMovedMsgArgs.y
	DECL(0xec56f27b, Mouse_MouseMovedMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_MouseMovedMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_MouseMovedMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_MouseMovedMsgArgs.deltaY
	DECL(0x43b27471, Mouse_MouseMovedMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_MouseMovedMsgArgs.button
	DECL(0xd9747336, Mouse_MouseMovedMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_MouseMovedMsgArgs.clickCount
};
static luaL_Reg _Mouse_ScrollWheelMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_ScrollWheelMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_ScrollWheelMsgArgs, x, x, kDataTypeFloat), // Mouse_ScrollWheelMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_ScrollWheelMsgArgs, y, y, kDataTypeFloat), // Mouse_ScrollWheelMsgArgs.y
	DECL(0xec56f27b, Mouse_ScrollWheelMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_ScrollWheelMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_ScrollWheelMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_ScrollWheelMsgArgs.deltaY
	DECL(0x43b27471, Mouse_ScrollWheelMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_ScrollWheelMsgArgs.button
	DECL(0xd9747336, Mouse_ScrollWheelMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_ScrollWheelMsgArgs.clickCount
};
static luaL_Reg _Mouse_DragDropMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_DragDropMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_DragDropMsgArgs, x, x, kDataTypeFloat), // Mouse_DragDropMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_DragDropMsgArgs, y, y, kDataTypeFloat), // Mouse_DragDropMsgArgs.y
	DECL(0xec56f27b, Mouse_DragDropMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_DragDropMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_DragDropMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_DragDropMsgArgs.deltaY
	DECL(0x43b27471, Mouse_DragDropMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_DragDropMsgArgs.button
	DECL(0xd9747336, Mouse_DragDropMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_DragDropMsgArgs.clickCount
};
static luaL_Reg _Mouse_DragEnterMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Mouse_DragEnterMsgArgs[] = {
	DECL(0xfd0c5087, Mouse_DragEnterMsgArgs, x, x, kDataTypeFloat), // Mouse_DragEnterMsgArgs.x
	DECL(0xfc0c4ef4, Mouse_DragEnterMsgArgs, y, y, kDataTypeFloat), // Mouse_DragEnterMsgArgs.y
	DECL(0xec56f27b, Mouse_DragEnterMsgArgs, deltaX, deltaX, kDataTypeInt), // Mouse_DragEnterMsgArgs.deltaX
	DECL(0xeb56f0e8, Mouse_DragEnterMsgArgs, deltaY, deltaY, kDataTypeInt), // Mouse_DragEnterMsgArgs.deltaY
	DECL(0x43b27471, Mouse_DragEnterMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Mouse_DragEnterMsgArgs.button
	DECL(0xd9747336, Mouse_DragEnterMsgArgs, clickCount, clickCount, kDataTypeInt), // Mouse_DragEnterMsgArgs.clickCount
};
static luaL_Reg _Keyboard_KeyMessageMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Keyboard_KeyMessageMsgArgs[] = {
	DECL(0xd803e2a9, Keyboard_KeyMessageMsgArgs, keyCode, keyCode, kDataTypeInt), // Keyboard_KeyMessageMsgArgs.keyCode
	DECL(0x8b3aa710, Keyboard_KeyMessageMsgArgs, character, character, kDataTypeInt), // Keyboard_KeyMessageMsgArgs.character
	DECL(0x411b1019, Keyboard_KeyMessageMsgArgs, modifiers, modifiers, kDataTypeInt), // Keyboard_KeyMessageMsgArgs.modifiers
	DECL(0xbde64e3e, Keyboard_KeyMessageMsgArgs, text, text, kDataTypeString), // Keyboard_KeyMessageMsgArgs.text
	DECL(0x8f82be34, Keyboard_KeyMessageMsgArgs, modifiersString, modifiersString, kDataTypeString), // Keyboard_KeyMessageMsgArgs.modifiersString
	DECL(0x9703dc13, Keyboard_KeyMessageMsgArgs, hotKey, hotKey, kDataTypeString), // Keyboard_KeyMessageMsgArgs.hotKey
};
static luaL_Reg _Keyboard_KeyDownMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Keyboard_KeyDownMsgArgs[] = {
	DECL(0xd803e2a9, Keyboard_KeyDownMsgArgs, keyCode, keyCode, kDataTypeInt), // Keyboard_KeyDownMsgArgs.keyCode
	DECL(0x8b3aa710, Keyboard_KeyDownMsgArgs, character, character, kDataTypeInt), // Keyboard_KeyDownMsgArgs.character
	DECL(0x411b1019, Keyboard_KeyDownMsgArgs, modifiers, modifiers, kDataTypeInt), // Keyboard_KeyDownMsgArgs.modifiers
	DECL(0xbde64e3e, Keyboard_KeyDownMsgArgs, text, text, kDataTypeString), // Keyboard_KeyDownMsgArgs.text
	DECL(0x8f82be34, Keyboard_KeyDownMsgArgs, modifiersString, modifiersString, kDataTypeString), // Keyboard_KeyDownMsgArgs.modifiersString
	DECL(0x9703dc13, Keyboard_KeyDownMsgArgs, hotKey, hotKey, kDataTypeString), // Keyboard_KeyDownMsgArgs.hotKey
};
static luaL_Reg _Keyboard_KeyUpMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Keyboard_KeyUpMsgArgs[] = {
	DECL(0xd803e2a9, Keyboard_KeyUpMsgArgs, keyCode, keyCode, kDataTypeInt), // Keyboard_KeyUpMsgArgs.keyCode
	DECL(0x8b3aa710, Keyboard_KeyUpMsgArgs, character, character, kDataTypeInt), // Keyboard_KeyUpMsgArgs.character
	DECL(0x411b1019, Keyboard_KeyUpMsgArgs, modifiers, modifiers, kDataTypeInt), // Keyboard_KeyUpMsgArgs.modifiers
	DECL(0xbde64e3e, Keyboard_KeyUpMsgArgs, text, text, kDataTypeString), // Keyboard_KeyUpMsgArgs.text
	DECL(0x8f82be34, Keyboard_KeyUpMsgArgs, modifiersString, modifiersString, kDataTypeString), // Keyboard_KeyUpMsgArgs.modifiersString
	DECL(0x9703dc13, Keyboard_KeyUpMsgArgs, hotKey, hotKey, kDataTypeString), // Keyboard_KeyUpMsgArgs.hotKey
};
static luaL_Reg _Keyboard_TextInputMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Keyboard_TextInputMsgArgs[] = {
	DECL(0xd803e2a9, Keyboard_TextInputMsgArgs, keyCode, keyCode, kDataTypeInt), // Keyboard_TextInputMsgArgs.keyCode
	DECL(0x8b3aa710, Keyboard_TextInputMsgArgs, character, character, kDataTypeInt), // Keyboard_TextInputMsgArgs.character
	DECL(0x411b1019, Keyboard_TextInputMsgArgs, modifiers, modifiers, kDataTypeInt), // Keyboard_TextInputMsgArgs.modifiers
	DECL(0xbde64e3e, Keyboard_TextInputMsgArgs, text, text, kDataTypeString), // Keyboard_TextInputMsgArgs.text
	DECL(0x8f82be34, Keyboard_TextInputMsgArgs, modifiersString, modifiersString, kDataTypeString), // Keyboard_TextInputMsgArgs.modifiersString
	DECL(0x9703dc13, Keyboard_TextInputMsgArgs, hotKey, hotKey, kDataTypeString), // Keyboard_TextInputMsgArgs.hotKey
};
static luaL_Reg _Object_CreateMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_CreateMsgArgs[] = {
};
static luaL_Reg _Object_StartMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_StartMsgArgs[] = {
};
static luaL_Reg _Object_AwakeMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_AwakeMsgArgs[] = {
};
static luaL_Reg _Object_ThemeChangedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_ThemeChangedMsgArgs[] = {
};
static luaL_Reg _Object_PropertyChangedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_PropertyChangedMsgArgs[] = {
	DECL(0x5221f9e8, Object_PropertyChangedMsgArgs, Property, Property, kDataTypeStruct, .TypeString = "Property"), // Object_PropertyChangedMsgArgs.Property
};
static luaL_Reg _Object_AttachedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_AttachedMsgArgs[] = {
};
static luaL_Reg _Object_ReleaseMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_ReleaseMsgArgs[] = {
};
static luaL_Reg _Object_DestroyMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_DestroyMsgArgs[] = {
};
static luaL_Reg _Object_TimerMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Object_TimerMsgArgs[] = {
};

STRUCT(Mouse_MouseMessageMsgArgs, Mouse_MouseMessageMsgArgs);
STRUCT(Mouse_LeftMouseDownMsgArgs, Mouse_LeftMouseDownMsgArgs);
STRUCT(Mouse_RightMouseDownMsgArgs, Mouse_RightMouseDownMsgArgs);
STRUCT(Mouse_OtherMouseDownMsgArgs, Mouse_OtherMouseDownMsgArgs);
STRUCT(Mouse_LeftMouseUpMsgArgs, Mouse_LeftMouseUpMsgArgs);
STRUCT(Mouse_RightMouseUpMsgArgs, Mouse_RightMouseUpMsgArgs);
STRUCT(Mouse_OtherMouseUpMsgArgs, Mouse_OtherMouseUpMsgArgs);
STRUCT(Mouse_LeftMouseDraggedMsgArgs, Mouse_LeftMouseDraggedMsgArgs);
STRUCT(Mouse_RightMouseDraggedMsgArgs, Mouse_RightMouseDraggedMsgArgs);
STRUCT(Mouse_OtherMouseDraggedMsgArgs, Mouse_OtherMouseDraggedMsgArgs);
STRUCT(Mouse_LeftDoubleClickMsgArgs, Mouse_LeftDoubleClickMsgArgs);
STRUCT(Mouse_RightDoubleClickMsgArgs, Mouse_RightDoubleClickMsgArgs);
STRUCT(Mouse_OtherDoubleClickMsgArgs, Mouse_OtherDoubleClickMsgArgs);
STRUCT(Mouse_MouseMovedMsgArgs, Mouse_MouseMovedMsgArgs);
STRUCT(Mouse_ScrollWheelMsgArgs, Mouse_ScrollWheelMsgArgs);
STRUCT(Mouse_DragDropMsgArgs, Mouse_DragDropMsgArgs);
STRUCT(Mouse_DragEnterMsgArgs, Mouse_DragEnterMsgArgs);
STRUCT(Keyboard_KeyMessageMsgArgs, Keyboard_KeyMessageMsgArgs);
STRUCT(Keyboard_KeyDownMsgArgs, Keyboard_KeyDownMsgArgs);
STRUCT(Keyboard_KeyUpMsgArgs, Keyboard_KeyUpMsgArgs);
STRUCT(Keyboard_TextInputMsgArgs, Keyboard_TextInputMsgArgs);
STRUCT(Object_CreateMsgArgs, Object_CreateMsgArgs);
STRUCT(Object_StartMsgArgs, Object_StartMsgArgs);
STRUCT(Object_AwakeMsgArgs, Object_AwakeMsgArgs);
STRUCT(Object_ThemeChangedMsgArgs, Object_ThemeChangedMsgArgs);
STRUCT(Object_PropertyChangedMsgArgs, Object_PropertyChangedMsgArgs);
STRUCT(Object_AttachedMsgArgs, Object_AttachedMsgArgs);
STRUCT(Object_ReleaseMsgArgs, Object_ReleaseMsgArgs);
STRUCT(Object_DestroyMsgArgs, Object_DestroyMsgArgs);
STRUCT(Object_TimerMsgArgs, Object_TimerMsgArgs);
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
	lua_setfield(L, ((void)luaopen_orca_Mouse_MouseMessageMsgArgs(L), -2), "Mouse_MouseMessageMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_LeftMouseDownMsgArgs(L), -2), "Mouse_LeftMouseDownMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_RightMouseDownMsgArgs(L), -2), "Mouse_RightMouseDownMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_OtherMouseDownMsgArgs(L), -2), "Mouse_OtherMouseDownMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_LeftMouseUpMsgArgs(L), -2), "Mouse_LeftMouseUpMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_RightMouseUpMsgArgs(L), -2), "Mouse_RightMouseUpMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_OtherMouseUpMsgArgs(L), -2), "Mouse_OtherMouseUpMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_LeftMouseDraggedMsgArgs(L), -2), "Mouse_LeftMouseDraggedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_RightMouseDraggedMsgArgs(L), -2), "Mouse_RightMouseDraggedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_OtherMouseDraggedMsgArgs(L), -2), "Mouse_OtherMouseDraggedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_LeftDoubleClickMsgArgs(L), -2), "Mouse_LeftDoubleClickMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_RightDoubleClickMsgArgs(L), -2), "Mouse_RightDoubleClickMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_OtherDoubleClickMsgArgs(L), -2), "Mouse_OtherDoubleClickMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_MouseMovedMsgArgs(L), -2), "Mouse_MouseMovedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_ScrollWheelMsgArgs(L), -2), "Mouse_ScrollWheelMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_DragDropMsgArgs(L), -2), "Mouse_DragDropMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse_DragEnterMsgArgs(L), -2), "Mouse_DragEnterMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Keyboard_KeyMessageMsgArgs(L), -2), "Keyboard_KeyMessageMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Keyboard_KeyDownMsgArgs(L), -2), "Keyboard_KeyDownMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Keyboard_KeyUpMsgArgs(L), -2), "Keyboard_KeyUpMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Keyboard_TextInputMsgArgs(L), -2), "Keyboard_TextInputMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_CreateMsgArgs(L), -2), "Object_CreateMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_StartMsgArgs(L), -2), "Object_StartMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_AwakeMsgArgs(L), -2), "Object_AwakeMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_ThemeChangedMsgArgs(L), -2), "Object_ThemeChangedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_PropertyChangedMsgArgs(L), -2), "Object_PropertyChangedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_AttachedMsgArgs(L), -2), "Object_AttachedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_ReleaseMsgArgs(L), -2), "Object_ReleaseMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_DestroyMsgArgs(L), -2), "Object_DestroyMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Object_TimerMsgArgs(L), -2), "Object_TimerMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Mouse(L), -2), "Input");
	lua_setfield(L, ((void)luaopen_orca_Keyboard(L), -2), "Input");
	lua_setfield(L, ((void)luaopen_orca_Object(L), -2), "Object");
	void on_core_module_registered(lua_State *L);
	on_core_module_registered(L);
	return 1;
}
