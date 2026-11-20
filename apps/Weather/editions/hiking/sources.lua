-- The hiking edition's extra requests (F4 `sources`), each optional so the
-- forecast stands without them:
--
--   points  the trailhead and the top in one multi-point request, each at
--           its own height, with wind on pressure levels for the ridge (HI1, HI2)
--   models  three weather models' daily answers, to say how sure (HI6)
--   air     smoke and dust (US AQI, PM2.5) at the trailhead (HI8)
--   alerts  official warnings for US places from api.weather.gov (HI7)
--
-- Parsing is pure: each `parse` reads decoded JSON into `forecast.hiking`
-- and onto the hours (`hour.top`, `hour.start`, `hour.aqi`). Hours are
-- matched by UTC time, so a point in another time zone still lines up.
local Sky = require "Weather/sky"
local Peaks = require "Weather/editions/hiking/peaks"
local Sources = {}

-- A place's two points: its own `top` and `start` (a place the user made),
-- else the edition's table.
function Sources.peak(spot)
    if not spot then return nil end
    if spot.top and spot.start then return spot end
    return Peaks[spot.id]
end

local function utc(epoch) return os.date("!%Y-%m-%dT%H:00", epoch) end

-- The forecast's hours by UTC time ("2026-11-18T06:00").
local function hoursByUtc(forecast)
    local byTime = {}
    for _, hour in ipairs(forecast.hours or {}) do
        if hour.epoch then byTime[utc(hour.epoch)] = hour end
    end
    return byTime
end

local function state(forecast)
    forecast.hiking = forecast.hiking or {}
    return forecast.hiking
end

-- "2026-11-20T10:00:00-08:00" (or "…Z") → UTC epoch, or nil.
function Sources.epoch(iso)
    local date, h, m, sign, oh, om = (iso or ""):match("^(%d+%-%d+%-%d+)T(%d+):(%d+)[:%d%.]*([%+%-]?)(%d*):?(%d*)")
    local start = date and Sky.epochOf(date)
    if not start then return nil end
    local offset = sign ~= "" and ((tonumber(oh) or 0) * 3600 + (tonumber(om) or 0) * 60) or 0
    return start + tonumber(h) * 3600 + tonumber(m) * 60 - (sign == "-" and -offset or offset)
end

-- Points: the same columns at both heights (Open-Meteo downscales
-- temperature to `elevation=`), plus pressure levels for free-air wind.
Sources.LEVELS = { 925, 850, 700, 600, 500 }
local POINT = {
    { "temperature_2m", "temp" }, { "apparent_temperature", "feels" }, { "weather_code", "code" },
    { "wind_speed_10m", "wind" }, { "wind_gusts_10m", "gust" }, { "wind_direction_10m", "windDir" },
    { "cloud_cover", "cloud" }, { "visibility", "visibility" }, { "precipitation_probability", "rainChance" },
    { "precipitation", "rain" }, { "freezing_level_height", "freezingLevel" }, { "cape", "cape" },
    { "lifted_index", "lifted_index" }, { "snow_depth", "snow_depth" }, { "snowfall", "snowfall" },
}
Sources.PAST_DAYS, Sources.POINT_DAYS = 3, 7

