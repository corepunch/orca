-- Collection replacement, local presentation ownership and release lifetimes.
local core = require 'orca.core'
local ui = require 'orca.UIKit'

local function collection(name, children)
    local source = core.DataObject { Name = name }
    for _, child in ipairs(children or {}) do source:addChild(child) end
    return source
end

local function record(name, children)
    local data = core.DataObject { Name = name }
    data:addChild(collection('Children', children))
    return data
end

local function template(caption, height)
    local header = ui.TextBlock { Text = caption or '', Name = 'Caption' }
    if not caption then header:attachPropertyProgram('TextRun.Text', '{DataContext/Name}', 'OneWay', true) end
    local item = ui.TreeViewItem { Header = header, HeaderHeight = height or 20 }
    item:attachPropertyProgram('ItemsControl.ItemsSource', '{DataContext/Children}', 'OneWay', true)
    return item
end

local function fixture()
    local screen = ui.Screen { Width = 320, Height = 300, ResizeMode = 'NoResize' }
    local function settle()
        core.flushQueue()
        core.advanceFrame()
        core.runAllPrograms(screen)
        core.flushQueue()
        screen:UpdateLayout(320, 300)
    end
    return screen, settle
end

local function item(tree, name)
    return assert(tree:findChild('Item' .. name, true), name .. ' container exists')
end

local function source_replacement()
    local screen, settle = fixture()
    local tree = screen:addChild(ui.TreeView { ItemNamePrefix = 'Item' })
    settle()
    assert(not tree:getFirstChild() and not tree.SelectedItem, 'A nil source starts empty and unselected')
    tree:send('TreeView.ExpandAll', { IsExpanded = true })
    assert(not tree:send('TreeView.SelectItem', { Value = 'Missing' }), 'Empty trees reject absent values')
    local oldSource = collection('Old', { record('First'), record('Second') })
    tree.ItemsSource = oldSource
    settle()
    local first, second = item(tree, 'First'), item(tree, 'Second')
    oldSource:findChild('Second'):moveBefore(oldSource:findChild('First'))
    settle()
    assert(tree:getFirstChild() == second and item(tree, 'First') == first, 'Root reordering retains containers')
    tree:send('TreeView.SelectItem', { Value = 'First' })
    tree.ItemsSource = nil
    settle()
    assert(not tree:getFirstChild() and not tree.SelectedItem and tree.SelectedValue == '',
        'Clearing the source removes generated roots and selection')
    oldSource:addChild(record('Detached'))
    settle()
    assert(not tree:getFirstChild(), 'The previous root source cannot repopulate a cleared tree')
    local replacement = collection('Replacement', { record('New') })
    tree.ItemsSource = replacement
    settle()
    local newItem = item(tree, 'New')
    assert(newItem.DataContext == replacement:findChild('New') and not tree:findChild('ItemFirst', true),
        'Replacement roots receive the new record scope')
    oldSource:clear()
    settle()
    assert(item(tree, 'New') == newItem, 'Mutation of the old source leaves replacement containers untouched')
    replacement:clear()
    settle()
    assert(not tree:getFirstChild(), 'Clearing a live collection removes all generated roots')
    screen:clear()
end

