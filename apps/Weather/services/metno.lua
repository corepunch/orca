-- MET Norway's MetAlerts 2.0 by point (`current.json?lat=&lon=`): keyless
-- GeoJSON under CC BY 4.0, an identifying User-Agent asked. The point
-- query beats MeteoAlarm's regions in Norway. A warning provider for
-- services/warnings.lua.
--
-- Shape read (docs.api.met.no/doc/metalerts/geojson; tested on a synthetic
-- fixture): `features[]` with `when.interval` [onset, end] and
-- `properties` `eventAwarenessName` (localised with `lang`), `title`,
-- `severity` (Minor … Extreme), `awareness_level` ("2; yellow; Moderate")
-- and `web`.
local Util = require "Weather/services/util"

local LEVEL = { yellow = "Moderate", orange = "Severe", red = "Extreme" }

return {
    id = "metno", name = "MET Norway", site = "met.no",
    link = "https://www.met.no/en/weather-and-climate/warnings",
    countries = { NO = true, SJ = true },
    attribution = { "Warnings in Norway by MET Norway",
        "Norwegian Meteorological Institute (MET Norway), CC BY 4.0, api.met.no." },
    url = function(spot)
        return ("https://api.met.no/weatherapi/metalerts/2.0/current.json?lat=%.4f&lon=%.4f&lang=en")
            :format(spot.lat, spot.lon)
    end,
    parse = function(json)
        local features = assert(type(json.features) == "table" and json.features, "no alert list")
        local list = {}
        for _, feature in ipairs(features) do
            local p = feature.properties or {}
            local interval = feature.when and feature.when.interval or {}
            local colour = tostring(p.awareness_level or ""):match("^%d+;%s*(%a+)")
            local event = p.eventAwarenessName or p.title or p.event
            if type(event) == "string" and p.status ~= "Test" then
                list[#list + 1] = { event = event, severity = p.severity or LEVEL[colour or ""], sender = "MET Norway",
                    ends = Util.isoEpoch(interval[2]), link = type(p.web) == "string" and p.web or nil }
            end
        end
        return list
    end,
}
