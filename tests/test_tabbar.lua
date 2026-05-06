local test = require "orca.test"
-- Unit tests for Tab, TabBar, and TabView components.
--
-- Covers:
--   TabBar: mutual-exclusion selection, SelectedValue update, SelectionChanged event,
--           OldValue propagation, no-op re-selection, IsSelected flag, Tab.selected flag.
--   TabView: initial auto-selection on ViewDidLoad, content panel visibility switching,
--            TabView.SelectionChanged event, SelectedValue propagation.
--   Button/RadioButton: new visual properties have expected defaults.
--
-- All tests are headless (no display required).
-- Run with: $(TARGET) -test=tests/test_tabbar.lua

local orca = require "orca"
local core = require "orca.core"
local ui   = require "orca.UIKit"

-- Override orca.async so that Lua event callbacks (fired via CORE_HandleObjectMessage)
-- execute synchronously in tests instead of being deferred to the event loop.
orca.async = function (fn, ...) fn(...) end

-- ---------------------------------------------------------------------------
-- TabBar: selecting a Tab updates SelectedValue and mutual exclusion
-- ---------------------------------------------------------------------------
local function test_tabbar_selection()
  local screen = ui.Screen { Width = 600, Height = 400, ResizeMode = "NoResize" }

  local bar = screen + ui.TabBar { Width = 300, Height = 40 }
  local tab_a = bar + ui.Tab { Value = "alpha", Width = 100, Height = 40 }
  local tab_b = bar + ui.Tab { Value = "beta",  Width = 100, Height = 40 }

  -- No tab is selected yet (nothing has been clicked or initialised via ViewDidLoad)
  test.expect(not tab_a.IsSelected, "Tab alpha should NOT be selected before any interaction")
  test.expect(not tab_b.IsSelected, "Tab beta should NOT be selected before any interaction")

  -- Programmatically trigger selection by sending a LeftButtonUp to tab_a
  tab_a:send("Node.LeftButtonUp")

  test.expect(tab_a.IsSelected,    "Tab alpha should be IsSelected after LeftButtonUp")
  test.expect(not tab_b.IsSelected, "Tab beta should NOT be IsSelected after sibling is selected")
  test.expect_eq(bar.SelectedValue, "alpha", "TabBar.SelectedValue should be 'alpha'")

  -- Select the second tab
  tab_b:send("Node.LeftButtonUp")

  test.expect(not tab_a.IsSelected, "Tab alpha should be deselected after sibling selected")
  test.expect(tab_b.IsSelected,     "Tab beta should be IsSelected after LeftButtonUp")
  test.expect_eq(bar.SelectedValue, "beta", "TabBar.SelectedValue should update to 'beta'")

  screen:clear()
  print("PASS: test_tabbar_selection")
end

-- ---------------------------------------------------------------------------
-- TabBar: SelectionChanged event fires with SelectedValue and OldValue
-- ---------------------------------------------------------------------------
local function test_tabbar_selection_changed_event()
  local screen = ui.Screen { Width = 600, Height = 400, ResizeMode = "NoResize" }

  local selected_value = nil
  local old_value      = nil

  local bar = screen + ui.TabBar {
    Width = 300,
    Height = 40,
    SelectionChanged = function (self, e)
      selected_value = self.SelectedValue
      if e then old_value = e.OldValue end
    end,
  }
  local tab_a = bar + ui.Tab { Value = "alpha", Width = 100, Height = 40 }
  local tab_b = bar + ui.Tab { Value = "beta",  Width = 100, Height = 40 }

  -- Select alpha first; OldValue should be nil
  tab_a:send("Node.LeftButtonUp")
  test.expect_eq(selected_value, "alpha", "SelectionChanged should report SelectedValue = 'alpha'")
  test.expect(old_value == nil, "OldValue should be nil for the first selection")

  -- Switch to beta; OldValue should be 'alpha'
  selected_value = nil
  old_value      = nil
  tab_b:send("Node.LeftButtonUp")
  test.expect_eq(selected_value, "beta",  "SelectionChanged should report SelectedValue = 'beta'")
  test.expect_eq(old_value,      "alpha", "SelectionChanged should report OldValue = 'alpha'")

  screen:clear()
  print("PASS: test_tabbar_selection_changed_event")
