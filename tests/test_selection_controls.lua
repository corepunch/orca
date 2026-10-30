local test = require "orca.test"
local orca = require "orca"
local core = require "orca.core"
local ui = require "orca.UIKit"
local filesystem = require "orca.filesystem"
orca.init()
filesystem.init("apps/Controls")
require("orca.core.startup").load_plugins()
orca.async = function(fn, ...) fn(...) end

local function pump(screen)
  core.flushQueue()
  while true do
    local message = orca.system.peekMessage()
    if not message then return end
    orca.system.dispatchMessage(screen, message)
  end
end

local function key(node, code)
  node:send("Node.KeyDown", { keyCode = code })
end

local function layout(screen)
  screen:UpdateLayout(screen.Width, screen.Height)
  screen:UpdateLayout(screen.Width, screen.Height)
end

local function tap(screen, node)
  local x, y, current = node.ActualWidth / 2, node.ActualHeight / 2, node
  while current and current.ActualX do
    x, y = x + current.ActualX, y + current.ActualY
    current = current:getParent()
  end
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = x, y = y }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = x, y = y }
  pump(screen)
end

local function checkbox()
  local screen = ui.Screen { Width = 390, Height = 500, ResizeMode = "NoResize" }
  local events = {}
  local check = screen + ui.CheckBox {
    Width = 300, Height = 48, Text = "remember me",
    CheckedChanged = function(_, e) events[#events + 1] = { e.IsChecked, e.IsIndeterminate } end,
  }
  layout(screen)
  local indicator, label = check:findChild("Indicator"), check:findChild("Label")
  test.expect(indicator and label, "checkbox creates styleable indicator and label children")
  test.expect(indicator:findChild("CheckMark") and indicator:findChild("Indeterminate"), "check and mixed artwork are separate CSS parts")
  test.expect_eq(label.Text, "remember me", "generated label reflects Text")
  test.expect(indicator.ActualWidth > 0 and label.ActualX >= indicator.ActualWidth, "native parts arrange beside each other")
  check.Text = "changed label"
  layout(screen)
  test.expect_eq(label.Text, "changed label", "programmatic Text updates generated label")
  tap(screen, check)
  test.expect(check.IsChecked, "pointer activation checks the control")
  test.expect(events[#events] and events[#events][1], "checked event carries current state")
  key(check, 32)
  pump(screen)
  test.expect(not check.IsChecked, "Space unchecks the control")
  key(check, 13)
  pump(screen)
  test.expect(check.IsChecked, "Enter checks the control")
  check.IsIndeterminate = true
  pump(screen)
  local count = #events
  check:send("Node.LeftButtonUp")
  pump(screen)
  test.expect(check.IsChecked and not check.IsIndeterminate, "activating mixed state checks and clears mixed state")
  test.expect_eq(#events, count + 1, "mixed-state user activation emits one coherent change event")
  check.IsEnabled = false
  check:send("Node.LeftButtonUp")
  key(check, 32)
  pump(screen)
  test.expect(check.IsChecked, "disabled checkbox rejects pointer and keyboard activation")
  check.IsChecked = false
  pump(screen)
  test.expect(not events[#events][1], "programmatic state remains bindable while disabled")
  screen:clear()
end

local function toggle()
  local screen = ui.Screen { Width = 390, Height = 500, ResizeMode = "NoResize" }
  local values = {}
  local toggle = screen + ui.ToggleSwitch {
    Width = 300, Height = 48, Text = "notifications",
    Toggled = function(_, e) values[#values + 1] = e.IsOn end,
  }
  layout(screen)
  local track = assert(toggle:findChild("Indicator"))
  local thumb = assert(track:findChild("Thumb"))
  local function expect_thumb_inside_track()
    -- ActualX/Y are relative to the thumb's parent track.
    test.expect(thumb.ActualX >= 0 and thumb.ActualY >= 0, "switch thumb starts inside its rail")
    test.expect(thumb.ActualX + thumb.ActualWidth <= track.ActualWidth,
      "switch thumb stays inside the rail horizontally")
    test.expect(thumb.ActualY + thumb.ActualHeight <= track.ActualHeight,
      "switch thumb stays inside the rail vertically")
    test.expect_near(thumb.ActualY + thumb.ActualHeight / 2, track.ActualHeight / 2, 0.001,
      "switch thumb is vertically centered within its rail")
    local endCenter = toggle.IsOn and track.ActualWidth - track.ActualHeight / 2 or track.ActualHeight / 2
    test.expect_near(thumb.ActualX + thumb.ActualWidth / 2, endCenter, 0.001,
      "switch thumb is centered within the rounded end of its rail")
  end
  expect_thumb_inside_track()
  local offX = thumb.ActualX
  tap(screen, toggle)
  layout(screen)
  expect_thumb_inside_track()
  test.expect(thumb.ActualX > offX, "switch on-state moves thumb to the other end of its rail")
  test.expect(toggle.IsOn and values[#values], "pointer activation turns switch on")
  key(toggle, 32)
  pump(screen)
  test.expect(not toggle.IsOn and values[#values] == false, "Space turns switch off")
  layout(screen)
  expect_thumb_inside_track()
  test.expect_eq(thumb.ActualX, offX, "switch off-state restores thumb position")
  toggle.IsOn = true
  pump(screen)
  test.expect(values[#values], "programmatic switch state emits Toggled")
  toggle.IsEnabled = false
  key(toggle, 13)
  toggle:send("Node.LeftButtonUp")
  test.expect(toggle.IsOn, "disabled switch ignores input")
  layout(screen)
  expect_thumb_inside_track()
  screen:clear()
end

local function radio()
  local screen = ui.Screen { Width = 390, Height = 500, ResizeMode = "NoResize" }
  local events = {}
  local group = screen + ui.RadioGroup {
    Width = 320, Height = 200,
    SelectionChanged = function(_, e) events[#events + 1] = e.SelectedValue end,
  }
  local a = group + ui.RadioButton { Text = "first", Value = "a", Height = 48 }
  local b = group + ui.RadioButton { Text = "disabled", Value = "b", IsEnabled = false, Height = 48 }
  local c = group + ui.RadioButton { Text = "third", Value = "c", Height = 48 }
  layout(screen)
  test.expect(a:findChild("Indicator"):findChild("Mark"), "radio has circular indicator and mark parts")
  a:send("Node.LeftButtonUp")
  pump(screen)
  test.expect(a.IsChecked and not c.IsChecked and group.SelectedValue == "a", "radio pointer selection updates group")
  key(a, 131)
  pump(screen)
  test.expect(c.IsChecked and not a.IsChecked and not b.IsChecked, "Right arrow skips disabled radio sibling")
  test.expect_eq(group.SelectedValue, "c", "keyboard radio selection updates group value")
  key(c, 131)
  pump(screen)
  test.expect(a.IsChecked and not c.IsChecked, "Right arrow wraps to first enabled radio")
  group.SelectedValue = "c"
  pump(screen)
  test.expect(c.IsChecked and not a.IsChecked, "programmatic group selection updates all radio states")
  b:send("Node.LeftButtonUp")
  key(b, 32)
  test.expect(not b.IsChecked and c.IsChecked, "disabled radio ignores activation")
  a.IsChecked = true
  pump(screen)
  test.expect(a.IsChecked and not c.IsChecked and group.SelectedValue == "a", "programmatic radio selection preserves group exclusivity")
  screen:clear()
end

local function css_parts()
  local original = SHAREDIR .. "/orca/UIKit/check.svg"
  -- Both asset paths refer to check artwork, but are distinct strings.
  local alternate = SHAREDIR .. "/orca/UIKit/../UIKit/check.svg"
  local screen = ui.Screen {
    Width = 390, Height = 500, ResizeMode = "NoResize",
    StyleSheet = ui.loadObjectFromCssString(string.format([[
      .custom .control-indicator { width: 32; height: 32; border-color: #ff0000; }
      .custom:checked .control-check { -orca-source: url(%q); }
    ]], alternate)),
  }
  local check = screen + ui.CheckBox { class = "custom", Text = "custom skin", Width = 300, Height = 48 }
  check:send("StyleController.ThemeChanged", { recursive = true })
  layout(screen)
  local indicator = check:findChild("Indicator")
  local mark = indicator:findChild("CheckMark")
  test.expect(indicator.IgnoreHitTest and check:findChild("Label").IgnoreHitTest, "generated parts leave input with the owning control")
  test.expect_eq(indicator.ActualWidth, 32, "application ancestor stylesheet overrides global indicator size")
  test.expect_eq(mark.Source, original, "global CSS assigns the reusable SVG source")
  test.expect_near(mark.Opacity, 0, 0.001, "unchecked CSS hides mark")
  check:send("Node.LeftButtonDown")
  test.expect(not check.IsChecked, "pressed feedback does not change checked state")
  test.expect_near(indicator.BackgroundColor.B, 0.831, 0.01, "pointer-down applies pressed CSS")
  check:send("Node.LeftButtonUp")
  pump(screen)
  test.expect_eq(mark.Source, alternate, "checked ancestor selector updates child SVG Source")
  test.expect_near(mark.Opacity, 1, 0.001, "programmatic checked change updates child CSS immediately")
  check.IsChecked = false
  pump(screen)
  test.expect_eq(mark.Source, original, "unchecking restores base SVG source")
  test.expect_near(mark.Opacity, 0, 0.001, "unchecking clears checked mark styling")
  check.IsIndeterminate = true
  pump(screen)
  test.expect_near(mark.Opacity, 0, 0.001, "mixed state hides checkmark")
  test.expect_near(indicator:findChild("Indeterminate").Opacity, 1, 0.001, "mixed state reveals its own authored part")
  check.IsIndeterminate = false
  pump(screen)
  test.expect_near(indicator:findChild("Indeterminate").Opacity, 0, 0.001, "clearing mixed state restores base styling")
  check:send("Node.LeftButtonDown")
  check.IsEnabled = false
  pump(screen)
  test.expect_near(check.Opacity, 0.4, 0.001, "disabled property styles root")
  check.IsEnabled = true
  pump(screen)
  test.expect_near(check.Opacity, 1, 0.001, "enabling clears disabled styling")
  test.expect_near(indicator.BackgroundColor.A, 0, 0.001, "disabling a pressed control clears pressed feedback")
  check.IndicatorSize = 39
  layout(screen)
  test.expect_eq(indicator.ActualWidth, 39, "explicit IndicatorSize overrides CSS dimensions")
  check.IsChecked = true
  pump(screen)
  layout(screen)
  test.expect_eq(indicator.ActualWidth, 39, "checked-state restyling preserves explicit IndicatorSize")
  screen:clear()
end

local function default_touch_rows()
  local screen = ui.Screen { Width = 390, Height = 500, ResizeMode = "NoResize" }
  local stack = screen + ui.StackView { Width = 300 }
  local first = stack + ui.CheckBox { Text = "first row" }
  local second = stack + ui.ToggleSwitch { Text = "second row" }
  layout(screen)
  test.expect(first.ActualHeight >= 44 and second.ActualHeight >= 44, "default controls preserve 44-point touch rows")
  test.expect(second.ActualY >= first.ActualY + first.ActualHeight, "min-height contributes to desired stack spacing")
  screen:clear()
end

checkbox()
toggle()
radio()
css_parts()
default_touch_rows()
print("PASS: CSS selection control behavior")
