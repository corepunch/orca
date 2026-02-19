static int f_new_${name}(lua_State *L) {
	${lpname} self = lua_newuserdata(L, sizeof(struct ${name}));
	luaL_setmetatable(L, "${name}");
	memset(self, 0, sizeof(struct ${name}));
${field_inits}	// if (lua_istable(L, 1)) {
	// }
	return 1;
}
static int f_${name}___call(lua_State *L) {
	lua_remove(L, 1); // remove ${name} from stack
	return f_new_${name}(L);
}
