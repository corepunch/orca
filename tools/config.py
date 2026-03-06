
TypeInfos = {
	"float": {
		"check": "luaL_checknumber(L, {arg})", 
		"push": "lua_pushnumber(L, {arg})", 
		"convert": "self.{addr} = {arg}", 
		"format": "%f"
	},
	"int": {
		"check": "luaL_checknumber(L, {arg})", 
		"push": "lua_pushnumber(L, {arg})", 
		"convert": "self.{addr} = {arg}", 
		"format": "%d"
	},
	"uint": {
		"check": "luaL_checknumber(L, {arg})", 
		"push": "lua_pushnumber(L, {arg})", 
		"convert": "self.{addr} = {arg}", 
		"format": "%u"
	},
	"long": {
		"check": "luaL_checkinteger(L, {arg})", 
		"push": "lua_pushinteger(L, {arg})", 
		"convert": "self.{addr} = {arg}", 
		"format": "%ld"
	},
	"bool": {
		"check": "lua_toboolean(L, {arg})", 
		"push": "lua_pushboolean(L, {arg})", 
		"convert": "self.{addr} = !strcmp({arg}, \"true\")", 
		"format": "%d"
	},
	"string": {
		"check": "luaL_checkstring(L, {arg})", 
		"push": "lua_pushstring(L, {arg})", 
		"convert": "self.{addr} = {arg}", 
		"format": "%s"
	},
	"fixed": {
		"check": "luaL_checkstring(L, {arg})", 
		"push": "lua_pushstring(L, {arg})", 
		"convert": "strncpy(self.{addr}, {arg}, sizeof(self.{addr}))", 
		"format": "%s"
	},
	"enum": {
		"check": "luaL_checkoption(L, {arg}, NULL, _{type})", 
		"push": "lua_pushstring(L, {type}ToString({arg}))", 
		"convert": "lua_pop(L, (lua_pushstring(L, {arg}), self.{addr} = luaL_checkoption(L, -1, NULL, _{type}), 1));",
		"format": "%s"
	},
	"struct": {
		"check": "luaX_check{type}(L, {arg})", 
		"push": "luaX_push{type}(L, {arg})", 
		"convert": "self.{addr} = {arg}", 
		"format": "%s"
	},
	"component": {
		"check": "luaX_check{type}(L, {arg})", 
		"push": "luaX_push{type}(L, {arg})", 
		"convert": "lua_pop(L, (f_convert_string(L, &(struct PropertyType) {{ .DataType = kDataTypeObject, .TypeString = \"{type}\" }}, {arg}, TRUE), self.{addr} = luaX_check{type}(L, -1), 1));",
		"format": "%s"
	},
	"resource": {
		"check": "luaX_check{type}(L, {arg})", 
		"push": "luaX_push{type}(L, {arg})", 
		"convert": "self.{addr} = {arg}", 
		"format": "%s"
	},
}

Axis = [
	(r"(.+)\.Axis\[0\]\.Left(.*)",              r"\1Left\2"),
	(r"(.+)\.Axis\[0\]\.Right(.*)",             r"\1Right\2"),
	(r"(.+)\.Axis\[1\]\.Left(.*)",              r"\1Top\2"),
	(r"(.+)\.Axis\[1\]\.Right(.*)",             r"\1Bottom\2"),
	(r"(.+)\.Axis\[2\]\.Left(.*)",              r"\1Front\2"),
	(r"(.+)\.Axis\[2\]\.Right(.*)",             r"\1Back\2"),
	(r"Size.Axis\[0\]\.(?:Requested)?(.*)",     r"\1Width"),
	(r"Size.Axis\[1\]\.(?:Requested)?(.*)",     r"\1Height"),
	(r"Size.Axis\[2\]\.(?:Requested)?(.*)",     r"\1Depth"),
	(r"(.+)\.Axis\[0\]",                        r"Horizontal\1"),
	(r"(.+)\.Axis\[1\]",                        r"Vertical\1"),
	(r"(.+)\.Axis\[2\]",                        r"Depth\1"),
	(r"Border\.Radius\.(.+)Radius",             r"Border\1Radius"),
]
