local test = require "orca.test"
-- Windows Phone controls: Pivot, Panorama and ApplicationBar driven by
-- simulated pointer input.

local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
local ui   = require "orca.UIKit"
filesystem.init("samples/Metro")

orca.async = function (fn, ...) fn(...) end

local function pump(root)
  while true do
    local msg = orca.system.peekMessage()
    if not msg then return end
    orca.system.dispatchMessage(root, msg)
  end
end

-- Actual positions are parent-relative; pointer events use window space.
local function origin(node)
  local x, y = 0, 0
  while node and node.ActualX do
    x, y = x + node.ActualX, y + node.ActualY
    node = node:getParent()
  end
  return x, y
end

local function tap(screen, node, dx, dy)
  local x, y = origin(node)
  x, y = x + (dx or node.ActualWidth / 2), y + (dy or node.ActualHeight / 2)
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = x, y = y }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = x, y = y }
  pump(screen)
end

local function swipe(screen, x, y, dx)
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = x, y = y }
  for step = 1, 4 do
    orca.system.dispatchMessage { target = screen, message = "LeftButtonDragged", x = x + dx * step / 4, y = y }
  end
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = x + dx, y = y }
  pump(screen)
end

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

local function test_pivot()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local changes = {}
  local pivot = screen + ui.Pivot {
    Title = "Messenger",
    SelectionChanged = function (self, e) changes[#changes + 1] = { e.SelectedIndex, e.OldIndex } end,
  }
  local rows = {}
  for i, header in ipairs { "All", "Unread", "Pinned" } do
    local item = pivot + ui.PivotItem { Name = header, Header = header }
    rows[i] = item + ui.Button { Name = header .. "Row", Height = 60, Click = function () rows.clicked = header end }
  end
  layout(screen)

  local headers = children(pivot, "PivotHeaderItem")
  test.expect_eq(#headers, 3, "Pivot generates one header per PivotItem")
  test.expect_eq(headers[1].Text, "all", "Pivot headers are lowercase")
  test.expect(headers[1].IsSelected and headers[1].Opacity == 1, "The first header starts selected and opaque")
  test.expect(headers[2].Opacity < 1, "Unselected headers are dimmed")
  test.expect(headers[1].ActualX <= headers[2].ActualX, "Headers follow the selected one from left to right")
  local title = pivot:getFirstChild()
  while title and title.className ~= "TextBlock" do title = title:getNext() end
  test.expect_eq(title.Text, "MESSENGER", "Pivot title uses small capitals")
  test.expect(title.ActualY < headers[1].ActualY, "The title sits above the headers")
  test.expect(pivot:findChild("All").Visible and not pivot:findChild("Unread").Visible,
    "Only the selected PivotItem is visible")
  test.expect(pivot:findChild("All").ActualY > headers[1].ActualY + headers[1].ActualHeight - 1,
    "Content starts below the header strip")
  test.expect_eq(pivot:findChild("All").ActualX, 20, "Content keeps the 20-point page margin")

  -- Headless runs have no font faces, so headers measure zero; send the tap directly.
  headers[3]:send("Node.LeftButtonUp")
  pump(screen)
  layout(screen)
  test.expect_eq(pivot.SelectedIndex, 2, "Tapping a header selects its item")
  test.expect(pivot:findChild("Pinned").Visible and not pivot:findChild("All").Visible, "Content follows the header")
  test.expect(changes[1] and changes[1][1] == 2 and changes[1][2] == 0, "SelectionChanged reports new and old indexes")

  local x, y = origin(rows[3])
  swipe(screen, x + 200, y + 20, -120)
  layout(screen)
  test.expect_eq(pivot.SelectedIndex, 0, "Swiping left past the last item wraps to the first")
  test.expect(rows.clicked == nil, "Releasing a swipe over a button does not click it")
  swipe(screen, x + 100, y + 20, 120)
  layout(screen)
  test.expect_eq(pivot.SelectedIndex, 2, "Swiping right from the first item wraps to the last")
  swipe(screen, x + 100, y + 20, 20)
  layout(screen)
  test.expect_eq(pivot.SelectedIndex, 2, "A short drag settles back without changing items")

  tap(screen, rows[3])
  test.expect_eq(rows.clicked, "Pinned", "Taps still reach controls inside the selected item")

  pivot.IsLocked = true
  layout(screen)
  headers = children(pivot, "PivotHeaderItem")
  local shown = 0
  for _, header in ipairs(headers) do shown = shown + (header.Visible and 1 or 0) end
  test.expect_eq(shown, 1, "A locked Pivot shows only the selected header")
  swipe(screen, x + 200, y + 20, -120)
  test.expect_eq(pivot.SelectedIndex, 2, "A locked Pivot ignores swipes")
  print("PASS: test_pivot")
end

local function test_application_bar()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local grid = screen + ui.Grid { Rows = "1fr 58px" }
  local page = grid + ui.Button { Name = "Page", Click = function () end }
  local clicks = {}
  local bar = grid + ui.ApplicationBar { Name = "AppBar" }
  local add = bar + ui.ApplicationBarIconButton { IconUri = "", Text = "New", Click = function () clicks[#clicks + 1] = "new" end }
  local off = bar + ui.ApplicationBarIconButton { Text = "Off", IsEnabled = false, Click = function () clicks[#clicks + 1] = "off" end }
  local about = bar + ui.ApplicationBarMenuItem { Text = "About", Click = function () clicks[#clicks + 1] = "about" end }
  layout(screen)

  test.expect_eq(bar.ActualHeight, 58, "The bar keeps the Windows Phone height")
  test.expect(add.ActualX + add.ActualWidth <= off.ActualX + 0.5, "Icon buttons sit side by side")
  local addX = origin(add)
  local offX = origin(off)
  test.expect(math.abs((addX + offX + off.ActualWidth) / 2 - 195) < 1, "Icon buttons are centred in the bar")
  test.expect(not about.Visible, "Menu items stay hidden while collapsed")

  tap(screen, add)
  test.expect_eq(clicks[1], "new", "Tapping an icon button fires Click")
  tap(screen, off)
  test.expect_eq(#clicks, 1, "Disabled buttons ignore taps")

  tap(screen, bar, 360, 10)
  layout(screen)
  test.expect(bar.IsExpanded, "Tapping the ellipsis expands the bar")
  test.expect(about.Visible, "Expanding reveals the menu")
  local aboutX, aboutY = origin(about)
  test.expect(aboutY < origin(bar) + 1e6 and select(2, origin(about)) > select(2, origin(add)),
    "Menu items are listed below the buttons")
  local _, barY = origin(bar)
  local _, addY = origin(add)
  test.expect(addY < barY, "The expanded bar grows upwards over the page")

  tap(screen, about)
  layout(screen)
  test.expect_eq(clicks[2], "about", "Tapping a menu item fires Click")
  test.expect(not bar.IsExpanded, "Choosing a command collapses the bar")

  bar.IsExpanded = true
  layout(screen)
  tap(screen, page, 100, 100)
  test.expect(not bar.IsExpanded, "Tapping outside collapses the bar")

  bar.Mode = "Minimized"
  layout(screen)
  test.expect(add.Opacity == 0 and add.IgnoreHitTest, "A minimized bar hides its buttons")
  bar.Mode = "Default"
  off.Visible = false
  layout(screen)
  test.expect(math.abs(origin(add) + add.ActualWidth / 2 - 195) < 1, "Hidden buttons leave the row")
  print("PASS: test_application_bar")
end

local function test_panorama()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local changes = 0
  local panorama = screen + ui.Panorama { Title = "People", SelectionChanged = function () changes = changes + 1 end }
  for _, header in ipairs { "Recent", "All", "New" } do
    local item = panorama + ui.PanoramaItem { Name = header, Header = header }
    item:addChild(ui.Node2D { Height = 200 })
  end
  layout(screen)
  local recent, all, new = panorama:findChild("Recent"), panorama:findChild("All"), panorama:findChild("New")
  test.expect(recent.ActualX < all.ActualX and all.ActualX < 390, "The next section peeks in from the right")
  test.expect(all.ActualX - recent.ActualX > 300, "Sections are almost a full screen wide")
  swipe(screen, 300, 400, -150)
  layout(screen)
  test.expect_eq(panorama.SelectedIndex, 1, "Swiping left moves to the next section")
  swipe(screen, 300, 400, -150)
  swipe(screen, 300, 400, -150)
  test.expect_eq(panorama.SelectedIndex, 0, "The panorama wraps from the last section to the first")
  test.expect_eq(changes, 3, "Each section change fires SelectionChanged")
  print("PASS: test_panorama")
end

local function test_xml()
  local screen = filesystem.loadObject("Metro/Screens/Pivot")
  layout(screen)
  local pivot = screen:findChild("Pivot", true)
  test.expect(pivot and #children(pivot, "PivotHeaderItem") == 3, "XML declares Pivot and PivotItem")
  local bar = screen:findChild("AppBar", true)
  test.expect(bar and #children(bar, "ApplicationBarIconButton") == 3, "XML declares the ApplicationBar")
  print("PASS: test_xml")
end

test_pivot()
test_application_bar()
test_panorama()
test_xml()
