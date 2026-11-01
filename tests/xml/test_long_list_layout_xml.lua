-- Long chat/feed content must retain its extent beyond the old 16-bit size
-- transport limit. Fixed rows make this native layout regression fast.
local orca = require "orca"
orca.init()
local core, fs = require "orca.core", require "orca.filesystem"
local Visibility = require "orca.data.visibility"

for _, axis in ipairs { "Vertical", "Horizontal" } do
    local horizontal = axis == "Horizontal"
    local dimension = horizontal and "Width" or "Height"
    local actual = "Actual" .. dimension
    local scroll = "Scroll" .. dimension
    local markup = ([[
<Screen Width="390" Height="844" ResizeMode="NoResize">
  <StackView Name="Scroller" Direction="%s" Overflow="scroll scroll" JustifyContent="Start">
    <StackView Name="Rows" Direction="%s" JustifyContent="Start">
      <Grid %s="40000"/>
      <Grid %s="40000"/>
      <Grid %s="40000"/>
      <Grid Name="Last" %s="40000"/>
    </StackView>
  </StackView>
</Screen>]]):format(axis, axis, dimension, dimension, dimension, dimension)
    local screen = assert(fs.loadObjectFromXmlString(markup))
    core.flushQueue(); core.advanceFrame(); core.runAllPrograms(screen)
    screen:UpdateLayout(390, 844)
    local scroller, rows, last = screen:findChild("Scroller", true), screen:findChild("Rows", true), screen:findChild("Last", true)
    assert(rows[actual] == 160000, "Long " .. axis .. " content must keep its full arranged size")
    assert(scroller[scroll] == 160000, "The full " .. axis .. " content must be reachable by scrolling")
    assert(last[horizontal and "ActualX" or "ActualY"] == 120000, "Children keep their full positions")
    local context = core.DataObject { Name = "LastRecord" }
    local record = { key = "last" }
    last.DataContext = context
    local scope = { store = { recordFor = function(_, object) if object == context then return record end end } }
    local visibility = Visibility.new { margin = 0 }
    visibility:watch(screen, scope)
    assert(#visibility:records(visibility.pages[1]) == 0, "The final row starts outside the viewport")
    scroller.ContentOffset = { X = horizontal and -(160000 - 390) or 0,
        Y = horizontal and 0 or -(160000 - 844) }
    local found = visibility:records(visibility.pages[1])
    assert(#found == 1 and found[1] == record, "The final " .. axis .. " row remains visible to media after scrolling")
end

print("Long list layout: 160000-point vertical and horizontal content keeps its size, scroll range and visibility")
