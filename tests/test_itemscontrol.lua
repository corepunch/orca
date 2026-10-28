local core = require "orca.core"
local ui = require "orca.UIKit"

local function children(owner)
  core.flushQueue()
  local rows = {}
  for child in owner.children do rows[#rows + 1] = child end
  return rows
end

local function expectRows(owner, expected)
  local rows = children(owner)
  assert(#rows == #expected, "generated row count differs from source")
  for index, data in ipairs(expected) do
    assert(rows[index].DataContext == data, "generated row order or DataContext differs from source")
  end
  return rows
end

local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
local a, b, c = core.DataObject { Name = "A" }, core.DataObject { Name = "B" }, core.DataObject { Name = "C" }
local source = core.DataObject {}
source:addChild(a)
source:addChild(b)
local template = ui.TextBlock { BackgroundColor = "#12345699" }
template:attachPropertyProgram("TextRun.Text", "{DataContext/Name}", "OneWay", true)
local list = ui.ListBox { ItemsSource = source, ItemTemplate = template }
screen:addChild(list)
local original = expectRows(list, { a, b })
core.advanceFrame()
core.runAllPrograms(screen)
assert(original[1].Text == "A" and original[2].Text == "B", "template bindings inherit each row context")
local color = original[1].BackgroundColor
assert(math.abs(color.R - 0x12 / 255) < 0.01 and math.abs(color.G - 0x34 / 255) < 0.01
  and math.abs(color.B - 0x56 / 255) < 0.01 and math.abs(color.A - 0.6) < 0.01,
  "template cloning must preserve translucent RGBA colors")

source:addChild(c)
local appended = expectRows(list, { a, b, c })
assert(appended[1] == original[1] and appended[2] == original[2], "append must retain existing views")

c:moveBefore(a)
local moved = expectRows(list, { c, a, b })
assert(moved[1] == appended[3] and moved[2] == original[1] and moved[3] == original[2], "move must retain all views")
b:moveBefore(nil)
assert(expectRows(list, { c, a, b })[3] == original[2], "moving the last item is idempotent")

a.Name = "Updated"
core.advanceFrame()
core.runAllPrograms(screen)
assert(expectRows(list, { c, a, b })[2].Text == "Updated", "record updates must reach retained template bindings")
a:removeFromParent()
local removed = expectRows(list, { c, b })
assert(removed[1] == appended[3] and removed[2] == original[2], "remove must retain surviving views")
assert(list.SelectedItem == nil, "removing selected source item clears SelectedItem")

local otherSource = core.DataObject {}
otherSource:addChild(a)
list.ItemsSource = otherSource
expectRows(list, { a })
source:addChild(core.DataObject { Name = "OldSourceOnly" })
expectRows(list, { a })

local firstTemplateRow = children(list)[1]
list.ItemTemplate = ui.Node2D { Name = "ReplacementTemplate" }
local replaced = expectRows(list, { a })
assert(replaced[1] ~= firstTemplateRow and replaced[1]:getClassName() == "Node2D", "template replacement recreates generated views")
local retained = replaced[1]
local shell = screen:addChild(ui.StackView {})
shell:addChild(list)
assert(expectRows(list, { a })[1] == retained, "reparenting a control preserves generated rows")
otherSource:clear()
expectRows(list, {})
list.ItemsSource = nil
otherSource:addChild(a)
expectRows(list, {})

local grid = ui.GridBox {}
screen:addChild(grid)
grid.ItemsSource = otherSource
local gridRows = expectRows(grid, { a })
assert(gridRows[1]:getClassName() == "TextBlock", "GridBox supplies the shared default item template")
otherSource:addChild(b)
assert(expectRows(grid, { a, b })[1] == gridRows[1], "GridBox observes collection changes")
local receivingSource = core.DataObject {}
local receivingGrid = ui.GridBox { ItemsSource = receivingSource }
screen:addChild(receivingGrid)
receivingSource:addChild(b)
assert(expectRows(grid, { a })[1] == gridRows[1], "reparenting data preserves surviving source views")
expectRows(receivingGrid, { b })
otherSource:addChild(b)
expectRows(receivingGrid, {})
expectRows(grid, { a, b })

local stack = ui.StackView {}
stack:addComponentByName("ItemsControl")
local header = stack:addChild(ui.TextBlock { Name = "AuthoredHeader" })
stack.ItemsSource = otherSource
stack.ItemTemplate = template
stack:send("ItemsControl.Refresh")
local stackRows = children(stack)
assert(#stackRows == 3 and stackRows[1] == header, "ItemsControl component preserves authored children")
assert(stackRows[2].DataContext == a and stackRows[3].DataContext == b, "ItemsControl works on an ordinary StackView")
otherSource:clear()
assert(#children(stack) == 1 and children(stack)[1] == header, "clearing source removes only generated views")

core.flushQueue()
collectgarbage("collect")
local beforeTransient = core.objectCount()
local function discardTransient()
  local transient = ui.GridBox { ItemsSource = source }
  transient:send("ItemsControl.Refresh")
  assert(#children(transient) > 0, "unattached control can synchronize explicitly")
end
discardTransient()
core.flushQueue()
collectgarbage("collect")
collectgarbage("collect")
assert(core.objectCount() == beforeTransient, "collection subscription and generated rows must not retain a discarded control")
source:addChild(core.DataObject { Name = "AfterObserverDisposal" })

screen:addChild(stack)
screen:clear()
source:clear()
otherSource:clear()
print("All ItemsControl tests passed.")
