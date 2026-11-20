-- Things the sky does that every weather profile reads: WMO weather codes,
-- compass points, angles, the phase of the moon and the sun's height with
-- the twilights it makes. Pure functions, so the
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
-- Clear and partly cloudy skies show the moon after dark (`isDay == false`).
local NIGHT = { sun = "moon", ["partly-cloudy"] = "cloud-moon" }
function Sky.icon(code, isDay)
    local name = ICONS[code] or "cloud"
    if isDay == false then name = NIGHT[name] or name end
    return "Weather/assets/icons/weather/" .. name .. ".svg"
end
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

-- Days since 1970-01-01 for a civil date (proleptic Gregorian).
local function daysFromCivil(y, m, d)
    y = m <= 2 and y - 1 or y
    local era = (y >= 0 and y or y - 399) // 400
    local yoe = y - era * 400
    local doy = (153 * (m + (m > 2 and -3 or 9)) + 2) // 5 + d - 1
    return era * 146097 + yoe * 365 + yoe // 4 - yoe // 100 + doy - 719468
end

-- The UTC epoch of a date's midnight ("2026-11-18"), or nil.
function Sky.epochOf(date)
    local y, m, d = (date or ""):match("^(%d+)-(%d+)-(%d+)")
    if not y then return nil end
    return daysFromCivil(tonumber(y), tonumber(m), tonumber(d)) * 86400
end

-- The sun's height above the horizon in degrees at a UTC epoch, without
-- refraction (NOAA's solar calculator, good to a few hundredths of a degree).
function Sky.sunAltitude(epoch, lat, lon)
    local rad, deg = math.pi / 180, 180 / math.pi
    local t = (epoch / 86400 + 2440587.5 - 2451545) / 36525
    local l0 = (280.46646 + t * (36000.76983 + t * 0.0003032)) % 360
    local m = 357.52911 + t * (35999.05029 - 0.0001537 * t)
    local e = 0.016708634 - t * (0.000042037 + 0.0000001267 * t)
    local c = math.sin(m * rad) * (1.914602 - t * (0.004817 + 0.000014 * t))
        + math.sin(2 * m * rad) * (0.019993 - 0.000101 * t) + math.sin(3 * m * rad) * 0.000289
    local omega = 125.04 - 1934.136 * t
    local lambda = l0 + c - 0.00569 - 0.00478 * math.sin(omega * rad)
    local eps0 = 23 + (26 + (21.448 - t * (46.815 + t * (0.00059 - t * 0.001813))) / 60) / 60
    local eps = eps0 + 0.00256 * math.cos(omega * rad)
    local decl = math.asin(math.sin(eps * rad) * math.sin(lambda * rad))
    local y = math.tan(eps * rad / 2) ^ 2
    local eqTime = 4 * deg * (y * math.sin(2 * l0 * rad) - 2 * e * math.sin(m * rad)
        + 4 * e * y * math.sin(m * rad) * math.cos(2 * l0 * rad) - 0.5 * y * y * math.sin(4 * l0 * rad)
        - 1.25 * e * e * math.sin(2 * m * rad))
    local minutes = (epoch % 86400) / 60
    local hourAngle = ((minutes + eqTime + 4 * lon) / 4 - 180) * rad
    local cosZenith = math.sin(lat * rad) * math.sin(decl) + math.cos(lat * rad) * math.cos(decl) * math.cos(hourAngle)
    return 90 - math.acos(math.max(-1, math.min(1, cosZenith))) * deg
end

-- How far below the horizon the sun is at each twilight's edge.
Sky.TWILIGHT = { sunrise = -0.833, civil = -6, nautical = -12, astronomical = -18 }

local twilights, STEP = {}, 300

-- Dawn and dusk on a local date for a place, `offset` seconds east of UTC:
--
--   { sunrise = "…T07:28", sunset = "…T16:05", polar = nil | "day" | "night",
--     civil = { dawn = "…T06:52", dusk = "…T16:41", dawnEpoch, duskEpoch },
--     nautical = { … }, astronomical = { … } }
--
-- A level is nil when the sun never gets that low that day (no full
-- darkness in a northern summer); it is `{ dark = true }` when the sun never
-- gets that high (polar night). A dawn or dusk that falls on the next date
-- is missing from this one. Times are local, like the forecast's.
function Sky.twilight(date, lat, lon, offset)
    offset = offset or 0
    local key = ("%s|%.3f|%.3f|%d"):format(date, lat, lon, offset)
    if twilights[key] then return twilights[key] end
    local midnight = Sky.epochOf(date)
    if not midnight then return nil end
    local start = midnight - offset
    local samples, low, high = {}, math.huge, -math.huge
    for index = 0, 86400 // STEP do
        local altitude = Sky.sunAltitude(start + index * STEP, lat, lon)
        samples[index] = altitude
        low, high = math.min(low, altitude), math.max(high, altitude)
    end
    local function at(epoch) return os.date("!%Y-%m-%dT%H:%M", math.floor(epoch + offset + 30)) end
    local function edge(limit)
        if low > limit then return nil end
        if high < limit then return { dark = true } end
        -- Dawn is the first rise through the limit; dusk the first fall after
        -- it (a fall before dawn ends the previous evening).
        local dawn, dusk
        for index = 1, #samples do
            local a, b = samples[index - 1], samples[index]
            local epoch = start + (index - 1 + (limit - a) / (b - a)) * STEP
            if a < limit and b >= limit and not dawn then dawn = epoch
            elseif a >= limit and b < limit and (dawn or not dusk) and not (dawn and dusk and dusk > dawn) then
                dusk = epoch
            end
        end
        if dawn and dusk and dusk < dawn then dusk = nil end
        return { dawnEpoch = dawn, duskEpoch = dusk, dawn = dawn and at(dawn), dusk = dusk and at(dusk) }
    end
    local rise = edge(Sky.TWILIGHT.sunrise)
    local result = { civil = edge(Sky.TWILIGHT.civil), nautical = edge(Sky.TWILIGHT.nautical),
        astronomical = edge(Sky.TWILIGHT.astronomical),
        polar = rise == nil and "day" or rise.dark and "night" or nil,
        sunrise = rise and rise.dawn, sunset = rise and rise.dusk }
    twilights[key] = result
    return result
end

return Sky
