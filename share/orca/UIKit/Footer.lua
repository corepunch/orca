local ui = require "orca.UIKit"

-- A Grid docked to the bottom of its parent whose background covers the bottom
-- system/keyboard inset while its children remain above it. In a Grid row it
-- is laid out as a row; use an auto-sized row so padding adds to the content.
local Footer = {}

setmetatable(Footer, { __call = function(_, properties)
    local footer = ui.Grid(properties or {})
    footer.SafeAreaEdge = "Bottom"
    footer:attachPropertyProgram("Node.PaddingBottom",
        "MAX({Screen.SafeAreaBottom}, {Screen.KeyboardInsetBottom})", "OneWay", true)
    for _, edge in ipairs { "Left", "Right" } do
        footer:attachPropertyProgram("Node.Padding" .. edge, "{Screen.SafeArea" .. edge .. "}", "OneWay", true)
    end
    return footer
end })

-- Standard edge-to-edge content + footer shell. The content fills the screen
-- and runs under the docked footer and the status bar; scrolling content insets
-- itself by both, and a Pivot keeps its header clear of the status bar.
function Footer.layout(screen, content, footer, properties)
    local layout = ui.Node2D(properties or {})
    local contentArea = ui.Node2D {}
    contentArea:addChild(content)
    layout:addChild(contentArea)
    layout:addChild(footer)
    screen:addChild(layout)
    screen.RespectSafeArea = false
    for _, edge in ipairs { "Left", "Right" } do
        contentArea:attachPropertyProgram("Node.Padding" .. edge, "{Screen.SafeArea" .. edge .. "}", "OneWay", true)
    end
    return layout
end

return Footer
