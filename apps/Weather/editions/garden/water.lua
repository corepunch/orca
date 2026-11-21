-- Water this week (GA2) and the soil at root depth (GA8). Plants lose water
-- to sun and wind; reference evapotranspiration (ET0, FAO-56) is what a
-- well-watered lawn would lose, a fair stand-in for a vegetable bed. The
-- balance counts the rain that fell and the water lost over the last
-- seven days (`pastDays = 7`), then looks at the rain the next days bring:
--
--   short = lost − rain, last 7 days
--   water        short ≥ the soil's limit (sandy 10, loam 15, clay 20 mm)
--   rain coming  …but a day in the next three brings enough rain
--   skip         the rain kept up
--
-- Model soil moisture at 3–9 and 9–27 cm, where roots are, can overrule
-- it: soil past field capacity needs nothing; soil near wilting point
-- needs water whatever the sums say. Both are estimates.
--
-- Two official answers the app cannot compute join the card:
--
--   US drought (SV-GA4)  the U.S. Drought Monitor's category at the point
--                        (NDMC, USDA, NOAA), from the current map's
--                        polygons; weekly, so asked at most once a day.
--                        D1 and worse add a line. A failed answer says
--                        nothing, never "no drought".
--   UK hosepipe bans     no feed exists anywhere, so no claim: in a dry
--   (SV-GA5)             summer week the card names where to look.
local Units = require "Weather/units"
local Score = require "Weather/score"
local Storage = require "Weather/storage"
local Plot = require "Weather/editions/garden/plot"
local Warnings = require "Weather/editions/garden/warnings"
local Water = {}

local DAY = 86400

-- The current USDM map as an ArcGIS FeatureServer (owner to be confirmed
-- with NDMC before shipping; see Research/services-garden.md).
Water.USDM = "https://services5.arcgis.com/0OTVzJS4K09zlixn/ArcGIS/rest/services/USDM_current/FeatureServer/0/query"
Water.DROUGHT = { [0] = "Abnormally dry (D0)", "Moderate drought (D1)", "Severe drought (D2)", "Extreme drought (D3)",
    "Exceptional drought (D4)" }
Water.HOSEPIPE = "water.org.uk"

local function droughtKey(spot) return ("%.2f,%.2f"):format(spot.lat, spot.lon) end
local function droughtSaved(spot)
    local all = Storage.load("garden-drought")
    all = type(all) == "table" and all or {}
    return all[droughtKey(spot)], all
end

-- The drought source (Garden.sources): US places, once a day. The answer
-- has no coordinates and the forecast no `spot` while its sources parse,
-- so the place asked is remembered from `url`; a replay is not kept.
local asking
Water.droughtSource = {
    id = "drought", optional = true,
    url = function(spot, _, now)
        asking = nil
        if not Warnings.us(spot) then return nil end
        local saved = droughtSaved(spot)
        if saved and saved.at and (now or os.time()) - saved.at < DAY then return nil end
        asking = { spot = spot, at = os.time() }
        return ("%s?geometry=%.4f,%.4f&geometryType=esriGeometryPoint&inSR=4326&spatialRel=esriSpatialRelIntersects"
            .. "&outFields=DM&returnGeometry=false&f=json"):format(Water.USDM, spot.lon, spot.lat)
    end,
    parse = function(json, forecast)
        assert(type(json) == "table" and not json.error, "drought map refused the query")
        local features = assert(type(json.features) == "table" and json.features, "no drought features")
        local dm = -1
        for _, feature in ipairs(features) do
            local value = tonumber((feature.attributes or {}).DM)
            if value and value > dm then dm = value end
        end
        -- Only straight after its own request (not a later replay).
        local spot = forecast.spot or asking and os.time() - asking.at < 60 and asking.spot or nil
        asking = nil
        local value = { at = forecast.updated or forecast.epoch or os.time(), dm = dm }
        forecast.drought = value
        if spot and spot.lat then
            local _, all = droughtSaved(spot)
            all[droughtKey(spot)] = value
            Storage.save("garden-drought", all)
        end
    end,
}

-- The drought category at the place (-1 none, 0–4), from this forecast or
-- a map read in the last eight days; nil when unknown.
function Water.drought(forecast, spot)
    spot = spot or forecast.spot
    if forecast.drought then return forecast.drought.dm end
    if not (spot and spot.lat and Warnings.us(spot)) then return nil end
    local saved = droughtSaved(spot)
    local now = forecast.epoch or os.time()
    if saved and saved.at and now - saved.at < 8 * DAY then return saved.dm end
end

-- Root-zone moisture: a weighted mean of the two layers (6 cm and 18 cm
-- thick), else the top centimetre. Returns the fraction and its depth text.
function Water.rootMoisture(hour)
    local upper, lower = hour.soil_moisture_3_to_9cm, hour.soil_moisture_9_to_27cm
    if upper and lower then return (upper * 6 + lower * 18) / 24, Plot.depth(3, 27) end
    if upper then return upper, Plot.depth(3, 9) end
    if hour.soilMoisture then return hour.soilMoisture, "the top " .. Plot.depth(1) end
    return nil
end

