void luaX_push${name}(lua_State *L, ${lpcname} ${name}) {
	lua_pushlightuserdata(L, (${lpname})${name});
}
${lpname} luaX_check${name}(lua_State *L, int idx) {
	return lua_touserdata(L, idx);
}
