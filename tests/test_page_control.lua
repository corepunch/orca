local orca = require "orca"
orca.init()
local core = require "orca.core"
local ui = require "orca.UIKit"
local filesystem = require "orca.filesystem"
local native = os.getenv("ORCA_PAGE_CONTROL_NATIVE") == "1"
if native then require("orca.renderer").init(320, 240, true) end

local LEFT, RIGHT = 130, 131
local screen = ui.Screen { Width = 320, Height = 240, ResizeMode = "NoResize" }
local region = ui.Grid { Width = 320, Height = 240 }
local dots = ui.PageControl { Name = "Dots", Foreground = "#18213a", NumberOfPages = 5,
    HorizontalAlignment = "Center", VerticalAlignment = "Center" }
screen:addChild(region)
region:addChild(dots)
local function settle()
    core.flushQueue()
    screen:UpdateLayout(320, 240)
end
local function render()
    if native then screen:send("Screen.RenderScreen", { width = 320, height = 240 }) end
end
settle()

-- Five 8-point dots 8 points apart, the current one a 24-point pill.
assert(dots.CurrentPage == 0 and not dots.HidesForSinglePage)
assert(dots.ActualWidth == 4 * 16 + 24 and dots.ActualHeight == 26, "the control measures its row of dots")
assert(dots.ActualX == (320 - 88) / 2 and dots.ActualY == (240 - 26) / 2, "the row centers in its slot")
assert(dots:send("Node2D.ForegroundContent"))
render()

-- CurrentPage stays a valid index, also when the page count shrinks.
dots.CurrentPage = 9
assert(dots.CurrentPage == 4, "a page past the end is the last page")
dots.CurrentPage = -3
assert(dots.CurrentPage == 0, "a negative page is the first page")
dots.CurrentPage = 4
dots.NumberOfPages = 3
assert(dots.CurrentPage == 2, "fewer pages move the current page back into range")
settle()
assert(dots.ActualWidth == 2 * 16 + 24, "the row follows the page count")

-- Taps and arrow keys move one page toward their side and notify observers.
local seen = {}
dots.onCurrentPageChanged = function(_, page) seen[#seen + 1] = page end
-- Change notifications are queued, so each move is delivered before the next.
local function step(message, args)
    local handled = dots:send(message, args)
    core.flushQueue()
    return handled
end
dots.CurrentPage = 1
core.flushQueue()
local half = dots.ActualWidth / 2
step("Node.LeftButtonUp", { x = half + 10, y = 13 })
assert(dots.CurrentPage == 2, "a tap on the right half goes to the next page")
step("Node.LeftButtonUp", { x = half + 10, y = 13 })
assert(dots.CurrentPage == 2, "the last page stays the last page")
step("Node.LeftButtonUp", { x = half - 10, y = 13 })
assert(dots.CurrentPage == 1, "a tap on the left half goes to the previous page")
assert(step("Node.KeyDown", { keyCode = LEFT }) and dots.CurrentPage == 0)
assert(step("Node.KeyDown", { keyCode = RIGHT }) and dots.CurrentPage == 1)
assert(not step("Node.KeyDown", { keyCode = 65 }), "other keys stay available to other handlers")
assert(table.concat(seen, ",") == "1,2,1,0,1", "observers see each move once: " .. table.concat(seen, ","))
render()

-- A single page can hide the control; no pages draw nothing.
dots.NumberOfPages = 1
settle()
assert(dots.CurrentPage == 0 and dots:send("Node2D.ForegroundContent"), "one page still shows by default")
dots.HidesForSinglePage = true
settle()
assert(not dots:send("Node2D.ForegroundContent") and dots.ActualWidth == 0, "a single page can hide the control")
dots:send("Node.LeftButtonUp", { x = 0, y = 0 })
assert(dots.CurrentPage == 0)
dots.NumberOfPages = 0
assert(dots.CurrentPage == 0 and not dots:send("Node2D.ForegroundContent"))
dots.HidesForSinglePage = false
dots.NumberOfPages = 4
dots.PageIndicatorColor = "#dce1ec"
settle()
render()

local scene = filesystem.loadObjectFromXmlString(
    '<PageControl NumberOfPages="6" CurrentPage="2" HidesForSinglePage="true" PageIndicatorColor="#DCE1EC"/>')
assert(scene and scene.NumberOfPages == 6 and scene.CurrentPage == 2 and scene.HidesForSinglePage,
    "the control loads from XML as well as Lua")
screen:clear()
print("PASS: PageControl measures its dots, keeps the current page in range, steps on taps and arrow keys, hides for a single page, and loads from XML")