end

-- ---------------------------------------------------------------------------
-- TabBar: clicking the already-selected tab does NOT fire SelectionChanged again
-- ---------------------------------------------------------------------------
local function test_tabbar_no_reselect()
  local screen = ui.Screen { Width = 600, Height = 400, ResizeMode = "NoResize" }

  local fire_count = 0
  local bar = screen + ui.TabBar {
    Width = 300,
    Height = 40,
    SelectionChanged = function () fire_count = fire_count + 1 end,
  }
  local tab_a = bar + ui.Tab { Value = "alpha", Width = 100, Height = 40 }

  tab_a:send("Node.LeftButtonUp")
  test.expect_eq(fire_count, 1, "SelectionChanged should fire once on first selection")

  -- Click the same tab again
  tab_a:send("Node.LeftButtonUp")
  test.expect_eq(fire_count, 1, "SelectionChanged should NOT fire again when clicking already-selected tab")

  screen:clear()
  print("PASS: test_tabbar_no_reselect")
end

-- ---------------------------------------------------------------------------
-- Tab: IsSelected and `selected` Lua flag are in sync
-- ---------------------------------------------------------------------------
local function test_tab_selected_flag()
  local screen = ui.Screen { Width = 600, Height = 400, ResizeMode = "NoResize" }

  local bar  = screen + ui.TabBar { Width = 300, Height = 40 }
  local tab_a = bar + ui.Tab { Value = "alpha", Width = 100, Height = 40 }
  local tab_b = bar + ui.Tab { Value = "beta",  Width = 100, Height = 40 }

  tab_a:send("Node.LeftButtonUp")

  test.expect(tab_a.IsSelected,     "Tab alpha: IsSelected should be true")
  test.expect(tab_a.selected,       "Tab alpha: .selected flag (OF_SELECTED) should be set")
  test.expect(not tab_b.IsSelected, "Tab beta: IsSelected should be false after sibling selected")
  test.expect(not tab_b.selected,   "Tab beta: .selected flag should be cleared after sibling selected")

  screen:clear()
  print("PASS: test_tab_selected_flag")
end

-- ---------------------------------------------------------------------------
-- TabView: ViewDidLoad auto-selects the first tab and shows its panel
-- ---------------------------------------------------------------------------
local function test_tabview_initial_selection()
  local screen = ui.Screen { Width = 600, Height = 400, ResizeMode = "NoResize" }

  local view = screen + ui.TabView { Width = 600, Height = 360 }

  local bar   = view + ui.TabBar { Width = 600, Height = 40 }
  local tab_a = bar + ui.Tab { Value = "alpha", Width = 100, Height = 40 }
  local tab_b = bar + ui.Tab { Value = "beta",  Width = 100, Height = 40 }

  local panel_a = view + ui.Node2D { Name = "alpha", Width = 600, Height = 320 }
  local panel_b = view + ui.Node2D { Name = "beta",  Width = 600, Height = 320 }

  -- Trigger ViewDidLoad which sets up initial selection
  view:send("Node.ViewDidLoad")

  test.expect_eq(view.SelectedValue, "alpha", "TabView.SelectedValue should default to first tab 'alpha'")
  test.expect(tab_a.IsSelected,      "First Tab should be selected after ViewDidLoad")
  test.expect(not tab_b.IsSelected,  "Second Tab should NOT be selected after ViewDidLoad")
  test.expect(tab_a:isFocused(),     "First Tab should receive keyboard focus after ViewDidLoad")
  test.expect(not tab_b:isFocused(), "Second Tab should not be focused after ViewDidLoad")
  test.expect(panel_a.Visible,       "Panel 'alpha' should be visible after ViewDidLoad")
  test.expect(not panel_b.Visible,   "Panel 'beta' should be hidden after ViewDidLoad")

  screen:clear()
  print("PASS: test_tabview_initial_selection")
