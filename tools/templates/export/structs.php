extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, int idx, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);

#define STRUCT(NAME, EXPORT) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
	if (data == NULL) { lua_pushnil(L); return; } \
	memcpy((luaL_setmetatable(L, #EXPORT), lua_newuserdata(L, sizeof(struct NAME))), data, sizeof(struct NAME)); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { return luaL_checkudata(L, idx, #EXPORT); } \
static int f_new_##NAME(lua_State *L) { \
	struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
	memset((luaL_setmetatable(L, #EXPORT), self), 0, sizeof(struct NAME)); \
	if (lua_istable(L, 1)) \
    for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
			lua_pop(L, (lua_getfield(L, 1, _##NAME[i].Name), read_property(L, -1, &_##NAME[i], self), 1)); \
	else for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		read_property(L, i + 1, &_##NAME[i], self); \
	return 1; \
} \
static int f_##NAME##___index(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (write_property(L, -1, &_##NAME[i], luaX_check##NAME(L, 1)), 1); \
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
			return (read_property(L, 3, &_##NAME[i], luaX_check##NAME(L, 1)), 0); \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___call(lua_State *L) { \
	return ((void)lua_remove(L, 1), f_new_##NAME(L)); \
} \
static int f_##NAME##___fromstring(lua_State *L) { \
	char* tmp = strdup(luaL_checkstring(L, 1)),* tok = strtok(tmp, " "); \
	struct NAME self; \
	memset(&self, 0, sizeof(struct NAME)); \
	for (uint32_t i = 0; tok && i < sizeof(_##NAME) / sizeof(*_##NAME); i++, tok = strtok(NULL, " ")) \
		if (_##NAME[i].DataType != kDataTypeStruct) \
			parse_property(tok, &_##NAME[i], &self); \
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
<?php foreach ($structs as $name => $struct):?>
<?php include_template("export/functions", ['functions' => $struct->getMethods(), 'prefix' => $struct->prefix]) ?>
static struct PropertyType _<?= $name ?>[] = {
	<?php include_template("export/properties", ['properties' => $struct->getFields(true), 'name' => $name]) ?>
};
static luaL_Reg _<?= $name ?>_Methods[] = {
<?php foreach ($struct->getMethods() as $method_name => $method):?>
	{ "<?= $method->export ?>", f_<?= $struct->prefix.$method_name ?> },
<?php endforeach ?>
	{ NULL, NULL }
};
<?php endforeach ?>

<?php foreach ($structs as $name => $struct):?>
STRUCT(<?= $name ?>, <?= $struct->export ?>);
<?php endforeach ?>
