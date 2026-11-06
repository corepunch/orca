-- Hierarchical container ownership, retained expansion and tree selection.
local core = require 'orca.core'
local ui = require 'orca.UIKit'

local function records(name)
    return core.DataObject { Name = name }
end
local function node(name, children)
    local record = records(name)
    local collection = record:addChild(records('Children'))
    for _, child in ipairs(children or {}) do collection:addChild(child) end
    return record
end
local a1 = node('A1')
local a2 = node('A2')
local a = node('A', { a1, a2 })
local b = node('B')
local roots = records('Roots')
roots:addChild(a)
roots:addChild(b)

local header = ui.TextBlock { Name = 'Label' }
header:attachPropertyProgram('TextRun.Text', '{DataContext/Name}', 'OneWay', true)
local template = ui.TreeViewItem { Header = header, ItemNamePrefix = 'Item', HeaderHeight = 22 }
template:attachPropertyProgram('ItemsControl.ItemsSource', '{DataContext/Children}', 'OneWay', true)
local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = 'NoResize' }
local tree = screen:addChild(ui.TreeView {
    ItemsSource = roots, ItemTemplate = template, ItemNamePrefix = 'Item', ValueProperty = 'Name',
})
local function settle()
    core.flushQueue()
    core.advanceFrame()
    core.runAllPrograms(screen)
    core.flushQueue()
    screen:UpdateLayout(400, 300)
end
local function find(name) return assert(tree:findChild('Item' .. name, true), name .. ' container exists') end
settle()
local ca, cb, ca1, ca2 = find('A'), find('B'), find('A1'), find('A2')
assert(ca:getClassName() == 'TreeViewItem' and ca.parent == tree and cb.parent == tree,
    'TreeView owns only root containers')
local host = assert(ca:findChild('ItemsPresenter'))
assert(ca1.parent == host and ca2.parent == host and host.parent == ca,
    'The parent container owns descendants through its own presenter')
assert(ca.ItemsSource == a:findChild('Children') and tree.ItemsSource == roots,
    'Each ItemsControl binds its own direct collection')
assert(ca.Header ~= cb.Header and ca.Header ~= header and ca.Header.Text == 'A' and cb.Header.Text == 'B',
    'Each generated container receives a distinct cloned Header with its own DataContext')
assert(not ca.IsExpanded and not host.Visible and ca.ActualHeight == 22,
    'Collapsed containers measure only their header row')
local expander = assert(ca:findChild('HeaderPresenter'):findChild('Expander'))
local disclosurePoint = { x = 8, y = 11 }
local disclosureHit = expander:send('Node.HitTest', disclosurePoint)
assert(disclosureHit and screen:send('Node.HitTest', disclosurePoint) == disclosureHit
    and not ca.Header:send('Node.HitTest', disclosurePoint),
    'Pointer hit testing reaches the disclosure without overlapping Header content')
expander:send('Node.LeftButtonUp')
settle()
assert(ca.IsExpanded and host.Visible and ca.ActualHeight == 66, 'Disclosure expands the local child presenter')
assert(find('A1') == ca1 and find('A2') == ca2, 'Expansion retains descendant containers')
local leafExpander = cb:findChild('HeaderPresenter'):findChild('Expander')
assert(leafExpander.Visible and not leafExpander:findChild('Chevron').Visible,
    'Leaf rows reserve disclosure space without displaying a disclosure glyph')
local aLabel, bLabel = ca.Header, cb.Header
assert(aLabel.ActualX == bLabel.ActualX, 'Branch and leaf header contents stay aligned')

local selected = tree:send('TreeView.SelectItem', { Value = 'A2' })
settle()
assert(selected and tree.SelectedItem == a2 and tree.SelectedValue == 'A2' and ca2.IsSelected,
    'Root selection identifies descendant records without flattening the source')
assert(ca2:findChild('HeaderPresenter').Visible and not ca.IsSelected,
    'Selection belongs to the descendant header rather than its ancestor')
local temporaryRoot = node('Temporary')
roots:addChild(temporaryRoot)
settle()
assert(tree.SelectedItem == a2 and find('A2') == ca2, 'Root collection refresh preserves selection owned by a descendant container')
temporaryRoot:removeFromParent()
settle()

