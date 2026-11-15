-- Chat list rows (contacts, settings and Me commands) are layout, never filled
-- controls: UIKit's default Button fill must not show behind them in either
-- appearance, in a pointer state, or without the palette class.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
local ui = require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/chat")
require("orca.core.startup").load_plugins()

for _, palette in ipairs { "", "chat-dark", "chat-light" } do
    for _, appearance in ipairs { "Dark", "Light" } do
        local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize", Appearance = appearance,
            StyleSheet = "Chat/Styles/app.css", class = palette }
        local plain = screen + ui.Button { class = "chat-action" }
        local row = screen + ui.Button { class = "chat-action chat-row" }
        core.flushQueue()
        if palette == "" and appearance == "Dark" then
            assert(plain.BackgroundColor.A > 0, "Probe: a classed Button without a background rule gets UIKit's fill")
        end
        for _, state in ipairs { "", "Hovered", "Pressed" } do
            if state ~= "" then row[state] = true end
            row:send("StyleController.ThemeChanged", { recursive = true })
            core.flushQueue()
            assert(row.BackgroundColor.A == 0, string.format(
                "Chat rows stay clear (palette %q, %s appearance, state %q)", palette, appearance, state))
            if state ~= "" then row[state] = false end
        end
    end
end
-- The contact row template carries the row class.
local contact = assert(fs.loadObject("Chat/Templates/ContactRow"))
local tag = fs.serializeObjectToString(contact):match("<[%w_:][^>]*>") or ""
assert((" " .. (tag:match('%sclass="([^"]*)"') or "") .. " "):find(" chat-row ", 1, true), "Contact rows use chat-row")
print("Chat rows: clear in both appearances and every pointer state; contact rows use them")
