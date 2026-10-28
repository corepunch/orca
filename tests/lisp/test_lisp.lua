local test = require 'orca.test'
local core = require 'orca.core'
local filesystem = require 'orca.filesystem'
require 'orca.UIKit'

local function load(source)
  local node = filesystem.loadObjectFromLispString(source)
  test.expect(node ~= nil, 'Lisp scene loads')
  return node
end

local root = load([[
; Names, whitespace, comments, escapes, and unicode are independent of XML.
(Screen Name = "Root" Width=100 Height=80 ResizeMode="NoResize"
  (TextBlock Name="Label" Text="A & B <tag> \"quoted\"\nПривет" FontSize=14)
  (Node2D Name="Computed" Width=(+ (bind "../Width") 10)
    Height=(if (step 90 (bind "../Width")) 40 20)))
]])
test.expect_eq(root:findChild('Label').Text, 'A & B <tag> "quoted"\nПривет', 'string escaping')
core.flushQueue()
core.advanceFrame()
core.runAllPrograms(root)
test.expect_eq(root:findChild('Computed').Width, 110, 'prefix arithmetic reads sibling property')
test.expect_eq(root:findChild('Computed').Height, 40, 'nested expression evaluates')
root.Width = 50
root:UpdateLayout()
core.flushQueue()
core.advanceFrame()
core.runAllPrograms(root)
test.expect_eq(root:findChild('Computed').Width, 60, 'binding updates when source changes')
test.expect_eq(root:findChild('Computed').Height, 20, 'conditional recomputes')

local owner = load([[(Node2D Name="Owner" DataContext=(DataObject Name="Data"))]])
core.flushQueue()
core.advanceFrame()
core.runAllPrograms(root)
test.expect(owner.DataContext ~= nil, 'node-valued property assigned')
test.expect_eq(owner.DataContext.Name, 'Data', 'node value keeps its properties')

local panel = load([[(Node2D Name="Panel" HorizontalPadding=(if true (vector2 12 14) (vector2 2 4)))]] )
core.flushQueue()
core.advanceFrame()
core.runAllPrograms(panel)
test.expect_eq(panel.PaddingLeft, 12, 'struct-valued expression compiles')
test.expect_eq(panel.PaddingRight, 14, 'vector components preserved')

for _, invalid in ipairs {
  '(Node2D', '(Node2D Name="oops)', '(Node2D) (Node2D)',
  '(Node2D Width=)', '(Node2D Width=1 Width=2)',
  '(Node2D Width=(bind "#MissingProperty"))',
  '(Node2D Width=(bind "#Root/"))',
  '(Node2D Width=(bind))', '(Node2D Width=(bind "Width" "Height"))',
  '(Node2D Width=(if 1 2))', '(Node2D Width=(+ 1 2 3 4 5))',
  '(Node2D Width=(if true 1 (missing 2)))',
  '(Node2D Name="bad\\q")',
} do
  test.expect(filesystem.loadObjectFromLispString(invalid) == nil,
              'malformed Lisp must not return a partial tree: ' .. invalid)
end
local nested = '(Node2D '..string.rep('(Node2D ', 130)..string.rep(')', 131)
test.expect(filesystem.loadObjectFromLispString(nested) == nil, 'nesting limit enforced')
print('PASS: Lisp scalars, objects, reactive expressions, syntax errors')

local encoded = filesystem.serializeObjectToLispString(root)
test.expect(encoded:sub(1, 1) == '(', 'serializer emits Lisp')
local copy = load(encoded)
core.flushQueue()
core.advanceFrame()
core.runAllPrograms(copy)
test.expect_eq(copy:findChild('Label').Text, root:findChild('Label').Text, 'roundtrip escaped text')
copy.Width = 75
copy:UpdateLayout()
core.flushQueue()
core.advanceFrame()
core.runAllPrograms(copy)
test.expect_eq(copy:findChild('Computed').Width, 85, 'roundtrip keeps live binding')
print('PASS: Lisp serialization roundtrip')

for _, value in ipairs { '<hello>', '{hello}', '{Binding Width}', '@handle', '' } do
  local label = load('(TextBlock Text="' .. value .. '")')
  test.expect_eq(label.Text, value, 'quoted property strings stay literal')
end
for _, source in ipairs {
  '(Node2D (MisspelledNode))',
  '(Node2D DataContext=(MisspelledNode))',
  '(Node2D Triggers=(list (MisspelledNode)))',
} do
  test.expect(filesystem.loadObjectFromLispString(source) == nil, 'unknown node rejects whole scene')
