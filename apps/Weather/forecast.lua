-- One forecast for one spot from Open-Meteo (https://open-meteo.com): the
-- general forecast and, when the profile needs the sea, the marine forecast,
-- merged hour by hour into plain tables. `fetch(url)` returns decoded JSON
-- or nil, err; the default uses orca.network and must run in an ORCA
-- coroutine. Parsing is pure so tests feed recorded JSON.
--
-- A request (Activities.request) adds the columns its activities declare
-- (`fields`) and extra requests (`sources`). A column the parser does not
-- name lands on the hour or day under its API name
-- (`hour.soil_temperature_6cm`); minutely_15 columns land on
-- `forecast.quarters`.
local Sky = require "Weather/sky"
local Forecast = {}

-- Every base URL and the optional API key in one place (PLAN.md 0.1): the
-- commercial API uses the same paths on "customer-" hosts with `apikey`.
Forecast.endpoints = {
    forecast = "https://api.open-meteo.com/v1/forecast",
    marine = "https://marine-api.open-meteo.com/v1/marine",
    airQuality = "https://air-quality-api.open-meteo.com/v1/air-quality",
    geocoding = "https://geocoding-api.open-meteo.com/v1/search",
    apiKey = nil,
}
local DAYS = 7

-- The base URL for an endpoint name, on the customer host when there is a key.
function Forecast.endpoint(name)
    local base = assert(Forecast.endpoints[name], "Unknown endpoint " .. tostring(name))
    if Forecast.endpoints.apiKey then base = base:gsub("^https://", "https://customer-") end
    return base
end

-- A full URL: endpoint, query string and the key when there is one.
function Forecast.url(name, query)
    local key = Forecast.endpoints.apiKey
    return Forecast.endpoint(name) .. "?" .. query .. (key and "&apikey=" .. key or "")
end

-- API column → hour field, for the columns every activity reads.
local HOURLY = {
    { "temperature_2m", "temp" }, { "apparent_temperature", "feels" }, { "weather_code", "code" },
    { "wind_speed_10m", "wind" }, { "wind_gusts_10m", "gust" }, { "wind_direction_10m", "windDir" },
    { "pressure_msl", "pressure" }, { "cloud_cover", "cloud" }, { "precipitation_probability", "rainChance" },
    { "precipitation", "rain" }, { "relative_humidity_2m", "humidity" }, { "visibility", "visibility" },
    { "uv_index", "uv" }, { "is_day", "isDay" }, { "dew_point_2m", "dewPoint" }, { "cloud_cover_low", "cloudLow" },
    { "cloud_cover_mid", "cloudMid" }, { "cloud_cover_high", "cloudHigh" },
    { "freezing_level_height", "freezingLevel" }, { "cape", "cape" }, { "soil_moisture_0_to_1cm", "soilMoisture" },
    { "soil_temperature_0cm", "soilTemp" }, { "et0_fao_evapotranspiration", "et0" },
}
local DAILY = {
    { "weather_code", "code" }, { "temperature_2m_max", "max" }, { "temperature_2m_min", "min" },
    { "precipitation_probability_max", "rainChance" }, { "precipitation_sum", "rain" },
    { "wind_speed_10m_max", "windMax" }, { "sunrise", "sunrise" }, { "sunset", "sunset" },
}
local MARINE = {
    { "wave_height", "waveHeight" }, { "wave_period", "wavePeriod" }, { "wave_direction", "waveDir" },
    { "swell_wave_height", "swellHeight" }, { "swell_wave_period", "swellPeriod" },
    { "swell_wave_direction", "swellDir" }, { "sea_surface_temperature", "seaTemp" },
}

local function names(map, extra)
    local list, seen = {}, {}
    for _, entry in ipairs(map) do list[#list + 1] = entry[1]; seen[entry[1]] = true end
    for _, name in ipairs(extra or {}) do
        if not seen[name] then list[#list + 1] = name; seen[name] = true end
    end
    return table.concat(list, ",")
end

local function query(spot, fields, request)
    local text = ("latitude=%s&longitude=%s&%s&wind_speed_unit=ms&timezone=auto&forecast_days=%d")
        :format(spot.lat, spot.lon, fields, request.forecastDays or DAYS)
    if request.pastDays and request.pastDays > 0 then text = text .. "&past_days=" .. request.pastDays end
    return text
end

-- The forecast URL and, when needed, the marine URL. `request` is optional.
function Forecast.urls(spot, needsMarine, request)
    request = request or {}
    local fields = "hourly=" .. names(HOURLY, request.hourly) .. "&daily=" .. names(DAILY, request.daily)
    if request.minutely15 and #request.minutely15 > 0 then
        fields = fields .. "&minutely_15=" .. table.concat(request.minutely15, ",")
    end
    return Forecast.url("forecast", query(spot, fields, request)),
        needsMarine and Forecast.url("marine", query(spot, "hourly=" .. names(MARINE, request.marine), request)) or nil
end

function Forecast.networkFetch(url)
    local network = require "orca.network"
    local ok, response = pcall(network.fetch, url)
    if not ok or (type(response) ~= "userdata" and type(response) ~= "table") then
        return nil, "No connection to the forecast service."
    end
    local codeOk, code = pcall(function() return response:code() end)
    if not codeOk or code ~= 200 then return nil, "The forecast service is not answering." end
    local decoded, body = pcall(function() return response:json() end)
    if not decoded or type(body) ~= "table" then return nil, "The forecast came back garbled." end
    return body
end

-- The model sometimes reports snow at temperatures where it can only be rain
-- (San Francisco, "heavy snow" at 11 °C). Snow codes above these
-- temperatures become the matching rain code.
local RAIN_FOR_SNOW = { [71] = 61, [73] = 63, [75] = 65, [77] = 61, [85] = 80, [86] = 82 }
local function plausible(code, temp, warmEnough)
    if RAIN_FOR_SNOW[code] and temp and temp >= warmEnough then return RAIN_FOR_SNOW[code] end
    return code
end
Forecast.plausible = plausible

local function column(block, name, index) return block and block[name] and block[name][index] end

-- Copies one row of a block: named columns to their field, the rest under
-- their API name.
local function row(block, index, map, into)
    local known = {}
    for _, entry in ipairs(map) do
        into[entry[2]] = column(block, entry[1], index)
        known[entry[1]] = true
    end
    for name, values in pairs(block) do
        if not known[name] and name ~= "time" and type(values) == "table" then into[name] = values[index] end
    end
    return into
end

-- Change of sea-level pressure over the last three hours, hPa.
local function trend(hours, index)
    local now, before = hours[index], hours[index - 3]
    if not (now and before and now.pressure and before.pressure) then return 0 end
    return now.pressure - before.pressure
end

-- Builds the report from decoded responses. `epoch` is the current UTC time;
-- `spot` (optional) gives the latitude and longitude when the response
-- has none.
function Forecast.parse(weather, marine, epoch, spot)
    local h, d = weather.hourly or {}, weather.daily or {}
    local m = marine and marine.hourly
    local marineAt = {}
    for index, time in ipairs(m and m.time or {}) do marineAt[time] = index end
    local hours = {}
    for index, time in ipairs(h.time or {}) do
        local hour = row(h, index, HOURLY, { time = time, date = time:sub(1, 10) })
        hour.isDay = hour.isDay == 1
        local mi = marineAt[time]
        if mi then row(m, mi, MARINE, hour) end
        hours[index] = hour
    end
    for index, hour in ipairs(hours) do
        hour.pressureTrend = trend(hours, index)
        hour.code = plausible(hour.code, hour.temp, 5)
    end
    local days = {}
    for index, date in ipairs(d.time or {}) do days[index] = row(d, index, DAILY, { date = date }) end
    -- A day whose coldest hour is above 3 °C had no snow.
    for _, day in ipairs(days) do day.code = plausible(day.code, day.min, 3) end
    local offset = weather.utc_offset_seconds or 0
    local localHour = os.date("!%Y-%m-%dT%H:00", epoch + offset)
    local now, found = 1, false
    for index, hour in ipairs(hours) do
        if hour.time == localHour then now, found = index, true; break end
    end
    -- Each hour's UTC time, counted from the current hour; past hours
    -- (pastDays) keep their order before it.
    local hourStart = epoch - (epoch + offset) % 3600
    for index, hour in ipairs(hours) do hour.epoch = hourStart + (index - now) * 3600 end
    -- The sun: its height at each hour and each day's twilights.
    local lat, lon = weather.latitude or spot and spot.lat, weather.longitude or spot and spot.lon
    local dayOf, today = {}, 1
    for index, day in ipairs(days) do
        dayOf[day.date] = day
        if day.date == localHour:sub(1, 10) then today = index end
        if lat and lon then day.twilight = Sky.twilight(day.date, lat, lon, offset) end
    end
    for _, hour in ipairs(hours) do
        hour.day = dayOf[hour.date]
        if lat and lon then hour.sunAltitude = Sky.sunAltitude(hour.epoch, lat, lon) end
    end
    -- Quarter hours (minutely_15), each with its UTC time.
    local quarters, q = {}, weather.minutely_15
    for index, time in ipairs(q and q.time or {}) do
        local quarter = row(q, index, {}, { time = time })
        local start = Sky.epochOf(time:sub(1, 10))
        local minutes = Sky.minutes(time)
        quarter.epoch = start and minutes and start + minutes * 60 - offset
        quarters[index] = quarter
    end
    return { hours = hours, days = days, now = now, today = today, current = found, epoch = epoch, offset = offset,
        hasMarine = m ~= nil, elevation = weather.elevation, quarters = quarters, updated = epoch, missing = {} }
end

-- Runs a request's extra sources over a parsed forecast. `fetch` nil means
-- replay: `raw.sources` holds the answers saved earlier.
local function runSources(forecast, spot, request, fetch, raw)
    for _, source in ipairs(request and request.sources or {}) do
        local json, err
        if fetch then
            local url = source.url(spot, Forecast)
            if url then json, err = fetch(url) end
            if json then raw.sources[source.id] = json end
        else
            json = raw.sources and raw.sources[source.id]
            err = json == nil and "not saved" or nil
        end
        if json then
            local ok, problem = pcall(source.parse, json, forecast)
            if not ok then forecast.missing[source.id] = tostring(problem) end
        elseif err then
            if not source.optional and fetch then return nil, err end
            forecast.missing[source.id] = err
        end
    end
    return forecast
end

-- Fetches and parses. Returns the forecast, nil and the decoded answers
-- (`raw`, for Forecast.restore); or nil, err.
function Forecast.load(spot, needsMarine, fetch, epoch, request)
    fetch = fetch or Forecast.networkFetch
    epoch = epoch or os.time()
    local weatherUrl, marineUrl = Forecast.urls(spot, needsMarine, request)
    local weather, err = fetch(weatherUrl)
    if not weather then return nil, err end
    local marine
    if marineUrl then
        -- "optional": waves help but the forecast stands without them.
        marine, err = fetch(marineUrl)
        if not marine and needsMarine ~= "optional" then return nil, err end
    end
    local raw = { at = epoch, weather = weather, marine = marine, sources = {} }
    local forecast, sourceErr = runSources(Forecast.parse(weather, marine, epoch, spot), spot, request, fetch, raw)
    if not forecast then return nil, sourceErr end
    return forecast, nil, raw
end

-- A forecast from answers saved earlier (`raw` from Forecast.load), as of
-- `epoch`. nil when the saved hours no longer reach the current hour.
function Forecast.restore(raw, spot, request, epoch)
    if type(raw) ~= "table" or type(raw.weather) ~= "table" then return nil end
    local forecast = Forecast.parse(raw.weather, raw.marine, epoch, spot)
    if not forecast.current then return nil end
    forecast.updated = raw.at or epoch
    return runSources(forecast, spot, request, nil, raw)
end

return Forecast
