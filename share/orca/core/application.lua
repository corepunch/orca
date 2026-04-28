local core = require "orca.core"
local system = require "orca.system"
local filesystem = require "orca.filesystem"
local renderer = require "orca.renderer"

local Widget = require "orca.core.widget"
local Router = require "orca.core.router"
local UIKit = require "orca.UIKit"
local Startup = require "orca.core.startup"

local Application
Application = Widget:extend {
  layout = {
    content = function(element)
      local screen = UIKit.Screen()
      screen:addChild(element)
      return screen
    end
  },

  __init = function(self)
    Widget.__init(self)
    self.router = Router(self)
  end,

  new_render_context = function(self, req)
    return {
      req = req,
      content = {}
    }
  end,

  match = function(self, name, url, func)
    return self.router:add(name, url, func)
  end,

  resolve_body = function(self, body, route_info)
    if type(body) == "table" and body.render == true and route_info and self.views_prefix then
      local view_cls = require(self.views_prefix .. "/" .. route_info.name)
      return view_cls():content()
    end
    return body
  end,

  dispatch = function(self, req)
    local ctx = self:new_render_context(req)
    local route_info = self.router:resolve(req)
    local body = self:resolve_body(self.router:dispatch(req), route_info)
    ctx.content.inner = body

    local layout_def = self.layout
    local view = body

    if type(layout_def) == "table" and layout_def.__class then
      local layout = layout_def()
      layout:set_render_context(ctx)
      layout:include_helper(self)
      if type(layout.content) == "function" then
        view = layout:content()
      end
    elseif type(layout_def) == "table" and type(layout_def.content) == "function" then
      view = layout_def.content(ctx.content.inner, ctx, self)
    elseif type(layout_def) == "function" then
      view = layout_def(ctx.content.inner, ctx, self)
    elseif layout_def == nil and body ~= nil then
      local screen = UIKit.Screen()
      screen:addChild(body)
      view = screen
    end

    return {
      view = view,
      context = ctx
    }
  end,

  activate_controller = function(self, controller)
    if not controller or not controller.view then
      return nil
    end

    Application.app = self
    self.controller = controller
    self.screen = controller.view

    Application.load_editor(self.screen)
    self.screen:post("Window.Paint", renderer.getSize())

    return controller
  end,

  activate_route = function(self, req)
    return self:activate_controller(self:dispatch(req))
  end,

  run = function(self)
    for msg in system.getMessage do
      if filesystem.hasChangedFiles() then return DATADIR end
      if msg:is "Window.Closed" then return
      elseif msg:is "Node.KeyDown" and msg.key == "q" then return
      elseif msg:is "RequestReload" then return DATADIR
      else
        local ok, result = pcall(system.dispatchMessage, self.screen, msg)
        if not ok then
          io.stderr:write(tostring(result) .. "\n")
        elseif result and not msg:is "Window.Paint" then
          self.screen:post("Window.Paint", renderer.getSize())
        end
        system.translateMessage(msg)
      end
    end
  end,
}

Application.projects = {}

function Application.current(required)
  if required == nil then
    required = true
  end
  local app = Application.app
  if required then
    assert(app, "Application has not been opened yet")
  end
  return app
end

function Application.open(path)
  return Startup.open(Application, path)
end

return Application