end
collectgarbage(); collectgarbage()
local before = core.objectCount()
for i = 1, 10 do
  local tree = load([[(Node2D DataContext=(DataObject Name="data")
    LeftButtonUp=(HideAction) RightButtonUp=(HideAction)
    (Node2D Name="Child"))]])
  tree = nil
  core.flushQueue()
  collectgarbage(); collectgarbage()
end
test.expect_eq(core.objectCount(), before, 'scene loader transfers ownership and releases nested nodes/actions')
print('PASS: literal strings, unknown nodes, loaded object lifetimes')

test.expect(filesystem.loadObjectFromXml == nil, 'XML scene file API removed')
test.expect(filesystem.loadObjectFromXmlString == nil, 'XML scene string API removed')
local data = filesystem.loadXmlDataStringWithSchema('<Node2D Width="12"/>', '')
test.expect_eq(data.className, 'DataObject', 'XML datasource parsing cannot construct a scene class')
print('PASS: XML scene APIs removed; XML datasource parser is data-only')
local rows = filesystem.loadXmlDataStringWithSchema(
  '<LispTestRow><LispTestRow name="one" Title="kept"/></LispTestRow>',
  '<Schema><Entity Name="LispTestRow"><Column Name="Title" Type="string"/></Entity></Schema>')
test.expect_eq(rows:findChild('one').Title, 'kept', 'same-name XML data collection preserves schema records')

-- Selection owns typed rows; an explicitly retained row also outlives its source.
collectgarbage(); collectgarbage()
local list_baseline = core.objectCount()
for i = 1, 5 do
  local list = load([[(ListBox ValueProperty="Title" SelectedValue="first"
    ItemsSource=(DataObject
      (LispTestRow Name="one" Title="first")
      (LispTestRow Name="two" Title="second"))
    ItemTemplate=(Node2D Width=10 Height=10))]])
  test.expect_eq(list.SelectedItem.Title, 'first', 'typed row selected')
  list.SelectedValue = 'second'
  test.expect_eq(list.SelectedItem.Title, 'second', 'selection owns replacement row')
  local selected = list.SelectedItem
  list.ItemsSource = nil
  list:clear()
  collectgarbage(); collectgarbage()
  test.expect_eq(list.SelectedItem, nil, 'removing the source clears selection')
  test.expect_eq(selected.Title, 'second', 'retained typed row outlives its source')
  selected = nil
  list = nil
  core.flushQueue()
  collectgarbage(); collectgarbage()
end
test.expect_eq(core.objectCount(), list_baseline, 'typed list rows and selection released after GC')
print('PASS: typed ListBox datasource and selection lifetime')

-- bind options carry the binding mode and enabled state only where bind is a
-- property's whole value; elsewhere they are rejected.
local form = load([[(StackView DataContext=(LispTestRow Name="Draft" Title="draft")
  (Input Name="Edit" Text=(bind "DataContext/Title" Mode=TwoWay))
  (Input Name="Off" Text="kept" TextRun.Text=(bind "DataContext/Title" Mode=TwoWay Enabled=false))
  (ImageView Name="Shown" Node.Visible=(bool "image.png"))
  (ImageView Name="Hidden" Node.Visible=(bool "")))]])
core.flushQueue()
core.advanceFrame()
core.runAllPrograms(form)
test.expect_eq(form:findChild('Edit').Text, 'draft', 'Mode=TwoWay initializes from the model')
test.expect_eq(form:findChild('Off').Text, 'kept', 'Enabled=false does not pull')
form:findChild('Edit').Text = 'Edited'
core.flushQueue()
test.expect_eq(form.DataContext.Title, 'Edited', 'Mode=TwoWay writes edits back')
test.expect_eq(form:findChild('Shown').Visible, true, 'bool of a non-empty string is true')
test.expect_eq(form:findChild('Hidden').Visible, false, 'bool of an empty string is false')
for _, source in ipairs {
  '(Node2D Width=(+ (bind "Height" Mode=TwoWay) 1))',
  '(Input Text=(bind "DataContext/Name" Source="x"))',
  '(Input Text=(bind "DataContext/Name" Mode=TwoWay Mode=OneWay))',
} do
  test.expect(filesystem.loadObjectFromLispString(source) == nil, 'rejects ' .. source)
end
print('PASS: bind modes, enabled state and string truthiness')
