local core = require "orca.core"
local system = require "orca.system"
local filesystem = require "orca.filesystem"
local renderer = require "orca.renderer"

local Widget = require "orca.core.widget"
local Router = require "orca.core.router"
local UIKit = require "orca.UIKit"
local Startup = require "orca.core.startup"
local async = require "orca.core.async"

-- Collect before_filters from the class hierarchy (parent filters run first).
local function collect_before_filters(cls)
  if not cls then return {} end
  local result = collect_before_filters(rawget(cls, '__parent'))
  local own = rawget(cls, '__before_filters')
  if own then
    for _, f in ipairs(own) do result[#result + 1] = f end
  end
  return result
end

-- Default layout: wraps the inner content in a Screen.
local DefaultLayout = Widget:extend {
  content = function(self)
    local inner = self:content_for("inner")
    local screen = UIKit.Screen()
    if inner then screen:addChild(inner) end
    return screen
  end
}

local Application
Application = Widget:extend {
  layout = DefaultLayout,

  __init = function(self)
    Widget.__init(self)
    self.router = Router(self)
  end,

  new_render_context = function(self, req)
    return {
      req = req,
      slots = {}
    }
  end,

  match = function(self, name, url, func)
    return self.router:add(name, url, func)
  end,

  -- Generate a URL from a named route or return the path unchanged.
  -- Usage: self:url_for("home")  →  "/"
  url_for = function(self, name)
    return self.router:url_for(name)
  end,

  resolve_body = function(self, body, route_info)
    if type(body) == "table" and body.render == true and route_info and self.views_prefix then
      local view_cls = require(self.views_prefix .. "/" .. route_info.name)
      return view_cls()
    end
    return body
  end,

  dispatch = function(self, req)
    local ctx = self:new_render_context(req)

    -- Run before filters; first filter returning non-nil short-circuits the action.
    local body = nil
    local route_info = nil
    local cls = rawget(getmetatable(self) or {}, '__class')
    for _, filter in ipairs(collect_before_filters(cls)) do
      body = filter(self, req)
      if body ~= nil then break end
    end

    if body == nil then
      route_info = self.router:resolve(req)
      body = self:resolve_body(self.router:dispatch(req), route_info)
    end

    if type(body) == "table" then
      if type(body.set_render_context) == "function" then
        body:set_render_context(ctx)
      end
      if type(body.content) == "function" then
        local inner = body:content()
        if inner ~= nil and ctx.slots["inner"] == nil then
          ctx.slots["inner"] = inner
        end
      end
      if ctx.slots["title"] == nil and body.title ~= nil then
        ctx.slots["title"] = type(body.title) == "function" and body:title() or body.title
      end
      if ctx.slots["footer"] == nil and type(body.footer) == "function" then
        ctx.slots["footer"] = body:footer()
      end
      if ctx.slots["header"] == nil and type(body.header) == "function" then
        ctx.slots["header"] = body:header()
      end
    elseif body ~= nil then
      ctx.slots["inner"] = body
    end

    local layout_def = self.layout
    local view = body

    if type(layout_def) == "table" and layout_def.__class then
      local layout = layout_def()
      layout:set_render_context(ctx)
      layout:include_helper(self)
      if type(layout.content) == "function" then
        view = layout:content()
      end
    elseif type(layout_def) == "function" then
      view = layout_def(ctx, self)
    elseif layout_def == nil and body ~= nil then
      local screen = UIKit.Screen()
      local inner = (ctx.slots and ctx.slots.inner) or body
      screen:addChild(inner)
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

    self.controller = controller
    self.screen = controller.view
    Application.app = self

    Application.load_editor(self.screen)
    self.screen:post("Window.Paint", renderer.getSize())

    return controller
  end,

  activate_route = function(self, req)
    return self:activate_controller(self:dispatch(req))
  end,

  run = function(self)
    local startup_dark_theme = system.get_theme()
    for msg in system.getMessage do
      if filesystem.hasChangedFiles() then return DATADIR end
      if msg:is "Window.Closed" then return
      elseif msg:is "Node.KeyDown" and msg.key == "q" then return
      elseif msg:is "RequestReload" then return DATADIR
      else
        -- Detect OS appearance changes by polling is_dark_theme() on each
        -- paint event.  When the theme flips we trigger a full reload so the
        -- tailwind config (re-evaluated at startup) picks up the new colours.
        if msg:is "Window.Paint" and system.is_dark_theme() ~= startup_dark_theme then
          return DATADIR
        end
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

-- Class method: register a before filter.
-- Filters run in order before the route action on every dispatch.
-- If a filter returns a non-nil value it short-circuits the action.
--
-- MoonScript usage inside a class body:
--   @before_filter => @current_user = Users\find @session.user_id
-- Lua usage:
--   App:before_filter(function(self, req) self.current_user = ... end)
function Application.before_filter(cls, fn)
  assert(type(fn) == "function", "before_filter: expected a function")
  local filters = rawget(cls, '__before_filters')
  if not filters then
    filters = {}
    rawset(cls, '__before_filters', filters)
  end
  filters[#filters + 1] = fn
end

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

function Application.load_editor(screen)
  -- stub: override in editor environments to attach the editor host
end

-- Lapis-compatible helpers importable directly from orca.core.application:
--   import respond_to, capture_errors, yield_error from require "orca.core.application"
Application.respond_to    = async.respond_to
Application.capture_errors = async.capture_errors
Application.yield_error   = async.yield_error

return Application
