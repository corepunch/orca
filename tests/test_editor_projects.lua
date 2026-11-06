-- Project discovery uses real native directory checks and literal history files.
local orca = require "orca"
orca.init()
local disk = require "orca.filesystem.native"
local temp = os.tmpname()
os.remove(temp)
assert(disk.mkdir(temp))
assert(disk.mkdir(temp .. '/Project One'))
assert(disk.mkdir(temp .. '/Project Two'))
assert(disk.mkdir(temp .. '/Empty'))
local function write(path, text)
	local handle = assert(io.open(path, 'wb'))
	assert(handle:write(text))
	assert(handle:close())
end
-- Executable package contents must never run during catalog discovery.
write(temp .. '/Project One/package.lua', 'error("launcher evaluated package.lua")')
write(temp .. '/Project Two/package.lua', 'Name = "Second"')
local originalGetenv, originalDataDir = os.getenv, DATADIR
os.getenv = function(key)
	if key == 'ORCA_EDITOR_RECENTS' then return temp .. '/recents' end
	if key == 'ORCA_EDITOR_PROJECT' then return temp .. '/Project One' end
	if key == 'HOME' then return temp end
	return originalGetenv(key)
end
DATADIR = temp .. '/Editor'
local Projects = dofile('apps/Editor/Scripts/Projects.lua')

local ok, err = pcall(function()
	assert(Projects.validate('~/Project One/package.lua') == temp .. '/Project One')
	assert(Projects.validate(temp .. '/Empty/../Project Two/') == temp .. '/Project Two')
	assert(not Projects.validate(temp .. '/Empty'))
	assert(not Projects.validate(temp .. '/Missing'))
	assert(not Projects.validate(temp .. '/Project One\nforged'))
	assert(Projects.displayPath(temp .. '/Project One') == '~/Project One')
	assert(Projects.remember(temp .. '/Project One'))
	assert(Projects.remember(temp .. '/Project Two/package.lua'))
	assert(Projects.remember(temp .. '/Project One/'))
	local rows = Projects.list()
	assert(#rows == 2, 'History and bundled projects should be deduplicated')
	assert(rows[1].path == temp .. '/Project One' and rows[1].kind == 'recent')
	assert(rows[2].path == temp .. '/Project Two')
	local directories, path = Projects.directories(temp .. '/Empty/..')
	assert(path == temp and #directories == 3)
	assert(directories[1].title == 'Empty' and directories[1].kind == 'folder')
	assert(directories[2].title == 'Project One' and directories[2].kind == 'project')
	assert(not Projects.directories(temp .. '/Missing'))
	-- Bad records are skipped, and their contents are never interpreted as Lua.
	write(temp .. '/recents', 'error("not data")\n' .. temp .. '/Missing\n' .. temp .. '/Project Two\n')
	rows = Projects.list()
	assert(rows[1].path == temp .. '/Project Two' and rows[1].kind == 'recent')
	assert(rows[2].path == temp .. '/Project One' and rows[2].kind == 'configured')
	-- An unwritable history must report failure without preventing selection.
	os.getenv = function(key)
		if key == 'ORCA_EDITOR_RECENTS' then return temp .. '/Missing/recents' end
		return key == 'HOME' and temp or originalGetenv(key)
	end
	assert(not Projects.remember(temp .. '/Project One'))
end)

os.getenv, DATADIR = originalGetenv, originalDataDir
for _, file in ipairs { '/recents', '/recents.tmp', '/Project One/package.lua', '/Project Two/package.lua' } do
	os.remove(temp .. file)
end
for _, folder in ipairs { '/Project One', '/Project Two', '/Empty', '' } do os.remove(temp .. folder) end
assert(ok, err)
print('Editor project catalog: passed')
