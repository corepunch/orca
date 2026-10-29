-- Text helpers shared by views and the data provider. Pure functions; no UI.
local Format = {}
-- Plain text for a phone card: drop the commonest Markdown markers.
function Format.plain(value)
    value = tostring(value or "")
    value = value:gsub("!%[([^%]]*)%]%([^%)]*%)", "%1"):gsub("%[([^%]]+)%]%([^%)]*%)", "%1")
    value = value:gsub("```[^\n]*\n", ""):gsub("```", ""):gsub("`", "")
    value = value:gsub("%*%*(.-)%*%*", "%1"):gsub("__(.-)__", "%1")
    value = value:gsub("\n#+%s*", "\n"):gsub("^#+%s*", ""):gsub("\n>%s?", "\n"):gsub("^>%s?", "")
    value = value:gsub("\r", ""):gsub("\n\n\n+", "\n\n")
    return (value:gsub("^%s+", ""):gsub("%s+$", ""))
end

function Format.excerpt(value, limit)
    value = Format.plain(value):gsub("%s+", " ")
    limit = limit or 220
    if #value <= limit then return value end
    -- Cut on a UTF-8 boundary and a word where possible.
    local cut = value:sub(1, limit)
    while #cut > 0 and cut:byte(-1) >= 128 and cut:byte(-1) < 192 do cut = cut:sub(1, -2) end
    if #cut > 0 and cut:byte(-1) >= 192 then cut = cut:sub(1, -2) end
    cut = cut:match("^(.*%S)%s+%S*$") or cut
    return cut .. "…"
end

function Format.domain(url)
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
Format.epoch = epoch

-- "now", "5m", "3h", "2d", then a short date, like a timeline.
function Format.ago(iso, now)
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
function Format.stamp(iso)
    local time = epoch(iso)
    if not time then return "" end
    return (os.date("%H:%M · %d %b %Y", time):gsub("· 0", "· "):lower())
end

function Format.count(value, singular, plural)
    value = value or 0
    return value .. " " .. (value == 1 and singular or (plural or singular .. "s"))
end


local function tone(key)
    local hash = 5
    for i = 1, #key do hash = (hash * 31 + key:byte(i)) % 2147483647 end
    return hash
end

-- The avatar tile's colour slot (1-based) for a user ID.
function Format.tone(key, count)
    return tone(key or "") % count + 1
end

function Format.initials(name)
    local initials = (name or ""):match("^%s*([%w])") or "O"
    local last = (name or ""):match("%s+([%w])[%w%.]*%s*$")
    return (initials .. (last or "")):upper()
end

return Format
