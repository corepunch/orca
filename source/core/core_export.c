// Auto-generated from core.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "core.h"

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

ORCA_API const char *_MessageRouting[] = {"TunnelingBubbling","Tunneling","Bubbling","Direct",NULL};
const char *MessageRoutingToString(enum MessageRouting value) {
	assert(value >= 0 && value < 4);
	return _MessageRouting[value];
}
enum MessageRouting luaX_checkMessageRouting(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _MessageRouting);
}
void luaX_pushMessageRouting(lua_State *L, enum MessageRouting value) {
	assert(value >= 0 && value < 4);
	lua_pushstring(L, _MessageRouting[value]);
}
ORCA_API const char *_PropertyState[] = {"Normal","Hover","Focus","Select","Disable","OldValue",NULL};
const char *PropertyStateToString(enum PropertyState value) {
	assert(value >= 0 && value < 6);
	return _PropertyState[value];
}
enum PropertyState luaX_checkPropertyState(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _PropertyState);
}
void luaX_pushPropertyState(lua_State *L, enum PropertyState value) {
	assert(value >= 0 && value < 6);
	lua_pushstring(L, _PropertyState[value]);
}
ORCA_API const char *_BindingMode[] = {"OneWay","TwoWay","OneWayToSource","Expression",NULL};
const char *BindingModeToString(enum BindingMode value) {
	assert(value >= 0 && value < 4);
	return _BindingMode[value];
}
enum BindingMode luaX_checkBindingMode(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _BindingMode);
}
void luaX_pushBindingMode(lua_State *L, enum BindingMode value) {
	assert(value >= 0 && value < 4);
	lua_pushstring(L, _BindingMode[value]);
}
ORCA_API const char *_PropertyAttribute[] = {"WholeProperty","ColorR","ColorG","ColorB","ColorA","VectorX","VectorY","VectorZ","VectorW",NULL};
const char *PropertyAttributeToString(enum PropertyAttribute value) {
	assert(value >= 0 && value < 9);
	return _PropertyAttribute[value];
}
enum PropertyAttribute luaX_checkPropertyAttribute(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _PropertyAttribute);
}
void luaX_pushPropertyAttribute(lua_State *L, enum PropertyAttribute value) {
	assert(value >= 0 && value < 9);
	lua_pushstring(L, _PropertyAttribute[value]);
}
ORCA_API const char *_DataType[] = {"None","Bool","Int","Enum","Float","String","Event","Struct","Color","Object",NULL};
const char *DataTypeToString(enum DataType value) {
	assert(value >= 0 && value < 10);
	return _DataType[value];
}
enum DataType luaX_checkDataType(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _DataType);
}
void luaX_pushDataType(lua_State *L, enum DataType value) {
	assert(value >= 0 && value < 10);
	lua_pushstring(L, _DataType[value]);
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
	enum MessageRouting routing = lua_isnoneornil(L, 3) ? kMessageRoutingBubbling : luaX_checkMessageRouting(L, 3);
	struct Object* result_ = OBJ_DispatchEvent(L, this_, event, routing);
	luaX_pushObject(L, result_);
	return 1;
}
int f_OBJ_PostMessage(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* message = luaL_checkstring(L, 2);
	OBJ_PostMessage(L, this_, message );
	return 0;
}
int f_OBJ_SendMessage2(lua_State *L) {
	struct Object* this_ = luaX_checkObject(L, 1);
	const char* message = luaL_checkstring(L, 2);
	OBJ_SendMessage2(L, this_, message );
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
		{ "sendMessage2", f_OBJ_SendMessage2 },
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
void luaX_pushPropertyEnumValue(lua_State *L, struct PropertyEnumValue const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct PropertyEnumValue* self = lua_newuserdata(L, sizeof(struct PropertyEnumValue));
	luaL_setmetatable(L, "PropertyEnumValue");
	memcpy(self, data, sizeof(struct PropertyEnumValue));
}
struct PropertyEnumValue* luaX_checkPropertyEnumValue(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "PropertyEnumValue");
}
static int f_new_PropertyEnumValue(lua_State *L) {
	struct PropertyEnumValue* self = lua_newuserdata(L, sizeof(struct PropertyEnumValue));
	luaL_setmetatable(L, "PropertyEnumValue");
	memset(self, 0, sizeof(struct PropertyEnumValue));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Name"), strncpy(self->Name, luaL_optstring(L, -1, ""), sizeof(self->Name)), 1));
		lua_pop(L, (lua_getfield(L, 1, "Value"), self->Value = (int32_t)luaL_optinteger(L, -1, 0), 1));
	} else {
		strncpy(self->Name, luaL_optstring(L, 1, ""), sizeof(self->Name));
		self->Value = (int32_t)luaL_optinteger(L, 2, 0);
	}
	return 1;
}


