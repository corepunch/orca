-- Tides for coastal places (FI5). Two sources, both optional:
--
--   * NOAA CO-OPS high and low predictions, free, US only. A place names its
--     station: `tideStation = { noaa = "8723970", name = "Vaca Key", lat =
--     24.71, lon = -81.11 }` (places.lua). The station's own predictions are
--     the best free tide there is.
--   * Open-Meteo marine `sea_level_height_msl` everywhere else: a model on a
--     grid of about 8 km, tides included, which Open-Meteo says is not for
--     coastal navigation. High and low come from its turning points and are
--     always labelled "approximate".
--
-- Times are local to the place (NOAA `lst_ldt`, Open-Meteo `timezone=auto`).
local Sky = require "Weather/sky"
local Tides = {}

local SLACK_MINUTES = 40

local function coastal(spot) return spot and not spot.lake and not spot.river end

-- Local "YYYY-MM-DDTHH:MM" as minutes on one running scale.
local function minutesOf(time)
    local start, minutes = Sky.epochOf(time and time:sub(1, 10)), Sky.minutes(time)
    return start and minutes and start // 60 + minutes
end
Tides.minutesOf = minutesOf

local function clock(minutes)
    return os.date("!%Y-%m-%dT%H:%M", minutes * 60)
end

-- The NOAA request for a place's station: four days of highs and lows from
-- yesterday on, in the station's local time.
function Tides.noaaUrl(spot, now)
    local station = spot and spot.tideStation
    if not (coastal(spot) and station and station.noaa) then return nil end
    return require("Weather/forecast").url("noaaTides", ("product=predictions&application=weather_app&begin_date=%s"
        .. "&range=96&datum=MLLW&station=%s&time_zone=lst_ldt&units=metric&interval=hilo&format=json")
        :format(os.date("!%Y%m%d", (now or os.time()) - 86400), station.noaa))
end

function Tides.seaLevelUrl(spot, Forecast)
    if not coastal(spot) then return nil end
    return Forecast.url("marine", ("latitude=%s&longitude=%s&hourly=sea_level_height_msl&timezone=auto"
        .. "&past_days=1&forecast_days=7"):format(spot.lat, spot.lon))
end

