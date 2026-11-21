-- The bright planets, Mercury to Saturn: where each stands, how bright it
-- is, and when it is up during tonight's dark hours. Paul Schlyter's
-- "How to compute planetary positions" (the same theory as moon.lua):
-- mean orbital elements of the date plus the largest Jupiter–Saturn
-- perturbations, good to a few arcminutes, which puts rise and set within
-- a minute or two. Computed on the device; no service. Pure functions:
-- tests load it without the app.
local Sky = require "Weather/sky"
local Moon = require "Weather/editions/stargazing/moon"
local Planets = {}

local rad, deg = math.pi / 180, 180 / math.pi
local sin, cos, atan, asin, acos, sqrt = math.sin, math.cos, math.atan, math.asin, math.acos, math.sqrt
local function log10(x) return math.log(x, 10) end

-- A planet's centre at rise and set: refraction (34'), no disc to speak of.
Planets.HORIZON = -0.567
-- Seen at all: this high, with the sun at least 6° down.
Planets.LOW, Planets.SUN = 5, -6

-- Elements as { value at day 0, change per day } (Schlyter's table).
-- `mag(r, R, fv)` is the visual magnitude from the distances to the sun
-- and the Earth (AU) and the phase angle (degrees).
Planets.ORDER = { "Mercury", "Venus", "Mars", "Jupiter", "Saturn" }
local ELEMENTS = {
    Mercury = { N = { 48.3313, 3.24587e-5 }, i = { 7.0047, 5.00e-8 }, w = { 29.1241, 1.01444e-5 }, a = 0.387098,
        e = { 0.205635, 5.59e-10 }, M = { 168.6562, 4.0923344368 },
        mag = function(r, R, fv) return -0.36 + 5 * log10(r * R) + 0.027 * fv + 2.2e-13 * fv ^ 6 end },
    Venus = { N = { 76.6799, 2.46590e-5 }, i = { 3.3946, 2.75e-8 }, w = { 54.8910, 1.38374e-5 }, a = 0.723330,
        e = { 0.006773, -1.302e-9 }, M = { 48.0052, 1.6021302244 },
        mag = function(r, R, fv) return -4.34 + 5 * log10(r * R) + 0.013 * fv + 4.2e-7 * fv ^ 3 end },
    Mars = { N = { 49.5574, 2.11081e-5 }, i = { 1.8497, -1.78e-8 }, w = { 286.5016, 2.92961e-5 }, a = 1.523688,
        e = { 0.093405, 2.516e-9 }, M = { 18.6021, 0.5240207766 },
        mag = function(r, R, fv) return -1.51 + 5 * log10(r * R) + 0.016 * fv end },
    Jupiter = { N = { 100.4542, 2.76854e-5 }, i = { 1.3030, -1.557e-7 }, w = { 273.8777, 1.64505e-5 }, a = 5.20256,
        e = { 0.048498, 4.469e-9 }, M = { 19.8950, 0.0830853001 },
        mag = function(r, R, fv) return -9.25 + 5 * log10(r * R) + 0.014 * fv end },
    Saturn = { N = { 113.6634, 2.38980e-5 }, i = { 2.4886, -1.081e-7 }, w = { 339.3939, 2.97661e-5 }, a = 9.55475,
        e = { 0.055546, -9.499e-9 }, M = { 316.9670, 0.0334442282 },
        mag = function(r, R, fv) return -9.0 + 5 * log10(r * R) + 0.044 * fv end },
}

local function dayNumber(epoch) return epoch / 86400 - 10956 end
local function at(pair, d) return pair[1] + pair[2] * d end

-- Kepler's equation for an eccentric anomaly (degrees), to 1e-6°.
local function anomaly(m, e)
    local E = m + e * deg * sin(m * rad) * (1 + e * cos(m * rad))
    for _ = 1, 8 do
        local step = (E - e * deg * sin(E * rad) - m) / (1 - e * cos(E * rad))
        E = E - step
        if math.abs(step) < 1e-6 then break end
    end
    return E
end

-- Heliocentric ecliptic longitude, latitude (degrees) and distance (AU).
local function orbit(el, d)
    local N, i, w, a, e = at(el.N, d) * rad, at(el.i, d) * rad, at(el.w, d), el.a, at(el.e, d)
    local m = at(el.M, d) % 360
    local E = anomaly(m, e) * rad
    local xv, yv = a * (cos(E) - e), a * sqrt(1 - e * e) * sin(E)
    local v, r = atan(yv, xv) * deg, sqrt(xv * xv + yv * yv)
    local vw = (v + w) * rad
    local x = r * (cos(N) * cos(vw) - sin(N) * sin(vw) * cos(i))
    local y = r * (sin(N) * cos(vw) + cos(N) * sin(vw) * cos(i))
    local z = r * sin(vw) * sin(i)
    return atan(y, x) * deg % 360, atan(z, sqrt(x * x + y * y)) * deg, r, m
end

-- The sun as seen from the Earth: ecliptic longitude and distance (AU).
local function sun(d)
    local w, e, m = 282.9404 + 4.70935e-5 * d, 0.016709 - 1.151e-9 * d, (356.0470 + 0.9856002585 * d) % 360
    local E = anomaly(m, e) * rad
    local xv, yv = cos(E) - e, sqrt(1 - e * e) * sin(E)
    return (atan(yv, xv) * deg + w) % 360, sqrt(xv * xv + yv * yv)
end

-- Jupiter and Saturn pull on each other (the "great inequality").
local function perturb(name, lon, lat, d)
    if name ~= "Jupiter" and name ~= "Saturn" then return lon, lat end
    local Mj, Ms = at(ELEMENTS.Jupiter.M, d) * rad, at(ELEMENTS.Saturn.M, d) * rad
    local function s(x) return sin(x * rad) end
    local function c(x) return cos(x * rad) end
    local J, S = Mj * deg, Ms * deg
    if name == "Jupiter" then
        lon = lon - 0.332 * s(2 * J - 5 * S - 67.6) - 0.056 * s(2 * J - 2 * S + 21) + 0.042 * s(3 * J - 5 * S + 21)
            - 0.036 * s(J - 2 * S) + 0.022 * c(J - S) + 0.023 * s(2 * J - 3 * S + 52) - 0.016 * s(J - 5 * S - 69)
    else
        lon = lon + 0.812 * s(2 * J - 5 * S - 67.6) - 0.229 * c(2 * J - 4 * S - 2) + 0.119 * s(J - 2 * S - 3)
            + 0.046 * s(2 * J - 6 * S - 69) + 0.014 * s(J - 3 * S + 32)
        lat = lat - 0.020 * c(2 * J - 4 * S - 2) + 0.018 * s(2 * J - 6 * S - 49)
    end
    return lon, lat
end

-- Where a planet stands at a UTC epoch, seen from the Earth's centre:
-- { ra, dec, lon, lat (ecliptic of date), r (from the sun), R (from the
-- Earth), elongation (from the sun), sunLon, mag }.
function Planets.position(name, epoch)
    local el = ELEMENTS[name]
    local d = dayNumber(epoch)
    local hlon, hlat, r = orbit(el, d)
    hlon, hlat = perturb(name, hlon, hlat, d)
    local x = r * cos(hlon * rad) * cos(hlat * rad)
    local y = r * sin(hlon * rad) * cos(hlat * rad)
    local z = r * sin(hlat * rad)
    local sunLon, rs = sun(d)
    x, y = x + rs * cos(sunLon * rad), y + rs * sin(sunLon * rad)
    local R = sqrt(x * x + y * y + z * z)
    local lon, lat = atan(y, x) * deg % 360, asin(z / R) * deg
    local ecl = (23.4393 - 3.563e-7 * d) * rad
    local ye, ze = y * cos(ecl) - z * sin(ecl), y * sin(ecl) + z * cos(ecl)
    local clamp = function(v) return math.max(-1, math.min(1, v)) end
    local elongation = acos(clamp((rs * rs + R * R - r * r) / (2 * rs * R))) * deg
    local fv = acos(clamp((r * r + R * R - rs * rs) / (2 * r * R))) * deg
    local mag = el.mag(r, R, fv)
    if name == "Saturn" then
        -- The rings add light by how open they are to us.
        local ir, Nr = 28.06 * rad, (169.51 + 3.82e-5 * d) * rad
        local B = asin(clamp(sin(lat * rad) * cos(ir) - cos(lat * rad) * sin(ir) * sin(lon * rad - Nr)))
        mag = mag - 2.6 * sin(math.abs(B)) + 1.2 * sin(B) ^ 2
    end
    return { ra = atan(ye, x) * deg % 360, dec = atan(ze, sqrt(x * x + ye * ye)) * deg, lon = lon, lat = lat, r = r, R = R,
        elongation = elongation, sunLon = sunLon, mag = mag }
end

-- Height and bearing (degrees, from north) of a right ascension and
-- declination at a place and UTC epoch.
function Planets.horizontal(ra, dec, epoch, lat, lon)
    local gmst = 280.46061837 + 360.98564736629 * (epoch / 86400 - 10957.5)
    local H, la, de = (gmst + lon - ra) * rad, lat * rad, dec * rad
    local altitude = asin(sin(la) * sin(de) + cos(la) * cos(de) * cos(H)) * deg
    local azimuth = (atan(sin(H), cos(H) * sin(la) - math.tan(de) * cos(la)) * deg + 180) % 360
    return altitude, azimuth
end

function Planets.altitude(name, epoch, lat, lon)
    local p = Planets.position(name, epoch)
    return Planets.horizontal(p.ra, p.dec, epoch, lat, lon)
end

-- The angle between two points on the sky (degrees).
function Planets.separation(ra1, dec1, ra2, dec2)
    local c = sin(dec1 * rad) * sin(dec2 * rad) + cos(dec1 * rad) * cos(dec2 * rad) * cos((ra1 - ra2) * rad)
    return acos(math.max(-1, math.min(1, c))) * deg
end

local STEP = 600
local nights = {}

-- Each planet during a night (Night.frame), brightest first, only those
-- seen at all (Planets.LOW up with the sun Planets.SUN down):
-- { name, mag, from, to, best = { epoch, altitude, azimuth }, rise, set,
--   moon = degrees to the moon at its best when the moon is up, near = { name, degrees } }.
function Planets.night(frame, spot)
    if not (frame and frame.from and frame.to) then return {} end
    local key = ("%.3f|%.3f|%d|%d"):format(spot.lat, spot.lon, frame.from // STEP, frame.to // STEP)
    if nights[key] then return nights[key] end
    local list = {}
    for _, name in ipairs(Planets.ORDER) do
        local entry, previous = { name = name }, nil
        local epoch = frame.from
        while epoch <= frame.to do
            local p = Planets.position(name, epoch)
            local altitude, azimuth = Planets.horizontal(p.ra, p.dec, epoch, spot.lat, spot.lon)
            local margin = altitude - Planets.HORIZON
            if previous and (previous > 0) ~= (margin > 0) then
                local edge = epoch - STEP + STEP * previous / (previous - margin)
                if margin > 0 then entry.rise = entry.rise or edge else entry.set = entry.set or edge end
            end
            previous = margin
            if altitude >= Planets.LOW and Sky.sunAltitude(epoch, spot.lat, spot.lon) <= Planets.SUN then
                entry.from, entry.to = entry.from or epoch, epoch
                if not entry.best or altitude > entry.best.altitude then
                    entry.best = { epoch = epoch, altitude = altitude, azimuth = azimuth, ra = p.ra, dec = p.dec }
                    entry.mag = p.mag
                end
            end
            epoch = epoch + STEP
        end
        if entry.best then list[#list + 1] = entry end
    end
    -- The moon and the other planets near each one at its best.
    for _, entry in ipairs(list) do
        local best = entry.best
        local moon = Moon.position(best.epoch)
        if Moon.altitude(best.epoch, spot.lat, spot.lon) > 0 then
            entry.moon = Planets.separation(best.ra, best.dec, moon.ra, moon.dec)
        end
        for _, other in ipairs(list) do
            if other ~= entry then
                local p = Planets.position(other.name, best.epoch)
                local apart = Planets.separation(best.ra, best.dec, p.ra, p.dec)
                if not entry.near or apart < entry.near.degrees then entry.near = { name = other.name, degrees = apart } end
            end
        end
    end
    table.sort(list, function(a, b) return a.mag < b.mag end)
    nights[key] = list
    return list
end

-- "−2.4", "0.8": a magnitude as people write it.
function Planets.magnitude(mag) return (("%.1f"):format(mag):gsub("^%-", "−")) end

return Planets
