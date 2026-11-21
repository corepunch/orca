-- Official warnings at a sailing spot (SA10, Research/services-sailing.md
-- SV-SA1 and SV-SA3), two optional sources:
--
--   sailing-nws         US points: the National Weather Service's active
--                       alerts (api.weather.gov, public domain, keyless; a
--                       User-Agent is required and every request has one)
--   sailing-meteoalarm  Europe: a country's MeteoAlarm feed (EUMETNET, terms
--                       equivalent to CC BY 4.0: credit the national
--                       service, show the issue time, link meteoalarm.org)
--
-- `parse` keeps every warning that bears on a rider; which ones cover the
-- spot and are still in force is worked out when they are read
-- (Warnings.active), because a saved answer is replayed hours later and
-- `parse` does not see the spot. A service that failed or was never asked
-- never reads as "no warnings" (Warnings.cards says so).
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Warnings = {}

-- Base URLs, kept here (not Open-Meteo hosts). MeteoAlarm may move to
-- "MeteoGate"; only this line would change.
Warnings.endpoints = {
    nws = "https://api.weather.gov/alerts/active",
    meteoalarm = "https://feeds.meteoalarm.org/api/v1/warnings/feeds-",
}

-- What a warning does to the rating: "cap" stops every profile, "learner"
-- caps learners and dinghies and is a caution for the rest, "caution" is a
-- reason against the hour.
local CAP, LEARNER, CAUTION = "cap", "learner", "caution"
local STRENGTH = { [CAP] = 1, [LEARNER] = 2, [CAUTION] = 3 }

-- The NWS events a sailor or kiter needs (marine, wind, thunder, fog);
-- every other event (heat, flood, frost) is left out.
Warnings.NWS_EVENTS = {
    ["Hurricane Force Wind Warning"] = CAP, ["Storm Warning"] = CAP, ["Gale Warning"] = CAP,
    ["Special Marine Warning"] = CAP, ["Hurricane Warning"] = CAP, ["Tropical Storm Warning"] = CAP,
    ["Severe Thunderstorm Warning"] = CAP, ["Tornado Warning"] = CAP, ["High Wind Warning"] = CAP,
    ["Extreme Wind Warning"] = CAP,
    ["Small Craft Advisory"] = LEARNER, ["Hazardous Seas Warning"] = LEARNER, ["Wind Advisory"] = LEARNER,
    ["Lake Wind Advisory"] = LEARNER, ["Brisk Wind Advisory"] = LEARNER, ["High Surf Warning"] = LEARNER,
    ["Gale Watch"] = CAUTION, ["Storm Watch"] = CAUTION, ["Hurricane Force Wind Watch"] = CAUTION,
    ["Hazardous Seas Watch"] = CAUTION, ["Hurricane Watch"] = CAUTION, ["Tropical Storm Watch"] = CAUTION,
    ["Dense Fog Advisory"] = CAUTION, ["High Surf Advisory"] = CAUTION, ["Rip Current Statement"] = CAUTION,
    ["Beach Hazards Statement"] = CAUTION, ["Low Water Advisory"] = CAUTION,
}

-- api.weather.gov's coverage: the lower 48 and its coastal waters, Alaska,
-- Hawaii and Puerto Rico. Alaska's marine alerts are not in its feed.
local US = { { south = 24, north = 50, west = -125.5, east = -66 }, { south = 51, north = 72, west = -170, east = -129 },
    { south = 18.5, north = 22.5, west = -161, east = -154 }, { south = 17.5, north = 18.6, west = -67.5, east = -65 } }
local function inside(spot, boxes)
    for _, box in ipairs(boxes) do
        if spot.lat >= box.south and spot.lat <= box.north and spot.lon >= box.west and spot.lon <= box.east then
            return true
        end
    end
    return false
end

-- MeteoAlarm's feed names by ISO country code.
Warnings.FEEDS = { AT = "austria", BE = "belgium", BG = "bulgaria", CH = "switzerland", CY = "cyprus",
    CZ = "czechia", DE = "germany", DK = "denmark", EE = "estonia", ES = "spain", FI = "finland", FR = "france",
    GB = "united-kingdom", GR = "greece", HR = "croatia", HU = "hungary", IE = "ireland", IS = "iceland",
    IT = "italy", LT = "lithuania", LU = "luxembourg", LV = "latvia", MT = "malta", NL = "netherlands",
    NO = "norway", PL = "poland", PT = "portugal", RO = "romania", RS = "serbia", SE = "sweden", SI = "slovenia",
    SK = "slovakia" }

