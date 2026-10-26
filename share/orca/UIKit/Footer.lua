local ui = require "orca.UIKit"

-- A Grid whose background covers the bottom system inset while its children
-- remain above it. Use an auto-sized parent row so padding adds to the content.
local Footer = {}

setmetatable(Footer, { __call = function(_, properties)
    local footer = ui.Grid(properties or {})
    footer:attachPropertyProgram("Node.PaddingBottom", "{Screen.SafeAreaBottom}", "OneWay", true)
    return footer
end })

-- Standard content + footer shell. Own the safe-area policy in one place:
-- content avoids top/side insets; the footer paints to the physical bottom.
function Footer.layout(screen, content, footer, properties)
    local layout = ui.Grid(properties or {})
    layout.Rows = "1fr auto"
    layout:addChild(content)
    layout:addChild(footer)
    screen:addChild(layout)
    screen.RespectSafeArea = false
    for _, edge in ipairs { "Left", "Top", "Right" } do
        layout:attachPropertyProgram("Node.Padding" .. edge, "{Screen.SafeArea" .. edge .. "}", "OneWay", true)
    end
    return layout
end

return Footer
