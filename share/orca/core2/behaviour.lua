local orca = require 'orca'
local Behaviour = {}

function Behaviour:loadImageAsync(url)
  orca.async(function (url)
    local network = require 'orca.network'
    self.Image = network.fetch(url):image()
  end, url)
end

function Behaviour:handleEvent(name, ...)
  if (not self[name] or not self[name](self, ...)) and self.parent then
    self.parent:handleEvent(name, ...)
  end
end

function Behaviour:loadView(filename, no_clear)
  if self.viewWillLoad then self:viewWillLoad(filename) end
  local ok, doc = pcall(require, filename)
  if ok then
    if not no_clear then self:clear() end
    self.view = doc()
    self:addChild(self.view, true)
    if self.viewDidLoad then self:viewDidLoad() end
    return self.view
  else
    io.stderr:write(string.format("%s: Can't load prefab\n", filename))
  end
end

function Behaviour:set(key, value)
  self[key] = value
end

function Behaviour:get(key)
  return self[key]
end

function Behaviour:async(...)
  orca.async(...) -- .error = function(error)
  --  self.error = error
  --  self:rebuild()
  -- end
end

function Behaviour:is(class)
  local cls = self
  while cls do
    if cls.__class == class or cls.__class.__name == class then
      return true
    end
    cls = getmetatable(cls)
  end
  return false
end

-- extended functionality

function Behaviour:populateInputs()
  local tbl = {}
  for e in self.children do
    if e.className == 'Input' then
      tbl[e.Name] = e.Text
    end
  end
  return tbl
end

function Behaviour:findParentOfType(class)
  for n in self.lineage do
    if n:is(class) then return n end
  end
end

-- class definition in line with how MoonScript generates classes

function Behaviour:extend(_base_0)
  local _parent_0 = self
  _base_0 = _base_0 or {}
  _base_0.__index = _base_0
  setmetatable(_base_0, _parent_0.__base)
  local _class_0 = setmetatable({
    __init = function(self_1, ...)
      return (rawget(_base_0, '__init') or _parent_0.__init)(self_1, ...)
    end,
    __base = _base_0,
    __name = 'LuaBehaviour',
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

local _base_0 = Behaviour
_base_0.__index = _base_0
local _class_0 = setmetatable({
  __init = function(self, ...)
    local _base_1 = getmetatable(self)
    _base_1.__add = function(_self_0, ...) _self_0:addChild(...) return ... end
    _base_1.__call = function(_self_0, ...) return _self_0:instantiate(...) end
    _base_1.__index = function(_self_0, key)
      local ud = rawget(_self_0, '__userdata')
      local mt = getmetatable(_base_1) or {}
      local vl = 
				(ud and (orca.core.Object[key] or orca.core.Object.__getproperty(_self_0, key))) or
				rawget(_base_1, key) or mt[key] or (mt.__class and mt.__class[key]) -- this is needed to access inherited classes (i.e. ui.Node2D)
      if type(vl) == 'function' then
        return function (_, ...) return vl(_self_0, ...) end
      else
        return vl
      end
    end
    
    _base_1.__newindex = function(_self_0, key, value)
      local ud = rawget(_self_0, '__userdata')
      local set = orca.core.Object.__setproperty
      return ud and set(_self_0, key, value) or rawset(_self_0, key, value)
    end

    orca.core.Object.new(self, ...)
    self.print = print
    self.assert = assert
    self.string = string
    self.ipairs = ipairs
    self.pairs = pairs
    self:rebuild()
  end,
  __base = _base_0,
  __name = 'Behaviour'
}, {
  __index = function(cls, name)
    local ud = rawget(cls, '__userdata')
    return ud and ud[name] or rawget(_base_0, name)
  end,
  __call = function(cls, ...)
    local _self_0 = setmetatable({}, _base_0)
    cls.__init(_self_0, ...)
    return _self_0
  end
})
_base_0.__class = _class_0
return _class_0

