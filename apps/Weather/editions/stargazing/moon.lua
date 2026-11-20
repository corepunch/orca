-- Where the moon is: its height over a place at any minute, how much of it
-- is lit, and when it rises and sets on a local date. Paul Schlyter's
-- low-precision lunar theory ("How to compute planetary positions": mean
-- orbit plus the twelve largest perturbations, about 2' in longitude),
-- corrected for parallax, so rise and set land within a few minutes of
-- published tables. Pure functions: tests load it without the app.
local Sky = require "Weather/sky"
local Moon = {}

local rad, deg = math.pi / 180, 180 / math.pi
local sin, cos, atan, asin, sqrt = math.sin, math.cos, math.atan, math.asin, math.sqrt

-- The moon's topocentric centre at rise and set: refraction (34') and its
-- half-disc (about 15') under the horizon.
Moon.HORIZON = -0.833

-- Days since 2000 Jan 0.0 UT, Schlyter's day number.
local function dayNumber(epoch) return epoch / 86400 - 10956 end

-- The sun's ecliptic longitude (degrees) and the moon's ecliptic longitude,
-- latitude (degrees) and distance (Earth radii) at a day number.
local function ecliptic(d)
    local ws, ms = 282.9404 + 4.70935e-5 * d, (356.0470 + 0.9856002585 * d) % 360
    local sunLon = (ws + ms + 1.915 * sin(ms * rad) + 0.020 * sin(2 * ms * rad)) % 360
    local n = (125.1228 - 0.0529538083 * d) % 360
    local i, w, a, e = 5.1454, (318.0634 + 0.1643573223 * d) % 360, 60.2666, 0.054900
    local m = (115.3654 + 13.0649929509 * d) % 360
    -- Kepler's equation, three Newton steps from a good start.
    local E = m + e * deg * sin(m * rad) * (1 + e * cos(m * rad))
    for _ = 1, 3 do E = E - (E - e * deg * sin(E * rad) - m) / (1 - e * cos(E * rad)) end
    local xv, yv = a * (cos(E * rad) - e), a * sqrt(1 - e * e) * sin(E * rad)
    local v, r = atan(yv, xv) * deg, sqrt(xv * xv + yv * yv)
    local vw, nr, ir = (v + w) * rad, n * rad, i * rad
    local xh = r * (cos(nr) * cos(vw) - sin(nr) * sin(vw) * cos(ir))
    local yh = r * (sin(nr) * cos(vw) + cos(nr) * sin(vw) * cos(ir))
    local zh = r * sin(vw) * sin(ir)
    local lon, lat = atan(yh, xh) * deg, atan(zh, sqrt(xh * xh + yh * yh)) * deg
    -- The largest perturbations by the sun.
    local Ls, Lm = ms + ws, m + w + n
    local D, F, Mm, Ms = (Lm - Ls) * rad, (Lm - n) * rad, m * rad, ms * rad
    lon = lon - 1.274 * sin(Mm - 2 * D) + 0.658 * sin(2 * D) - 0.186 * sin(Ms) - 0.059 * sin(2 * Mm - 2 * D)
        - 0.057 * sin(Mm - 2 * D + Ms) + 0.053 * sin(Mm + 2 * D) + 0.046 * sin(2 * D - Ms) + 0.041 * sin(Mm - Ms)
        - 0.035 * sin(D) - 0.031 * sin(Mm + Ms) - 0.015 * sin(2 * F - 2 * D) + 0.011 * sin(Mm - 4 * D)
    lat = lat - 0.173 * sin(F - 2 * D) - 0.055 * sin(Mm - F - 2 * D) - 0.046 * sin(Mm + F - 2 * D)
        + 0.033 * sin(F + 2 * D) + 0.017 * sin(2 * Mm + F)
    r = r - 0.58 * cos(Mm - 2 * D) - 0.46 * cos(2 * D)
    return sunLon, lon % 360, lat, r
end

-- Right ascension and declination (degrees) and distance (Earth radii).
function Moon.position(epoch)
    local d = dayNumber(epoch)
    local _, lon, lat, r = ecliptic(d)
    local ecl = (23.4393 - 3.563e-7 * d) * rad
    local x = cos(lon * rad) * cos(lat * rad)
    local y = sin(lon * rad) * cos(lat * rad)
    local z = sin(lat * rad)
    local ye, ze = y * cos(ecl) - z * sin(ecl), y * sin(ecl) + z * cos(ecl)
    return { ra = atan(ye, x) * deg % 360, dec = atan(ze, sqrt(x * x + ye * ye)) * deg, distance = r,
        lon = lon, lat = lat }
end

-- The moon's height above the horizon in degrees, seen from the ground
-- (parallax included, refraction not), at a UTC epoch.
function Moon.altitude(epoch, lat, lon)
    local p = Moon.position(epoch)
    local gmst = 280.46061837 + 360.98564736629 * (epoch / 86400 - 10957.5)
    local hourAngle = (gmst + lon - p.ra) * rad
    local la, dec = lat * rad, p.dec * rad
    local altitude = asin(sin(la) * sin(dec) + cos(la) * cos(dec) * cos(hourAngle)) * deg
    return altitude - asin(1 / p.distance) * deg * cos(altitude * rad)
end

-- The share of the disc that is lit (0..1), the phase (0 new, 0.5 full,
-- from the sun–moon elongation) and its name.
local NAMES = {
    { 0.0339, "new moon" }, { 0.2161, "waxing crescent" }, { 0.2839, "first quarter" },
    { 0.4661, "waxing gibbous" }, { 0.5339, "full moon" }, { 0.7161, "waning gibbous" },
    { 0.7839, "last quarter" }, { 0.9661, "waning crescent" }, { 1.0001, "new moon" },
}
function Moon.phase(epoch)
    local sunLon, lon, lat = ecliptic(dayNumber(epoch))
    local elongation = (lon - sunLon) % 360
    local lit = (1 - cos(elongation * rad) * cos(lat * rad)) / 2
    local phase, name = elongation / 360
    for _, entry in ipairs(NAMES) do
        if phase < entry[1] then name = entry[2]; break end
    end
    return { phase = phase, lit = lit, name = name }
end

local STEP = 600
local events = {}

-- Moonrise and moonset on a local date, `offset` seconds east of UTC:
-- { rise = "…T14:03", set = "…T02:13", riseEpoch, setEpoch } with either
-- missing when it does not happen that date (the moon rises about 50
-- minutes later each day, so one date in about 29 has no rise), and
-- `always = "up" | "down"` when it neither rises nor sets (far north and
-- south). Linear between ten-minute samples: well under a minute.
function Moon.events(date, lat, lon, offset)
    offset = offset or 0
    local key = ("%s|%.3f|%.3f|%d"):format(date, lat, lon, offset)
    if events[key] then return events[key] end
    local midnight = Sky.epochOf(date)
    if not midnight then return {} end
    local start = midnight - offset
    local result, previous, up = {}, nil, nil
    for index = 0, 86400 // STEP do
        local epoch = start + index * STEP
        local altitude = Moon.altitude(epoch, lat, lon) - Moon.HORIZON
        if index == 0 then up = altitude > 0 end
        if previous and (previous <= 0) ~= (altitude <= 0) then
            local at = epoch - STEP + STEP * previous / (previous - altitude)
            local field = altitude > 0 and "rise" or "set"
            if not result[field] then
                result[field .. "Epoch"] = at
                result[field] = os.date("!%Y-%m-%dT%H:%M", math.floor(at + offset + 30))
            end
        end
        previous = altitude
    end
    if not (result.rise or result.set) then result.always = up and "up" or "down" end
    events[key] = result
    return result
end

return Moon
