local orca = require "orca"

local fallbacks = {
	Texture = function (path)
		local extensions = { "", ".png", ".jpg", ".jpeg", ".svg"}
		local filesystem = require "orca.filesystem"
		local renderer = require "orca.renderer"
		for _, ext in ipairs(extensions) do
			local fullpath = path..ext
			if filesystem.fileExists(fullpath) then
				return renderer.Image {
					Name = filesystem.getBaseName(path),
					Source = fullpath,
				}
			end
		end
	end,
}

local function passthrough(value)
	return value
end

local bool_map = { ["true"] = true, ["false"] = false }

orca.typeconverter = {
	none = passthrough,
	bool = function(value, type)
		if bool_map[value] ~= nil then return bool_map[value] end
		local n = tonumber(value)
		if n then return n ~= 0 end
		error(string.format("Cannot convert to %s(bool): %s", type.TypeString, value))
	end,
	int = function(value, type)
		return tonumber(value) or error(string.format("Cannot convert to %s(int): %s", type.TypeString, value))
	end,
	float = function(value, type)
		return tonumber(value) or error(string.format("Cannot convert to %s(float): %s", type.TypeString, value))
	end,
	enum = function(value, type)
		local i = 0
		for w in type.TypeString:gmatch("[^,]+") do
			i = i + 1
			if w == value then return i - 1 end
		end
		error(string.format("Cannot convert to %s(enum): %s", type.TypeString, value))
	end,
	fixed      = passthrough,
	longstring = passthrough,
	event      = passthrough,
	struct     = passthrough,
	group      = passthrough,
	edges      = passthrough,
	objecttags = passthrough,
	object = function(path, type)
		local ok, resource = pcall(require, path)
		if ok then
			return resource
		end
		local fallback = fallbacks[type.TypeString]
		if fallback then
			return fallback(path)
		else
			error(string.format("Cannot load resource %s of type %s", path, type.TypeString))
		end
	end,
}