local ui = require "orca.UIKit"
local C = { colors = require "Chat/theme", brandBackground = "#7930ff" }
local colors = C.colors
local auto = 0/0
-- ORCA draws a border outside Width/Height (it counts as margin), so bordered
-- controls subtract it to keep every control on the same 48-point grid.
local BORDER = 2
-- Lucide draws its check across two thirds of the icon: at the box's own 32
-- points it spans about 70% of the box, near the Windows Phone check's size.
-- `check-bold` widens its stroke to match the phone's heavy mark.
local CHECK_MARK = 32

-- Palette colours come from CSS roles (Styles/app.css) under the Screen's
-- chat-light or chat-dark, so an appearance change restyles built views.
-- `role` names one, such as "chat-muted"; a fixed colour stays a property.
local function addClass(props, role)
    props.class = props.class and props.class .. " " .. role or role
end
C.addClass = addClass

function C.text(value, props)
    props = props or {}
    props.FontSize = props.FontSize or 15
    if not props.Foreground and not props.class then props.class = "chat-ink" end
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

-- `tint` is a role class, or a fixed colour such as "#ffffff".
function C.icon(name, size, tint)
    tint = tint or "chat-muted"
    local fixed = tint:sub(1, 1) == "#"
    return ui.VectorView {
        Source = "Chat/assets/icons/" .. name .. ".svg",
        Width = size or 20, Height = size or 20, Foreground = fixed and tint or nil, class = not fixed and tint or nil,
        MarginLeft = auto, MarginRight = auto, MarginTop = auto, MarginBottom = auto,
        IgnoreHitTest = true,
    }
end

function C.appIcon(size)
    return ui.ImageView {
        Src = "Chat/assets/app-icon.png", Width = size, Height = size,
        Stretch = "Uniform", Foreground = "#ffffff", IgnoreHitTest = true,
    }
end

-- Windows Phone buttons: no background means a hyperlink, the "chat-primary"
-- role fills a primary command, and anything else is the outlined 2-pixel
-- button. A role class in `props.class` replaces the default colours.
function C.button(label, action, props)
    props = props or {}
    local role = props.class
    local primary = role == "chat-primary"
    local link = props.Background == nil and not primary
    props.Height = props.Height or 48
    props.FontSize = props.FontSize or 15
    props.FontWeight = props.FontWeight or (link and "Normal" or "Bold")
    props.Background = props.Background or "#00000000"
    if not link and not primary and props.BorderWidth == nil then
        props.BorderWidth = tostring(BORDER)
        props.Height = props.Height - 2 * BORDER
        role = role or "chat-outline"
    end
    props.TextHorizontalAlignment = props.TextHorizontalAlignment or "Center"
    props.TextVerticalAlignment = "Center"
    -- A fixed Foreground (as on photos) keeps its colour instead of a role.
    role = role or not props.Foreground and (link and "chat-link" or "chat-ink")
    props.Click, props.class = action, role and "chat-action " .. role or "chat-action"
    return ui.Button(props, label)
end

-- `tint` is the icon's role class or fixed colour (C.icon).
function C.iconButton(name, action, props, tint)
    props = props or {}
    props.Width, props.Height = props.Width or 44, props.Height or 44
    props.BorderWidth = props.BorderWidth or "0"
    local button = C.button("", action, props)
    button:addChild(C.icon(name, 20, tint))
    return button
end

-- Windows Phone text box: pale fill and dark text in both themes.
function C.input(name, placeholder, props)
    props = props or {}
    props.Name, props.PlaceholderText = name, placeholder
    props.Height, props.FontSize = (props.Height or 48) - 2 * BORDER, props.FontSize or 15
    props.Padding = props.Padding or "12 0"
    props.BorderWidth = tostring(BORDER)
    props.class = "chat-input chat-field" .. (props.class and " " .. props.class or "")
    return ui.Input(props)
end

-- A command row after Telegram's New Message: an accent outline icon in the
-- avatar column and an accent label, so commands read apart from people.
function C.command(name, icon, label, action)
    local button = C.button("", action, { Name = name, Height = 56, BorderWidth = "0", FontWeight = "Normal",
        Background = colors.surface, class = "chat-row" })
    local grid = ui.Grid { Columns = "52px 1fr", Spacing = 14, IgnoreHitTest = true }
    grid:addChild(C.icon(icon, 26, "chat-accent-text"))
    grid:addChild(C.text(label, { FontSize = 20, class = "chat-accent-text", TextVerticalAlignment = "Center",
        TextOverflow = "Ellipsis" }))
    button:addChild(grid)
    return button
end

-- Windows Phone check box: a 32-point square in the text box fill with a dark
-- check filling most of it, its label to the right; the whole row toggles.
-- `changed(checked)` receives the new state; the check is named `name .. "Check"`.
function C.checkbox(name, label, checked, changed)
    checked = checked == true
    local mark = C.icon("check-bold", CHECK_MARK, "chat-field-ink")
    mark.Name, mark.Visible = name .. "Check", checked
    local row = C.button("", function()
        checked = not checked
        mark.Visible = checked
        if changed then changed(checked) end
    end, { Name = name, BorderWidth = "0", FontWeight = "Normal", Background = colors.surface, class = "chat-row" })
    -- A Grid centres the mark; a StackView would pin it to the top-left.
    local box = ui.Grid { Width = 32, Height = 32, MarginTop = auto, MarginBottom = auto,
        IgnoreHitTest = true, class = "chat-field chat-check" }
    box:addChild(mark)
    local grid = ui.Grid { Columns = "32px 1fr", Spacing = 14, IgnoreHitTest = true }
    grid:addChild(box)
    grid:addChild(C.text(label, { FontSize = 17, TextVerticalAlignment = "Center", TextOverflow = "Ellipsis" }))
    row:addChild(grid)
    return row
