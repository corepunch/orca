-- Minimal single-inheritance classes for editor panels.
local Class = {}
Class.__index = Class

function Class:extend(def)
	def = def or {}
	def.__index = def
	def.super = self
	return setmetatable(def, {
		__index = self,
		__call = function(cls, ...)
			local o = setmetatable({}, cls)
			o:init(...)
			return o
		end,
	})
end

function Class:init() end

function Class:is(cls)
	local mt = getmetatable(self)
	while mt do
		if mt == cls then return true end
		mt = mt.super
	end
	return false
end

return Class
