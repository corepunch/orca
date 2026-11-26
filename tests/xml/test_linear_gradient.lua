local test = require "orca.test"
require "orca.renderer"
require "orca.UIKit"
require "orca.SceneKit"
local filesystem = require "orca.filesystem"

local root = filesystem.loadObjectFromXmlString([[
<TextBlock3D Text="Gradient">
  <TextBlock3D.ForegroundGradient>
    <LinearGradientBrush StartColor="sc#1,0.29999998,0.29999998,0.29999998" EndColor="#ffffffff"
      StartPoint="0,0" EndPoint="0,1" Mapping="TextLine"
      Interpolation="SmoothStep" SmoothStart="0.2" SmoothEnd="0.8"
      ColorSpace="Linear"/>
  </TextBlock3D.ForegroundGradient>
</TextBlock3D>
]])
test.expect(root ~= nil, "3D gradient XML loads")
local brush = root.ForegroundGradient
test.expect(brush ~= nil, "3D foreground gradient is assigned")
test.expect_near(brush.StartColor.R, 0.29999998, 0.0000001, "gradient start color")
test.expect_near(brush.StartPoint.X, 0, 0.001, "gradient start x")
test.expect_near(brush.StartPoint.Y, 0, 0.001, "gradient start y")
test.expect_near(brush.EndPoint.X, 0, 0.001, "gradient end x")
test.expect_near(brush.EndPoint.Y, 1, 0.001, "gradient end y")
test.expect_eq(brush.Mapping, "TextLine", "line mapping")
test.expect_eq(brush.Interpolation, "SmoothStep", "smooth interpolation")
test.expect_eq(brush.ColorSpace, "Linear", "linear color encoding")
test.expect_near(brush.SmoothStart, 0.2, 0.001, "smooth start")
test.expect_near(brush.SmoothEnd, 0.8, 0.001, "smooth end")

local node = filesystem.loadObjectFromXmlString([[
<Node2D>
  <Node2D.BackgroundGradient><LinearGradientBrush/></Node2D.BackgroundGradient>
</Node2D>
]])
test.expect(node ~= nil and node.BackgroundGradient ~= nil, "2D gradient XML loads")
local defaults = node.BackgroundGradient
test.expect_eq(defaults.Mapping, "Box", "default mapping")
test.expect_eq(defaults.Interpolation, "Linear", "default interpolation")
test.expect_eq(defaults.ColorSpace, "sRGB", "default color encoding")
test.expect_near(defaults.StartColor.A, 1, 0.001, "default start alpha")
test.expect_near(defaults.EndColor.R, 1, 0.001, "default end color")
test.expect_near(defaults.SmoothEnd, 1, 0.001, "default smooth end")
print("PASS: linear gradient XML")
