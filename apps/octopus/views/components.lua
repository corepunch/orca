local ui = require "orca.UIKit"
local Format = require "Octopus/format"
local C = { colors = require "Octopus/theme" }
-- A 4-point spacing scale shared by post lists and cards. Lists keep
-- Twitter's density: posts sit 12 points apart below their action row.
C.spacing = { tight = 4, inline = 8, column = 12, section = 16, post = 12 }
-- Avatar tile in timelines and people lists.
C.AVATAR = 44
local colors = C.colors
local auto = 0/0
-- ORCA draws a border outside Width/Height (it counts as margin), so bordered
-- controls subtract it to keep every control on the same 48-point grid.
local BORDER = 2
local ICONS = "Octopus/assets/icons/"

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

function C.row(props, children)
    props = props or {}
    props.Direction = "Horizontal"
    props.AlignItems = props.AlignItems or "Center"
    return C.stack(props, children)
end

function C.clear(node)
    while node:getFirstChild() do node:getFirstChild():removeFromParent() end
    return node
end

-- Reserve content space while data is pending; the indicator never captures input.
function C.loading(props)
    props = props or {}
    local name = props.Name or "Loading"
    props.IgnoreHitTest = true
    local area = ui.Grid(props)
    area:addChild(ui.ActivityIndicator { Name = name .. "Indicator", Foreground = colors.dim })
    return area
end

function C.icon(name, size, color, props)
    props = props or {}
    props.Source, props.Width, props.Height = ICONS .. name .. ".svg", size or 20, size or 20
    props.Foreground, props.IgnoreHitTest = color or colors.muted, true
    if props.MarginLeft == nil then props.MarginLeft, props.MarginRight = auto, auto end
    props.MarginTop, props.MarginBottom = auto, auto
    return ui.VectorView(props)
end

-- The octopus mark from the website, tinted like an icon.
function C.logo(size, color)
    return ui.VectorView { Source = "Octopus/assets/octopus.svg", Width = size, Height = size,
        Foreground = color or colors.onAccent, IgnoreHitTest = true,
        MarginLeft = auto, MarginRight = auto, MarginTop = auto, MarginBottom = auto }
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
        props.BorderWidth, props.BorderColor = tostring(BORDER), colors.ink
        props.Height = props.Height - 2 * BORDER
    end
    props.TextHorizontalAlignment = props.TextHorizontalAlignment or (link and "Left" or "Center")
    props.TextVerticalAlignment = "Center"
    props.Click, props.class = action, "octopus-action"
    return ui.Button(props, label)
end

function C.primary(label, action, props)
    props = props or {}
    props.Background, props.Foreground = colors.accent, colors.onAccent
    return C.button(label, action, props)
end

function C.outline(label, action, props)
    props = props or {}
    props.Background = colors.surface
    return C.button(label, action, props)
end

-- A transparent button that hosts arbitrary content, such as a post body.
function C.area(content, action, props)
    props = props or {}
    props.Background, props.Click, props.class = props.Background or "#00000000", action, props.class or "octopus-row"
    local button = ui.Button(props)
    button:addChild(content)
    return button
end

-- Windows Phone text box: pale fill and dark text in both themes.
function C.input(name, placeholder, props)
    props = props or {}
    props.Name, props.PlaceholderText = name, placeholder
    props.Height, props.FontSize = (props.Height or 48) - 2 * BORDER, props.FontSize or 15
    props.Padding = props.Padding or (props.Multiline and "12 10" or "12 0")
    props.Background = props.Background or colors.textbox
    props.Foreground, props.PlaceholderColor = colors.textboxInk, colors.placeholder
    props.BorderWidth, props.BorderColor = tostring(BORDER), props.Background
    props.class = "octopus-input"
    return ui.Input(props)
end

-- Group header, as in a Windows Phone long list: lowercase in the accent.
function C.eyebrow(value, props)
    props = props or {}
    props.FontSize, props.Foreground = props.FontSize or 20, colors.accentText
    props.MarginTop = props.MarginTop or 8
    return C.text(value:lower(), props)
end

-- A Windows Phone page: the application title and a large lowercase page
-- name above the content. A locked Pivot with one item gives every page the
-- same metrics as the timeline pivot. Scrolling content runs under the frosted
-- header; other content stays below it unless the page asks otherwise.
-- Pages you go into pass `back`: a round back button before the title, which
-- the title line and a swipe in from the left edge also trigger.
function C.page(header, content, props)
    props = props or {}
    local extend = props.ExtendBodyBehindHeader
    if extend == nil then extend = content.OverflowY == "Scroll" end
    local pivot = ui.Pivot { Name = props.Name, Title = "Octopus", IsLocked = true, Foreground = colors.ink,
        ExtendBodyBehindHeader = extend, class = props.class,
        BackIcon = props.back and ICONS .. "arrow-left.svg" or "", BackRequested = props.back }
    local item = ui.PivotItem { Header = header }
    item:addChild(content)
    pivot:addChild(item)
    return pivot
end

-- An ApplicationBar from { name, icon, text, action } buttons and
-- { name, text, action } menu items. It docks to the bottom of Footer.layout as
-- a frosted bar that extends under the home indicator; pages scroll under it.
function C.appBar(buttons, menu)
    local bar = ui.ApplicationBar { Name = "AppBar", class = "frosted bar-top", Foreground = colors.ink,
        SafeAreaEdge = "Bottom", ContentInsetAdjustment = "Always" }
    for _, edge in ipairs { "Left", "Right" } do
        bar:attachPropertyProgram("Node.Padding" .. edge, "{Screen.SafeArea" .. edge .. "}", "OneWay", true)
    end
    for _, b in ipairs(buttons) do
        bar:addChild(ui.ApplicationBarIconButton { Name = b[1], IconUri = ICONS .. b[2] .. ".svg", Text = b[3], Click = b[4] })
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

-- An empty or error state: large muted lowercase line and a short hint.
function C.empty(title, hint, props)
    props = props or {}
    props.Spacing, props.Padding = 8, props.Padding or "0 0 24 0"
    return C.stack(props, {
        C.text(title, { FontSize = 22, Foreground = colors.muted, WordWrap = true }),
        hint and C.text(hint, { FontSize = 14, Foreground = colors.dim, WordWrap = true }) or nil,
    })
end

-- Text helpers live in Octopus/format so the data provider shares them.
for _, name in ipairs { "plain", "excerpt", "domain", "epoch", "ago", "stamp", "count" } do C[name] = Format[name] end

return C
