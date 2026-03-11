<?php

class config_new {
	public static $TypeInfos = [
		"float" => [
			'decl' => 'float', 
			'pop' => '%s = luaL_checknumber/*%s*/(L, %d)', 
			'push' => 'lua_pushnumber/*%s*/(L, %s)',  
			'convert' => '%s = {arg}', 
			'format' => "%f"
		],
		"int" => [
			'decl' => 'int32_t', 
			'pop' => '%s = luaL_checknumber/*%s*/(L, %d)', 
			'push' => 'lua_pushnumber/*%s*/(L, %s)',  
			'convert' => '%s = {arg}', 
			'format' => "%d"
		],
		"uint" => [
			'decl' => 'uint32_t', 
			'pop' => '%s = luaL_checknumber/*%s*/(L, %d)', 
			'push' => 'lua_pushnumber/*%s*/(L, %s)',  
			'convert' => '%s = {arg}', 
			'format' => "%u"
		],
		"long" => [
			'decl' => 'long', 
			'pop' => '%s = luaL_checkinteger/*%s*/(L, %d)',
			'push' => 'lua_pushinteger/*%s*/(L, %s)', 
			'convert' => '%s = {arg}', 
			'format' => "%ld"
		],
		"bool" => [
			'decl' => 'bool_t', 
			'pop' => '%s = lua_toboolean/*%s*/(L, %d)', 
			'push' => 'lua_pushboolean/*%s*/(L, %s)', 
			'convert' => '%s = !strcmp({arg}, "true")', 
			'format' => "%s"
		],
		"string" => [
			'decl' => 'const char*', 
			'pop' => '%s = strdup(luaL_check%s(L, %d))', 
			'push' => 'lua_push%s(L, %s)',  
			'convert' => '%s = {arg}', 
			'format' => "%s"
		],
		"objectTags" => [
			'decl' => 'objectTags_t', 
			'pop' => '%s = luaL_checkinteger(L, %d)',
			'push' => 'lua_pushinteger/*%s*/(L, %s)', 
			'convert' => '%s = {arg}', 
			'format' => "%ld"
		],
		"fixed" => [
			'decl' => '%sString_t', 
			'pop' => 'strncpy(%s, luaL_checkstring(L, %d), sizeof(%s))', 
			'push' => 'lua_pushstring/*%s*/(L, %s)',  
			'convert' => 'strncpy(%s, %d, sizeof(%s))', 
			'format' => "%s"
		],
		"enum" => [
			'decl' => 'enum %s', 
			'pop' => '%s = luaX_check%s(L, %d)', 
			'push' => 'luaX_push%s(L, %s)', 
			'convert' => 'lua_pop(L, (lua_pushstring(L, %d), %s = luaL_checkoption(L, -1, NULL, _%s), 1));', 
			'format' => "%s"
		],
		"struct" => [
			'decl' => 'struct %s', 
			'pop' => '%s = *luaX_check%s(L, %d)',
			'push' => 'luaX_push%s(L, &%s)',
			'convert' => '%s = {arg}', 
			'format' => "%s"
		],
		"interface" => [
			'decl' => 'struct %s', 
			'pop' => '%s = luaX_check%s(L, %d)',
			'push' => 'luaX_push%s(L, %s)',
			'convert' => '%s = {arg}', 
			'format' => "%s"
		],
		"component" => [
			'decl' => 'struct %s', 
			'pop' => '%s = luaX_check%s(L, %d)',
			'push' => 'luaX_push%s(L, %s)', 
			'convert' => 'lua_pop(L, (f_convert_string(L, &(struct PropertyType) { .DataType = kDataTypeObject, .TypeString = "%s" }, %d, TRUE), %s = luaX_check%s(L, -1), 1));', 
			'format' => "%s"
		],
		"resource" => [
			'decl' => 'struct %s', 
			'pop' => '%s = luaX_check%s(L, %d)',
			'push' => 'luaX_push%s(L, %s)', 
			'convert' => '%s = {arg}', 
			'format' => "%s"
		],
		"external_struct" => [
			'decl' => 'struct %s', 
			'pop' => '%s = NULL',
			'push' => 'luaX_push%s(L, %s)',
			'convert' => '', 
			'format' => "%s"
		],
		"nresults" => [
			'decl' => 'int',
			'push' => 'lua_pushnil(L)', 
			'return' => 'return {arg};' 
		],
		"void" => [
			'decl' => '%s', 
			'return' => '{arg};\n\treturn 0;' 
		],
		"lua_State" => [],
	];

	public static $Axis = [
		["(.+)\\.Axis\\[0\\]\\.Left(.*)",         "\\1Left\\2"],
		["(.+)\\.Axis\\[0\\]\\.Right(.*)",        "\\1Right\\2"],
		["(.+)\\.Axis\\[1\\]\\.Left(.*)",         "\\1Top\\2"],
		["(.+)\\.Axis\\[1\\]\\.Right(.*)",        "\\1Bottom\\2"],
		["(.+)\\.Axis\\[2\\]\\.Left(.*)",         "\\1Front\\2"],
		["(.+)\\.Axis\\[2\\]\\.Right(.*)",        "\\1Back\\2"],
		["Size.Axis\\[0\\]\\.(?:Requested)?(.*)", "\\1Width"],
		["Size.Axis\\[1\\]\\.(?:Requested)?(.*)", "\\1Height"],
		["Size.Axis\\[2\\]\\.(?:Requested)?(.*)", "\\1Depth"],
		["(.+)\\.Axis\\[0\\]",                    "Horizontal\\1"],
		["(.+)\\.Axis\\[1\\]",                    "Vertical\\1"],
		["(.+)\\.Axis\\[2\\]",                    "Depth\\1"],
		["Border\\.Radius\\.(.+)Radius",          "Border\\1Radius"],
	];
}