-- The curated spots' country and the area names their national service
-- uses (a district or a sea area; a CAP area matches when its name
-- contains one). Estimates from each service's area lists: a warning for
-- another area of the country is told apart, never dropped silently.
Warnings.AREAS = {
    bodensee = { country = "DE", names = { "Bodensee", "Konstanz", "Lindau" } },
    spo = { country = "DE", names = { "Nordfriesland", "Nordsee", "Eiderstedt" } },
    garda = { country = "IT", names = { "Garda", "Brescia", "Verona", "Trento", "Trentino" } },
    tarifa = { country = "ES", names = { "Cádiz", "Estrecho", "Campo de Gibraltar" } },
    mamry = { country = "PL", names = { "giżycki", "węgorzewski", "Giżycko", "Węgorzewo" } },
}

-- "2026-11-20T10:00:00-08:00" (or "…Z", "…+00:00") → UTC epoch, or nil.
function Warnings.epoch(iso)
    if type(iso) ~= "string" then return nil end
    local date, h, m, sign, oh, om = iso:match("^(%d+%-%d+%-%d+)T(%d+):(%d+)[:%d%.]*([%+%-]?)(%d*):?(%d*)")
    local start = date and Sky.epochOf(date)
    if not start then return nil end
    local offset = sign ~= "" and ((tonumber(oh) or 0) * 3600 + (tonumber(om) or 0) * 60) or 0
    return start + tonumber(h) * 3600 + tonumber(m) * 60 - (sign == "-" and -offset or offset)
end
local epoch = Warnings.epoch

local function text(value) return type(value) == "string" and value ~= "" and value or nil end

local function state(forecast)
    forecast.sailing = forecast.sailing or {}
    return forecast.sailing
end

Warnings.nws = {
    id = "sailing-nws", optional = true,
    url = function(spot)
        if not (spot and spot.lat and spot.lon and inside(spot, US)) then return nil end
        return ("%s?point=%.4f,%.4f"):format(Warnings.endpoints.nws, spot.lat, spot.lon)
    end,
    headers = { Accept = "application/geo+json" },
    -- An answer without `features` fails the source: it is not "no warnings".
    parse = function(json, forecast)
        local features = assert(type(json.features) == "table" and json.features, "no alert list")
        local list = {}
        for _, feature in ipairs(features) do
            local p = type(feature) == "table" and type(feature.properties) == "table" and feature.properties or {}
            local kind = Warnings.NWS_EVENTS[p.event]
            if kind and p.status ~= "Test" and p.messageType ~= "Cancel" then
                list[#list + 1] = { service = "nws", event = p.event, kind = kind, headline = text(p.headline),
                    sender = text(p.senderName) or "National Weather Service", severity = p.severity,
                    onset = epoch(p.onset or p.effective), ends = epoch(p.ends) or epoch(p.expires),
                    sent = epoch(p.sent) }
            end
        end
        state(forecast).nws = list
    end,
}

-- The feed for a spot: the curated table's country, else the place's own.
function Warnings.feed(spot)
    local known = spot and Warnings.AREAS[spot.id]
    local country = known and known.country or spot and type(spot.country) == "string" and spot.country:upper()
    return country and Warnings.FEEDS[country], country
end

-- MeteoAlarm's awareness types a rider needs ("1; Wind") and levels
-- ("3; orange; Severe"); green (1) is no warning.
local TYPES = { [1] = "wind", [3] = "thunderstorm", [7] = "coastal event" }
local LEVELS = { [2] = "yellow", [3] = "orange", [4] = "red" }
local KIND = {
    wind = { yellow = LEARNER, orange = CAP, red = CAP },
    thunderstorm = { yellow = CAUTION, orange = CAP, red = CAP },
    ["coastal event"] = { yellow = LEARNER, orange = CAP, red = CAP },
}

local function parameter(info, name)
    for _, p in ipairs(type(info.parameter) == "table" and info.parameter or {}) do
        if type(p) == "table" and p.valueName == name then return tonumber(tostring(p.value):match("^%s*(%d+)")) end
    end
end

-- The English block of a CAP alert's `info` list, else the first.
local function english(infos)
    local first
    for _, info in ipairs(type(infos) == "table" and infos or {}) do
        if type(info) == "table" then
            first = first or info
            if tostring(info.language or ""):lower():find("^en") then return info end
        end
    end
    return first
end

