<?php

class config {
	// Each type entry must be a single-line array (pyphp limitation with nested multi-line arrays)
	public static $TypeInfos = [
		"atomic"    => ["check" => "luaL_checknumber",  "push" => "lua_pushnumber"],
		"enum"      => ["check" => "luaL_checkoption",  "push" => "lua_pushstring"],
		"struct"    => ["check" => "luaX_check{type}",  "push" => "luaX_push{type}"],
		"component" => ["check" => "luaX_check{type}",  "push" => "luaX_push{type}"],
		"resource"  => ["check" => "luaX_check{type}",  "push" => "luaX_push{type}"],
		"fixed"     => ["check" => "luaL_checkstring",  "push" => "lua_pushstring"],
		"unknown"   => ["check" => "lua_touserdata",    "push" => "lua_pushlightuserdata"],
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
