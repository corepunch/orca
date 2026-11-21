-- What the sea is doing now (SV-SU2, SV-SU3): the nearest NOAA buoy's
-- last six hours, and the water temperature measured at a NOAA tide
-- station. Both are public domain and keyless. Each break names its
-- stations in breaks.lua (`buoy`, `coops`); a break without one asks
-- nothing. A buoy that is late or silent says so: buoys go quiet for days.
--
-- The buoy comes from NOAA CoastWatch's ERDDAP copy of the NDBC feed as
-- JSON, six hours at a time (a few kilobytes). NDBC's own realtime2 text
-- files hold 45 days (about half a megabyte), and answers are saved with
-- the forecast, so they are not used.
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Score = require "Weather/score"
local Breaks = require "Weather/editions/surf/breaks"
local Buoys = {
    ERDDAP = "https://coastwatch.pfeg.noaa.gov/erddap/tabledap/cwwcNDBCMet.json",
    STALE = 3 * 3600, -- older than this, a reading is not "now"
    WATER_STALE = 6 * 3600,
}

-- "2026-11-18T05:50:00Z" or "2026-11-18 05:54" (UTC) → epoch, or nil.
local function utc(text)
    local date, h, m = tostring(text or ""):match("^(%d+%-%d+%-%d+)[T ](%d+):(%d+)")
    local base = date and Sky.epochOf(date)
    return base and base + tonumber(h) * 3600 + tonumber(m) * 60
end
Buoys.utc = utc

-- ERDDAP writes a missing value as null, NaN or a string.
local function number(value)
    value = tonumber(value)
    return value and value == value and value or nil
end

Buoys.source = {
    id = "surf-buoy", optional = true,
    url = function(spot)
        local station = Breaks.of(spot).buoy
        if not station then return nil end
        return ("%s?station,time,wvht,dpd,apd,mwd,wtmp&station=%%22%s%%22&time%%3E=now-6hours")
            :format(Buoys.ERDDAP, station)
    end,
    parse = function(json, forecast) forecast.buoy = Buoys.parse(json) end,
}

-- ERDDAP's table as readings, oldest first: { station, epoch, height,
-- period, average, direction, water }. Raises without a table.
function Buoys.parse(json)
    local tbl = assert(type(json) == "table" and type(json.table) == "table" and json.table, "no buoy table")
    local names, rows = tbl.columnNames, tbl.rows
    assert(type(names) == "table" and type(rows) == "table", "no buoy table")
    local column = {}
    for index, name in ipairs(names) do column[name] = index end
    local readings = {}
    for _, row in ipairs(rows) do
        local function at(name) return column[name] and row[column[name]] end
        local epoch = utc(at("time"))
        if epoch then
            readings[#readings + 1] = { station = at("station"), epoch = epoch, height = number(at("wvht")),
                period = number(at("dpd")), average = number(at("apd")), direction = number(at("mwd")),
                water = number(at("wtmp")) }
        end
    end
    table.sort(readings, function(a, b) return a.epoch < b.epoch end)
    return { readings = readings }
end

-- The last reading with a wave height, or nil.
function Buoys.latest(buoy, field)
    local readings = buoy and buoy.readings or {}
    for index = #readings, 1, -1 do
        if readings[index][field or "height"] then return readings[index] end
    end
end

-- "23 min ago", "4 h ago".
local function ago(seconds)
    if seconds < 3600 then return math.max(0, math.floor(seconds / 60 + 0.5)) .. " min ago" end
    return math.floor(seconds / 3600 + 0.5) .. " h ago"
end
Buoys.ago = ago

-- A new swell shows first as a longer period: the period now against the
-- shortest of the readings three or more hours earlier.
function Buoys.arriving(buoy, last)
    local before
    for _, reading in ipairs(buoy.readings) do
        if reading.period and reading.epoch <= last.epoch - 3 * 3600 then
            before = before and math.min(before, reading.period) or reading.period
        end
    end
    if before and last.period and last.period >= before + 3 then return before end
end

