-- Tides and tidal streams (SA6), approximate: Open-Meteo's marine model
-- sea level (`sea_level_height_msl`, tides included) and currents
-- (`ocean_current_velocity`, `ocean_current_direction`, the way the water
-- flows TO). The model grid is about 8 km and Open-Meteo says it is not
-- suitable for coastal navigation (PLAN.md decision 0.4), so every line
-- that shows it says "approximate". Lakes have no tide and skip the call.
--
-- An own request rather than `fields.marine`: it reads the answer's units,
-- so the current is in m/s whatever the API's default.
local Sky = require "Weather/sky"
local Tides = {}

local TO_MS = { ["m/s"] = 1, ["km/h"] = 1 / 3.6, kn = 0.514444, mph = 0.44704 }

local function number(v) return type(v) == "number" and v or nil end

Tides.source = {
    id = "tides", optional = true,
    url = function(spot, Forecast)
        if not (spot and spot.lat and spot.lon) or spot.lake then return nil end
        return Forecast.url("marine", ("latitude=%s&longitude=%s&hourly=sea_level_height_msl,ocean_current_velocity,"
            .. "ocean_current_direction&wind_speed_unit=ms&timezone=auto&forecast_days=7"):format(spot.lat, spot.lon))
    end,
    -- Puts `hour.seaLevel` (m), `hour.current` (m/s) and `hour.currentDir`
    -- on the hours, matched by local time.
    parse = function(json, forecast)
        local hourly = type(json) == "table" and json.hourly
        if type(hourly) ~= "table" or type(hourly.time) ~= "table" then return end
        local units = type(json.hourly_units) == "table" and json.hourly_units or {}
        local scale = TO_MS[units.ocean_current_velocity or "km/h"] or 1 / 3.6
        local at = {}
        for index, hour in ipairs(forecast.hours) do at[hour.time] = index end
        local any = false
        for index, time in ipairs(hourly.time) do
            local hour = forecast.hours[at[time] or 0]
            if hour then
                local level = number(hourly.sea_level_height_msl and hourly.sea_level_height_msl[index])
                local speed = number(hourly.ocean_current_velocity and hourly.ocean_current_velocity[index])
                hour.seaLevel, hour.current = level, speed and speed * scale
                hour.currentDir = number(hourly.ocean_current_direction and hourly.ocean_current_direction[index])
                any = any or level ~= nil
            end
        end
        forecast.tides = any
    end,
}

-- High and low waters on a date: turning points of the hourly sea level,
-- each { kind = "high" | "low", time, level }. Hourly steps put a turn
-- within about half an hour of the true one. A range under 10 cm is no tide.
function Tides.turns(forecast, date)
    local hours, turns = forecast.hours, {}
    local low, high = math.huge, -math.huge
    for index = 2, #hours - 1 do
        local a, b, c = hours[index - 1].seaLevel, hours[index].seaLevel, hours[index + 1].seaLevel
        if hours[index].date == date and a and b and c then
            low, high = math.min(low, b), math.max(high, b)
            if b > a and b >= c then turns[#turns + 1] = { kind = "high", time = hours[index].time, level = b }
            elseif b < a and b <= c then turns[#turns + 1] = { kind = "low", time = hours[index].time, level = b } end
        end
    end
    if high - low < 0.1 then return {} end
    return turns
end

-- Whether the wind blows against the stream at an hour: a usable wind
-- (6 m/s, about 12 kn) from where a stream of at least 0.5 m/s (about 1 kn)
-- is flowing to. That raises short, steep chop.
function Tides.windAgainst(hour)
    if not (hour.current and hour.currentDir and hour.windDir and hour.wind) then return false end
    return hour.current >= 0.5 and hour.wind >= 6 and Sky.angle(hour.windDir, hour.currentDir) <= 60
end

return Tides
