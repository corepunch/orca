-- Editor model: the project server process, the open documents (tabs), and a
-- small event bus that panels subscribe to.
--
-- Every open document keeps its own selection, undo/redo history and unsaved
-- state. Workspace.document, .selection, .undoStack, .redoStack and .dirty
-- always describe the active tab, so panels never need to know about tabs.
-- All tabs share one project runtime: documents reference each other's
-- prefabs, themes and data sources, which separate processes could not see.
local Xml = require "Editor.Scripts.Xml"
local config = require "Editor.Scripts.config"

local Workspace = {
	listeners = {},
	undoStack = {},
	redoStack = {},
	selection = nil,
	document = nil,
	dirty = false,
	documents = {},
	snapshotCount = 0,
}

local TAB_FIELDS = { 'selection', 'undoStack', 'redoStack', 'dirty' }

-- Events: log(level, text), document(path), select(path), changed(path),
-- structure(parent), saved(), tabs()
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

function Workspace.start(project, connection)
	Workspace.project = project or Workspace.projectPath()
	Workspace.connection = connection or Workspace.connection
	assert(Workspace.connection, "Editor runtime connection is missing")
	Workspace.connection:close()
	Workspace.connection.Executable = EXENAME
	Workspace.connection.Arguments = { { Value = '-server=true' }, { Value = Workspace.project } }
	Workspace.log('info', "Opened project %s", Workspace.project)
end

-- This configures the native source; responses never pass through Lua.
function Workspace.configureSource(source, request)
	if not source.ConnectionPath or source.ConnectionPath == '' then source.Connection = Workspace.connection end
	source.Source = request
	local ok = source:reload()
	if not ok then Workspace.log('error', "%s", source.ErrorMessage or 'Data source failed') end
	return ok
end