local function child_source_and_header()
    local screen, settle = fixture()
    local oldHeader = ui.TextBlock { Text = 'Old' }
    local owner = ui.TreeViewItem { Header = oldHeader, HeaderHeight = 18, Indent = 30, ItemNamePrefix = 'Item' }
    local tree = screen:addChild(ui.TreeView {})
    tree:addChild(owner)
    settle()
    local headerHost, host = owner:findChild('HeaderPresenter'), owner:findChild('ItemsPresenter')
    local expander = headerHost:findChild('Expander')
    assert(not owner.HasItems and not expander:findChild('Chevron').Visible, 'An empty item is a leaf')
    owner:send('TreeViewItem.ToggleExpanded')
    assert(not owner.IsExpanded, 'Disclosure does not expand a leaf')
    local oldChildren = collection('OldChildren', { record('Child') })
    owner.ItemsSource = oldChildren
    settle()
    assert(owner.HasItems and expander:findChild('Chevron').Visible, 'Adding a child source turns a leaf into a branch')
    owner.IsExpanded = true
    settle()
    local child = item(tree, 'Child')
    assert(child.parent == host and host.ActualX == 30 and host.ActualWidth == tree.ActualWidth - 30,
        'Indent applies to the owned child presenter only')
    assert(headerHost.ActualHeight == 18 and owner.ActualHeight == 18 + child.ActualHeight,
        'HeaderHeight contributes once above the visible children')
    local newHeader = ui.TextBlock { Text = 'New' }
    owner.Header = newHeader
    owner.HeaderHeight = 26
    owner.Indent = 12
    settle()
    assert(oldHeader.parent == nil and newHeader.parent == headerHost and item(tree, 'Child') == child,
        'Replacing Header detaches only old content and retains generated children')
    assert(headerHost.ActualHeight == 26 and host.ActualX == 12, 'Header and indent properties update layout after attachment')
    owner.Header = nil
    settle()
    assert(newHeader.parent == nil and headerHost:findChild('Expander') == expander and item(tree, 'Child') == child,
        'Clearing Header preserves disclosure and the child presenter')
    owner.ItemsSource = collection('ReplacementChildren', { record('Replacement') })
    settle()
    local replacement = item(tree, 'Replacement')
    oldChildren:addChild(record('Stale'))
    settle()
    assert(not tree:findChild('ItemStale', true) and replacement.parent == host,
        'Replacing a child collection unsubscribes the old source without replacing its presenter')
    tree:send('TreeView.SelectItem', { Value = 'Replacement' })
    owner.ItemsSource = nil
    settle()
    assert(not owner.HasItems and not host:getFirstChild() and not expander:findChild('Chevron').Visible,
        'Removing the final child collection restores leaf presentation')
    assert(not tree.SelectedItem and tree.SelectedValue == '', 'Removing a selected child source clears the tree selection')
    screen:clear()
end

local function headered_collection()
    local screen, settle = fixture()
    local source = collection('Rows', { record('One') })
    local rowTemplate = ui.TextBlock { Height = 15 }
    rowTemplate:attachPropertyProgram('TextRun.Text', '{DataContext/Name}', 'OneWay', true)
    local stack = screen:addChild(ui.StackView {})
    local owner = stack:addChild(ui.HeaderedItemsControl {
        Header = ui.TextBlock { Text = 'A collection header' }, HeaderHeight = 19, Indent = 8,
        ItemsSource = source, ItemTemplate = rowTemplate, ItemNamePrefix = 'Row',
    })
    settle()
    local host = assert(owner:findChild('ItemsPresenter'))
    local row = assert(host:findChild('RowOne'))
    assert(owner.HasItems and row.Text == 'One' and row.DataContext == source:findChild('One'),
        'HeaderedItemsControl owns ordinary item templates without requiring TreeViewItem children')
    assert(owner.Header.parent == owner:findChild('HeaderPresenter') and not owner:findChild('Expander', true),
        'A general headered collection presents Header without a tree disclosure control')
    assert(host.ActualX == 8 and owner.ActualHeight == 34, 'A headered collection arranges its header above its own item presenter')
    source:clear()
    settle()
    assert(not owner.HasItems and not host:getFirstChild() and owner.ActualHeight == 19,
        'Emptying a headered collection retains only its header in layout')
    source:addChild(record('Two'))
    settle()
    assert(owner.HasItems and host:findChild('RowTwo').Text == 'Two', 'A headered collection observes new rows after becoming empty')
    screen:clear()
end

local function templates_and_selection_removal()
    local screen, settle = fixture()
    local leaf = record('Leaf')
    local group = record('Group', { leaf })
    local branch = record('Branch', { group })
    local roots = collection('Roots', { branch })
    local originalTemplate = template()
    local tree = screen:addChild(ui.TreeView { ItemsSource = roots, ItemTemplate = originalTemplate, ItemNamePrefix = 'Item' })
    settle()
    local branchItem, groupItem, leafItem = item(tree, 'Branch'), item(tree, 'Group'), item(tree, 'Leaf')
    assert(branchItem.ItemTemplate == originalTemplate and groupItem.ItemTemplate == originalTemplate,
        'Recursive containers inherit their nearest owner template')
    tree:send('TreeView.SelectItem', { Value = 'Leaf' })
    settle()
    branch:removeFromParent()
    settle()
    assert(not tree.SelectedItem and tree.SelectedValue == '' and not tree:getFirstChild(),
        'Removing a selected descendant ancestor clears root selection without retaining the branch')
    roots:addChild(branch)
    settle()
    local oldBranch = item(tree, 'Branch')
    tree.ItemTemplate = template('Replacement header', 28)
    settle()
    local replaced = item(tree, 'Branch')
    assert(replaced ~= oldBranch and replaced.Header.Text == 'Replacement header' and replaced.HeaderHeight == 28,
        'Changing ItemTemplate replaces generated presentation for retained data')
    assert(item(tree, 'Leaf').Header.Text == 'Replacement header' and item(tree, 'Group').ItemTemplate == tree.ItemTemplate,
        'A replacement root template also defines recursive child presentation')
    local custom = template('Explicit child template', 16)
    replaced.ItemTemplate = custom
    settle()
    assert(item(tree, 'Group').Header.Text == 'Explicit child template' and item(tree, 'Leaf').Header.Text == 'Explicit child template',
        'An explicit per-item template overrides recursive inheritance below that item')
    local plain = ui.TextBlock {}
    plain:attachPropertyProgram('TextRun.Text', '{DataContext/Name}', 'OneWay', true)
    tree.ItemTemplate = plain
    settle()
    local wrapped = item(tree, 'Branch')
    assert(wrapped:getClassName() == 'TreeViewItem' and wrapped.Header:getClassName() == 'TextBlock'
        and wrapped.Header ~= plain and wrapped.Header.Text == 'Branch',
        'A non-container template is cloned into a TreeViewItem Header with the record context')
    tree.ItemTemplate = nil
    settle()
    local defaultItem = item(tree, 'Branch')
    assert(defaultItem ~= wrapped and defaultItem.Header.Text == 'Branch',
        'Clearing an explicit ItemTemplate restores the default container and bound header')
    screen:clear()
