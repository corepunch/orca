-- Things the sky does that every weather profile reads: WMO weather codes,
-- compass points, angles and the phase of the moon. Pure functions, so the
-- standalone tests can load this module without ORCA.
local Sky = {}

-- WMO weather interpretation codes, as Open-Meteo reports them.
local DESCRIPTIONS = {
    [0] = "clear sky", [1] = "mainly clear", [2] = "partly cloudy", [3] = "overcast",
    [45] = "fog", [48] = "icy fog",
    [51] = "light drizzle", [53] = "drizzle", [55] = "heavy drizzle",
    [56] = "freezing drizzle", [57] = "heavy freezing drizzle",
    [61] = "light rain", [63] = "rain", [65] = "heavy rain",
    [66] = "freezing rain", [67] = "heavy freezing rain",
    [71] = "light snow", [73] = "snow", [75] = "heavy snow", [77] = "snow grains",
    [80] = "light showers", [81] = "showers", [82] = "violent showers",
    [85] = "snow showers", [86] = "heavy snow showers",
    [95] = "thunderstorm", [96] = "thunderstorm with hail", [99] = "thunderstorm with heavy hail",
}

local ICONS = {
    [0] = "sun", [1] = "sun", [2] = "partly-cloudy", [3] = "cloud", [45] = "fog", [48] = "fog",
    [51] = "drizzle", [53] = "drizzle", [55] = "drizzle", [56] = "drizzle", [57] = "drizzle",
    [61] = "rain", [63] = "rain", [65] = "rain", [66] = "rain", [67] = "rain",
    [71] = "snow", [73] = "snow", [75] = "snow", [77] = "snow", [80] = "rain", [81] = "rain", [82] = "rain",
    [85] = "snow", [86] = "snow", [95] = "thunderstorm", [96] = "thunderstorm", [99] = "thunderstorm",
}

local COMPASS = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" }

function Sky.describe(code) return DESCRIPTIONS[code] or "unknown" end
function Sky.icon(code) return "WeatherKit/assets/icons/weather/" .. (ICONS[code] or "cloud") .. ".svg" end
function Sky.isStorm(code) return code ~= nil and code >= 95 end
function Sky.isWet(code) return code ~= nil and code >= 51 and code < 95 end
function Sky.isSnow(code) return code ~= nil and ((code >= 71 and code <= 77) or code == 85 or code == 86) end
function Sky.isHeavy(code) return code == 55 or code == 65 or code == 67 or code == 75 or code == 82 or code == 86 end

function Sky.compass(degrees)
    if not degrees then return "—" end
    return COMPASS[math.floor((degrees % 360 + 22.5) / 45) % 8 + 1]
end

-- The smallest angle between two bearings, 0..180.
function Sky.angle(a, b)
    local d = math.abs((a - b) % 360)
    return d > 180 and 360 - d or d
end

-- Moon phase at a UTC epoch time: fraction 0..1 of the synodic month
-- (0 new, 0.5 full), the lit share of the disc and a name.
local SYNODIC = 29.530588853
local NEW_MOON = 947182440 -- 2000-01-06 18:14 UTC
local PHASES = {
    { 0.0339, "new moon" }, { 0.2161, "waxing crescent" }, { 0.2839, "first quarter" },
    { 0.4661, "waxing gibbous" }, { 0.5339, "full moon" }, { 0.7161, "waning gibbous" },
    { 0.7839, "last quarter" }, { 0.9661, "waning crescent" }, { 1.0001, "new moon" },
}

function Sky.moon(epoch)
    local days = (epoch - NEW_MOON) / 86400
    local phase = (days % SYNODIC) / SYNODIC
    local name
    for _, entry in ipairs(PHASES) do
        if phase < entry[1] then name = entry[2]; break end
    end
    return { phase = phase, lit = (1 - math.cos(2 * math.pi * phase)) / 2, name = name,
        -- Days to the nearest new or full moon, when tides and feeding peak.
        daysFromSyzygy = math.min(phase, math.abs(phase - 0.5), 1 - phase) * SYNODIC }
end

-- Whether the moon is about above the horizon at a local hour. The moon
-- crosses the sky about 50 minutes later each day: overhead at noon when
-- new, at midnight when full, and up for roughly twelve hours around that.
-- Good to about an hour; latitude and season shift it.
function Sky.moonUp(phase, hourOfDay)
    local transit = (12 + phase * 24) % 24
    local d = math.abs(hourOfDay - transit) % 24
    return math.min(d, 24 - d) < 6
end

-- "2026-11-18T06:00" → hours and minutes, and a lowercase short weekday.
function Sky.clock(iso) return iso and iso:sub(12, 16) or "—" end
function Sky.hour(iso) return tonumber(iso and iso:sub(12, 13)) end
function Sky.minutes(iso)
    local h, m = (iso or ""):match("T(%d%d):(%d%d)")
    return h and tonumber(h) * 60 + tonumber(m)
end

local WEEKDAYS = { "sun", "mon", "tue", "wed", "thu", "fri", "sat" }
function Sky.weekday(date)
    local y, m, d = (date or ""):match("^(%d+)-(%d+)-(%d+)")
    if not y then return "—" end
    y, m, d = tonumber(y), tonumber(m), tonumber(d)
    -- Zeller-style day of week, 0 = Sunday.
    local t = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 }
    if m < 3 then y = y - 1 end
    return WEEKDAYS[(y + y // 4 - y // 100 + y // 400 + t[m] + d) % 7 + 1] .. " " .. tostring(d)
end

return Sky
