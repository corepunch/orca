local ui = require "orca.UIKit"
local core = require "orca.core"

-- The default stylesheet (share/orca/UIKit/default.css) is the engine's user-agent
-- sheet: it runs before application sheets, which override it rule by rule.
-- Loading plugins again is harmless.
if ui._defaultStylesLoaded then return end
local function register(sheet)
    assert(sheet, "Could not load the default stylesheet")
    local rule = sheet:getFirstChild()
    while rule do
        local nextRule = rule:getNext()
        core.addGlobalStyleRule(rule)
        rule = nextRule
    end
end
register(ui.loadObjectFromCss(SHAREDIR .. "/orca/UIKit/default.css"))
-- Resolve the bundled artwork independently of the app's working directory.
register(ui.loadObjectFromCssString(string.format(
    'CheckBox .control-check { -orca-source: url(%q); }',
    SHAREDIR .. "/orca/UIKit/check.svg")))
ui._defaultStylesLoaded = true