-- How full the root zone is: 0 at wilting point, 1 at field capacity,
-- above 1 waterlogged. nil when unknown.
function Water.fullness(hour, plot)
    local moisture = Water.rootMoisture(hour)
    if not moisture then return nil end
    local soil = (plot or Plot.current()).soilInfo
    return (moisture - soil.wilt) / (soil.full - soil.wilt)
end

-- The water sums: last seven days, the next seven, and the first day in
-- the next three with real rain (chance 50 % or more, 5 mm or more).
function Water.balance(forecast, plot)
    plot = plot or Plot.current()
    local now = Plot.nowEpoch(forecast)
    if not now then return nil end
    local past = Plot.between(forecast, now - 7 * DAY, now)
    local nextWeek = Plot.between(forecast, now, now + 7 * DAY)
    local b = {
        pastHours = #past, rainPast = Plot.sum(past, "rain"), lostPast = Plot.sum(past, "et0"),
        rainNext = Plot.sum(nextWeek, "rain"), lostNext = Plot.sum(nextWeek, "et0"),
        limit = plot.soilInfo.dry,
    }
    b.short = b.lostPast - b.rainPast
    -- Rain coming: per local date in the next three days, rain likely to fall.
    for _, day in ipairs(Plot.byDate(Plot.between(forecast, now, now + 3 * DAY))) do
        local likely = 0
        for _, hour in ipairs(day.hours) do
            if (hour.rainChance or 100) >= 50 then likely = likely + (hour.rain or 0) end
        end
        if likely >= 5 then b.rainDay, b.rainComing = day.date, likely; break end
    end
    local hour = forecast.hours[forecast.now]
    b.fullness = hour and Water.fullness(hour, plot)
    b.depth = hour and select(2, Water.rootMoisture(hour))
    -- No week of history yet: judge on the week ahead instead.
    if b.pastHours < 48 then b.short = b.lostNext - b.rainNext end
    local dry = b.short >= b.limit or (b.fullness ~= nil and b.fullness < 0.3)
    if b.fullness and b.fullness >= 0.9 then dry = false end
    if not dry then b.verdict = "skip"
    elseif b.rainComing and b.rainComing >= math.min(b.short * 0.5, 10) then b.verdict = "rain coming"
    else b.verdict = "water" end
    -- One deep soak, not a daily sprinkle: the shortfall, between 10 and 25.
    b.amount = math.max(10, math.min(25, math.floor((math.max(b.short, 0) + 2.5) / 5) * 5))
    return b
end

-- The "water this week" card.
function Water.card(forecast, plot)
    local b = Water.balance(forecast, plot)
    if not b then return nil end
    local history = b.pastHours >= 48
    local sums = history and ("last 7 days: rain %s, plants lost %s"):format(Units.rain(b.rainPast), Units.rain(b.lostPast))
        or ("next 7 days: rain %s, plants lose %s"):format(Units.rain(b.rainNext), Units.rain(b.lostNext))
    local card = { id = "water", title = "water this week" }
    local parts = { sums }
    if b.verdict == "skip" then
        card.value, card.tone = "no need", "good"
        if b.fullness and b.fullness >= 0.9 then parts[#parts + 1] = "the soil is moist at root depth" end
    elseif b.verdict == "rain coming" then
        card.value, card.tone = "rain coming", "good"
        parts[#parts + 1] = ("wait: %s brings about %s"):format(Plot.dayName(b.rainDay, forecast), Units.rain(b.rainComing))
    else
        card.value, card.tone = "water " .. Plot.water(b.amount), "fair"
        if b.fullness and b.fullness < 0.3 then parts[#parts + 1] = "dry at root depth (" .. b.depth .. ")" end
        parts[#parts + 1] = "one deep soak in the evening or early morning, not a daily sprinkle"
    end
    if not history then parts[#parts + 1] = "no rain record for last week yet" end
    local spot = forecast.spot
    local dm = Water.drought(forecast, spot)
    if dm and dm >= 1 then
        parts[#parts + 1] = ("%s here (U.S. Drought Monitor): check your utility's watering days"):format(
            Water.DROUGHT[math.min(4, dm)])
    end
    local month = Plot.month(forecast)
    if b.verdict == "water" and history and b.short >= b.limit and month and month >= 6 and month <= 9
        and Warnings.uk(spot) then
        parts[#parts + 1] = "is there a hosepipe ban? ask your water company (Water UK: " .. Water.HOSEPIPE .. ")"
    end
    card.detail = table.concat(parts, " · ")
    return card, b
end

-- The root zone as a metric row.
function Water.metric(hour, plot)
    local f = Water.fullness(hour, plot)
    local moisture, depth = Water.rootMoisture(hour)
    if not f then return { label = "soil moisture", value = "—" } end
    local word = f >= 1.1 and "waterlogged" or f >= 0.9 and "moist" or f >= 0.5 and "fine" or f >= 0.3 and "drying"
        or "dry"
    return { label = "soil at the roots", value = word,
        tone = (f >= 1.1 or f < 0.3) and "poor" or f < 0.5 and "fair" or "good",
        note = ("%s water at %s (model estimate)"):format(Score.fmt(moisture * 100, "%"), depth) }
end

return Water
