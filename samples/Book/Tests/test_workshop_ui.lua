-- From ORCA root: build/bin/orca -test=samples/Book/Tests/test_workshop_ui.lua
local core = require "orca.core"
local ui = require "orca.UIKit"
local filesystem = require "orca.filesystem"
local system = require "orca.system"
local projection = require "Book.Scripts.SceneProjection"
local camera_export = require "Book.Scripts.WorkshopCamera"
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

-- Establishing page: the workshop floor projects its visible subjects as circles
-- and offers room exits as text choices.
assert(find("Hotspot_KEY-HOOK"), "Projected hook circle missing")
assert(find("Hotspot_PET-DOOR"), "Projected door circle missing")
assert(find("Hotspot_CLOCK-FACE"), "Projected clock circle missing")
local oil = assert(find("Hotspot_OIL-CAN"), "Projected oil circle missing")
assert(math.abs(oil.ActualWidth - 48) < 1 and math.abs(oil.ActualHeight - 48) < 1)
local expectedOil = assert(projection.project(
    camera_export.cameras[camera_export.default_camera], camera_export.anchors["OIL-CAN"],
    camera_export.source_width, camera_export.source_height, 1024, 768))
assert(math.abs(oil.MarginLeft - (expectedOil.x - 24)) < 1)
assert(math.abs(oil.ActualY - (expectedOil.y - 24)) < 1, "Circle lost projected vertical position")
assert(oil.BorderWidthLeft == 3, "Circle stylesheet did not load")
assert(find("Choices"):getFirstChild(), "Room exits must be offered as choices")
assert(find("SceneDescription").Text:find("Grandfather Tolliver", 1, true))

-- Tapping the oil can holds a take beat: circles hide and Continue appears.
assert(system.dispatchMessage { target = screen, message = "LeftButtonUp", x = oil.ActualX + 24, y = oil.ActualY + 24 })
pump(screen)
assert(not find("Hotspots").Visible, "Targets must be hidden during action beat")
assert(find("Continue").Visible)
assert(find("SceneDescription").Text:find("oil can", 1, true), "Beat shows the result prose")

-- Continue restores the establishing page with the collected oil can removed.
screen:UpdateLayout(1024, 768)
local next = find("Continue")
assert(system.dispatchMessage { target = screen, message = "LeftButtonUp", x = next.ActualX + 10, y = next.ActualY + 10 })
pump(screen)
assert(not find("Continue").Visible)
assert(not find("Hotspot_OIL-CAN"), "Collected oil target must stay removed")
assert(find("Hotspot_PET-DOOR"), "Permanent scenery must return")
assert(find("SceneDescription").Text:find("Grandfather Tolliver", 1, true))
print("PASS: workshop XML, projected circles, focus choices and beat/Continue events")
