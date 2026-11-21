-- Existing services the hiking edition reads besides Open-Meteo
-- (Research/services-hiking.md, SV-HI1 to SV-HI7). Each is an optional
-- source: the forecast and the rating stand without it, and a service that
-- fails or stays silent never reads as "safe" or "no warnings". Results go
-- under `forecast.hiking.<key>` and show as cards; the rating stays
-- weather-only (lightning potential is a weather model field).
--
--   hiking-avalanche   avalanche.org map layer (US centres)        SV-HI1
--   hiking-bulletin    EAWS CAAML v6 JSON (SLF, Euregio)           SV-HI2
--   hiking-fire        WFIGS current fire perimeters (US)          SV-HI3
--   hiking-fire-starts WFIGS current incident points (US)          SV-HI3
--   hiking-snow        NRCS AWDB SNOTEL measured snow depth        SV-HI4
--   hiking-park        NPS alerts, only with Forecast.endpoints.npsKey  SV-HI5
--   hiking-lightning   Open-Meteo ICON-D2 lightning potential      SV-HI7
--
-- `Services.CLEARED` names the providers whose terms allow showing their
-- data. A provider whose licence is still unclear is off: its peaks keep
-- the link to the centre (peaks.lua `avalanche = { name, where }`).
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Sources = require "Weather/editions/hiking/sources"
local Avalanche = require "Weather/editions/hiking/avalanche"
local Peaks = require "Weather/editions/hiking/peaks"
local Trail = require "Weather/editions/hiking/trail"
local Services = {}

-- avalanche.org asks for written permission (National Avalanche Center),
-- Euregio's data licence is disputed: both stay links until they answer.
-- SLF is CC BY 4.0.
Services.CLEARED = { avalancheOrg = false, slf = true, euregio = false }

-- Base URLs of the services that are not Open-Meteo (verify each once on a
-- network that reaches them; this research sandbox could not).
Services.endpoints = {
    avalancheOrg = "https://api.avalanche.org/v2/public/products/map-layer/",
    slf = "https://aws.slf.ch/api/bulletin/caaml/en/json",
    euregio = "https://static.avalanche.report/bulletins/latest/EUREGIO_en_CAAMLv6.json",
    perimeters = "https://services3.arcgis.com/T4QMspbfLg3qTGWY/arcgis/rest/services/"
        .. "WFIGS_Interagency_Perimeters_Current/FeatureServer/0/query",
    incidents = "https://services3.arcgis.com/T4QMspbfLg3qTGWY/arcgis/rest/services/"
        .. "WFIGS_Incident_Locations_Current/FeatureServer/0/query",
    awdb = "https://wcc.sc.egov.usda.gov/awdbRestApi/services/v1/data",
    nps = "https://developer.nps.gov/api/v1/alerts",
}
Services.FIRE_KM = 30

local function state(forecast)
    forecast.hiking = forecast.hiking or {}
    return forecast.hiking
end

-- Snow at the top, from the forecast when the source is given one: lying
-- snow, or snow in the last three days. Without a forecast, ask anyway.
local function snowy(forecast)
    if not (forecast and forecast.hours and forecast.now) then return true end
    for index = math.max(1, forecast.now - 72), math.min(#forecast.hours, forecast.now + 23) do
        local hour = forecast.hours[index]
        local at = hour.top or hour
        if (at.snow_depth or 0) > 0 or (index <= forecast.now and (at.snowfall or 0) > 0) then return true end
    end
    return false
end

-- SV-HI1: one centre's zones; asked only when there is snow (when the
-- foundation hands `url` the forecast) and avalanche.org has said yes.
local avalancheOrg = {
    id = "hiking-avalanche", optional = true,
    url = function(spot, _, forecast)
        local peak = Sources.peak(spot)
        local centre = peak and peak.avalanche and peak.avalanche.center
        if not (centre and Services.CLEARED.avalancheOrg and snowy(forecast)) then return nil end
        return Services.endpoints.avalancheOrg .. centre
    end,
    -- The zone is picked when the card is drawn (it knows the place).
    parse = function(json, forecast)
        assert(type(json) == "table" and type(json.features) == "table", "no zones")
        state(forecast).avalancheOrg = json
    end,
}

-- SV-HI2: the provider's latest bulletins, the peak's micro-region picked out.
local bulletin = {
    id = "hiking-bulletin", optional = true,
    url = function(spot, _, forecast)
        local peak = Sources.peak(spot)
        local caaml = peak and peak.avalanche and peak.avalanche.caaml
        if not (caaml and caaml.region and Services.CLEARED[caaml.provider] and snowy(forecast)) then return nil end
        return Services.endpoints[caaml.provider]
    end,
    headers = { Accept = "application/json" },
    parse = function(json, forecast)
        assert(type(json) == "table" and type(json.bulletins) == "table", "no bulletins")
        state(forecast).bulletins = json
    end,
}

