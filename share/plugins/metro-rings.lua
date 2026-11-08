local ui = require "orca.UIKit"
local core = require "orca.core"

-- The ring around the Pivot back glyph is vector artwork, as Windows Phone
-- strokes an ellipse around it. ApplicationBar buttons are plain glyphs.
-- Global defaults run before application sheets; loading plugins again is harmless.
if ui._metroRingStylesLoaded then return end
local css = ""
for class, file in pairs { ["pivot-back"] = "pivot-back-ring.svg" } do
    css = css .. string.format(".%s { -orca-source: url(%q); }\n", class, SHAREDIR .. "/orca/UIKit/" .. file)
end
local sheet = assert(ui.loadObjectFromCssString(css), "Could not load Metro ring styles")
local rule = sheet:getFirstChild()
while rule do
    local nextRule = rule:getNext()
    core.addGlobalStyleRule(rule)
    rule = nextRule
end
ui._metroRingStylesLoaded = true
