-- Measured water near a fishing place (Research/services-fishing.md,
-- SV-FI1, SV-FI3, SV-FI4). Public gauge networks answer with the river's
-- real level, flow and, at some stations, its temperature:
--
--   * USGS Water Data OGC API (US, public domain; recent values provisional);
--   * Environment Agency flood monitoring (England, OGL v3, a beta);
--   * PEGELONLINE (German federal waterways, DL-DE Zero 2.0);
--   * OPW waterlevel.ie (Ireland, CC BY 4.0; latest level only).
--
-- A place names its gauge: `gauge = { usgs = "01646500", name = "Potomac at
-- Little Falls" }`, `{ ea = "1491TH" }`, `{ pegel = "<uuid or shortname>" }`
-- or `{ opw = "0000025017" }`, optionally with `lat`/`lon`. A US river place
-- without one takes the nearest USGS gauge within NEAR_KM. NOAA GLERL adds
-- the daily lake-average surface temperature of the Great Lakes.
--
-- Every adapter turns its answer into the same reading { id, name, lat,
-- lon, level (m), flow (m³/s), temp (°C), epoch, state, provisional } and,
-- when asked, a week's `series`. `Gauges.of(forecast)` picks the place's
-- reading and adds its age, trend and state. A reading older than STALE is
-- "silent": shown as such, never scored, never read as calm water.
local Sky = require "Weather/sky"
local Tides = require "Weather/editions/fishing/tides"
local Gauges = {}

-- Base URLs, here so tests and a later proxy can swap them. `usgsKey` is an
-- optional api.data.gov key for USGS's higher rate limit.
Gauges.endpoints = {
    usgs = "https://api.waterdata.usgs.gov/ogcapi/v0/collections",
    ea = "https://environment.data.gov.uk/flood-monitoring/id/stations",
    pegel = "https://www.pegelonline.wsv.de/webservices/rest-api/v2/stations",
    opw = "https://waterlevel.ie/geojson/latest/",
    glerl = "https://apps.glerl.noaa.gov/erddap/tabledap/glsea_avgtemps_3.json",
    usgsKey = nil,
}
Gauges.STALE = 6 * 3600
Gauges.LAKE_STALE = 3 * 86400 -- GLSEA is a daily analysis
Gauges.NEAR_KM = 10
Gauges.NETWORKS = { usgs = "USGS", ea = "Environment Agency", pegel = "PEGELONLINE", opw = "OPW waterlevel.ie" }

local FT3, FT = 0.0283168, 0.3048
local WEEK = 7 * 86400

-- An ISO time ("2026-11-18T05:15:00.000-05:00", "…Z", "2026-11-18 05:48")
-- as a UTC epoch; no offset means UTC.
function Gauges.epoch(iso)
    if type(iso) ~= "string" then return nil end
    local date, h, m, rest = iso:match("^(%d%d%d%d%-%d%d%-%d%d)[T ](%d%d):(%d%d)(.*)$")
    local start = date and Sky.epochOf(date)
    if not start then return nil end
    local epoch = start + tonumber(h) * 3600 + tonumber(m) * 60 + (tonumber(rest:match("^:(%d%d)")) or 0)
    local sign, oh, om = rest:match("([+-])(%d%d):?(%d%d)$")
    if sign then epoch = epoch - (sign == "+" and 1 or -1) * (tonumber(oh) * 3600 + tonumber(om) * 60) end
    return epoch
end
local epochOf = Gauges.epoch

local function isoUtc(epoch) return os.date("!%Y-%m-%dT%H:%M:%SZ", epoch) end

-- The place's configured gauge: { network, id, name, lat, lon }, or nil.
function Gauges.station(spot)
    local gauge = spot and spot.gauge
    if type(gauge) ~= "table" then return nil end
    for network in pairs(Gauges.NETWORKS) do
        if gauge[network] then
            return { network = network, id = tostring(gauge[network]), name = gauge.name, lat = gauge.lat, lon = gauge.lon }
        end
    end
end

local function inUS(spot) return spot and spot.country == "US" end
local function wants(spot, network)
    local station = Gauges.station(spot)
    return station and station.network == network and station or nil
end

-- USGS -----------------------------------------------------------------

