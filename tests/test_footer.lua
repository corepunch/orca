local core = require "orca.core"
local ui = require "orca.UIKit"
local Footer = require "orca.UIKit.Footer"

local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
local content = ui.Node2D {}
local footer = Footer { Rows = "58px", Background = "#dddddd" }
local controls = ui.Node2D {}
footer:addChild(controls)
local root = Footer.layout(screen, content, footer)

local function check(width, height, top, right, bottom, left, keyboard)
    screen.Width, screen.Height = width, height
    screen.SafeAreaTop, screen.SafeAreaRight = top, right
    screen.SafeAreaBottom, screen.SafeAreaLeft = bottom, left
    screen.KeyboardInsetBottom = keyboard or 0
    local inset = math.max(bottom, keyboard or 0)
    core.advanceFrame()
    core.runAllPrograms(screen)
    screen:UpdateLayout(width, height)
    assert(not screen.RespectSafeArea)
    assert(footer.SafeAreaEdge == "Bottom")
    assert(content:getParent().PaddingLeft == left and content:getParent().PaddingRight == right)
    assert(footer.PaddingLeft == left and footer.PaddingRight == right)
    assert(footer.ActualX == 0 and footer.ActualWidth == width,
        "Footer background must cover side safe areas as well as the bottom")
    assert(footer.PaddingBottom == inset)
    assert(math.abs(controls.ActualHeight - 58) < 1, "Inset must not resize footer controls")
    assert(math.abs(footer.ActualY + footer.ActualHeight - height) < 1, "Footer must reach physical bottom")
    assert(math.abs(footer.ActualY + controls.ActualY + controls.ActualHeight - (height - inset)) < 1,
        "Footer controls must remain above the system gesture area and keyboard")
    assert(content.ActualY == 0 and math.abs(content.ActualHeight - height) < 1,
        "Content must run edge to edge, under the status bar and the footer")
    assert(math.abs(controls.ActualWidth - (width - left - right)) < 1)
end

check(390, 844, 59, 0, 34, 0)
check(390, 844, 59, 0, 34, 0, 318)
check(390, 844, 59, 0, 34, 0, 352)
check(390, 844, 59, 0, 34, 0, 12)
check(390, 844, 59, 0, 34, 0)
check(844, 390, 0, 44, 21, 44)
check(844, 390, 0, 44, 21, 44, 200)
check(390, 844, 0, 0, 0, 0)

local safeScreen = ui.Screen { Width = 430, Height = 932, ResizeMode = "NoResize",
    SafeAreaTop = 59, SafeAreaBottom = 34 }
local safeContent = ui.Node2D {}
safeScreen:addChild(safeContent)
for _, keyboard in ipairs { 0, 318, 352, 0 } do
    safeScreen.KeyboardInsetBottom = keyboard
    core.advanceFrame()
    safeScreen:UpdateLayout(430, 932)
    assert(safeScreen.Height == 932, "Keyboard must not shrink the drawable")
    assert(safeContent.ActualY == 59)
    assert(safeContent.ActualHeight == 932 - 59 - math.max(34, keyboard))
end
-- In a Grid row the footer is laid out as a row, as before it docked.
local rowScreen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize", RespectSafeArea = false,
    SafeAreaBottom = 34 }
local rows = ui.Grid { Rows = "1fr auto" }
local rowContent, rowFooter = ui.Node2D {}, Footer { Rows = "58px" }
rows:addChild(rowContent); rows:addChild(rowFooter)
rowScreen:addChild(rows)
core.advanceFrame()
core.runAllPrograms(rowScreen)
rowScreen:UpdateLayout(390, 844)
assert(math.abs(rowContent.ActualHeight - (844 - 58 - 34)) < 1, "A footer row must still shrink the content row")
assert(math.abs(rowFooter.ActualY + rowFooter.ActualHeight - 844) < 1)
print("PASS: footer and safe-area content respond to keyboard, rotation and zero insets")
