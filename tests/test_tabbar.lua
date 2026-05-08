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
local filesystem = require "orca.filesystem"
local geometry = require "orca.geometry"
local ui   = require "orca.UIKit"

local FALLBACK_WHITE = { R = 1.0, G = 1.0, B = 1.0, A = 1.0 }
local FALLBACK_CONTROL_FOREGROUND = { R = 0.95, G = 0.95, B = 0.97, A = 1.0 }
local FALLBACK_CONTROL_BORDER = { R = 0.7, G = 0.7, B = 0.75, A = 1.0 }
local filesystem_initialized = false

local function ensure_filesystem_init()
  if filesystem_initialized then return end
  filesystem.init("samples/Example")
  filesystem_initialized = true
end

local function get_theme_color(...)
  local keys = { ... }
  for _, key in ipairs(keys) do
    local value = filesystem.getThemeValue(key)
    if value then
      return geometry.Color.parse(value)
    end
  end
  return nil
end

-- Override orca.async so that Lua event callbacks (fired via CORE_HandleObjectMessage)
-- execute synchronously in tests instead of being deferred to the event loop.
orca.async = function (fn, ...) fn(...) end

local function pump_messages(root)
  while true do
    local msg = orca.system.peekMessage()
    if not msg then return end
    orca.system.dispatchMessage(root, msg)
  end
end

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
  pump_messages(screen)
  test.expect_eq(selected_value, "alpha", "SelectionChanged should report SelectedValue = 'alpha'")
  test.expect(old_value == nil, "OldValue should be nil for the first selection")

  -- Switch to beta; OldValue should be 'alpha'
  selected_value = nil
  old_value      = nil
  tab_b:send("Node.LeftButtonUp")
  pump_messages(screen)
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
  pump_messages(screen)
  test.expect_eq(fire_count, 1, "SelectionChanged should fire once on first selection")

  -- Click the same tab again
  tab_a:send("Node.LeftButtonUp")
  pump_messages(screen)
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
  pump_messages(screen)

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
  pump_messages(screen)

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
  pump_messages(screen)

  test.expect_eq(view.SelectedValue, "beta",  "TabView.SelectedValue should update to 'beta'")
  test.expect(not panel_a.Visible,   "Panel 'alpha' should be hidden after switching to beta")
  test.expect(panel_b.Visible,       "Panel 'beta' should be visible after switching to beta")

  -- Switch back to alpha
  tab_a:send("Node.LeftButtonUp")
  pump_messages(screen)

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
  pump_messages(screen)

  test.expect_eq(selected_value, "beta",  "TabView SelectionChanged should report SelectedValue = 'beta'")
  test.expect_eq(old_value,      "alpha", "TabView SelectionChanged should report OldValue = 'alpha'")

  -- Switch back to alpha
  selected_value = nil
  old_value      = nil
  tab_a:send("Node.LeftButtonUp")
  pump_messages(screen)

  test.expect_eq(selected_value, "alpha", "TabView SelectionChanged should report SelectedValue = 'alpha'")
  test.expect_eq(old_value,      "beta",  "TabView SelectionChanged should report OldValue = 'beta'")

  screen:clear()
  print("PASS: test_tabview_selection_changed_event")
end

-- ---------------------------------------------------------------------------
-- Button: new visual properties have correct defaults
-- ---------------------------------------------------------------------------
local function test_button_visual_defaults()
  ensure_filesystem_init()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local btn = screen + ui.Button { Width = 100, Height = 40 }
  local accent = get_theme_color("$control-background", "$accent-background", "$accent")
  local fg = get_theme_color("$control-foreground") or FALLBACK_WHITE

  test.expect_near(btn.DiffuseColor.R, accent.R, 0.001, "Button.DiffuseColor.R theme default")
  test.expect_near(btn.DiffuseColor.G, accent.G, 0.001, "Button.DiffuseColor.G theme default")
  test.expect_near(btn.DiffuseColor.B, accent.B, 0.001, "Button.DiffuseColor.B theme default")
  test.expect_near(btn.DiffuseColor.A, accent.A, 0.001, "Button.DiffuseColor.A theme default")
  test.expect_near(btn.ForegroundColor.R, fg.R, 0.001, "Button.ForegroundColor.R theme default")
  test.expect_near(btn.ForegroundColor.G, fg.G, 0.001, "Button.ForegroundColor.G theme default")
  test.expect_near(btn.ForegroundColor.B, fg.B, 0.001, "Button.ForegroundColor.B theme default")
  test.expect_near(btn.ForegroundColor.A, fg.A, 0.001, "Button.ForegroundColor.A theme default")

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
  ensure_filesystem_init()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local group = screen + ui.RadioGroup { Width = 200, Height = 50, Direction = "Horizontal" }
  local rb    = group  + ui.RadioButton { Value = "x", Width = 50, Height = 30 }
  local accent = get_theme_color("$accent-background", "$accent")

  test.expect_near(rb.AccentColor.R, accent.R, 0.001, "RadioButton.AccentColor.R theme default")
  test.expect_near(rb.AccentColor.G, accent.G, 0.001, "RadioButton.AccentColor.G theme default")
  test.expect_near(rb.AccentColor.B, accent.B, 0.001, "RadioButton.AccentColor.B theme default")
  test.expect_near(rb.AccentColor.A, accent.A, 0.001, "RadioButton.AccentColor.A theme default")

  test.expect(type(rb.IndicatorSize) == "number",  "RadioButton.IndicatorSize should be a number")
  test.expect(rb.IndicatorSize > 0,                "RadioButton.IndicatorSize should have a positive default")

  -- Setting IndicatorSize should be reflected back
  rb.IndicatorSize = 20
  test.expect_eq(rb.IndicatorSize, 20, "RadioButton.IndicatorSize should be settable")

  screen:clear()
  print("PASS: test_radiobutton_visual_defaults")
