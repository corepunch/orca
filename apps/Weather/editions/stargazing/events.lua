-- Sky events on a calendar: the major meteor showers and the lunar
-- eclipses of the next ten years, as small static tables, each rated for a
-- place by moonlight (moon.lua) and by the radiant's or the moon's height.
-- No service. Refresh the shower dates once a year from the IMO calendar.
--
-- Showers: peak night, active window, ZHR and radiant after the
-- International Meteor Organization's Meteor Shower Calendar and the IAU
-- Meteor Data Center (facts, written here in our own table). Peaks drift
-- by a day between years; `peak` is the night (the evening's date) of the
-- usual maximum. Lunar eclipses: greatest eclipse (UTC) after "Eclipse
-- Predictions by Fred Espenak, NASA/GSFC Emeritus" (eclipse.gsfc.nasa.gov),
-- partial and total only; penumbral ones barely show.
local Sky = require "Weather/sky"
local Moon = require "Weather/editions/stargazing/moon"
local Planets = require "Weather/editions/stargazing/planets"
local Events = {}

Events.SHOWERS = {
    { id = "quadrantids", name = "Quadrantids", from = "12-28", to = "01-12", peak = "01-03", zhr = 80, ra = 230, dec = 49 },
    { id = "lyrids", name = "Lyrids", from = "04-14", to = "04-30", peak = "04-21", zhr = 18, ra = 271, dec = 34 },
    { id = "eta-aquariids", name = "Eta Aquariids", from = "04-19", to = "05-28", peak = "05-05", zhr = 50, ra = 338, dec = -1 },
    { id = "delta-aquariids", name = "Southern Delta Aquariids", from = "07-12", to = "08-23", peak = "07-30", zhr = 25,
        ra = 340, dec = -16 },
    { id = "alpha-capricornids", name = "Alpha Capricornids", from = "07-03", to = "08-15", peak = "07-30", zhr = 5,
        ra = 307, dec = -10 },
    { id = "perseids", name = "Perseids", from = "07-17", to = "08-24", peak = "08-12", zhr = 100, ra = 48, dec = 58 },
    { id = "draconids", name = "Draconids", from = "10-06", to = "10-10", peak = "10-08", zhr = 10, ra = 262, dec = 54 },
    { id = "orionids", name = "Orionids", from = "10-02", to = "11-07", peak = "10-20", zhr = 20, ra = 95, dec = 16 },
    { id = "leonids", name = "Leonids", from = "11-06", to = "11-30", peak = "11-16", zhr = 15, ra = 152, dec = 22 },
    { id = "geminids", name = "Geminids", from = "12-04", to = "12-20", peak = "12-13", zhr = 150, ra = 112, dec = 33 },
    { id = "ursids", name = "Ursids", from = "12-17", to = "12-26", peak = "12-21", zhr = 10, ra = 217, dec = 76 },
}

-- Greatest eclipse in UTC. Verify against eclipse.gsfc.nasa.gov when the
-- table is refreshed; tests check each is a full moon near a node.
Events.LUNAR_ECLIPSES = {
    { at = "2026-03-03T11:33", kind = "total" }, { at = "2026-08-28T04:13", kind = "partial" },
    { at = "2028-01-12T04:13", kind = "partial" }, { at = "2028-07-06T18:19", kind = "partial" },
    { at = "2028-12-31T16:52", kind = "total" }, { at = "2029-06-26T03:22", kind = "total" },
    { at = "2029-12-20T22:42", kind = "total" }, { at = "2030-06-15T18:33", kind = "partial" },
    { at = "2032-04-25T15:13", kind = "total" }, { at = "2032-10-18T19:02", kind = "total" },
    { at = "2033-04-14T19:13", kind = "total" }, { at = "2033-10-08T10:55", kind = "total" },
    { at = "2034-09-28T02:46", kind = "partial" }, { at = "2035-08-19T01:11", kind = "partial" },
    { at = "2036-02-11T22:12", kind = "total" }, { at = "2036-08-07T02:52", kind = "total" },
}

local function epochOf(iso) return Sky.epochOf(iso) + (Sky.minutes(iso) or 0) * 60 end
Events.epochOf = epochOf

local DAY = 86400
-- The night's evening date as a local date ("2026-08-12").
local function evening(frame) return os.date("!%Y-%m-%d", math.floor(frame.from + (frame.offset or 0))) end

-- How many nights a date is from a shower's peak in the nearest year
-- (negative before), and whether it is inside the active window.
local function fromPeak(shower, date)
    local year, best = tonumber(date:sub(1, 4)), nil
    for y = year - 1, year + 1 do
        local days = (Sky.epochOf(date) - Sky.epochOf(("%d-%s"):format(y, shower.peak))) // DAY
        if not best or math.abs(days) < math.abs(best) then best = days end
    end
    return best
end
Events.fromPeak = fromPeak

-- The showers near their peak on a night: within two nights before and
-- one after, the strongest first.
function Events.showersOn(frame)
    local date, list = evening(frame), {}
    for _, shower in ipairs(Events.SHOWERS) do
        local days = fromPeak(shower, date)
        if days >= -2 and days <= 1 then list[#list + 1] = { shower = shower, days = days } end
    end
    table.sort(list, function(a, b) return a.shower.zhr > b.shower.zhr end)
    return list
end

-- What a stargazer can expect of a shower on a night (Night.frame) at a
-- spot: the best ten minutes in the dark (sun 12° down), the rate there,
-- ZHR scaled by the radiant's height, by how far from the peak, and by
-- moonlight: { rate, at, radiant, moonLit, moonUp }, or nil with no dark
-- hour at all. An estimate: real rates vary from year to year.
function Events.rate(shower, frame, spot, days)
    if not (frame and frame.from) then return nil end
    local best
    local epoch = frame.from
    while epoch <= frame.to do
        if Sky.sunAltitude(epoch, spot.lat, spot.lon) <= -12 then
            local radiant = Planets.horizontal(shower.ra, shower.dec, epoch, spot.lat, spot.lon)
            local moonHeight = Moon.altitude(epoch, spot.lat, spot.lon)
            local lit = Moon.phase(epoch).lit
            local moonlight = moonHeight > 0 and 1 - 0.7 * lit * math.min(1, (moonHeight + 10) / 25) or 1
            local rate = radiant > 0 and shower.zhr * math.sin(radiant * math.pi / 180) * moonlight * 0.5 ^ math.abs(days or 0) or 0
            if not best or rate > best.rate then
                best = { rate = rate, at = epoch, radiant = radiant, moonUp = moonHeight > 0, moonLit = lit }
            end
        end
        epoch = epoch + 600
    end
    return best
end

-- The lunar eclipse whose greatest moment falls in a night, or nil:
-- { kind, at, altitude } with the moon's height then; nil when the moon
-- is below the horizon or the sun is up then.
function Events.eclipseIn(frame, spot)
    if not (frame and frame.from) then return nil end
    for _, eclipse in ipairs(Events.LUNAR_ECLIPSES) do
        local at = epochOf(eclipse.at)
        if at >= frame.from and at <= frame.to then
            local altitude = Moon.altitude(at, spot.lat, spot.lon)
            if altitude > 0 and Sky.sunAltitude(at, spot.lat, spot.lon) < -0.833 then
                return { kind = eclipse.kind, at = at, altitude = altitude }
            end
        end
    end
end

-- The next lunar eclipse seen from a spot after an epoch, within `days`.
function Events.nextEclipse(spot, epoch, days)
    for _, eclipse in ipairs(Events.LUNAR_ECLIPSES) do
        local at = epochOf(eclipse.at)
        if at > epoch and at <= epoch + (days or 7) * DAY then
            local altitude = Moon.altitude(at, spot.lat, spot.lon)
            if altitude > 0 and Sky.sunAltitude(at, spot.lat, spot.lon) < -0.833 then
                return { kind = eclipse.kind, at = at, altitude = altitude }
            end
        end
    end
end

return Events
