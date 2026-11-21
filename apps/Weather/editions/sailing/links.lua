-- Station and forecast pages a sailor checks by hand (SA6,
-- Research/services-sailing.md SV-SA6): what we may not read into the app
-- (Holfuy, Windguru, the Met Office's inshore waters, closed networks) or
-- read only in part (a buoy's page, the NWS marine text) is still named on
-- the activity page. Links only: no data from these pages is shown.
--
-- A spot's own `links` (Spots.byId) come first; the rest follow from where
-- it is and the services that cover it.
local Spots = require "Weather/editions/sailing/spots"
local Live = require "Weather/editions/sailing/live"
local Warnings = require "Weather/editions/sailing/warnings"
local Links = {}

-- Pages for a place, { title, address } each, at most `limit` (default 4).
function Links.forPlace(place, limit)
    if not place then return {} end
    local list, seen = {}, {}
    local function add(title, address)
        if address and not seen[address] then seen[address], list[#list + 1] = true, { title, address } end
    end
    for _, link in ipairs((Spots.byId[place.id] or {}).links or {}) do add(link[1], link[2]) end
    for _, station in ipairs(Live.BUOYS[place.id] or {}) do
        add(("NDBC %s %s"):format(station.kind, station.id), "ndbc.noaa.gov/station_page.php?station=" .. station.id)
    end
    for _, service in ipairs(Warnings.services(place)) do
        if service.key == "nws" and place.lat then
            add("NWS forecast and marine text", ("forecast.weather.gov/MapClick.php?lat=%.4f&lon=%.4f"):format(place.lat, place.lon))
        elseif service.key == "meteoalarm" then
            add("MeteoAlarm warnings map", "meteoalarm.org")
        end
    end
    local _, country = Warnings.feed(place)
    if country == "GB" then add("Met Office inshore waters", "weather.metoffice.gov.uk/guides/coast-and-sea/inshore-waters-forecast") end
    limit = limit or 4
    for index = #list, limit + 1, -1 do list[index] = nil end
    return list
end

-- The links as one card on the activity page, or none.
function Links.cards(place)
    local list = Links.forPlace(place)
    if #list == 0 then return {} end
    local parts = {}
    for _, link in ipairs(list) do parts[#parts + 1] = link[1] .. ": " .. link[2] end
    return { { id = "links", title = "check by hand", value = list[1][1], tone = "info", hub = false,
        detail = table.concat(parts, " · ") } }
end

return Links
