-- How warm the water is (FI1), from the best source a place has:
--
--   1. the angler's own thermometer reading (the log keeps one per place
--      for three days);
--   2. a service's fresh measurement: the river gauge, the NOAA station or
--      the Great Lake's average (Water.measured);
--   3. on a coast, the sea surface from Open-Meteo's marine model;
--   4. on a big lake, Open-Meteo's `soil_temperature_0cm`, which is the
--      water surface where the model's grid cell is water. A cell that is
--      really land swings with day and night, so a 24-hour swing of more
--      than 5° means "no estimate here" rather than a wrong number.
--
-- Each answer says where it came from, so the page never claims a lake
-- temperature it does not have.
local Water = {}

Water.READING_SECONDS = 3 * 24 * 3600
Water.MEASURED_SECONDS = 2 * 24 * 3600 -- the hours a measurement stands for
local LAND_SWING = 5
local cache = setmetatable({}, { __mode = "k" })

-- The day's lake estimate around an hour: the mean of the 24 hours centred
-- on it, or nil when the cell behaves like land.
local function lakeEstimate(forecast, hour)
    local hours = forecast.hours or {}
    local memo = cache[forecast]
    if not memo then memo = { index = {}, at = {} }; cache[forecast] = memo
        for index, h in ipairs(hours) do memo.index[h] = index end
    end
    local index = memo.index[hour]
    if not index then return hour.soilTemp, 0 end
    if memo.at[index] ~= nil then return memo.at[index] or nil end
    local low, high, sum, n = math.huge, -math.huge, 0, 0
    for other = math.max(1, index - 12), math.min(#hours, index + 11) do
        local t = hours[other].soilTemp
        if t then low, high, sum, n = math.min(low, t), math.max(high, t), sum + t, n + 1 end
    end
    local value = n >= 6 and high - low <= LAND_SWING and sum / n or false
    memo.at[index] = value
    return value or nil
end

-- What a service measured near the place now (Gauges, editions/fishing/
-- gauges.lua): the river gauge's sensor, the NOAA station's or the Great
-- Lake's daily average, the first that is fresh. { temp, epoch, kind =
-- "measured", source, short }, or nil. Stale readings are left out.
function Water.measured(forecast)
    if not forecast then return nil end
    local Gauges = require "Weather/editions/fishing/gauges"
    local Units = require "Weather/units"
    local now = forecast.epoch or os.time()
    local gauge = Gauges.of(forecast)
    if gauge and gauge.fresh and gauge.temp then
        return { temp = gauge.temp, epoch = gauge.epoch, kind = "measured", short = "at the gauge",
            source = ("measured at %s, %s (%s)"):format(gauge.name, Gauges.ago(gauge.age), gauge.source) }
    end
    local noaa = forecast.waterNoaa
    if noaa and now - noaa.epoch <= Gauges.STALE then
        local station = forecast.spot and forecast.spot.tideStation or {}
        local km = require("Weather/editions/fishing/tides").distanceKm(forecast.spot and forecast.spot.lat,
            forecast.spot and forecast.spot.lon, noaa.lat or station.lat, noaa.lon or station.lon)
        return { temp = noaa.temp, epoch = noaa.epoch, kind = "measured", short = "NOAA station",
            source = ("measured at %s%s, %s (NOAA)"):format(station.name or noaa.name or "the NOAA station",
                km and ", " .. Units.distance(km) .. " away" or "", Gauges.ago(now - noaa.epoch)) }
    end
    local lake = Gauges.lake(forecast)
    if lake then
        local week = lake.week and math.abs(lake.week) >= 0.5 and (", %s %s this week"):format(
            lake.week > 0 and "up" or "down", Units.tempDelta(math.abs(lake.week))) or ""
        return { temp = lake.temp, epoch = lake.epoch, kind = "measured", short = "lake average",
            source = ("Lake %s average surface from satellites, not your shore%s (NOAA GLERL)"):format(lake.lake, week) }
    end
end

-- The water at an hour: { temp, kind = "reading" | "measured" | "sea" |
-- "lake", source, short }, or nil with no source. `reading` is the place's
-- stored reading ({ temp, epoch }) or nil; `measured` is Water.measured's.
function Water.at(hour, forecast, reading, measured)
    local spot = forecast and forecast.spot or {}
    local epoch = hour.epoch or forecast and forecast.epoch
    if reading and reading.temp and reading.epoch and epoch and math.abs(epoch - reading.epoch) <= Water.READING_SECONDS then
        return { temp = reading.temp, kind = "reading", source = "your own reading", short = "your reading" }
    end
    if measured and epoch and math.abs(epoch - measured.epoch) <= Water.MEASURED_SECONDS then return measured end
    if not spot.lake and hour.seaTemp then
        return { temp = hour.seaTemp, kind = "sea", source = "sea surface, Open-Meteo marine model", short = "sea surface" }
    end
    if spot.lake and hour.soilTemp then
        local estimate = lakeEstimate(forecast or {}, hour)
        if estimate then
            return { temp = estimate, kind = "lake", short = "lake estimate",
                source = "lake surface estimate from the weather model; check with a thermometer" }
        end
    end
    return nil
end

-- Cold water is a safety matter, whatever the fishing: below 10° a fall in
-- takes the breath away, below 15° it still chills fast.
function Water.danger(temp)
    if temp == nil then return nil end
    if temp < 10 then return "cold" end
    if temp < 15 then return "cool" end
    return nil
end

return Water
