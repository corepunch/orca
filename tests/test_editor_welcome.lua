-- Welcome startup and real native project controls, without launching a runtime.
local orca = require 'orca'
orca.init()
local fs, core, ui = require 'orca.filesystem', require 'orca.core', require 'orca.UIKit'
require('orca.renderer').init(1040, 680, true)
fs.init('apps/Editor')
DATADIR = 'apps/Editor'
require('orca.core.startup').load_plugins()
local Native = require 'Editor.Scripts.Native'
local Workspace = require 'Editor.Scripts.Workspace'
local App = require 'Editor.App'
local app = App()
local starts, opened = 0, nil
local originalStart = Workspace.start
Workspace.start = function(path, connection)
	starts, opened = starts + 1, path
	Workspace.project, Workspace.connection = path, connection
end
local screen = app:build()
screen.Width, screen.Height, screen.ResizeMode = 1040, 680, 'NoResize'
local welcome = app.welcome
local function settle(width, height)
	core.flushQueue()
	core.advanceFrame()
	core.runAllPrograms(screen)
	core.flushQueue()
	screen:UpdateLayout(width or 1040, height or 680)
	welcome:update()
	core.flushQueue()
	screen:UpdateLayout(width or 1040, height or 680)
end
local function click(node)
	local x,y = Native.origin(node)
	orca.system.dispatchMessage{target=screen,message='LeftButtonDown',x=x+node.ActualWidth/2,y=y+node.ActualHeight/2}
	orca.system.dispatchMessage{target=screen,message='LeftButtonUp',x=x+node.ActualWidth/2,y=y+node.ActualHeight/2}
	settle()
end
settle()
local splash = welcome:node('Splash').Source
assert(splash and splash.Scale == 2, 'The splash declares its Retina density')
assert(splash.MinificationFilter == 'Linear' and splash.MagnificationFilter == 'Linear', 'Both texture filters keep scaled artwork smooth')
assert(welcome.view.Visible and not app.editorRoot.Visible, 'Welcome owns initial startup')
app:afterPaint()
assert(starts == 0 and not app.started, 'The welcome screen does not launch a project server')
assert(#welcome.filtered > 0 and welcome.list:getFirstChild(), 'Bundled project records materialize through the native template')
assert(welcome:selected(), 'The first project can be opened immediately')
local first = welcome.list:getFirstChild()
assert(first.ActualHeight == 64 and first.ActualWidth > 400, 'Project cards use their intended native layout')
local second = first:getNext()
if second then
	click(second)
	assert(welcome.list.SelectedValue == 'Row2' and welcome:node('SelectedTitle').Text == welcome.filtered[2].title, 'Pointer selection updates launch details')
end
welcome.search.Text = 'no-such-project-871344'
settle()
assert(#welcome.filtered == 0 and welcome:node('NoProjects').Visible, 'Search shows an honest empty state')
assert(welcome:node('OpenSelected').IgnoreHitTest and not welcome:openSelected(), 'Empty search cannot launch a project')
welcome.search.Text = ''
settle()
click(welcome:node('RecentProjects'))
assert(welcome.mode == 'recent', 'Recent tab is interactive')
click(welcome:node('AllProjects'))
assert(welcome.mode == 'all' and #welcome.filtered > 0, 'All projects restores bundled projects')
click(welcome:node('Browse'))
assert(welcome:node('FolderBrowser').Visible and #welcome.folders > 0, 'Open project provides a real folder browser')
welcome:node('FolderPath').Text = '/nonexistent/orca-welcome-test'
click(welcome:node('OpenFolder'))
assert(starts == 0 and welcome:node('FolderError').Visible, 'An invalid external project remains in the chooser')
welcome:folder(DATADIR .. '/../Example')
assert(welcome:node('FolderPath').Text:match('/Example$'), 'Folder browsing accepts a bundled project')
welcome:cancelBrowse()
-- Isolate persistent history: launching the test must not change personal recents.
local Projects = require 'Editor.Scripts.Projects'
local originalRemember = Projects.remember
Projects.remember = function() return true end
local chosen = welcome:selected().path
Workspace.undoStack = {{kind='old'}}
Workspace.dirty = true
click(welcome:node('OpenSelected'))
assert(starts == 1 and opened == chosen, 'Open selected uses the actual chosen project')
assert(app.editorRoot.Visible and not welcome.view.Visible and not app.started, 'Launching hands off to editor before deferred runtime loading')
assert(#Workspace.undoStack == 0 and not Workspace.dirty, 'A different project starts with a clean edit journal')
assert(app:showWelcome() and welcome.view.Visible and welcome:node('ReturnToEditor').Visible, 'File chooser can return to an existing workspace')
settle()
click(welcome:node('ReturnToEditor'))
assert(not welcome.view.Visible and app.editorRoot.Visible, 'Cancel returns to the workspace')
Workspace.dirty = true
assert(not app:showWelcome() and not welcome.view.Visible, 'Unsaved edits are protected when switching projects')
Workspace.dirty = false
app:showWelcome()
screen.Width, screen.Height = 720, 620
settle(720,620)
assert(not welcome:node('Artwork').Visible and welcome.view.Columns == '1fr', 'Small windows dedicate space to usable project controls')
local _,openY = Native.origin(welcome:node('OpenSelected'))
assert(openY + welcome:node('OpenSelected').ActualHeight <= 620, 'The launch action remains reachable in a smaller window')
Projects.remember, Workspace.start = originalRemember, originalStart
print('All editor welcome tests passed.')
