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

  io.stderr:write("Initializing application module\n")

  require "orca.UIKit"

  local project = filesystem.init(DATADIR)
  renderer.init(project.WindowWidth, project.WindowHeight, false)

  Application.load_plugins()

  if project.StartupViewController then
    Application.load_controller(project.StartupViewController, project.StartupRoute)
    self.screen = Application.screen
    self.controller = Application.controller
  else
    self.screen = Application.load_screen(project.StartupScreen)
    self.controller = { view = self.screen }
    Application.screen = self.screen
  end

  Application.load_editor()

  io.stderr:write("Application module initialized\n")
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

function Application:dispatch(req)
  local ctx = self:new_render_context(req)
  local body = self.router:dispatch(req)
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
  Application.app = class()
  Application.controller = Application.app:dispatch(route or "/")
  Application.screen = Application.controller.view
end

function Application.load_screen(path)
  io.stderr:write("Loading startup screen: " .. path .. "\n")
  local ok, class = pcall(require, path)
  assert(ok, "Failed to load screen: " .. path .. ", " .. tostring(class))
  local screen = class()
  Application.screen = screen
  return screen
end

function Application.load_editor()
  assert(Application.screen, "Screen must be loaded before editor")
  local ok, editor = pcall(require, "orca.EditorKit")
  if ok then
    editor.setScreen(Application.screen)
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
