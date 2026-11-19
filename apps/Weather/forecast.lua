-- One forecast for one spot from Open-Meteo (https://open-meteo.com, free,
-- no key): the general forecast and, when the profile needs the sea, the
-- marine forecast, merged hour by hour into plain tables. `fetch(url)`
-- returns decoded JSON or nil, err; the default uses orca.network and must
-- run in an ORCA coroutine. Parsing is pure so tests feed recorded JSON.
local Forecast = {}

local FORECAST_URL = "https://api.open-meteo.com/v1/forecast"
local MARINE_URL = "https://marine-api.open-meteo.com/v1/marine"
local DAYS = 7

local HOURLY = "temperature_2m,apparent_temperature,weather_code,wind_speed_10m,wind_gusts_10m," ..
    "wind_direction_10m,pressure_msl,cloud_cover,precipitation_probability,precipitation," ..
    "relative_humidity_2m,visibility,uv_index,is_day,dew_point_2m,cloud_cover_low,cloud_cover_mid," ..
    "cloud_cover_high,freezing_level_height,cape,soil_moisture_0_to_1cm,soil_temperature_0cm,et0_fao_evapotranspiration"
local DAILY = "weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max," ..
    "precipitation_sum,wind_speed_10m_max,sunrise,sunset"
local MARINE = "wave_height,wave_period,wave_direction,swell_wave_height,swell_wave_period," ..
    "swell_wave_direction,sea_surface_temperature"

local function query(base, spot, fields)
    return ("%s?latitude=%s&longitude=%s&%s&wind_speed_unit=ms&timezone=auto&forecast_days=%d")
        :format(base, spot.lat, spot.lon, fields, DAYS)
end

function Forecast.urls(spot, needsMarine)
    return query(FORECAST_URL, spot, "hourly=" .. HOURLY .. "&daily=" .. DAILY),
        needsMarine and query(MARINE_URL, spot, "hourly=" .. MARINE) or nil
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

-- Change of sea-level pressure over the last three hours, hPa.
local function trend(hours, index)
    local now, before = hours[index], hours[index - 3]
    if not (now and before and now.pressure and before.pressure) then return 0 end
    return now.pressure - before.pressure
end

-- Builds the report from decoded responses. `epoch` is the current UTC time.
function Forecast.parse(weather, marine, epoch)
    local h, d = weather.hourly or {}, weather.daily or {}
    local m = marine and marine.hourly
    local marineAt = {}
    for index, time in ipairs(m and m.time or {}) do marineAt[time] = index end
    local hours = {}
    for index, time in ipairs(h.time or {}) do
        local mi = marineAt[time]
        hours[index] = {
            time = time, date = time:sub(1, 10),
            temp = column(h, "temperature_2m", index), feels = column(h, "apparent_temperature", index),
            code = column(h, "weather_code", index), wind = column(h, "wind_speed_10m", index),
            gust = column(h, "wind_gusts_10m", index), windDir = column(h, "wind_direction_10m", index),
            pressure = column(h, "pressure_msl", index), cloud = column(h, "cloud_cover", index),
            rainChance = column(h, "precipitation_probability", index), rain = column(h, "precipitation", index),
            humidity = column(h, "relative_humidity_2m", index), visibility = column(h, "visibility", index),
            uv = column(h, "uv_index", index), isDay = column(h, "is_day", index) == 1,
            dewPoint = column(h, "dew_point_2m", index), cloudLow = column(h, "cloud_cover_low", index),
            cloudMid = column(h, "cloud_cover_mid", index), cloudHigh = column(h, "cloud_cover_high", index),
            freezingLevel = column(h, "freezing_level_height", index), cape = column(h, "cape", index),
            soilMoisture = column(h, "soil_moisture_0_to_1cm", index),
            soilTemp = column(h, "soil_temperature_0cm", index), et0 = column(h, "et0_fao_evapotranspiration", index),
            waveHeight = mi and column(m, "wave_height", mi), wavePeriod = mi and column(m, "wave_period", mi),
            waveDir = mi and column(m, "wave_direction", mi), swellHeight = mi and column(m, "swell_wave_height", mi),
            swellPeriod = mi and column(m, "swell_wave_period", mi), swellDir = mi and column(m, "swell_wave_direction", mi),
            seaTemp = mi and column(m, "sea_surface_temperature", mi),
        }
    end
    for index, hour in ipairs(hours) do
        hour.pressureTrend = trend(hours, index)
        hour.code = plausible(hour.code, hour.temp, 5)
    end
    local days = {}
    for index, date in ipairs(d.time or {}) do
        days[index] = {
            date = date, code = column(d, "weather_code", index),
            max = column(d, "temperature_2m_max", index), min = column(d, "temperature_2m_min", index),
            rainChance = column(d, "precipitation_probability_max", index), rain = column(d, "precipitation_sum", index),
            windMax = column(d, "wind_speed_10m_max", index),
            sunrise = column(d, "sunrise", index), sunset = column(d, "sunset", index),
        }
    end
    -- A day whose coldest hour is above 3 °C had no snow.
    for _, day in ipairs(days) do day.code = plausible(day.code, day.min, 3) end
    local offset = weather.utc_offset_seconds or 0
    local localHour = os.date("!%Y-%m-%dT%H:00", epoch + offset)
    local now = 1
    for index, hour in ipairs(hours) do
        if hour.time == localHour then now = index; break end
    end
    -- Each hour's UTC time, counted from the current hour.
    local hourStart = epoch - (epoch + offset) % 3600
    for index, hour in ipairs(hours) do hour.epoch = hourStart + (index - now) * 3600 end
    local dayOf = {}
    for _, day in ipairs(days) do dayOf[day.date] = day end
    for _, hour in ipairs(hours) do hour.day = dayOf[hour.date] end
    return { hours = hours, days = days, now = now, epoch = epoch, offset = offset, hasMarine = m ~= nil,
        elevation = weather.elevation }
end

function Forecast.load(spot, needsMarine, fetch, epoch)
    fetch = fetch or Forecast.networkFetch
    local weatherUrl, marineUrl = Forecast.urls(spot, needsMarine)
    local weather, err = fetch(weatherUrl)
    if not weather then return nil, err end
    local marine
    if marineUrl then
        -- "optional": waves help but the forecast stands without them.
        marine, err = fetch(marineUrl)
        if not marine and needsMarine ~= "optional" then return nil, err end
    end
    return Forecast.parse(weather, marine, epoch or os.time())
end

return Forecast