-- One request/response round trip. Returns the response root, or nil and
-- the server's error text.
function Workspace.request(method, route, path, args)
	if not Workspace.connection then return nil, "No project server" end
	local url = method .. "/" .. route .. (path and Workspace.encode(path, true) or "") .. query(args)
	local ok, reply = pcall(function()
		local reply = Workspace.connection:request(url)
		if not reply then error(Workspace.connection.ErrorMessage or 'Runtime request failed') end
		return reply
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

function Workspace.findDocument(path)
	for i, doc in ipairs(Workspace.documents) do
		if doc.path == path then return doc, i end
	end
end

-- Copies the active tab's state out of the mirrored fields, or back in.
local function stash()
	local doc = Workspace.findDocument(Workspace.document)
	if doc then for _, key in ipairs(TAB_FIELDS) do doc[key] = Workspace[key] end end
	return doc
end

local function restore(doc)
	Workspace.document = doc and doc.path
	for _, key in ipairs(TAB_FIELDS) do
		if doc then Workspace[key] = doc[key] end
	end
	if not doc then
		Workspace.selection, Workspace.undoStack, Workspace.redoStack, Workspace.dirty = nil, {}, {}, false
	end
end

-- Opens a document in a new tab after the active one, or shows its tab.
function Workspace.open(path)
	if not path then
		local root = Workspace.get('startup-screen')
		path = root and root:get'source'
	end
	if not path then
		Workspace.log('error', "No document to open")
		return
	end
	if Workspace.findDocument(path) then return Workspace.activate(path) end
	local _, index = Workspace.findDocument(Workspace.document)
	stash()
	local doc = { path = path, undoStack = {}, redoStack = {}, dirty = false }
	table.insert(Workspace.documents, (index or #Workspace.documents) + 1, doc)
	restore(doc)
	Workspace.log('info', "Opened %s", path)
	Workspace.emit('tabs')
	Workspace.emit('document', path)
	Workspace.select(path)
	return true
end

function Workspace.activate(path)
	local doc = Workspace.findDocument(path)
	if not doc then return false end
	if path == Workspace.document then return true end
	stash()
	restore(doc)
	Workspace.emit('tabs')
	Workspace.emit('document', path)
	Workspace.emit('select', Workspace.selection)
	return true
end

-- Moves to the next (1) or previous (-1) tab, wrapping around.
function Workspace.cycle(delta)
	local n = #Workspace.documents
	local _, index = Workspace.findDocument(Workspace.document)
	if n < 2 or not index then return false end
	return Workspace.activate(Workspace.documents[(index - 1 + delta) % n + 1].path)
end

function Workspace.isDirty(path)
	if not path or path == Workspace.document then return Workspace.dirty end
	local doc = Workspace.findDocument(path)
	return doc and doc.dirty or false
end

function Workspace.anyDirty()
	stash()
	if Workspace.dirty then return true end
	for _, doc in ipairs(Workspace.documents) do if doc.dirty then return true end end
	return false
end

-- Closes a tab. Unsaved edits are kept unless discard is set, in which case
-- the runtime forgets them and reloads the document from disk when reopened.
function Workspace.close(path, discard)
	path = path or Workspace.document
	local doc, index = Workspace.findDocument(path)
	if not doc then return false end
	if Workspace.isDirty(path) and not discard then return false, 'dirty' end
	if Workspace.isDirty(path) then
		local root, err = Workspace.request('post', 'revert/', path)
		if not root then
			Workspace.log('error', "Can't discard %s: %s", Workspace.leaf(path), err)
			return false, err
		end
		Workspace.log('warning', "Discarded unsaved changes to %s", Workspace.leaf(path))
	end
	local active = path == Workspace.document
	if not active then stash() end
	table.remove(Workspace.documents, index)
	if active then
		local next = Workspace.documents[math.min(index, #Workspace.documents)]
		restore(next)
		Workspace.emit('tabs')
		Workspace.emit('document', Workspace.document)
		Workspace.emit('select', Workspace.selection)
	else
		Workspace.emit('tabs')
	end
	return true
end

-- Forgets every tab, e.g. when another project is opened.
function Workspace.reset()
	Workspace.documents = {}
	restore(nil)
	Workspace.emit('tabs')
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

-- The open document containing a node path (the longest matching tab).
function Workspace.owner(path)
	local best
	for _, doc in ipairs(Workspace.documents) do
		if path and (path == doc.path or path:sub(1, #doc.path + 1) == doc.path .. '/')
			and (not best or #doc.path > #best.path) then best = doc end
	end
	return best
end

-- Marks the document that owns path (the active one by default) unsaved.
function Workspace.markDirty(path)
	local doc = path and Workspace.owner(path)
	if doc and doc.path ~= Workspace.document then
		if not doc.dirty then doc.dirty = true Workspace.emit('tabs') end
		return
	end
	local changed = not Workspace.dirty
	Workspace.dirty = true
	if changed then Workspace.emit('tabs') end
end

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
	-- History belongs to the edited document, even when it is not active.
	local doc = Workspace.owner(entry.path)
	if doc and doc.path ~= Workspace.document then
		push(doc.undoStack, entry)
		doc.redoStack = {}
	else
		push(Workspace.undoStack, entry)
		Workspace.redoStack = {}
	end
	Workspace.markDirty(entry.path)
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
	Workspace.markDirty(entry.path)
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
	Workspace.markDirty(parent)
	Workspace.log('info', "Created %s %s", class, name)
	Workspace.emit('structure', parent)
	Workspace.select(parent .. "/" .. name)
	return true
end

function Workspace.remove(path)
	local root, err = Workspace.request('delete', 'node/', path)
	if not root then return Workspace.log('error', "Delete: %s", err) end
	Workspace.markDirty(path)
	Workspace.log('info', "Deleted %s", Workspace.leaf(path))
	Workspace.emit('structure', Workspace.parentPath(path))
	Workspace.select(Workspace.parentPath(path))
	return true
end

function Workspace.duplicate(path)
	local root, err = Workspace.request('post', 'duplicate/', path)
	if not root then return Workspace.log('error', "Duplicate: %s", err) end
	Workspace.markDirty(path)
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

-- Saves one document (the active one by default); returns true when nothing
-- is left pending for it.
function Workspace.save(path)
	path = type(path) == 'string' and path or Workspace.document
	if not path then return end
	local root, err = Workspace.request('post', 'save/', path)
	if not root then return Workspace.log('error', "Save failed: %s", err) end
	local skipped = tonumber(root:get'skipped') or 0
	if path == Workspace.document then Workspace.dirty = skipped > 0
	else
		local doc = Workspace.findDocument(path)
		if doc then doc.dirty = skipped > 0 end
	end
	Workspace.log('ok', "Saved %s (%s changes)", root:get'file', root:get'applied')
	for n in root:each() do
		Workspace.log('warning', "Not saved: %s is not a named element in the source", n:get'path')
	end
	Workspace.emit('tabs')
	Workspace.emit('saved')
	return skipped == 0
end

function Workspace.saveAll()
	stash()
	local saved = 0
	for _, doc in ipairs(Workspace.documents) do
		if doc.dirty then
			Workspace.save(doc.path)
			saved = saved + 1
		end
	end
	if saved == 0 then Workspace.log('info', "Nothing to save") end
	return saved
end

------------------------------------------------------------------------------
-- Preview support

-- Renders the open document at its design size (width/height are the fallback
-- for screens without one); returns the image, its size and node rectangles.
function Workspace.snapshot(width, height)
	if not Workspace.document then return nil end
	local dir = os.getenv("TMPDIR") or "/tmp"
	Workspace.snapshotCount = Workspace.snapshotCount + 1
	local file = string.format("%s/orca-editor-preview-%d.png", dir, Workspace.snapshotCount)
	local root, err = Workspace.request('get', 'snapshot/', Workspace.document, {
		{ 'width', math.floor(width) }, { 'height', math.floor(height) }, { 'file', file }, { 'design', 1 },
	})
	if not root then
		Workspace.log('warning', "Preview: %s", err)
		return nil
	end
	if Workspace.lastSnapshot then os.remove(Workspace.lastSnapshot) end
	Workspace.lastSnapshot = file
	return file, Workspace.rects(root), tonumber(root:get'width') or width, tonumber(root:get'height') or height
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
