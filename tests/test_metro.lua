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

local function test_pivot_wide_headers()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local pivot = screen + ui.Pivot { Title = "Messenger" }
  for _, name in ipairs { "All", "Unread", "Pinned" } do
    local item = pivot + ui.PivotItem { Name = name, Header = name }
    local _ = item + ui.Button { Height = 60 }
  end
  layout(screen)
  local headers = children(pivot, "PivotHeaderItem")
  for _, header in ipairs(headers) do header.Width = 80 end
  layout(screen)
  headers = children(pivot, "PivotHeaderItem")
  local poolCount, repeatNode = #headers, headers[4]
  test.expect(poolCount > 3 and repeatNode.Visible, "Compact Pivot keeps its repeating phone strip")

  local function visibleHeaders()
    local count = 0
    for _, header in ipairs(children(pivot, "PivotHeaderItem")) do
      if header.Visible then count = count + 1 end
    end
    return count
  end
  screen.Width = 1032
  layout(screen)
  test.expect_eq(visibleHeaders(), 3, "Wide Pivot shows each fitting header only once")
  test.expect_eq(#children(pivot, "PivotHeaderItem"), poolCount, "Wide layout retains its compact copy pool")
  test.expect(not repeatNode.Visible, "Wide layout hides spare repeat nodes")
  headers[3]:send("Node.LeftButtonUp")
  pump(screen)
  layout(screen)
  test.expect_eq(pivot.SelectedIndex, 2, "Wide header taps still select their page")
  test.expect(headers[3].ActualX < headers[1].ActualX and headers[1].ActualX < headers[2].ActualX,
    "Wide headers still follow the selected item in cyclic order")
  test.expect_eq(visibleHeaders(), 3, "Wide selection does not introduce duplicate headers")
  swipe(screen, 200, 400, -120)
  layout(screen)
  test.expect_eq(pivot.SelectedIndex, 0, "Wide content swipes wrap to the first item")
  test.expect_eq(visibleHeaders(), 3, "Wide swipes retain one unique header sequence")

  screen.Width = 600
  layout(screen)
  test.expect(visibleHeaders() > 3, "Compact breakpoint preserves phone-style repetition")
  local expandedPool = #children(pivot, "PivotHeaderItem")
  screen.Width = 601
  layout(screen)
  test.expect_eq(visibleHeaders(), 3, "Above the compact breakpoint fitting headers do not repeat")
  screen.Width = 1032
  for i = 1, 3 do headers[i].Width = 350 end
  layout(screen)
  -- Move the overflowing header strip left to expose its trailing continuation.
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = 700, y = 400 }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDragged", x = 100, y = 400 }
  layout(screen)
  test.expect(visibleHeaders() > 3, "Overflowing wide strips retain a cyclic continuation while dragging")
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = 100, y = 400 }
  pump(screen)
  for i = 1, 3 do headers[i].Width = 80 end
  screen.Width = 390
  layout(screen)
  test.expect(repeatNode.Visible, "Returning to compact layout reuses the original repeat node")
  test.expect_eq(#children(pivot, "PivotHeaderItem"), expandedPool, "Resize and drag do not grow an existing sufficient pool")
  print("PASS: test_pivot_wide_headers")
end

local function test_pivot_preserve_header_case()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local pivot = screen + ui.Pivot {}
  local person = pivot + ui.PivotItem { Header = "Alex McDonald", PreserveHeaderCase = true }
  local settings = pivot + ui.PivotItem { Header = "Settings" }
  layout(screen)
  for _, header in ipairs(children(pivot, "PivotHeaderItem")) do header.Width = 80 end
  layout(screen)
  local headers = children(pivot, "PivotHeaderItem")
  test.expect(#headers > 2, "Fixed-width headers create repeated labels")
  for i, header in ipairs(headers) do
    test.expect_eq(header.Text, i % 2 == 1 and "Alex McDonald" or "settings",
      "Original and repeated headers use their item's casing policy")
  end
  person.PreserveHeaderCase = false
  layout(screen)
  test.expect_eq(headers[1].Text, "alex mcdonald", "Turning preservation off restores the Metro default")
  test.expect_eq(headers[3].Text, "alex mcdonald", "Repeated headers follow casing changes")
  person.PreserveHeaderCase, person.Header = true, "Team iOS"
  layout(screen)
  test.expect_eq(headers[1].Text, "Team iOS", "Changed user titles keep authored capitalization")
  test.expect_eq(headers[3].Text, "Team iOS", "Repeated user titles also retain the updated case")
  test.expect(not settings.PreserveHeaderCase, "Navigation headings remain lowercase by default")
  print("PASS: test_pivot_preserve_header_case")
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
  test.expect(title.ActualX >= 12 + 30, "A round back button precedes the title")

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

local function test_pivot_transitions()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local taps, completed = 0, 0
  local pivot = screen + ui.Pivot { Title = "Messenger", IsLocked = true,
    TransitionCompleted = function () completed = completed + 1 end }
  local item = pivot + ui.PivotItem { Header = "Simon" }
  local row = item + ui.Button { Name = "Row", Height = 60, Click = function () taps = taps + 1 end }
  layout(screen)
  test.expect_eq(pivot.NavigationTransition, "Turnstile", "Pages turn like a turnstile by default")
  test.expect_eq(pivot.TransitionMode, "None", "Pages start in place")
  local function play(mode)
    taps, completed = 0, 0
    pivot.TransitionMode = mode
    layout(screen)
    test.expect_eq(pivot.TransitionMode, mode, mode .. " waits for its first frame")
    -- Pumping runs the animation frames until the transition finishes.
    tap(screen, row, 360)
    test.expect_eq(completed, 1, mode .. " reports its end")
    layout(screen)
  end
  -- A page comes in, taking no input until it settles where it lies.
  for _, mode in ipairs { "ForwardIn", "BackwardIn" } do
    play(mode)
    test.expect_eq(taps, 0, mode .. " takes no input while it plays")
    test.expect_eq(pivot.TransitionMode, "None", mode .. " ends in place")
    tap(screen, row, 360)
    test.expect_eq(taps, 1, mode .. " settles the page in place")
  end
  -- A page goes out and stays away, taking no input, until the mode changes.
  for _, kind in ipairs { "Turnstile", "Slide", "Fade" } do
    pivot.NavigationTransition = kind
    for _, mode in ipairs { "ForwardOut", "BackwardOut" } do
      play(mode)
      pivot:send("Object.Animate")
      pump(screen)
      tap(screen, row, 360)
      test.expect_eq(pivot.TransitionMode, mode, kind .. " " .. mode .. " holds the page away")
      test.expect_eq(taps, 0, kind .. " " .. mode .. " leaves nothing to tap")
      test.expect_eq(completed, 1, kind .. " " .. mode .. " reports its end once")
      pivot.TransitionMode = "None"
      layout(screen)
      tap(screen, row, 360)
      test.expect_eq(taps, 1, "None returns a page held away")
    end
  end
  pivot.TransitionMode, pivot.NavigationTransition = "None", "Turnstile"
  print("PASS: test_pivot_transitions")
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
  test.expect(math.abs((addX + offX + off.ActualWidth) / 2 - 173) < 1, "Icon buttons are centred before the overflow target")
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
  test.expect(math.abs(origin(add) + add.ActualWidth / 2 - 173) < 1, "Hidden buttons leave the row")
  print("PASS: test_application_bar")
end

local function test_application_bar_overflow_target()
  local screen = ui.Screen { Width = 320, Height = 844, ResizeMode = "NoResize" }
  local grid = screen + ui.Grid { Rows = "1fr 58px" }
  grid:addChild(ui.Node2D {})
  local bar = grid + ui.ApplicationBar {}
  local clicked, buttons = nil, {}
  for i = 1, 4 do
    buttons[i] = bar + ui.ApplicationBarIconButton {
      Text = tostring(i), Click = function () clicked = i end,
    }
  end
  bar:addChild(ui.ApplicationBarMenuItem { Text = "About" })
  for _, width in ipairs { 320, 390, 844 } do
    screen.Width = width
    layout(screen)
    for i, button in ipairs(buttons) do
      test.expect(button.ActualWidth >= 44, "Every command keeps a usable target at supported widths")
      test.expect(button.ActualX + button.ActualWidth <= width - 44 + 0.5,
        "Every command target ends before the reserved overflow target")
      if i > 1 then
        test.expect(button.ActualX >= buttons[i - 1].ActualX + buttons[i - 1].ActualWidth - 0.5,
          "Adaptive command targets do not overlap")
      end
    end
    clicked = nil
    tap(screen, bar, width - 22, 29)
    layout(screen)
    test.expect(bar.IsExpanded and clicked == nil, "Tapping overflow expands the bar instead of choosing the last command")
    -- The expanded ellipsis is above the compact bar; route through its actual visual centre.
    local ellipsis = bar:getFirstChild()
    while ellipsis and not (ellipsis.className == "Node2D" and #children(ellipsis, "Node2D") == 3) do
      ellipsis = ellipsis:getNext()
    end
    tap(screen, ellipsis)
    layout(screen)
    test.expect(not bar.IsExpanded and clicked == nil, "Expanded overflow collapses without choosing a command")
    tap(screen, buttons[4], nil, 19)
    layout(screen)
    test.expect_eq(clicked, 4, "The adjacent final command retains its own pointer target")
    test.expect(not bar.IsExpanded, "Choosing a command leaves the bar collapsed")
  end
  print("PASS: test_application_bar_overflow_target")
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

local function test_panorama_wide_sections()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local changes, clicked = 0, 0
  local panorama = screen + ui.Panorama {
    Title = "Contacts", SelectionChanged = function () changes = changes + 1 end,
  }
  local recent = panorama + ui.PanoramaItem { Header = "recent", Orientation = "Horizontal" }
  local content = recent + ui.Grid { Width = 720, Height = 200, Columns = "1fr 1fr" }
  content:addChild(ui.Node2D {})
  local lastPortrait = content + ui.Button { Click = function () clicked = clicked + 1 end }
  local all = panorama + ui.PanoramaItem { Header = "all" }
  all:addChild(ui.Node2D { Height = 200 })
  local groups = panorama + ui.PanoramaItem { Header = "groups", Orientation = "Horizontal" }
  groups:addChild(ui.Node2D { Width = 1010, Height = 200 })
  layout(screen)
  test.expect_eq(recent.ActualWidth, 720, "A horizontal item uses its measured content width")
  test.expect(not all.Visible, "The next section stays beyond the wide portrait matrix")
  swipe(screen, 300, 300, -150)
  layout(screen)
  test.expect_eq(panorama.SelectedIndex, 0, "One swipe reveals the second half of the same section")
  test.expect(recent.ActualX < -300, "The whole wide section moves to reveal more portraits")
  test.expect_eq(math.floor(recent.ActualX + recent.ActualWidth), 390,
    "The second landing aligns the 720-point matrix's trailing edge with the viewport")
  test.expect_eq(changes, 0, "Panning inside a section does not announce a new selection")
  tap(screen, lastPortrait)
  test.expect_eq(clicked, 1, "Portraits in the revealed half remain interactive")
  swipe(screen, 300, 300, -150)
  layout(screen)
  test.expect_eq(panorama.SelectedIndex, 1, "After the wide section, the following section is selected")
  test.expect_eq(math.floor(all.ActualX), 12, "Different section widths settle on the same leading inset")
  swipe(screen, 100, 300, 150)
  layout(screen)
  test.expect_eq(panorama.SelectedIndex, 0, "Swiping back enters the trailing half of a wide section")
  test.expect(recent.ActualX < -300, "Backward navigation preserves the adjacent trailing edge")
  swipe(screen, 100, 300, 150)
  layout(screen)
  test.expect_eq(panorama.SelectedIndex, 0, "Backward panning stays within the wide section")
  test.expect_eq(math.floor(recent.ActualX), 12, "Backward panning reaches the beginning")
  swipe(screen, 100, 300, 150)
  layout(screen)
  test.expect_eq(panorama.SelectedIndex, 2, "Wrapping backwards enters the last section")
  test.expect(groups.ActualX < -600, "Wrapping enters the trailing edge of a differently sized section")
  swipe(screen, 300, 300, -150)
  layout(screen)
  test.expect_eq(panorama.SelectedIndex, 0, "Wrapping forward returns to the first wide section")
  test.expect_eq(math.floor(recent.ActualX), 12, "Forward wrapping resets the internal position")
  test.expect_eq(changes, 4, "Only section changes emit selection events")
  screen.Width = 500
  layout(screen)
  test.expect_eq(recent.ActualWidth, 720, "Resizing remeasures the wide section without shrinking its content")
  print("PASS: test_panorama_wide_sections")
end

local function test_panorama_wide_center()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local changes = 0
  local panorama = screen + ui.Panorama {
    Title = "Photos", SelectionChanged = function () changes = changes + 1 end,
  }
  local photos = panorama + ui.PanoramaItem { Header = "photos", Orientation = "Horizontal" }
  photos:addChild(ui.Node2D { Width = 1010, Height = 200 })
  local albums = panorama + ui.PanoramaItem { Header = "albums" }
  albums:addChild(ui.Node2D { Height = 200 })
  layout(screen)
  swipe(screen, 300, 300, -150)
  layout(screen)
  test.expect_eq(math.floor(photos.ActualX), 12 - 390, "A larger section can rest at its center")
  test.expect_eq(panorama.SelectedIndex, 0, "Center panning retains the same section")
  test.expect_eq(changes, 0, "A center landing emits no selection change")
  swipe(screen, 300, 300, -150)
  layout(screen)
  test.expect_eq(math.floor(photos.ActualX + photos.ActualWidth), 390, "The next landing aligns the right edge")
  swipe(screen, 100, 300, 150)
  layout(screen)
  test.expect(photos.ActualX < 12 and photos.ActualX + photos.ActualWidth > 390,
    "The center is also navigable backwards without selecting a new section")
  test.expect_eq(changes, 0, "Reverse center panning emits no selection change")
  screen.Width = 800
  layout(screen)
  test.expect(photos.ActualX + photos.ActualWidth >= 800, "Resizing clamps the internal offset to the new right edge")
  print("PASS: test_panorama_wide_center")
end

local function test_panorama_programmatic_skip()
  local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
  local panorama = screen + ui.Panorama { Title = "Sections" }
  local items = {}
  for i = 1, 5 do
    items[i] = panorama + ui.PanoramaItem { Header = tostring(i) }
    items[i]:addChild(ui.Node2D { Height = 200 })
  end
  layout(screen)
  panorama.SelectedIndex = 2
  layout(screen)
  test.expect(items[1].Visible, "A nonadjacent forward selection keeps the outgoing section visible")
  test.expect_eq(math.floor(items[1].ActualX), 12, "The forward animation starts from the outgoing position")
  pump(screen)
  layout(screen)
  test.expect_eq(math.floor(items[3].ActualX), 12, "The forward skip settles on the requested section")
  panorama.SelectedIndex = 0
  layout(screen)
  test.expect(items[3].Visible, "A nonadjacent backward selection keeps the outgoing section visible")
  test.expect_eq(math.floor(items[3].ActualX), 12, "The backward animation starts from the outgoing position")
  pump(screen)
  layout(screen)
  test.expect_eq(math.floor(items[1].ActualX), 12, "The backward skip settles on the requested section")
  print("PASS: test_panorama_programmatic_skip")
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

-- As a Windows Phone PivotItem's template margin, the Pivot owns the space
-- between its header and the page: neither page content nor a bar docked
-- under the header can start against the header's edge.
local function test_pivot_content_padding()
  local PADDING = 20
  for _, extend in ipairs { false, true } do
    local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
    local pivot = screen + ui.Pivot { Title = "Messenger", IsLocked = true, ExtendBodyBehindHeader = extend }
    local item = pivot + ui.PivotItem { Header = "page" }
    local body = item + ui.Node2D {}
    local bar = body + ui.Node2D { SafeAreaEdge = "Top" }
    local field = bar + ui.Node2D { Height = 48 }
    local list = body + ui.StackView { Overflow = "hidden scroll", JustifyContent = "Start" }
    local row = list + ui.Node2D { Height = 40 }
    layout(screen)
    local edge
    for _, child in ipairs(children(pivot, "Node2D")) do edge = select(2, origin(child)) + child.ActualHeight end
    local _, barY = origin(bar)
    local _, fieldY = origin(field)
    local mode = extend and "behind the header" or "below the header"
    if extend then
      test.expect(math.abs(barY - edge) < 1, "A docked bar meets the header's edge (" .. mode .. ")")
    end
    test.expect(math.abs(fieldY - (edge + PADDING)) < 1,
      "A docked bar's content starts one page padding below the header (" .. mode .. "): " .. fieldY - edge)
    local _, rowY = origin(row)
    -- Below the header, a bar is ordinary page content that already sits
    -- inside the padding; behind it, the padding continues below the bar.
    local gap = extend and PADDING or 0
    test.expect(math.abs(rowY - (barY + bar.ActualHeight + gap)) < 1,
      "Content below a docked bar keeps the page padding (" .. mode .. "): " .. rowY - barY - bar.ActualHeight)
    bar:removeFromParent()
    layout(screen)
    _, rowY = origin(row)
    test.expect(math.abs(rowY - (edge + PADDING)) < 1,
      "Page content starts one page padding below the header (" .. mode .. "): " .. rowY - edge)
  end
  print("PASS: test_pivot_content_padding")
end

test_pivot()
test_pivot_content_padding()
test_pivot_drag_reuse()
test_pivot_header_metrics()
test_pivot_wide_headers()
test_pivot_preserve_header_case()
test_pivot_back()
test_pivot_transitions()
test_application_bar()
test_application_bar_overflow_target()
test_panorama()
test_panorama_wide_sections()
test_panorama_wide_center()
test_panorama_programmatic_skip()
test_pivot_fill()
