void luaX_push${cname}(lua_State *L, ${lpcname} ${cname}) {
	luaX_pushObject(L, CMP_GetObject(${cname}));
}
${lpname} luaX_check${cname}(lua_State *L, int idx) {
	return Get${cname}(luaX_checkObject(L, idx));
}
