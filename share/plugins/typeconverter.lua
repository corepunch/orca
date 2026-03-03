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

orca.tags = {
	parse = function(self, tag)
		for i = 1, #self do if self[i] == tag then return i end end
		table.insert(self, tag)
		return #self
	end
}

orca.typeconverter = {
	none = function() error("Cannot convert to none type") end,
	bool = function(value, type)
		if value == "true" then return true end
		if value == "false" then return false end
		return tonumber(value) or error(string.format("Cannot convert '%s' to %s(bool)", value, type.TypeString))
	end,
	int = function(value, type)
		return tonumber(value) or error(string.format("Cannot convert '%s' to %s(int)", value, type.TypeString))
	end,
	float = function(value, type)
		return tonumber(value) or error(string.format("Cannot convert '%s' to %s(float)", value, type.TypeString))
	end,
	enum = function(value, type)
		local i = 0
		for w in type.TypeString:gmatch("[^,]+") do
			i = i + 1
			if w == value then return i - 1 end
		end
		error(string.format("Cannot convert '%s' to %s(enum)", value, type.TypeString))
	end,
  longstring= function(value) return value end,
	fixed = function(value) return value end,
	object = function(path, type)
		-- local filesystem = require "orca.filesystem"
		-- local existing = filesystem.getWorkspace():findByPath(path)
		-- if existing then return existing end
		local ok, resource = pcall(require, path)
		if ok then return resource end
		local fallback = fallbacks[type.TypeString]
		if fallback then
			local result = fallback(path)
			package.loaded[path] = result
			if result then return result end
		end
		error(string.format("Cannot convert '%s' to %s(object)", path, type.TypeString))
	end,
  edges = function() error("Cannot convert to edges type") end,
  objecttags = function(path)
		local tags = 0
		for tag in path:gmatch("[^,]+") do tags = tags | (1 << orca.tags:parse(tag)) end
		return tags
	end,
  event = function(value, type)
		error(string.format("Cannot convert '%s' to %s(event)", value, type.TypeString))
	end,
  struct = function(value, pt)
		if pt.TypeString == "Color" then
			local geom = require "orca.geometry"
			return geom.Color.parse(value)
		end
		local mt = orca.find_metatable(pt.TypeString)
		assert(mt, string.format("No metatable found for %s(struct)", pt.TypeString))
		assert(mt.fromstring, string.format("Type %s does not support fromstring() method", pt.TypeString))
		return mt.fromstring(value)
	end,
  group = function(value, type)
		error(string.format("Cannot convert '%s' to %s(group)", value, type.TypeString))
	end,
}