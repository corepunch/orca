-- The northern (and southern) lights: NOAA SWPC's planetary Kp forecast
-- (public domain, three-hour steps, about three days ahead) against the
-- place's geomagnetic latitude. Kp alone sends people out under cloud and
-- in daylight, so the stars activity only speaks of it on a dark night,
-- and says when clouds are in the way. All of it is an estimate.
local Sky = require "Weather/sky"
local Aurora = {}

local function utcDate(format, epoch) return os.date(format, math.floor(epoch + 0.5)) end

Aurora.URL = "https://services.swpc.noaa.gov/products/noaa-planetary-k-index-forecast.json"
-- The geomagnetic north pole of the dipole field (IGRF, 2025).
Aurora.POLE = { lat = 80.8, lon = -72.6 }

-- Latitude in the Earth's magnetic frame (a dipole: good to a degree or two).
function Aurora.magneticLatitude(lat, lon)
    local r = math.pi / 180
    local p = Aurora.POLE
    local s = math.sin(lat * r) * math.sin(p.lat * r) + math.cos(lat * r) * math.cos(p.lat * r) * math.cos((lon - p.lon) * r)
    return math.asin(math.max(-1, math.min(1, s))) / r
end

-- The magnetic latitude where the oval's edge stands overhead at a Kp,
-- after NOAA's Kp table (66.5° at Kp 0 down to 48° at Kp 9).
function Aurora.edge(kp) return 66.5 - 2.04 * kp end

-- "overhead", "low" (low over the poleward horizon, up to about four
-- degrees beyond the edge) or nil, for a place and a Kp.
function Aurora.reach(lat, lon, kp)
    if not kp then return nil end
    local m, edge = math.abs(Aurora.magneticLatitude(lat, lon)), Aurora.edge(kp)
    if m >= edge then return "overhead" end
    if m >= edge - 4 then return "low" end
    return nil
end

-- "2026-11-22T18:00:00" or "2026-11-22 18:00:00" (UTC) → epoch.
local function epochOf(tag)
    local start, h = Sky.epochOf(tag), tonumber(tag:match("[T ](%d%d):"))
    return start and h and start + h * 3600
end

