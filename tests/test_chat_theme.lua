-- Contrast is checked after native CSS matching, including a retained theme
-- change and real pointer hover, rather than against a second palette table.
local orca = require "orca"
orca.init()
local core, fs = require "orca.core", require "orca.filesystem"
local ui, system = require "orca.UIKit", require "orca.system"
fs.init("apps/chat")
require("orca.core.startup").load_plugins()

local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize",
    StyleSheet = "Chat/Styles/app.css", StateClasses = "chat-light" }
local canvas = screen + ui.Node2D { class = "chat-background", IgnoreHitTest = true }
local dim = canvas + ui.TextBlock { Text = "preview", class = "chat-dim" }
local muted = canvas + ui.TextBlock { Text = "secondary", class = "chat-muted" }
local link = canvas + ui.TextBlock { Text = "command", class = "chat-link" }
local preview = canvas + ui.TextBlock { Text = "last message", class = "room-preview" }
local field = screen + ui.Input { class = "chat-field", PlaceholderText = "search", IgnoreHitTest = true }
local primary = screen + ui.Button { Text = "Continue", Height = 48, class = "chat-action chat-primary" }
local pointerSink = screen + ui.Button { Height = 48, MarginTop = 796 }
local badge = canvas + ui.Node2D { class = "chat-badge" }
local function bubble(state)
    local row = canvas + ui.Node2D { class = state }
    local fill = row + ui.Node2D { class = "bubble" }
    return { fill = fill,
        text = fill + ui.TextBlock { Text = "message", class = "bubble-ink" },
        time = fill + ui.TextBlock { Text = "09:41", class = "bubble-meta" } }
end
local sent, received = bubble("sent"), bubble("received")
local function luminance(color)
    local function linear(value)
        return value <= 0.04045 and value / 12.92 or ((value + 0.055) / 1.055) ^ 2.4
    end
    return 0.2126 * linear(color.R) + 0.7152 * linear(color.G) + 0.0722 * linear(color.B)
end
local function contrast(foreground, background, name)
    local a, b = luminance(foreground), luminance(background)
    local ratio = (math.max(a, b) + 0.05) / (math.min(a, b) + 0.05)
    assert(ratio >= 4.5, string.format("%s contrast %.2f:1 is below 4.5:1", name, ratio))
end
local function matches(color, hex, name)
    for i, channel in ipairs { "R", "G", "B" } do
        local expected = tonumber(hex:sub(i * 2, i * 2 + 1), 16) / 255
        assert(math.abs(color[channel] - expected) < 0.001, string.format("%s %s=%.6f differs from %s %.6f", name, channel, color[channel], hex, expected))
    end
end
local function restyle()
    screen:send("StyleController.ThemeChanged", { recursive = true })
    core.flushQueue()
end
local function hover(x, y)
    screen:UpdateLayout(390, 844)
    system.dispatchMessage { target = screen, message = "MouseMoved", x = x, y = y }
    core.advanceFrame()
    core.flushQueue()
end

local originalGetenv = os.getenv
local selected
os.getenv = function(name) return name == "CHAT_APPEARANCE" and selected or originalGetenv(name) end
local colors = require "Chat/theme"
for _, appearance in ipairs { "light", "dark", "light" } do
    selected = appearance
    colors.refresh()
    screen.StateClasses = "chat-" .. appearance
    restyle()
    hover(389, 843)
    assert(pointerSink.hover and not primary.hover, "Pointer exit must clear primary hover")
    local background = canvas.BackgroundColor
    contrast(primary.ForegroundColor, primary.BackgroundColor, appearance .. " primary label")
    contrast(badge.ForegroundColor, badge.BackgroundColor, appearance .. " badge")
    contrast(sent.text.ForegroundColor, sent.fill.BackgroundColor, appearance .. " sent text")
    contrast(sent.time.ForegroundColor, sent.fill.BackgroundColor, appearance .. " sent time")
    contrast(received.text.ForegroundColor, received.fill.BackgroundColor, appearance .. " received text")
    contrast(received.time.ForegroundColor, received.fill.BackgroundColor, appearance .. " received time")
    contrast(dim.ForegroundColor, background, appearance .. " dim text")
    contrast(muted.ForegroundColor, background, appearance .. " secondary text")
    contrast(preview.ForegroundColor, background, appearance .. " room preview")
    contrast(link.ForegroundColor, background, appearance .. " link")
    contrast(field.PlaceholderColor, field.BackgroundColor, appearance .. " placeholder")
    matches(primary.BackgroundColor, colors.accent, "primary fill")
    matches(sent.fill.BackgroundColor, colors.sent, "sent fill")
    matches(sent.time.ForegroundColor, colors.sentMeta, "sent time")
    matches(muted.ForegroundColor, colors.muted, "secondary text")
    matches(dim.ForegroundColor, colors.dim, "dim text")
    matches(field.PlaceholderColor, colors.placeholder, "placeholder")
    if appearance == "dark" then matches(muted.ForegroundColor, "#999999", "dark secondary text") end
    hover(20, 20)
    assert(primary.hover, "Pointer routing must reach the primary button")
    assert(primary.Opacity == 1, "Primary hover must not fade its label into the canvas")
    contrast(primary.ForegroundColor, primary.BackgroundColor, appearance .. " hovered primary label")
end
os.getenv = originalGetenv
colors.refresh()
print("Chat palette contrast, hover and retained theme changes passed.")
