-- Small helpers the service modules share: time stamps, coverage boxes,
-- GeoJSON point-in-polygon and the reader's language. No requests here.
local Sky = require "Weather/sky"

local Util = {}

-- "2026-11-18T14:00:00-08:00", "…Z", "…+0100" or "…:00.000Z" → UTC epoch.
-- nil without a zone: a local time with no offset is not guessed.
function Util.isoEpoch(text)
    if type(text) ~= "string" then return nil end
    local date, h, m, rest = text:match("^(%d+%-%d+%-%d+)[T ](%d+):(%d+)(.*)$")
    local base = date and Sky.epochOf(date)
    if not base then return nil end
    local s = tonumber(rest:match("^:(%d+)")) or 0
    rest = rest:gsub("^:%d+", ""):gsub("^%.%d+", "")
    local offset
    if rest == "Z" or rest == "z" then offset = 0
    else
        local sign, oh, om = rest:match("^([%+%-])(%d%d):?(%d%d)$")
        if not sign then return nil end
        offset = (tonumber(oh) * 3600 + tonumber(om) * 60) * (sign == "-" and -1 or 1)
    end
    return base + tonumber(h) * 3600 + tonumber(m) * 60 + s - offset
end

-- Whether the spot lies in one of `boxes` ({ south, north, west, east }).
function Util.inBoxes(spot, boxes)
    if not (spot and spot.lat and spot.lon) then return false end
    for _, box in ipairs(boxes or {}) do
        if spot.lat >= box.south and spot.lat <= box.north and spot.lon >= box.west and spot.lon <= box.east then
            return true
        end
    end
    return false
end

-- The spot's ISO country code ("DE"), when the place has one (search,
-- "use my location"). Curated places have none.
function Util.country(spot)
    local code = spot and spot.country
    return type(code) == "string" and code:match("^%a%a$") and code:upper() or nil
end

-- Ray casting over one ring of [lon, lat] pairs.
local function inRing(ring, x, y)
    local inside, j = false, #ring
    for i = 1, #ring do
        local xi, yi, xj, yj = ring[i][1], ring[i][2], ring[j][1], ring[j][2]
        if ((yi > y) ~= (yj > y)) and x < (xj - xi) * (y - yi) / (yj - yi) + xi then inside = not inside end
        j = i
    end
    return inside
end
local function inPolygon(rings, x, y)
    if not (rings and rings[1] and inRing(rings[1], x, y)) then return false end
    for index = 2, #rings do if inRing(rings[index], x, y) then return false end end -- holes
    return true
end

-- Whether a GeoJSON geometry (Polygon or MultiPolygon) holds the point.
function Util.contains(geometry, lon, lat)
    if type(geometry) ~= "table" or type(geometry.coordinates) ~= "table" then return false end
    if geometry.type == "Polygon" then return inPolygon(geometry.coordinates, lon, lat) end
    if geometry.type == "MultiPolygon" then
        for _, polygon in ipairs(geometry.coordinates) do
            if inPolygon(polygon, lon, lat) then return true end
        end
    end
    return false
end

-- The reader's language, two letters ("fr"), or nil.
function Util.language()
    local ok, core = pcall(require, "orca.core")
    local tag = ok and type(core) == "table" and core.getPreferredLanguage and core.getPreferredLanguage()
    if type(tag) ~= "string" or tag == "" then tag = os.getenv("LANG") or "" end
    return tag:lower():match("^(%a%a)")
end

-- "Moderate" from "moderate"; nil stays nil.
function Util.capital(word)
    return type(word) == "string" and word:sub(1, 1):upper() .. word:sub(2):lower() or nil
end

return Util
