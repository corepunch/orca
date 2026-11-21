-- Beach warnings from the US National Weather Service (SV-SU1): rip
-- currents, beach hazards (sneaker waves), high surf and tsunamis, with
-- the time each ends. The same api.weather.gov alert list the full app
-- reads for everyday warnings (activities/outdoors.lua), filtered to the
-- ones about the beach. Public domain, keyless; every request carries the
-- app's User-Agent. Outside the US nothing is asked, and an answer without
-- an alert list fails the source: neither reads as "no warnings".
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Outdoors = require "Weather/activities/outdoors"
local Hazards = { BASE = "https://api.weather.gov/alerts/active" }

-- The beach events, the worst first: `cap` limits an hour's score while
-- one is in force (`beginner` for a beginner), `advice` follows it.
Hazards.EVENTS = {
    ["Tsunami Warning"] = { order = 1, cap = 0, advice = "leave the beach and the water" },
    ["High Surf Warning"] = { order = 2, cap = 20, advice = "dangerous, breaking waves: stay out of the water" },
    ["Tsunami Advisory"] = { order = 3, cap = 10, advice = "strong currents: stay out of the water" },
    ["High Surf Advisory"] = { order = 4, cap = 50, beginner = 25, advice = "big, dangerous surf for most surfers" },
    ["Rip Current Statement"] = { order = 5, cap = 60, beginner = 40,
        advice = "surf near a lifeguard; if a rip pulls you, paddle along the beach, not against it" },
    ["Beach Hazards Statement"] = { order = 6, cap = 60, beginner = 45,
        advice = "sneaker waves and rips: never turn your back on the sea" },
    ["Tsunami Watch"] = { order = 7, advice = "listen for official instructions" },
}

Hazards.source = {
    id = "surf-hazards", optional = true,
    url = function(spot)
        if not Outdoors.nwsCovers(spot) then return nil end
        return ("%s?point=%.4f,%.4f"):format(Hazards.BASE, spot.lat, spot.lon)
    end,
    headers = { Accept = "application/geo+json" },
    parse = function(json, forecast) forecast.beachHazards = Hazards.parse(json) end,
}

-- The beach events in an alerts GeoJSON, the worst first. Raises without
-- a `features` list.
function Hazards.parse(json)
    local features = assert(type(json) == "table" and type(json.features) == "table" and json.features, "no alert list")
    local list = {}
    for _, feature in ipairs(features) do
        local p = type(feature) == "table" and feature.properties or {}
        local kind = Hazards.EVENTS[p.event]
        if kind and p.status ~= "Test" then
            list[#list + 1] = { event = p.event, sender = p.senderName, order = kind.order,
                starts = Outdoors.isoEpoch(p.onset or p.effective), ends = Outdoors.isoEpoch(p.ends or p.expires) }
        end
    end
    table.sort(list, function(a, b) return a.order < b.order end)
    return list
end

-- The warnings in force at `epoch`, the worst first.
function Hazards.at(forecast, epoch)
    local list = {}
    for _, hazard in ipairs(forecast and forecast.beachHazards or {}) do
        if epoch and (not hazard.starts or hazard.starts <= epoch) and (not hazard.ends or hazard.ends > epoch) then
            list[#list + 1] = hazard
        end
    end
    return list
end

-- Caps a rating card while a warning is in force at the hour.
function Hazards.rate(card, hour, forecast, level)
    local hazard = Hazards.at(forecast, hour.epoch)[1]
    local kind = hazard and Hazards.EVENTS[hazard.event]
    if not kind then return end
    local cap = level == "beginner" and kind.beginner or kind.cap
    local text = ("%s in force: %s."):format(hazard.event, kind.advice)
    if cap then card:cap(cap, text) else card:note(text) end
end

local function untilText(hazard, forecast)
    if not hazard.ends then return "" end
    local stamp = os.date("!%Y-%m-%dT%H:%M", hazard.ends + (forecast.offset or 0))
    return "until " .. Sky.weekday(stamp:sub(1, 10)) .. " " .. Units.clock(stamp) .. " · "
end

-- The worst warning now as a card that leads the hub, or nil.
function Hazards.card(forecast)
    local list = Hazards.at(forecast, forecast.epoch)
    local hazard = list[1]
    if not hazard then return nil end
    local more = #list > 1 and (" · and %d more"):format(#list - 1) or ""
    return { id = "hazard", title = "beach warning", value = hazard.event:lower(), tone = "poor", rank = 0,
        detail = untilText(hazard, forecast) .. Hazards.EVENTS[hazard.event].advice .. more .. " · "
            .. (hazard.sender or "National Weather Service") }
end

Hazards.attribution = { "Warnings: U.S. National Weather Service", "Beach and surf warnings from api.weather.gov, public domain." }

return Hazards
