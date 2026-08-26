-- From ORCA root: build/bin/orca -test=samples/Book/Tests/test_workshop_ui.lua
local core = require "orca.core"
local ui = require "orca.UIKit"
local filesystem = require "orca.filesystem"
local system = require "orca.system"
local function pump(root)
    while true do
        local msg = system.peekMessage()
        if not msg then break end
        -- Window.Paint requires a live GL context; this is a headless interaction test.
        if msg.message ~= 139758431 then system.dispatchMessage(root, msg) end
    end
end
local project = assert(filesystem.init("samples/Book"))
local file = assert(io.open("Book/Screens/Start.xml", "r"))
local xml = file:read("*a")
file:close()
local screen = assert(filesystem.loadObjectFromXmlString(xml))
core.flushQueue()
screen:UpdateLayout(1024, 768)
local function find(name) return screen:findChild(name, true) end
assert(find("Hotspot_hook"), "Projected hook circle missing")
assert(find("Hotspot_door"), "Projected door circle missing")
assert(find("Hotspot_clock"), "Projected clock circle missing")
local oil = assert(find("Hotspot_oil"), "Projected oil circle missing")
assert(math.abs(oil.ActualWidth - 48) < 1 and math.abs(oil.ActualHeight - 48) < 1)
assert(oil.MarginLeft > 800 and oil.MarginLeft < 850)
assert(oil.ActualY > 490 and oil.ActualY < 550, "Circle lost projected vertical position")
assert(oil.BorderWidthLeft == 3, "Circle stylesheet did not load")
assert(not find("Options"), "Companion choices must not be rendered")
assert(system.dispatchMessage { target = screen, message = "LeftButtonUp", x = oil.ActualX + 24, y = oil.ActualY + 24 })
pump(screen)

assert(not find("Hotspots").Visible, "Targets must be hidden during action beat")
assert(find("Continue").Visible)
screen:UpdateLayout(1024, 768)
local next = find("Continue")
assert(system.dispatchMessage { target = screen, message = "LeftButtonUp", x = next.ActualX + 10, y = next.ActualY + 10 })
pump(screen)

assert(not find("Continue").Visible)
assert(not find("Hotspot_oil"), "Collected oil target must stay removed")
assert(find("Hotspot_door"), "Permanent scenery must return")
assert(find("SceneDescription").Text:find("Grandfather Tolliver", 1, true))
print("PASS: workshop XML, projected circles, CSS and click/Continue events")
