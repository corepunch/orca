-- Environment and Climate Change Canada's public alerts through the GeoMet
-- OGC API (`weather-alerts`): keyless GeoJSON, free under ECCC's Data
-- Servers End-use Licence with the credit below. One request for a small
-- bbox around the spot; the warning cards then keep the alerts whose area
-- holds the spot (point in polygon). A warning provider for services/warnings.lua.
--
-- Shape read (the collection's documented queryables; tested on a
-- synthetic fixture): `features[].properties` with `alert_name_en` /
-- `alert_name_fr`, `alert_type` (warning, watch, advisory, statement),
-- `status_en`, `publication_datetime`, `event_end_datetime` or
-- `expiration_datetime`; `features[].geometry` a Polygon or MultiPolygon.
local Util = require "Weather/services/util"

local SEVERITY = { warning = "Severe", watch = "Moderate", advisory = "Moderate", statement = "Minor" }
local PAD = 0.05 -- degrees around the spot for the bbox

return {
    id = "eccc", name = "Environment Canada", site = "weather.gc.ca",
    link = "https://weather.gc.ca/warnings/index_e.html",
    countries = { CA = true },
    -- Without a country: north of the US boxes, and the Maritimes.
    boxes = { { south = 50, north = 84, west = -141, east = -52 }, { south = 43, north = 50, west = -66, east = -52 } },
    attribution = { "Warnings in Canada by Environment and Climate Change Canada",
        "Data source: Environment and Climate Change Canada, under its Data Servers End-use Licence (api.weather.gc.ca)." },
    url = function(spot)
        return ("https://api.weather.gc.ca/collections/weather-alerts/items?f=json&bbox=%.3f,%.3f,%.3f,%.3f")
            :format(spot.lon - PAD, spot.lat - PAD, spot.lon + PAD, spot.lat + PAD)
    end,
    headers = { Accept = "application/geo+json" },
    -- Each alert keeps its polygon as `area`: Warnings.cards keeps only the
    -- ones whose area holds the place (the forecast knows its spot by then).
    parse = function(json)
        local features = assert(type(json.features) == "table" and json.features, "no alert list")
        local french = Util.language() == "fr"
        local list = {}
        for _, feature in ipairs(features) do
            local p = feature.properties or {}
            local name = french and p.alert_name_fr or p.alert_name_en or p.alert_name_fr
            local status = tostring(p.status_en or ""):lower()
            if type(name) == "string" and not status:find("end") and not status:find("cancel") then
                list[#list + 1] = { event = name, severity = SEVERITY[tostring(p.alert_type or ""):lower()] or "Moderate",
                    sender = "Environment Canada", ends = Util.isoEpoch(p.event_end_datetime or p.expiration_datetime),
                    issued = Util.isoEpoch(p.publication_datetime), area = feature.geometry }
            end
        end
        return list
    end,
}
