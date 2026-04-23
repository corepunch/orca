local orca = require "orca"
local core = require "orca.core"
local system = require "orca.system"
local filesystem = require "orca.filesystem"
local renderer = require "orca.renderer"

local Object = require "orca.core.object"
local Router = require "orca.core.router"
local UIKit = require "orca.UIKit"

local Application = Object:extend {
  layout = {
    content = function(element)
      local screen = UIKit.Screen()
      screen:addChild(element)
      return screen
    end
  }
}

Application.projects = {}

function Application:__init()
  Object.__init(self)
  self.router = Router(self)
end

function Application.open(path)
  io.stderr:write("Initializing application module\n")

  require "orca.UIKit"

  local project = filesystem.init(path or DATADIR)
  renderer.init(project.WindowWidth, project.WindowHeight, false)

  Application.load_plugins()

  local app
  local startup_view_controller = project.StartupViewController
  if startup_view_controller then
    app = Application.load_controller(startup_view_controller, project.StartupRoute)
  else
    app = Application()
    app.screen = Application.load_screen(project.StartupScreen)
    app.controller = { view = app.screen }
  end

  Application.app = app
  Application.screen = app.screen
  Application.controller = app.controller

  Application.load_editor(app.screen)

  io.stderr:write("Application module initialized\n")
  return app
end

function Application:new_render_context(req)
  return {
    req = req,
    content = {}
  }
end

function Application:match(name, url, func)
  return self.router:add(name, url, func)
end

function Application:resolve_body(body, route_info)
  if type(body) == "table" and body.render == true and route_info and self.views_prefix then
    local view_cls = require(self.views_prefix .. "/" .. route_info.name)
    return view_cls():content()
  end
  return body
end

function Application:dispatch(req)
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
end

function Application.load_controller(path, route)
  io.stderr:write("Loading startup view controller: " .. path .. "\n")
  local ok, class = pcall(require, path)
  assert(ok, "Failed to load view controller: " .. path .. ", " .. tostring(class))
  local app = class()
  app.controller = app:dispatch(route or "/")
  app.screen = app.controller.view
  return app
end

function Application.load_screen(path)
  io.stderr:write("Loading startup screen: " .. path .. "\n")
  return filesystem.loadXml(path)
end

function Application.load_editor(screen)
  assert(screen, "Screen must be loaded before editor")
  local ok, editor = pcall(require, "orca.EditorKit")
  if ok then
    editor.setScreen(screen)
  else
    io.stderr:write(string.format("Failed to load editor module %s\n", tostring(editor)))
  end
end

function Application.load_plugin_config(name)
  orca.config[name] = {}
  if not filesystem.getWorkspace() then return end
  for node in filesystem.getWorkspace().children do
    local filename = filesystem.joinPaths(node.Name, "config/" .. name .. ".lua")
    local file = filesystem.readTextFile(filename)
    local chunk = file and load(file, "@" .. filename, "t", orca.config[name])
    if chunk then
      print("Loading plugin config", filename)
      local ok, err = pcall(chunk)
      if not ok then
        io.stderr:write(string.format("Error loading config '%s': %s\n", filename, err))
      end
    end
  end
end

function Application.load_plugins()
  for path in system.list_dir(SHAREDIR .. "/plugins") do
    if path:match("%.lua$") then
      Application.load_plugin_config(path:match("^(.+)%.lua$"))
      if xpcall(dofile, print, SHAREDIR .. "/plugins/" .. path) then
        io.stderr:write(string.format("Loaded plugin %s\n", path))
      else
        io.stderr:write(string.format("Failed to load plugin %s\n", path))
      end
    end
  end
end

function Application:run()
  while true do
    for msg in system.getMessage(self.screen) do
      if filesystem.hasChangedFiles() then return DATADIR end
      if msg:is "Window.Closed" then return
      elseif msg:is "Node.KeyDown" and msg.key == "q" then return
      elseif msg:is "RequestReload" then return DATADIR
      else
        system.translateMessage(msg)
        local ok, result = pcall(system.dispatchMessage, msg)
        if not ok then
          io.stderr:write(tostring(result) .. "\n")
        elseif result and not msg:is "Window.Paint" then
          self.screen:post("Window.Paint", renderer.getSize())
        end
      end
    end
  end
end

return Application
