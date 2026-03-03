local orca = require "orca"

local fallbacks = {
	Texture = function (path)
		local extensions = { "", ".png", ".jpg", ".jpeg", ".svg"}
		local filesystem = require "orca.filesystem"
		local renderer = require "orca.renderer"
		local cleanpath, query = path:match("^([^?]+)%??(.*)")
		local attrs = { Name = filesystem.getBaseName(cleanpath) }
		for pair in query:gmatch("[^&]+") do
			local k, v = pair:match("([^=]+)=?(.*)")
			if k and #k > 0 then attrs[k:gsub("^%l", string.upper)] = v end
		end
		for _, ext in ipairs(extensions) do
			local fullpath = cleanpath .. ext
			if filesystem.fileExists(fullpath) then
				attrs.Source = fullpath
				return renderer.Image(attrs)
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
	None = function() error("Cannot convert to none type") end,
	Bool = function(value, type)
		if value == "true" then return true end
		if value == "false" then return false end
		return tonumber(value) or error(string.format("Cannot convert '%s' to %s(Bool)", value, type.TypeString))
	end,
	Int = function(value, type)
		return tonumber(value) or error(string.format("Cannot convert '%s' to %s(Int)", value, type.TypeString))
	end,
	Float = function(value, type)
		return tonumber(value) or error(string.format("Cannot convert '%s' to %s(Float)", value, type.TypeString))
	end,
	Enum = function(value, type)
		local i = 0
		for w in type.TypeString:gmatch("[^,]+") do
			i = i + 1
			if w == value then return i - 1 end
		end
		error(string.format("Cannot convert '%s' to %s(Enum)", value, type.TypeString))
	end,
  LongString = function(value) return value end,
	Fixed = function(value) return value end,
	Object = function(path, type)
		local ok, resource = pcall(require, path)
		if ok then return resource end
		local fallback = fallbacks[type.TypeString]
		if fallback then
			local result = fallback(path)
			package.loaded[path] = result
			if result then return result end
		end
		error(string.format("Cannot convert '%s' to %s(Object)", path, type.TypeString))
	end,
  ObjectTags = function(path)
		local tags = 0
		for tag in path:gmatch("[^,]+") do tags = tags | (1 << orca.tags:parse(tag)) end
		return tags
	end,
  Event = function(value, type)
		error(string.format("Cannot convert '%s' to %s(Event)", value, type.TypeString))
	end,
  Struct = function(value, pt)
		if pt.TypeString == "Color" then
			local geom = require "orca.geometry"
			return geom.Color.parse(value)
		end
		local mt = orca.find_metatable(pt.TypeString)
		assert(mt, string.format("No metatable found for %s(Struct)", pt.TypeString))
		assert(mt.fromstring, string.format("Type %s does not support fromstring() method", pt.TypeString))
		return mt.fromstring(value)
	end,
  Group = function(value, type)
		error(string.format("Cannot convert '%s' to %s(Group)", value, type.TypeString))
	end,
}