-- The Kp steps from NOAA's answer, as { { epoch, kp, observed }, … }. The
-- feed is a list of objects; it used to be a list of rows under a header
-- row, which is read too.
function Aurora.parseKp(json)
    local steps = {}
    if type(json) ~= "table" then return steps end
    for index, entry in ipairs(json) do
        local tag, kp, observed
        if type(entry) == "table" and entry.time_tag then tag, kp, observed = entry.time_tag, entry.kp, entry.observed
        elseif type(entry) == "table" and index > 1 then tag, kp, observed = entry[1], entry[2], entry[3] end
        kp = tonumber(kp)
        local epoch = type(tag) == "string" and epochOf(tag)
        if epoch and kp then steps[#steps + 1] = { epoch = epoch, kp = kp, observed = observed == "observed" } end
    end
    return steps
end

-- The Kp for the three hours holding an epoch, or nil past the feed's end.
function Aurora.kpAt(steps, epoch)
    for _, step in ipairs(steps or {}) do
        if epoch >= step.epoch and epoch < step.epoch + 3 * 3600 then return step.kp end
    end
end

-- What the requests below know of now: the time and the Kp steps of the
-- last forecast an answer was read into. Kp is planetary, so it holds for
-- every place. With nothing read yet: the device clock and no Kp.
local context = {}
function Aurora.reset() context = {} end
local function remember(forecast, kp)
    context.epoch = forecast.epoch or context.epoch
    if kp ~= nil then context.kp = kp or nil end
end

Aurora.source = {
    id = "kp", optional = true,
    url = function() return Aurora.URL end,
    parse = function(json, forecast)
        local steps = Aurora.parseKp(json)
        forecast.kp = #steps > 0 and steps or nil
        remember(forecast, forecast.kp or false)
        for _, hour in ipairs(forecast.hours) do
            if hour.epoch then hour.kp = Aurora.kpAt(steps, hour.epoch + 1800) end
        end
    end,
}

-- Live space weather (SV-ST1 to SV-ST3). Each request is optional and
-- asked only where the aurora can matter; a failed one is recorded in
-- `forecast.missing` and the cards say the data is missing, never "no
-- aurora".
local function number(value)
    if type(value) == "number" then return value end
    return type(value) == "string" and tonumber(value) or nil
end

-- "2026-11-18 05:01:00.000", "2026-11-18T05:50:00Z" (UTC) → epoch.
local function stamp(tag)
    if type(tag) ~= "string" then return nil end
    local start = Sky.epochOf(tag:sub(1, 10))
    local h, m, s = tag:match("^%d%d%d%d%-%d%d%-%d%d[T ](%d%d):(%d%d):?(%d*)")
    return start and h and start + tonumber(h) * 3600 + tonumber(m) * 60 + (tonumber(s) or 0)
end
Aurora.stamp = stamp

-- The highest Kp in the twelve hours from an epoch, or nil.
local function kpAhead(steps, epoch)
    local top
    for _, step in ipairs(steps or {}) do
        if step.epoch + 3 * 3600 > epoch and step.epoch < epoch + 12 * 3600 then top = math.max(top or 0, step.kp) end
    end
    return top
end

-- Whether the aurora can matter at a place: within reach of the oval at a
-- Kp (3 when unknown) with eight degrees to spare, and never below 45°
-- magnetic. Keeps the requests below off the tropics and quiet nights.
function Aurora.canMatter(lat, lon, kp)
    return math.abs(Aurora.magneticLatitude(lat, lon)) >= math.max(45, Aurora.edge(kp or 3) - 8)
end

-- Whether to ask for a place now: the aurora can matter there and, with
-- `dark`, the sun is 6° down now or within the hour. With no answer read
-- yet (the Kp forecast failed on the first load), the time is unknown, so
-- the darkness test is skipped rather than guessed from the device clock.
local function worth(spot, dark, kp)
    if not (spot and spot.lat and spot.lon) then return false end
    local epoch = context.epoch
    if not Aurora.canMatter(spot.lat, spot.lon, kp or (epoch and kpAhead(context.kp, epoch))) then return false end
    if not (dark and epoch) then return true end
    return math.min(Sky.sunAltitude(epoch, spot.lat, spot.lon), Sky.sunAltitude(epoch + 3600, spot.lat, spot.lon)) <= -6
end
Aurora.worth = worth

-- SWPC's product tables: a header row and rows of strings, or (newer
-- feeds) a list of objects. `fields` maps a column to a field:
-- { bz_gsm = "bz" } → { { epoch, bz }, … } in time order. Nulls stay nil.
function Aurora.rows(json, fields)
    local list = {}
    if type(json) ~= "table" then return list end
    local header = type(json[1]) == "table" and json[1][1] == "time_tag" and json[1]
    for index, entry in ipairs(json) do
        local row
        if type(entry) == "table" and header and index > 1 then
            row = { epoch = stamp(entry[1]) }
            for column, name in ipairs(header) do
                if fields[name] then row[fields[name]] = number(entry[column]) end
            end
        elseif type(entry) == "table" and not header and entry.time_tag then
            row = { epoch = stamp(entry.time_tag) }
            for name, field in pairs(fields) do row[field] = number(entry[name]) end
        end
        if row and row.epoch then list[#list + 1] = row end
    end
    table.sort(list, function(a, b) return a.epoch < b.epoch end)
    return list
end

Aurora.MAG_URL = "https://services.swpc.noaa.gov/products/solar-wind/mag-2-hour.json"
Aurora.PLASMA_URL = "https://services.swpc.noaa.gov/products/solar-wind/plasma-2-hour.json"
-- Rows older than this count as missing: the spacecraft may be in a gap.
Aurora.FRESH = 15 * 60

-- The solar wind at L1 now, from the magnetometer and plasma rows:
-- { bz, bt, speed, density, minutesSouth, at } or nil when neither has a
-- row from the last 15 minutes. `minutesSouth` is how long Bz has pointed
-- south without a break (gaps of up to ten minutes bridged).
function Aurora.solarWind(mag, plasma, now)
    local function latest(rows, field)
        for index = #rows, 1, -1 do
            local row = rows[index]
            if row[field] and row.epoch <= now + 300 then return now - row.epoch <= Aurora.FRESH and row or nil end
        end
    end
    mag, plasma = mag or {}, plasma or {}
    local m, p = latest(mag, "bz"), latest(plasma, "speed")
    if not (m or p) then return nil end
    local wind = { at = m and m.epoch or p.epoch, bz = m and m.bz, bt = m and m.bt, speed = p and p.speed,
        density = p and p.density, minutesSouth = 0 }
    if m and m.bz < 0 then
        local start, last = m.epoch, m.epoch
        for index = #mag, 1, -1 do
            local row = mag[index]
            if row.bz and row.epoch <= m.epoch then
                if row.bz > 0 or last - row.epoch > 600 then break end
                start, last = row.epoch, row.epoch
            end
        end
        wind.minutesSouth = math.floor((m.epoch - start) / 60 + 0.5) + 1
    end
    return wind
end

local function signed(value) return (("%.0f"):format(value):gsub("^%-", "−")) end

-- What the solar wind means for the next hour, in one calm line, and how
-- much: "strong", "watch", "south" or "quiet". Southward Bz held for half
-- an hour is what sets off a substorm about an hour later.
function Aurora.windText(wind)
    local parts, level = {}, "quiet"
    if wind.bz then
        if wind.bz <= -5 and wind.minutesSouth >= 30 then
            level = wind.bz <= -10 and "strong" or "watch"
            parts[1] = ("Bz south %s nT for %d min: the next hour is the one to watch"):format(signed(wind.bz), wind.minutesSouth)
        elseif wind.bz < -1 then
            level = "south"
            parts[1] = ("Bz south %s nT for %d min"):format(signed(wind.bz), wind.minutesSouth)
        else
            parts[1] = "Bz north: the solar wind holds the aurora back for now"
        end
    end
    if wind.speed then
        parts[#parts + 1] = (wind.speed >= 500 and "fast solar wind %d km/s" or "solar wind %d km/s"):format(math.floor(wind.speed + 0.5))
    end
    return table.concat(parts, " · "), level
end

local function windSource(id, url, fields, key)
    return {
        id = id, optional = true,
        url = function(spot) return worth(spot, true) and url or nil end,
        parse = function(json, forecast)
            remember(forecast)
            forecast[key] = Aurora.rows(json, fields)
            forecast.solarWind = Aurora.solarWind(forecast.solarMag, forecast.solarPlasma, forecast.epoch)
        end,
    }
end
Aurora.magSource = windSource("solarMag", Aurora.MAG_URL, { bz_gsm = "bz", bt = "bt" }, "solarMag")
Aurora.plasmaSource = windSource("solarPlasma", Aurora.PLASMA_URL, { speed = "speed", density = "density" }, "solarPlasma")

-- OVATION, SWPC's 30-minute aurora nowcast: the chance of visible aurora
-- on a 1° grid, { "Forecast Time", "Observation Time", coordinates =
-- { { lon 0–359, lat −90…90, percent }, … } }. About 900 KB, so it is asked
-- only on a dark night where the aurora can matter, and the answer is cut
-- to the cells within 10° of the place before it is saved.
Aurora.OVATION_URL = "https://services.swpc.noaa.gov/json/ovation_aurora_latest.json"
Aurora.OVATION_FRESH = 45 * 60

local function cell(lon, lat) return (math.floor(lat + 0.5) + 90) * 360 + math.floor(lon + 0.5) % 360 end

-- Keeps the cells within 10° of a spot in a decoded answer (in place, so
-- the trimmed answer is what is saved with the forecast).
function Aurora.trimOvation(json, spot)
    local list = type(json) == "table" and json.coordinates
    if type(list) ~= "table" then return end
    local kept = {}
    for _, c in ipairs(list) do
        local lon, lat = type(c) == "table" and number(c[1]), type(c) == "table" and number(c[2])
        if lon and lat and math.abs(lat - spot.lat) <= 10 and math.abs((lon - spot.lon + 180) % 360 - 180) <= 10 then
            kept[#kept + 1] = c
        end
    end
    json.coordinates = kept
end

-- { at (forecast time), observed, cells } from an answer, or nil.
function Aurora.ovation(json)
    if type(json) ~= "table" or type(json.coordinates) ~= "table" then return nil end
    local cells, count = {}, 0
    for _, c in ipairs(json.coordinates) do
        if type(c) == "table" then
            local lon, lat, value = number(c[1]), number(c[2]), number(c[3])
            if lon and lat and value then cells[cell(lon, lat)], count = value, count + 1 end
        end
    end
    if count == 0 then return nil end
    local at = stamp(json["Forecast Time"])
    return { at = at, observed = stamp(json["Observation Time"]) or (at and at - 1800), cells = cells }
end

-- The nowcast at a place: { chance, horizonChance, at, stale }, chance at
-- the nearest cell and horizonChance the highest 2–6° nearer the pole
-- (aurora there shows low on the poleward horizon). nil off the grid.
function Aurora.ovationAt(ovation, lat, lon, now)
    if not ovation then return nil end
    local here = ovation.cells[cell(lon, lat)]
    if not here then return nil end
    local step, horizon = lat < 0 and -1 or 1, 0
    for d = 2, 6 do
        for side = -2, 2 do horizon = math.max(horizon, ovation.cells[cell(lon + side, lat + step * d)] or 0) end
    end
    local stale = not (ovation.observed and now) or now - ovation.observed > Aurora.OVATION_FRESH
    return { chance = here, horizonChance = horizon, at = ovation.at, stale = stale }
end

local trimFor
Aurora.ovationSource = {
    id = "ovation", optional = true,
    url = function(spot)
        trimFor = nil
        if not worth(spot, true) then return nil end
        trimFor = spot
        return Aurora.OVATION_URL
    end,
    parse = function(json, forecast)
        -- Only a whole file is cut, around the place just asked for: a
        -- replayed answer is already cut and has no place to go with.
        local spot = trimFor
        trimFor = nil
        if spot and type(json) == "table" and type(json.coordinates) == "table" and #json.coordinates > 5000 then
            Aurora.trimOvation(json, spot)
        end
        forecast.ovation = Aurora.ovation(json)
    end,
}

-- SWPC's alerts, warnings and watches (alerts.json: { product_id,
-- issue_datetime, message }): the geomagnetic ones still in force, without
-- those a later message cancels: { { kind = "watch" | "warning" | "alert",
-- g, at, to, days = { ["2026-11-19"] = 2 } } }, newest first.
Aurora.ALERTS_URL = "https://services.swpc.noaa.gov/products/alerts.json"
local MONTHS = { Jan = 1, Feb = 2, Mar = 3, Apr = 4, May = 5, Jun = 6, Jul = 7, Aug = 8, Sep = 9, Oct = 10, Nov = 11, Dec = 12 }

-- "2026 Nov 18 0300 UTC" → epoch.
local function spoken(text)
    local y, mon, d, h, m = (text or ""):match("(%d%d%d%d) (%a%a%a) (%d%d?) (%d%d)(%d%d) UTC")
    if not (y and MONTHS[mon]) then return nil end
    return Sky.epochOf(("%s-%02d-%02d"):format(y, MONTHS[mon], tonumber(d))) + tonumber(h) * 3600 + tonumber(m) * 60
end

local function readAlert(entry)
    local message = type(entry) == "table" and entry.message
    if type(message) ~= "string" then return nil end
    local at = stamp(entry.issue_datetime) or spoken(message:match("Issue Time:%s*([^\r\n]+)"))
    local alert = { serial = message:match("[\r\n]Serial Number:%s*(%d+)"), at = at,
        cancels = message:match("Cancel Serial Number:%s*(%d+)") }
    local body = message:gsub("%s+", " ")
    local g = body:match("WATCH: Geomagnetic Storm Category G(%d)")
    local k = body:match("WARNING: Geomagnetic K%-index of (%d)")
    local reached = body:match("ALERT: Geomagnetic K%-index of (%d)")
    if body:find("CANCEL WATCH") or body:find("CANCEL WARNING") then alert.kind = "cancel"
    elseif g then
        alert.kind, alert.g, alert.days = "watch", tonumber(g), {}
        local year = tonumber((entry.issue_datetime or ""):match("^(%d%d%d%d)")) or tonumber(utcDate("!%Y", at or 0))
        local issued = at and tonumber(utcDate("!%m", at))
        local list = body:match("Predicted by Day:(.-)$") or ""
        for mon, d, level in list:gmatch("(%a%a%a) (%d%d?):%s*(%S+)") do
            if MONTHS[mon] then
                local y = (issued == 12 and MONTHS[mon] == 1) and year + 1 or year
                alert.days[("%d-%02d-%02d"):format(y, MONTHS[mon], tonumber(d))] = tonumber(level:match("^G(%d)")) or 0
            end
        end
    elseif k then
        alert.kind, alert.g = "warning", math.max(0, tonumber(k) - 4)
        alert.from, alert.to = spoken(body:match("Valid From: (.-UTC)")), spoken(body:match("Valid To: (.-UTC)"))
    elseif reached then
        alert.kind, alert.g = "alert", math.max(0, tonumber(reached) - 4)
        alert.from = spoken(body:match("Threshold Reached: (.-UTC)")) or at
        alert.to = alert.from and alert.from + 3 * 3600
    else return nil end
    return alert
end

function Aurora.parseAlerts(json, now)
    local all, cancelled, list = {}, {}, {}
    for _, entry in ipairs(type(json) == "table" and json or {}) do
        local alert = readAlert(entry)
        if alert then all[#all + 1] = alert end
        if alert and alert.cancels then cancelled[alert.cancels] = true end
    end
    local today = now and utcDate("!%Y-%m-%d", now)
    for _, alert in ipairs(all) do
        local live = alert.kind ~= "cancel" and not (alert.serial and cancelled[alert.serial])
        if live and now and alert.kind == "watch" then
            local ahead = false
            for date, g in pairs(alert.days) do if date >= today and g > 0 then ahead = true end end
            live = ahead
        elseif live and now then live = (alert.to or (alert.at or 0) + 12 * 3600) >= now end
        if live then list[#list + 1] = alert end
    end
    table.sort(list, function(a, b) return (a.at or 0) > (b.at or 0) end)
    return list
end

Aurora.alertsSource = {
    id = "spaceAlerts", optional = true,
    url = function(spot) return worth(spot, false, 9) and Aurora.ALERTS_URL or nil end,
    parse = function(json, forecast)
        remember(forecast)
        forecast.spaceAlerts = Aurora.parseAlerts(json, forecast.epoch)
    end,
}

-- One calm line from the alerts for a place, or nil: a warning or alert in
-- force, else the strongest watch ahead whose storm could reach the place.
-- `offset` puts "tonight" and "tomorrow night" on the local calendar.
function Aurora.alertLine(alerts, lat, lon, now, offset)
    local reachable = function(g) return g > 0 and Aurora.reach(lat, lon, g + 4) ~= nil end
    for _, alert in ipairs(alerts or {}) do
        if (alert.kind == "warning" or alert.kind == "alert") and reachable(alert.g)
            and (alert.from or alert.at or now) <= now + 3600 then
            local when = alert.to and Sky.clock(utcDate("!%Y-%m-%dT%H:%M", alert.to + (offset or 0)))
            return alert.kind == "alert" and ("NOAA: a G%d storm is under way"):format(alert.g)
                or ("NOAA: a G%d storm warning until %s"):format(alert.g, when or "later")
        end
    end
    local today = utcDate("!%Y-%m-%d", now + (offset or 0))
    local tomorrow = utcDate("!%Y-%m-%d", now + (offset or 0) + 86400)
    local best
    for _, alert in ipairs(alerts or {}) do
        if alert.kind == "watch" then
            for date, g in pairs(alert.days) do
                if date >= today and reachable(g) and (not best or g > best.g or (g == best.g and date < best.date)) then
                    best = { g = g, date = date }
                end
            end
        end
    end
    if not best then return nil end
    local when = best.date == today and "tonight" or best.date == tomorrow and "tomorrow night" or Sky.weekday(best.date)
    return ("A G%d storm watch for %s"):format(best.g, when)
end

-- GFZ Potsdam's Hp30: Kp's half-hourly sibling (CC BY 4.0), for "activity
-- is rising now" between the three-hour Kp steps. Answer: { datetime =
-- { "2026-11-18T00:00:00Z", … }, Hp30 = { 2.333, … }, status = { … } }.
function Aurora.hp30Url(now)
    local function iso(epoch) return utcDate("!%Y-%m-%dT%H:%M:%SZ", epoch) end
    return ("https://kp.gfz.de/app/json/?start=%s&end=%s&index=Hp30"):format(iso(now - 6 * 3600 - now % 1800), iso(now - now % 1800))
end

function Aurora.parseHp30(json)
    local steps = {}
    local times, values = type(json) == "table" and json.datetime, type(json) == "table" and json.Hp30
    if type(times) ~= "table" or type(values) ~= "table" then return steps end
    for index, time in ipairs(times) do
        local epoch, value = stamp(time), number(values[index])
        if epoch and value then steps[#steps + 1] = { epoch = epoch, value = value } end
    end
    table.sort(steps, function(a, b) return a.epoch < b.epoch end)
    return steps
end

-- Activity now from Hp30 steps: { value, at, rising } or nil when the last
-- half hour is more than an hour old. Rising: up by more than a Kp step
-- over the last two hours, to at least 4.
function Aurora.activityNow(steps, now)
    local last
    for index = #(steps or {}), 1, -1 do
        if steps[index].epoch <= now then last = steps[index]; break end
    end
    if not last or now - (last.epoch + 1800) > 3600 then return nil end
    local low = last.value
    for _, step in ipairs(steps) do
        if step.epoch >= last.epoch - 2 * 3600 and step.epoch <= last.epoch then low = math.min(low, step.value) end
    end
    return { value = last.value, at = last.epoch, rising = last.value >= 4 and last.value - low >= 1 }
end

Aurora.hp30Source = {
    id = "hp30", optional = true,
    url = function(spot) return worth(spot, false) and Aurora.hp30Url(context.epoch or os.time()) or nil end,
    parse = function(json, forecast)
        remember(forecast)
        local steps = Aurora.parseHp30(json)
        forecast.hp30 = #steps > 0 and steps or nil
    end,
}

Aurora.sources = { Aurora.source, Aurora.magSource, Aurora.plasmaSource, Aurora.ovationSource, Aurora.alertsSource,
    Aurora.hp30Source }

return Aurora
