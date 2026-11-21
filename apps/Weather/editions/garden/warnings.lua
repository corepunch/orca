-- Official warnings for the plot (SV-GA1, SV-GA6). The app's own frost
-- estimate (frost.lua) is only an estimate; the warning the neighbours
-- hear comes from the national service, and it is shown as issued: its
-- event name and headline unchanged, under the issuer's name.
--
--   US   NWS active alerts at the place's point (api.weather.gov, public
--        domain, keyless; the app's User-Agent is required). Kept: frost,
--        freeze, heat and high wind. A failed answer is never read as
--        "no warnings": the frost card says the alerts were not reached.
--        NWS offices stop frost headlines after the growing season ends
--        (weather.gov/rlx/frost-freeze-information), so a silent NWS with
--        a frost estimate says "this is our estimate".
--   UK   Met Office Weather Warnings: a link only. The public feed needs
--        no key, but its NSWWS Open Licence and style guide have not been
--        read for this app, and the feed names regions, not a point.
--
-- Other countries get neither, and the app says nothing about warnings.
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Warnings = {}

Warnings.NWS = "https://api.weather.gov/alerts/active"
Warnings.METOFFICE = "metoffice.gov.uk/weather/warnings-and-advice"

-- The US: the lower 48, Alaska and Hawaii (as activities/outdoors.lua).
local US = { { south = 24, north = 50, west = -125, east = -66 }, { south = 51, north = 72, west = -170, east = -129 },
    { south = 18.5, north = 22.5, west = -161, east = -154 } }
local function inside(spot, box)
    return spot and spot.lat and spot.lon and spot.lat >= box.south and spot.lat <= box.north and spot.lon >= box.west
        and spot.lon <= box.east
end
function Warnings.us(spot)
    for _, box in ipairs(US) do
        if inside(spot, box) then return true end
    end
    return false
end

-- Great Britain, Northern Ireland and the islands around them.
local UK = { south = 49.8, north = 61, west = -8.7, east = 2 }
function Warnings.uk(spot) return inside(spot, UK) and not (spot.lon < -6 and spot.lat < 54) or false end

-- The NWS events a gardener acts on, and which of them are about frost.
Warnings.KEEP = {
    ["Frost Advisory"] = "frost", ["Freeze Watch"] = "frost", ["Freeze Warning"] = "frost",
    ["Hard Freeze Watch"] = "frost", ["Hard Freeze Warning"] = "frost",
    ["Excessive Heat Watch"] = "heat", ["Excessive Heat Warning"] = "heat", ["Extreme Heat Watch"] = "heat",
    ["Extreme Heat Warning"] = "heat", ["Heat Advisory"] = "heat",
    ["High Wind Watch"] = "wind", ["High Wind Warning"] = "wind",
}

-- "2026-11-18T14:00:00-08:00" → UTC epoch, or nil.
local function isoEpoch(text)
    local date, h, m, sign, oh, om = (text or ""):match("^(%d+%-%d+%-%d+)T(%d+):(%d+):%d+([%+%-])(%d+):(%d+)")
    local base = date and Sky.epochOf(date)
    if not base then return nil end
    local offset = (tonumber(oh) * 3600 + tonumber(om) * 60) * (sign == "-" and -1 or 1)
    return base + tonumber(h) * 3600 + tonumber(m) * 60 - offset
end
Warnings.isoEpoch = isoEpoch

-- The NWS source (Garden.sources). An answer without `features` fails the
-- source, so it is recorded as missing, not as "no alerts".
Warnings.nws = {
    id = "nws", optional = true,
    headers = { Accept = "application/geo+json" },
    url = function(spot)
        if not Warnings.us(spot) then return nil end
        return ("%s?point=%.4f,%.4f"):format(Warnings.NWS, spot.lat, spot.lon)
    end,
    parse = function(json, forecast)
        local features = assert(type(json) == "table" and type(json.features) == "table" and json.features, "no alert list")
        local list = {}
        for _, feature in ipairs(features) do
            local p = feature.properties or {}
            local kind = Warnings.KEEP[p.event]
            if kind and p.status ~= "Test" and p.messageType ~= "Cancel" then
                list[#list + 1] = { event = p.event, kind = kind, headline = p.headline, severity = p.severity,
                    sender = p.senderName, onset = isoEpoch(p.onset or p.effective), ends = isoEpoch(p.ends or p.expires) }
            end
        end
        forecast.official = { issuer = "National Weather Service", list = list }
    end,
}

