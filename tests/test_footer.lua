local core = require "orca.core"
local ui = require "orca.UIKit"
local Footer = require "orca.UIKit.Footer"

local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
local content = ui.Node2D {}
local footer = Footer { Rows = "58px", Background = "#dddddd" }
local controls = ui.Node2D {}
footer:addChild(controls)
local root = Footer.layout(screen, content, footer)

local function check(width, height, top, right, bottom, left)
    screen.Width, screen.Height = width, height
    screen.SafeAreaTop, screen.SafeAreaRight = top, right
    screen.SafeAreaBottom, screen.SafeAreaLeft = bottom, left
    core.advanceFrame()
    core.runAllPrograms(screen)
    screen:UpdateLayout(width, height)
    assert(not screen.RespectSafeArea)
    assert(root.PaddingTop == top and root.PaddingLeft == left and root.PaddingRight == right)
    assert(footer.PaddingBottom == bottom)
    assert(math.abs(controls.ActualHeight - 58) < 1, "Inset must not resize footer controls")
    assert(math.abs(footer.ActualY + footer.ActualHeight - height) < 1, "Footer must reach physical bottom")
    assert(math.abs(footer.ActualY + controls.ActualY + controls.ActualHeight - (height - bottom)) < 1,
        "Footer controls must remain above the system gesture area")
    assert(math.abs(content.ActualHeight - (height - top - bottom - 58)) < 1)
    assert(math.abs(controls.ActualWidth - (width - left - right)) < 1)
end

check(390, 844, 59, 0, 34, 0)
check(844, 390, 0, 44, 21, 44)
check(390, 844, 0, 0, 0, 0)
print("PASS: safe-area footer responds to portrait, landscape and zero insets")
