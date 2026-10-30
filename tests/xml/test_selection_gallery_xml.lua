local test = require "orca.test"
local orca = require "orca"
local core = require "orca.core"
local filesystem = require "orca.filesystem"
orca.init()
filesystem.init("apps/Controls")
require("orca.core.startup").load_plugins()
local screen = filesystem.loadObject("Controls/Screens/Selection")
assert(screen, "selection gallery XML loads")
core.flushQueue()
screen:UpdateLayout(780, 844)
screen:UpdateLayout(780, 844)

for _, prefix in ipairs { "Dark", "Light" } do
  local previous
  for _, suffix in ipairs { "Unchecked", "Checked", "Mixed", "Disabled" } do
    local control = assert(screen:findChild(prefix .. suffix, true))
    test.expect_eq(control.ActualHeight, 44, "XML gallery checkbox keeps authored 44-point row")
    test.expect_eq(control:findChild("Indicator").ActualWidth, 24, "XML checkbox receives default indicator CSS")
    if previous then
      test.expect_eq(control.ActualY - previous.ActualY, 44, "checkbox rows stack without overlapping")
    end
    previous = control
  end
  local group = assert(screen:findChild(prefix .. "Radios", true))
  previous = nil
  for _, suffix in ipairs { "RadioAll", "RadioUnread", "RadioDisabled" } do
    local control = assert(group:findChild(prefix .. suffix))
    test.expect_eq(control.ActualHeight, 44, "XML gallery radio keeps authored 44-point row")
    if previous then
      test.expect_eq(control.ActualY - previous.ActualY, 44, "radio rows stack without overlapping")
    end
    previous = control
  end
  for _, suffix in ipairs { "SwitchOff", "SwitchOn", "SwitchDisabled" } do
    local control = assert(screen:findChild(prefix .. suffix, true))
    test.expect_eq(control.ActualHeight, 44, "XML gallery switch keeps authored 44-point row")
    test.expect(control:findChild("Indicator"):findChild("Thumb").ActualHeight <= control.ActualHeight,
      "switch thumb fits its touch row")
  end
end
print("PASS: native XML selection gallery layout")
