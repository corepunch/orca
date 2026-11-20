-- The northern (and southern) lights: NOAA SWPC's planetary Kp forecast
-- (public domain, three-hour steps, about three days ahead) against the
-- place's geomagnetic latitude. Kp alone sends people out under cloud and
-- in daylight, so the stars activity only speaks of it on a dark night,
-- and says when clouds are in the way. All of it is an estimate.
local Sky = require "Weather/sky"
local Aurora = {}

Aurora.URL = "https://services.swpc.noaa.gov/products/noaa-planetary-k-index-forecast.json"
-- The geomagnetic north pole of the dipole field (IGRF, 2025).
Aurora.POLE = { lat = 80.8, lon = -72.6 }

-- Latitude in the Earth's magnetic frame (a dipole: good to a degree or two).
function Aurora.magneticLatitude(lat, lon)
    local r = math.pi / 180
    local p = Aurora.POLE
    local s = math.sin(lat * r) * math.sin(p.lat * r) + math.cos(lat * r) * math.cos(p.lat * r) * math.cos((lon - p.lon) * r)
    return math.asin(math.max(-1, math.min(1, s))) / r
end

-- The magnetic latitude where the oval's edge stands overhead at a Kp,
-- after NOAA's Kp table (66.5° at Kp 0 down to 48° at Kp 9).
function Aurora.edge(kp) return 66.5 - 2.04 * kp end

-- "overhead", "low" (low over the poleward horizon, up to about four
-- degrees beyond the edge) or nil, for a place and a Kp.
function Aurora.reach(lat, lon, kp)
    if not kp then return nil end
    local m, edge = math.abs(Aurora.magneticLatitude(lat, lon)), Aurora.edge(kp)
    if m >= edge then return "overhead" end
    if m >= edge - 4 then return "low" end
    return nil
end

-- "2026-11-22T18:00:00" or "2026-11-22 18:00:00" (UTC) → epoch.
local function epochOf(tag)
    local start, h = Sky.epochOf(tag), tonumber(tag:match("[T ](%d%d):"))
    return start and h and start + h * 3600
end

-- The Kp steps from NOAA's answer, as { { epoch, kp, observed }, … }. The
-- feed is a list of objects; it used to be a list of rows under a header
-- row, which is read too.
function Aurora.parseKp(json)
    local steps = {}
    if type(json) ~= "table" then return steps end
    for index, entry in ipairs(json) do
        local tag, kp, observed
        if type(entry) == "table" and entry.time_tag then tag, kp, observed = entry.time_tag, entry.kp, entry.observed
        elseif type(entry) == "table" and index > 1 then tag, kp, observed = entry[1], entry[2], entry[3] end
        kp = tonumber(kp)
        local epoch = type(tag) == "string" and epochOf(tag)
        if epoch and kp then steps[#steps + 1] = { epoch = epoch, kp = kp, observed = observed == "observed" } end
    end
    return steps
end

-- The Kp for the three hours holding an epoch, or nil past the feed's end.
function Aurora.kpAt(steps, epoch)
    for _, step in ipairs(steps or {}) do
        if epoch >= step.epoch and epoch < step.epoch + 3 * 3600 then return step.kp end
    end
end

Aurora.source = {
    id = "kp", optional = true,
    url = function() return Aurora.URL end,
    parse = function(json, forecast)
        local steps = Aurora.parseKp(json)
        forecast.kp = #steps > 0 and steps or nil
        for _, hour in ipairs(forecast.hours) do
            if hour.epoch then hour.kp = Aurora.kpAt(steps, hour.epoch + 1800) end
        end
    end,
}

return Aurora