-- The buoy card. A break without a buoy has none; a buoy that failed,
-- is silent or late says so calmly, on the activity page only.
function Buoys.card(forecast)
    local station = Breaks.of(forecast.spot).buoy
    if not station then return nil end
    local title = "buoy " .. station
    local credit = "NOAA National Data Buoy Center"
    local function quiet(value, why)
        return { id = "buoy", title = title, value = value, tone = "info", hub = false, detail = why .. " · " .. credit }
    end
    if not forecast.buoy then
        if forecast.missing and forecast.missing["surf-buoy"] then
            return quiet("no reading", "the buoy feed is not answering; the forecast above stands without it")
        end
        return nil
    end
    local last = Buoys.latest(forecast.buoy)
    if not last then return quiet("no reading", "no wave reading in the last 6 hours: the buoy may be down") end
    local age = (forecast.epoch or last.epoch) - last.epoch
    if age > Buoys.STALE then
        return quiet("no recent reading", ("last wave reading %s: too old to say what the sea does now"):format(ago(age)))
    end
    local face = Units.height(last.height, 1) .. " · " .. Score.fmt(last.period, " s")
    local parts = { ago(age) .. (last.direction and ", from " .. Sky.compass(last.direction) or "") }
    local now = forecast.hours and forecast.hours[forecast.now]
    local model = now and (now.waveHeight or now.swellHeight)
    if model then
        local delta = last.height - model
        parts[#parts + 1] = math.abs(delta) < 0.2 and "as forecast"
            or ("%s %s than forecast"):format(Units.height(math.abs(delta), 1), delta > 0 and "bigger" or "smaller")
    end
    local before = Buoys.arriving(forecast.buoy, last)
    if before then parts[#parts + 1] = ("swell arriving: the period rose from %s to %s"):format(
        Score.fmt(before, " s"), Score.fmt(last.period, " s")) end
    parts[#parts + 1] = credit
    return { id = "buoy", title = title, value = face, tone = "info", detail = table.concat(parts, " · ") }
end

-- Measured water temperature (SV-SU3) from a NOAA CO-OPS station.
Buoys.waterSource = {
    id = "surf-water", optional = true,
    url = function(spot, Forecast)
        local station = Breaks.of(spot).coops
        if not station then return nil end
        return Forecast.url("noaaTides", ("product=water_temperature&application=weather_app&date=latest"
            .. "&station=%s&units=metric&time_zone=gmt&format=json"):format(station))
    end,
    -- CO-OPS answers { metadata, data = { { t, v } } }, or { error = … }.
    parse = function(json, forecast)
        assert(type(json) == "table" and not json.error, "the station has no water temperature")
        local first = type(json.data) == "table" and json.data[1]
        local value, epoch = first and number(first.v), first and utc(first.t)
        assert(value and epoch, "no water temperature reading")
        local meta = type(json.metadata) == "table" and json.metadata or {}
        forecast.waterTemp = { value = value, epoch = epoch, station = meta.name or meta.id }
    end,
}

-- The water temperature for an hour: measured (`forecast.waterTemp`, then
-- the buoy) when fresh and the hour is now, else the model's sea surface.
-- Returns the value and where it comes from: "measured" or "model".
function Buoys.water(hour, forecast)
    local now = forecast and forecast.epoch
    local current = now and hour.epoch and math.abs(hour.epoch - now) < 3600
    if current then
        local measured = forecast.waterTemp
        if measured and now - measured.epoch <= Buoys.WATER_STALE then
            return measured.value, "measured", measured.station
        end
        local reading = Buoys.latest(forecast.buoy, "water")
        if reading and now - reading.epoch <= Buoys.WATER_STALE then
            return reading.water, "measured", "buoy " .. (reading.station or "")
        end
    end
    return hour.seaTemp, hour.seaTemp and "model"
end

Buoys.attribution = {
    { "Buoy data: NOAA National Data Buoy Center", "Live wave readings via NOAA CoastWatch ERDDAP, public domain." },
    { "Water temperature: NOAA CO-OPS", "Measured at NOAA tide stations, tidesandcurrents.noaa.gov, public domain." },
}

return Buoys
