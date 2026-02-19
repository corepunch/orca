void luaX_push${name}(lua_State *L, ${lpcname} data) {
	${lpname} self = lua_newuserdata(L, sizeof(struct ${name}));
	luaL_setmetatable(L, "${name}");
	memcpy(self, data, sizeof(struct ${name}));
}
