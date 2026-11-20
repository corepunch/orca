-- Model agreement (SA4): the same spot's wind from several weather models in
-- one extra request (Open-Meteo `models=`, columns suffixed with the model
-- id: `wind_speed_10m_gfs_seamless`). Per hour the spread between them
-- gives a confidence word: "sure", "likely" or "uncertain".
--
-- The list asks for provider families ("_seamless"), which Open-Meteo keeps
-- pointing at each provider's current models, so a retired model (HRRR and
-- NAM in November 2026, the old ICON grids) drops out without a code
-- change. The parser reads whatever models come back, so the list may
-- change freely. If Open-Meteo rejects a name the source fails quietly
-- (`forecast.missing.models`) and the rating goes on without it.
local Models = {}

Models.GLOBAL = { "ecmwf_ifs025", "gfs_seamless", "icon_seamless", "ukmo_seamless" }
-- A regional family where it is the local high-resolution model.
Models.REGIONAL = {
    { "meteofrance_seamless", south = 35, north = 56, west = -12, east = 20 },
    { "gem_seamless", south = 25, north = 75, west = -150, east = -50 },
}

-- Short names for the strip; an unknown id shows its first word.
local NAMES = { ecmwf = "ECMWF", gfs = "GFS", icon = "ICON", ukmo = "UK", meteofrance = "AROME", gem = "GEM", jma = "JMA",
    knmi = "KNMI", dmi = "DMI", metno = "MET" }
function Models.name(id)
    local first = tostring(id):match("^[^_]+") or tostring(id)
    return NAMES[first] or first:upper()
end

function Models.list(spot)
    local list = {}
    for _, id in ipairs(Models.GLOBAL) do list[#list + 1] = id end
    for _, region in ipairs(Models.REGIONAL) do
        if spot.lat >= region.south and spot.lat <= region.north and spot.lon >= region.west and spot.lon <= region.east then
            list[#list + 1] = region[1]
        end
    end
    return list
end

local COLUMNS = { wind = "wind_speed_10m", gust = "wind_gusts_10m", dir = "wind_direction_10m" }

local function number(v) return type(v) == "number" and v or nil end

-- Mean wind direction (degrees) of a list, by vectors; nil when calm.
local function meanDirection(list)
    local x, y = 0, 0
    for _, m in ipairs(list) do
        if m.dir then
            x, y = x + math.cos(math.rad(m.dir)), y + math.sin(math.rad(m.dir))
        end
    end
    if x == 0 and y == 0 then return nil end
    return math.deg(math.atan(y, x)) % 360
end

local function angle(a, b)
    local d = math.abs((a - b) % 360)
    return d > 180 and 360 - d or d
end

-- The agreement of models at one hour: { word, low, high, spread, models }.
-- "sure" when they are within about 5 kn (or a fifth of the wind) and point
-- the same way; "likely" within about 10 kn (or a third); else
-- "uncertain". Direction only counts in a usable wind.
function Models.agreement(list)
    if #list < 2 then return nil end
    local low, high, sum = math.huge, -math.huge, 0
    for _, m in ipairs(list) do
        low, high, sum = math.min(low, m.wind), math.max(high, m.wind), sum + m.wind
    end
    local mean, spread = sum / #list, high - low
    local turn = 0
    local centre = mean >= 4 and meanDirection(list)
    if centre then
        for _, m in ipairs(list) do
            if m.dir then turn = math.max(turn, angle(m.dir, centre)) end
        end
    end
    local word = "uncertain"
    if spread <= math.max(2.6, 0.2 * mean) and turn <= 30 then word = "sure"
    elseif spread <= math.max(5.1, 0.35 * mean) and turn <= 60 then word = "likely" end
    return { word = word, low = low, high = high, spread = spread, turn = turn, models = list }
end

-- The source for Activities.request: one call, the models' wind only.
Models.source = {
    id = "models", optional = true,
    url = function(spot, Forecast)
        if not (spot and spot.lat and spot.lon) then return nil end
        return Forecast.url("forecast", ("latitude=%s&longitude=%s&hourly=%s,%s,%s&models=%s"
            .. "&wind_speed_unit=ms&timezone=auto&forecast_days=7")
            :format(spot.lat, spot.lon, COLUMNS.wind, COLUMNS.gust, COLUMNS.dir, table.concat(Models.list(spot), ",")))
    end,
    -- Puts `hour.models` (each model's wind) and `hour.agreement` on the
    -- hours the answer covers, matched by local time.
    parse = function(json, forecast)
        local hourly = type(json) == "table" and json.hourly
        if type(hourly) ~= "table" or type(hourly.time) ~= "table" then return end
        local ids = {}
        for name in pairs(hourly) do
            local id = type(name) == "string" and name:match("^" .. COLUMNS.wind .. "_(.+)$")
            if id then ids[#ids + 1] = id end
        end
        table.sort(ids)
        local at = {}
        for index, hour in ipairs(forecast.hours) do at[hour.time] = index end
        for index, time in ipairs(hourly.time) do
            local hour = forecast.hours[at[time] or 0]
            if hour then
                local list = {}
                for _, id in ipairs(ids) do
                    local wind = number(hourly[COLUMNS.wind .. "_" .. id] and hourly[COLUMNS.wind .. "_" .. id][index])
                    if wind then
                        local gusts, dirs = hourly[COLUMNS.gust .. "_" .. id], hourly[COLUMNS.dir .. "_" .. id]
                        list[#list + 1] = { id = id, name = Models.name(id), wind = wind,
                            gust = number(gusts and gusts[index]), dir = number(dirs and dirs[index]) }
                    end
                end
                hour.models, hour.agreement = list, Models.agreement(list)
            end
        end
        forecast.models = ids
    end,
}

return Models
