-- XML-declared retained lists and data-driven style classes.
local core = require "orca.core"
local fs = require "orca.filesystem"
local ui = require "orca.UIKit"

local schema = [[<Schema><Entity Name="StackRow"><Column Name="Title" Type="string"/>
  <Column Name="State" Type="string"/></Entity></Schema>]]
local root = assert(fs.loadXmlDataStringWithSchema('<Rows><StackRow name="Proto"/></Rows>', schema))
local proto = root:findChild("Proto")
local source = core.DataObject { Name = "Items" }
local function row(title, state)
  local record = proto:instantiate()
  record.Title, record.State = title, state
  source:addChild(record)
  return record
end
local first, second = row("One", "unread"), row("Two", "")

local screen = assert(fs.loadObjectFromXmlString([[
<Screen Width="320" Height="240" ResizeMode="NoResize">
  <ItemsStack Name="List" Overflow="hidden scroll" ItemsSource="{Binding DataContext/Items}">
    <TextBlock Name="Authored" Text="header"/>
  </ItemsStack>
</Screen>]]))
local list = screen:findChild("List", true)
list.ItemTemplate = assert(fs.loadObjectFromXmlString([[
<StackView Name="Row" class="row" StateClasses="{Binding DataContext/State}">
  <TextBlock Name="Label" class="label" Text="{Binding DataContext/Title}"/>
</StackView>]]))
local holder = core.DataObject {}
holder:addChild(source)
list.DataContext = holder

local function settle()
  core.flushQueue(); core.advanceFrame(); core.runAllPrograms(screen); core.flushQueue()
end
local function rows()
  local result = {}
  for child in list.children do if child.Name == "Row" then result[#result + 1] = child end end
  return result
end
settle()
local views = rows()
assert(#views == 2 and views[1].DataContext == first and views[2].DataContext == second,
  "ItemsStack binds ItemsSource from XML and creates one retained row per record")
assert(list:findChild("Authored") ~= nil, "authored children are preserved")
assert(views[1]:findChild("Label").Text == "One", "row bindings read their record")
first:moveBefore(nil)
settle()
local moved = rows()
assert(moved[1] == views[2] and moved[2] == views[1], "reordering keeps native row identity")
assert(list.SelectedItem == nil, "ItemsStack has no implicit selection")

-- StateClasses replaces only the classes it added.
local view = views[1]
assert(view.StateClasses == "unread", "StateClasses follows the record")
local saved = fs.serializeObjectToString(view)
assert(saved:find('class="row"', 1, true) and saved:find("StyleController.StateClasses", 1, true),
  "authored classes serialize; bound state classes do not")
screen.StyleSheet = ui.loadObjectFromCssString ".row .label { opacity: 0.9; } .unread .label { opacity: 0.5; } .muted .label { opacity: 0.25; }"
screen:send("StyleController.ThemeChanged", { recursive = true })
first.State = "unread"
settle()
local label = view:findChild("Label")
assert(math.abs(label.Opacity - 0.5) < 0.01, "descendant selectors follow a bound state class")
first.State = "pinned muted"
settle()
assert(view.StateClasses == "pinned muted", "a new record value replaces the bound classes")
assert(math.abs(label.Opacity - 0.25) < 0.01, "the old state class no longer applies")
first.State = ""
settle()
assert(view.StateClasses == "", "clearing the field removes bound classes")
assert(math.abs(label.Opacity - 0.9) < 0.01, "authored rules apply once state classes are gone")

print("PASS: ItemsStack XML lists and StateClasses")
