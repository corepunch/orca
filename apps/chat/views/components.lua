local ui = require "orca.UIKit"
local C = { colors = require "Chat/theme" }
local colors = C.colors
local auto = 0/0

function C.text(value, props)
    props = props or {}
    props.FontSize = props.FontSize or 15
    props.Foreground = props.Foreground or colors.ink
    props.IgnoreHitTest = true
    return ui.TextBlock(props, value)
end

function C.stack(props, children)
    props = props or {}
    props.JustifyContent = props.JustifyContent or "Start"
    local node = ui.StackView(props)
    for _, child in ipairs(children or {}) do node:addChild(child) end
    return node
end

function C.icon(name, size, color)
    return ui.VectorView {
        Source = "Chat/assets/icons/" .. name .. ".svg",
        Width = size or 20, Height = size or 20, Foreground = color or colors.muted,
        MarginLeft = auto, MarginRight = auto, MarginTop = auto, MarginBottom = auto,
        IgnoreHitTest = true,
    }
end

function C.iconTile(name, size, iconSize)
    local tile = ui.Grid { Width = size, Height = size, Background = colors.accent }
    tile:addChild(C.icon(name, iconSize, colors.onAccent))
    return tile
end

function C.button(label, action, props)
    props = props or {}
    props.Height = props.Height or 44
    props.FontSize = props.FontSize or 14
    props.FontWeight = props.FontWeight or "Bold"
    props.Foreground = props.Foreground or colors.accentText
    props.Background = props.Background or "#00000000"
    props.TextHorizontalAlignment, props.TextVerticalAlignment = "Center", "Center"
    props.Click, props.class = action, "chat-action"
    return ui.Button(props, label)
end

function C.iconButton(name, action, props)
    props = props or {}
    local color = props.Foreground or colors.muted
    props.Width, props.Height = props.Width or 44, props.Height or 44
    local button = C.button("", action, props)
    button:addChild(C.icon(name, 20, color))
    return button
end

function C.input(name, placeholder, props)
    props = props or {}
    props.Name, props.PlaceholderText = name, placeholder
    props.Height, props.FontSize = props.Height or 50, props.FontSize or 14
    props.Padding = props.Padding or "16 0"
    props.Background = props.Background or colors.surface
    props.Foreground, props.PlaceholderColor = colors.ink, colors.dim
    props.BorderWidth, props.BorderColor = "2", colors.line
    props.class = "chat-input"
    return ui.Input(props)
end

function C.avatar(name, size, key)
    local hash = 0
    for i = 1, #(key or name) do hash = hash + (key or name):byte(i) * i end
    local tone = colors.avatar[hash % #colors.avatar + 1]
    local initials = name:match("^([%w])") or "C"
    local last = name:match("%s+([%w])[%w]*$")
    if last then initials = initials .. last end
    return C.text(initials:upper(), {
        Width = size or 52, Height = size or 52, FontSize = (size or 52) < 40 and 12 or 17,
        FontWeight = "Bold",
        Background = tone.background, Foreground = tone.foreground,
        TextHorizontalAlignment = "Center", TextVerticalAlignment = "Center",
    })
end

function C.badge(value, props)
    props = props or {}
    props.Width, props.Height = props.Width or 24, props.Height or 24
    props.Background, props.Foreground = props.Background or colors.accent, props.Foreground or colors.onAccent
    props.FontSize, props.FontWeight = 11, "Bold"
    props.TextHorizontalAlignment, props.TextVerticalAlignment = "Center", "Center"
    return C.text(tostring(value), props)
end

function C.eyebrow(value)
    return C.text(value:lower(), { FontSize = 15, Foreground = colors.accentText })
end

-- Metro app title: small capitals above the page content.
function C.brand()
    return C.text("MESSENGER", { FontSize = 13, FontWeight = "Bold", LetterSpacing = 1.2, Height = 32,
        TextVerticalAlignment = "Center" })
end

function C.navItem(label, icon, key)
    local button = ui.Tab {
        Name = "Tab" .. key, Value = key, Height = 58, class = "chat-tab",
        SelectedColor = colors.surface, UnselectedColor = colors.surface,
    }
    local grid = ui.Grid { Rows = "32px 16px", Padding = "2 8", Spacing = 2, IgnoreHitTest = true }
    -- App-bar button: an outlined circle around the glyph.
    local ring = ui.Grid { Name = "Tab" .. key .. "Ring", Width = 32, Height = 32, BorderRadius = "16", BorderWidth = "2",
        BorderColor = colors.muted, MarginLeft = auto, MarginRight = auto, IgnoreHitTest = true }
    local image = C.icon(icon, 16, colors.muted)
    image.Name = "Tab" .. key .. "Icon"
    ring:addChild(image)
    grid:addChild(ring)
    grid:addChild(C.text(label:lower(), {
        Name = "Tab" .. key .. "Label", FontSize = 10, FontWeight = "Bold",
        Foreground = colors.muted, TextHorizontalAlignment = "Center",
    }))
    button:addChild(grid)
    return button
end

-- Speech-bubble tail, tinted like the bubble it joins.
function C.tail(up, color, mine)
    return ui.VectorView {
        Source = "Chat/assets/bubble-tail-" .. (up and "up" or "down") .. ".svg", Width = 14, Height = 12,
        Foreground = color, MarginLeft = mine and auto or 16, MarginRight = mine and 16 or auto, IgnoreHitTest = true,
    }
end

function C.time(iso)
    return type(iso) == "string" and iso:match("T(%d%d:%d%d)") or ""
end

function C.date(iso)
    local y, m, d = (iso or ""):match("^(%d%d%d%d)%-(%d%d)%-(%d%d)")
    if not y then return "CONVERSATION" end
    return os.date("%a, %d %b", os.time { year = tonumber(y), month = tonumber(m), day = tonumber(d), hour = 12 })
end

return C