local function pointColumns()
    local list = {}
    for _, entry in ipairs(POINT) do list[#list + 1] = entry[1] end
    for _, level in ipairs(Sources.LEVELS) do
        for _, name in ipairs { "wind_speed_", "wind_direction_", "geopotential_height_" } do
            list[#list + 1] = name .. level .. "hPa"
        end
    end
    return table.concat(list, ",")
end

-- Wind at a height from the pressure levels around it: linear in height,
-- the direction of the nearer level. nil below the lowest level (there the
-- 10 m wind is the better guide) or when the levels are missing.
function Sources.ridge(levels, elevation)
    table.sort(levels, function(a, b) return a.height < b.height end)
    if not elevation or #levels == 0 or elevation < levels[1].height then return nil end
    for index = 2, #levels do
        local low, high = levels[index - 1], levels[index]
        if elevation <= high.height then
            local t = (elevation - low.height) / (high.height - low.height)
            return low.speed + (high.speed - low.speed) * t, t < 0.5 and low.dir or high.dir
        end
    end
    return levels[#levels].speed, levels[#levels].dir
end

local function readPoint(json, index, ridgeAt)
    local h = json.hourly or {}
    local point = {}
    for _, entry in ipairs(POINT) do point[entry[2]] = h[entry[1]] and h[entry[1]][index] end
    point.code = require("Weather/forecast").plausible(point.code, point.temp, 5)
    if ridgeAt then
        local levels = {}
        for _, level in ipairs(Sources.LEVELS) do
            local speed, height = h["wind_speed_" .. level .. "hPa"], h["geopotential_height_" .. level .. "hPa"]
            speed, height = speed and speed[index], height and height[index]
            if speed and height then
                local dir = h["wind_direction_" .. level .. "hPa"]
                levels[#levels + 1] = { speed = speed, height = height, dir = dir and dir[index] }
            end
        end
        point.ridge, point.ridgeDir = Sources.ridge(levels, ridgeAt)
    end
    return point
end

-- Each hour's `start` and `top` read through to the hour for what they
-- lack (`isDay`, `uv`, the day).
local function attach(forecast, json, key, ridge)
    local byTime = hoursByUtc(forecast)
    for index, time in ipairs(json.hourly and json.hourly.time or {}) do
        local hour = byTime[time]
        if hour then
            local point = readPoint(json, index, ridge and json.elevation)
            point.elevation = json.elevation
            hour[key] = setmetatable(point, { __index = hour })
        end
    end
end

local points = {
    id = "hiking-points", optional = true,
    url = function(spot, Forecast)
        local peak = Sources.peak(spot)
        if not peak then return nil end
        local a, b = peak.start, peak.top
        return Forecast.url("forecast", ("latitude=%s,%s&longitude=%s,%s&elevation=%d,%d&hourly=%s"
            .. "&wind_speed_unit=ms&timezone=GMT&past_days=%d&forecast_days=%d")
            :format(a.lat, b.lat, a.lon, b.lon, a.elevation, b.elevation, pointColumns(), Sources.PAST_DAYS,
                Sources.POINT_DAYS))
    end,
    -- A list of two answers, the start first.
    parse = function(json, forecast)
        assert(type(json[1]) == "table" and type(json[2]) == "table", "expected two points")
        local hiking = state(forecast)
        hiking.start, hiking.top = { elevation = json[1].elevation }, { elevation = json[2].elevation }
        attach(forecast, json[1], "start", false)
        attach(forecast, json[2], "top", true)
    end,
}

-- Models: their ids are only in this request; parsing reads whatever
-- `<column>_<model>` columns come back, so a retired model drops out.
Sources.MODELS = "ecmwf_ifs025,gfs_seamless,icon_seamless"
local DAILY = { "temperature_2m_max", "precipitation_sum", "wind_gusts_10m_max" }

-- How far the models' answers for one day lie apart, as a word: "sure",
-- "likely" or "uncertain"; nil with fewer than two models.
function Sources.agreement(temps, rains, gusts)
    if #temps < 2 then return nil end
    local function spread(list)
        if #list < 2 then return 0 end
        return math.max(table.unpack(list)) - math.min(table.unpack(list))
    end
    local wet = 0
    for _, rain in ipairs(rains) do if rain >= 1 then wet = wet + 1 end end
    local agree = wet == 0 or wet == #rains
    local most = #rains > 0 and math.max(table.unpack(rains)) or 0
    local t, g = spread(temps), spread(gusts)
    if t > 5 or g > 10 or (not agree and most >= 5) then return "uncertain" end
    if t <= 2.5 and g <= 5 and agree then return "sure" end
    return "likely"
end

local models = {
    id = "hiking-models", optional = true,
    url = function(spot, Forecast)
        local at = (Sources.peak(spot) or {}).top or spot
        return Forecast.url("forecast", ("latitude=%s&longitude=%s&daily=%s&models=%s&wind_speed_unit=ms"
            .. "&timezone=auto&forecast_days=16"):format(at.lat, at.lon, table.concat(DAILY, ","), Sources.MODELS))
    end,
    parse = function(json, forecast)
        local d = assert(json.daily and json.daily.time and json.daily, "no daily answers")
        local confidence = {}
        for index, date in ipairs(d.time) do
            local values = { {}, {}, {} }
            for name, column in pairs(d) do
                for slot, base in ipairs(DAILY) do
                    if name:sub(1, #base + 1) == base .. "_" and type(column) == "table" and column[index] then
                        table.insert(values[slot], column[index])
                    end
                end
            end
            local word = Sources.agreement(values[1], values[2], values[3])
            if word then confidence[date] = { word = word, models = #values[1] } end
        end
        state(forecast).confidence = confidence
    end,
}

local air = {
    id = "hiking-air", optional = true,
    url = function(spot, Forecast)
        local at = (Sources.peak(spot) or {}).start or spot
        return Forecast.url("airQuality", ("latitude=%s&longitude=%s&hourly=us_aqi,pm2_5&timezone=GMT&forecast_days=5")
            :format(at.lat, at.lon))
    end,
    parse = function(json, forecast)
        local h = assert(json.hourly, "no air quality")
        local byTime = hoursByUtc(forecast)
        for index, time in ipairs(h.time or {}) do
            local hour = byTime[time]
            if hour then hour.aqi, hour.pm25 = h.us_aqi and h.us_aqi[index], h.pm2_5 and h.pm2_5[index] end
        end
        state(forecast).air = h.us_aqi ~= nil
    end,
}

-- api.weather.gov needs no key but answers 403 without a User-Agent, which
-- every request carries (Forecast.requestOptions). An answer without
-- `features` is not "no warnings".
local alerts = {
    id = "hiking-alerts", optional = true,
    url = function(spot)
        local peak = Sources.peak(spot)
        if not (peak and peak.nws) then return nil end
        return ("https://api.weather.gov/alerts/active?point=%.4f,%.4f"):format(peak.top.lat, peak.top.lon)
    end,
    headers = { Accept = "application/geo+json" },
    parse = function(json, forecast)
        local features = assert(type(json.features) == "table" and json.features, "no alert list")
        local list = {}
        for _, feature in ipairs(features) do
            local p = feature.properties or {}
            if p.event and p.status ~= "Test" then
                list[#list + 1] = { event = p.event, headline = p.headline, severity = p.severity,
                    sender = p.senderName, onset = Sources.epoch(p.onset or p.effective),
                    ends = Sources.epoch(p.ends or p.expires) }
            end
        end
        state(forecast).alerts = list
    end,
}

Sources.list = { points, models, air, alerts }
return Sources