int f_PropertyEnumValue___index(lua_State *L) {
	struct PropertyEnumValue* self = luaX_checkPropertyEnumValue(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x0fe07306: lua_pushstring(L, self->Name); return 1; // Name
	case 0xd147f96a: lua_pushinteger(L, self->Value); return 1; // Value
	}
	return luaL_error(L, "Unknown field in PropertyEnumValue(%p): %s", self, luaL_checkstring(L, 2));
}
int f_PropertyEnumValue___newindex(lua_State *L) {
	struct PropertyEnumValue* self = luaX_checkPropertyEnumValue(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x0fe07306: strncpy(self->Name, luaL_optstring(L, 3, ""), sizeof(self->Name)); return 0; // Name
	case 0xd147f96a: self->Value = (int32_t)luaL_optinteger(L, 3, 0); return 0; // Value
	}
	return luaL_error(L, "Unknown field in PropertyEnumValue(%p): %s", self, luaL_checkstring(L, 2));
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_PropertyEnumValue___fromstring(lua_State *L) {
	fixedString_t Name;
	int Value;
	struct PropertyEnumValue self = {0};
	switch (sscanf(luaL_checkstring(L, 1), "%s %d", Name, &Value)) {
	case 2: 
		strncpy(self.Name, Name, sizeof(self.Name));
		self.Value = Value;
		return (luaX_pushPropertyEnumValue(L, &self), 1);
	default:
		return luaL_error(L, "Invalid format for PropertyEnumValue: %s", luaL_checkstring(L, 1));
	}
}
static int f_PropertyEnumValue___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_PropertyEnumValue(L));  // remove PropertyEnumValue from stack and call constructor
}
int luaopen_orca_PropertyEnumValue(lua_State *L) {
	luaL_newmetatable(L, "PropertyEnumValue");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_PropertyEnumValue },
		{ "fromstring", f_PropertyEnumValue___fromstring },
		{ "__newindex", f_PropertyEnumValue___newindex },
		{ "__index", f_PropertyEnumValue___index },
		{ NULL, NULL },
	}), 0);
	// Make PropertyEnumValue creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_PropertyEnumValue___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushPropertyType(lua_State *L, struct PropertyType const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct PropertyType* self = lua_newuserdata(L, sizeof(struct PropertyType));
	luaL_setmetatable(L, "PropertyType");
	memcpy(self, data, sizeof(struct PropertyType));
}
struct PropertyType* luaX_checkPropertyType(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "PropertyType");
}
static int f_new_PropertyType(lua_State *L) {
	struct PropertyType* self = lua_newuserdata(L, sizeof(struct PropertyType));
	luaL_setmetatable(L, "PropertyType");
	memset(self, 0, sizeof(struct PropertyType));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Name"), strncpy(self->Name, luaL_optstring(L, -1, ""), sizeof(self->Name)), 1));
		lua_pop(L, (lua_getfield(L, 1, "Category"), strncpy(self->Category, luaL_optstring(L, -1, ""), sizeof(self->Category)), 1));
		lua_pop(L, (lua_getfield(L, 1, "DataType"), self->DataType = lua_type(L, -1) == LUA_TSTRING ? luaX_checkDataType(L, -1) : 0, 1));
		lua_pop(L, (lua_getfield(L, 1, "DefaultValue"), strncpy(self->DefaultValue, luaL_optstring(L, -1, ""), sizeof(self->DefaultValue)), 1));
		lua_pop(L, (lua_getfield(L, 1, "TypeString"), strncpy(self->TypeString, luaL_optstring(L, -1, ""), sizeof(self->TypeString)), 1));
		lua_pop(L, (lua_getfield(L, 1, "AffectLayout"), self->AffectLayout = lua_toboolean(L, -1), 1));
		lua_pop(L, (lua_getfield(L, 1, "AffectRender"), self->AffectRender = lua_toboolean(L, -1), 1));
		lua_pop(L, (lua_getfield(L, 1, "IsReadOnly"), self->IsReadOnly = lua_toboolean(L, -1), 1));
		lua_pop(L, (lua_getfield(L, 1, "IsHidden"), self->IsHidden = lua_toboolean(L, -1), 1));
		lua_pop(L, (lua_getfield(L, 1, "IsInherited"), self->IsInherited = lua_toboolean(L, -1), 1));
		lua_pop(L, (lua_getfield(L, 1, "Key"), strncpy(self->Key, luaL_optstring(L, -1, ""), sizeof(self->Key)), 1));
		lua_pop(L, (lua_getfield(L, 1, "Value"), strncpy(self->Value, luaL_optstring(L, -1, ""), sizeof(self->Value)), 1));
		lua_pop(L, (lua_getfield(L, 1, "Step"), self->Step = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "UpperBound"), self->UpperBound = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "LowerBound"), self->LowerBound = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "ShortIdentifier"), self->ShortIdentifier = (uint32_t)luaL_optinteger(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "FullIdentifier"), self->FullIdentifier = (uint32_t)luaL_optinteger(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Offset"), self->Offset = (uint32_t)luaL_optinteger(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "DataSize"), self->DataSize = (uint32_t)luaL_optinteger(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "IsArray"), self->IsArray = lua_toboolean(L, -1), 1));
	} else {
		strncpy(self->Name, luaL_optstring(L, 1, ""), sizeof(self->Name));
		strncpy(self->Category, luaL_optstring(L, 2, ""), sizeof(self->Category));
		self->DataType = lua_type(L, 3) == LUA_TSTRING ? luaX_checkDataType(L, 3) : 0;
		strncpy(self->DefaultValue, luaL_optstring(L, 4, ""), sizeof(self->DefaultValue));
		strncpy(self->TypeString, luaL_optstring(L, 5, ""), sizeof(self->TypeString));
		self->AffectLayout = lua_toboolean(L, 6);
		self->AffectRender = lua_toboolean(L, 7);
		self->IsReadOnly = lua_toboolean(L, 8);
		self->IsHidden = lua_toboolean(L, 9);
		self->IsInherited = lua_toboolean(L, 10);
		strncpy(self->Key, luaL_optstring(L, 11, ""), sizeof(self->Key));
		strncpy(self->Value, luaL_optstring(L, 12, ""), sizeof(self->Value));
		self->Step = luaL_optnumber(L, 13, 0);
		self->UpperBound = luaL_optnumber(L, 14, 0);
		self->LowerBound = luaL_optnumber(L, 15, 0);
		self->ShortIdentifier = (uint32_t)luaL_optinteger(L, 16, 0);
		self->FullIdentifier = (uint32_t)luaL_optinteger(L, 17, 0);
		self->Offset = (uint32_t)luaL_optinteger(L, 18, 0);
		self->DataSize = (uint32_t)luaL_optinteger(L, 19, 0);
		self->IsArray = lua_toboolean(L, 20);
	}
	return 1;
}


