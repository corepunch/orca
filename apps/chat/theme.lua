-- Metro palette after Messenger on Windows Phone: black or white canvas,
-- Messenger blue, solid tile colours, the phone chrome and its pale text
-- boxes. Surfaces are flat, so `surface` is transparent. The OS appearance picks the
-- variant; CHAT_APPEARANCE=light|dark overrides it. Views take their colours from
-- the matching CSS roles in Styles/app.css, so an appearance change restyles them.
local system = require "orca.system"

local tiles = {
    { background = "#0084ff", foreground = "#ffffff" }, { background = "#00aba9", foreground = "#ffffff" },
    { background = "#a200ff", foreground = "#ffffff" }, { background = "#f09609", foreground = "#ffffff" },
}

local palettes = {
    dark = {
        background = "#000000", surface = "#00000000", raised = "#2e2e2e", chrome = "#1f1f1f",
        textbox = "#bfbfbf", textboxInk = "#000000", placeholder = "#666666",
        ink = "#ffffff", muted = "#a6a6a6", dim = "#999999", faint = "#5c5c5c",
        line = "#3a3a3a", accent = "#0084ff", accentText = "#41beec", onAccent = "#ffffff",
        soft = "#0a2a4d", sent = "#0084ff", sentText = "#ffffff", sentMeta = "#cfe6ff",
        error = "#ff5c5c", avatar = tiles,
    },
    light = {
        background = "#ffffff", surface = "#00000000", raised = "#e6e6e6", chrome = "#dddddd",
        textbox = "#d9d9d9", textboxInk = "#000000", placeholder = "#6e6e6e",
        ink = "#000000", muted = "#555555", dim = "#7a7a7a", faint = "#b8b8b8",
        line = "#cccccc", accent = "#0078e7", accentText = "#006ad1", onAccent = "#ffffff",
        soft = "#dcebff", sent = "#0078e7", sentText = "#ffffff", sentMeta = "#d4e9ff",
        error = "#e51400", avatar = tiles,
    },
}

-- Views retain this table, so update its values rather than replacing it.
local colors = {}
function colors.refresh()
    local appearance = os.getenv("CHAT_APPEARANCE")
    if appearance ~= "light" and appearance ~= "dark" then appearance = system.is_dark_theme() and "dark" or "light" end
    if colors.appearance == appearance then return false end
    for key, value in pairs(palettes[appearance]) do colors[key] = value end
    colors.appearance = appearance
    return true
end
colors.refresh()
return colors
