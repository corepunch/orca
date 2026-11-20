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
-- `forecast.quarters`. With `models`, the main call asks several weather
-- models at once: the best match keeps the plain names and every other
-- model's columns land under their suffixed API names
-- (`hour.wind_speed_10m_gfs_seamless`).
local Sky = require "Weather/sky"
local Forecast = {}

-- Every base URL and the optional API key in one place (PLAN.md 0.1): the
-- commercial API uses the same paths on "customer-" hosts with `apikey`,
-- so a key switches every Open-Meteo host at once. NOAA's tide predictions
-- are free and keyless.
Forecast.endpoints = {
    forecast = "https://api.open-meteo.com/v1/forecast",
    marine = "https://marine-api.open-meteo.com/v1/marine",
    airQuality = "https://air-quality-api.open-meteo.com/v1/air-quality",
    geocoding = "https://geocoding-api.open-meteo.com/v1/search",
    flood = "https://flood-api.open-meteo.com/v1/flood",
    archive = "https://archive-api.open-meteo.com/v1/archive",
    noaaTides = "https://api.tidesandcurrents.noaa.gov/api/prod/datagetter",
    apiKey = nil,
}
local DAYS = 7

local function openMeteo(base) return base:find("^https://[%w%-]*%.?open%-meteo%.com/") ~= nil end

-- The base URL for an endpoint name, on the customer host when there is a
-- key and the host is Open-Meteo's.
function Forecast.endpoint(name)
    local base = assert(Forecast.endpoints[name], "Unknown endpoint " .. tostring(name))
    if Forecast.endpoints.apiKey and openMeteo(base) then base = base:gsub("^https://", "https://customer-") end
    return base
end

-- A full URL: endpoint, query string and, for Open-Meteo, the key when
-- there is one.
function Forecast.url(name, query)
    local key = openMeteo(Forecast.endpoints[name] or "") and Forecast.endpoints.apiKey
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
-- Quarter hours stop after `minutely15Steps` steps from now, so a rain
-- nowcast does not pull 16 days of them.
function Forecast.urls(spot, needsMarine, request)
    request = request or {}
    local fields = "hourly=" .. names(HOURLY, request.hourly) .. "&daily=" .. names(DAILY, request.daily)
    if request.minutely15 and #request.minutely15 > 0 then
        fields = fields .. "&minutely_15=" .. table.concat(request.minutely15, ",")
        if request.minutely15Steps then
            fields = fields .. "&forecast_minutely_15=" .. request.minutely15Steps .. "&past_minutely_15=0"
        end
    end
    if request.models and #request.models > 0 then
        fields = fields .. "&models=best_match," .. table.concat(request.models, ",")
    end
    return Forecast.url("forecast", query(spot, fields, request)),
        needsMarine and Forecast.url("marine", query(spot, "hourly=" .. names(MARINE, request.marine), request)) or nil
end

-- Every request names the app (a generic User-Agent with no contact:
-- api.weather.gov answers 403 without one) and sends no cookies. `extra`
-- adds a source's own headers (a table, or a function of the spot).
Forecast.userAgent = "Weather/1.0 (ORCA)"
function Forecast.requestOptions(extra, spot)
    local headers = { ["User-Agent"] = Forecast.userAgent }
    if type(extra) == "function" then extra = extra(spot) end
    for name, value in pairs(extra or {}) do headers[name] = value end
    return { headers = headers, nocookies = true }
end

function Forecast.networkFetch(url, options)
    local network = require "orca.network"
    local ok, response = pcall(network.fetch, url, options or Forecast.requestOptions())
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

-- A several-model answer names the best match's columns `<name>_best_match`;
-- they become the plain columns every rule reads.
local function bestMatch(block)
    if type(block) ~= "table" then return block end
    local renamed = {}
    for name, values in pairs(block) do
        local plain = type(name) == "string" and name:match("^(.+)_best_match$")
        if plain and block[plain] == nil then renamed[plain] = values end
    end
    for name, values in pairs(renamed) do block[name], block[name .. "_best_match"] = values, nil end
    return block
end

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
    local h, d = bestMatch(weather.hourly) or {}, bestMatch(weather.daily) or {}
    bestMatch(weather.minutely_15)
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
-- replay: `raw.sources` holds the answers saved earlier, and a source never
-- asked for (`raw.asked`) is left out. `lenient` (a page adding its sources
-- later) never fails the forecast.
local function asked(raw, id) return raw.asked and raw.asked[id] or raw.sources and raw.sources[id] ~= nil end
local function runSources(forecast, spot, sources, fetch, raw, lenient)
    raw.sources, raw.asked = raw.sources or {}, raw.asked or {}
    for _, source in ipairs(sources or {}) do
        local json, err
        if fetch then
            local url = source.url(spot, Forecast)
            if url then json, err = fetch(url, Forecast.requestOptions(source.headers, spot)) end
            raw.asked[source.id] = true
            if json then raw.sources[source.id] = json end
        elseif asked(raw, source.id) then
            json = raw.sources[source.id]
            err = json == nil and "not saved" or nil
        end
        if json then
            local ok, problem = pcall(source.parse, json, forecast)
            if not ok then forecast.missing[source.id] = tostring(problem) end
        elseif err then
            if not source.optional and fetch and not lenient then return nil, err end
            forecast.missing[source.id] = err
        end
    end
    return forecast
end

-- The sources of `request` that `raw` has not asked yet.
function Forecast.unasked(request, raw)
    local list = {}
    for _, source in ipairs(request and request.sources or {}) do
        if not (raw and asked(raw, source.id)) then list[#list + 1] = source end
    end
    return list
end

-- Adds sources to a forecast already made (an activity page asks for its
-- own); their answers join `raw`. Returns how many it fetched.
function Forecast.extend(forecast, spot, sources, fetch, raw)
    if #sources == 0 then return 0 end
    runSources(forecast, spot, sources, fetch or Forecast.networkFetch, raw, true)
    return #sources
end

-- Fetches and parses. Returns the forecast, nil and the decoded answers
-- (`raw`, for Forecast.restore); or nil, err.
function Forecast.load(spot, needsMarine, fetch, epoch, request)
    fetch = fetch or Forecast.networkFetch
    epoch = epoch or os.time()
    local weatherUrl, marineUrl = Forecast.urls(spot, needsMarine, request)
    local weather, err = fetch(weatherUrl, Forecast.requestOptions())
    if not weather then return nil, err end
    local marine
    if marineUrl then
        -- "optional": waves help but the forecast stands without them.
        marine, err = fetch(marineUrl, Forecast.requestOptions())
        if not marine and needsMarine ~= "optional" then return nil, err end
    end
    local raw = { at = epoch, weather = weather, marine = marine, sources = {}, asked = {} }
    local forecast, sourceErr = runSources(Forecast.parse(weather, marine, epoch, spot), spot,
        request and request.sources, fetch, raw)
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
    return runSources(forecast, spot, request and request.sources, nil, raw)
end

return Forecast
