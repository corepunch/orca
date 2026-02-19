${lpname} luaX_check${name}(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "${name}");
}
