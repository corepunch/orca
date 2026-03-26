local orca = require "orca"
local core = require "orca.core"
local system = require "orca.system"
local filesystem = require "orca.filesystem"
local renderer = require "orca.renderer"

local core = {}

function core.init()
	require "orca.core2.project"
	io.stderr:write("Initializing core module\n")
	core.projects = {}

	require "orca.UIKit"

	local project = filesystem.init(DATADIR)
	renderer.init(project.WindowWidth, project.WindowHeight, false)
	-- core.load_project(DATADIR)
	-- orca.core = core
	-- orca.plugins = {}
	core.load_plugins()
	core.screen = core.load_screen(project.StartupScreen)
	core.load_editor(core.screen)

	io.stderr:write("Core module initialized\n")
end

function core.load_screen(path)
	io.stderr:write("Loading startup screen: "..path.."\n")
	local ok, class = pcall(require, path)
	assert(ok, "Failed to load screen: "..path)
	return class()
end

function core.load_editor(screen)
	local ok, editor = pcall(require, "orca.editor")
	if ok then
		editor.setScreen(screen)
	else
		io.stderr:write("Failed to load editor module\n")
	end
end

function core.load_plugins()
	for path in system.list_dir(SHAREDIR.."/plugins") do
		if xpcall(dofile, print, SHAREDIR.."/plugins/"..path) then	
			io.stderr:write(string.format("Loaded plugin: %s\n", path))
		else
			io.stderr:write(string.format("Failed to load plugin: %s\n", path))
		end
	end
end

function core.run()
	while true do
		for msg in system.getMessage(core.screen) do
    	filesystem.trackChangedFiles()
			if msg:is "WindowClosed" then
				return
			elseif msg:is "KeyDown" and msg.Key == "q" then
				return
			elseif msg:is "RequestReload" then
				return DATADIR
			else
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