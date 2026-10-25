local ui = require "orca.UIKit"
local C = { colors = require "Chat/theme" }
local colors = C.colors
local auto = 0/0

function C.text(value, props)
    props = props or {}
    props.FontSize = props.FontSize or 15
    props.Foreground = props.Foreground or colors.ink
    props.IgnoreHitTest = true
    -- Ellipsis applies to a single line; text otherwise wraps by default.
    if props.TextOverflow == "Ellipsis" and not props.WordWrap then props.TextWrapping = "NoWrap" end
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

-- Windows Phone buttons: no background means a hyperlink, the accent fills a
-- primary command, and anything else is the outlined 2-pixel button.
function C.button(label, action, props)
    props = props or {}
    local link, primary = props.Background == nil, props.Background == colors.accent
    props.Height = props.Height or 48
    props.FontSize = props.FontSize or 15
    props.FontWeight = props.FontWeight or (link and "Normal" or "Bold")
    props.Foreground = props.Foreground or (link and colors.accentText or primary and colors.onAccent or colors.ink)
    props.Background = props.Background or "#00000000"
    if not link and not primary and props.BorderWidth == nil then
        props.BorderWidth, props.BorderColor = "2", colors.ink
    end
    props.TextHorizontalAlignment = props.TextHorizontalAlignment or "Center"
    props.TextVerticalAlignment = "Center"
    props.Click, props.class = action, "chat-action"
    return ui.Button(props, label)
end

function C.iconButton(name, action, props)
    props = props or {}
    local color = props.Foreground or colors.muted
    props.Width, props.Height = props.Width or 44, props.Height or 44
    props.BorderWidth = props.BorderWidth or "0"
    local button = C.button("", action, props)
    button:addChild(C.icon(name, 20, color))
    return button
end

-- Windows Phone text box: pale fill and dark text in both themes.
function C.input(name, placeholder, props)
    props = props or {}
    props.Name, props.PlaceholderText = name, placeholder
    props.Height, props.FontSize = props.Height or 48, props.FontSize or 15
    props.Padding = props.Padding or "12 0"
    props.Background = props.Background or colors.textbox
    props.Foreground, props.PlaceholderColor = colors.textboxInk, colors.placeholder
    props.BorderWidth, props.BorderColor = "2", props.Background
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
        Width = size or 52, Height = size or 52, FontSize = (size or 52) < 40 and 12 or 18,
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

-- Group header, as in a Windows Phone long list: lowercase in the accent.
function C.eyebrow(value)
    return C.text(value:lower(), { FontSize = 18, Foreground = colors.accentText, MarginTop = 8 })
end

-- A Windows Phone page: the application title and a large lowercase page
-- name above the content. A locked Pivot with one item gives every page the
-- same metrics as the inbox pivot.
function C.page(header, content, props)
    props = props or {}
    local pivot = ui.Pivot { Name = props.Name, Title = props.Title or "Messenger", IsLocked = true, Foreground = colors.ink }
    local item = ui.PivotItem { Header = header }
    item:addChild(content)
    pivot:addChild(item)
    return pivot
end

-- An ApplicationBar from { name, icon, text, action } buttons and
-- { name, text, action } menu items.
function C.appBar(buttons, menu)
    local bar = ui.ApplicationBar { Name = "AppBar", Background = colors.chrome, Foreground = colors.ink }
    for _, b in ipairs(buttons) do
        bar:addChild(ui.ApplicationBarIconButton {
            Name = b[1], IconUri = "Chat/assets/icons/" .. b[2] .. ".svg", Text = b[3], Click = b[4],
        })
    end
    for _, m in ipairs(menu or {}) do bar:addChild(ui.ApplicationBarMenuItem { Name = m[1], Text = m[2], Click = m[3] }) end
    return bar
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
