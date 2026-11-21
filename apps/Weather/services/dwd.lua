-- Deutscher Wetterdienst warnings through Bright Sky (`/alerts?lat=&lon=`):
-- keyless JSON. DWD's data are open with a source credit (GeoNutzV,
-- CC BY 4.0 compatible); Bright Sky is MIT-licensed code on a hobby-run
-- instance, so it stays optional and DWD direct (CAP) is the fallback. A
-- warning provider for services/warnings.lua.
--
-- Shape read (brightsky.dev/docs, /alerts; tested on a synthetic fixture):
-- `alerts[]` with `event_en`, `headline_en`, `event_de`, `severity`
-- (minor … extreme), `status`, `effective`, `expires`; `location` names
-- the DWD warn cell.
local Util = require "Weather/services/util"

return {
    id = "dwd", name = "Deutscher Wetterdienst", site = "dwd.de",
    link = "https://www.dwd.de/DE/wetter/warnungen_gemeinden/warnWetter_node.html",
    countries = { DE = true },
    attribution = { "Warnings in Germany by Deutscher Wetterdienst, via Bright Sky",
        "Source: Deutscher Wetterdienst (dwd.de), open data with source credit; served by brightsky.dev." },
    url = function(spot)
        return ("https://api.brightsky.dev/alerts?lat=%.4f&lon=%.4f"):format(spot.lat, spot.lon)
    end,
    parse = function(json)
        local alerts = assert(type(json.alerts) == "table" and json.alerts, "no alert list")
        local list = {}
        for _, alert in ipairs(alerts) do
            local event = alert.event_en or alert.headline_en or alert.event_de
            if type(event) == "string" and alert.status ~= "test" then
                list[#list + 1] = { event = event, severity = Util.capital(alert.severity), sender = "Deutscher Wetterdienst",
                    ends = Util.isoEpoch(alert.expires), issued = Util.isoEpoch(alert.effective) }
            end
        end
        return list
    end,
}
