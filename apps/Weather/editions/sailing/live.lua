-- Is the wind on right now? (SA9, Research/services-sailing.md SV-SA2,
-- SV-SA4 and SV-SA5.) Measured wind near the spot from open services, each
-- an optional source:
--
--   sailing-metar        the airports within about 50 km: METAR from the
--                        NOAA Aviation Weather Center (public domain, keyless)
--   sailing-openwindmap  OpenWindMap / Pioupiou volunteer stations (Community
--                        Licence: free, commercial use with credit; other
--                        data shown beside it must be open too, which NOAA's is)
--   sailing-ndbc-1, -2   a spot's NOAA NDBC shore station and offshore buoy
--                        (public domain, fixed-width text: `format = "text"`)
--
-- `parse` keeps the readings with their place and time; the nearest fresh
-- one is chosen when the card is drawn (Live.readings), so a replayed
-- answer ages honestly and a stale station drops out. Every reading says
-- it was observed, where (name and distance) and how long ago.
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Live = {}

local KN = 0.514444 -- m/s in a knot

-- Base URLs, kept here (not Open-Meteo hosts).
Live.endpoints = {
    metar = "https://aviationweather.gov/api/data/metar",
    openwindmap = "https://api.pioupiou.fr/v1/live-with-meta/",
    ndbc = "https://www.ndbc.noaa.gov/data/realtime2/",
}

-- Per service, how old a reading may be before it is not "live" (minutes:
-- METAR is hourly, OpenWindMap every few minutes, NDBC files lag up to an
-- hour) and how far its station may be (km).
Live.FRESH = { metar = 90, openwindmap = 30, ndbc = 90 }
Live.NEAR = { metar = 60, openwindmap = 15, ndbc = 80 }
-- A volunteer station this close is the spot's own and comes first.
Live.OWN = 10

-- OpenWindMap is dense in France, the Alps and their lakes; elsewhere the
-- whole-network answer is not worth asking for. A place may name its
-- station (`windStation`, OpenWindMap's number) to ask for that one only.
Live.OPENWINDMAP_AREA = { south = 41, north = 51.5, west = -5.5, east = 16.5 }

-- NDBC stations per curated spot, the shore station first. Positions are
-- NDBC's station pages, rounded.
Live.BUOYS = {
    crissy = {
        { id = "FTPC1", name = "Fort Point", kind = "station", lat = 37.806, lon = -122.466 },
        { id = "46026", name = "San Francisco buoy", kind = "buoy", lat = 37.755, lon = -122.839 },
    },
    islamorada = { { id = "MLRF1", name = "Molasses Reef", kind = "station", lat = 25.012, lon = -80.376 } },
}

local function number(value) return type(value) == "number" and value or nil end

local function state(forecast)
    forecast.sailing = forecast.sailing or {}
    return forecast.sailing
end

-- Great-circle distance in km.
function Live.distance(a, b)
    local r, rad = 6371, math.rad
    local dlat, dlon = rad(b.lat - a.lat), rad(b.lon - a.lon)
    local h = math.sin(dlat / 2) ^ 2 + math.cos(rad(a.lat)) * math.cos(rad(b.lat)) * math.sin(dlon / 2) ^ 2
    return 2 * r * math.asin(math.min(1, math.sqrt(h)))
end

-- "2026-11-18T05:56:12.000Z" → UTC epoch.
local epoch = require("Weather/editions/sailing/warnings").epoch

Live.metar = {
    id = "sailing-metar", optional = true,
    url = function(spot)
        if not (spot and spot.lat and spot.lon) then return nil end
        return ("%s?bbox=%.2f,%.2f,%.2f,%.2f&format=json&hours=2"):format(Live.endpoints.metar,
            spot.lat - 0.5, spot.lon - 0.7, spot.lat + 0.5, spot.lon + 0.7)
    end,
    -- A list of reports, several per airport; the latest of each is kept.
    -- Knots; `wdir` is degrees or "VRB".
    parse = function(json, forecast)
        assert(type(json) == "table", "no reports")
        local byId, list = {}, {}
        for _, report in ipairs(json) do
            local at = type(report) == "table" and (number(report.obsTime) or epoch(report.reportTime))
            local lat, lon, speed = at and number(report.lat), at and number(report.lon), at and number(report.wspd)
            local id = at and tostring(report.icaoId or report.name or "")
            if lat and lon and speed and (not byId[id] or byId[id].at < at) then
                byId[id] = { id = id, name = type(report.name) == "string" and report.name:match("^[^,]+") or id,
                    kind = "airport", lat = lat, lon = lon, at = at, speed = speed * KN,
                    gust = number(report.wgst) and report.wgst * KN, dir = number(report.wdir),
                    variable = report.wdir == "VRB" or nil }
            end
        end
        for _, reading in pairs(byId) do list[#list + 1] = reading end
        table.sort(list, function(a, b) return a.id < b.id end)
        state(forecast).metar = list
    end,
}

Live.openwindmap = {
    id = "sailing-openwindmap", optional = true,
    url = function(spot)
        if not (spot and spot.lat and spot.lon) then return nil end
        local own = math.tointeger(tonumber(spot.windStation))
        if own then return Live.endpoints.openwindmap .. own end
        local box = Live.OPENWINDMAP_AREA
        if spot.lat < box.south or spot.lat > box.north or spot.lon < box.west or spot.lon > box.east then return nil end
        return Live.endpoints.openwindmap .. "all"
    end,
    -- `data` is a list (all) or one station; km/h.
    parse = function(json, forecast)
        local data = assert(type(json) == "table" and type(json.data) == "table" and json.data, "no stations")
        if data.id then data = { data } end
        local list = {}
        for _, station in ipairs(data) do
            local where = type(station) == "table" and type(station.location) == "table" and station.location or {}
            local m = type(station) == "table" and type(station.measurements) == "table" and station.measurements or {}
            local status = type(station) == "table" and type(station.status) == "table" and station.status or {}
            local meta = type(station) == "table" and type(station.meta) == "table" and station.meta or {}
            local at, speed = epoch(m.date), number(m.wind_speed_avg)
            if at and speed and number(where.latitude) and number(where.longitude) then
                list[#list + 1] = { id = tostring(station.id), name = type(meta.name) == "string" and meta.name
                    or "station " .. tostring(station.id), kind = "station", lat = where.latitude, lon = where.longitude,
                    at = at, speed = speed / 3.6, gust = number(m.wind_speed_max) and m.wind_speed_max / 3.6,
                    dir = number(m.wind_heading), off = status.state ~= nil and status.state ~= "on" or nil }
            end
        end
        state(forecast).openwindmap = list
    end,
}

-- An NDBC realtime2 file: two header lines (names, units), then the
-- newest row first; "MM" is missing. Times are UTC; wind in m/s.
function Live.ndbcRow(body)
    local names, row
    for line in tostring(body):gmatch("[^\r\n]+") do
        if not names and line:match("^#%s*YY") then
            names = {}
            for name in line:sub(2):gmatch("%S+") do names[#names + 1] = name end
        elseif names and not line:match("^#") and line:match("%S") then
            row = {}
            local index = 0
            for value in line:gmatch("%S+") do
                index = index + 1
                if names[index] then row[names[index]] = value ~= "MM" and tonumber(value) or nil end
            end
            break
        end
    end
    assert(names and row and row.YY and row.MM and row.DD and row.hh, "not an NDBC file")
    local day = Sky.epochOf(("%04d-%02d-%02d"):format(row.YY, row.MM, row.DD))
    return { at = day + row.hh * 3600 + (row.mm or 0) * 60, speed = row.WSPD, gust = row.GST, dir = row.WDIR,
        waveHeight = row.WVHT, wavePeriod = row.DPD, waterTemp = row.WTMP, pressure = row.PRES }
end

local function buoy(n)
    return {
        id = "sailing-ndbc-" .. n, optional = true, format = "text",
        url = function(spot)
            local station = spot and Live.BUOYS[spot.id] and Live.BUOYS[spot.id][n]
            return station and Live.endpoints.ndbc .. station.id .. ".txt" or nil
        end,
        parse = function(body, forecast)
            local sailing = state(forecast)
            sailing.ndbc = sailing.ndbc or {}
            sailing.ndbc[n] = Live.ndbcRow(body)
        end,
    }
end
Live.buoys = { buoy(1), buoy(2) }

Live.sources = { Live.metar, Live.openwindmap, Live.buoys[1], Live.buoys[2] }

-- Every fresh reading near the spot, the one to trust first: a volunteer
-- station within Live.OWN km, then the spot's NDBC stations in order,
-- then the rest by distance. Each is { kind, name, km, at, age (minutes),
-- speed, gust, dir, variable, waveHeight, wavePeriod, service }, and the
-- second list holds the stale ones.
function Live.readings(forecast, spot, now)
    local sailing = forecast.sailing or {}
    now = now or forecast.epoch or os.time()
    spot = spot or forecast.spot or {}
    local fresh, stale = {}, {}
    local function consider(reading, meta, service, order)
        local where = meta or reading
        if not (spot.lat and where.lat and reading.at and reading.speed) then return end
        local kind = where.kind or reading.kind
        local entry = setmetatable({ kind = kind, name = where.name, service = service, order = order,
            km = Live.distance(spot, where), age = math.max(0, (now - reading.at) / 60) }, { __index = reading })
        if entry.km > Live.NEAR[service] or reading.off then return end
        if entry.age > Live.FRESH[service] then stale[#stale + 1] = entry else fresh[#fresh + 1] = entry end
    end
    for _, reading in ipairs(sailing.openwindmap or {}) do consider(reading, nil, "openwindmap", 3) end
    for n, station in ipairs(Live.BUOYS[spot.id] or {}) do
        local reading = sailing.ndbc and sailing.ndbc[n]
        if reading then consider(reading, station, "ndbc", 2) end
    end
    for _, reading in ipairs(sailing.metar or {}) do consider(reading, nil, "metar", 3) end
    for _, entry in ipairs(fresh) do
        if entry.service == "openwindmap" and entry.km <= Live.OWN then entry.order = 1 end
    end
    local function before(a, b)
        if a.order ~= b.order then return a.order < b.order end
        return a.km < b.km
    end
    table.sort(fresh, before)
    table.sort(stale, function(a, b) return a.age < b.age end)
    return fresh, stale
end

-- Which live sources a spot asks for, and whether any answered or failed.
function Live.status(forecast, spot)
    local asked, failed = false, false
    local sailing, missing = forecast.sailing or {}, forecast.missing or {}
    for _, source in ipairs(Live.sources) do
        if source.url(spot or {}) then
            asked = true
            if missing[source.id] then failed = true end
        end
    end
    local any = sailing.metar or sailing.openwindmap or sailing.ndbc
    return asked, any ~= nil, failed
end

local WORDS = { airport = "airport", station = "wind station", buoy = "buoy" }
local CREDIT = { metar = "METAR via NOAA", openwindmap = "OpenWindMap", ndbc = "NOAA NDBC" }

-- "18 min ago", "2 h ago".
function Live.age(minutes)
    if minutes < 2 then return "just now" end
    if minutes < 90 then return ("%d min ago"):format(math.floor(minutes + 0.5)) end
    return ("%d h ago"):format(math.floor(minutes / 60 + 0.5))
end

local function wind(reading)
    local dir = reading.variable and "variable" or reading.dir and Sky.compass(reading.dir) or ""
    return Units.gust(reading.speed, reading.gust) .. (dir ~= "" and " " .. dir or "")
end
Live.wind = wind

-- "airport, 6.2 km away, 18 min ago".
function Live.where(reading)
    return ("%s, %s away, %s"):format(WORDS[reading.kind] or reading.kind, Units.distance(reading.km), Live.age(reading.age))
end

-- The forecast hour a reading falls in.
local function forecastHour(forecast, at)
    for _, hour in ipairs(forecast.hours or {}) do
        if hour.epoch and at >= hour.epoch and at < hour.epoch + 3600 then return hour end
    end
    return forecast.hours and forecast.hours[forecast.now]
end

-- "forecast 12 g 18 kn: running 2 kn above".
function Live.against(forecast, reading)
    local hour = forecastHour(forecast, reading.at)
    if not (hour and hour.wind) then return nil end
    local diff = Units.value("wind", reading.speed) - Units.value("wind", hour.wind)
    local unit = Units.label("wind")
    local word = math.abs(diff) < 1.5 and "close to it"
        or ("running %d %s %s"):format(math.floor(math.abs(diff) + 0.5), unit, diff > 0 and "above" or "below")
    return ("forecast %s: %s"):format(Units.gust(hour.wind, hour.gust), word)
end

-- The live card, and a buoy card when the offshore buoy reads waves.
-- Nothing when no live source covers the spot.
function Live.cards(forecast, spot, now)
    local asked, answered, failed = Live.status(forecast, spot)
    if not asked then return {} end
    local fresh, stale = Live.readings(forecast, spot, now)
    local cards, first = {}, fresh[1]
    if first then
        local parts = { Live.where(first) }
        local against = Live.against(forecast, first)
        if against then parts[#parts + 1] = against end
        local other = fresh[2]
        if other then parts[#parts + 1] = ("%s %s, %s"):format(other.name, wind(other), Units.distance(other.km)) end
        parts[#parts + 1] = CREDIT[first.service]
        cards[1] = { id = "live", title = "observed now · " .. first.name, value = wind(first), tone = "info", rank = 3,
            detail = table.concat(parts, " · ") }
    elseif answered or failed then
        local last = stale[1]
        cards[1] = { id = "live", title = "observed now", value = "no fresh reading", tone = "info", hub = false,
            detail = last and ("last: %s %s, %s · the forecast alone"):format(last.name, wind(last), Live.where(last))
                or failed and not answered and "the stations did not answer: the forecast alone"
                or "no station reporting near here: the forecast alone" }
    end
    for _, reading in ipairs(fresh) do
        if reading.kind == "buoy" and reading.waveHeight then
            cards[#cards + 1] = { id = "buoy", title = "buoy · " .. reading.name, tone = "info", hub = false,
                value = Units.height(reading.waveHeight) .. (reading.wavePeriod and (" at %d s"):format(math.floor(reading.wavePeriod + 0.5)) or ""),
                detail = ("wind %s · %s · NOAA NDBC"):format(wind(reading), Live.where(reading)) }
            break
        end
    end
    return cards
end

return Live