end

-- ---------------------------------------------------------------------------
-- Input: text-input defaults should use theme control colors and spacing
-- ---------------------------------------------------------------------------
local function test_input_visual_defaults()
  ensure_filesystem_init()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local input = screen + ui.Input { Width = 160, Height = 40 }

  local bg = get_theme_color("$control-background", "$card-background")
  local fg = get_theme_color("$control-foreground") or FALLBACK_CONTROL_FOREGROUND
  local border = get_theme_color("$control-border", "$control-muted") or FALLBACK_CONTROL_BORDER

  test.expect_near(input.BackgroundColor.R, bg.R, 0.001, "Input.BackgroundColor.R theme default")
  test.expect_near(input.BackgroundColor.G, bg.G, 0.001, "Input.BackgroundColor.G theme default")
  test.expect_near(input.BackgroundColor.B, bg.B, 0.001, "Input.BackgroundColor.B theme default")
  test.expect_near(input.BackgroundColor.A, bg.A, 0.001, "Input.BackgroundColor.A theme default")
  test.expect_near(input.ForegroundColor.R, fg.R, 0.001, "Input.ForegroundColor.R theme default")
  test.expect_near(input.ForegroundColor.G, fg.G, 0.001, "Input.ForegroundColor.G theme default")
  test.expect_near(input.ForegroundColor.B, fg.B, 0.001, "Input.ForegroundColor.B theme default")
  test.expect_near(input.ForegroundColor.A, fg.A, 0.001, "Input.ForegroundColor.A theme default")
  test.expect_near(input.BorderColor.R, border.R, 0.001, "Input.BorderColor.R theme default")
  test.expect_near(input.BorderColor.G, border.G, 0.001, "Input.BorderColor.G theme default")
  test.expect_near(input.BorderColor.B, border.B, 0.001, "Input.BorderColor.B theme default")
  test.expect_near(input.BorderColor.A, border.A, 0.001, "Input.BorderColor.A theme default")
  test.expect_near(input.BorderWidthLeft, 1.0, 0.001, "Input left border should default to 1px")
  test.expect_near(input.BorderWidthRight, 1.0, 0.001, "Input right border should default to 1px")
  test.expect_near(input.BorderWidthTop, 1.0, 0.001, "Input top border should default to 1px")
  test.expect_near(input.BorderWidthBottom, 1.0, 0.001, "Input bottom border should default to 1px")
  test.expect_near(input.PaddingLeft, 8.0, 0.001, "Input left padding should default to 8px")
  test.expect_near(input.PaddingRight, 8.0, 0.001, "Input right padding should default to 8px")
  test.expect_near(input.PaddingTop, 6.0, 0.001, "Input top padding should default to 6px")
  test.expect_near(input.PaddingBottom, 6.0, 0.001, "Input bottom padding should default to 6px")

  screen:clear()
  print("PASS: test_input_visual_defaults")
end

-- ---------------------------------------------------------------------------
-- Tab: default selected and unselected colors should be distinct and readable
-- ---------------------------------------------------------------------------
local function test_tab_visual_defaults()
  ensure_filesystem_init()

  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local bar = screen + ui.TabBar { Width = 300, Height = 40 }
  local tab = screen + ui.Tab { Value = "alpha", Width = 100, Height = 40 }

  local selected = geometry.Color.parse(filesystem.getThemeValue("$accent"))
  local unselected = geometry.Color.parse(filesystem.getThemeValue(
    "$panel-background", "$control-background", "$card-background"))

  test.expect_near(bar.BorderWidthTop, 0.0, 0.001, "TabBar top border should stay at 0px")
  test.expect_near(bar.BorderWidthBottom, 1.0, 0.001, "TabBar bottom border should default to 1px")
  test.expect_near(bar.PaddingBottom, 1.0, 0.001, "TabBar bottom padding should reserve space for the underline")
  test.expect_near(bar.BorderColor.R, selected.R, 0.001, "TabBar.BorderColor.R theme default")
  test.expect_near(bar.BorderColor.G, selected.G, 0.001, "TabBar.BorderColor.G theme default")
  test.expect_near(bar.BorderColor.B, selected.B, 0.001, "TabBar.BorderColor.B theme default")
  test.expect_near(bar.BorderColor.A, selected.A, 0.001, "TabBar.BorderColor.A theme default")

  test.expect_near(tab.SelectedColor.R, selected.R, 0.001, "Tab.SelectedColor.R theme default")
  test.expect_near(tab.SelectedColor.G, selected.G, 0.001, "Tab.SelectedColor.G theme default")
  test.expect_near(tab.SelectedColor.B, selected.B, 0.001, "Tab.SelectedColor.B theme default")
  test.expect_near(tab.SelectedColor.A, selected.A, 0.001, "Tab.SelectedColor.A theme default")

  test.expect_near(tab.UnselectedColor.R, unselected.R, 0.001, "Tab.UnselectedColor.R theme default")
  test.expect_near(tab.UnselectedColor.G, unselected.G, 0.001, "Tab.UnselectedColor.G theme default")
  test.expect_near(tab.UnselectedColor.B, unselected.B, 0.001, "Tab.UnselectedColor.B theme default")
  test.expect_near(tab.UnselectedColor.A, unselected.A, 0.001, "Tab.UnselectedColor.A theme default")

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
test_input_visual_defaults()
test_tab_visual_defaults()

print("All TabBar/TabView tests passed.")
