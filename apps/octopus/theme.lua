-- Metro palette for Octopus: a black or white canvas, the website's reef teal
-- as the phone accent, solid avatar tiles and pale Windows Phone text boxes.
-- The OS appearance picks the variant; OCTOPUS_APPEARANCE=light|dark overrides it.
local system = require "orca.system"

local tiles = {
    { background = "#0e8f86", foreground = "#ffffff" }, { background = "#1ba1e2", foreground = "#ffffff" },
    { background = "#a200ff", foreground = "#ffffff" }, { background = "#f09609", foreground = "#ffffff" },
    { background = "#d80073", foreground = "#ffffff" }, { background = "#60a917", foreground = "#ffffff" },
}

local palettes = {
    dark = {
        background = "#000000", surface = "#00000000", raised = "#1f1f1f", chrome = "#1f1f1f",
        textbox = "#bfbfbf", textboxInk = "#000000", placeholder = "#666666",
        ink = "#ffffff", muted = "#a6a6a6", dim = "#8a8a8a", faint = "#3a3a3a",
        accent = "#0e8f86", accentText = "#2cc5b8", onAccent = "#ffffff", soft = "#06302d",
        liked = "#ff4f6d", reposted = "#7ed321", error = "#ff5c5c", avatar = tiles,
    },
    light = {
        background = "#ffffff", surface = "#00000000", raised = "#ececec", chrome = "#dddddd",
        textbox = "#d9d9d9", textboxInk = "#000000", placeholder = "#6e6e6e",
        ink = "#000000", muted = "#555555", dim = "#767676", faint = "#cccccc",
        accent = "#0e8f86", accentText = "#0a7068", onAccent = "#ffffff", soft = "#d5f0ee",
        liked = "#d80032", reposted = "#3c8a00", error = "#e51400", avatar = tiles,
    },
}

local appearance = os.getenv("OCTOPUS_APPEARANCE")
if appearance ~= "light" and appearance ~= "dark" then appearance = system.is_dark_theme() and "dark" or "light" end
return palettes[appearance]
