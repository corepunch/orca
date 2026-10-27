local ui = require "orca.UIKit"
local C = { colors = require "Octopus/theme" }
-- A 4-point spacing scale shared by post lists and cards.
C.spacing = { tight = 4, inline = 8, column = 12, section = 16, post = 24 }
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

local function tone(key)
    local hash = 5
    for i = 1, #key do hash = (hash * 31 + key:byte(i)) % 2147483647 end
    return colors.avatar[hash % #colors.avatar + 1]
end

function C.initials(name)
    local initials = (name or ""):match("^%s*([%w])") or "O"
    local last = (name or ""):match("%s+([%w])[%w%.]*%s*$")
    return (initials .. (last or "")):upper()
end

-- Square Metro avatar tile: the person's initials until `app` has their
-- profile photo. `key` is their user ID.
function C.avatar(name, size, key, app)
    local color = tone(key or name or "")
    size = size or 52
    local tile = ui.Grid { Width = size, Height = size, Background = color.background, IgnoreHitTest = true }
    tile:addChild(C.text(C.initials(name), {
        FontSize = size < 40 and 13 or (size > 80 and 34 or 19), Foreground = color.foreground,
        TextHorizontalAlignment = "Center", TextVerticalAlignment = "Center",
    }))
    if app and key then app:avatarPhoto(tile, key) end
    return tile
end

-- Replace an avatar tile's initials with a photo file. White keeps the
-- photo's own colours under the theme's ink.
function C.photo(tile, path)
    C.clear(tile):addChild(ui.ImageView { Name = "AvatarPhoto", Src = path, Foreground = "#ffffff",
        Stretch = "UniformToFill", IgnoreHitTest = true })
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
-- same metrics as the timeline pivot.
function C.page(header, content, props)
    props = props or {}
    local pivot = ui.Pivot { Name = props.Name, Title = "Octopus", IsLocked = true, Foreground = colors.ink }
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
        bar:addChild(ui.ApplicationBarIconButton { Name = b[1], IconUri = ICONS .. b[2] .. ".svg", Text = b[3], Click = b[4] })
    end
    for _, m in ipairs(menu or {}) do bar:addChild(ui.ApplicationBarMenuItem { Name = m[1], Text = m[2], Click = m[3] }) end
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

-- Text helpers ----------------------------------------------------------------

-- Plain text for a phone card: drop the commonest Markdown markers.
function C.plain(value)
    value = tostring(value or "")
    value = value:gsub("!%[([^%]]*)%]%([^%)]*%)", "%1"):gsub("%[([^%]]+)%]%([^%)]*%)", "%1")
    value = value:gsub("```[^\n]*\n", ""):gsub("```", ""):gsub("`", "")
    value = value:gsub("%*%*(.-)%*%*", "%1"):gsub("__(.-)__", "%1")
    value = value:gsub("\n#+%s*", "\n"):gsub("^#+%s*", ""):gsub("\n>%s?", "\n"):gsub("^>%s?", "")
    value = value:gsub("\r", ""):gsub("\n\n\n+", "\n\n")
    return (value:gsub("^%s+", ""):gsub("%s+$", ""))
end

function C.excerpt(value, limit)
    value = C.plain(value):gsub("%s+", " ")
    limit = limit or 220
    if #value <= limit then return value end
    -- Cut on a UTF-8 boundary and a word where possible.
    local cut = value:sub(1, limit)
    while #cut > 0 and cut:byte(-1) >= 128 and cut:byte(-1) < 192 do cut = cut:sub(1, -2) end
    if #cut > 0 and cut:byte(-1) >= 192 then cut = cut:sub(1, -2) end
    cut = cut:match("^(.*%S)%s+%S*$") or cut
    return cut .. "…"
end

function C.domain(url)
    return tostring(url or ""):match("^%a+://([^/:?#]+)") or tostring(url or "")
end

local function epoch(iso)
    local y, m, d, h, min, s = tostring(iso or ""):match("^(%d+)%-(%d+)%-(%d+)T(%d+):(%d+):(%d+)")
    if not y then return nil end
    local localTime = os.time { year = tonumber(y), month = tonumber(m), day = tonumber(d),
        hour = tonumber(h), min = tonumber(min), sec = tonumber(s) }
    -- os.time reads the fields as local time; correct to UTC.
    local now = os.time()
    local offset = os.time(os.date("*t", now)) - os.time(os.date("!*t", now))
    return localTime + offset
end
C.epoch = epoch

-- "now", "5m", "3h", "2d", then a short date, like a timeline.
function C.ago(iso, now)
    local time = epoch(iso)
    if not time then return "" end
    local diff = math.max(0, (now or os.time()) - time)
    if diff < 60 then return "now" end
    if diff < 3600 then return math.floor(diff / 60) .. "m" end
    if diff < 86400 then return math.floor(diff / 3600) .. "h" end
    if diff < 7 * 86400 then return math.floor(diff / 86400) .. "d" end
    return os.date("!%d %b", time):gsub("^0", ""):lower()
end

-- The full local time of a post, as on a tweet's own page: "14:05 · 24 oct 2026".
function C.stamp(iso)
    local time = epoch(iso)
    if not time then return "" end
    return (os.date("%H:%M · %d %b %Y", time):gsub("· 0", "· "):lower())
end

function C.count(value, singular, plural)
    value = value or 0
    return value .. " " .. (value == 1 and singular or (plural or singular .. "s"))
end

return C
