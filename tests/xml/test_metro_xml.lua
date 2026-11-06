local test = require "orca.test"
-- Windows Phone controls declared in XML; tests/test_metro.lua covers their
-- behavior independently of scene loading.

local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
require "orca.UIKit"
filesystem.init("apps/Metro")

local function children(node, class)
  local found, child = {}, node:getFirstChild()
  while child do
    if child.className == class then found[#found + 1] = child end
    child = child:getNext()
  end
  return found
end

local function layout(screen)
  screen:UpdateLayout(screen.Width, screen.Height)
  screen:UpdateLayout(screen.Width, screen.Height)
end

local function test_xml()
  local screen = filesystem.loadObject("tests/fixtures/metro/Pivot.xml")
  layout(screen)
  local pivot = screen:findChild("Pivot", true)
  test.expect(pivot and #children(pivot, "PivotHeaderItem") == 3, "XML declares Pivot and PivotItem")
  local bar = screen:findChild("AppBar", true)
  test.expect(bar and #children(bar, "ApplicationBarIconButton") == 3, "XML declares the ApplicationBar")
  print("PASS: test_xml")
end

test_xml()
