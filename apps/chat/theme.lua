-- Metro palette after Messenger on Windows Phone: black or white canvas,
-- Messenger blue, and solid tile colours. The OS appearance picks the
-- variant; CHAT_APPEARANCE=light|dark overrides it.
local system = require "orca.system"

local tiles = {
    { background = "#0084ff", foreground = "#ffffff" }, { background = "#00aba9", foreground = "#ffffff" },
    { background = "#a200ff", foreground = "#ffffff" }, { background = "#f09609", foreground = "#ffffff" },
}

local palettes = {
    dark = {
        background = "#000000", surface = "#1f1f1f", raised = "#2e2e2e",
        ink = "#ffffff", muted = "#a6a6a6", dim = "#8a8a8a", faint = "#5c5c5c",
        line = "#3a3a3a", accent = "#0084ff", accentText = "#3d9eff", onAccent = "#ffffff",
        soft = "#0a2a4d", sent = "#0084ff", sentText = "#ffffff", sentMeta = "#cfe6ff",
        error = "#ff5c5c", avatar = tiles,
    },
    light = {
        background = "#ffffff", surface = "#e9e9e9", raised = "#e6e6e6",
        ink = "#000000", muted = "#555555", dim = "#7a7a7a", faint = "#b8b8b8",
        line = "#cccccc", accent = "#0078e7", accentText = "#006ad1", onAccent = "#ffffff",
        soft = "#dcebff", sent = "#0078e7", sentText = "#ffffff", sentMeta = "#d4e9ff",
        error = "#e51400", avatar = tiles,
    },
}

local appearance = os.getenv("CHAT_APPEARANCE")
if appearance ~= "light" and appearance ~= "dark" then appearance = system.is_dark_theme() and "dark" or "light" end
return palettes[appearance]
