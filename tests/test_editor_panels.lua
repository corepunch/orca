-- Native XML data sources drive real editor controls; no Lua record adapter.
local orca = require "orca"
orca.init()
local fs, core, ui = require "orca.filesystem", require "orca.core", require "orca.UIKit"
require("orca.renderer").init(1440, 900, true)
fs.init("apps/Editor")
require("orca.core.startup").load_plugins()

local Workspace = require "Editor.Scripts.Workspace"
local hierarchyXML = [[<response status="success">
  <EditorNode name="node-1" Id="node-1" Path="Example/Screens/Test" Label="Test" ClassName="Screen" Indent="0" HasChildren="true" Expanded="true" Disclosure="−"/>
  <EditorNode name="node-2" Id="node-2" Path="Example/Screens/Test/Title" Label="Title" ClassName="TextBlock" Indent="16" HasChildren="false"/>
</response>]]
local additionalNodes = {}
for i = 3, 200 do
    additionalNodes[#additionalNodes + 1] = ('<EditorNode name="node-%d" Id="node-%d" Path="Example/Screens/Test/Node%d" Label="A long node label that must stay inside its column" ClassName="TextBlock" Indent="48"/>'):format(i, i, i)
end
hierarchyXML = hierarchyXML:gsub('</response>', table.concat(additionalNodes) .. '</response>')
local propertyXML = [[<response status="success">
  <EditorProperty name="group" Id="group" Label="TextBlock" IsGroup="true" ReadOnly="true"/>
  <EditorProperty name="text" Id="text" PropertyName="TextBlock.Text" Label="Text" Value="Hello" Kind="String" IsText="true" IsProperty="true"/>
  <EditorProperty name="visible" Id="visible" PropertyName="Node.Visible" Label="Visible" Value="true" Kind="Bool" IsBool="true" IsProperty="true" BoolValue="true"/>
  <EditorProperty name="boundBool" Id="boundBool" PropertyName="Node.Enabled" Label="Enabled" Value="true" Kind="Bool" IsBool="true" IsProperty="true" BoolValue="true" HasBinding="true" Expression="{State.Enabled}"/>
  <EditorProperty name="boundEnum" Id="boundEnum" PropertyName="Node.OverflowY" Label="Overflow" Value="Scroll" Kind="Enum" IsEnum="true" IsProperty="true" HasBinding="true" Expression="{State.Overflow}"/>
  <EditorProperty name="reference" Id="reference" PropertyName="ImageView.Source" Label="Image" Value="Example/Images/Picture" Kind="Object" IsReference="true" IsProperty="true" ValueReadOnly="true"/>
  <EditorProperty name="align" Id="align" PropertyName="TextBlock.TextAlignment" Label="Alignment" Value="Left" Kind="Enum" IsEnum="true" IsProperty="true">
    <Options><EditorOption name="left" Label="Left" Value="Left"/><EditorOption name="right" Label="Right" Value="Right"/></Options>
  </EditorProperty>
</response>]]
local projectXML = [[<response status="success">
  <EditorFile name="file1" Id="file1" Path="Screens/Test.xml" Label="Test.xml" Kind="document" SourcePath="Example/Screens/Test" Indent="0"/>
</response>]]
local requests, edits = {}, {}
Workspace.configureSource = function(source, request)
    requests[#requests + 1] = request
    source.Connection, source.ConnectionPath = nil, nil
    source.Content = request:find('data-scene', 1, true) and hierarchyXML
        or request:find('data-properties', 1, true) and propertyXML or projectXML
    source:send('XmlDataSource.Reload')
    assert(not source.HasError and source.Items, source.ErrorMessage)
end
Workspace.setValue = function(path, property, value)
    edits[#edits + 1] = { path, property, value }
    return true
end
Workspace.project, Workspace.document = 'apps/Example', 'Example/Screens/Test'
local Hierarchy, Project, Inspector = require "Editor.Scripts.Hierarchy", require "Editor.Scripts.ProjectView", require "Editor.Scripts.Inspector"
local hierarchy, project, binding = Hierarchy(), Project(), nil
local inspector = Inspector(function(opts) binding = opts end)
local screen = assert(fs.loadObjectFromDisk('Editor/Screens/Editor'))
screen.Width, screen.Height, screen.ResizeMode = 1440, 900, 'NoResize'
local Panel = require 'Editor.Scripts.Panel'
screen:findChild('HierarchySlot', true):addChild(Panel.wrap(hierarchy))
screen:findChild('ProjectSlot', true):addChild(Panel.wrap(project))
screen:findChild('InspectorSlot', true):addChild(Panel.wrap(inspector))
local function settle()
    core.flushQueue()
    core.advanceFrame()
    core.runAllPrograms(screen)
    core.flushQueue()
    screen:UpdateLayout(1440, 900)
end
settle()
hierarchy:reload()
project:reload()
inspector:show(Workspace.document)
settle()
assert(hierarchy.view:findChild('SceneNodenode-2', true), 'Native node records generate hierarchy rows')
for _, panel in ipairs {hierarchy, project, inspector} do
    assert(panel.view.ActualHeight <= 836 and panel.list.ActualHeight <= 836,
        panel.title .. ' remains bounded by the editor viewport')
end
local labelRow = assert(hierarchy.view:findChild('SceneNodenode-3', true))
local label, icon = labelRow:findChild('NodeLabel', true), labelRow:findChild('NodeIcon', true)
assert(icon.ActualX + icon.ActualWidth <= label.ActualX, 'The class icon precedes the node label')
assert(label.ActualX + label.ActualWidth <= labelRow.ActualWidth, 'Long node labels stay inside the row')
hierarchy.list:send('ListBox.SelectItem', { Value = 'Example/Screens/Test/Title' })
settle()
assert(Workspace.selection == 'Example/Screens/Test/Title', 'ListBox selection selects the runtime path')
assert(project.view:findChild('ProjectFilefile1', true), 'Project native source creates asset rows')
assert(#project:documents() == 1, 'Open Document menus read native records')
local textRow = assert(inspector.view:findChild('Propertytext', true), 'Native property records generate inspector rows')
local input = assert(textRow:findChild('PropertyText', true))
assert(input.Text == 'Hello', 'Property text binds to the native record')
assert(input.ActualWidth > 0 and input.ActualHeight > 0, 'The property input has visible measured bounds')
local ancestor = input.parent
while ancestor do
    if ancestor.Visible ~= nil then assert(ancestor.Visible, 'The property input ancestors stay visible') end
    ancestor = ancestor.parent
end
input:post('Input.Submit', { Text = 'Updated' })
settle()
assert(#edits == 1 and edits[1][2] == 'TextBlock.Text' and edits[1][3] == 'Updated', 'Submitting a native input edits the runtime property')
local boolRow = assert(inspector.view:findChild('Propertyvisible', true))
local checkbox = assert(boolRow:findChild('PropertyBoolean', true))
checkbox:send('Node.LeftButtonUp')
settle()
assert(#edits == 2 and edits[2][2] == 'Node.Visible' and edits[2][3] == 'false', 'A real checkbox edits a boolean property')
local bindingButton = assert(textRow:findChild('BindProperty', true))
bindingButton:send('Node.LeftButtonUp')
settle()
assert(binding and binding.property == 'TextBlock.Text', 'The row binding button opens the binding editor for its native record')
assert(#edits == 2, 'Binding initialization does not commit boolean property edits')
local boundBool = assert(inspector.view:findChild('PropertyboundBool', true)):findChild('PropertyBoolean', true)
boundBool:send('Node.LeftButtonUp')
settle()
assert(binding.property == 'Node.Enabled' and #edits == 2, 'Activating a bound checkbox edits its binding')
local boundEnum = assert(inspector.view:findChild('PropertyboundEnum', true)):findChild('PropertyEnum', true)
boundEnum:send('Node.LeftButtonUp')
settle()
assert(binding.property == 'Node.OverflowY' and #edits == 2, 'Activating a bound enum edits its binding')
local reference = assert(inspector.view:findChild('Propertyreference', true)):findChild('BindProperty', true)
reference:send('Node.LeftButtonUp')
settle()
assert(binding.property == 'ImageView.Source', 'Object references accept bindings while their value stays read-only')
hierarchy:startSearch('Title')
settle()
assert(requests[#requests]:find('filter=Title', 1, true), 'Search is a runtime data-source query')
hierarchy.source.Content = '<response status="error">Runtime disconnected</response>'
hierarchy.source:send('XmlDataSource.Reload')
settle()
local errorLabel = assert(hierarchy.view:findChild('DataSourceError', true))
assert(errorLabel.Visible and errorLabel.Text == 'Runtime disconnected', 'Source errors bind directly into the panel')
assert(hierarchy.view:findChild('SceneNodenode-2', true), 'A failed refresh keeps the last successful hierarchy')
-- Exercise the real app configuration: Widget's class lookup can fall through
-- a false class default, so ordinary q must reach the editor's focused input.
local previousDataDir = DATADIR
DATADIR = DATADIR or 'apps/Editor'
local editor = require('Editor.App')()
editor.screen = editor:build()
DATADIR = previousDataDir
local system = require 'orca.system'
local getMessage, dispatchMessage, translateMessage = system.getMessage, system.dispatchMessage, system.translateMessage
local hasChangedFiles = fs.hasChangedFiles
local typed = { key='q', is=function(_, name) return name == 'Node.KeyDown' end }
local closed = { is=function(_, name) return name == 'Window.Closed' end }
local nextEvent, received = 0, false
system.getMessage = function() nextEvent = nextEvent + 1; return ({typed, closed})[nextEvent] end
system.dispatchMessage = function(_, message) received = message == typed; return false end
system.translateMessage = function() end
fs.hasChangedFiles = function() return false end
local ran, runError = pcall(function() editor:run() end)
system.getMessage, system.dispatchMessage, system.translateMessage = getMessage, dispatchMessage, translateMessage
fs.hasChangedFiles = hasChangedFiles
assert(ran, runError)
assert(received, 'Typing q reaches the editor instead of quitting its application')
print('PASS: native editor panels, selection, property inputs, checkbox and binding action')
