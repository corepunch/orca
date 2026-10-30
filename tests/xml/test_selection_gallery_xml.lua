local test = require "orca.test"
local orca = require "orca"
local core = require "orca.core"
local filesystem = require "orca.filesystem"
orca.init()
orca.async = function(fn, ...) fn(...) end
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

local function pump(root)
  core.advanceFrame()
  core.runAllPrograms(root)
  core.flushQueue()
  while true do
    local message = orca.system.peekMessage()
    if not message then return end
    orca.system.dispatchMessage(root, message)
  end
end

local form = assert(filesystem.loadObjectFromXmlString([[
  <Screen xmlns="http://schemas.corepunch.com/orca/2006/xml/presentation"
          Width="390" Height="500" ResizeMode="NoResize">
    <StackView Name="Form">
      <TextBlock Name="Model" Text="second"/>
      <RadioGroup Name="Bound">
        <Binding Target="RadioGroup.SelectedValue" Mode="TwoWay" Expression="../Model/Text"/>
        <RadioButton Name="First" Value="first" Text="First" IsChecked="true"/>
        <RadioButton Name="Second" IsChecked="false" Text="Second" Value="second"/>
      </RadioGroup>
      <RadioGroup Name="Explicit" SelectedValue="second">
        <RadioButton Name="ExplicitFirst" Value="first" IsChecked="true"/>
        <RadioButton Name="ExplicitSecond" Value="second"/>
      </RadioGroup>
      <RadioGroup Name="Adopted">
        <RadioButton Name="AdoptedFirst" IsChecked="true" Value="first"/>
        <RadioButton Name="AdoptedSecond" Value="second"/>
      </RadioGroup>
    </StackView>
  </Screen>
]]))
pump(form)
local model = assert(form:findChild("Model", true))
local bound = assert(form:findChild("Bound", true))
local first, second = assert(bound:findChild("First")), assert(bound:findChild("Second"))
test.expect_eq(bound.SelectedValue, "second", "XML TwoWay binding initializes group from model")
test.expect(second.IsChecked and not first.IsChecked, "bound group value wins over authored child checked state")
local explicit = assert(form:findChild("Explicit", true))
test.expect(explicit:findChild("ExplicitSecond").IsChecked and not explicit:findChild("ExplicitFirst").IsChecked,
  "XML group SelectedValue takes precedence over child IsChecked")
local adopted = assert(form:findChild("Adopted", true))
test.expect_eq(adopted.SelectedValue, "first", "XML checked child supplies group value when none is authored")
test.expect(adopted:findChild("AdoptedFirst").IsChecked and not adopted:findChild("AdoptedSecond").IsChecked,
  "XML checked child remains exclusive when Value attribute follows IsChecked")
model.Text = "first"
pump(form)
test.expect(first.IsChecked and not second.IsChecked, "model update synchronizes bound radio children")
second:send("Node.LeftButtonUp")
pump(form)
test.expect_eq(model.Text, "second", "radio activation writes group selection back through TwoWay binding")
test.expect_eq(bound.SelectedValue, "second", "TwoWay feedback retains selected group value")
test.expect(second.IsChecked and not first.IsChecked, "TwoWay feedback preserves radio exclusivity")
second.IsChecked = false
test.expect_eq(bound.SelectedValue, nil, "clearing selected bound radio clears group value")
test.expect_eq(model.Text, nil, "clearing selected radio writes nil back through TwoWay binding")
pump(form)
test.expect_eq(bound.SelectedValue, "", "forward string binding renders nil source as empty group value")
test.expect_eq(model.Text, nil, "forward empty-string normalization does not overwrite nil model value")
test.expect(not first.IsChecked and not second.IsChecked, "nil TwoWay feedback leaves all radios unchecked")
model.Text = "unmatched"
pump(form)
test.expect(not first.IsChecked and not second.IsChecked, "unmatched bound model value clears checked children")
print("PASS: native XML selection gallery layout and radio bindings")
