local orca = require "orca"
local system = require "orca.system"
local core = {}

function core.init()
	require "orca.core2.project"
	io.stderr:write('Initializing core module\n')
	core.projects = {}
	core.load_project(DATADIR)
	core.load_plugins()
	-- orca.core = core
	-- orca.plugins = {}
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

return core