end

local function independent_trees()
    local screen, settle = fixture()
    local leaf = record('Leaf')
    local branch = record('Branch', { leaf })
    local roots = collection('Roots', { branch })
    local prototype = template()
    local a = screen:addChild(ui.TreeView { ItemsSource = roots, ItemTemplate = prototype, ItemNamePrefix = 'Item' })
    local b = screen:addChild(ui.TreeView { ItemsSource = roots, ItemTemplate = prototype, ItemNamePrefix = 'Item' })
    settle()
    local aBranch, bBranch = item(a, 'Branch'), item(b, 'Branch')
    a:send('TreeView.SelectItem', { Value = 'Leaf' })
    settle()
    assert(aBranch ~= bBranch and aBranch.Header ~= bBranch.Header and aBranch.IsExpanded and not bBranch.IsExpanded,
        'Trees sharing records and a template own independent headers and expansion state')
    assert(a.SelectedItem == leaf and not b.SelectedItem, 'Selection is local to each tree sharing the source')
    b:removeFromParent()
    branch:findChild('Children'):addChild(record('New'))
    settle()
    assert(item(a, 'New').parent == aBranch:findChild('ItemsPresenter') and a.SelectedItem == leaf,
        'A live tree continues observing shared data when another tree is detached')
    screen:clear()
end

local function authored_attachment_order()
    local screen, settle = fixture()
    local parentData, childData = record('Parent'), record('Child')
    local tree = screen:addChild(ui.TreeView {})
    local parent = tree:addChild(ui.TreeViewItem { Header = ui.TextBlock { Text = 'Parent' }, DataContext = parentData })
    settle()
    local child = parent:addChild(ui.TreeViewItem { Header = ui.TextBlock { Text = 'Child' }, DataContext = childData })
    settle()
    assert(child.parent == parent:findChild('ItemsPresenter') and parent.HasItems,
        'Adding an authored child after its parent is attached uses the owned presenter and updates HasItems')
    child.IsSelected = true
    settle()
    assert(parent.IsExpanded and child.IsSelected and tree.SelectedItem == childData,
        'A late authored child participates in root selection and reveals its parent')
    parent.IsExpanded = false
    settle()
    assert(parent.IsSelected and not child.IsSelected and tree.SelectedItem == parentData,
        'Collapsing a late authored child ancestor moves selection to that parent')
    screen:clear()
end

local cases = {
    { 'source replacement', source_replacement },
    { 'child sources and Header replacement', child_source_and_header },
    { 'generic HeaderedItemsControl collections', headered_collection },
    { 'templates and ancestor removal', templates_and_selection_removal },
    { 'shared-source lifetime', independent_trees },
    { 'authored attachment order', authored_attachment_order },
}
for _, case in ipairs(cases) do
    case[2]()
    core.flushQueue()
    collectgarbage('collect')
    collectgarbage('collect')
    print('PASS: TreeView ' .. case[1])
end

local baseline = core.objectCount()
for _ = 1, 3 do
    for _, case in ipairs(cases) do
        case[2]()
        core.flushQueue()
        collectgarbage('collect')
        collectgarbage('collect')
        assert(core.objectCount() == baseline, case[1] .. ' returns to the native object baseline')
    end
end
print('PASS: TreeView repeated source, Header, template and shared-source lifetimes')
