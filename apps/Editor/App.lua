-- ORCA Editor: menu bar, Hierarchy/Library on the left, document tabs above
-- the live Preview and Console in the middle, Inspector on the right, status
-- bar at the bottom.
-- The edited project runs in a separate `orca -server=true` process
-- (plugins/EditorKit/server.c); set ORCA_EDITOR_PROJECT to choose it.
local Application = require "orca.core.application"
local fs = require "orca.filesystem"
local Native = require "Editor.Scripts.Native"
local Workspace = require "Editor.Scripts.Workspace"
local Popups = require "Editor.Scripts.Popups"
local Panel = require "Editor.Scripts.Panel"
local DataPanel = require "Editor.Scripts.DataPanel"
local MenuBar = require "Editor.Scripts.MenuBar"
local Hierarchy = require "Editor.Scripts.Hierarchy"
local ProjectView = require "Editor.Scripts.ProjectView"
local Inspector = require "Editor.Scripts.Inspector"
local Preview = require "Editor.Scripts.Preview"
local Console = require "Editor.Scripts.Console"
local StatusBar = require "Editor.Scripts.StatusBar"
local BindingEditor = require "Editor.Scripts.BindingEditor"
local Dialog = require "Editor.Scripts.Dialog"
local Welcome = require "Editor.Scripts.Welcome"
local DocumentTabs = require "Editor.Scripts.DocumentTabs"

local SHORTCUTS = {
	{ 'Ctrl+O', 'Choose project' },
	{ 'Ctrl+S', 'Save document' },
	{ 'Ctrl+Shift+S', 'Save all documents' },
	{ 'Ctrl+W', 'Close document tab' },
	{ 'Ctrl+Tab', 'Next tab (Ctrl+Shift+Tab: previous)' },
	{ 'Ctrl+P', 'Find resource in the Library' },
	{ 'Ctrl+Z', 'Undo' },
	{ 'Ctrl+Shift+Z', 'Redo (also Ctrl+Y)' },
	{ 'Ctrl+B', 'Edit binding of the selected property' },
	{ 'Ctrl+D', 'Duplicate selected node' },
	{ 'F2', 'Rename selected node' },
	{ 'Delete', 'Delete selected node (Hierarchy)' },
	{ 'Ctrl+F', 'Find in Hierarchy (or just type)' },
	{ 'F5', 'Refresh preview' },
	{ 'Ctrl+1..4', 'Focus Hierarchy, Library, Inspector, Console' },
	{ 'Alt+F/E/C/V/H', 'Open a menu' },
	{},
	{ 'Binding editor' },
	{ 'Ctrl+Enter', 'Apply binding' },
	{ 'Ctrl+Space', 'Complete function or {property}' },
	{ 'Tab', 'Accept completion' },
	{ 'Esc', 'Dismiss completion, then close' },
}

local App = Application:extend {
	layout = function(self, context) return context.slots.inner end,

	["/"] = function(self) return self:build() end,

	afterPaint = function(self)
		if self.welcome.view.Visible then self.welcome:update() return end
		if not self.started then
			self.started = true
			self.project:reload()
			Workspace.open()
		end
		self.preview:update()
		DataPanel.autoExpand(self.hierarchy)
		DataPanel.autoExpand(self.project)
		local popup = Popups.top()
		if popup and popup.checkResize then popup:checkResize() end
	end,
}

function App:focusPanel(panel)
	if self.activePanel == panel then return end
	if self.activePanel then
		self.activePanel.active = false
		if self.activePanel.header then self.activePanel.header:invalidate() end
	end
	self.activePanel = panel
	if panel and panel.header then
		panel.active = true
		panel.header:invalidate()
	end
end

function App:activate(panel)
	self:focusPanel(panel)
	if panel and panel.focus then panel:focus() end
end

function App:editBinding()
	local item = self.inspector:selectedItem()
	if item and not item.IsGroup then
		self.inspector:editBinding(item)
	else
		Workspace.log('warning', "Select a property in the Inspector, then press Ctrl+B")
	end
end

