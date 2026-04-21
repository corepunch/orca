local Widget = {}

function Widget:is(class)
  local cls = self
  while cls do
    if cls.__class == class or cls.__class.__name == class then
      return true
    end
    cls = getmetatable(cls)
  end
  return false
end

function Widget:extend(_base_0)
  local _parent_0 = self
  _base_0 = _base_0 or {}
  _base_0.__index = _base_0
  setmetatable(_base_0, _parent_0.__base)
  local _class_0 = setmetatable({
    __init = function(self_1, ...)
      return (rawget(_base_0, '__init') or _parent_0.__init)(self_1, ...)
    end,
    __base = _base_0,
    __name = 'LuaWidget',
    __parent = _parent_0
  }, {
    __index = function(_, name)
      return rawget(_base_0, name) or _parent_0[name]
    end,
    __call = function(cls, ...)
      local _self_0 = setmetatable({}, _base_0)
      cls.__init(_self_0, ...)
      return _self_0
    end,
  })
  _base_0.__class = _class_0
  if _parent_0.__inherited then
    _parent_0.__inherited(_parent_0, _class_0)
  end
  return _class_0
end

local _base_0 = Widget
_base_0.__index = _base_0
local _class_0 = setmetatable({
  __init = function(self, ...)
    local _base_1 = getmetatable(self)
    _base_1.__add = function(_self_0, ...) _self_0:addChild(...) return ... end
    _base_1.__call = function(_self_0, ...) return _self_0:instantiate(...) end
    _base_1.__index = function(_self_0, key)
      local mt = getmetatable(_base_1) or {}
      local cls = rawget(_base_1, '__class')
      local vl = rawget(_base_1, key) or (cls and cls[key]) or mt[key] or (mt.__class and mt.__class[key])
      if type(vl) == 'function' then
        return function (_, ...) return vl(_self_0, ...) end
      else
        return vl
      end
    end
    
    -- _base_1.__newindex = function(_self_0, key, value)
    --   return rawset(_self_0, key, value)
    -- end

    -- self.print = print
    -- self.assert = assert
    -- self.string = string
    -- self.ipairs = ipairs
    -- self.pairs = pairs
  end,
  __base = _base_0,
  __name = 'Widget'
}, {
  __index = _base_0,
  __call = function(cls, ...)
    local _self_0 = setmetatable({}, _base_0)
    cls.__init(_self_0, ...)
    return _self_0
  end
})
_base_0.__class = _class_0
return _class_0

