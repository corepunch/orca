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
assert(indicator.ActualWidth == 32 and indicator.ActualHeight == 32)
assert(indicator.ActualX == 144 and indicator.ActualY == 104,
    "The default indicator must center in the allocated content region")
assert(indicator:send("Node2D.ForegroundContent"))
if native then screen:send("Screen.RenderScreen", { width = 320, height = 240 }) end

indicator.IsActive = false
assert(not indicator:send("Node2D.ForegroundContent"),
    "An inactive indicator must stop drawing while preserving its reserved space")
screen:UpdateLayout(320, 240)
assert(indicator.ActualWidth == 32 and indicator.ActualHeight == 32)
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

local xml = filesystem.loadObjectFromXmlString('<ActivityIndicator IsActive="False" Width="24" Height="24"/>')
assert(xml and not xml.IsActive and xml.Width == 24 and xml.IgnoreHitTest,
    "The loading control must support declarative XML as well as Lua")
screen:clear()
print("PASS: ActivityIndicator defaults, centering, sizing, inactive/hidden/detached states and XML")
