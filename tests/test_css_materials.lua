local test = require "orca.test"
-- -orca-foreground-material and -orca-background-material attach a shader
-- Material by path, as background-image attaches a texture. The gradient
-- looks (apps/octopus/Styles/looks.css) use them for gradient type and fills.
local core = require "orca.core"
local ui = require "orca.UIKit"
require("orca.filesystem").init("apps/octopus")
require("orca.core.startup").load_plugins()

local screen = ui.Screen {
    Width = 390, Height = 844, ResizeMode = "NoResize",
    StyleSheet = ui.loadObjectFromCssString [[
        .type { -orca-foreground-material: url("Octopus/Materials/NeonTitle"); }
        .fill { -orca-background-material: url("Octopus/Materials/NeonFill"); }
    ]],
}
local text, fill, plain = ui.TextBlock { class = "type", Text = "Gradient" }, ui.Node2D { class = "fill" }, ui.Node2D {}
for _, node in ipairs { text, fill, plain } do screen:addChild(node) end
core.advanceFrame()
core.runAllPrograms(screen)
screen:UpdateLayout(screen.Width, screen.Height)

test.expect(text.ForegroundMaterial ~= nil, "the foreground material is attached")
test.expect(text.ForegroundMaterial.Name == "NeonTitle", "the foreground material is the one named by url()")
test.expect(fill.BackgroundMaterial ~= nil and fill.BackgroundMaterial.Name == "NeonFill", "the background material is attached")
test.expect(plain.ForegroundMaterial == nil and plain.BackgroundMaterial == nil, "unstyled nodes keep no material")
screen:clear()
print("PASS: CSS foreground and background materials")
