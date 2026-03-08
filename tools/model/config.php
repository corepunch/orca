<?php

class config {
	// Each type entry must be a single-line array (pyphp limitation with nested multi-line dicts)
	public static $TypeInfos = [
		"float"     => ["check" => 'luaL_checknumber(L, {arg})', "pop" => 'luaL_checknumber(L, {arg})', "push" => 'lua_pushnumber(L, {arg})',  "convert" => 'self.{addr} = {arg}', "format" => "%f"],
		"int"       => ["check" => 'luaL_checknumber(L, {arg})', "pop" => 'luaL_checknumber(L, {arg})', "push" => 'lua_pushnumber(L, {arg})',  "convert" => 'self.{addr} = {arg}', "format" => "%d"],
		"uint"      => ["check" => 'luaL_checknumber(L, {arg})', "pop" => 'luaL_checknumber(L, {arg})', "push" => 'lua_pushnumber(L, {arg})',  "convert" => 'self.{addr} = {arg}', "format" => "%u"],
		"long"      => ["check" => 'luaL_checkinteger(L, {arg})',"pop" => 'luaL_checkinteger(L, {arg})',"push" => 'lua_pushinteger(L, {arg})', "convert" => 'self.{addr} = {arg}', "format" => "%ld"],
		"bool"      => ["check" => 'lua_toboolean(L, {arg})',    "pop" => 'lua_toboolean(L, {arg})',    "push" => 'lua_pushboolean(L, {arg})', "convert" => 'self.{addr} = !strcmp({arg}, "true")', "format" => "%d"],
		"string"    => ["check" => 'luaL_checkstring(L, {arg})', "pop" => 'luaL_checkstring(L, {arg})', "push" => 'lua_pushstring(L, {arg})',  "convert" => 'self.{addr} = {arg}', "format" => "%s"],
		"fixed"     => ["check" => 'luaL_checkstring(L, {arg})', "pop" => 'luaL_checkstring(L, {arg})', "push" => 'lua_pushstring(L, {arg})',  "convert" => 'strncpy(self.{addr}, {arg}, sizeof(self.{addr}))', "format" => "%s"],
		"enum"      => ["check" => 'luaX_check{type}(L, {arg})', "pop" => 'luaX_check{type}(L, {arg})', "push" => 'luaX_push{type}(L, {arg})', "convert" => 'lua_pop(L, (lua_pushstring(L, {arg}), self.{addr} = luaL_checkoption(L, -1, NULL, _{type}), 1));', "format" => "%s"],
		"struct"    => ["check" => 'luaX_check{type}(L, {arg})', "pop" => '*luaX_check{type}(L, {arg})',"push" => 'luaX_push{type}(L, &{arg})',"convert" => 'self.{addr} = {arg}', "format" => "%s"],
		"component" => ["check" => 'luaX_check{type}(L, {arg})', "pop" => '*luaX_check{type}(L, {arg})',"push" => 'luaX_push{type}(L, {arg})', "convert" => 'lua_pop(L, (f_convert_string(L, &(struct PropertyType) { .DataType = kDataTypeObject, .TypeString = "{type}" }, {arg}, TRUE), self.{addr} = luaX_check{type}(L, -1), 1));', "format" => "%s"],
		"resource"  => ["check" => 'luaX_check{type}(L, {arg})', "pop" => '*luaX_check{type}(L, {arg})',"push" => 'luaX_push{type}(L, {arg})', "convert" => 'self.{addr} = {arg}', "format" => "%s"],
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
