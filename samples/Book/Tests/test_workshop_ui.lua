-- Display-backed test, from ORCA root: build/bin/orca -test=samples/Book/Tests/test_workshop_ui.lua
local core = require "orca.core"
local renderer = require "orca.renderer"
local ui = require "orca.UIKit"
local filesystem = require "orca.filesystem"
local system = require "orca.system"
local projection = require "Book.Scripts.SceneProjection"
local camera_export = require "Book.Scripts.WorkshopCamera"
local function pump(root)
    for _ = 1, 1000 do
        local msg = system.peekMessage()
        if not msg then break end
        -- Exercise input/layout without scheduling incidental paint work.
        if msg.message ~= 139758431 then system.dispatchMessage(root, msg) end
    end
end
local project = assert(filesystem.init("samples/Book"))
renderer.init(1024, 768, false)
local file = assert(io.open("Book/Screens/Start.xml", "r"))
local xml = file:read("*a")
file:close()
local screen = assert(filesystem.loadObjectFromXmlString(xml))
pump(screen)
screen:UpdateLayout(1024, 768)
local function find(name) return screen:findChild(name, true) end

-- Establishing page: the workshop floor projects its visible subjects as circles
-- and offers room exits as text choices.
assert(find("Hotspot_KEY-HOOK"), "Projected hook circle missing")
assert(find("Hotspot_PET-DOOR"), "Projected door circle missing")
assert(find("Hotspot_CLOCK-FACE"), "Projected clock circle missing")
local oil = assert(find("Hotspot_OIL-CAN"), "Projected oil circle missing")
assert(math.abs(oil.ActualWidth + oil.BorderWidthLeft + oil.BorderWidthRight - 48) < 1)
assert(math.abs(oil.ActualHeight + oil.BorderWidthTop + oil.BorderWidthBottom - 48) < 1)
local expectedOil = assert(projection.project(
    camera_export.cameras[camera_export.default_camera], camera_export.anchors["OIL-CAN"],
    camera_export.source_width, camera_export.source_height, 1024, 768))
assert(math.abs(oil.MarginLeft - (expectedOil.x - 24)) < 1)
assert(math.abs(oil.ActualY - oil.BorderWidthTop - (expectedOil.y - 24)) < 1,
    "Circle border lost projected vertical position")
assert(oil.BorderWidthLeft == 3, "Circle stylesheet did not load")
assert(find("Choices"):getFirstChild(), "Room exits must be offered as choices")
assert(find("SceneDescription").Text:find("Grandfather Tolliver", 1, true))

-- Tapping the oil can holds a take beat: circles hide and Continue appears.
assert(system.dispatchMessage { target = screen, message = "LeftButtonUp", x = oil.ActualX + 24, y = oil.ActualY + 24 })
pump(screen)
assert(not find("Hotspots").Visible, "Targets must be hidden during action beat")
assert(find("Continue").Visible)
assert(find("SceneDescription").Text:find("Taken.", 1, true), "Beat shows the take result prose")

-- Continue restores the establishing page with the collected oil can removed.
screen:UpdateLayout(1024, 768)
local next = find("Continue")
assert(system.dispatchMessage { target = screen, message = "LeftButtonUp", x = next.ActualX + 10, y = next.ActualY + 10 })
pump(screen)
assert(not find("Continue").Visible)
assert(not find("Hotspot_OIL-CAN"), "Collected oil target must stay removed")
assert(find("Hotspot_PET-DOOR"), "Permanent scenery must return")
assert(find("SceneDescription").Text:find("Grandfather Tolliver", 1, true))
local function click(node)
    screen:UpdateLayout(1024, 768)
    assert(system.dispatchMessage { target = screen, message = "LeftButtonUp",
        x = node.ActualX + node.ActualWidth / 2, y = node.ActualY + node.ActualHeight / 2 })
    pump(screen)
end
local function choice(label)
    local node = find("Choices"):getFirstChild()
    while node do
        if node.Text == label then return node end
        node = node:getNext()
    end
    error("Missing choice: " .. label)
end

click(assert(find("Hotspot_KEY-HOOK")))
assert(not find("Hotspots").Visible, "Focus hides establishing markers")
assert(choice("Examine the empty hook"))
click(choice("Step back into the workshop"))
assert(find("Hotspots").Visible and find("Hotspot_KEY-HOOK"), "Back restores room markers")

click(choice("Cross to the tool bench"))
assert(choice("Return to the workshop floor"), "East reaches the tool bench")
assert(not find("Hotspot_KEY-HOOK"), "Previous room markers must be removed")
click(choice("Return to the workshop floor"))
assert(find("Hotspot_KEY-HOOK"), "West restores workshop markers")
assert(not find("Hotspot_OIL-CAN"), "Inventory survives room navigation")
print("PASS: native workshop XML/JPEG/fonts, projected rings, take/Continue, hook focus/Back, east/west navigation")
