local test = require "orca.test"
-- Windows Phone controls declared in Lisp; tests/test_metro.lua covers their
-- behavior in both scene formats.

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

local function test_lisp()
  local screen = filesystem.loadObject("tests/fixtures/metro/Pivot.lisp")
  layout(screen)
  local pivot = screen:findChild("Pivot", true)
  test.expect(pivot and #children(pivot, "PivotHeaderItem") == 3, "Lisp declares Pivot and PivotItem")
  local bar = screen:findChild("AppBar", true)
  test.expect(bar and #children(bar, "ApplicationBarIconButton") == 3, "Lisp declares the ApplicationBar")
  print("PASS: test_lisp")
end

test_lisp()
