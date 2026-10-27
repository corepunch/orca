local ui = require "orca.UIKit"

-- A Grid whose background covers the bottom system/keyboard inset while its children
-- remain above it. Use an auto-sized parent row so padding adds to the content.
local Footer = {}

setmetatable(Footer, { __call = function(_, properties)
    local footer = ui.Grid(properties or {})
    footer:attachPropertyProgram("Node.PaddingBottom",
        "MAX({Screen.SafeAreaBottom}, {Screen.KeyboardInsetBottom})", "OneWay", true)
    for _, edge in ipairs { "Left", "Right" } do
        footer:attachPropertyProgram("Node.Padding" .. edge, "{Screen.SafeArea" .. edge .. "}", "OneWay", true)
    end
    return footer
end })

-- Standard content + footer shell. Own the safe-area policy in one place:
-- content avoids top/side insets; the footer paints to the physical bottom.
function Footer.layout(screen, content, footer, properties)
    local layout = ui.Grid(properties or {})
    local contentArea = ui.Grid {}
    contentArea:addChild(content)
    layout.Rows = "1fr auto"
    layout:addChild(contentArea)
    layout:addChild(footer)
    screen:addChild(layout)
    screen.RespectSafeArea = false
    layout:attachPropertyProgram("Node.PaddingTop", "{Screen.SafeAreaTop}", "OneWay", true)
    for _, edge in ipairs { "Left", "Right" } do
        contentArea:attachPropertyProgram("Node.Padding" .. edge, "{Screen.SafeArea" .. edge .. "}", "OneWay", true)
    end
    return layout
end

return Footer
