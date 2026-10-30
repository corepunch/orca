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

local function fixture(value)
  local screen = ui.Screen { Width = 390, Height = 500, ResizeMode = "NoResize" }
  local events = {}
  local group = screen + ui.RadioGroup {
    Width = 320, Height = 240, SelectedValue = value,
    SelectionChanged = function(_, e)
      events[#events + 1] = { old = e.OldValue, selected = e.SelectedValue }
    end,
  }
  local a = group + ui.RadioButton { Text = "first", Value = "a", Height = 48 }
  group:addChild(ui.TextBlock { Text = "non-radio child", Height = 20 })
  local b = group + ui.RadioButton { Text = "disabled", Value = "b", Height = 48, IsEnabled = false }
  local c = group + ui.RadioButton { Text = "third", Value = "c", Height = 48 }
  screen:UpdateLayout(screen.Width, screen.Height)
  screen:UpdateLayout(screen.Width, screen.Height)
  pump(screen)
  return screen, group, a, b, c, events
end

local function selected(group, a, b, c, value, message)
  test.expect_eq(group.SelectedValue, value, message .. ": group value")
  test.expect_eq(a.IsChecked, value == "a", message .. ": first state")
  test.expect_eq(b.IsChecked, value == "b", message .. ": disabled state")
  test.expect_eq(c.IsChecked, value == "c", message .. ": third state")
end

local function key(node, code)
  node:send("Node.KeyDown", { keyCode = code })
end

local function initial_and_programmatic_selection()
  local screen, group, a, b, c, events = fixture("c")
  selected(group, a, b, c, "c", "initial group value synchronizes attached children")
  test.expect_eq(#events, 0, "initial group value does not emit a user-selection event")
  group.SelectedValue = "a"
  pump(screen)
  selected(group, a, b, c, "a", "programmatic group selection is exclusive")
  group.SelectedValue = "b"
  pump(screen)
  selected(group, a, b, c, "b", "disabled options remain programmatically selectable")
  test.expect_eq(#events, 0, "group value assignments synchronize silently")
  c.IsChecked = true
  pump(screen)
  selected(group, a, b, c, "c", "programmatic button selection updates its group")
  test.expect_eq(#events, 1, "programmatic button selection emits one group event")
  test.expect_eq(events[1].old, "b", "programmatic button event preserves previous value")
  test.expect_eq(events[1].selected, "c", "programmatic button event carries next value")
  group.SelectedValue = "unknown"
  pump(screen)
  test.expect(not a.IsChecked and not b.IsChecked and not c.IsChecked, "unmatched group value clears every checked child")
  group.SelectedValue = nil
  pump(screen)
  test.expect(not a.IsChecked and not b.IsChecked and not c.IsChecked, "nil group value clears every checked child")
  test.expect_eq(#events, 1, "unmatched and nil group value assignments remain silent")
  screen:clear()
end

local function activation_is_idempotent()
  local screen, group, a, b, c, events = fixture("a")
  a:send("Node.LeftButtonUp")
  key(a, 32)
  key(a, 13)
  pump(screen)
  selected(group, a, b, c, "a", "activating selected option never deselects it")
  test.expect_eq(#events, 0, "repeated selection emits no change events")
  c:send("Node.LeftButtonUp")
  pump(screen)
  selected(group, a, b, c, "c", "pointer selects a different option")
  key(a, 32)
  pump(screen)
  selected(group, a, b, c, "a", "Space selects a different option")
  key(c, 13)
  pump(screen)
  selected(group, a, b, c, "c", "Enter selects a different option")
  test.expect_eq(#events, 3, "each distinct activation emits exactly one event")
  b:send("Node.LeftButtonUp")
  key(b, 32)
  key(b, 13)
  key(b, 131)
  pump(screen)
  selected(group, a, b, c, "c", "disabled option rejects pointer, activation, and navigation keys")
  test.expect_eq(#events, 3, "disabled input emits no events")
  screen:clear()
end

local function keyboard_navigation()
  local screen, group, a, b, c = fixture("a")
  for _, code in ipairs { 131, 129 } do
    group.SelectedValue = "a"
    key(a, code)
    pump(screen)
    selected(group, a, b, c, "c", "forward arrow skips disabled and non-radio siblings")
    key(c, code)
    pump(screen)
    selected(group, a, b, c, "a", "forward arrow wraps from last enabled option")
  end
  for _, code in ipairs { 130, 128 } do
    group.SelectedValue = "c"
    key(c, code)
    pump(screen)
    selected(group, a, b, c, "a", "backward arrow skips disabled and non-radio siblings")
    key(a, code)
    pump(screen)
    selected(group, a, b, c, "c", "backward arrow wraps from first enabled option")
  end
  a.IsEnabled = false
  group.SelectedValue = "c"
  for _, code in ipairs { 128, 129, 130, 131 } do key(c, code) end
  pump(screen)
  selected(group, a, b, c, "c", "one enabled option remains selected through every arrow")
  screen:clear()
end

local function independent_groups()
  local screen, first, a, b, c, events = fixture("a")
  local second = screen + ui.RadioGroup { SelectedValue = "a", Width = 320, Height = 120 }
  local otherA = second + ui.RadioButton { Value = "a", Text = "other first" }
  local otherC = second + ui.RadioButton { Value = "c", Text = "other third" }
  pump(screen)
  c.IsChecked = true
  pump(screen)
  selected(first, a, b, c, "c", "first group selects independently")
  test.expect(otherA.IsChecked and not otherC.IsChecked, "same values in another group are unaffected")
  otherC.IsChecked = true
  pump(screen)
  test.expect(otherC.IsChecked and not otherA.IsChecked, "second group enforces its own exclusivity")
  selected(first, a, b, c, "c", "second group leaves first group unchanged")
  test.expect_eq(#events, 1, "second group selection does not fire first group events")
  screen:clear()
end

local function queued_event_snapshots()
  local screen, group, a, b, c, events = fixture("a")
  c.IsChecked = true
  a.IsChecked = true
  c.IsChecked = true
  pump(screen)
  test.expect_eq(#events, 3, "multiple changes before dispatch deliver all selection events")
  local expected = { { "a", "c" }, { "c", "a" }, { "a", "c" } }
  for index, value in ipairs(expected) do
    local event = events[index] or {}
    test.expect_eq(event.old, value[1], "queued event retains its own previous value")
    test.expect_eq(event.selected, value[2], "queued event retains its own selected value")
  end
  selected(group, a, b, c, "c", "dispatching stale events does not roll back current selection")
  screen:clear()
end

local function empty_group()
  local screen = ui.Screen { Width = 390, Height = 500, ResizeMode = "NoResize" }
  local group = screen + ui.RadioGroup { SelectedValue = "missing", Width = 320, Height = 120 }
  group.SelectedValue = nil
  group.SelectedValue = "still missing"
  pump(screen)
  test.expect_eq(group.SelectedValue, "still missing", "empty group safely retains requested value")
  screen:clear()
end

local function clearing_and_value_changes()
  local screen, group, a, b, c, events = fixture("a")
  a.IsChecked = false
  pump(screen)
  selected(group, a, b, c, nil, "clearing selected button clears group")
  test.expect_eq(#events, 1, "clearing selected button emits one event")
  test.expect_eq((events[1] or {}).old, "a", "clear event preserves previous value")
  test.expect_eq((events[1] or {}).selected, nil, "clear event carries nil next value")
  a.IsChecked = false
  pump(screen)
  test.expect_eq(#events, 1, "clearing an unchecked button is a no-op")
  group.SelectedValue = "c"
  c.Value = "renamed"
  pump(screen)
  test.expect_eq(group.SelectedValue, "renamed", "selected button value change updates group")
  test.expect(c.IsChecked and not a.IsChecked and not b.IsChecked, "renamed selected button remains exclusive")
  test.expect_eq(#events, 2, "renaming selected button emits a value-change event")
  test.expect_eq((events[2] or {}).old, "c", "rename event preserves old value")
  test.expect_eq((events[2] or {}).selected, "renamed", "rename event carries new value")
  group.SelectedValue = "future"
  a.Value = "future"
  pump(screen)
  test.expect(a.IsChecked and not b.IsChecked and not c.IsChecked, "unselected value change adopts matching group value")
  test.expect_eq(#events, 2, "matching existing requested group value synchronizes silently")
  group.SelectedValue = ""
  pump(screen)
  test.expect(not a.IsChecked and not b.IsChecked and not c.IsChecked, "empty group value clears all options")
  screen:clear()
end

local function dynamic_attachment()
  local screen = ui.Screen { Width = 390, Height = 500, ResizeMode = "NoResize" }
  local group = screen + ui.RadioGroup { Width = 320, Height = 160 }
  local first = group + ui.RadioButton { Value = "first", IsChecked = true }
  pump(screen)
  test.expect_eq(group.SelectedValue, "first", "initially checked attached button supplies absent group value")
  test.expect(first.IsChecked, "initially checked button stays selected")
  local second = group + ui.RadioButton { Value = "second", IsChecked = true }
  pump(screen)
  test.expect(first.IsChecked and not second.IsChecked, "existing group value wins over newly attached checked button")
  group.SelectedValue = "later"
  local later = group + ui.RadioButton { Value = "later" }
  pump(screen)
  test.expect(later.IsChecked and not first.IsChecked and not second.IsChecked, "late matching button adopts requested group value")
  local destination = screen + ui.RadioGroup { SelectedValue = "second", Width = 320, Height = 160 }
  second:removeFromParent()
  destination:addChild(second)
  pump(screen)
  test.expect(second.IsChecked and destination.SelectedValue == "second", "reparented button conforms to destination group value")
  test.expect(later.IsChecked and group.SelectedValue == "later", "moving unchecked sibling leaves source selection unchanged")
  screen:clear()
end

local function disabled_only_group()
  local screen, group, a, b, c, events = fixture("b")
  a.IsEnabled, c.IsEnabled = false, false
  for _, radio in ipairs { a, b, c } do
    radio:send("Node.LeftButtonUp")
    for _, code in ipairs { 13, 32, 128, 129, 130, 131 } do key(radio, code) end
  end
  pump(screen)
  selected(group, a, b, c, "b", "disabled-only group preserves programmatic selection through input")
  test.expect_eq(#events, 0, "disabled-only group emits no input events")
  screen:clear()
end

initial_and_programmatic_selection()
activation_is_idempotent()
keyboard_navigation()
independent_groups()
queued_event_snapshots()
empty_group()
clearing_and_value_changes()
dynamic_attachment()
disabled_only_group()
print("PASS: RadioGroup selection, navigation, and event snapshots")
