-- Official warnings, one registry for every activity and edition (SV-W1).
-- Open-Meteo has no warnings, so the app asks the national service that
-- covers a place, and where none can be read it says where to look:
-- never "no warnings".
--
-- API (editions reuse it as it is):
--
--   Warnings.coverage(spot)  → "covered", provider  (a service is read here)
--                            → "link", entry        (an official site to see)
--                            → "none", entry|nil    (no service known; the
--                              entry points at the WMO's list of services)
--   Warnings.sources()       the `sources` entries, one per provider, each
--                            asking only where coverage picks it (optional)
--   Warnings.source(id)      one of them ("nws", "eccc", "metno", "dwd")
--   Warnings.attribution()   { { title, detail } } for an activity's credit
--   Warnings.cards(forecast, now, options)
--                            the warnings in force as cards, the most severe
--                            first (at most three); with `options.note`, a
--                            calm card where nothing is read: "see the Met
--                            Office", or "not available right now"
--   Warnings.register(provider, position)
--                            adds a provider (a key arrives, an edition's
--                            own service); `Warnings.links` holds link-only
--                            entries the same way
--
-- A provider is `{ id, name, site, link, countries = { CC = true }, boxes,
-- attribution = { title, detail }, url(spot, Forecast), parse(json, forecast)
-- → list, headers, format }`. `parse` returns the shared warning shape
-- `{ event, severity = "Minor" … "Extreme", sender, ends, issued, link,
-- area }` (epochs in UTC; `area` a GeoJSON geometry the place must lie in)
-- and raises when the answer has no list, so a broken answer fails the
-- source instead of reading as "no warnings". The source writes the list to
-- `forecast.warnings` and the provider's id to `forecast.warningsFrom`.
--
-- Coverage: a place's ISO country (`spot.country`, from search or "use my
-- location") picks first; without one, the providers' and links' `boxes`
-- do, in order. Curated places have no country, so a box should only be
-- given where it does not reach far into a neighbour.
local Util = require "Weather/services/util"
local Sky = require "Weather/sky"
local Units = require "Weather/units"

local Warnings = {}
Warnings.isoEpoch = Util.isoEpoch

Warnings.providers = {
    require "Weather/services/nws",
    require "Weather/services/eccc",
    require "Weather/services/metno",
    require "Weather/services/dwd",
}

-- Where the official warnings cannot be read here (no key, a
-- non-commercial licence, CAP XML only, or a licence still to confirm).
local EUROPE = { AT = true, BE = true, BA = true, BG = true, HR = true, CY = true, CZ = true, DK = true, EE = true,
    FI = true, FR = true, GR = true, HU = true, IS = true, IE = true, IL = true, IT = true, LV = true, LT = true,
    LU = true, MT = true, MD = true, ME = true, NL = true, MK = true, PL = true, PT = true, RO = true, RS = true,
    SK = true, SI = true, ES = true, SE = true, CH = true, UA = true, NO = true, DE = true }
Warnings.links = {
    -- NSWWS needs a key and a request form: then it becomes a provider.
    { id = "metoffice", name = "the Met Office", site = "metoffice.gov.uk/weather/warnings-and-advice",
        link = "https://www.metoffice.gov.uk/weather/warnings-and-advice", countries = { GB = true, IM = true, JE = true, GG = true },
        boxes = { { south = 49.9, north = 60.9, west = -6.5, east = 1.8 } } },
    -- BoM's free feeds are for personal, non-commercial use only.
    { id = "bom", name = "the Bureau of Meteorology", site = "bom.gov.au", link = "http://www.bom.gov.au/australia/warnings/",
        countries = { AU = true }, boxes = { { south = -44, north = -10, west = 112, east = 154 } } },
    { id = "metservice", name = "MetService", site = "metservice.com/warnings", link = "https://www.metservice.com/warnings/home",
        countries = { NZ = true }, boxes = { { south = -47.5, north = -34, west = 166, east = 179 } } },
    -- JMA's bosai JSON is undocumented and needs an area-code table.
    { id = "jma", name = "the Japan Meteorological Agency", site = "jma.go.jp", link = "https://www.jma.go.jp/bosai/warning/",
        countries = { JP = true }, boxes = { { south = 24, north = 46, west = 122, east = 146 } } },
    -- MeteoAlarm: licence to confirm in writing (meteoalarm@geosphere.at)
    -- before its feed is read; until then, a link. Norway and Germany are
    -- read above when the place has its country.
    { id = "meteoalarm", name = "MeteoAlarm", site = "meteoalarm.org", link = "https://www.meteoalarm.org/",
        countries = EUROPE, boxes = { { south = 34, north = 72, west = -25, east = 45 } } },
}
Warnings.WMO = { id = "wmo", name = "your national weather service", site = "severeweather.wmo.int",
    link = "https://severeweather.wmo.int/" }

function Warnings.register(provider, position)
    table.insert(Warnings.providers, position or #Warnings.providers + 1, provider)
    Warnings.cache = nil
end

function Warnings.coverage(spot)
    if not (spot and spot.lat and spot.lon) then return "none", nil end
    local country = Util.country(spot)
    for _, pass in ipairs { { "covered", Warnings.providers }, { "link", Warnings.links } } do
        for _, entry in ipairs(pass[2]) do
            if country and entry.countries and entry.countries[country]
                or not country and Util.inBoxes(spot, entry.boxes) then return pass[1], entry end
        end
    end
    return "none", Warnings.WMO
end

-- One optional source per provider, asked only where coverage picks it.
local function sourceOf(provider)
    return {
        id = "warnings-" .. provider.id, optional = true, headers = provider.headers, format = provider.format,
        url = function(spot, Forecast)
            local state, entry = Warnings.coverage(spot)
            if state ~= "covered" or entry ~= provider then return nil end
            return provider.url(spot, Forecast)
        end,
        parse = function(json, forecast)
            local list = provider.parse(json, forecast)
            for _, warning in ipairs(list) do
                warning.sender = warning.sender or provider.name
                warning.link = warning.link or provider.link
            end
            forecast.warnings, forecast.warningsFrom = list, provider.id
        end,
    }
end

function Warnings.sources()
    if not Warnings.cache then
        Warnings.cache = {}
        for _, provider in ipairs(Warnings.providers) do Warnings.cache[#Warnings.cache + 1] = sourceOf(provider) end
    end
    return Warnings.cache
end

function Warnings.source(id)
    for _, source in ipairs(Warnings.sources()) do
        if source.id == "warnings-" .. id then return source end
    end
end

function Warnings.attribution()
    local list = {}
    for _, provider in ipairs(Warnings.providers) do list[#list + 1] = provider.attribution end
    return list
end

local SEVERE = { Extreme = true, Severe = true }

-- "thu 19 18:00" in the place's clock.
local function stamp(epoch, forecast)
    local text = os.date("!%Y-%m-%dT%H:%M", epoch + (forecast.offset or 0))
    return Sky.weekday(text:sub(1, 10)) .. " " .. Units.clock(text)
end

-- The calm card where no warning list is read here, or nil.
local function note(forecast, spot)
    local state, entry = Warnings.coverage(spot)
    if state == "covered" then
        if not (forecast.missing and forecast.missing["warnings-" .. entry.id]) then return nil end
        return { value = "not available right now", detail = ("%s did not answer · see %s"):format(entry.name, entry.site),
            link = entry.link }
    end
    if state == "link" then
        return { value = "see " .. entry.name, detail = "official site: " .. entry.site, link = entry.link }
    end
    if entry then
        return { value = "not checked here", detail = "ask " .. entry.name .. " · " .. entry.site, link = entry.link,
            hub = false }
    end
end

function Warnings.cards(forecast, now, options)
    options = options or {}
    local spot = options.spot or forecast.spot
    now = now or forecast.epoch
    local list, cards, seen = {}, {}, {}
    for _, warning in ipairs(forecast.warnings or {}) do
        local key = tostring(warning.event) .. "|" .. tostring(warning.ends)
        local here = not (warning.area and spot and spot.lat) or Util.contains(warning.area, spot.lon, spot.lat)
        if here and not seen[key] and (not warning.ends or warning.ends > now) then
            seen[key], list[#list + 1] = true, warning
        end
    end
    table.sort(list, function(a, b)
        if (SEVERE[a.severity] or false) ~= (SEVERE[b.severity] or false) then return SEVERE[a.severity] or false end
        return (a.ends or math.huge) < (b.ends or math.huge)
    end)
    for index = 1, math.min(3, #list) do
        local warning = list[index]
        local parts = {}
        if warning.ends then parts[#parts + 1] = "until " .. stamp(warning.ends, forecast) end
        if warning.issued then parts[#parts + 1] = "issued " .. stamp(warning.issued, forecast) end
        parts[#parts + 1] = warning.sender or "official warning"
        cards[index] = { id = "warning-" .. index, title = "official warning", value = warning.event:lower(),
            detail = table.concat(parts, " · "), tone = SEVERE[warning.severity] and "poor" or "fair", link = warning.link }
    end
    if #cards == 0 and options.note then
        local card = note(forecast, spot)
        if card then
            card.id, card.title, card.tone, card.rank = "warnings", "official warnings", "info", 5
            cards[1] = card
        end
    end
    return cards
end

return Warnings
