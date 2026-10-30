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
  local radioIndicator = assert(group:findChild(prefix .. "RadioAll"):findChild("Indicator"))
  local radioDot = assert(radioIndicator:findChild("Mark"))
  local offTrack = assert(screen:findChild(prefix .. "SwitchOff", true):findChild("Indicator"))
  local offThumb = assert(offTrack:findChild("Thumb"))
  test.expect_eq(offTrack.ActualHeight, radioIndicator.ActualHeight,
    "off switch rail matches the radio indicator height")
  test.expect_eq(offTrack.BorderWidthLeft, radioIndicator.BorderWidthLeft,
    "off switch outline matches the radio indicator weight")
  test.expect_near(offTrack.BorderWidthLeft, 2, 0.001,
    "off switch preserves the two-point outline")
  test.expect(offTrack.ActualWidth > radioIndicator.ActualWidth,
    "switch rail is a wider version of the radio indicator")
  test.expect_eq(offThumb.ActualWidth, 16, "switch thumb uses the larger sixteen-point circle")
  test.expect(offThumb.ActualWidth > radioDot.ActualWidth, "switch thumb is larger than the radio dot")
  test.expect_eq(offThumb.ActualHeight, offThumb.ActualWidth, "switch thumb remains circular")
  for _, suffix in ipairs { "SwitchOff", "SwitchOn", "SwitchDisabled" } do
    local control = assert(screen:findChild(prefix .. suffix, true))
    test.expect_eq(control.ActualHeight, 44, "XML gallery switch keeps authored 44-point row")
    local track = assert(control:findChild("Indicator"))
    local thumb = assert(track:findChild("Thumb"))
    -- Thumb coordinates are relative to its immediate parent track.
    test.expect(thumb.ActualX >= 0 and thumb.ActualY >= 0,
      "XML switch thumb starts inside the rail")
    test.expect(thumb.ActualX + thumb.ActualWidth <= track.ActualWidth,
      "XML switch thumb fits the rail horizontally")
    test.expect(thumb.ActualY + thumb.ActualHeight <= track.ActualHeight,
      "XML switch thumb fits the rail vertically")
    test.expect_near(thumb.ActualY + thumb.ActualHeight / 2, track.ActualHeight / 2, 0.001,
      "XML switch thumb is vertically centered within the rail")
    local endCenter = control.IsOn and track.ActualWidth - track.ActualHeight / 2 or track.ActualHeight / 2
    test.expect_near(thumb.ActualX + thumb.ActualWidth / 2, endCenter, 0.001,
      "XML switch thumb is centered within the rounded end of its rail")
  end
end
print("PASS: native XML selection gallery layout")