-- NOAA's answer → { kind = "noaa", turns = { { time, height, high } } }.
function Tides.parseNoaa(json)
    local turns = {}
    for _, p in ipairs(json.predictions or {}) do
        local time = type(p.t) == "string" and p.t:gsub(" ", "T")
        if time and (p.type == "H" or p.type == "L") then
            turns[#turns + 1] = { time = time, height = tonumber(p.v), high = p.type == "H" }
        end
    end
    if #turns < 2 then error(json.error and json.error.message or "no tide predictions") end
    return { kind = "noaa", turns = turns }
end

-- Open-Meteo's hourly sea level → the same turns, each refined between the
-- hours with a parabola through its neighbours; also the curve and the
-- grid cell the model answered for.
function Tides.parseSeaLevel(json)
    local h = json.hourly or {}
    local times, levels = h.time or {}, h.sea_level_height_msl or {}
    local turns, curve = {}, {}
    for index, time in ipairs(times) do curve[time] = levels[index] end
    for index = 2, #times - 1 do
        local a, b, c = levels[index - 1], levels[index], levels[index + 1]
        if a and b and c and ((b > a and b >= c) or (b < a and b <= c)) then
            local denominator = a - 2 * b + c
            local shift = denominator ~= 0 and 0.5 * (a - c) / denominator or 0
            local at = minutesOf(times[index]) + math.floor(shift * 60 + 0.5)
            turns[#turns + 1] = { time = clock(at), height = b - 0.25 * (a - c) * shift, high = b > a }
        end
    end
    if #turns < 2 then error("no tide in the sea level") end
    return { kind = "model", turns = turns, curve = curve, lat = json.latitude, lon = json.longitude }
end

-- Measured water temperature at the place's NOAA station (SV-FI2): the
-- recent 6-minute readings in UTC and °C. A station without the sensor
-- answers `error.message`, and the water falls back to the model.
function Tides.waterUrl(spot)
    local station = spot and spot.tideStation
    if not (station and station.noaa) then return nil end
    return require("Weather/forecast").url("noaaTides", ("product=water_temperature&application=weather_app&date=recent"
        .. "&station=%s&time_zone=gmt&units=metric&format=json"):format(station.noaa))
end

-- { data = { { t = "2026-11-18 05:48", v = "18.4", f = "0,0,0" } } } →
-- { temp, epoch, name, lat, lon } of the newest reading.
function Tides.parseWater(json)
    local newest
    for _, point in ipairs(json.data or {}) do
        local temp = tonumber(point.v)
        local d, h, m = (type(point.t) == "string" and point.t or ""):match("^(%d+%-%d+%-%d+) (%d+):(%d+)")
        local start = d and Sky.epochOf(d)
        local epoch = start and start + tonumber(h) * 3600 + tonumber(m) * 60
        if temp and epoch and (not newest or epoch > newest.epoch) then newest = { temp = temp, epoch = epoch } end
    end
    if not newest then error(json.error and json.error.message or "no water temperature") end
    local meta = json.metadata or {}
    newest.name, newest.lat, newest.lon = meta.name, tonumber(meta.lat), tonumber(meta.lon)
    return newest
end

-- The sources as forecast `sources`; the forecast keeps each answer.
function Tides.sources()
    return {
        { id = "water-noaa", optional = true, url = Tides.waterUrl,
            parse = function(json, forecast) forecast.waterNoaa = Tides.parseWater(json) end },
        { id = "tides-noaa", optional = true, url = function(spot) return Tides.noaaUrl(spot) end,
            parse = function(json, forecast) forecast.tidesNoaa = Tides.parseNoaa(json) end },
        { id = "tides-model", optional = true, url = Tides.seaLevelUrl,
            parse = function(json, forecast) forecast.tidesModel = Tides.parseSeaLevel(json) end },
    }
end

local function distanceKm(lat1, lon1, lat2, lon2)
    if not (lat1 and lon1 and lat2 and lon2) then return nil end
    local r = math.pi / 180
    local a = math.sin((lat2 - lat1) * r / 2) ^ 2
        + math.cos(lat1 * r) * math.cos(lat2 * r) * math.sin((lon2 - lon1) * r / 2) ^ 2
    return 6371 * 2 * math.atan(math.sqrt(a), math.sqrt(1 - a))
end
Tides.distanceKm = distanceKm

-- The tide a forecast has: NOAA's when its station answered, else the
-- model's; with `source` (what to tell the user) and `distance` (km from
-- the place to the station or the model's cell).
function Tides.of(forecast)
    local spot = forecast.spot or {}
    if not coastal(spot) then return nil end
    local station = spot.tideStation
    local tide = forecast.tidesNoaa or forecast.tidesModel
    if not tide then return nil end
    if tide.kind == "noaa" then
        tide.source = "NOAA station " .. (station and (station.name or station.noaa) or "")
        tide.distance = station and distanceKm(spot.lat, spot.lon, station.lat, station.lon)
        tide.approximate = false
    else
        tide.source = "Open-Meteo sea level model"
        tide.distance = distanceKm(spot.lat, spot.lon, tide.lat, tide.lon)
        tide.approximate = true
    end
    return tide
end

-- The stage at a local time: { stage = "rising" | "falling" | "slack",
-- running = true in the middle third of the flow (the rule of twelfths: it
-- moves most then), last and next turn, minutes to the nearest }.
function Tides.stage(tide, time)
    local now = minutesOf(time)
    if not (tide and now) then return nil end
    local last, next
    for _, turn in ipairs(tide.turns) do
        local at = minutesOf(turn.time)
        if at <= now then last = turn else next = next or turn end
    end
    if not (last and next) then return nil end
    local from, to = minutesOf(last.time), minutesOf(next.time)
    local nearest = math.min(now - from, to - now)
    local share = (now - from) / math.max(1, to - from)
    return { stage = nearest <= SLACK_MINUTES and "slack" or next.high and "rising" or "falling",
        running = share >= 1 / 3 and share <= 2 / 3, last = last, next = next, nearest = nearest }
end

-- The next `count` highs and lows from a local time on.
function Tides.upcoming(tide, time, count)
    local now, list = minutesOf(time), {}
    for _, turn in ipairs(tide and tide.turns or {}) do
        if now and minutesOf(turn.time) >= now and #list < count then list[#list + 1] = turn end
    end
    return list
end

return Tides