-- The latest discharge (00060), gage height (00065) and water temperature
-- (00010): of the place's gauge, or of every gauge in a box of about
-- NEAR_KM round a US river place.
function Gauges.usgsUrl(spot)
    local base = Gauges.endpoints.usgs .. "/latest-continuous/items?f=json&parameter_code=00060,00065,00010"
    local station = wants(spot, "usgs")
    if station then return base .. "&monitoring_location_id=USGS-" .. station.id end
    if Gauges.station(spot) or not (spot and spot.river and inUS(spot) and spot.lat and spot.lon) then return nil end
    local dLat = Gauges.NEAR_KM / 111
    local dLon = dLat / math.max(0.2, math.cos(math.rad(spot.lat)))
    return base .. ("&bbox=%.3f,%.3f,%.3f,%.3f"):format(spot.lon - dLon, spot.lat - dLat, spot.lon + dLon, spot.lat + dLat)
end

-- The last seven days of discharge and gage height, for the trend and the
-- week's usual (a configured gauge only: a box would bring every gauge's week).
function Gauges.usgsWeekUrl(spot)
    local station = wants(spot, "usgs")
    if not station then return nil end
    return Gauges.endpoints.usgs .. "/continuous/items?f=json&parameter_code=00060,00065&time=P7D&limit=2000"
        .. "&monitoring_location_id=USGS-" .. station.id
end

local function usgsHeaders()
    local key = Gauges.endpoints.usgsKey
    return key and { ["X-Api-Key"] = key } or nil
end

-- A USGS value in metric: ft³/s → m³/s, ft → m; °C stays.
local function usgsValue(p)
    local value = tonumber(p.value)
    if not value then return nil end
    local unit = tostring(p.unit_of_measure or "")
    if p.parameter_code == "00060" then return unit:find("^m") and value or value * FT3, "flow" end
    if p.parameter_code == "00065" then return unit:find("^m") and value or value * FT, "level" end
    if p.parameter_code == "00010" then return unit:find("F") and (value - 32) * 5 / 9 or value, "temp" end
end

local function usgsId(id) return tostring(id or ""):gsub("^USGS%-", "") end