end

-- ---------------------------------------------------------------------------
-- TabView: selecting a different tab switches visible content panel
-- ---------------------------------------------------------------------------
local function test_tabview_panel_switch()
  local screen = ui.Screen { Width = 600, Height = 400, ResizeMode = "NoResize" }

  local view = screen + ui.TabView { Width = 600, Height = 360 }

  local bar   = view + ui.TabBar { Width = 600, Height = 40 }
  local tab_a = bar + ui.Tab { Value = "alpha", Width = 100, Height = 40 }
  local tab_b = bar + ui.Tab { Value = "beta",  Width = 100, Height = 40 }

  local panel_a = view + ui.Node2D { Name = "alpha", Width = 600, Height = 320 }
  local panel_b = view + ui.Node2D { Name = "beta",  Width = 600, Height = 320 }

  view:send("Node.ViewDidLoad")

  -- Initial state: alpha panel visible
  test.expect(panel_a.Visible,     "Panel 'alpha' should be visible initially")
  test.expect(not panel_b.Visible, "Panel 'beta' should be hidden initially")

  -- Click second tab
  tab_b:send("Node.LeftButtonUp")

  test.expect_eq(view.SelectedValue, "beta",  "TabView.SelectedValue should update to 'beta'")
  test.expect(not panel_a.Visible,   "Panel 'alpha' should be hidden after switching to beta")
  test.expect(panel_b.Visible,       "Panel 'beta' should be visible after switching to beta")

  -- Switch back to alpha
  tab_a:send("Node.LeftButtonUp")

  test.expect_eq(view.SelectedValue, "alpha", "TabView.SelectedValue should update back to 'alpha'")
  test.expect(panel_a.Visible,       "Panel 'alpha' should be visible after switching back")
  test.expect(not panel_b.Visible,   "Panel 'beta' should be hidden after switching back")

  screen:clear()
  print("PASS: test_tabview_panel_switch")
end

-- ---------------------------------------------------------------------------
-- TabView: SelectionChanged event fires with SelectedValue and OldValue
-- ---------------------------------------------------------------------------
local function test_tabview_selection_changed_event()
  local screen = ui.Screen { Width = 600, Height = 400, ResizeMode = "NoResize" }

  local selected_value = nil
  local old_value      = nil

  local view = screen + ui.TabView {
    Width = 600,
    Height = 360,
    SelectionChanged = function (self, e)
      selected_value = self.SelectedValue
      if e then old_value = e.OldValue end
    end,
  }

  local bar   = view + ui.TabBar { Width = 600, Height = 40 }
  local tab_a = bar + ui.Tab { Value = "alpha", Width = 100, Height = 40 }
  local tab_b = bar + ui.Tab { Value = "beta",  Width = 100, Height = 40 }

  local panel_a = view + ui.Node2D { Name = "alpha", Width = 600, Height = 320 }
  local panel_b = view + ui.Node2D { Name = "beta",  Width = 600, Height = 320 }

  view:send("Node.ViewDidLoad")

  -- No SelectionChanged should have fired during ViewDidLoad init
  test.expect(selected_value == nil, "SelectionChanged should NOT fire during ViewDidLoad init")

  -- Switch to beta; should fire SelectionChanged
  tab_b:send("Node.LeftButtonUp")

  test.expect_eq(selected_value, "beta",  "TabView SelectionChanged should report SelectedValue = 'beta'")
  test.expect_eq(old_value,      "alpha", "TabView SelectionChanged should report OldValue = 'alpha'")

  -- Switch back to alpha
  selected_value = nil
  old_value      = nil
  tab_a:send("Node.LeftButtonUp")

  test.expect_eq(selected_value, "alpha", "TabView SelectionChanged should report SelectedValue = 'alpha'")
  test.expect_eq(old_value,      "beta",  "TabView SelectionChanged should report OldValue = 'beta'")

  screen:clear()
  print("PASS: test_tabview_selection_changed_event")
