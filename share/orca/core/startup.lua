local orca = require "orca"
local core = require "orca.core"
local filesystem = require "orca.filesystem"
local renderer = require "orca.renderer"
local system = require "orca.system"
local UIKit = require "orca.UIKit"

local Startup = {}

function Startup.load_screen(path)
  if not path then
    return UIKit.Screen()
  end
  io.stderr:write("Loading startup screen: " .. tostring(path) .. "\n")
  return filesystem.loadObject(path)
end

function Startup.load_controller(Application, path, route)
  assert(path, "StartupViewController is not set")
  io.stderr:write("Loading startup view controller: " .. tostring(path) .. "\n")
  local ok, class = pcall(require, path)
  assert(ok, "Failed to load view controller: " .. path .. ", " .. tostring(class))
  local app = class()
  app:activate_route(route or "/")
  return app
end

function Startup.load_plugin_config(name)
  orca.config[name] = setmetatable({}, { __index = _G })
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

function Startup.load_plugins()
  for path in system.list_dir(SHAREDIR .. "/plugins") do
    if path:match("%.lua$") then
      local plugin_name = path:match("^(.+)%.lua$")
      if plugin_name ~= "tailwind" then
        Startup.load_plugin_config(plugin_name)
      end
      if xpcall(dofile, print, SHAREDIR .. "/plugins/" .. path) then
        io.stderr:write(string.format("Loaded plugin %s\n", path))
      else
        io.stderr:write(string.format("Failed to load plugin %s\n", path))
      end
    end
  end
end

function Startup.open(Application, path)
  io.stderr:write("Initializing application module\n")

  require "orca.UIKit"

  local project = filesystem.init(path or DATADIR)
  renderer.init(project.WindowWidth, project.WindowHeight, false)

  Startup.load_plugins()

  local app
  local startup_view_controller = project.StartupViewController
  if startup_view_controller then
    app = Startup.load_controller(Application, startup_view_controller, project.StartupRoute)
  else
    app = Application()
    if project.StartupScreen then
      app.screen = Startup.load_screen(project.StartupScreen)
    else
      app.screen = UIKit.Screen()
    end
    app.controller = { view = app.screen }
  end

  Application.app = app

  io.stderr:write("Application module initialized\n")
  return app
end

return Startup
