local test = require "orca.test"
-- Windows Phone controls: Pivot, Panorama and ApplicationBar driven by
-- simulated pointer input.

local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
local ui   = require "orca.UIKit"
filesystem.init("apps/Metro")

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
  test.expect(#headers >= 3, "Pivot generates one header per PivotItem")
  test.expect_eq(#headers, 3, "Without font faces headers measure zero and are not repeated")
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
  test.expect_eq(pivot:findChild("All").ActualX, 12, "Content keeps the 12-point page margin")

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
  test.expect_eq(#headers, 3, "A locked Pivot does not repeat headers")
  swipe(screen, x + 200, y + 20, -120)
  test.expect_eq(pivot.SelectedIndex, 2, "A locked Pivot ignores swipes")
  print("PASS: test_pivot")
end

-- Fixed header widths make allocation behavior deterministic without font faces.
local function test_pivot_drag_reuse()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local pivot = screen + ui.Pivot { Title = "Messenger" }
  for _, name in ipairs { "All", "Unread" } do
    local _ = pivot + ui.PivotItem { Name = name, Header = name }
  end
  layout(screen)
  for _, header in ipairs(children(pivot, "PivotHeaderItem")) do header.Width = 80 end
  layout(screen)
  local headers = children(pivot, "PivotHeaderItem")
  test.expect_eq(#headers, 4, "Fixed-width headers fill the strip with two copies")
  local spare = headers[4]
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = 40, y = 400 }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDragged", x = 340, y = 400 }
  layout(screen)
  test.expect_eq(#children(pivot, "PivotHeaderItem"), 4, "Dragging keeps spare header copies")
  test.expect(not spare.Visible, "The spare copy is hidden beyond the strip edge")
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDragged", x = 60, y = 400 }
  layout(screen)
  test.expect(spare.Visible, "Dragging back reuses the same header copy")
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = 60, y = 400 }
  layout(screen)
  test.expect_eq(pivot.SelectedIndex, 0, "A canceled drag retains the selected item")
  test.expect_eq(pivot:findChild("All").Opacity, 1, "Settling back keeps the existing page opaque")
  pump(screen)
  print("PASS: test_pivot_drag_reuse")
end

local function test_pivot_header_metrics()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local pivot = screen + ui.Pivot {}
  for _, name in ipairs { "All", "Unread" } do
    local _ = pivot + ui.PivotItem { Header = name }
  end
  layout(screen)
  local headers = children(pivot, "PivotHeaderItem")
  headers[1].Width, headers[2].Width = 40, 80
  layout(screen)
  headers = children(pivot, "PivotHeaderItem")
  test.expect_eq(#headers, 6, "Unequal header widths repeat until the strip is filled")
  for i, x in ipairs { 12, 70, 168, 226, 324, 382 } do
    test.expect_eq(headers[i].ActualX, x, "Repeated headers use the source width and spacing")
    test.expect_eq(headers[i].Text, i % 2 == 1 and "all" or "unread", "Copies keep their source text")
  end
  local spare = headers[5]
  headers[1].Width = 100
  layout(screen)
  test.expect_eq(headers[3].ActualX, 228, "Changing a source width updates repeated positions")
  test.expect_eq(headers[3].ActualWidth, 100, "Copies use fresh source metrics")
  test.expect(not spare.Visible, "A wider strip hides spare copies")
  screen.Width = 250
  layout(screen)
  test.expect(headers[3].Visible and not headers[4].Visible, "Resizing updates the visible repeat count")
  screen.Width = 390
  headers[1].Width = 40
  layout(screen)
  test.expect(spare.Visible, "Restoring the width reuses the spare copy")
  test.expect_eq(#children(pivot, "PivotHeaderItem"), 6, "Metric changes do not grow the copy pool")
  print("PASS: test_pivot_header_metrics")
end

local function test_pivot_back()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local events = {}
  local pivot = screen + ui.Pivot {
    Title = "Messenger", IsLocked = true, BackIcon = "Metro/Icons/plus.svg",
    BackRequested = function () events[#events + 1] = "back" end,
    HeaderClick = function () events[#events + 1] = "header" end,
  }
  local item = pivot + ui.PivotItem { Name = "Thread", Header = "Simon" }
  local row = item + ui.Button { Name = "Row", Height = 60, Click = function () events[#events + 1] = "row" end }
  layout(screen)
  local title = pivot:getFirstChild()
  while title and title.className ~= "TextBlock" do title = title:getNext() end
  test.expect(title.ActualX > 12, "A back glyph precedes the title")

  tap(screen, title, 4, 4)
  test.expect_eq(events[1], "back", "Tapping the title line goes back")
  local _, y = origin(row)
  swipe(screen, 8, y + 20, 140)
  layout(screen)
  test.expect_eq(events[2], "back", "Swiping in from the left edge goes back")
  test.expect_eq(#events, 2, "Releasing a back swipe over a button does not click it")
  swipe(screen, 8, y + 20, 40)
  swipe(screen, 200, y + 20, 140)
  swipe(screen, 8, y + 20, -140)
  local function count(name)
    local n = 0
    for _, event in ipairs(events) do n = n + (event == name and 1 or 0) end
    return n
  end
  test.expect_eq(count("back"), 2, "Short, mid-screen and leftward swipes do not go back")
  events = {}
  tap(screen, row)
  test.expect_eq(events[1], "row", "Taps still reach the content")
  children(pivot, "PivotHeaderItem")[1]:send("Node.LeftButtonUp")
  pump(screen)
  test.expect_eq(events[2], "header", "Tapping the selected header reports HeaderClick")

  pivot.BackIcon = ""
  layout(screen)
  tap(screen, title, 4, 4)
  swipe(screen, 8, y + 20, 140)
  test.expect_eq(count("back"), 0, "Without a BackIcon the title and edge stay inert")
  test.expect_eq(title.ActualX, 12, "Without a BackIcon the title keeps the page margin")
  print("PASS: test_pivot_back")
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

  -- The ellipsis is the generated part holding three dots.
  local ellipsis = bar:getFirstChild()
  while ellipsis and not (ellipsis.className == "Node2D" and #children(ellipsis, "Node2D") == 3) do
    ellipsis = ellipsis:getNext()
  end
  local function middle(node) local _, y = origin(node); return y + node.ActualHeight / 2 end
  local ring = 38 / 2
  test.expect(ellipsis and math.abs(middle(ellipsis) - (select(2, origin(add)) + ring)) < 0.5,
    "The ellipsis is vertically centred on the round buttons")

  tap(screen, bar, 360, 10)
  layout(screen)
  test.expect(bar.IsExpanded, "Tapping the ellipsis expands the bar")
  test.expect(math.abs(middle(ellipsis) - (select(2, origin(add)) + ring)) < 0.5,
    "The ellipsis stays centred on the buttons when expanded")
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

local function test_lisp()
  local screen = filesystem.loadObject("tests/fixtures/metro/Pivot.lisp")
  layout(screen)
  local pivot = screen:findChild("Pivot", true)
  test.expect(pivot and #children(pivot, "PivotHeaderItem") == 3, "Lisp declares Pivot and PivotItem")
  local bar = screen:findChild("AppBar", true)
  test.expect(bar and #children(bar, "ApplicationBarIconButton") == 3, "Lisp declares the ApplicationBar")
  print("PASS: test_lisp")
end

-- Loads fonts so headers have real widths; runs last to leave the others unchanged.
local function test_pivot_fill()
  require("orca.core.startup").load_plugins()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local pivot = screen + ui.Pivot { Title = "Octopus" }
  for _, header in ipairs { "Me", "You" } do local _ = pivot + ui.PivotItem { Name = header, Header = header } end
  layout(screen)
  local headers = children(pivot, "PivotHeaderItem")
  if headers[1].ActualWidth == 0 then
    test.expect_eq(#headers, 2, "Headers without font metrics are not repeated")
    print("SKIP: test_pivot_fill (no font metrics)")
    return
  end
  test.expect(#headers > 2, "A short header strip repeats its headers")
  local right = 0
  for i, header in ipairs(headers) do
    test.expect_eq(header.Text, i % 2 == 1 and "me" or "you", "Copies continue the cycle in order")
    test.expect(header.Visible and (i == 1 or header.Opacity < 1), "Only the leading header is opaque")
    test.expect(i == 1 or header.ActualX > headers[i - 1].ActualX, "Copies run left to right")
    right = math.max(right, header.ActualX + header.ActualWidth)
  end
  test.expect(right + 18 >= 390, "The header strip reaches the right edge")
  test.expect(headers[#headers].ActualX < 390, "No copy starts beyond the right edge")

  local count = #headers
  layout(screen)
  test.expect_eq(#children(pivot, "PivotHeaderItem"), count, "Layout reuses the copies")
  headers[4]:send("Node.LeftButtonUp")
  pump(screen)
  layout(screen)
  test.expect_eq(pivot.SelectedIndex, 1, "Tapping a copy selects the item it repeats")
  test.expect_eq(children(pivot, "PivotHeaderItem")[2].Opacity, 1, "The tapped item's header leads")

  pivot.IsLocked = true
  layout(screen)
  test.expect_eq(#children(pivot, "PivotHeaderItem"), 2, "A locked Pivot drops the copies")
  print("PASS: test_pivot_fill")
end

test_pivot()
test_pivot_drag_reuse()
test_pivot_header_metrics()
test_pivot_back()
test_application_bar()
test_panorama()
test_lisp()
test_pivot_fill()
