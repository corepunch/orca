-- Exercise public input dispatch, selection events and visible-tree navigation.
local orca = require 'orca'
local core = require 'orca.core'
local ui = require 'orca.UIKit'
local key = { up = 128, down = 129, left = 130, right = 131, home = 151, finish = 152 }

local function record(name, children)
    local item = core.DataObject { Name = name }
    local source = item:addChild(core.DataObject { Name = 'Children' })
    for _, child in ipairs(children or {}) do source:addChild(child) end
    return item
end

local function fixture(empty)
    local screen = ui.Screen { Width = 400, Height = 360, ResizeMode = 'NoResize' }
    local roots = core.DataObject { Name = 'Roots' }
    local a21, a1, b = record('A21'), record('A1'), record('B')
    local a2 = record('A2', { a21 })
    local a = record('A', { a1, a2 })
    if not empty then roots:addChild(a) roots:addChild(b) end
    local header = ui.TextBlock { Name = 'Caption' }
    header:attachPropertyProgram('TextRun.Text', '{DataContext/Name}', 'OneWay', true)
    local template = ui.TreeViewItem { Header = header }
    template:attachPropertyProgram('ItemsControl.ItemsSource', '{DataContext/Children}', 'OneWay', true)
    local events = {}
    local tree = screen:addChild(ui.TreeView {
        ItemsSource = roots, ItemTemplate = template, ItemNamePrefix = 'Item',
        SelectionChanged = function(_, e)
            events[#events + 1] = { old = e.OldValue, value = e.SelectedValue }
        end,
    })
    local function pump()
        core.flushQueue()
        while true do
            local message = orca.system.peekMessage()
            if not message then return end
            orca.system.dispatchMessage(screen, message)
        end
    end
    local function settle()
        pump()
        core.advanceFrame()
        core.runAllPrograms(screen)
        pump()
        screen:UpdateLayout(400, 360)
    end
    local f = { screen = screen, tree = tree, events = events, settle = settle,
        a = a, a1 = a1, a2 = a2, a21 = a21, b = b }
    function f:item(name) return assert(tree:findChild('Item' .. name, true), name .. ' container exists') end
    function f:tap(target)
        local x, y, parent = target.ActualWidth / 2, target.ActualHeight / 2, target
        while parent and parent.ActualX do
            x, y = x + parent.ActualX, y + parent.ActualY
            parent = parent.parent
        end
        orca.system.dispatchMessage { target = screen, message = 'LeftButtonDown', x = x, y = y }
        orca.system.dispatchMessage { target = screen, message = 'LeftButtonUp', x = x, y = y }
        settle()
    end
    function f:press(code)
        orca.system.dispatchMessage { target = screen, message = 'KeyDown', key = code }
        settle()
    end
    function f:expectEvent(count, old, value)
        assert(#events == count, 'SelectionChanged fires exactly once per change, count=' .. #events)
        local event = assert(events[count])
        assert(event.old == old and event.value == value,
            ('SelectionChanged snapshot: expected %q -> %q, got %q -> %q'):format(old, value, event.old, event.value))
        assert(tree.SelectedValue == value, 'The selection is coherent when its event is delivered')
    end
    function f:close() screen:clear() pump() end
    settle()
    return f
end

local function pointer_and_events()
    local f = fixture()
    local ca, ca1, cb = f:item('A'), f:item('A1'), f:item('B')
    assert(#f.events == 0 and f.tree.SelectedItem == nil, 'Creating a tree does not invent a selection')
    f:tap(ca.Header)
    f:expectEvent(1, '', 'A')
    assert(ca.IsSelected and not ca.IsExpanded, 'Clicking Header selects without toggling disclosure')
    f:tap(ca:findChild('Expander', true))
    assert(ca.IsExpanded and #f.events == 1, 'Disclosure pointer input expands without changing selection')
    f:tap(ca1.Header)
    f:expectEvent(2, 'A', 'A1')
    assert(f.tree.SelectedItem == f.a1 and ca1.IsSelected and not ca.IsSelected,
        'Pointer selection is owned by the nested item, with one root SelectedItem')
    f:tap(ca1.Header)
    assert(#f.events == 2, 'Clicking an already selected header emits no duplicate event')
    f:tap(ca:findChild('Expander', true))
    f:expectEvent(3, 'A1', 'A')
    assert(not ca.IsExpanded and not ca:findChild('ItemsPresenter').Visible,
        'Collapsing through pointer input hides the selected descendant and selects its parent')
    -- A leaf keeps the disclosure-sized area in its Header, but it is not a toggle.
    f:tap(cb:findChild('Expander', true))
    f:expectEvent(4, 'A', 'B')
    assert(not cb.IsExpanded, 'The leaf disclosure area selects its header without expanding an empty branch')
    cb:send('TreeViewItem.ToggleExpanded')
    f.settle()
    assert(not cb.IsExpanded and #f.events == 4, 'ToggleExpanded on a leaf is a no-op')
    f.tree:send('ItemsControl.Refresh')
    f.settle()
    assert(#f.events == 4, 'A refresh retaining selection emits no spurious selection event')
    f.tree.SelectedValue = 'B'
    f.settle()
    assert(#f.events == 4, 'Assigning the current value emits no event')
    assert(not f.tree:send('TreeView.SelectItem', { Value = 'Missing' }), 'SelectItem reports a missing value')
    assert(not f.tree:send('TreeView.SelectItem'), 'SelectItem rejects an omitted value')
    f.settle()
    assert(f.tree.SelectedItem == f.b and #f.events == 4, 'A failed SelectItem preserves existing selection')
    f.tree.SelectedValue = 'Missing'
    f.settle()
    f:expectEvent(5, 'B', '')
    assert(f.tree.SelectedItem == nil and not cb.IsSelected, 'An unmatched property assignment clears selection')
    f.tree.SelectedValue = ''
    f.settle()
    assert(#f.events == 5, 'Clearing an empty selection is a no-op')
    local nested = f:item('A21')
    nested.IsSelected = true
    f.settle()
    f:expectEvent(6, '', 'A21')
    assert(ca.IsExpanded and f:item('A2').IsExpanded and f.tree.SelectedItem == f.a21,
        'Assigning IsSelected reveals every ancestor of a deeply nested item')
    -- Queue two changes before pumping: event strings must be independent snapshots.
    f.tree.SelectedValue = 'A1'
    f.tree.SelectedValue = 'B'
    f.settle()
    assert(#f.events == 8 and f.events[7].old == 'A21' and f.events[7].value == 'A1'
        and f.events[8].old == 'A1' and f.events[8].value == 'B',
        'Queued selection events retain their own old and new values')
    f:close()
end

local function keyboard_navigation()
    local f = fixture()
    f:tap(f:item('A').Header) -- Focus a container through the actual pointer routing path.
    local steps = {
        { key.up, 'A' }, -- first item boundary
        { key.left, 'A' }, -- a collapsed root has no parent
        { key.right, 'A' }, -- expand, retaining selection
        { key.right, 'A1' },
        { key.down, 'A2' },
        { key.right, 'A2' },
        { key.right, 'A21' },
        { key.down, 'B' },
        { key.down, 'B' }, -- last visible item boundary
        { key.up, 'A21' },
        { key.home, 'A' },
        { key.left, 'A' }, -- collapse hides all descendants from subsequent navigation
        { key.down, 'B' },
        { key.up, 'A' },
        { key.finish, 'B' },
        { key.home, 'A' },
    }
    for index, step in ipairs(steps) do
        f:press(step[1])
        assert(f.tree.SelectedValue == step[2], 'Focused keyboard navigation follows visible order at step ' .. index)
    end
    local before = #f.events
    assert(not f.tree:send('Node.KeyDown', { keyCode = 65 }), 'Unrecognized keys remain available to other handlers')
    f.settle()
    assert(#f.events == before and f.tree.SelectedValue == 'A', 'Unrecognized keys preserve selection')
    f:close()
end

local function empty_and_independent_trees()
    local empty, other = fixture(true), fixture()
    for _, code in pairs(key) do
        assert(empty.tree:send('Node.KeyDown', { keyCode = code }), 'Empty trees handle navigation safely')
    end
    empty.tree:send('TreeView.ExpandAll', { IsExpanded = true })
    empty.tree:send('TreeView.ExpandAll', { IsExpanded = false })
    empty.settle()
    assert(empty.tree.SelectedItem == nil and (empty.tree.SelectedValue or '') == '' and #empty.events == 0,
        'Empty navigation and expansion keep an empty selection without events')
    other.tree:send('TreeView.SelectItem', { Value = 'A21' })
    other.settle()
    assert(other.tree.SelectedItem == other.a21 and #other.events == 1 and #empty.events == 0,
        'Independent tree roots do not share selection or events')
    empty:close()
    other:close()
end

local function presentation_states()
    local screen = ui.Screen {
        Width = 400, Height = 300, ResizeMode = 'NoResize',
        StyleSheet = ui.loadObjectFromCssString [[
            .treeview-header { border-width: 1; }
            .treeview-header:selected { border-width: 4; }
            .treeview-expander { opacity: 1; }
            .treeview-expander:checked { opacity: 0.25; }
        ]],
    }
    local tree = screen:addChild(ui.TreeView {})
    local parent = ui.TreeViewItem { Header = ui.TextBlock { Text = 'Parent' } }
    local child = parent:addChild(ui.TreeViewItem { Header = ui.TextBlock { Text = 'Child' } })
    tree:addChild(parent)
    local function settle()
        core.flushQueue()
        core.advanceFrame()
        core.runAllPrograms(screen)
        core.flushQueue()
        screen:UpdateLayout(400, 300)
    end
    settle()
    local parentHeader, childHeader = parent:findChild('HeaderPresenter'), child:findChild('HeaderPresenter')
    local expander = parentHeader:findChild('Expander')
    assert(parentHeader.BorderWidthLeft == 1 and childHeader.BorderWidthLeft == 1 and expander.Opacity == 1,
        'Generated header and expander parts participate in ordinary CSS styling')
    child.IsSelected = true
    settle()
    assert(childHeader.BorderWidthLeft == 4 and parentHeader.BorderWidthLeft == 1
        and parent.BorderWidthLeft == 0 and child.BorderWidthLeft == 0,
        'The selected pseudo-state styles only the selected Header, not an ancestor or its whole subtree')
    assert(expander.Opacity == 0.25, 'Expansion updates the expander checked pseudo-state')
    parent.IsExpanded = false
    settle()
    assert(parentHeader.BorderWidthLeft == 4 and childHeader.BorderWidthLeft == 1 and expander.Opacity == 1,
        'Collapsing updates both the disclosure style and selection style of the retained headers')
    screen:clear()
    core.flushQueue()
end

local function deep_tree()
    local depth = 64
    local data, cursor = {}, nil
    for level = depth, 1, -1 do
        cursor = record('Level' .. level, cursor and { cursor } or {})
        data[level] = cursor
    end
    local roots = core.DataObject { Name = 'DeepRoots' }
    roots:addChild(cursor)
    local prototype = ui.TreeViewItem { Header = ui.TextBlock {}, HeaderHeight = 16 }
    prototype:attachPropertyProgram('ItemsControl.ItemsSource', '{DataContext/Children}', 'OneWay', true)
    local screen = ui.Screen { Width = 1280, Height = 1600, ResizeMode = 'NoResize' }
    local tree = screen:addChild(ui.TreeView { ItemsSource = roots, ItemTemplate = prototype, ItemNamePrefix = 'Item' })
    local function settle()
        core.flushQueue()
        core.advanceFrame()
        core.runAllPrograms(screen)
        core.flushQueue()
        screen:UpdateLayout(1280, 1600)
    end
    settle()
    local root = assert(tree:findChild('ItemLevel1'))
    assert(root.ActualHeight == 16 and root:getNext() == nil, 'A deep collapsed tree lays out one root header')
    assert(tree:send('TreeView.SelectItem', { Value = 'Level64' }), 'Deep selection has no shallow tree-depth cutoff')
    settle()
    local owner, containers = tree, {}
    for level = 1, depth do
        local container = assert(owner:findChild('ItemLevel' .. level), 'Every level is owned by its direct presenter')
        assert(container.DataContext == data[level] and container.IsSelected == (level == depth),
            'Deep selection keeps record scopes and a single selected container at level ' .. level)
        assert(level == depth or container.IsExpanded, 'Deep selection reveals every ancestor')
        containers[level] = container
        owner = assert(container:findChild('ItemsPresenter'))
    end
    assert(tree.SelectedItem == data[depth] and root.ActualHeight == depth * 16,
        'Recursive measurement includes each revealed header once')
    tree:send('TreeView.ExpandAll', { IsExpanded = false })
    settle()
    assert(root.ActualHeight == 16 and tree.SelectedItem == data[1], 'Deep collapse restores one visible root and its selection')
    tree:send('TreeView.ExpandAll', { IsExpanded = true })
    settle()
    for level, container in ipairs(containers) do
        assert(tree:findChild('ItemLevel' .. level, true) == container and container.IsExpanded,
            'Deep expand-all retains the container at level ' .. level)
    end
    screen:clear()
    core.flushQueue()
end

local function focused_input_lifetimes()
    local function cycle()
        local f = fixture()
        f:tap(f:item('A').Header)
        f:tap(f:item('A'):findChild('Expander', true))
        f:tap(f:item('A1').Header)
        f:press(key.down)
        f.tree:send('TreeView.SelectItem', { Value = 'A21' })
        f.settle()
        assert(f.tree.SelectedItem == f.a21 and #f.events > 0, 'The lifetime fixture owns focused input and selection callbacks')
        f:close()
    end
    cycle()
    core.flushQueue()
    collectgarbage('collect')
    collectgarbage('collect')
    local baseline = core.objectCount()
    for _ = 1, 3 do
        cycle()
        core.flushQueue()
        collectgarbage('collect')
        collectgarbage('collect')
        assert(core.objectCount() == baseline, 'Focused containers and queued selection callbacks release with the tree')
    end
end

for _, case in ipairs {
    pointer_and_events, keyboard_navigation, empty_and_independent_trees, presentation_states, deep_tree, focused_input_lifetimes,
} do case() end
print('PASS: TreeView pointer dispatch, selection events, keyboard navigation, CSS parts and deep trees')
