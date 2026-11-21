-- US National Weather Service alerts (W10): public domain, keyless, a
-- User-Agent required (Forecast.requestOptions sends the app's). A warning
-- provider for services/warnings.lua; `parse` returns the shared list.
local Util = require "Weather/services/util"

return {
    id = "nws", name = "National Weather Service", site = "weather.gov",
    link = "https://www.weather.gov/",
    countries = { US = true, PR = true, GU = true, VI = true, AS = true, MP = true },
    -- Without a country: the lower 48, Alaska and Hawaii.
    boxes = { { south = 24, north = 50, west = -125, east = -66 }, { south = 51, north = 72, west = -170, east = -129 },
        { south = 18.5, north = 22.5, west = -161, east = -154 } },
    attribution = { "Warnings in the US by the National Weather Service", "Public domain, weather.gov." },
    url = function(spot)
        return ("https://api.weather.gov/alerts/active?point=%.4f,%.4f"):format(spot.lat, spot.lon)
    end,
    headers = { Accept = "application/geo+json" },
    parse = function(json)
        local features = assert(type(json.features) == "table" and json.features, "no alert list")
        local list = {}
        for _, feature in ipairs(features) do
            local p = feature.properties or {}
            if p.event and p.status ~= "Test" then
                list[#list + 1] = { event = p.event, severity = p.severity, sender = p.senderName,
                    ends = Util.isoEpoch(p.ends or p.expires), issued = Util.isoEpoch(p.sent) }
            end
        end
        return list
    end,
}
