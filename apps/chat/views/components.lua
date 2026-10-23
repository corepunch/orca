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

function C.iconTile(name, size, iconSize, radius)
    local tile = ui.Grid { Width = size, Height = size, Background = colors.soft, BorderRadius = tostring(radius or 20) }
    tile:addChild(C.icon(name, iconSize, colors.accentText))
    return tile
end

function C.button(label, action, props)
    props = props or {}
    props.Height = props.Height or 44
    props.FontSize = props.FontSize or 14
    props.FontWeight = props.FontWeight or "Bold"
    props.Foreground = props.Foreground or colors.accentText
    props.Background = props.Background or "#00000000"
    props.BorderRadius = props.BorderRadius or "14"
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
    props.BorderRadius, props.BorderWidth, props.BorderColor = "16", "1", colors.line
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
        FontWeight = "Bold", BorderRadius = tostring((size or 52) / 2),
        Background = tone.background, Foreground = tone.foreground,
        TextHorizontalAlignment = "Center", TextVerticalAlignment = "Center",
    })
end

function C.badge(value, props)
    props = props or {}
    props.Width, props.Height = props.Width or 24, props.Height or 24
    props.Background, props.Foreground = props.Background or colors.accent, props.Foreground or "white"
    props.FontSize, props.FontWeight, props.BorderRadius = 11, "Bold", "12"
    props.TextHorizontalAlignment, props.TextVerticalAlignment = "Center", "Center"
    return C.text(tostring(value), props)
end

function C.eyebrow(value)
    return C.text(value, { FontSize = 10, FontWeight = "Bold", Foreground = colors.dim, LetterSpacing = 1.7 })
end

function C.brand()
    return C.stack({ Direction = "Horizontal", AlignItems = "Center", Spacing = 9, Height = 32 }, {
        C.iconTile("message-circle", 30, 17, 10),
        C.text("orca", { FontSize = 17, FontWeight = "Bold", LetterSpacing = 0.5 }),
        C.text("/ messenger", { FontSize = 12, Foreground = colors.dim }),
    })
end

function C.navItem(label, icon, action, active)
    local color = active and colors.accentText or colors.dim
    local button = C.button("", action, { Height = 58, Background = active and colors.soft or "#00000000" })
    local grid = ui.Grid { Rows = "24px 18px", Padding = "8 8", Spacing = 2, IgnoreHitTest = true }
    grid:addChild(C.icon(icon, 20, color))
    grid:addChild(C.text(label, { FontSize = 10, FontWeight = "Bold", Foreground = color, TextHorizontalAlignment = "Center" }))
    button:addChild(grid)
    return button
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