-- "lat,lon lat,lon …" (CAP polygon) → a list of { lat, lon }.
local function polygon(textValue)
    local points = {}
    for lat, lon in tostring(textValue):gmatch("([%-%d%.]+),([%-%d%.]+)") do
        points[#points + 1] = { tonumber(lat), tonumber(lon) }
    end
    return #points >= 3 and points or nil
end

Warnings.meteoalarm = {
    id = "sailing-meteoalarm", optional = true,
    url = function(spot)
        local feed = Warnings.feed(spot)
        return feed and Warnings.endpoints.meteoalarm .. feed or nil
    end,
    headers = { Accept = "application/json" },
    parse = function(json, forecast)
        local warnings = assert(type(json.warnings) == "table" and json.warnings, "no warning list")
        local list = {}
        for _, entry in ipairs(warnings) do
            local alert = type(entry) == "table" and type(entry.alert) == "table" and entry.alert or {}
            local info = english(alert.info)
            local kindOf = info and TYPES[parameter(info, "awareness_type")]
            local level = info and LEVELS[parameter(info, "awareness_level")]
            if kindOf and level and alert.status ~= "Test" and alert.msgType ~= "Cancel" then
                local areas = {}
                for _, area in ipairs(type(info.area) == "table" and info.area or {}) do
                    if type(area) == "table" then
                        local codes = {}
                        for _, code in ipairs(type(area.geocode) == "table" and area.geocode or {}) do
                            if type(code) == "table" and code.value then codes[#codes + 1] = tostring(code.value) end
                        end
                        areas[#areas + 1] = { name = text(area.areaDesc), codes = codes, polygon = polygon(area.polygon) }
                    end
                end
                list[#list + 1] = { service = "meteoalarm", event = text(info.event) or (level .. " " .. kindOf .. " warning"),
                    awareness = kindOf, level = level, kind = KIND[kindOf][level], headline = text(info.headline),
                    sender = text(info.senderName) or text(alert.sender) or "the national weather service",
                    onset = epoch(info.onset or info.effective), ends = epoch(info.expires), sent = epoch(alert.sent),
                    areas = areas }
            end
        end
        state(forecast).meteoalarm = list
    end,
}

Warnings.sources = { Warnings.nws, Warnings.meteoalarm }

local function contains(points, lat, lon)
    local hit, j = false, #points
    for i = 1, #points do
        local a, b = points[i], points[j]
        if (a[1] > lat) ~= (b[1] > lat) and lon < (b[2] - a[2]) * (lat - a[1]) / (b[1] - a[1]) + a[2] then hit = not hit end
        j = i
    end
    return hit
end

-- Whether a MeteoAlarm warning covers the spot: inside one of its
-- polygons, or one of its area names or codes is the spot's (the curated
-- table, else the place's own title and region).
function Warnings.covers(warning, spot)
    if warning.service ~= "meteoalarm" then return true end
    spot = spot or {}
    local known = Warnings.AREAS[spot.id] or {}
    local names = { table.unpack(known.names or {}) }
    for _, own in ipairs { spot.title, spot.region } do
        if type(own) == "string" and #own >= 4 then names[#names + 1] = own end
    end
    for _, area in ipairs(warning.areas or {}) do
        if area.polygon and spot.lat and contains(area.polygon, spot.lat, spot.lon) then return true end
        for _, code in ipairs(area.codes or {}) do
            for _, own in ipairs(known.codes or {}) do if code == own then return true end end
        end
        local desc = area.name and area.name:lower()
        for _, name in ipairs(desc and names or {}) do
            if desc:find(name:lower(), 1, true) then return true end
        end
    end
    return false
end

local ORDER = { red = 1, orange = 2, yellow = 3 }
-- The warnings in force now or later at the spot, the strongest first,
-- and how many MeteoAlarm lists elsewhere in the country.
function Warnings.active(forecast, spot, now)
    local sailing = forecast.sailing or {}
    now = now or forecast.epoch or os.time()
    local list, elsewhere = {}, 0
    for _, key in ipairs { "nws", "meteoalarm" } do
        for _, warning in ipairs(sailing[key] or {}) do
            if not warning.ends or warning.ends > now then
                if Warnings.covers(warning, spot) then list[#list + 1] = warning else elsewhere = elsewhere + 1 end
            end
        end
    end
    table.sort(list, function(a, b)
        if STRENGTH[a.kind] ~= STRENGTH[b.kind] then return STRENGTH[a.kind] < STRENGTH[b.kind] end
        if (ORDER[a.level] or 9) ~= (ORDER[b.level] or 9) then return (ORDER[a.level] or 9) < (ORDER[b.level] or 9) end
        return (a.onset or 0) < (b.onset or 0)
    end)
    return list, elsewhere
end

-- The services that cover a spot, with their source id and credit.
function Warnings.services(spot)
    local list = {}
    if spot and spot.lat and spot.lon and inside(spot, US) then
        list[#list + 1] = { key = "nws", id = Warnings.nws.id, name = "the National Weather Service", link = "weather.gov" }
    end
    local feed = Warnings.feed(spot)
    if feed then
        list[#list + 1] = { key = "meteoalarm", id = Warnings.meteoalarm.id, name = "MeteoAlarm", link = "meteoalarm.org" }
    end
    return list
end

local function localTime(forecast, at)
    local stamp = os.date("!%Y-%m-%dT%H:%M", at + (forecast.offset or 0))
    return Sky.weekday(stamp:sub(1, 10)):sub(1, 3) .. " " .. Units.clock(stamp)
end
Warnings.localTime = localTime

-- The hour's warnings as reasons: the strongest of each kind once.
-- `rider` is the profile (Profile.current()).
function Warnings.rate(card, hour, forecast, rider)
    if not (forecast.sailing and hour.epoch) then return end
    local done = {}
    for _, warning in ipairs((Warnings.active(forecast, forecast.spot, hour.epoch))) do
        local from, to = warning.onset or 0, warning.ends or math.huge
        if not done[warning.kind] and hour.epoch + 3600 > from and hour.epoch < to then
            done[warning.kind] = true
            local name = warning.level and (warning.level .. " " .. warning.awareness .. " warning") or warning.event
            if warning.kind == CAP then
                card:cap(15, ("Official %s in force: stay ashore."):format(name))
            elseif warning.kind == LEARNER and (rider.level == "beginner" or rider.sport == "dinghy") then
                card:cap(25, ("Official %s in force: not for learners or small boats."):format(name))
            elseif warning.kind == LEARNER then
                card:add(-12, ("Official %s in force: strong wind or rough water."):format(name))
            else
                card:add(-6, ("Official %s."):format(name))
            end
        end
    end
end

-- The warning card: the strongest warning in force, or what the services
-- said. A service that failed says so; one that answered with nothing for
-- this spot says "none in force" with when it was checked, on the activity
-- page only.
function Warnings.cards(forecast, spot, now)
    local services = Warnings.services(spot)
    if #services == 0 then return {} end
    local sailing, missing = forecast.sailing or {}, forecast.missing or {}
    local active, elsewhere = Warnings.active(forecast, spot, now)
    local warning = active[1]
    if warning then
        local parts = {}
        if warning.onset and warning.onset > (now or forecast.epoch or 0) then
            parts[#parts + 1] = "from " .. localTime(forecast, warning.onset)
        end
        if warning.ends then parts[#parts + 1] = "until " .. localTime(forecast, warning.ends) end
        local by = warning.sender
        if warning.service == "meteoalarm" then by = by .. " via EUMETNET – MeteoAlarm" end
        parts[#parts + 1] = by
        if warning.sent then parts[#parts + 1] = "issued " .. localTime(forecast, warning.sent) end
        if #active > 1 then parts[#parts + 1] = ("%d more"):format(#active - 1) end
        -- MeteoAlarm's event is in the national language: its level and type
        -- read the same everywhere.
        local value = warning.level and ("%s %s warning"):format(warning.level, warning.awareness) or warning.event:lower()
        return { { id = "warning", title = "official warning", value = value, detail = table.concat(parts, " · "), rank = 0,
            tone = warning.kind == CAP and "poor" or "fair" } }
    end
    local answered, failed = {}, {}
    for _, service in ipairs(services) do
        if sailing[service.key] then answered[#answered + 1] = service
        elseif missing[service.id] then failed[#failed + 1] = service end
    end
    if #failed > 0 then
        return { { id = "warning", title = "official warnings", value = "not checked", tone = "info",
            detail = ("%s did not answer: look at %s before you go out."):format(failed[1].name, failed[1].link) } }
    end
    if #answered == 0 then return {} end
    local checked = forecast.updated and " · checked " .. Units.clock(os.date("!%H:%M", forecast.updated + (forecast.offset or 0))) or ""
    local names = {}
    for _, service in ipairs(answered) do names[#names + 1] = service.name end
    -- A found place has no area names of its own: a warning elsewhere in
    -- its country may still be its own, so it is not "none".
    if elsewhere > 0 and not (spot and Warnings.AREAS[spot.id]) then
        return { { id = "warning", title = "official warnings", value = "check your area", tone = "info",
            detail = ("MeteoAlarm lists %d in the country but cannot place this spot: see meteoalarm.org%s")
                :format(elsewhere, checked) } }
    end
    local detail = "none for this spot from " .. table.concat(names, " and ") .. checked
    if elsewhere > 0 then detail = detail .. (" · %d elsewhere in the country: see meteoalarm.org"):format(elsewhere) end
    return { { id = "warning", title = "official warnings", value = "none in force", tone = "info", hub = false,
        detail = detail } }
end

return Warnings
