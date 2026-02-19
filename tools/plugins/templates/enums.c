static const char *_${ename}[] = {${values}};
${ename_t} luaX_check${ename}(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _${ename});
}
void luaX_push${ename}(lua_State *L, ${ename_t} value) {
	assert(value >= 0 && value < ${count});
	lua_pushstring(L, _${ename}[value]);
}
