-- Native XML data sources drive real editor controls; no Lua record adapter.
local orca = require "orca"
orca.init()
local fs, core, ui = require "orca.filesystem", require "orca.core", require "orca.UIKit"
require("orca.renderer").init(1440, 900, true)
fs.init("apps/Editor")
require("orca.core.startup").load_plugins()

local Workspace = require "Editor.Scripts.Workspace"
local hierarchyXML = [[<response status="success">
  <EditorNode name="node-1" Id="node-1" Path="Example/Screens/Test" Label="Test" ClassName="Screen" HasChildren="true">
    <Children>
      <EditorNode name="node-2" Id="node-2" Path="Example/Screens/Test/Title" Label="Title" ClassName="TextBlock"/>
      <EditorNode name="node-201" Id="node-201" Path="Example/Screens/Test/Group" Label="Group" ClassName="StackView" HasChildren="true">
        <Children><EditorNode name="node-202" Id="node-202" Path="Example/Screens/Test/Group/Nested" Label="Nested" ClassName="TextBlock"/></Children>
      </EditorNode>
      <!-- additional nodes -->
    </Children>
  </EditorNode>
</response>]]
local additionalNodes = {}
for i = 3, 200 do
    additionalNodes[#additionalNodes + 1] = ('<EditorNode name="node-%d" Id="node-%d" Path="Example/Screens/Test/Node%d" Label="A long node label that must stay inside its column" ClassName="TextBlock"/>'):format(i, i, i)
end
hierarchyXML = hierarchyXML:gsub('<!%-%- additional nodes %-%->', table.concat(additionalNodes))
local propertyXML = [[<response status="success">
  <EditorProperty name="group" Id="group" Label="TextBlock" IsGroup="true" ReadOnly="true"/>
  <EditorProperty name="text" Id="text" PropertyName="TextBlock.Text" Label="Text" Value="Hello" Kind="String" IsText="true" IsProperty="true" IsBindable="true"/>
  <EditorProperty name="visible" Id="visible" PropertyName="Node.Visible" Label="Visible" Value="true" Kind="Bool" IsBool="true" IsProperty="true" BoolValue="true" IsBindable="true"/>
  <EditorProperty name="boundBool" Id="boundBool" PropertyName="Node.Enabled" Label="Enabled" Value="true" Kind="Bool" IsBool="true" IsProperty="true" BoolValue="true" HasBinding="true" Expression="{State.Enabled}" IsBindable="true"/>
  <EditorProperty name="boundEnum" Id="boundEnum" PropertyName="Node.OverflowY" Label="Overflow" Value="Scroll" Kind="Enum" IsEnum="true" IsProperty="true" HasBinding="true" Expression="{State.Overflow}" IsBindable="true"/>
  <EditorProperty name="reference" Id="reference" PropertyName="ImageView.Source" Label="Image" Value="Example/Images/Picture" ReferenceLabel="Picture" ReferencePath="Example/Images/Picture" Kind="Object" IsReference="true" IsProperty="true" IsBindable="true"/>
  <EditorProperty name="align" Id="align" PropertyName="TextBlock.TextAlignment" Label="Alignment" Value="Left" Kind="Enum" IsEnum="true" IsProperty="true">
    <Options><EditorOption name="left" Label="Left" Value="Left"/><EditorOption name="right" Label="Right" Value="Right"/></Options>
  </EditorProperty>
  <EditorProperty name="padding" Id="padding" PropertyName="Node.Padding" Label="Padding" Value="1 2 3 4" IsCompound="true" IsNumericCompound="true" FieldColumns="1fr 1fr 1fr 1fr" IsProperty="true" DisclosureIcon="Editor/Icons/chevron-right.svg">
    <Fields>
      <EditorProperty name="left" Id="left" PropertyName="Node.PaddingLeft" Label="Left" ShortLabel="L" Value="1" Kind="Float" IsText="true" IsProperty="true" IsBindable="true"/>
      <EditorProperty name="top" Id="top" PropertyName="Node.PaddingTop" Label="Top" ShortLabel="T" Value="2" Kind="Float" IsText="true" IsProperty="true" IsBindable="true"/>
      <EditorProperty name="right" Id="right" PropertyName="Node.PaddingRight" Label="Right" ShortLabel="R" Value="3" Kind="Float" IsText="true" IsProperty="true" IsBindable="true"/>
      <EditorProperty name="bottom" Id="bottom" PropertyName="Node.PaddingBottom" Label="Bottom" ShortLabel="B" Value="4" Kind="Float" IsText="true" IsProperty="true" IsBindable="true"/>
    </Fields>
  </EditorProperty>
  <EditorProperty name="font" Id="font" PropertyName="TextBlock.Font" Label="Font" Value="Arial · 12 · Regular" IsCompound="true" IsSummary="true" IsProperty="true"/>
  <EditorProperty name="color" Id="color" PropertyName="Node2D.BackgroundColor" Label="Background" Value="#4a8cf7" Kind="Color" IsColor="true" IsProperty="true" IsBindable="true"/>
  <EditorProperty name="filter" Id="filter" PropertyName="Node2D.BackdropFilter" Label="Backdrop filter" IsCompound="true" IsNumericCompound="true" FieldColumns="1fr 1fr" IsProperty="true">
    <Fields>
      <EditorProperty name="blur" Id="blur" PropertyName="Node2D.BackdropFilterBlurRadius" Label="BlurRadius" ShortLabel="BlurRadius" Value="0" Kind="Float" IsText="true" IsProperty="true" IsBindable="true"/>
      <EditorProperty name="saturation" Id="saturation" PropertyName="Node2D.BackdropFilterSaturation" Label="Saturation" ShortLabel="Saturation" Value="1" Kind="Float" IsText="true" IsProperty="true" IsBindable="true"/>
    </Fields>
  </EditorProperty>
</response>]]
local resourceXML = [[<response status="success">
  <EditorResource name="none" Id="none" Label="None (Image)" Value="" Path="" Kind="Image" Icon="Editor/Icons/object.svg"/>
  <EditorResource name="picture" Id="picture" Label="Picture" Value="Example/Images/Picture" Path="Example/Images/Picture" Kind="Image" Icon="Editor/Icons/image.svg"/>
  <EditorResource name="other" Id="other" Label="Other" Value="Example/Images/Other" Path="Example/Images/Other" Kind="Image" Icon="Editor/Icons/image.svg"/>
</response>]]
propertyXML = propertyXML:gsub('(<EditorProperty[^>]- Value="([^"]*)")', function(tag, value)
    return tag .. ' DraftValue="' .. value .. '"'
end)
local projectXML = [[<response status="success">
  <EditorFile name="folder1" Id="folder1" Path="Screens" Label="Screens" Kind="group" HasChildren="true">
    <Children>
      <EditorFile name="file1" Id="file1" Path="Screens/Test.xml" Label="Test.xml" Kind="document" SourcePath="Example/Screens/Test"/>
      <EditorFile name="folder2" Id="folder2" Path="Screens/Nested" Label="Nested" Kind="group" HasChildren="true">
        <Children><EditorFile name="file2" Id="file2" Path="Screens/Nested/Details.xml" Label="Details.xml" Kind="document" SourcePath="Example/Screens/Nested/Details"/></Children>
      </EditorFile>
    </Children>
  </EditorFile>
</response>]]
local requests, edits, propertySource = {}, {}, nil
Workspace.configureSource = function(source, request)
    requests[#requests + 1] = request
    source.Connection, source.ConnectionPath = nil, nil
    source.Content = request:find('data-scene', 1, true) and hierarchyXML
        or request:find('data-properties', 1, true) and propertyXML
        or request:find('data-resources', 1, true) and resourceXML or projectXML
    source:send('XmlDataSource.Reload')
    assert(not source.HasError and source.Items, source.ErrorMessage)
end
Workspace.setValue = function(path, property, value)
    edits[#edits + 1] = { path, property, value }
    local function update(source)
        if not source then return end
        for record in source.children do
            if record.PropertyName == property then
                record.Value = value
                record.DraftValue = value
                if record.IsBool then record.BoolValue = value == 'true' end
            end
            update(record:findChild('Fields', false))
        end
    end
    update(propertySource and propertySource.Items)
    return true
end
Workspace.project, Workspace.document = 'apps/Example', 'Example/Screens/Test'
local Hierarchy, Project, Inspector = require "Editor.Scripts.Hierarchy", require "Editor.Scripts.ProjectView", require "Editor.Scripts.Inspector"
local hierarchy, project, binding = Hierarchy(), Project(), nil
local inspector = Inspector(function(opts) binding = opts end)
propertySource = inspector.source
local screen = assert(fs.loadObjectFromDisk('Editor/Screens/Editor'))
screen.Width, screen.Height, screen.ResizeMode = 1440, 900, 'NoResize'
local Popups = require 'Editor.Scripts.Popups'
Popups.attach(screen:findChild('Overlay'))
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
assert(hierarchy.list:getClassName() == 'TreeView', 'Hierarchy uses the reusable native TreeView')
assert(project.list:getClassName() == 'TreeView', 'Project uses the same native TreeView')
local rootItem = assert(hierarchy.view:findChild('SceneNodenode-1', true))
local rootRecord = assert(hierarchy.source.Items:findChild('node-1'))
assert(rootItem.parent == hierarchy.list and hierarchy.source.Items:getFirstChild():getNext() == nil,
    'TreeView presents only root records')
assert(rootItem.ItemsSource == rootRecord:findChild('Children'), 'The item binds its own child collection')
assert(not rootItem.IsExpanded, 'An item starts as one collapsed header row')
rootItem.IsExpanded = true
settle()
local titleItem = assert(hierarchy.view:findChild('SceneNodenode-2', true), 'The root item generates its descendants')
assert(titleItem.parent == rootItem:findChild('ItemsPresenter'), 'Children belong to the parent item presenter')
assert(rootItem.Header ~= titleItem.Header, 'Each generated item owns a distinct header instance')
for _, panel in ipairs {hierarchy, project, inspector} do
    assert(panel.view.ActualHeight <= 836 and panel.list.ActualHeight <= 836,
        panel.title .. ' remains bounded by the editor viewport')
end
local labelRow = assert(hierarchy.view:findChild('SceneNodenode-3', true))
local label, icon = labelRow:findChild('NodeLabel', true), labelRow:findChild('NodeIcon', true)
assert(icon.ActualX + icon.ActualWidth <= label.ActualX, 'The class icon precedes the node label')
assert(label.ActualX + label.ActualWidth <= label.parent.ActualWidth, 'Long node labels stay inside their header')
hierarchy.list:send('TreeView.SelectItem', { Value = 'Example/Screens/Test/Title' })
settle()
assert(Workspace.selection == 'Example/Screens/Test/Title', 'TreeView descendant selection selects the runtime path')
local function sceneRequestCount()
    local count = 0
    for _, request in ipairs(requests) do if request:find('data-scene/', 1, true) then count = count + 1 end end
    return count
end
local requestCount = sceneRequestCount()
local expander = assert(rootItem:findChild('Expander', true))
expander:send('Node.LeftButtonUp')
settle()
assert(not rootItem.IsExpanded and sceneRequestCount() == requestCount, 'Disclosure collapses locally without requesting the scene again')
expander:send('Node.LeftButtonUp')
settle()
assert(rootItem.IsExpanded and hierarchy.view:findChild('SceneNodenode-2', true) == titleItem, 'Reopening a branch retains its child container')
rootItem.IsExpanded = false
hierarchy:reload()
settle()
assert(hierarchy.view:findChild('SceneNodenode-1', true) == rootItem and not rootItem.IsExpanded,
    'Data refresh preserves item identity and local expansion state')
hierarchy.list:send('TreeView.SelectItem', { Value = 'Example/Screens/Test/Group/Nested' })
settle()
local groupItem = assert(hierarchy.view:findChild('SceneNodenode-201', true))
assert(rootItem.IsExpanded and groupItem.IsExpanded and hierarchy.list.SelectedValue == 'Example/Screens/Test/Group/Nested',
    'Revealing a deep selection expands its ancestor containers')
local nestedItem = assert(hierarchy.view:findChild('SceneNodenode-202', true))
assert(nestedItem.IsSelected and not rootItem.IsSelected and not groupItem.IsSelected,
    'The tree has one selection across all nested item owners')
hierarchyXML = hierarchyXML:gsub('Path="Example/Screens/Test/Group/Nested"', 'Path="Example/Screens/Test/Group/Renamed"')
    :gsub('Label="Nested"', 'Label="Renamed"')
hierarchy:reload()
settle()
assert(hierarchy.view:findChild('SceneNodenode-202', true) == nestedItem
    and hierarchy.list.SelectedItem == nestedItem.DataContext
    and hierarchy.list.SelectedValue == 'Example/Screens/Test/Group/Renamed',
    'Renaming a nested record preserves its container and updates the selected value')
assert(nestedItem.Header:findChild('NodeLabel', true).Text == 'Renamed'
    and groupItem.Header:findChild('NodeLabel', true).Text == 'Group',
    'Retained headers update in their own record scope without changing ancestor labels')
requestCount = sceneRequestCount()
hierarchy:expandAll(false)
settle()
assert(not rootItem.IsExpanded and not groupItem.IsExpanded and rootItem.IsSelected,
    'Collapse All moves the hidden selection to its visible ancestor')
hierarchy.view:post('Node.KeyDown', { keyCode = 131 })
settle()
assert(rootItem.IsExpanded and not groupItem.IsExpanded, 'The panel key controller delegates Right to the native tree')
hierarchy.view:post('Node.KeyDown', { keyCode = 131 })
settle()
assert(titleItem.IsSelected and Workspace.selection == 'Example/Screens/Test/Title',
    'Right from the root selects its first child through the editor selection event')
hierarchy.view:post('Node.KeyDown', { keyCode = 129 })
settle()
assert(groupItem.IsSelected, 'The panel key controller follows native visible sibling order')
groupItem.Header:post('Node.LeftDoubleClick', { x = 0, y = 0 })
settle()
assert(groupItem.IsExpanded and sceneRequestCount() == requestCount,
    'Double clicking an authored branch Header expands its own children without a tree request')
hierarchy:expandAll(true)
settle()
assert(rootItem.IsExpanded and groupItem.IsExpanded and sceneRequestCount() == requestCount,
    'Expand All operates locally on nested containers without a tree request')
local folderItem = assert(project.view:findChild('ProjectFilefolder1', true))
assert(not folderItem.IsExpanded, 'Project folders start collapsed too')
folderItem.IsExpanded = true
settle()
local documentItem = assert(project.view:findChild('ProjectFilefile1', true), 'A project folder creates its own child items')
assert(documentItem.parent == folderItem:findChild('ItemsPresenter'), 'Project descendants belong to their folder')
local nestedFolder = assert(project.view:findChild('ProjectFilefolder2', true))
local nestedDocument = assert(project.view:findChild('ProjectFilefile2', true))
assert(nestedDocument.parent == nestedFolder:findChild('ItemsPresenter') and not nestedFolder.IsExpanded,
    'Project records preserve a second folder level independently of the root expansion')
local documents = project:documents()
assert(#documents == 2 and documents[2].label == 'Screens/Nested/Details',
    'Open Document menus traverse native records even inside collapsed folders')
local opened, oldOpen = {}, Workspace.open
Workspace.open = function(path) opened[#opened + 1] = path end
documents[2].action()
nestedFolder.Header:post('Node.LeftDoubleClick', { x = 0, y = 0 })
settle()
assert(nestedFolder.IsExpanded, 'Project folder Header actions find their owning nested container')
nestedDocument.Header:post('Node.LeftDoubleClick', { x = 0, y = 0 })
settle()
Workspace.open = oldOpen
assert(#opened == 2 and opened[1] == 'Example/Screens/Nested/Details' and opened[2] == opened[1],
    'Menu and authored document Header actions open the native SourcePath')
local projectRequests = #requests
nestedFolder.IsExpanded = false
project:reload()
settle()
assert(#requests == projectRequests + 1 and project.view:findChild('ProjectFilefolder2', true) == nestedFolder
    and not nestedFolder.IsExpanded and project.view:findChild('ProjectFilefile2', true) == nestedDocument,
    'A project refresh retains nested folder state and document containers')
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
local referenceRow = inspector.view:findChild('Propertyreference', true)
local resourceButton = referenceRow:findChild('PropertyReference', true)
assert(resourceButton.Visible and referenceRow:findChild('ReferenceLabel', true).Text == 'Picture',
    'A resource has a labelled object selector instead of an empty text input')
assert(not referenceRow:findChild('PropertyText', true).Visible, 'Resource links are visually distinct from editable strings')
local paddingRow = assert(inspector.view:findChild('Propertypadding', true))
local fields = assert(paddingRow:findChild('CompoundFields', true))
local left = assert(fields:findChild('Fieldleft', true)):findChild('ComponentValue', true)
local top = assert(fields:findChild('Fieldtop', true)):findChild('ComponentValue', true)
assert(left.Text == '1' and top.Text == '2' and left.ActualWidth >= 28,
    ('A compound presents readable labelled native inputs (%s, %s; width %s, container %s)'):format(left.Text, top.Text, left.ActualWidth, fields.ActualWidth))
assert(left.ActualY == top.ActualY, 'Compound components share one compact row')
assert(fields:findChild('Fieldleft', true):findChild('ComponentLabel', true).ActualWidth > 0, 'Compact edge labels retain their intrinsic measured width')
left:setFocus()
left.Cursor = 0
left:post('Node.KeyDown', { keyCode = 131 })
settle()
left:post('Node.KeyDown', { keyCode = 127 })
settle()
assert(left.Text == '', 'The XML key handler preserves native cursor movement and deletion')
left:post('Node.TextInput', { text = '9' })
settle()
assert(left.Text == '9', 'Native text entry edits a compact field')
local beforeSubmit = #edits
left:post('Node.KeyDown', { keyCode = 13 })
settle()
assert(#edits == beforeSubmit + 1 and edits[#edits][2] == 'Node.PaddingLeft' and edits[#edits][3] == '9', 'Enter commits a component once and targets only its qualified leaf')
local beforeBlur = #edits
top.Text = '8'
top:post('Node.KillFocus')
settle()
assert(#edits == beforeBlur + 1 and edits[#edits][2] == 'Node.PaddingTop', 'Leaving a component field commits its edit')
top.Text = 'unfinished'
top:post('Node.KeyDown', { keyCode = 27 })
settle()
assert(top.Text == '8', 'Escape restores the last committed component value')
local summary = inspector.view:findChild('Propertyfont', true):findChild('PropertySummary', true)
assert(summary.Visible and summary.Text:find('Arial', 1, true), 'A mixed compound shows a concise expandable summary')
summary:send('Node.LeftButtonUp')
settle()
assert(requests[#requests]:find('expand=TextBlock.Font', 1, true), 'Disclosure asks the runtime for compound details')
local color = inspector.view:findChild('Propertycolor', true):findChild('PropertyColor', true)
assert(color.Visible and color:findChild('ColorValue', true).Text == '#4a8cf7', 'Colors combine a swatch and an exact editable value')
local enumButton = inspector.view:findChild('Propertyalign', true):findChild('PropertyEnum', true)
enumButton:send('Node.LeftButtonUp')
settle()
local picker = assert(Popups.top(), 'Enum opens a dropdown')
local options = inspector.source.Items:findChild('align'):findChild('Options')
assert(picker.list.ItemsSource == options and picker.list.SelectedValue == 'Left', 'Dropdown directly presents native enum options and current selection')
picker.view:post('Node.KeyDown', { keyCode = 129 })
settle()
picker.view:post('Node.KeyDown', { keyCode = 13 })
settle()
assert(not Popups.top() and edits[#edits][2] == 'TextBlock.TextAlignment' and edits[#edits][3] == 'Right', 'Dropdown supports keyboard selection and commits enum values')
resourceButton:send('Node.LeftButtonUp')
settle()
picker = assert(Popups.top(), 'Resource selector opens its own picker')
assert(picker.list.ItemsSource == picker.source.Items and picker.list.SelectedValue == 'Example/Images/Picture', 'Resource response feeds the native list directly')
local other = assert(picker.view:findChild('Choiceother', true))
other:send('Node.LeftButtonUp')
settle()
assert(not Popups.top() and edits[#edits][2] == 'ImageView.Source' and edits[#edits][3] == 'Example/Images/Other', 'Choosing a resource assigns its canonical path')
local ids = table.concat(inspector:propertyIds(), '|')
assert(ids:find('Node.PaddingBottom', 1, true), 'Collapsed compound leaves remain available to binding completion')
local filterFields = inspector.view:findChild('Propertyfilter', true):findChild('CompoundFields', true)
local blurRow = filterFields:findChild('Fieldblur', true)
local blurLabel, blurInput = blurRow:findChild('ComponentLabel', true), blurRow:findChild('ComponentValue', true)
assert(blurLabel.ActualX + blurLabel.ActualWidth <= blurInput.parent.ActualX and blurInput.ActualWidth >= 28,
    'Descriptive component labels fit alongside inputs without overlapping their values')
local draftField = fields:findChild('Fieldleft', true):findChild('ComponentValue', true)
local draftRecord = fields:findChild('Fieldleft', true).DataContext
local original, editCount = draftRecord.Value, #edits
draftField.Text = '17'
for _, appearance in ipairs { 'Dark', 'Light' } do
    screen.Appearance = appearance
    settle()
    assert(draftField.Text == '17' and draftRecord.Value == original,
        'Theme changes preserve a native editing draft without changing its committed value')
end
draftField:post('Node.KeyDown', { keyCode = 27 })
settle()
assert(draftField.Text == original and #edits == editCount, 'Escape discards a draft without a runtime command')
hierarchy:expandAll(false)
settle()
hierarchy:startSearch('Title')
settle()
assert(requests[#requests]:find('filter=Title', 1, true), 'Search is a runtime data-source query')
assert(rootItem.IsExpanded and groupItem.IsExpanded,
    'The editor reveals filtered results by expanding their native ancestors')
local retainedItems, selectedBeforeError = hierarchy.source.Items, hierarchy.list.SelectedValue
hierarchy.source.Content = '<response status="error">Runtime disconnected</response>'
hierarchy.source:send('XmlDataSource.Reload')
settle()
local errorLabel = assert(hierarchy.view:findChild('DataSourceError', true))
assert(errorLabel.Visible and errorLabel.Text == 'Runtime disconnected', 'Source errors bind directly into the panel')
assert(hierarchy.source.Items == retainedItems and hierarchy.view:findChild('SceneNodenode-2', true) == titleItem
    and rootItem.IsExpanded and groupItem.IsExpanded and hierarchy.list.SelectedValue == selectedBeforeError,
    'A failed refresh keeps the last successful records, containers, expansion and selection')
hierarchy:clearSearch()
settle()
assert(not errorLabel.Visible and not hierarchy.source.HasError and hierarchy.source.Items == retainedItems,
    'Clearing search recovers the existing tree after a failed refresh')
assert(hierarchy.view:findChild('SceneNodenode-202', true) == nestedItem and rootItem.IsExpanded,
    'Successful recovery preserves the retained nested containers and local state')
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