-- GeoJSON features (one per gauge and parameter) → { stations = { … } }.
function Gauges.parseUsgs(json)
    local byId, stations = {}, {}
    for _, feature in ipairs(json.features or {}) do
        local p = feature.properties or {}
        local value, kind = usgsValue(p)
        local id = usgsId(p.monitoring_location_id)
        if value and id ~= "" then
            local station = byId[id]
            if not station then
                local xy = feature.geometry and feature.geometry.coordinates or {}
                station = { id = id, lon = tonumber(xy[1]), lat = tonumber(xy[2]), provisional = false }
                byId[id], stations[#stations + 1] = station, station
            end
            station[kind] = value
            station.epoch = math.max(station.epoch or 0, epochOf(p.time) or 0)
            station.provisional = station.provisional or p.approval_status ~= "Approved"
        end
    end
    if #stations == 0 then error((json.error and json.error.message) or "no gauge readings") end
    return { network = "usgs", stations = stations }
end

-- The week: { flow = { { epoch, value } }, level = … }, oldest first.
local function sortSeries(series)
    for _, list in pairs(series) do table.sort(list, function(a, b) return a[1] < b[1] end) end
    return series
end

function Gauges.parseUsgsWeek(json)
    local series = { flow = {}, level = {} }
    for _, feature in ipairs(json.features or {}) do
        local p = feature.properties or {}
        local value, kind = usgsValue(p)
        local epoch = epochOf(p.time)
        if value and epoch and series[kind] then series[kind][#series[kind] + 1] = { epoch, value } end
    end
    if #series.flow + #series.level < 3 then error("no gauge week") end
    return sortSeries(series)
end

-- Environment Agency ---------------------------------------------------

-- A week of readings, newest first, of every measure at the station.
function Gauges.eaUrl(spot, now)
    local station = wants(spot, "ea")
    if not station then return nil end
    return ("%s/%s/readings?_sorted&_limit=2000&since=%s"):format(Gauges.endpoints.ea, station.id,
        isoUtc((now or os.time()) - WEEK))
end

-- items = { { dateTime, value, measure = ".../1491TH-level-stage-i-15_min-mASD" } }.
function Gauges.parseEa(json)
    local series, station = { flow = {}, level = {} }, { provisional = false }
    for _, item in ipairs(json.items or {}) do
        local measure, value, epoch = tostring(item.measure or ""), tonumber(item.value), epochOf(item.dateTime)
        local kind = measure:find("%-flow%-") and "flow" or measure:find("%-level%-") and "level"
        if kind and value and epoch then
            series[kind][#series[kind] + 1] = { epoch, value }
            if not station.epoch or epoch >= station.epoch then station.epoch = epoch end
            station.id = station.id or measure:match("/measures/([^%-]+)")
        end
    end
    if #series.flow + #series.level == 0 then error("no gauge readings") end
    sortSeries(series)
    for kind, list in pairs(series) do
        local last = list[#list]
        if last and last[1] == station.epoch then station[kind] = last[2] end
    end
    return { network = "ea", stations = { station } }, series
end

-- PEGELONLINE ----------------------------------------------------------

local function pegelPath(spot)
    local station = wants(spot, "pegel")
    return station and Gauges.endpoints.pegel .. "/" .. station.id
end
function Gauges.pegelUrl(spot)
    local path = pegelPath(spot)
    return path and path .. ".json?includeTimeseries=true&includeCurrentMeasurement=true"
end
function Gauges.pegelWeekUrl(spot)
    local path = pegelPath(spot)
    return path and path .. "/W/measurements.json?start=P7D"
end

local PEGEL_STATE = { low = "low", normal = "normal", high = "high" }

-- The station with its time series: W (cm), Q (m³/s), WT (°C).
function Gauges.parsePegel(json)
    local station = { id = json.uuid, name = json.longname, lat = tonumber(json.latitude), lon = tonumber(json.longitude),
        provisional = true }
    if json.water and json.water.longname and station.name then
        station.name = station.name .. ", " .. json.water.longname
    end
    for _, series in ipairs(json.timeseries or {}) do
        local current = series.currentMeasurement or {}
        local value, epoch = tonumber(current.value), epochOf(current.timestamp)
        local kind = ({ W = "level", Q = "flow", WT = "temp" })[series.shortname]
        if kind and value and epoch then
            station[kind] = kind == "level" and value / 100 or value
            station.epoch = math.max(station.epoch or 0, epoch)
            if kind == "level" then station.state = PEGEL_STATE[current.stateMnwMhw] end
        end
    end
    if not station.epoch then error("no gauge readings") end
    return { network = "pegel", stations = { station } }
end

-- W measurements: a list of { timestamp, value (cm) }.
function Gauges.parsePegelWeek(json)
    local level = {}
    for _, m in ipairs(json) do
        local epoch, value = epochOf(m.timestamp), tonumber(m.value)
        if epoch and value then level[#level + 1] = { epoch, value / 100 } end
    end
    if #level < 3 then error("no gauge week") end
    return sortSeries { flow = {}, level = level }
end

-- OPW ------------------------------------------------------------------

function Gauges.opwUrl(spot) return wants(spot, "opw") and Gauges.endpoints.opw end

-- The latest of every OPW sensor as GeoJSON; `station_ref` and
-- `sensor_ref` "0001" (level, m) and "0002" (water temperature, °C).
function Gauges.parseOpw(json)
    local byId, stations = {}, {}
    for _, feature in ipairs(json.features or {}) do
        local p = feature.properties or {}
        local kind = ({ ["0001"] = "level", ["0002"] = "temp" })[tostring(p.sensor_ref)]
        local value, epoch = tonumber(p.value), epochOf(p.datetime)
        if kind and value and epoch and p.station_ref then
            local id = tostring(p.station_ref)
            local station = byId[id]
            if not station then
                local xy = feature.geometry and feature.geometry.coordinates or {}
                station = { id = id, name = p.station_name, lon = tonumber(xy[1]), lat = tonumber(xy[2]), provisional = true }
                byId[id], stations[#stations + 1] = station, station
            end
            station[kind] = value
            station.epoch = math.max(station.epoch or 0, epoch)
        end
    end
    if #stations == 0 then error("no gauge readings") end
    return { network = "opw", stations = stations }
end

-- GLERL GLSEA ----------------------------------------------------------

Gauges.GREAT_LAKES = { superior = "Superior", michigan = "Michigan", huron = "Huron", erie = "Erie",
    ontario = "Ontario", stclair = "St. Clair" }

-- The Great Lake a place is on: its `greatLake`, else a lake place inside
-- the Great Lakes box whose name says which.
function Gauges.greatLake(spot)
    if not spot then return nil end
    if spot.greatLake then return Gauges.GREAT_LAKES[spot.greatLake] and spot.greatLake end
    if not (spot.lake and spot.lat and spot.lon and spot.lat > 41 and spot.lat < 49.5 and spot.lon > -93
        and spot.lon < -75.5) then return nil end
    local name = (spot.title or ""):lower():gsub("saint", "st."):gsub("st%.%s*clair", "stclair")
    for key in pairs(Gauges.GREAT_LAKES) do
        if name:find("%f[%a]" .. key .. "%f[%A]") then return key end
    end
end

-- Ten days of every lake's daily average (ERDDAP tabledap, all columns).
function Gauges.glerlUrl(spot)
    return Gauges.greatLake(spot) and Gauges.endpoints.glerl .. "?&time%3E=now-10days" or nil
end

-- table = { columnNames = { "time", …lakes }, rows = { { time, … } } } →
-- { lakes = { erie = { { epoch, temp } } } }, each column found by name.
function Gauges.parseGlerl(json)
    local t = json.table or {}
    local names, columns = t.columnNames or {}, {}
    local timeColumn
    for index, name in ipairs(names) do
        local key = tostring(name):lower():gsub("[%s%._]", ""):gsub("^saint", "st")
        if key == "time" then timeColumn = index end
        for lake in pairs(Gauges.GREAT_LAKES) do if key == lake then columns[lake] = index end end
    end
    local lakes, n = {}, 0
    for _, row in ipairs(timeColumn and t.rows or {}) do
        local epoch = epochOf(row[timeColumn])
        for lake, index in pairs(columns) do
            local temp = tonumber(row[index])
            if epoch and temp then
                lakes[lake] = lakes[lake] or {}
                table.insert(lakes[lake], { epoch, temp })
                n = n + 1
            end
        end
    end
    if n == 0 then error("no lake temperatures") end
    return { lakes = sortSeries(lakes) }
end

-- The sources --------------------------------------------------------------

function Gauges.sources()
    local function latest(parse)
        return function(json, forecast)
            local latestOf, series = parse(json)
            forecast.gaugeLatest = latestOf
            if series then forecast.gaugeSeries = series end
        end
    end
    local function week(parse) return function(json, forecast) forecast.gaugeSeries = parse(json) end end
    return {
        { id = "gauge-usgs", optional = true, url = Gauges.usgsUrl, headers = usgsHeaders, parse = latest(Gauges.parseUsgs) },
        { id = "gauge-usgs-week", optional = true, url = Gauges.usgsWeekUrl, headers = usgsHeaders,
            parse = week(Gauges.parseUsgsWeek) },
        { id = "gauge-ea", optional = true, url = function(spot) return Gauges.eaUrl(spot) end, parse = latest(Gauges.parseEa) },
        { id = "gauge-pegel", optional = true, url = Gauges.pegelUrl, parse = latest(Gauges.parsePegel) },
        { id = "gauge-pegel-week", optional = true, url = Gauges.pegelWeekUrl, parse = week(Gauges.parsePegelWeek) },
        { id = "gauge-opw", optional = true, url = Gauges.opwUrl, parse = latest(Gauges.parseOpw) },
        { id = "lake-glerl", optional = true, url = Gauges.glerlUrl,
            parse = function(json, forecast) forecast.glerl = Gauges.parseGlerl(json) end },
    }
end

-- Reading the answers --------------------------------------------------------

local function median(list, field)
    local values = {}
    for _, point in ipairs(list) do values[#values + 1] = point[field or 2] end
    if #values == 0 then return nil end
    table.sort(values)
    return values[(#values + 1) // 2]
end

-- The newest point at least `seconds` before `epoch`.
local function before(list, epoch, seconds)
    local found
    for _, point in ipairs(list or {}) do
        if point[1] <= epoch - seconds then found = point end
    end
    return found
end

-- Did the place's gauge fail to answer? (the source's id in forecast.missing)
function Gauges.failed(forecast)
    for id in pairs(forecast.missing or {}) do
        if id:find("^gauge%-") and not id:find("%-week$") then return true end
    end
    return false
end

local memo = setmetatable({}, { __mode = "k" })

-- The place's gauge: the reading plus { network, source, distance, age,
-- fresh, time (local ISO), trend = "rising" | "falling" | "steady" | nil,
-- usual, ratio, state = "high" | "low" | "normal" | nil }, or nil.
function Gauges.of(forecast)
    local latest = forecast and forecast.gaugeLatest
    if not latest then return nil end
    local cached = memo[forecast]
    if cached and cached.latest == latest and cached.series == forecast.gaugeSeries and cached.epoch == forecast.epoch then
        return cached.gauge
    end
    local spot, wanted = forecast.spot or {}, Gauges.station(forecast.spot) or {}
    local pick, best
    for _, station in ipairs(latest.stations or {}) do
        if wanted.id and (station.id == wanted.id or #latest.stations == 1) then pick = station; break end
        local km = Tides.distanceKm(spot.lat, spot.lon, station.lat, station.lon)
        if not wanted.id and km and km <= Gauges.NEAR_KM and (not best or km < best) then pick, best = station, km end
    end
    local gauge
    if pick then
        gauge = {}
        for key, value in pairs(pick) do gauge[key] = value end
        gauge.network = latest.network
        gauge.name = wanted.name or pick.name or (Gauges.NETWORKS[latest.network] .. " gauge " .. tostring(pick.id or ""))
        gauge.distance = Tides.distanceKm(spot.lat, spot.lon, pick.lat or wanted.lat, pick.lon or wanted.lon)
        gauge.age = math.max(0, (forecast.epoch or os.time()) - (pick.epoch or 0))
        gauge.fresh = gauge.age <= Gauges.STALE
        gauge.time = os.date("!%Y-%m-%dT%H:%M", (pick.epoch or 0) + (forecast.offset or 0))
        gauge.source = Gauges.NETWORKS[latest.network] .. (pick.provisional and latest.network == "usgs" and ", provisional" or "")
        local series = forecast.gaugeSeries or {}
        local kind = gauge.flow and series.flow and #series.flow > 0 and "flow" or gauge.level and "level"
        local list, now = kind and series[kind], kind and gauge[kind]
        if list and #list > 0 and now then
            local prior = before(list, pick.epoch, 86400) or before(list, pick.epoch, 6 * 3600)
            if prior then
                local change = now - prior[2]
                if kind == "flow" then
                    local ratio = prior[2] > 0 and now / prior[2] or 1
                    gauge.trend = ratio >= 1.15 and "rising" or ratio <= 0.87 and "falling" or "steady"
                else
                    gauge.trend = change >= 0.05 and "rising" or change <= -0.05 and "falling" or "steady"
                end
            end
            if kind == "flow" then
                gauge.usual = median(list)
                gauge.ratio = gauge.usual and gauge.usual > 0 and now / gauge.usual or nil
            end
        end
        gauge.state = gauge.state or gauge.ratio and (gauge.ratio >= 2 and "high" or gauge.ratio <= 0.5 and "low" or "normal")
    end
    memo[forecast] = { latest = latest, series = forecast.gaugeSeries, epoch = forecast.epoch, gauge = gauge }
    return gauge
end

-- "40 min ago", "3 h ago", "2 days ago".
function Gauges.ago(seconds)
    if seconds < 90 * 60 then return ("%d min ago"):format(math.max(1, seconds // 60)) end
    if seconds < 48 * 3600 then return ("%d h ago"):format(seconds // 3600) end
    return ("%d days ago"):format(seconds // 86400)
end

-- The Great Lake's daily average: { temp, epoch, lake, week (change over
-- the last week, °C) }, or nil when old or absent.
function Gauges.lake(forecast)
    local key = forecast.glerl and Gauges.greatLake(forecast.spot)
    local list = key and forecast.glerl.lakes[key]
    local last = list and list[#list]
    if not last then return nil end
    local age = (forecast.epoch or os.time()) - last[1]
    if age > Gauges.LAKE_STALE then return nil end
    local weekAgo = before(list, last[1], 6 * 86400)
    return { temp = last[2], epoch = last[1], lake = Gauges.GREAT_LAKES[key], age = age,
        week = weekAgo and last[2] - weekAgo[2] or nil }
end

return Gauges
