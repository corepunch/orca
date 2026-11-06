-- Local project discovery and a small, data-only recent-project history.
-- Package files are checked for existence, never evaluated by the launcher.
local disk = require "orca.filesystem.native"
local Projects = {}
local MAX_RECENT = 12

local function normalize(path)
	if type(path) ~= 'string' then return nil end
	path = path:match('^%s*(.-)%s*$')
	if path == '' or path:find('[%z\r\n]') then return nil end
	local home = os.getenv('HOME')
	if home and (path == '~' or path:sub(1, 2) == '~/') then path = home .. path:sub(2) end
	path = path:gsub('/package%.lua/?$', '')
	local prefix = path:sub(1, 1) == '/' and '/' or ''
	local parts = {}
	for part in path:gmatch('[^/]+') do
		if part == '..' and #parts > 0 and parts[#parts] ~= '..' then
			table.remove(parts)
		elseif part ~= '.' and (part ~= '..' or prefix == '' or #parts > 0) then
			parts[#parts + 1] = part
		end
	end
	local result = prefix .. table.concat(parts, '/')
	return result == '' and '.' or result
end

local function historyPath()
	local override = os.getenv('ORCA_EDITOR_RECENTS')
	if override and override ~= '' then return override end
	local home = os.getenv('HOME')
	return home and (home .. '/.orca-editor-recent-projects')
end

function Projects.validate(path)
	path = normalize(path)
	if not path or path == '' then return nil, 'Choose a project folder or its package.lua file.' end
	if not disk.direxists(path) then return nil, 'This project folder could not be found.' end
	local info = disk.getfileinfo(path .. '/package.lua')
	if not info or info.type ~= 'file' then return nil, 'Choose an ORCA project folder containing package.lua.' end
	return path
end

function Projects.displayPath(path)
	local home = os.getenv('HOME')
	if home and path:sub(1, #home + 1) == home .. '/' then return '~' .. path:sub(#home + 1) end
	return path
end

local function recentPaths()
	local paths, seen = {}, {}
	local file = historyPath()
	if not file then return paths end
	local handle = io.open(file, 'rb')
	if not handle then return paths end
	-- Bound damaged or hand-edited history; each line is a literal path.
	local content = handle:read(65536) or ''
	handle:close()
	for line in content:gmatch('[^\r\n]+') do
		local path = Projects.validate(line)
		if path and not seen[path] then
			paths[#paths + 1], seen[path] = path, true
			if #paths == MAX_RECENT then break end
		end
	end
	return paths
end

function Projects.remember(path)
	local valid, err = Projects.validate(path)
	if not valid then return nil, err end
	local file = historyPath()
	if not file then return nil, 'Recent projects could not be saved: no home directory is available.' end
	local paths = { valid }
	for _, previous in ipairs(recentPaths()) do
		if previous ~= valid and #paths < MAX_RECENT then paths[#paths + 1] = previous end
	end
	local handle, openError = io.open(file .. '.tmp', 'wb')
	if not handle then return nil, openError end
	local ok, writeError = handle:write(table.concat(paths, '\n') .. '\n')
	local closed, closeError = handle:close()
	if not ok or not closed then
		os.remove(file .. '.tmp')
		return nil, writeError or closeError
	end
	local renamed, renameError = os.rename(file .. '.tmp', file)
	if not renamed then os.remove(file .. '.tmp') return nil, renameError end
	return true
end

local function row(path, kind)
	return {
		title = path:match('([^/]+)$') or path,
		path = path,
		kind = kind,
		description = Projects.displayPath(path),
	}
end

function Projects.list()
	local rows, seen = {}, {}
	local function add(path, kind)
		path = Projects.validate(path)
		if path and not seen[path] then
			rows[#rows + 1], seen[path] = row(path, kind), true
		end
	end
	for _, path in ipairs(recentPaths()) do add(path, 'recent') end
	add(os.getenv('ORCA_EDITOR_PROJECT'), 'configured')
	local editor = normalize(DATADIR or 'apps/Editor')
	local root = normalize(editor .. '/..')
	local samples = {}
	-- iterateDirectory is a native API and never invokes a shell.
	local ok = pcall(function()
		for path, isDirectory in disk.iterateDirectory(root) do
			if isDirectory and normalize(path) ~= editor and Projects.validate(path) then
				samples[#samples + 1] = path
			end
		end
	end)
	if ok then
		table.sort(samples, function(a, b) return a:lower() < b:lower() end)
		for _, path in ipairs(samples) do add(path, 'sample') end
	end
	return rows
end

-- Folder browsing for the launcher's in-app chooser. Hidden folders remain
-- accessible through its path field; the native iterator omits dot entries.
function Projects.directories(path)
	path = normalize(path)
	if not path or path == '' then return nil, 'Enter a folder path.' end
	if not disk.direxists(path) then return nil, 'This folder could not be found.' end
	local rows = {}
	local ok, err = pcall(function()
		for child, isDirectory in disk.iterateDirectory(path, true) do
			if isDirectory then
				rows[#rows + 1] = row(normalize(child), Projects.validate(child) and 'project' or 'folder')
			end
		end
	end)
	if not ok then return nil, tostring(err) end
	table.sort(rows, function(a, b) return a.title:lower() < b.title:lower() end)
	return rows, path
end

return Projects