-- An ArcGIS answer: its features, or an error (ArcGIS answers 200 with
-- `error` when a query fails, which is not "no fires").
local function features(json)
    assert(type(json) == "table" and not json.error, json and json.error and json.error.message or "no answer")
    return assert(type(json.features) == "table" and json.features, "no feature list")
end

local function arcgis(base, at, fields, extra)
    return ("%s?geometry=%.4f,%.4f&geometryType=esriGeometryPoint&inSR=4326&spatialRel=esriSpatialRelIntersects"
        .. "&distance=%d&units=esriSRUnit_Kilometer&outFields=%s%s&outSR=4326&f=json")
        :format(base, at.lon, at.lat, Services.FIRE_KM, fields, extra or "")
end

-- SV-HI3: perimeters within 30 km of the trailhead, with their centroids.
-- No relative date filter (NIFC: it loads the service, and stale fires fall off).
local fire = {
    id = "hiking-fire", optional = true,
    url = function(spot)
        local peak = Sources.peak(spot)
        if not (peak and peak.nws) then return nil end
        return arcgis(Services.endpoints.perimeters, peak.start,
            "poly_IncidentName,attr_IncidentSize,attr_PercentContained,attr_ModifiedOnDateTime_dt",
            "&returnGeometry=false&returnCentroid=true")
    end,
    parse = function(json, forecast)
        local list = {}
        for _, feature in ipairs(features(json)) do
            local a, c = feature.attributes or {}, feature.centroid or {}
            if c.x and c.y then
                list[#list + 1] = { name = a.poly_IncidentName, acres = a.attr_IncidentSize,
                    contained = a.attr_PercentContained, updated = a.attr_ModifiedOnDateTime_dt
                    and a.attr_ModifiedOnDateTime_dt // 1000, lat = c.y, lon = c.x }
            end
        end
        state(forecast).fires = list
    end,
}

-- New starts without a perimeter yet: wildfire incident points.
local fireStarts = {
    id = "hiking-fire-starts", optional = true,
    url = function(spot)
        local peak = Sources.peak(spot)
        if not (peak and peak.nws) then return nil end
        return arcgis(Services.endpoints.incidents, peak.start,
            "IncidentName,IncidentSize,PercentContained,ModifiedOnDateTime_dt",
            "&where=IncidentTypeCategory%3D%27WF%27&returnGeometry=true")
    end,
    parse = function(json, forecast)
        local list = {}
        for _, feature in ipairs(features(json)) do
            local a, g = feature.attributes or {}, feature.geometry or {}
            if g.x and g.y then
                list[#list + 1] = { name = a.IncidentName, acres = a.IncidentSize, contained = a.PercentContained,
                    updated = a.ModifiedOnDateTime_dt and a.ModifiedOnDateTime_dt // 1000, lat = g.y, lon = g.x, start = true }
            end
        end
        state(forecast).fireStarts = list
    end,
}

-- SV-HI4: SNOTEL snow depth (inches) for four days, newest last.
local function day(epoch) return os.date("!%Y-%m-%d", epoch) end
local snow = {
    id = "hiking-snow", optional = true,
    url = function(spot)
        local peak = Sources.peak(spot)
        local station = peak and peak.station
        if not (station and station.kind == "snotel") then return nil end
        local now = os.time()
        return ("%s?stationTriplets=%s&elements=SNWD&duration=DAILY&beginDate=%s&endDate=%s")
            :format(Services.endpoints.awdb, station.id, day(now - 3 * 86400), day(now))
    end,
    parse = function(json, forecast)
        assert(type(json) == "table" and type(json[1]) == "table", "no stations")
        local values
        for _, item in ipairs(json[1].data or {}) do
            if not item.stationElement or item.stationElement.elementCode == "SNWD" then values = item.values; break end
        end
        assert(type(values) == "table", "no snow depth")
        local readings = {}
        for _, v in ipairs(values) do
            local epoch = Sky.epochOf(v.date)
            if epoch and type(v.value) == "number" then readings[#readings + 1] = { epoch = epoch, cm = v.value * 2.54 } end
        end
        table.sort(readings, function(a, b) return a.epoch < b.epoch end)
        local newest = readings[#readings]
        -- A reading older than two days says nothing about today's trail.
        if not newest or (forecast.epoch or 0) - newest.epoch > 2 * 86400 + 86399 then
            state(forecast).measured = { stale = true }
            return
        end
        local oldest = readings[1]
        state(forecast).measured = { cm = newest.cm, epoch = newest.epoch,
            change = oldest ~= newest and newest.cm - oldest.cm or nil,
            days = oldest ~= newest and (newest.epoch - oldest.epoch) // 86400 or nil }
    end,
}

-- SV-HI5: the NPS key is shared by every user of the app (1,000 requests
-- an hour), so it is asked only with a key and saved with the forecast.
local function npsKey() return require("Weather/forecast").endpoints.npsKey end
local park = {
    id = "hiking-park", optional = true,
    url = function(spot)
        local peak = Sources.peak(spot)
        if not (peak and peak.nps and npsKey()) then return nil end
        return ("%s?parkCode=%s&limit=20"):format(Services.endpoints.nps, peak.nps)
    end,
    headers = function() return { ["X-Api-Key"] = npsKey() } end,
    parse = function(json, forecast)
        local data = assert(type(json) == "table" and type(json.data) == "table" and json.data, "no alert list")
        local list = {}
        for _, alert in ipairs(data) do
            if alert.title then list[#list + 1] = { title = alert.title, category = alert.category, url = alert.url } end
        end
        state(forecast).park = list
    end,
}

-- SV-HI7: ICON-D2's lightning potential index (J/kg) at the top, two days,
-- for peaks inside its central-European domain (`iconD2` in peaks.lua).
local lightning = {
    id = "hiking-lightning", optional = true,
    url = function(spot, Forecast)
        local peak = Sources.peak(spot)
        if not (peak and peak.iconD2) then return nil end
        return Forecast.url("forecast", ("latitude=%s&longitude=%s&elevation=%d&hourly=lightning_potential"
            .. "&models=icon_d2&timezone=GMT&forecast_days=2"):format(peak.top.lat, peak.top.lon, peak.top.elevation))
    end,
    parse = function(json, forecast)
        local h = assert(json.hourly and json.hourly.lightning_potential and json.hourly, "no lightning potential")
        local byTime = {}
        for _, hour in ipairs(forecast.hours or {}) do
            if hour.epoch then byTime[os.date("!%Y-%m-%dT%H:00", hour.epoch)] = hour end
        end
        for index, time in ipairs(h.time or {}) do
            local hour = byTime[time]
            if hour then hour.lightning = h.lightning_potential[index] end
        end
        state(forecast).lightning = true
    end,
}

-- The avalanche rating for a place: what its provider answered, read for
-- its top; nil when nothing was asked or the answer did not load.
function Services.avalanche(forecast, peak)
    local hiking = forecast.hiking or {}
    local a = peak and peak.avalanche
    if not a then return nil end
    if a.center and hiking.avalancheOrg then return Avalanche.mapLayer(hiking.avalancheOrg, peak) end
    if a.caaml and hiking.bulletins then return Avalanche.caaml(hiking.bulletins, peak) end
    return nil
end

-- The source's error when the place's avalanche source was asked and failed.
function Services.avalancheMissing(forecast, peak)
    local a, missing = peak and peak.avalanche, forecast.missing or {}
    if not a then return nil end
    return a.center and missing["hiking-avalanche"] or a.caaml and missing["hiking-bulletin"] or nil
end

Services.list = { avalancheOrg, bulletin, fire, fireStarts, snow, park, lightning }

-- Great-circle distance (km) and the bearing from `a` to `b`.
function Services.distance(a, b)
    local r, rad = 6371, math.pi / 180
    local la1, la2, dlat, dlon = a.lat * rad, b.lat * rad, (b.lat - a.lat) * rad, (b.lon - a.lon) * rad
    local h = math.sin(dlat / 2) ^ 2 + math.cos(la1) * math.cos(la2) * math.sin(dlon / 2) ^ 2
    local km = 2 * r * math.asin(math.min(1, math.sqrt(h)))
    local y = math.sin(dlon) * math.cos(la2)
    local x = math.cos(la1) * math.sin(la2) - math.sin(la1) * math.cos(la2) * math.cos(dlon)
    return km, (math.deg(math.atan(y, x)) + 360) % 360
end

local function area(acres)
    if not acres then return nil end
    if Units.current("distance") == "mi" then return ("%d acres"):format(math.floor(acres + 0.5)) end
    return ("%d ha"):format(math.floor(acres * 0.4047 + 0.5))
end

-- The nearest fire (perimeters, then new starts not already among them),
-- from the trailhead: "fire 12 km NE".
function Services.fireCard(forecast, peak)
    local hiking = forecast.hiking or {}
    if not (peak and peak.start and (hiking.fires or hiking.fireStarts)) then return nil end
    local named, all = {}, {}
    for _, f in ipairs(hiking.fires or {}) do all[#all + 1] = f; if f.name then named[f.name:lower()] = true end end
    for _, f in ipairs(hiking.fireStarts or {}) do
        if not (f.name and named[f.name:lower()]) then all[#all + 1] = f end
    end
    local near, km, bearing
    for _, f in ipairs(all) do
        local d, b = Services.distance(peak.start, f)
        if not km or d < km then near, km, bearing = f, d, b end
    end
    if not near then return nil end
    local parts = { (near.name or "unnamed fire") .. (near.start and " (new start)" or "") }
    parts[#parts + 1] = area(near.acres)
    if near.contained then parts[#parts + 1] = ("%d %% contained"):format(math.floor(near.contained + 0.5)) end
    if near.updated then parts[#parts + 1] = "updated " .. Avalanche.at(near.updated, forecast) end
    if #all > 1 then parts[#parts + 1] = ("%d fires within %s"):format(#all, Units.distance(Services.FIRE_KM)) end
    local order = peak.forest and (peak.forest[1] .. ", " .. peak.forest[2]) or "the forest or park order"
    parts[#parts + 1] = "closures can reach far beyond the fire; check " .. order
    return { id = "fire", title = "wildfire near the trail",
        value = ("fire %s %s"):format(Units.distance(km), Sky.compass(bearing)),
        tone = km < 10 and "poor" or "fair", detail = table.concat(parts, " · ") }
end

-- Park alerts: a closure or danger is poor, a caution fair; information
-- alone gives no card.
local PARK = { ["Park Closure"] = { 1, "park closure", "poor" }, Danger = { 2, "danger", "poor" },
    Caution = { 3, "caution", "fair" } }
function Services.parkCard(forecast, peak)
    local list = forecast.hiking and forecast.hiking.park
    if not (list and peak) then return nil end
    local first, worst
    for _, alert in ipairs(list) do
        local kind = PARK[alert.category]
        if kind and (not worst or kind[1] < worst[1]) then first, worst = alert, kind end
    end
    if not first then return nil end
    local parts = { first.title }
    if #list > 1 then parts[#parts + 1] = ("%d park alerts in all"):format(#list) end
    parts[#parts + 1] = first.url or ("nps.gov/" .. peak.nps .. "/planyourvisit/conditions.htm")
    return { id = "park", title = "park alerts", value = worst[2], tone = worst[3], detail = table.concat(parts, " · "),
        link = first.url }
end

-- The measured snow line for the trail card: "measured 85 cm at Echo Peak
-- (2377 m), yesterday; +20 cm in 3 days". nil without a fresh reading.
function Services.measuredSnow(forecast, peak)
    local m = forecast.hiking and forecast.hiking.measured
    local station = peak and peak.station
    if not (m and m.cm and station) then return nil end
    local today = os.date("!%Y-%m-%d", (forecast.epoch or 0) + (forecast.offset or 0))
    local date = os.date("!%Y-%m-%d", m.epoch)
    local when = date == today and "today" or date == os.date("!%Y-%m-%d", Sky.epochOf(today) - 86400) and "yesterday"
        or Sky.weekday(date)
    local text = ("measured %s at %s (%s), %s"):format(Trail.snow(m.cm), station.name,
        Units.height(station.elevation, 0), when)
    if m.change and math.abs(m.change) >= 1 then
        text = text .. ("; %s%s in %d days"):format(m.change > 0 and "+" or "−", Trail.snow(math.abs(m.change)), m.days)
    end
    return text, m.cm
end

-- The services this edition credits on the settings' "about", for the
-- ones that are on.
function Services.attribution()
    local list = { { "Warnings: US National Weather Service", "Public domain, weather.gov." } }
    local centres, seen = {}, {}
    for _, peak in pairs(Peaks) do
        local a = peak.avalanche
        if a and a.center and not seen[a[1]] then seen[a[1]], centres[#centres + 1] = true, a[1] end
    end
    table.sort(centres)
    if Services.CLEARED.avalancheOrg and #centres > 0 then
        list[#list + 1] = { "Avalanche danger from " .. table.concat(centres, ", ") .. " via avalanche.org",
            "Ratings expire with each centre's forecast; read the full forecast at the centre." }
    end
    if Services.CLEARED.slf then
        list[#list + 1] = { "Avalanche bulletin: WSL Institute for Snow and Avalanche Research SLF", "CC BY 4.0, slf.ch." }
    end
    if Services.CLEARED.euregio then
        list[#list + 1] = { "Avalanche bulletin: Euregio Tyrol, South Tyrol, Trentino", "avalanche.report." }
    end
    list[#list + 1] = { "Fire data: NIFC / WFIGS", "Public domain, US National Interagency Fire Center." }
    list[#list + 1] = { "Snow: USDA NRCS SNOTEL", "Public domain, measured station data." }
    if npsKey() then list[#list + 1] = { "Park alerts: National Park Service", "Public domain, nps.gov." } end
    return list
end

return Services
