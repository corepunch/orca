local orca = require "orca"
orca.init()
local core = require "orca.core"
local ui = require "orca.UIKit"
local filesystem = require "orca.filesystem"
filesystem.init("apps/chat")
local native = os.getenv("ORCA_ACTIVITY_NATIVE") == "1"
if native then require("orca.renderer").init(320, 240, true) end

local screen = ui.Screen { Width = 320, Height = 240, ResizeMode = "NoResize" }
local region = ui.Grid { Width = 320, Height = 240 }
local indicator = ui.ActivityIndicator { Name = "Loading", Foreground = "#0084ff" }
screen:addChild(region)
region:addChild(indicator)
core.flushQueue()
screen:UpdateLayout(320, 240)
assert(indicator.IsActive and indicator.IgnoreHitTest)
assert(indicator.ActualWidth == 64 and indicator.ActualHeight == 64)
assert(indicator.ActualX == 128 and indicator.ActualY == 88,
    "The default indicator must center in the allocated content region")
assert(indicator:send("Node2D.ForegroundContent"))
if native then screen:send("Screen.RenderScreen", { width = 320, height = 240 }) end

indicator.IsActive = false
assert(not indicator:send("Node2D.ForegroundContent"),
    "An inactive indicator must stop drawing while preserving its reserved space")
screen:UpdateLayout(320, 240)
assert(indicator.ActualWidth == 64 and indicator.ActualHeight == 64)
indicator.IsActive = true
region.Visible = false
assert(not indicator:send("Object.Animate"), "Hidden pages must not keep animating")
region.Visible = true
indicator.Width, indicator.Height = 48, 48
screen:UpdateLayout(320, 240)
assert(indicator.ActualX == 136 and indicator.ActualY == 96)
if native then screen:send("Screen.RenderScreen", { width = 320, height = 240 }) end
indicator:removeFromParent()
assert(not indicator:send("Object.Animate"), "Detached indicators must not keep animating")

-- The same intrinsic indicator shrinks to the shorter available dimension,
-- including padding, rather than clipping a full-size circle in a narrow slot.
local compact = ui.ActivityIndicator { Foreground = "#0084ff" }
region:addChild(compact)
for _, size in ipairs { { 20, 120, 0, 20 }, { 120, 20, 0, 20 }, { 24, 96, 4, 16 }, { 80, 80, 12, 56 }, { 1, 1, 0, 1 }, { 0, 24, 0, 0 } } do
    region.Width, region.Height, region.Padding = size[1], size[2], tostring(size[3])
    screen:UpdateLayout(320, 240)
    assert(compact.ActualWidth == size[4] and compact.ActualHeight == size[4],
        "Indicators must stay square and fit the available content box")
    assert(math.abs(compact.ActualX + compact.ActualWidth / 2 - region.ActualWidth / 2) < 1
        and math.abs(compact.ActualY + compact.ActualHeight / 2 - region.ActualHeight / 2) < 1,
        "Compact indicators must stay centered")
    if native then screen:send("Screen.RenderScreen", { width = 320, height = 240 }) end
end

local load_scene = filesystem.loadObjectFromXmlString or filesystem.loadObjectFromLispString
local source = filesystem.loadObjectFromXmlString
    and '<ActivityIndicator IsActive="False" Width="24" Height="24"/>'
    or '(ActivityIndicator IsActive=false Width=24 Height=24)'
local scene = load_scene(source)
assert(scene and not scene.IsActive and scene.Width == 24 and scene.IgnoreHitTest,
    "The loading control must support the selected declarative scene format as well as Lua")
screen:clear()
print("PASS: ActivityIndicator 64-point defaults, container clamping, centering, explicit sizing, inactive/hidden/detached states and scene loading")