int f_PropertyType___index(lua_State *L) {
	struct PropertyType* self = luaX_checkPropertyType(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x0fe07306: lua_pushstring(L, self->Name); return 1; // Name
	case 0xafb3e591: lua_pushstring(L, self->Category); return 1; // Category
	case 0x840d6c6d: luaX_pushDataType(L, self->DataType); return 1; // DataType
	case 0xcd093f9f: lua_pushstring(L, self->DefaultValue); return 1; // DefaultValue
	case 0xdf6c0780: lua_pushstring(L, self->TypeString); return 1; // TypeString
	case 0xd2d3694e: lua_pushboolean(L, self->AffectLayout); return 1; // AffectLayout
	case 0xcae7b378: lua_pushboolean(L, self->AffectRender); return 1; // AffectRender
	case 0xd9ee91e7: lua_pushboolean(L, self->IsReadOnly); return 1; // IsReadOnly
	case 0x3bf0d5c9: lua_pushboolean(L, self->IsHidden); return 1; // IsHidden
	case 0x26e59151: lua_pushboolean(L, self->IsInherited); return 1; // IsInherited
	case 0xcd1ac90c: lua_pushstring(L, self->Key); return 1; // Key
	case 0xd147f96a: lua_pushstring(L, self->Value); return 1; // Value
	case 0x4771f92f: lua_pushnumber(L, self->Step); return 1; // Step
	case 0x48b88645: lua_pushnumber(L, self->UpperBound); return 1; // UpperBound
	case 0xccc57b3a: lua_pushnumber(L, self->LowerBound); return 1; // LowerBound
	case 0x0f76864e: lua_pushinteger(L, self->ShortIdentifier); return 1; // ShortIdentifier
	case 0x429417cf: lua_pushinteger(L, self->FullIdentifier); return 1; // FullIdentifier
	case 0x8995c7ea: lua_pushinteger(L, self->Offset); return 1; // Offset
	case 0x58ff2a7c: lua_pushinteger(L, self->DataSize); return 1; // DataSize
	case 0x660880b6: lua_pushboolean(L, self->IsArray); return 1; // IsArray
	}
	return luaL_error(L, "Unknown field in PropertyType(%p): %s", self, luaL_checkstring(L, 2));
}
int f_PropertyType___newindex(lua_State *L) {
	struct PropertyType* self = luaX_checkPropertyType(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x0fe07306: strncpy(self->Name, luaL_optstring(L, 3, ""), sizeof(self->Name)); return 0; // Name
	case 0xafb3e591: strncpy(self->Category, luaL_optstring(L, 3, ""), sizeof(self->Category)); return 0; // Category
	case 0x840d6c6d: self->DataType = lua_type(L, 3) == LUA_TSTRING ? luaX_checkDataType(L, 3) : 0; return 0; // DataType
	case 0xcd093f9f: strncpy(self->DefaultValue, luaL_optstring(L, 3, ""), sizeof(self->DefaultValue)); return 0; // DefaultValue
	case 0xdf6c0780: strncpy(self->TypeString, luaL_optstring(L, 3, ""), sizeof(self->TypeString)); return 0; // TypeString
	case 0xd2d3694e: self->AffectLayout = lua_toboolean(L, 3); return 0; // AffectLayout
	case 0xcae7b378: self->AffectRender = lua_toboolean(L, 3); return 0; // AffectRender
	case 0xd9ee91e7: self->IsReadOnly = lua_toboolean(L, 3); return 0; // IsReadOnly
	case 0x3bf0d5c9: self->IsHidden = lua_toboolean(L, 3); return 0; // IsHidden
	case 0x26e59151: self->IsInherited = lua_toboolean(L, 3); return 0; // IsInherited
	case 0xcd1ac90c: strncpy(self->Key, luaL_optstring(L, 3, ""), sizeof(self->Key)); return 0; // Key
	case 0xd147f96a: strncpy(self->Value, luaL_optstring(L, 3, ""), sizeof(self->Value)); return 0; // Value
	case 0x4771f92f: self->Step = luaL_optnumber(L, 3, 0); return 0; // Step
	case 0x48b88645: self->UpperBound = luaL_optnumber(L, 3, 0); return 0; // UpperBound
	case 0xccc57b3a: self->LowerBound = luaL_optnumber(L, 3, 0); return 0; // LowerBound
	case 0x0f76864e: self->ShortIdentifier = (uint32_t)luaL_optinteger(L, 3, 0); return 0; // ShortIdentifier
	case 0x429417cf: self->FullIdentifier = (uint32_t)luaL_optinteger(L, 3, 0); return 0; // FullIdentifier
	case 0x8995c7ea: self->Offset = (uint32_t)luaL_optinteger(L, 3, 0); return 0; // Offset
	case 0x58ff2a7c: self->DataSize = (uint32_t)luaL_optinteger(L, 3, 0); return 0; // DataSize
	case 0x660880b6: self->IsArray = lua_toboolean(L, 3); return 0; // IsArray
	}
	return luaL_error(L, "Unknown field in PropertyType(%p): %s", self, luaL_checkstring(L, 2));
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_PropertyType___fromstring(lua_State *L) {
	fixedString_t Name;
	fixedString_t Category;
	fixedString_t DataType;
	fixedString_t DefaultValue;
	fixedString_t TypeString;
	fixedString_t AffectLayout;
	fixedString_t AffectRender;
	fixedString_t IsReadOnly;
	fixedString_t IsHidden;
	fixedString_t IsInherited;
	fixedString_t Key;
	fixedString_t Value;
	float Step;
	float UpperBound;
	float LowerBound;
	unsigned ShortIdentifier;
	unsigned FullIdentifier;
	unsigned Offset;
	unsigned DataSize;
	fixedString_t IsArray;
	struct PropertyType self = {0};
	switch (sscanf(luaL_checkstring(L, 1), "%s %s %s %s %s %s %s %s %s %s %s %s %f %f %f %u %u %u %u %s", Name, Category, DataType, DefaultValue, TypeString, AffectLayout, AffectRender, IsReadOnly, IsHidden, IsInherited, Key, Value, &Step, &UpperBound, &LowerBound, &ShortIdentifier, &FullIdentifier, &Offset, &DataSize, IsArray)) {
	case 20: 
		strncpy(self.Name, Name, sizeof(self.Name));
		strncpy(self.Category, Category, sizeof(self.Category));
		lua_pop(L, (lua_pushstring(L, DataType), self.DataType = luaL_checkoption(L, -1, NULL, _DataType), 1));;
		strncpy(self.DefaultValue, DefaultValue, sizeof(self.DefaultValue));
		strncpy(self.TypeString, TypeString, sizeof(self.TypeString));
		self.AffectLayout = !strcmp(AffectLayout, "true");
		self.AffectRender = !strcmp(AffectRender, "true");
		self.IsReadOnly = !strcmp(IsReadOnly, "true");
		self.IsHidden = !strcmp(IsHidden, "true");
		self.IsInherited = !strcmp(IsInherited, "true");
		strncpy(self.Key, Key, sizeof(self.Key));
		strncpy(self.Value, Value, sizeof(self.Value));
		self.Step = Step;
		self.UpperBound = UpperBound;
		self.LowerBound = LowerBound;
		self.ShortIdentifier = ShortIdentifier;
		self.FullIdentifier = FullIdentifier;
		self.Offset = Offset;
		self.DataSize = DataSize;
		self.IsArray = !strcmp(IsArray, "true");
		return (luaX_pushPropertyType(L, &self), 1);
	default:
		return luaL_error(L, "Invalid format for PropertyType: %s", luaL_checkstring(L, 1));
	}
}
static int f_PropertyType___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_PropertyType(L));  // remove PropertyType from stack and call constructor
}
int luaopen_orca_PropertyType(lua_State *L) {
	luaL_newmetatable(L, "PropertyType");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_PropertyType },
		{ "fromstring", f_PropertyType___fromstring },
		{ "__newindex", f_PropertyType___newindex },
		{ "__index", f_PropertyType___index },
		{ NULL, NULL },
	}), 0);
	// Make PropertyType creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_PropertyType___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushWindowPaintMsgArgs(lua_State *L, struct WindowPaintMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct WindowPaintMsgArgs* self = lua_newuserdata(L, sizeof(struct WindowPaintMsgArgs));
	luaL_setmetatable(L, "WindowPaintMsgArgs");
	memcpy(self, data, sizeof(struct WindowPaintMsgArgs));
}
struct WindowPaintMsgArgs* luaX_checkWindowPaintMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "WindowPaintMsgArgs");
}
static int f_new_WindowPaintMsgArgs(lua_State *L) {
	struct WindowPaintMsgArgs* self = lua_newuserdata(L, sizeof(struct WindowPaintMsgArgs));
	luaL_setmetatable(L, "WindowPaintMsgArgs");
	memset(self, 0, sizeof(struct WindowPaintMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "WindowWidth"), self->WindowWidth = (uint32_t)luaL_optinteger(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "WindowHeight"), self->WindowHeight = (uint32_t)luaL_optinteger(L, -1, 0), 1));
	} else {
		self->WindowWidth = (uint32_t)luaL_optinteger(L, 1, 0);
		self->WindowHeight = (uint32_t)luaL_optinteger(L, 2, 0);
	}
	return 1;
}
int f_WindowPaintMsgArgs___index(lua_State *L) {
	struct WindowPaintMsgArgs* self = luaX_checkWindowPaintMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xdc5503a7: lua_pushinteger(L, self->WindowWidth); return 1; // WindowWidth
	case 0xbd75892a: lua_pushinteger(L, self->WindowHeight); return 1; // WindowHeight
	}
	return luaL_error(L, "Unknown field in WindowPaintMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_WindowPaintMsgArgs___newindex(lua_State *L) {
	struct WindowPaintMsgArgs* self = luaX_checkWindowPaintMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xdc5503a7: self->WindowWidth = (uint32_t)luaL_optinteger(L, 3, 0); return 0; // WindowWidth
	case 0xbd75892a: self->WindowHeight = (uint32_t)luaL_optinteger(L, 3, 0); return 0; // WindowHeight
	}
	return luaL_error(L, "Unknown field in WindowPaintMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_WindowPaintMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_WindowPaintMsgArgs(L));  // remove WindowPaintMsgArgs from stack and call constructor
}
int luaopen_orca_WindowPaintMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "WindowPaintMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_WindowPaintMsgArgs },
		{ "__newindex", f_WindowPaintMsgArgs___newindex },
		{ "__index", f_WindowPaintMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make WindowPaintMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_WindowPaintMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushUpdateMatrixMsgArgs(lua_State *L, struct UpdateMatrixMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct UpdateMatrixMsgArgs* self = lua_newuserdata(L, sizeof(struct UpdateMatrixMsgArgs));
	luaL_setmetatable(L, "UpdateMatrixMsgArgs");
	memcpy(self, data, sizeof(struct UpdateMatrixMsgArgs));
}
struct UpdateMatrixMsgArgs* luaX_checkUpdateMatrixMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "UpdateMatrixMsgArgs");
}
static int f_new_UpdateMatrixMsgArgs(lua_State *L) {
	struct UpdateMatrixMsgArgs* self = lua_newuserdata(L, sizeof(struct UpdateMatrixMsgArgs));
	luaL_setmetatable(L, "UpdateMatrixMsgArgs");
	memset(self, 0, sizeof(struct UpdateMatrixMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "parent"), self->parent = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkmat4(L, -1) : (struct mat4){0}, 1));
		lua_pop(L, (lua_getfield(L, 1, "opacity"), self->opacity = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "force"), self->force = lua_toboolean(L, -1), 1));
	} else {
		self->parent = lua_type(L, 1) == LUA_TUSERDATA ? *luaX_checkmat4(L, 1) : (struct mat4){0};
		self->opacity = luaL_optnumber(L, 2, 0);
		self->force = lua_toboolean(L, 3);
	}
	return 1;
}
int f_UpdateMatrixMsgArgs___index(lua_State *L) {
	struct UpdateMatrixMsgArgs* self = luaX_checkUpdateMatrixMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xeacdfcfd: luaX_pushmat4(L, &self->parent); return 1; // parent
	case 0xc6c2dd66: lua_pushnumber(L, self->opacity); return 1; // opacity
	case 0x79a98884: lua_pushboolean(L, self->force); return 1; // force
	}
	return luaL_error(L, "Unknown field in UpdateMatrixMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_UpdateMatrixMsgArgs___newindex(lua_State *L) {
	struct UpdateMatrixMsgArgs* self = luaX_checkUpdateMatrixMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xeacdfcfd: self->parent = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkmat4(L, 3) : (struct mat4){0}; return 0; // parent
	case 0xc6c2dd66: self->opacity = luaL_optnumber(L, 3, 0); return 0; // opacity
	case 0x79a98884: self->force = lua_toboolean(L, 3); return 0; // force
	}
	return luaL_error(L, "Unknown field in UpdateMatrixMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_UpdateMatrixMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_UpdateMatrixMsgArgs(L));  // remove UpdateMatrixMsgArgs from stack and call constructor
}
int luaopen_orca_UpdateMatrixMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "UpdateMatrixMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_UpdateMatrixMsgArgs },
		{ "__newindex", f_UpdateMatrixMsgArgs___newindex },
		{ "__index", f_UpdateMatrixMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make UpdateMatrixMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_UpdateMatrixMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushHitTestMsgArgs(lua_State *L, struct HitTestMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct HitTestMsgArgs* self = lua_newuserdata(L, sizeof(struct HitTestMsgArgs));
	luaL_setmetatable(L, "HitTestMsgArgs");
	memcpy(self, data, sizeof(struct HitTestMsgArgs));
}
struct HitTestMsgArgs* luaX_checkHitTestMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "HitTestMsgArgs");
}
static int f_new_HitTestMsgArgs(lua_State *L) {
	struct HitTestMsgArgs* self = lua_newuserdata(L, sizeof(struct HitTestMsgArgs));
	luaL_setmetatable(L, "HitTestMsgArgs");
	memset(self, 0, sizeof(struct HitTestMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "x"), self->x = (int32_t)luaL_optinteger(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "y"), self->y = (int32_t)luaL_optinteger(L, -1, 0), 1));
	} else {
		self->x = (int32_t)luaL_optinteger(L, 1, 0);
		self->y = (int32_t)luaL_optinteger(L, 2, 0);
	}
	return 1;
}
int f_HitTestMsgArgs___index(lua_State *L) {
	struct HitTestMsgArgs* self = luaX_checkHitTestMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xfd0c5087: lua_pushinteger(L, self->x); return 1; // x
	case 0xfc0c4ef4: lua_pushinteger(L, self->y); return 1; // y
	}
	return luaL_error(L, "Unknown field in HitTestMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_HitTestMsgArgs___newindex(lua_State *L) {
	struct HitTestMsgArgs* self = luaX_checkHitTestMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xfd0c5087: self->x = (int32_t)luaL_optinteger(L, 3, 0); return 0; // x
	case 0xfc0c4ef4: self->y = (int32_t)luaL_optinteger(L, 3, 0); return 0; // y
	}
	return luaL_error(L, "Unknown field in HitTestMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_HitTestMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_HitTestMsgArgs(L));  // remove HitTestMsgArgs from stack and call constructor
}
int luaopen_orca_HitTestMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "HitTestMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_HitTestMsgArgs },
		{ "__newindex", f_HitTestMsgArgs___newindex },
		{ "__index", f_HitTestMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make HitTestMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_HitTestMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushPropertyChangedMsgArgs(lua_State *L, struct PropertyChangedMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct PropertyChangedMsgArgs* self = lua_newuserdata(L, sizeof(struct PropertyChangedMsgArgs));
	luaL_setmetatable(L, "PropertyChangedMsgArgs");
	memcpy(self, data, sizeof(struct PropertyChangedMsgArgs));
}
struct PropertyChangedMsgArgs* luaX_checkPropertyChangedMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "PropertyChangedMsgArgs");
}
static int f_new_PropertyChangedMsgArgs(lua_State *L) {
	struct PropertyChangedMsgArgs* self = lua_newuserdata(L, sizeof(struct PropertyChangedMsgArgs));
	luaL_setmetatable(L, "PropertyChangedMsgArgs");
	memset(self, 0, sizeof(struct PropertyChangedMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Property"), self->Property = NULL, 1));
	} else {
		self->Property = NULL;
	}
	return 1;
}
int f_PropertyChangedMsgArgs___index(lua_State *L) {
	struct PropertyChangedMsgArgs* self = luaX_checkPropertyChangedMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x5221f9e8: luaX_pushProperty(L, self->Property); return 1; // Property
	}
	return luaL_error(L, "Unknown field in PropertyChangedMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_PropertyChangedMsgArgs___newindex(lua_State *L) {
	struct PropertyChangedMsgArgs* self = luaX_checkPropertyChangedMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x5221f9e8: self->Property = NULL; return 0; // Property
	}
	return luaL_error(L, "Unknown field in PropertyChangedMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_PropertyChangedMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_PropertyChangedMsgArgs(L));  // remove PropertyChangedMsgArgs from stack and call constructor
}
int luaopen_orca_PropertyChangedMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "PropertyChangedMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_PropertyChangedMsgArgs },
		{ "__newindex", f_PropertyChangedMsgArgs___newindex },
		{ "__index", f_PropertyChangedMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make PropertyChangedMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_PropertyChangedMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}


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
	void on_core_module_registered(lua_State *L);
	on_core_module_registered(L);
	lua_setfield(L, ((void)luaopen_orca_PropertyEnumValue(L), -2), "PropertyEnumValue");
	lua_setfield(L, ((void)luaopen_orca_PropertyType(L), -2), "PropertyType");
	lua_setfield(L, ((void)luaopen_orca_WindowPaintMsgArgs(L), -2), "WindowPaintMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_UpdateMatrixMsgArgs(L), -2), "UpdateMatrixMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_HitTestMsgArgs(L), -2), "HitTestMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_PropertyChangedMsgArgs(L), -2), "PropertyChangedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Object(L), -2), "Object");
	return 1;
}