-- The alerts still in force at `now`, frost first, then by onset.
function Warnings.active(forecast, now)
    local list = {}
    local official = forecast.official
    for _, alert in ipairs(official and official.list or {}) do
        if not alert.ends or alert.ends > now then list[#list + 1] = alert end
    end
    table.sort(list, function(a, b)
        if (a.kind == "frost") ~= (b.kind == "frost") then return a.kind == "frost" end
        return (a.onset or 0) < (b.onset or 0)
    end)
    return list
end

local function localTime(forecast, epoch)
    local stamp = os.date("!%Y-%m-%dT%H:%M", epoch + (forecast.offset or 0))
    return Sky.weekday(stamp:sub(1, 10)) .. " " .. Units.clock(stamp)
end

-- What NWS says, for the place, at `now`:
--   "none"     not a US place, or the source was not asked
--   "missing"  asked and failed
--   "quiet"    answered, nothing a gardener acts on in force
--   "alerts"   the alerts, in `list`
function Warnings.state(forecast, spot, now)
    if not Warnings.us(spot or forecast.spot) then return "none" end
    if forecast.missing and forecast.missing.nws then return "missing" end
    if not forecast.official then return "none" end
    local list = Warnings.active(forecast, now or forecast.epoch)
    return #list > 0 and "alerts" or "quiet", list
end

-- The official card: the first alert in force, unmodified, with its
-- issuer. Ranked 0, ahead of the app's own frost estimate.
function Warnings.card(forecast, spot, now)
    local state, list = Warnings.state(forecast, spot, now)
    if state ~= "alerts" then return nil end
    local alert = list[1]
    local parts = {}
    if alert.headline then parts[#parts + 1] = alert.headline
    elseif alert.ends then parts[#parts + 1] = "until " .. localTime(forecast, alert.ends) end
    if #list > 1 then parts[#parts + 1] = ("%d more in force"):format(#list - 1) end
    parts[#parts + 1] = alert.sender or "National Weather Service"
    return { id = "official", title = "official warning (NWS)", value = alert.event, rank = 0,
        tone = (alert.event:find("Warning", 1, true) or alert.severity == "Severe" or alert.severity == "Extreme") and "poor"
            or "fair",
        detail = table.concat(parts, " · ") }
end

-- The line the app's frost card adds about the official alerts, or nil.
-- It never claims "no warning" when NWS was not reached.
function Warnings.frostNote(forecast, spot, frostLevel, now)
    local state, list = Warnings.state(forecast, spot, now)
    if state == "missing" then return "NWS alerts not reached; this is our estimate" end
    if state == "alerts" then
        for _, alert in ipairs(list) do
            if alert.kind == "frost" then return "NWS: " .. alert.event .. " in force" end
        end
    end
    if (state == "quiet" or state == "alerts") and frostLevel > 0 then
        return "no NWS frost alert in force (offices stop them after the growing season); this is our estimate"
    end
end

-- The "official warnings" row on the activity page: who issues them here.
function Warnings.metric(forecast, spot, now)
    spot = spot or forecast.spot
    if Warnings.uk(spot) then
        return { label = "official warnings", value = "Met Office",
            note = "Met Office Weather Warnings: " .. Warnings.METOFFICE .. " · not read by this app" }
    end
    local state, list = Warnings.state(forecast, spot, now)
    if state == "missing" then
        return { label = "official warnings", value = "not reached", tone = "fair",
            note = "National Weather Service alerts could not be loaded · check weather.gov" }
    elseif state == "quiet" then
        return { label = "official warnings", value = "none for the garden",
            note = "no frost, freeze, heat or high wind alert in force · National Weather Service" }
    elseif state == "alerts" then
        local names = {}
        for _, alert in ipairs(list) do names[#names + 1] = alert.event end
        return { label = "official warnings", value = list[1].event, tone = "poor",
            note = table.concat(names, ", ") .. " · National Weather Service" }
    end
end

return Warnings
