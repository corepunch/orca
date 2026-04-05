local orca = require "orca"
local core = require "orca.core"
local system = require "orca.system"
local filesystem = require "orca.filesystem"
local renderer = require "orca.renderer"

local core = {}

function core.init()
	io.stderr:write("Initializing core module\n")

	require "orca.UIKit"

	local project = filesystem.init(DATADIR)
	renderer.init(project.WindowWidth, project.WindowHeight, false)

	-- require "orca.core2.project"
	-- core.projects = {}
	-- core.load_project(DATADIR)
	core.load_plugins()
	core.load_screen(project.StartupScreen)
	core.load_editor()

	io.stderr:write("Core module initialized\n")
end

function core.load_screen(path)
	io.stderr:write("Loading startup screen: "..path.."\n")
	local ok, class = pcall(require, path)
	assert(ok, "Failed to load screen: "..path)
	core.screen = class()
end

function core.load_editor()
	assert(core.screen, "Screen must be loaded before editor")
	local ok, editor = pcall(require, "orca.editor")
	if ok then
		editor.setScreen(core.screen)
	else
		io.stderr:write("Failed to load editor module\n")
	end
end

function core.load_plugin_config(name)
	orca.config[name] = {}
	if not filesystem.getWorkspace() then return end
	for node in filesystem.getWorkspace().children do
		local filename = filesystem.joinPaths(node.Name, 'config/'..name..".lua")
		-- print("Checking for plugin config:", filename)
		-- local chunk = loadfile(filename, "t", core.config[name])
		local file = filesystem.readTextFile(filename)
		local chunk = file and load(file, "@"..filename, "t", orca.config[name])
		if chunk then
			print("Loading plugin config", filename)
			local ok, err = pcall(chunk)
			if not ok then
				io.stderr:write(string.format("Error loading config '%s': %s\n", filename, err))
			end
		end
	end
end

function core.load_plugins()
	for path in system.list_dir(SHAREDIR.."/plugins") do
		if path:match("%.lua$") then
			if xpcall(dofile, print, SHAREDIR.."/plugins/"..path) then
				io.stderr:write(string.format("Loaded plugin %s\n", path))
				core.load_plugin_config(path:match("^(.+)%.lua$"))
			else
				io.stderr:write(string.format("Failed to load plugin %s\n", path))
			end
		end
	end
end

function core.run()
	while true do
		for msg in system.getMessage(core.screen) do
    	if filesystem.hasChangedFiles() then return DATADIR end
			if msg:is "Window.Closed" then return
			elseif msg:is "Keyboard.KeyDown" and msg.key == "q" then return
			elseif msg:is "RequestReload" then return DATADIR
			else
				system.translateMessage(msg)
				local ok, result = pcall(system.dispatchMessage, msg)
				if not ok then
					io.stderr:write(tostring(result) .. "\n")
					-- core.screen:clear() 
					-- core.screen:addChild(ui.TextBlock(result))
				elseif result and not msg:is "WindowPaint" then
					core.screen:postMessage "WindowPaint"
				end
			end
		end
	end
end

return core
