local test = require "orca.test"
-- Integration tests for Tab/TabBar/TabView via simulated mouse clicks.
--
-- Requires a display (run via: xvfb-run build/bin/orca -test=tests/test_tab_interaction.lua)
-- or directly when a display is available.

local orca = require "orca"
local core = require "orca.core"
local ui   = require "orca.UIKit"

-- Override orca.async so that Lua event callbacks execute synchronously in tests.
orca.async = function (fn, ...) fn(...) end

local function pump_messages(root)
  while true do
    local msg = orca.system.peekMessage()
    if not msg then return end
    orca.system.dispatchMessage(root, msg)
  end
end

local screen = ui.Screen { Width = 1000, Height = 800, ResizeMode = "NoResize" }

-- ---------------------------------------------------------------------------
-- TabBar: click events select tabs and fire SelectionChanged
-- ---------------------------------------------------------------------------
local function test_tabbar_click_interaction()
  local selected_value = nil
  local old_value      = nil

  local bar = screen + ui.TabBar {
    Width = 400,
    Height = 40,
    SelectionChanged = function (self, e)
      selected_value = self.SelectedValue
      if e then old_value = e.OldValue end
    end,
  }
  local tab_a = bar + ui.Tab { Value = "alpha", Width = 100, Height = 40 }
  local tab_b = bar + ui.Tab { Value = "beta",  Width = 100, Height = 40 }

  screen:UpdateLayout(screen.Width, screen.Height)

  -- Click tab_a using its computed screen position
  local ax = tab_a.ActualX + 5
  local ay = tab_a.ActualY + 5
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = ax, y = ay }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp",   x = ax, y = ay }
  pump_messages(screen)

  test.expect_eq(bar.SelectedValue, "alpha", "TabBar.SelectedValue should be 'alpha' after click")
  test.expect(tab_a.IsSelected,              "Tab alpha should be IsSelected after click")
  test.expect(not tab_b.IsSelected,          "Tab beta should NOT be IsSelected after sibling click")
  test.expect_eq(selected_value, "alpha",    "SelectionChanged should report SelectedValue = 'alpha'")
  test.expect(old_value == nil,              "OldValue should be nil for the first selection")

  -- Click tab_b
  local bx = tab_b.ActualX + 5
  local by = tab_b.ActualY + 5
  selected_value = nil
  old_value      = nil
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = bx, y = by }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp",   x = bx, y = by }
  pump_messages(screen)

  test.expect_eq(bar.SelectedValue, "beta",  "TabBar.SelectedValue should update to 'beta'")
  test.expect(not tab_a.IsSelected,          "Tab alpha should be deselected")
  test.expect(tab_b.IsSelected,              "Tab beta should be IsSelected after click")
  test.expect_eq(selected_value, "beta",     "SelectionChanged should report SelectedValue = 'beta'")
  test.expect_eq(old_value, "alpha",         "SelectionChanged should report OldValue = 'alpha'")

  bar:removeFromParent()
  print("PASS: test_tabbar_click_interaction")
end

-- ---------------------------------------------------------------------------
-- TabView: clicking tabs switches content panel visibility
-- ---------------------------------------------------------------------------
local function test_tabview_click_interaction()
  local tab_sel  = nil
  local tab_old  = nil

  local view = screen + ui.TabView {
    Width = 1000,
    Height = 700,
    SelectionChanged = function (self, e)
      tab_sel = self.SelectedValue
      if e then tab_old = e.OldValue end
    end,
  }

  local bar   = view + ui.TabBar { Width = 1000, Height = 40 }
  local tab_a = bar + ui.Tab { Value = "alpha", Width = 200, Height = 40 }
  local tab_b = bar + ui.Tab { Value = "beta",  Width = 200, Height = 40 }
  local tab_c = bar + ui.Tab { Value = "gamma", Width = 200, Height = 40 }

  local panel_a = view + ui.Node2D { Name = "alpha", Width = 1000, Height = 660 }
  local panel_b = view + ui.Node2D { Name = "beta",  Width = 1000, Height = 660 }
  local panel_c = view + ui.Node2D { Name = "gamma", Width = 1000, Height = 660 }

  -- ViewDidLoad auto-selects first tab
  view:send("Node.ViewDidLoad")

  screen:UpdateLayout(screen.Width, screen.Height)

  -- Verify initial state
  test.expect_eq(view.SelectedValue, "alpha", "Initial SelectedValue should be 'alpha'")
  test.expect(panel_a.Visible,       "Panel 'alpha' should be visible initially")
  test.expect(not panel_b.Visible,   "Panel 'beta' should be hidden initially")
  test.expect(not panel_c.Visible,   "Panel 'gamma' should be hidden initially")

  -- Click tab_b
  local bx = tab_b.ActualX + 5
  local by = tab_b.ActualY + 5
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = bx, y = by }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp",   x = bx, y = by }
  pump_messages(screen)

  test.expect_eq(view.SelectedValue, "beta",  "SelectedValue should be 'beta' after click")
  test.expect(not panel_a.Visible,   "Panel 'alpha' should be hidden after switching to beta")
  test.expect(panel_b.Visible,       "Panel 'beta' should be visible after switching to beta")
  test.expect(not panel_c.Visible,   "Panel 'gamma' should still be hidden")
  test.expect_eq(tab_sel, "beta",    "TabView SelectionChanged should report SelectedValue = 'beta'")
  test.expect_eq(tab_old, "alpha",   "TabView SelectionChanged should report OldValue = 'alpha'")

  -- Click tab_c
  local cx = tab_c.ActualX + 5
  local cy = tab_c.ActualY + 5
  tab_sel = nil
  tab_old = nil
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = cx, y = cy }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp",   x = cx, y = cy }
  pump_messages(screen)

  test.expect_eq(view.SelectedValue, "gamma",  "SelectedValue should be 'gamma' after click")
  test.expect(not panel_a.Visible,   "Panel 'alpha' should still be hidden")
  test.expect(not panel_b.Visible,   "Panel 'beta' should be hidden after switching to gamma")
  test.expect(panel_c.Visible,       "Panel 'gamma' should be visible")
  test.expect_eq(tab_sel, "gamma",   "TabView SelectionChanged: SelectedValue = 'gamma'")
  test.expect_eq(tab_old, "beta",    "TabView SelectionChanged: OldValue = 'beta'")

  -- Click the already-selected tab; SelectionChanged should NOT fire
  tab_sel = nil
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = cx, y = cy }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp",   x = cx, y = cy }
  pump_messages(screen)
  test.expect(tab_sel == nil, "TabView SelectionChanged should NOT fire when re-clicking the selected tab")

  view:removeFromParent()
  print("PASS: test_tabview_click_interaction")
