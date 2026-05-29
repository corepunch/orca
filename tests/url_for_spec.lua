local test = require("orca.test")
local Widget = require("orca.core.widget")
local Application = require("orca.core.application")
local Game
do
  local _class_0
  local _base_0 = {
    url_params = function(self, req, ...)
      return "Adventure", nil, {
        game = self.id
      }, ...
    end
  }
  _base_0.__index = _base_0
  _class_0 = setmetatable({
    __init = function(self, attrs)
      if attrs == nil then
        attrs = { }
      end
      for k, v in pairs(attrs) do
        self[k] = v
      end
    end,
    __base = _base_0,
    __name = "Game"
  }, {
    __index = _base_0,
    __call = function(cls, ...)
      local _self_0 = setmetatable({}, _base_0)
      cls.__init(_self_0, ...)
      return _self_0
    end
  })
  _base_0.__class = _class_0
  Game = _class_0
end
local build_app
build_app = function()
  local App
  do
    local _class_0
    local _parent_0 = Application
    local _base_0 = {
      layout = Widget:extend({
        content = function()
          return "layout"
        end
      }),
      [{
        Adventure = "/adventure"
      }] = function()
        return "ok"
      end
    }
    _base_0.__index = _base_0
    setmetatable(_base_0, _parent_0.__base)
    _class_0 = setmetatable({
      __init = function(self, ...)
        return _class_0.__parent.__init(self, ...)
      end,
      __base = _base_0,
      __name = "App",
      __parent = _parent_0
    }, {
      __index = function(cls, name)
        local val = rawget(_base_0, name)
        if val == nil then
          local parent = rawget(cls, "__parent")
          if parent then
            return parent[name]
          end
        else
          return val
        end
      end,
      __call = function(cls, ...)
        local _self_0 = setmetatable({}, _base_0)
        cls.__init(_self_0, ...)
        return _self_0
      end
    })
    _base_0.__class = _class_0
    if _parent_0.__inherited then
      _parent_0.__inherited(_parent_0, _class_0)
    end
    App = _class_0
  end
  return App()
end
local application_url_for_named_route
application_url_for_named_route = function()
  local app = build_app()
  test.expect_eq(app:url_for("Adventure"), "/adventure", "app:url_for('Adventure') should return the named route path")
  return print("PASS: application_url_for_named_route")
end
local application_url_for_model_object
application_url_for_model_object = function()
  local app = build_app()
  local game = Game({
    id = "zork1"
  })
  test.expect_eq(app:url_for(game), "/adventure?game=zork1", "app:url_for(game) should use game:url_params()")
  return print("PASS: application_url_for_model_object")
end
local widget_url_for_delegates_to_application
widget_url_for_delegates_to_application = function()
  local previous = Application.app
  local app = build_app()
  Application.app = app
  local widget = Widget()
  local game = Game({
    id = "zork1"
  })
  test.expect_eq(widget:url_for("Adventure"), "/adventure", "widget:url_for should resolve named routes through the current app")
  test.expect_eq(widget:url_for(game), "/adventure?game=zork1", "widget:url_for should resolve model objects through the current app")
  Application.app = previous
  return print("PASS: widget_url_for_delegates_to_application")
end
application_url_for_named_route()
application_url_for_model_object()
widget_url_for_delegates_to_application()
return print("All url_for tests passed.")
