-- Octopus row buttons (post author, body, reactions, activity rows) are
-- layout containers. UIKit's default Button fill must never show behind them:
-- not in either appearance, not in a pointer state, and not when the palette
-- class has not reached the row's ancestors.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
local ui = require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/octopus")
require("orca.core.startup").load_plugins()

local function restyle(node)
    node:send("StyleController.ThemeChanged", { recursive = true })
    core.flushQueue()
end

for _, palette in ipairs { "", "octopus-dark", "octopus-light" } do
    for _, appearance in ipairs { "Dark", "Light" } do
        local screen = ui.Screen {
            Width = 390, Height = 844, ResizeMode = "NoResize", Appearance = appearance,
            StyleSheet = "Octopus/Styles/octopus.css", class = palette,
        }
        local plain = screen + ui.Button {}
        local row = screen + ui.Button { class = "octopus-row" }
        local reaction = screen + ui.Button { class = "octopus-row reaction-button" }
        core.flushQueue()
        assert(plain.BackgroundColor.A > 0, "Probe: UIKit gives a plain Button a fill")
        for _, state in ipairs { "", "Hovered", "Pressed" } do
            for _, button in ipairs { row, reaction } do
                if state ~= "" then button[state] = true end
                restyle(button)
                assert(button.BackgroundColor.A == 0, string.format(
                    "Row buttons stay clear (palette %q, %s appearance, state %q)", palette, appearance, state))
                if state ~= "" then button[state] = false end
            end
        end
    end
end

print("Octopus row buttons: clear in both appearances, every pointer state, with or without the palette class")