end

-- Initials on a tile colour until `app` has the person's profile photo.
-- `userId` is given only where the tile stands for one person.
function C.avatar(name, size, key, app, userId)
    local hash = 0
    for i = 1, #(key or name) do hash = hash + (key or name):byte(i) * i end
    local tone = colors.avatar[hash % #colors.avatar + 1]
    local initials = name:match("^([%w])") or "C"
    local last = name:match("%s+([%w])[%w]*$")
    if last then initials = initials .. last end
    size = size or 52
    local tile = ui.Grid { Width = size, Height = size, Background = tone.background, class = "avatar", IgnoreHitTest = true }
    tile:addChild(C.text(initials:upper(), {
        FontSize = size < 40 and 12 or 18, Foreground = tone.foreground,
        TextHorizontalAlignment = "Center", TextVerticalAlignment = "Center",
    }))
    if app and userId then app:avatarPhoto(tile, userId) end
    return tile
end

-- Replace an avatar tile's initials with a photo file. White keeps the
-- photo's own colours under the light theme's ink.
function C.photo(tile, path)
    while tile:getFirstChild() do tile:getFirstChild():removeFromParent() end
    tile:addChild(ui.ImageView { Name = "AvatarPhoto", Src = path, Foreground = "#ffffff",
        Stretch = "UniformToFill", class = "avatar", IgnoreHitTest = true })
end

function C.badge(value, props)
    props = props or {}
    props.Width, props.Height = props.Width or 24, props.Height or 24
    addClass(props, "chat-badge")
    props.FontSize, props.FontWeight = 11, "Bold"
    props.TextHorizontalAlignment, props.TextVerticalAlignment = "Center", "Center"
    return C.text(tostring(value), props)
end

-- Group header, as in a Windows Phone long list: lowercase in the accent.
function C.eyebrow(value)
    return C.text(value:lower(), { FontSize = 18, class = "chat-accent-text", MarginTop = 8 })
end

-- A Windows Phone page: the application title and a large lowercase page
-- name above the content. A locked Pivot with one item gives every page the
-- same metrics as the inbox pivot. Scrolling content runs under the frosted
-- header; other content stays below it unless the page asks otherwise.
-- Pages you go into pass `back`: a round back button before the title, which
-- the title line and a swipe in from the left edge also trigger. `details`
-- runs when the page name is tapped.
function C.page(header, content, props)
    props = props or {}
    local extend = props.ExtendBodyBehindHeader
    if extend == nil then extend = content.OverflowY == "Scroll" end
    local pivot = ui.Pivot { Name = props.Name, Title = props.Title or "Messenger", IsLocked = true,
        ExtendBodyBehindHeader = extend, class = props.class and props.class .. " chat-ink" or "chat-ink",
        BackIcon = props.back and "Chat/assets/icons/arrow-left.svg" or "", BackRequested = props.back,
        HeaderClick = props.details }
    local item = ui.PivotItem { Header = header }
    item:addChild(content)
    pivot:addChild(item)
    return pivot
end

-- An ApplicationBar from { name, icon, text, action } buttons and
-- { name, text, action } menu items. It docks to the bottom of Footer.layout as
-- a frosted bar that extends under the home indicator; pages scroll under it.
function C.appBar(buttons, menu)
    local bar = ui.ApplicationBar { Name = "AppBar", class = "frosted bar-top chat-ink",
        SafeAreaEdge = "Bottom", ContentInsetAdjustment = "Always" }
    for _, edge in ipairs { "Left", "Right" } do
        bar:attachPropertyProgram("Node.Padding" .. edge, "{Screen.SafeArea" .. edge .. "}", "OneWay", true)
    end
    for _, b in ipairs(buttons) do
        bar:addChild(ui.ApplicationBarIconButton {
            Name = b[1], IconUri = "Chat/assets/icons/" .. b[2] .. ".svg", Text = b[3], Click = b[4],
        })
    end
    for _, m in ipairs(menu or {}) do bar:addChild(ui.ApplicationBarMenuItem { Name = m[1], Text = m[2], Click = m[3] }) end
    return bar
end

-- A frosted toolbar docked to the "Top" or "Bottom" of a page body. Inside a
-- page it spans the horizontal margins, so the frost meets the screen edges.
function C.toolbar(edge, children, props)
    props = props or {}
    props.SafeAreaEdge, props.class = edge, edge == "Top" and "frosted bar-bottom" or "frosted bar-top"
    props.MarginLeft, props.MarginRight = -20, -20
    props.Padding = props.Padding or "20 0"
    local bar = ui.Node2D(props)
    for _, child in ipairs(children) do bar:addChild(child) end
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
