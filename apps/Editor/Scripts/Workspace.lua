-- Editor model: the project server process, the open document, selection,
-- undo/redo and a small event bus that panels subscribe to.
local fs = require "orca.filesystem"
local Xml = require "Editor.Scripts.Xml"
local config = require "Editor.Scripts.config"

local Workspace = {
	listeners = {},
	undoStack = {},
	redoStack = {},
	selection = nil,
	document = nil,
	snapshotCount = 0,
}

-- Events: log(level, text), document(path), select(path), changed(path)
function Workspace.on(event, fn)
	local list = Workspace.listeners[event] or {}
	Workspace.listeners[event] = list
	list[#list + 1] = fn
end

function Workspace.emit(event, ...)
	for _, fn in ipairs(Workspace.listeners[event] or {}) do fn(...) end
end

function Workspace.log(level, fmt, ...)
	local text = select('#', ...) > 0 and string.format(fmt, ...) or tostring(fmt)
	Workspace.emit('log', level, text)
end

-- Percent-encode a value; '/' stays readable inside node paths.
function Workspace.encode(s, keepSlash)
	local pattern = keepSlash and "[^%w%-%._~/]" or "[^%w%-%._~]"
	return (tostring(s):gsub(pattern, function(c) return string.format("%%%02X", c:byte()) end))
end

local function query(args)
	local parts = {}
	for _, kv in ipairs(args or {}) do
		parts[#parts + 1] = Workspace.encode(kv[1]) .. "=" .. Workspace.encode(kv[2])
	end
	return #parts > 0 and ("?" .. table.concat(parts, "&")) or ""
end

function Workspace.projectPath()
	local project = os.getenv("ORCA_EDITOR_PROJECT")
	return (project and project ~= '') and project or (DATADIR .. "/../Example")
end

function Workspace.start(project)
	Workspace.project = project or Workspace.projectPath()
	Workspace.process = fs.pipe.popen(EXENAME, "-server=true", Workspace.project)
	Workspace.log('info', "Opened project %s", Workspace.project)
end

-- One request/response round trip. Returns the response root, or nil and
-- the server's error text.
function Workspace.request(method, route, path, args)
	if not Workspace.process then return nil, "No project server" end
	local url = method .. "/" .. route .. (path and Workspace.encode(path, true) or "") .. query(args)
	local ok, reply = pcall(function()
		Workspace.process:write(url .. "\n")
		return Workspace.process:read()
	end)
	local root, err
	if ok then root, err = Xml.parse(reply) else err = reply end
	if not root then
		Workspace.log('error', "%s %s failed: %s", method, route, tostring(err))
		return nil, tostring(err)
	end
	if root:get'status' == 'error' then
		return nil, root.text or 'error', root
	end
	return root
end

function Workspace.get(route, path, args) return Workspace.request('get', route, path, args) end

------------------------------------------------------------------------------
-- Documents and selection

function Workspace.open(path)
	if not path then
		local root = Workspace.get('startup-screen')
		path = root and root:get'source'
	end
	if not path then
		Workspace.log('error', "No document to open")
		return
	end
	Workspace.document = path
	Workspace.selection = nil
	Workspace.log('info', "Opened %s", path)
	Workspace.emit('document', path)
	Workspace.select(path)
end

function Workspace.select(path)
	Workspace.selection = path
	Workspace.emit('select', path)
end

function Workspace.parentPath(path)
	return path and path:match("^(.*)/[^/]+$")
end

function Workspace.leaf(path)
	return path and (path:match("([^/]+)$") or path)
end

------------------------------------------------------------------------------
-- Edits with undo

local function push(stack, entry)
	stack[#stack + 1] = entry
	if #stack > config.max_undos then table.remove(stack, 1) end
end

local function apply(entry, value)
	if entry.kind == 'edit' then
		return Workspace.request('put', 'node/', entry.path, { { entry.name, value } })
	else
		return Workspace.request('put', 'binding/', entry.path, {
			{ 'property', entry.name }, { 'expression', value or '' }, { 'mode', entry.mode or 'Expression' },
		})
	end
end

local function commit(entry)
	local root, err = apply(entry, entry.new)
	if not root then
		Workspace.log('error', "%s.%s: %s", Workspace.leaf(entry.path), entry.name, err)
		return false, err
	end
	if entry.kind == 'edit' then
		for n in root:each() do entry.old = n:get'value' end
	else
		entry.old = root:get'previous'
	end
	push(Workspace.undoStack, entry)
	Workspace.redoStack = {}
	Workspace.dirty = true
	Workspace.emit('changed', entry.path)
	return true
end

function Workspace.setValue(path, name, value)
	local ok, err = commit { kind = 'edit', path = path, name = name, new = value }
	if ok then Workspace.log('info', "%s.%s = %s", Workspace.leaf(path), name, value) end
	return ok, err
end

function Workspace.setBinding(path, property, expression, mode)
	local ok, err = commit { kind = 'bind', path = path, name = property, new = expression, mode = mode }
	if ok then
		if expression and expression ~= '' then
			Workspace.log('info', "%s.%s bound to %s", Workspace.leaf(path), property, expression)
		else
			Workspace.log('info', "%s.%s binding removed", Workspace.leaf(path), property)
		end
	end
	return ok, err
end

local function replay(from, to, field)
	local entry = table.remove(from)
	if not entry then return false end
	local root, err = apply(entry, entry[field])
	if not root then
		Workspace.log('error', "Can't %s: %s", field == 'old' and 'undo' or 'redo', err)
		return false
	end
	push(to, entry)
	Workspace.emit('changed', entry.path)
	return true
end

function Workspace.undo() return replay(Workspace.undoStack, Workspace.redoStack, 'old') end
function Workspace.redo() return replay(Workspace.redoStack, Workspace.undoStack, 'new') end

------------------------------------------------------------------------------
-- Structure

function Workspace.create(parent, class, name)
	local root, err = Workspace.request('post', 'node/', parent, { { 'class', class }, { 'name', name } })
	if not root then return Workspace.log('error', "Create %s: %s", class, err) end
	Workspace.dirty = true
	Workspace.log('info', "Created %s %s", class, name)
	Workspace.emit('structure', parent)
	Workspace.select(parent .. "/" .. name)
	return true
end

function Workspace.remove(path)
	local root, err = Workspace.request('delete', 'node/', path)
	if not root then return Workspace.log('error', "Delete: %s", err) end
	Workspace.dirty = true
	Workspace.log('info', "Deleted %s", Workspace.leaf(path))
	Workspace.emit('structure', Workspace.parentPath(path))
	Workspace.select(Workspace.parentPath(path))
	return true
end

function Workspace.duplicate(path)
	local root, err = Workspace.request('post', 'duplicate/', path)
	if not root then return Workspace.log('error', "Duplicate: %s", err) end
	Workspace.dirty = true
	Workspace.log('info', "Duplicated %s", Workspace.leaf(path))
	Workspace.emit('structure', Workspace.parentPath(path))
	Workspace.select(Workspace.parentPath(path) .. "/" .. root:get'name')
	return true
end

function Workspace.rename(path, name)
	if not Workspace.setValue(path, 'Name', name) then return end
	Workspace.emit('structure', Workspace.parentPath(path))
	Workspace.select(Workspace.parentPath(path) .. "/" .. name)
end

function Workspace.save()
	if not Workspace.document then return end
	local root, err = Workspace.request('post', 'save/', Workspace.document)
	if not root then return Workspace.log('error', "Save failed: %s", err) end
	local skipped = tonumber(root:get'skipped') or 0
	Workspace.dirty = skipped > 0
	Workspace.log('ok', "Saved %s (%s changes)", root:get'file', root:get'applied')
	for n in root:each() do
		Workspace.log('warning', "Not saved: %s is not a named element in the source", n:get'path')
	end
	Workspace.emit('saved')
end

------------------------------------------------------------------------------
-- Preview support

function Workspace.snapshot(width, height)
	if not Workspace.document then return nil end
	local dir = os.getenv("TMPDIR") or "/tmp"
	Workspace.snapshotCount = Workspace.snapshotCount + 1
	local file = string.format("%s/orca-editor-preview-%d.png", dir, Workspace.snapshotCount)
	local root, err = Workspace.request('get', 'snapshot/', Workspace.document, {
		{ 'width', math.floor(width) }, { 'height', math.floor(height) }, { 'file', file },
	})
	if not root then
		Workspace.log('warning', "Preview: %s", err)
		return nil
	end
	if Workspace.lastSnapshot then os.remove(Workspace.lastSnapshot) end
	Workspace.lastSnapshot = file
	return file, Workspace.rects(root)
end

-- Screen-space rectangles of the document's nodes, deepest last.
function Workspace.rects(root)
	local rects = {}
	for r in root:each() do
		rects[#rects + 1] = {
			path = r:get'path',
			x = tonumber(r:get'x'), y = tonumber(r:get'y'),
			w = tonumber(r:get'w'), h = tonumber(r:get'h'),
		}
	end
	return rects
end

return Workspace
