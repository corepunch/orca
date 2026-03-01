local orca = require "orca"

orca.typeconverter = {
	int = function(value)
		return tonumber(value) or error("Cannot convert to int: "..value)
	end,
	float = function(value)
		return tonumber(value) or error("Cannot convert to float: "..value)
	end,
}