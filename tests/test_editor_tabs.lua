-- Document tabs: per-document state, the native tab strip and the
-- close/save questions, against a stubbed runtime (no project process).
local orca = require 'orca'
orca.init()
local fs, core = require 'orca.filesystem', require 'orca.core'
require('orca.renderer').init(1040, 680, true)
fs.init('apps/Editor')
DATADIR = 'apps/Editor'
require('orca.core.startup').load_plugins()
local Native = require 'Editor.Scripts.Native'
local Workspace = require 'Editor.Scripts.Workspace'
local Popups = require 'Editor.Scripts.Popups'
local Xml = require 'Editor.Scripts.Xml'
local App = require 'Editor.App'

-- Runtime stub: records requests and answers like the EditorKit server.
local requests = {}
Workspace.start = function(path, connection) Workspace.project, Workspace.connection = path, connection end
Workspace.request = function(method, route, path)
	requests[#requests + 1] = method .. '/' .. route .. (path or '')
	if route == 'node/' and method == 'put' then
		return Xml.parse('<response status="success"><n value="old"/></response>')
	elseif route == 'save/' then
		return Xml.parse('<response status="success" file="x.xml" applied="1" skipped="0"/>')
	end
	return Xml.parse('<response status="success"/>')
end
Workspace.configureSource = function(source)
	source.Connection, source.ConnectionPath = nil, nil
	source.Content = '<response status="success"/>'
	source:send('XmlDataSource.Reload')
	return true
end
local function requested(entry)
	for _, r in ipairs(requests) do if r == entry then return true end end
	return false
end

local app = App()
local screen = app:build()
screen.Width, screen.Height, screen.ResizeMode = 1040, 680, 'NoResize'
app:openProject('apps/Example')
app.started = true
local function settle()
	core.flushQueue()
	core.advanceFrame()
	core.runAllPrograms(screen)
	core.flushQueue()
	screen:UpdateLayout(1040, 680)
end
local function click(node)
	local x, y = Native.origin(node)
	orca.system.dispatchMessage{target=screen,message='LeftButtonDown',x=x+node.ActualWidth/2,y=y+node.ActualHeight/2}
	orca.system.dispatchMessage{target=screen,message='LeftButtonUp',x=x+node.ActualWidth/2,y=y+node.ActualHeight/2}
	settle()
end
local function tabs()
	local list, child = {}, app.tabs.tabs:getFirstChild()
	while child do
		if child.DataContext then list[#list + 1] = child end
		child = child:getNext()
	end
	return list
end
local function label(tab) return tab.DataContext.Label end
local function button(tab, name) return assert(tab:findChild(name, true), name) end

settle()
assert(#tabs() == 0 and app.tabs.hint.Visible, 'An empty strip explains how to open a document')
local A, B, C = 'Example/Screens/Application', 'Example/Prefabs/FeatureCard', 'Example/Prefabs/GalleryCard'
Workspace.open(A)
Workspace.open(B)
settle()
assert(#tabs() == 2 and not app.tabs.hint.Visible, 'Each open document has a native tab')
assert(label(tabs()[1]) == 'Application' and label(tabs()[2]) == 'FeatureCard')
assert(tabs()[2].DataContext.State == 'tab-active' and Workspace.document == B, 'The newest tab is active')
assert(Workspace.open(A) and #Workspace.documents == 2 and Workspace.document == A,
	'Opening an open document shows its tab instead of duplicating it')

-- Per-document state follows the active tab.
Workspace.select(A .. '/Root')
assert(Workspace.setValue(A .. '/Root', 'Node.Visible', 'false'))
settle()
assert(Workspace.dirty and #Workspace.undoStack == 1 and label(tabs()[1]) == 'Application *',
	'An edit marks its tab as unsaved')
click(button(tabs()[2], 'TabTitle'))
assert(Workspace.document == B and not Workspace.dirty and #Workspace.undoStack == 0 and Workspace.selection == B,
	'Clicking a tab restores that document\'s own selection and history')
assert(app:hotkey('ctrl+tab') and Workspace.document == A and Workspace.selection == A .. '/Root',
	'Ctrl+Tab cycles to the next tab with its selection intact')
assert(app:hotkey('ctrl+shift+tab') and Workspace.document == B)

-- A new tab opens beside the active one; Ctrl+W closes a clean tab at once.
Workspace.activate(A)
Workspace.open(C)
assert(Workspace.documents[2].path == C, 'New tabs open next to the active tab')
assert(app:hotkey('ctrl+w') and not Workspace.findDocument(C) and Workspace.document == B,
	'Closing a clean tab activates its neighbour without asking')
settle()
click(button(tabs()[2], 'TabClose'))
assert(not Workspace.findDocument(B) and Workspace.document == A, 'The tab close button closes its own tab')

-- Unsaved documents ask before closing.
requests = {}
app:closeDocument(A)
local dialog = Popups.top()
assert(dialog and Workspace.findDocument(A), 'Closing an unsaved tab asks first')
dialog:answer(dialog.opts.secondary)
assert(requested('post/revert/' .. A) and #Workspace.documents == 0 and not Workspace.document,
	"Don't Save discards the runtime edits and closes the tab")
settle()
assert(#tabs() == 0 and app.tabs.hint.Visible and app.preview.image.Src == '',
	'Closing the last tab empties the strip and the Preview')

Workspace.open(A)
assert(Workspace.setValue(A, 'Node.Visible', 'false'))
requests = {}
app:closeDocument(A)
Popups.top():onKey('enter')
assert(requested('post/save/' .. A) and not Workspace.findDocument(A), 'Save writes the document, then closes it')

-- Leaving the project or quitting protects unsaved tabs too.
Workspace.open(A)
Workspace.open(B)
assert(Workspace.setValue(A, 'Node.Visible', 'true'))
assert(Workspace.anyDirty() and not Workspace.dirty, 'Unsaved state of background tabs is still tracked')
assert(not app:showWelcome() and Popups.top(), 'Switching projects asks about unsaved background tabs')
Popups.top():onKey('escape')
assert(#Workspace.documents == 2 and app.editorRoot.Visible, 'Cancel keeps every tab open')
requests = {}
assert(Workspace.saveAll() == 1 and requested('post/save/' .. A) and not requested('post/save/' .. B),
	'Save All writes only the unsaved documents')
assert(not Workspace.anyDirty())
print('All editor tab tests passed.')
