-- Data-backed snapshots update on record/context changes, not every Paint.
local orca = require "orca"
orca.init()
local core, fs, ui = require "orca.core", require "orca.filesystem", require "orca.UIKit"
local schema = [[<Schema><Entity Name="OneTimeRecord">
  <Column Name="Title" Type="string"/><Column Name="Count" Type="int"/>
</Entity></Schema>]]
local prototype = assert(fs.loadXmlDataStringWithSchema('<OneTimeRecord Title="seed" Count="1"/>', schema))
local function record(title)
    local value = prototype:instantiate()
    value.Title = title
    return value
end
local function pass(root)
    core.flushQueue(); core.advanceFrame(); core.runAllPrograms(root); core.flushQueue()
end
local root = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
local left, right = ui.StackView {}, ui.StackView {}
root:addChild(left); root:addChild(right)
local first, second = record("first"), record("second")
left.DataContext, right.DataContext = first, second
local label = ui.TextBlock {}
left:addChild(label)
assert(label:attachPropertyProgram("TextRun.Text", "{DataContext/Title}", "OneTime", true))
pass(root)
assert(label.Text == "first", "OneTime resolves inherited DataContext")
label.Text = "sentinel"
for _ = 1, 3 do pass(root) end
assert(label.Text == "sentinel", "Unchanged frames do not execute a OneTime program")
first.Title = "changed"
pass(root)
assert(label.Text == "changed", "DataObject property changes refresh the snapshot")
right:addChild(label)
pass(root)
assert(label.Text == "second", "Reparenting replaces the inherited record")
left.DataContext = second
left:addChild(label)
pass(root)
assert(label.Text == "second", "Reparenting to a matching record remains valid")
left.DataContext = first
pass(root)
assert(label.Text == "changed", "Replacing DataContext invalidates resolved paths")
first = nil
left.DataContext = nil
for _ = 1, 3 do collectgarbage("collect"); pass(root) end
local replacement = record("replacement")
left.DataContext = replacement
pass(root)
assert(label.Text == "replacement", "Destroyed context can be rebound safely")

local xml = assert(fs.loadObjectFromXmlString([[
<StackView><TextBlock Name="Snapshot" Text="{Binding DataContext/Title, Mode=OneTime}"/>
<TextBlock Name="Live" Text="{Binding ../Width}"/></StackView>]]))
root:addChild(xml); xml.DataContext = replacement
xml.Width = 100
pass(root)
assert(xml:findChild("Snapshot", true).Text == "replacement", "XML OneTime shorthand loads")
local live = xml:findChild("Live", true)
local before = live.Text
xml.Width = 200
pass(root)
assert(live.Text ~= before, "Default OneWay geometry remains live")
local serialized = fs.serializeObjectToString(xml)
assert(serialized:find('Mode="OneTime"', 1, true), "Serialization preserves OneTime")

local records = core.DataObject {}
local a, b, c = record("A"), record("B"), record("C")
records:addChild(a); records:addChild(b)
local template = ui.TextBlock {}
assert(template:attachPropertyProgram("TextRun.Text", "{DataContext/Title}", "OneTime", true))
local list = ui.ListBox { ItemsSource = records, ItemTemplate = template }
root:addChild(list)
local function rows()
    pass(root)
    local result = {}
    for child in list.children do result[#result + 1] = child end
    return result
end
local original = rows()
assert(#original == 2 and original[1].Text == "A" and original[2].Text == "B")
records:addChild(c)
local appended = rows()
assert(#appended == 3 and appended[1] == original[1] and appended[3].Text == "C",
    "Adding a record initializes the new snapshot and retains existing rows")
c:moveBefore(a)
local moved = rows()
assert(moved[1] == appended[3] and moved[2] == original[1] and moved[3] == original[2],
    "Reordering retains snapshot views and the correct contexts")
a.Title = "edited A"
assert(rows()[2].Text == "edited A", "A moved row still receives record changes")
b:removeFromParent()
local remaining = rows()
assert(#remaining == 2 and remaining[1] == appended[3] and remaining[2] == original[1],
    "Removal retains surviving snapshot rows")
records:clear()
assert(#rows() == 0, "Clearing a collection removes snapshot rows")
print("PASS: OneTime snapshots, data mutation, context replacement, reparenting, destruction, XML and live geometry")
