-- HeaderedItemsControl inherited property elements use concrete WPF-style names.
local core = require 'orca.core'
local ui = require 'orca.UIKit'
local fs = require 'orca.filesystem'
local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = 'NoResize' }
local function settle()
    core.flushQueue()
    core.advanceFrame()
    core.runAllPrograms(screen)
    core.flushQueue()
    screen:UpdateLayout(400, 300)
end
local authored = assert(fs.loadObjectFromXmlString [[
<TreeView>
  <TreeViewItem Name="Authored" HeaderHeight="20" Indent="18">
    <TreeViewItem.Header><Grid Columns="1fr"><TextBlock Name="Caption" Text="Authored header"/></Grid></TreeViewItem.Header>
    <TreeViewItem Name="AuthoredChild" HeaderHeight="26" IsExpanded="true">
      <HeaderedItemsControl.Header><TextBlock Text="Child"/></HeaderedItemsControl.Header>
      <TreeViewItem Name="Grandchild" HeaderHeight="30">
        <TreeViewItem.Header><TextBlock Text="Grandchild"/></TreeViewItem.Header>
      </TreeViewItem>
    </TreeViewItem>
  </TreeViewItem>
  <TreeViewItem Name="Sibling">
    <TreeViewItem.Header><TextBlock Text="Sibling"/></TreeViewItem.Header>
  </TreeViewItem>
</TreeView>]])
screen:addChild(authored)
settle()
local authoredItem = authored:findChild('Authored', true)
local authoredChild = authored:findChild('AuthoredChild', true)
local grandchild = authored:findChild('Grandchild', true)
local sibling = authored:findChild('Sibling')
assert(authoredItem.Header:findChild('Caption').Text == 'Authored header' and authoredItem:findChild('ItemsPresenter'):findChild('AuthoredChild'),
    'Concrete TreeViewItem.Header XML property syntax and authored children share native ownership')
assert(authoredItem.parent == authored and sibling.parent == authored, 'The tree owns only authored root containers')
assert(authoredChild.parent ~= authored and grandchild.parent == authoredChild:findChild('ItemsPresenter'),
    'Authored descendants belong to their item presenter rather than the root')
assert(authoredItem.Header.parent == authoredItem:findChild('HeaderPresenter') and authoredChild.Header.Text == 'Child',
    'Concrete and declaring-class Header qualifiers both mount visual header content')
assert(not authoredItem.IsExpanded and authoredChild.IsExpanded and authoredItem.ActualHeight == 20,
    'An authored expanded descendant does not contribute layout while its parent is collapsed')
authoredItem:send('TreeViewItem.ToggleExpanded')
settle()
assert(authoredItem.IsExpanded and authoredItem.ActualHeight == 76 and authoredChild.ActualHeight == 56,
    'Authored HeaderHeight and child expansion determine each level of recursive layout')
assert(authoredItem:findChild('ItemsPresenter').ActualX == 18 and authoredChild:findChild('ItemsPresenter').ActualX == 16,
    'Each authored child presenter applies its own inherited or explicit indentation')
authoredItem:send('TreeViewItem.ToggleExpanded')
settle()
assert(authoredItem.ActualHeight == 20 and authoredChild.IsExpanded and authored:findChild('Grandchild', true) == grandchild,
    'Collapsing an authored branch retains nested expansion and objects')

-- The declarative template carries child bindings, while each Header instance
-- inherits the generated container's record rather than the TreeView's context.
local holder = core.DataObject { Name = 'Holder' }
local roots = holder:addChild(core.DataObject { Name = 'Roots' })
local schema = [[<Schema><Entity Name="TreeRecord">
  <Column Name="Key" Type="string"/><Column Name="AlternateKey" Type="string"/>
</Entity></Schema>]]
local prototypes = assert(fs.loadXmlDataStringWithSchema('<Rows><TreeRecord name="Proto"/></Rows>', schema))
local prototype = assert(prototypes:findChild('Proto'))
local function record(name, key, alternate)
    local value = prototype:instantiate()
    value.Name, value.Key, value.AlternateKey = name, key, alternate
    return value
end
local parent = roots:addChild(record('Parent', '/scene/parent', 'Родитель'))
local children = parent:addChild(core.DataObject { Name = 'Children' })
local leaf = children:addChild(record('Leaf', '/scene/Лист 🌿', '葉 / 子'))
leaf:addChild(core.DataObject { Name = 'Children' })
local other = roots:addChild(record('Other', '/scene/other', 'Другой'))
other:addChild(core.DataObject { Name = 'Children' })
local bound = assert(fs.loadObjectFromXmlString [[
<TreeView Name="Bound" ItemsSource="{Binding DataContext/Roots}" ItemNamePrefix="Row" ValueProperty="Key">
  <TreeView.ItemTemplate>
    <TreeViewItem ItemsSource="{Binding DataContext/Children}" HeaderHeight="25" Indent="12">
      <TreeViewItem.Header>
        <Grid Columns="1fr"><TextBlock Name="BoundCaption" Text="{Binding DataContext/Name}"/></Grid>
      </TreeViewItem.Header>
    </TreeViewItem>
  </TreeView.ItemTemplate>
</TreeView>]])
screen:addChild(bound)
bound.DataContext = holder
settle()
local parentView = assert(bound:findChild('RowParent'))
local otherView = assert(bound:findChild('RowOther'))
local leafView = assert(parentView:findChild('ItemsPresenter'):findChild('RowLeaf'))
local parentCaption = parentView.Header:findChild('BoundCaption')
local leafCaption = leafView.Header:findChild('BoundCaption')
assert(bound.ItemsSource == roots and parentView.ItemsSource == children and leafView.DataContext == leaf,
    'XML root and recursive ItemsSource bindings resolve from their respective data scopes')
assert(parentCaption.Text == 'Parent' and leafCaption.Text == 'Leaf' and otherView.Header:findChild('BoundCaption').Text == 'Other',
    'Cloned XML headers bind independently at root and descendant levels')
assert(parentView.Header ~= leafView.Header and parentView.Header ~= otherView.Header and leafView.HeaderHeight == 25,
    'An inherited XML ItemTemplate clones Header objects and authored settings for every level')
leaf.Name = 'RenamedLeaf'
settle()
assert(leafCaption.Text == 'RenamedLeaf' and parentCaption.Text == 'Parent' and leafView.DataContext == leaf,
    'A bound header follows its own retained record without contaminating its parent template')
bound:send('TreeView.SelectItem', { Value = '/scene/Лист 🌿' })
settle()
assert(bound.SelectedItem == leaf and bound.SelectedValue == leaf.Key and parentView.IsExpanded and leafView.IsSelected and not authoredItem.IsSelected,
    'A declaratively bound tree reveals descendants without affecting a neighboring authored tree')
bound.ValueProperty = 'AlternateKey'
settle()
assert(bound.SelectedItem == leaf and bound.SelectedValue == '葉 / 子' and leafView.IsSelected,
    'Changing ValueProperty rebinds existing containers to another typed field without losing selection')
leaf.AlternateKey = '葉 / 改名 🪴'
settle()
assert(bound.SelectedItem == leaf and bound.SelectedValue == '葉 / 改名 🪴' and parentCaption.Text == 'Parent',
    'A selected Unicode key can change independently of display names and ancestor keys')
bound.SelectedValue = 'Другой'
settle()
assert(bound.SelectedItem == other and otherView.IsSelected and not leafView.IsSelected,
    'Selection assignment uses the new typed ValueProperty across root and descendant containers')
print('PASS: TreeView XML headers, authored recursive layout, template inheritance and nested binding scopes')