function App:menus()
	local app = self
	local function selection() return Workspace.selection end
	local function canEdit() return selection() and selection() ~= Workspace.document end
	local function documents()
		local items = app.project:documents()
		if #items == 0 then items[1] = { label = 'No documents', disabled = true } end
		return items
	end
	local function noTab() return not Workspace.document end
	return {
		{ title = 'File', items = function() return {
			{ label = 'Choose Project…', shortcut = 'Ctrl+O', action = function() app:showWelcome() end },
			{ separator = true },
			{ label = 'Open Document', items = documents() },
			{ label = 'Close Tab', shortcut = 'Ctrl+W', disabled = noTab(), action = function() app:closeDocument() end },
			{ label = 'Close All Tabs', disabled = noTab(), action = function() app:closeAll() end },
			{ separator = true },
			{ label = 'Save', shortcut = 'Ctrl+S', disabled = noTab(), action = function() Workspace.save() end },
			{ label = 'Save All', shortcut = 'Ctrl+Shift+S', disabled = noTab(), action = Workspace.saveAll },
			{ separator = true },
			{ label = 'Reload Project', action = function() app:restartServer() end },
			{ separator = true },
			{ label = 'Quit', shortcut = 'Ctrl+Q', action = function() app:quit() end },
		} end },
		{ title = 'Edit', items = function() return {
			{ label = 'Undo', shortcut = 'Ctrl+Z', disabled = #Workspace.undoStack == 0, action = Workspace.undo },
			{ label = 'Redo', shortcut = 'Ctrl+Shift+Z', disabled = #Workspace.redoStack == 0, action = Workspace.redo },
			{ separator = true },
			{ label = 'Rename', shortcut = 'F2', disabled = not canEdit(), action = function() app.hierarchy:rename(selection()) end },
			{ label = 'Duplicate', shortcut = 'Ctrl+D', disabled = not canEdit(), action = function() Workspace.duplicate(selection()) end },
			{ label = 'Delete', shortcut = 'Del', disabled = not canEdit(), action = function() Workspace.remove(selection()) end },
			{ separator = true },
			{ label = 'Edit Binding...', shortcut = 'Ctrl+B', action = function() app:editBinding() end },
			{ label = 'Find in Hierarchy', shortcut = 'Ctrl+F', action = function()
				app:activate(app.hierarchy)
				app.hierarchy:startSearch('')
			end },
		} end },
		{ title = 'Create', items = function()
			local parent = selection() or Workspace.document
			if not parent then return { { label = 'Open a document first', disabled = true } } end
			return app.hierarchy:classMenu(parent)
		end },
		{ title = 'View', items = function() return {
			{ label = 'Refresh Preview', shortcut = 'F5', action = function() app.preview:refresh() end },
			{ separator = true },
			{ label = 'Next Tab', shortcut = 'Ctrl+Tab', disabled = #Workspace.documents < 2, action = function() Workspace.cycle(1) end },
			{ label = 'Previous Tab', shortcut = 'Ctrl+Shift+Tab', disabled = #Workspace.documents < 2, action = function() Workspace.cycle(-1) end },
			{ separator = true },
			{ label = 'Library Panel', checked = app.showProject, action = function() app:toggle('showProject') end },
			{ label = 'Group by Library', checked = app.project.mode == 'libraries', action = function()
				app.project:setMode(app.project.mode == 'libraries' and 'files' or 'libraries')
			end },
			{ label = 'Console Panel', checked = app.showConsole, action = function() app:toggle('showConsole') end },
			{ separator = true },
			{ label = 'Expand All', action = function() app.hierarchy:expandAll(true) end },
			{ label = 'Collapse All', action = function() app.hierarchy:expandAll(false) end },
		} end },
		{ title = 'Help', items = function() return {
			{ label = 'Keyboard Shortcuts', shortcut = 'F1', action = function() Dialog.show('Keyboard Shortcuts', SHORTCUTS) end },
			{ label = 'About ORCA Editor', action = function()
				Dialog.show('About ORCA Editor', {
					{ 'Edit ORCA projects live: the project runs in a server process' },
					{ 'and every change is applied to the running scene.' },
					{},
					{ 'Project', Workspace.project or '' },
				})
			end },
		} end },
	}
end

function App:toggle(key)
	self[key] = not self[key]
	self.left.Rows = self.showProject and "1fr 1fr" or "1fr 0px"
	self.center.Rows = self.showConsole and "30px 1fr 190px" or "30px 1fr 0px"
	self.preview.stale = true
end

-- Asks before an action that would lose unsaved edits in any tab.
function App:whenSaved(title, verb, action)
	if not Workspace.anyDirty() then return action() end
	Dialog.confirm(title, 'Some open documents have unsaved changes. Save them before you ' .. verb .. '?', {
		primary = { label = 'Save All', action = function()
			Workspace.saveAll()
			if Workspace.anyDirty() then
				Workspace.log('warning', "Some changes could not be saved; see the Console")
			else
				action()
			end
		end },
		secondary = { label = "Don't Save", action = action },
	})
	return false
end

-- Restarts the runtime and reopens the same tabs from disk.
function App:restartServer()
	return self:whenSaved('Reload project', 'reload', function()
		local paths, active = {}, Workspace.document
		for _, doc in ipairs(Workspace.documents) do paths[#paths + 1] = doc.path end
		Workspace.reset()
		Workspace.start(Workspace.project)
		BindingEditor.functions = nil
		self.project:reload()
		if #paths == 0 then Workspace.open() end
		for _, path in ipairs(paths) do Workspace.open(path) end
		if active then Workspace.activate(active) end
	end)
end

-- Closes a tab, asking first when the document has unsaved changes.
function App:closeDocument(path)
	path = path or Workspace.document
	if not path then return false end
	if not Workspace.isDirty(path) then return Workspace.close(path) end
	Dialog.confirm('Save changes?', 'Save the changes to ' .. Workspace.leaf(path) .. ' before closing it?', {
		primary = { label = 'Save', action = function()
			if Workspace.save(path) then Workspace.close(path)
			else Workspace.log('warning', "%s still has changes that could not be saved", Workspace.leaf(path)) end
		end },
		secondary = { label = "Don't Save", action = function() Workspace.close(path, true) end },
	})
	return false
end

function App:closeAll()
	return self:whenSaved('Close all tabs', 'close them', function()
		for i = #Workspace.documents, 1, -1 do Workspace.close(Workspace.documents[i].path, true) end
	end)
end

function App:quit()
	return self:whenSaved('Quit ORCA Editor', 'quit', function() os.exit(0) end)
end

-- discard skips the unsaved-changes question (it was already answered).
function App:showWelcome(discard)
	if not discard and Workspace.anyDirty() then
		self:whenSaved('Choose another project', 'switch projects', function() self:showWelcome(true) end)
		return false
	end
	Popups.closeAll()
	self.editorRoot.Visible = false
	self.welcome:show(self.projectOpened)
	return true
end

function App:openProject(path)
	Workspace.reset()
	Workspace.start(path, self.runtime)
	BindingEditor.functions = nil
	self.preview.image.Src = ''
	self.preview.rects, self.preview.size, self.preview.stale = {}, nil, true
	self.preview.selection.Visible, self.preview.hover.Visible = false, false
	self.started, self.projectOpened = false, true
	self.welcome.view.Visible = false
	self.editorRoot.Visible = true
	self:activate(self.hierarchy)
	return true
end

function App:hotkey(chord)
	if not chord then return false end
	local actions = {
		['ctrl+o'] = function() self:showWelcome() end,
		['ctrl+s'] = function() Workspace.save() end,
		['ctrl+shift+s'] = Workspace.saveAll,
		['ctrl+w'] = function() self:closeDocument() end,
		['ctrl+tab'] = function() Workspace.cycle(1) end,
		['ctrl+shift+tab'] = function() Workspace.cycle(-1) end,
		['ctrl+pagedown'] = function() Workspace.cycle(1) end,
		['ctrl+pageup'] = function() Workspace.cycle(-1) end,
		['ctrl+p'] = function() self:activate(self.project) self.project:startSearch('') end,
		['ctrl+z'] = Workspace.undo,
		['ctrl+shift+z'] = Workspace.redo,
		['ctrl+y'] = Workspace.redo,
		['ctrl+b'] = function() self:editBinding() end,
		['ctrl+d'] = function()
			if Workspace.selection and Workspace.selection ~= Workspace.document then Workspace.duplicate(Workspace.selection) end
		end,
		['f2'] = function() self.hierarchy:rename(Workspace.selection) end,
		['f5'] = function() self.preview:refresh() end,
		['f1'] = function() Dialog.show('Keyboard Shortcuts', SHORTCUTS) end,
		['ctrl+f'] = function() self:activate(self.hierarchy) self.hierarchy:startSearch('') end,
		['ctrl+q'] = function() self:quit() end,
		['ctrl+1'] = function() self:activate(self.hierarchy) end,
		['ctrl+2'] = function() self:activate(self.project) end,
		['ctrl+3'] = function() self:activate(self.inspector) end,
		['ctrl+4'] = function() self:activate(self.console) end,
	}
	local action = actions[chord]
	if action then action() return true end
	local letter = chord:match('^alt%+(%a)$')
	return letter and self.menubar:accelerator(letter) or false
end

function App:build()
	local app = self
	self.quitOnQ = false
	self.showProject, self.showConsole = true, true
	local screen = fs.loadObject('Editor/Screens/Editor')
	self.editorRoot = screen:findChild('EditorRoot')
	self.editorRoot.KeyDown = function(_, args) return app:hotkey(Native.chord(args)) end
	self.runtime = screen:findChild('Runtime')

	self.hierarchy = Hierarchy()
	self.project = ProjectView()
	self.inspector = Inspector(function(opts) BindingEditor.open(opts) end)
	self.preview = Preview()
	self.console = Console()
	self.statusbar = StatusBar()
	self.tabs = DocumentTabs(function(path) app:closeDocument(path) end)
	self.menubar = MenuBar(self:menus(), function()
		return Workspace.project and Workspace.leaf(Workspace.project) or ''
	end)

	local hotkey = function(chord) return app:hotkey(chord) end
	Native.onActivate = function(panel) if panel.header then app:focusPanel(panel) end end
	for _, p in ipairs { self.hierarchy, self.project, self.inspector, self.console, self.menubar, self.statusbar, self.tabs } do
		p.hotkey = hotkey
	end
	self.left = screen:findChild('LeftPanels', true)
	self.center = screen:findChild('CenterPanels', true)
	local function mount(name, view) screen:findChild(name, true):addChild(view) end
	mount('MenuSlot', self.menubar.view)
	mount('HierarchySlot', Panel.wrap(self.hierarchy))
	mount('ProjectSlot', Panel.wrap(self.project))
	mount('TabsSlot', self.tabs.view)
	mount('PreviewSlot', Panel.wrap(self.preview))
	mount('ConsoleSlot', Panel.wrap(self.console))
	mount('InspectorSlot', Panel.wrap(self.inspector))
	mount('StatusSlot', self.statusbar.view)
	local overlay = screen:findChild('Overlay')
	overlay.LeftButtonDown = function(_, args) return Popups.outside(args) end
	overlay.RightButtonDown = function(_, args) return Popups.outside(args) end
	Popups.attach(overlay, function(args)
		if args.y < 36 then
			local i = app.menubar:indexAt(args.x)
			if i then app.menubar:open(i) end
		end
	end)
	Popups.restoreFocus = function() if app.activePanel then app.activePanel:focus() end end

	for _, event in ipairs { 'document', 'select', 'changed', 'structure', 'saved', 'tabs' } do
		Workspace.on(event, function()
			app.statusbar:invalidate()
			app.menubar:invalidate()
		end)
	end
	for _, p in ipairs { self.hierarchy, self.project, self.inspector, self.console, self.preview } do
		if p.header then
			p.header:invalidate()
		end
	end
	self.menubar:invalidate()
	self.statusbar:invalidate()
	self.welcome = Welcome.new {
		open = function(path) return app:openProject(path) end,
		cancel = function()
			if not app.projectOpened then return end
			app.welcome.view.Visible = false
			app.editorRoot.Visible = true
			app:activate(app.hierarchy)
		end,
		warning = function(err) Workspace.log('warning', 'Recent projects: %s', err) end,
	}
	screen:addChild(self.welcome.view)
	self.editorRoot.Visible = false
	return screen
end

return App
