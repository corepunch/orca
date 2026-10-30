local ui = require "orca.UIKit"
local core = require "orca.core"

-- Global defaults run before application sheets; loading plugins again is harmless.
if ui._selectionStylesLoaded then return end
local function register(sheet)
    assert(sheet, "Could not load selection control styles")
    local rule = sheet:getFirstChild()
    while rule do
        local nextRule = rule:getNext()
        core.addGlobalStyleRule(rule)
        rule = nextRule
    end
end
register(ui.loadObjectFromCss(SHAREDIR .. "/orca/UIKit/selection-controls.css"))
-- Resolve the bundled artwork independently of the app's working directory.
register(ui.loadObjectFromCssString(string.format(
    'CheckBox .control-check { -orca-source: url(%q); }',
    SHAREDIR .. "/orca/UIKit/check.svg")))
ui._selectionStylesLoaded = true