end

-- ---------------------------------------------------------------------------
-- TabView: scrolling a parent container must not break later tab clicks,
-- and the active panel should still switch after the scroll offset changes.
-- ---------------------------------------------------------------------------
local function test_tabview_scroll_then_click()
  local view = nil
  local tab_a = nil
  local tab_b = nil
  local clicked = nil

  local scroll = screen + ui.StackView {
    Width = 1000,
    Height = 320,
    Direction = "Vertical",
    OverflowY = "Scroll",
    OverflowX = "Hidden",
  }

  local spacer = scroll + ui.Node2D { Height = 520 }

  view = scroll + ui.TabView {
    Width = 1000,
    Height = 280,
    SelectionChanged = function (self)
      clicked = self.SelectedValue
    end,
  }

  local bar   = view + ui.TabBar { Width = 1000, Height = 40 }
  tab_a = bar + ui.Tab { Value = "alpha", Width = 200, Height = 40 }
  tab_b = bar + ui.Tab { Value = "beta",  Width = 200, Height = 40 }

  local panel_a = view + ui.Node2D { Name = "alpha", Width = 1000, Height = 240 }
  local panel_b = view + ui.Node2D { Name = "beta",  Width = 1000, Height = 240 }

  view:send("Node.ViewDidLoad")
  screen:UpdateLayout(screen.Width, screen.Height)

  test.expect(panel_a.Visible,      "Panel 'alpha' should be visible before scrolling")
  test.expect(not panel_b.Visible, "Panel 'beta' should be hidden before scrolling")

  -- Scroll the parent using the same wheel path the app uses.
  scroll:send("Node.ScrollWheel", { x = 0, y = 0, deltaX = 0, deltaY = -spacer.Height })
  screen:UpdateLayout(screen.Width, screen.Height)

  test.expect(scroll.ContentOffset.Y < 0, "Scroll container should have a negative ContentOffset after scrolling")

  local click_x = view.ActualX + tab_b.ActualX + 5
  local click_y = scroll.ContentOffset.Y + view.ActualY + tab_b.ActualY + 5
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = click_x, y = click_y }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp",   x = click_x, y = click_y }
  pump_messages(screen)

  test.expect_eq(view.SelectedValue, "beta", "TabView should still react to clicks after scrolling")
  test.expect(not panel_a.Visible, "Panel 'alpha' should be hidden after clicking beta")
  test.expect(panel_b.Visible,     "Panel 'beta' should be visible after clicking beta")

  spacer:removeFromParent()
  scroll:removeFromParent()
  print("PASS: test_tabview_scroll_then_click")
end

-- ---------------------------------------------------------------------------
-- Overflow clipping: a vertical scroll container clips Y but leaves X visible.
-- ---------------------------------------------------------------------------
local function test_vertical_overflow_keeps_horizontal_hit_visible()
  local horizontal_hit = false
  local vertical_hit = false

  local scroll = screen + ui.Node2D {
    Width = 100,
    Height = 100,
    OverflowY = "Scroll",
  }

  local horizontal_child = scroll + ui.Node2D {
    Width = 40,
    Height = 40,
    MarginLeft = 140,
    MarginTop = 20,
    LeftButtonUp = function()
      horizontal_hit = true
      return true
    end,
  }

  local vertical_child = scroll + ui.Node2D {
    Width = 40,
    Height = 40,
    MarginLeft = 20,
    MarginTop = 140,
    LeftButtonUp = function()
      vertical_hit = true
      return true
    end,
  }

  screen:UpdateLayout(screen.Width, screen.Height)

  orca.system.dispatchMessage {
    target = screen,
    message = "LeftButtonUp",
    x = 145,
    y = 25,
  }
  test.expect(horizontal_hit, "overflow-y scroll should not clip horizontally visible child hit testing")

  orca.system.dispatchMessage {
    target = screen,
    message = "LeftButtonUp",
    x = 25,
    y = 145,
  }
  test.expect(not vertical_hit, "overflow-y scroll should clip vertically overflowing child hit testing")

  scroll:removeFromParent()
  print("PASS: test_vertical_overflow_keeps_horizontal_hit_visible")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_tabbar_click_interaction()
test_tabview_click_interaction()
test_tabview_scroll_then_click()
test_vertical_overflow_keeps_horizontal_hit_visible()

print("All Tab interaction tests passed.")
