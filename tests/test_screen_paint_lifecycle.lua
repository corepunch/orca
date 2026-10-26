local orca = require "orca"
orca.init()
local core = require "orca.core"
local renderer = require "orca.renderer"
local ui = require "orca.UIKit"

local screen = ui.Screen { Width = 64, Height = 64, ResizeMode = "NoResize" }
screen:addChild(ui.TextBlock { Text = "Pending", Width = 64, Height = 32 })
local continued = 0
local function paint()
    screen:post("Window.Paint", 64, 64)
    orca.async(function() continued = continued + 1 end)
    core.flushQueue()
end

paint()
assert(continued == 1, "A paint before renderer initialization must not block queued callbacks")

-- Opt in to a real offscreen GL context to check the complete lifetime.
if os.getenv("ORCA_RENDERER_LIFECYCLE_NATIVE") == "1" then
    renderer.init(64, 64, true)
    paint()
    assert(continued == 2, "Initialized rendering must continue to process the queue")
end
renderer.shutdown()
local before = continued
paint()
assert(continued == before + 1, "A queued paint after renderer shutdown must be harmless")
print("PASS: Screen painting tolerates an absent renderer and keeps dispatching callbacks")