tree.SelectedValue = 'B'
settle()
assert(tree.SelectedItem == b and cb.IsSelected and not ca2.IsSelected, 'Assigning SelectedValue updates the selected descendant container')
tree.SelectedValue = ''
settle()
assert(tree.SelectedItem == nil and not cb.IsSelected, 'Clearing SelectedValue clears native selection')
tree:send('TreeView.SelectItem', { Value = 'A2' })
settle()

ca.IsExpanded = false
settle()
assert(tree.SelectedItem == a and tree.SelectedValue == 'A' and ca.IsSelected,
    'Collapsing the selected descendant returns selection to its parent')
tree:send('TreeView.SelectItem', { Value = 'A2' })
settle()
assert(ca.IsExpanded and find('A2') == ca2 and tree.SelectedItem == a2,
    'Selecting a hidden descendant reveals ancestors and preserves its container')
tree:send('Node.KeyDown', { keyCode = 130 })
settle()
assert(tree.SelectedItem == a, 'Left from a leaf selects its parent')
tree:send('Node.KeyDown', { keyCode = 130 })
settle()
assert(not ca.IsExpanded, 'Left from an expanded branch collapses it')
tree:send('Node.KeyDown', { keyCode = 131 })
settle()
assert(ca.IsExpanded, 'Right expands a collapsed branch')
tree:send('Node.KeyDown', { keyCode = 131 })
settle()
assert(tree.SelectedItem == a1, 'Right from an expanded branch selects its first child')
tree:send('Node.KeyDown', { keyCode = 129 })
settle()
assert(tree.SelectedItem == a2, 'Down follows visible nested containers')
tree:send('Node.KeyDown', { keyCode = 152 })
settle()
assert(tree.SelectedItem == b, 'End selects the last visible root')
cb.IsSelected = false
settle()
assert(tree.SelectedItem == nil and tree.SelectedValue == '', 'Clearing IsSelected clears the root selection')

local added = node('A3')
a:findChild('Children'):addChild(added)
settle()
local ca3 = find('A3')
assert(ca3.parent == host and find('A1') == ca1, 'Nested collection changes synchronize only its owner and retain siblings')
added:moveBefore(a1)
settle()
assert(host:getFirstChild() == ca3 and ca1.parent == host, 'Nested ordering retains existing containers')
tree:send('TreeView.SelectItem', { Value = 'A3' })
settle()
added.Name = 'Renamed'
settle()
assert(tree.SelectedValue == 'Renamed' and tree.SelectedItem == added,
    'Renaming a selected record updates its value without losing native identity')
added:removeFromParent()
settle()
assert(tree.SelectedItem == nil and find('A1') == ca1, 'Removing the selected nested record clears selection and retains siblings')

local c = node('C')
roots:addChild(c)
settle()
assert(find('C').parent == tree and find('A') == ca and find('B') == cb,
    'Adding a root does not recreate other root containers')
tree:send('TreeView.ExpandAll', { IsExpanded = false })
settle()
assert(not ca.IsExpanded and find('A1') == ca1, 'ExpandAll collapse is local and retains children')
tree:send('TreeView.ExpandAll', { IsExpanded = true })
settle()
assert(ca.IsExpanded and find('A1') == ca1, 'ExpandAll restore retains child identities')

-- Default containers wrap header data, while explicit Header content is still
-- a proper property-owned subtree when an item is created without a template.
local simpleSource = records('Simple')
simpleSource:addChild(records('Named'))
local simple = screen:addChild(ui.TreeView { ItemsSource = simpleSource })
settle()
local defaultItem = assert(simple:getFirstChild())
assert(defaultItem:getClassName() == 'TreeViewItem' and defaultItem.Header.Text == 'Named',
    'Non-container items receive a TreeViewItem and a default named header')
screen:clear()
core.flushQueue()
collectgarbage('collect')
local baseline = core.objectCount()
local function discard()
    local localHeader = ui.TextBlock { Text = 'Header' }
    local localTemplate = ui.TreeViewItem { Header = localHeader }
    local control = ui.TreeView { ItemsSource = roots, ItemTemplate = localTemplate }
    local owner = ui.Screen { Width = 200, Height = 100, ResizeMode = 'NoResize' }
    owner:addChild(control)
    core.flushQueue()
end
discard()
core.flushQueue()
collectgarbage('collect')
collectgarbage('collect')
assert(core.objectCount() == baseline, 'Tree headers, inherited templates and collection observers release without cycles')
print('PASS: hierarchical TreeView ownership, headers, expansion, selection, keys and collection retention')
