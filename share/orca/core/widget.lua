local Widget = {}

local function get_helpers(instance)
  local helpers = rawget(instance, '__helpers')
  if not helpers then
    helpers = {}
    rawset(instance, '__helpers', helpers)
  end
  return helpers
end

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

function Widget:include_helper(helper)
  assert(type(helper) == 'table', 'helper must be a table')
  local helpers = get_helpers(self)
  helpers[#helpers + 1] = helper
  return helper
end

function Widget:include_helpers(...)
  local argc = select('#', ...)
  if argc == 1 and type((...)) == 'table' and not (...)['__class'] then
    local list = (...)
    if #list > 0 then
      for i = 1, #list do
        self:include_helper(list[i])
      end
    else
      self:include_helper(list)
    end
    return self
  end
  for i = 1, argc do
    self:include_helper(select(i, ...))
  end
  return self
end

function Widget:set_render_context(ctx)
  rawset(self, '__render_ctx', ctx)
  return ctx
end

function Widget:get_render_context()
  return rawget(self, '__render_ctx')
end

function Widget:content_for(name, value)
  local ctx = self:get_render_context()
  assert(type(name) == 'string', 'content_for name must be a string')
  assert(ctx and type(ctx) == 'table', 'content_for called without render context')
  ctx.content = ctx.content or {}
  if value == nil then
    return ctx.content[name]
  end
  ctx.content[name] = value
  return value
end

function Widget:has_content_for(name)
  local ctx = self:get_render_context()
  return type(ctx) == 'table' and type(ctx.content) == 'table' and ctx.content[name] ~= nil
end

function Widget:_find_helper_value(key)
  local function scan_helpers(helpers)
    if not helpers then return nil end
    for i = #helpers, 1, -1 do
      local helper = helpers[i]
      local value = helper and helper[key]
      if value ~= nil then
        if type(value) == 'function' then
            local needs_helper_self = rawget(helper, key) == value
            return function(_, ...)
              if needs_helper_self then
                return value(helper, ...)
              end
              return value(...)
          end
        end
        return value
      end
    end
    return nil
  end

  local value = scan_helpers(rawget(self, '__helpers'))
  if value ~= nil then return value end

  local cls = rawget(self, '__class') or rawget(getmetatable(self) or {}, '__class')
  while cls do
    value = scan_helpers(rawget(cls, '__helpers'))
    if value ~= nil then return value end
    cls = rawget(cls, '__parent')
  end

  return nil
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
      local vl = rawget(_base_1, key)
        or (cls and cls[key])
        or mt[key]
        or (mt.__class and mt.__class[key])
        or _base_1._find_helper_value(_self_0, key)
      if type(vl) == 'function' then
        return function (...)
          local argc = select('#', ...)
          local first = argc > 0 and select(1, ...) or nil
          if first == _self_0 then
            return vl(...)
          end
          return vl(_self_0, ...)
        end
      else
        return vl
      end
    end
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
