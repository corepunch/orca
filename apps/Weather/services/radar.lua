-- A radar-backed "rain nearby" line (SV-W5) from Deutscher Wetterdienst's
-- RADOLAN RV composite through Bright Sky (`/radar`, 5 minutes, 1 km
-- cells): keyless JSON, DWD open data with a source credit. Only Germany
-- and its margins have it; elsewhere `url` is nil and nothing is said.
--
-- It reads where rain is now, never when it arrives: nowcasting stays the
-- model's job (the next-hour card's quarter hours).
--
--   Radar.source              a `sources` entry (id "radar-dwd", optional)
--   Radar.covers(spot)        whether the composite reaches the spot
--   Radar.line(forecast, now) "radar: rain about 8 km away, moving closer",
--                             "radar: dry within 15 km", or nil
--   Radar.attribution         { title, detail } for an activity's credit
--
-- Shape read (Bright Sky /radar with `format=plain`; tested on a synthetic
-- fixture): `radar[]` frames `{ timestamp, precipitation_5 = rows of
-- cells }`, precipitation in 0.01 mm per 5 minutes; `bbox` [top, left,
-- bottom, right] and `latlon_position` { x, y } in grid cells. Without a
-- usable position the grid's centre is the spot.
local Util = require "Weather/services/util"

local Radar = {}

Radar.RADIUS = 15000 -- metres asked around the spot
Radar.WET = 4 -- 0.04 mm in 5 minutes, about 0.5 mm an hour
Radar.FRESH = 30 * 60 -- an older frame says nothing

-- The RADOLAN composite's reach: Germany and its margins.
Radar.boxes = { { south = 46.5, north = 55.5, west = 3.5, east = 16 } }
-- Neighbours inside it (the Netherlands, Denmark, …) have radar too.
function Radar.covers(spot) return Util.inBoxes(spot, Radar.boxes) end

Radar.attribution = { "Radar: Deutscher Wetterdienst via Bright Sky",
    "Source: Deutscher Wetterdienst (RADOLAN RV), open data with source credit; served by brightsky.dev." }

-- The nearest wet cell's distance in km (0 at the spot), or nil when dry.
local function nearest(frame, px, py)
    local best
    for r, row in ipairs(frame.precipitation_5 or {}) do
        for c, value in ipairs(row) do
            if type(value) == "number" and value >= Radar.WET then
                local d = math.sqrt((c - 1 - px) ^ 2 + (r - 1 - py) ^ 2)
                if not best or d < best then best = d end
            end
        end
    end
    return best
end

function Radar.read(json)
    local frames = assert(type(json.radar) == "table" and json.radar, "no radar frames")
    local list = {}
    for _, frame in ipairs(frames) do
        local at = Util.isoEpoch(frame.timestamp)
        if at and type(frame.precipitation_5) == "table" and #frame.precipitation_5 > 0 then
            list[#list + 1] = { at = at, frame = frame }
        end
    end
    assert(#list > 0, "no radar frames")
    table.sort(list, function(a, b) return a.at < b.at end)
    local rows, cols = #list[1].frame.precipitation_5, #list[1].frame.precipitation_5[1]
    local pos, box = json.latlon_position, json.bbox
    local px, py = (cols - 1) / 2, (rows - 1) / 2
    if type(pos) == "table" and type(box) == "table" and tonumber(pos.x) and tonumber(box[2]) then
        local x, y = pos.x - box[2], pos.y - box[1]
        if x >= 0 and x <= cols - 1 and y >= 0 and y <= rows - 1 then px, py = x, y end
    end
    local frames = {}
    for _, item in ipairs(list) do frames[#frames + 1] = { at = item.at, nearest = nearest(item.frame, px, py) } end
    return { frames = frames, within = math.floor(math.min(px, py, cols - 1 - px, rows - 1 - py)) }
end

Radar.source = {
    id = "radar-dwd", optional = true,
    url = function(spot)
        if not Radar.covers(spot) then return nil end
        return ("https://api.brightsky.dev/radar?lat=%.4f&lon=%.4f&distance=%d&format=plain")
            :format(spot.lat, spot.lon, Radar.RADIUS)
    end,
    parse = function(json, forecast) forecast.radar = Radar.read(json) end,
}

-- The line for the next-hour card, from the latest frame up to `now` and
-- one about half an hour before it.
function Radar.line(forecast, now)
    local radar = forecast and forecast.radar
    if not radar then return nil end
    now = now or forecast.epoch
    local latest, earlier
    for _, frame in ipairs(radar.frames) do
        if frame.at <= now then latest = frame end
    end
    if not latest or now - latest.at > Radar.FRESH then return nil end
    for _, frame in ipairs(radar.frames) do
        if frame.at <= latest.at - 15 * 60 then earlier = frame end
    end
    if not latest.nearest then return ("radar: dry within %d km"):format(radar.within) end
    if latest.nearest < 1.5 then return "radar: rain here now" end
    local km = math.floor(latest.nearest + 0.5)
    local trend = ""
    if earlier and earlier.nearest then
        if earlier.nearest - latest.nearest >= 2 then trend = ", moving closer"
        elseif latest.nearest - earlier.nearest >= 2 then trend = ", moving away" end
    end
    return ("radar: rain about %d km away%s"):format(km, trend)
end

return Radar