end

-- ---------------------------------------------------------------------------
-- Button: new visual properties have correct defaults
-- ---------------------------------------------------------------------------
local function test_button_visual_defaults()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local btn = screen + ui.Button { Width = 100, Height = 40 }

  -- Default DiffuseColor should be a non-zero color (0.3, 0.55, 0.85, 1.0)
  -- We can only test that CornerRadius is a number and SpecularPower > 0.
  test.expect(type(btn.CornerRadius) == "number",  "Button.CornerRadius should be a number")
  test.expect(btn.CornerRadius > 0,                "Button.CornerRadius should have a positive default")
  test.expect(type(btn.SpecularPower) == "number", "Button.SpecularPower should be a number")
  test.expect(btn.SpecularPower > 0,               "Button.SpecularPower should have a positive default")

  -- Setting CornerRadius should be reflected back
  btn.CornerRadius = 12
  test.expect_eq(btn.CornerRadius, 12, "Button.CornerRadius should be settable")

  screen:clear()
  print("PASS: test_button_visual_defaults")
end

-- ---------------------------------------------------------------------------
-- RadioButton: AccentColor and IndicatorSize defaults
-- ---------------------------------------------------------------------------
local function test_radiobutton_visual_defaults()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local group = screen + ui.RadioGroup { Width = 200, Height = 50, Direction = "Horizontal" }
  local rb    = group  + ui.RadioButton { Value = "x", Width = 50, Height = 30 }

  test.expect(type(rb.IndicatorSize) == "number",  "RadioButton.IndicatorSize should be a number")
  test.expect(rb.IndicatorSize > 0,                "RadioButton.IndicatorSize should have a positive default")

  -- Setting IndicatorSize should be reflected back
  rb.IndicatorSize = 20
  test.expect_eq(rb.IndicatorSize, 20, "RadioButton.IndicatorSize should be settable")

  screen:clear()
  print("PASS: test_radiobutton_visual_defaults")
end

-- ---------------------------------------------------------------------------
-- Tab: default selected and unselected colors should be distinct and readable
-- ---------------------------------------------------------------------------
local function test_tab_visual_defaults()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local tab = screen + ui.Tab { Value = "alpha", Width = 100, Height = 40 }

  test.expect_near(tab.SelectedColor.R, 0.24, 0.001, "Tab.SelectedColor.R default")
  test.expect_near(tab.SelectedColor.G, 0.36, 0.001, "Tab.SelectedColor.G default")
  test.expect_near(tab.SelectedColor.B, 0.58, 0.001, "Tab.SelectedColor.B default")
  test.expect_near(tab.SelectedColor.A, 1.00, 0.001, "Tab.SelectedColor.A default")

  test.expect_near(tab.UnselectedColor.R, 0.18, 0.001, "Tab.UnselectedColor.R default")
  test.expect_near(tab.UnselectedColor.G, 0.19, 0.001, "Tab.UnselectedColor.G default")
  test.expect_near(tab.UnselectedColor.B, 0.22, 0.001, "Tab.UnselectedColor.B default")
  test.expect_near(tab.UnselectedColor.A, 0.95, 0.001, "Tab.UnselectedColor.A default")

  test.expect(tab.SelectedColor.R ~= tab.UnselectedColor.R,
    "Tab default selected/unselected colors should differ")
  test.expect_eq(tab.TextHorizontalAlignment, "Center",
    "Tab text should be horizontally centered by default")
  test.expect_eq(tab.TextVerticalAlignment, "Center",
    "Tab text should be vertically centered by default")

  screen:clear()
  print("PASS: test_tab_visual_defaults")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_tabbar_selection()
test_tabbar_selection_changed_event()
test_tabbar_no_reselect()
test_tab_selected_flag()
test_tabview_initial_selection()
test_tabview_panel_switch()
test_tabview_selection_changed_event()
test_button_visual_defaults()
test_radiobutton_visual_defaults()
test_tab_visual_defaults()

print("All TabBar/TabView tests passed.")
