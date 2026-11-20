-- The moon's real rise, set and transit for a place (FI8), and the solunar
-- periods anglers' tradition builds on them. The position uses the
-- Astronomical Almanac's low-precision series (about 0.3°), good to a few
-- minutes for rise and set. Solunar tables are tradition: the major periods
-- are the two hours around the moon overhead and underfoot, the minor ones
-- the hour around moonrise and moonset. Studies have not found the effect.
local Sky = require "Weather/sky"
local Moon = {}

local rad, deg = math.pi / 180, 180 / math.pi
local function sin(d) return math.sin(d * rad) end
local function cos(d) return math.cos(d * rad) end

-- Right ascension and declination (degrees) and horizontal parallax at a
-- UTC epoch, with the ecliptic longitude for checks.
function Moon.position(epoch)
    local jd = epoch / 86400 + 2440587.5
    local t = (jd - 2451545.0) / 36525
    local lambda = 218.32 + 481267.881 * t + 6.29 * sin(135.0 + 477198.87 * t) - 1.27 * sin(259.3 - 413335.36 * t)
        + 0.66 * sin(235.7 + 890534.22 * t) + 0.21 * sin(269.9 + 954397.74 * t) - 0.19 * sin(357.5 + 35999.05 * t)
        - 0.11 * sin(186.5 + 966404.03 * t)
    local beta = 5.13 * sin(93.3 + 483202.02 * t) + 0.28 * sin(228.2 + 960400.89 * t)
        - 0.28 * sin(318.3 + 6003.15 * t) - 0.17 * sin(217.6 - 407332.21 * t)
    local parallax = 0.9508 + 0.0518 * cos(135.0 + 477198.87 * t) + 0.0095 * cos(259.3 - 413335.36 * t)
        + 0.0078 * cos(235.7 + 890534.22 * t) + 0.0028 * cos(269.9 + 954397.74 * t)
    local l = cos(beta) * cos(lambda)
    local m = 0.9175 * cos(beta) * sin(lambda) - 0.3978 * sin(beta)
    local n = 0.3978 * cos(beta) * sin(lambda) + 0.9175 * sin(beta)
    return { ra = math.atan(m, l) * deg % 360, dec = math.asin(n) * deg, parallax = parallax,
        longitude = lambda % 360, jd = jd }
end

-- The moon's hour angle (−180..180, 0 overhead) and height above the
-- horizon (degrees, geocentric) at a UTC epoch for a place.
function Moon.sky(epoch, lat, lon)
    local p = Moon.position(epoch)
    local gmst = 280.46061837 + 360.98564736629 * (p.jd - 2451545.0)
    local h = (gmst + lon - p.ra + 180) % 360 - 180
    local altitude = math.asin(sin(lat) * sin(p.dec) + cos(lat) * cos(p.dec) * cos(h)) * deg
    return h, altitude, p.parallax
end

local STEP = 600 -- seconds between samples; crossings are refined below

local function iso(epoch, offset) return os.date("!%Y-%m-%dT%H:%M", math.floor(epoch + offset + 30)) end

-- Bisects a sign change of f between two epochs.
local function refine(f, a, b, fa)
    for _ = 1, 12 do
        local mid = (a + b) / 2
        local fm = f(mid)
        if (fm > 0) == (fa > 0) then a, fa = mid, fm else b = mid end
    end
    return (a + b) / 2
end

-- One local day's moon: { rise, set, transit, underfoot } as local
-- "YYYY-MM-DDTHH:MM" (nil when it does not happen that day: the moon rises
-- about 50 minutes later each day, so some days miss one).
function Moon.day(date, lat, lon, offset)
    offset = offset or 0
    local start = Sky.epochOf(date)
    if not start then return {} end
    start = start - offset
    -- The horizon for the moon's centre: refraction and semidiameter lift
    -- it, parallax lowers it.
    local function above(epoch)
        local _, altitude, parallax = Moon.sky(epoch, lat, lon)
        return altitude - (0.7275 * parallax - 0.5667)
    end
    local function angle(epoch) local h = Moon.sky(epoch, lat, lon); return h end
    local function underfoot(epoch) local h = Moon.sky(epoch, lat, lon); return (h + 360) % 360 - 180 end
    local result = {}
    local a, fa, ha, ua = start, above(start), angle(start), underfoot(start)
    for epoch = start + STEP, start + 86400, STEP do
        local fb, hb, ub = above(epoch), angle(epoch), underfoot(epoch)
        if (fa > 0) ~= (fb > 0) then
            local at = refine(above, a, epoch, fa)
            result[fb > 0 and "rise" or "set"] = result[fb > 0 and "rise" or "set"] or iso(at, offset)
        end
        if ha < 0 and hb >= 0 and hb - ha < 90 then result.transit = result.transit or iso(refine(angle, a, epoch, ha), offset) end
        if ua < 0 and ub >= 0 and ub - ua < 90 then
            result.underfoot = result.underfoot or iso(refine(underfoot, a, epoch, ua), offset)
        end
        a, fa, ha, ua = epoch, fb, hb, ub
    end
    return result
end

local days = {}
-- Moon.day, remembered per place and date.
function Moon.cached(date, lat, lon, offset)
    local key = ("%s|%.3f|%.3f|%d"):format(date, lat, lon, offset or 0)
    if not days[key] then days[key] = Moon.day(date, lat, lon, offset) end
    return days[key]
end

-- The traditional solunar periods of a day: majors two hours around the
-- moon overhead and underfoot, minors one hour around rise and set. Each is
-- { kind = "major" | "minor", at, from, to } in local minutes of the day,
-- in time order.
function Moon.solunar(day)
    local periods = {}
    local function add(kind, at, half)
        local minutes = Sky.minutes(at)
        if minutes then periods[#periods + 1] = { kind = kind, at = at, from = minutes - half, to = minutes + half } end
    end
    add("major", day.transit, 60); add("major", day.underfoot, 60)
    add("minor", day.rise, 30); add("minor", day.set, 30)
    table.sort(periods, function(x, y) return x.from < y.from end)
    return periods
end

-- The solunar period an hour falls in (by its start), or nil.
function Moon.periodAt(periods, time)
    local minutes = Sky.minutes(time)
    if not minutes then return nil end
    for _, period in ipairs(periods) do
        if minutes + 59 >= period.from and minutes <= period.to then return period end
    end
    return nil
end

return Moon
