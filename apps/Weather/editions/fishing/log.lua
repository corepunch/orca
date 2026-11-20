-- The private catch log (FI6) and what it teaches (FI7). A catch, or a
-- trip with no fish, is one tap: it saves the hour's conditions from the
-- forecast already on the device, so it works on the water without signal.
-- Everything stays in this edition's own folder ("fishing-log"); export
-- writes the whole log as CSV beside it, free, for good.
--
--   Log.add(forecast, place, { kind = "catch", species = "pike" }, epoch)
--   Log.entries()                 newest first
--   Log.setReading(placeId, 9.5, epoch); Log.reading(placeId)
--   Log.csv(); Log.export()       the text, or the file it wrote
--   Log.patterns()                "your catches cluster in…" once there are enough
local Storage = require "Weather/storage"
local Sky = require "Weather/sky"
local Fronts = require "Weather/editions/fishing/fronts"
local Water = require "Weather/editions/fishing/water"
local Tides = require "Weather/editions/fishing/tides"
local Log = {}

Log.FILE = "fishing-log"
Log.PATTERN_MINIMUM = 15

local state = {}

local function store() return state.store or Storage.default() end

-- The log as saved: { entries = { … }, readings = { [placeId] = { temp, epoch } }, serial }.
local function data()
    local current = store()
    if state.loadedFrom ~= current then
        local saved = current:load(Log.FILE)
        state.data = type(saved) == "table" and saved or {}
        state.data.entries = state.data.entries or {}
        state.data.readings = state.data.readings or {}
        state.data.serial = state.data.serial or #state.data.entries
        state.loadedFrom = current
    end
    return state.data
end

local function save() return store():save(Log.FILE, data()) end

-- Tests and the app may point the log at a store; nil follows Storage.default().
function Log.use(storeOrNil) state.store, state.loadedFrom = storeOrNil, nil end

-- The forecast hour an epoch falls in (the current hour, or an earlier one
-- the forecast still holds: two past days), or nil.
local function hourAt(forecast, epoch)
    for _, hour in ipairs(forecast.hours or {}) do
        if hour.epoch and epoch >= hour.epoch and epoch < hour.epoch + 3600 then return hour end
    end
    return nil
end

local function round(value, decimals)
    if value == nil then return nil end
    local scale = 10 ^ (decimals or 0)
    local rounded = math.floor(value * scale + 0.5) / scale
    return decimals and decimals > 0 and rounded or math.tointeger(rounded) or rounded
end

-- Saves a catch or a blank and returns it, or nil, why. `entry.kind` is
-- "catch" (default) or "skunk"; `species`, `length` (cm), `lure` and
-- `note` are optional. The conditions come from the forecast's hour at
-- `epoch` (default now).
function Log.add(forecast, place, entry, epoch)
    entry = entry or {}
    epoch = epoch or os.time()
    local hour = forecast and hourAt(forecast, epoch)
    if not hour then return nil, "The forecast does not reach that hour." end
    local d = data()
    local water = Water.at(hour, forecast, Log.reading(place.id, epoch))
    local tide = Tides.of(forecast)
    local stage = tide and Tides.stage(tide, os.date("!%Y-%m-%dT%H:%M", epoch + (forecast.offset or 0)))
    local moon = Sky.moon(epoch)
    -- Required here: the activity reads the log for water readings.
    local verdict = require("Weather/activities/fishing").rate(hour, forecast):result()
    d.serial = d.serial + 1
    local saved = {
        id = d.serial, kind = entry.kind == "skunk" and "skunk" or "catch", epoch = epoch,
        time = os.date("!%Y-%m-%dT%H:%M", epoch + (forecast.offset or 0)), place = place.id, placeTitle = place.title,
        species = entry.species, length = entry.length, lure = entry.lure, note = entry.note,
        temp = round(hour.temp, 1), wind = round(hour.wind, 1), gust = round(hour.gust, 1), windDir = round(hour.windDir),
        pressure = round(hour.pressure, 1), trend3 = round(hour.pressureTrend, 1),
        trend6 = round(Fronts.change(forecast, hour, 6), 1), cloud = round(hour.cloud), code = hour.code,
        rain = round(hour.rain, 1), water = water and round(water.temp, 1), waterSource = water and water.kind,
        tide = stage and stage.stage, moon = moon.name, isDay = hour.isDay,
        score = verdict and verdict.score, verdict = verdict and verdict.verdict,
    }
    table.insert(d.entries, 1, saved)
    local ok, err = save()
    if not ok then table.remove(d.entries, 1); return nil, err end
    return saved
end

function Log.entries() return data().entries end

-- Takes back the newest entry (a slip of a wet thumb).
function Log.undo()
    local d = data()
    local removed = table.remove(d.entries, 1)
    if removed then save() end
    return removed
end

-- The angler's own water reading for a place, kept three days.
function Log.setReading(placeId, temp, epoch)
    local d = data()
    d.readings[placeId] = temp and { temp = temp, epoch = epoch or os.time() } or nil
    return save()
end

function Log.reading(placeId, now)
    local reading = placeId and data().readings[placeId]
    if not reading then return nil end
    if now and math.abs(now - reading.epoch) > Water.READING_SECONDS then return nil end
    return reading
end

-- CSV with a header row; text fields are quoted.
Log.COLUMNS = { "time", "kind", "placeTitle", "species", "length", "lure", "note", "temp", "water", "waterSource",
    "wind", "gust", "windDir", "pressure", "trend3", "trend6", "cloud", "rain", "code", "tide", "moon", "score", "verdict" }
local HEADER = { placeTitle = "place", trend3 = "pressure_change_3h", trend6 = "pressure_change_6h", temp = "air_c",
    water = "water_c", wind = "wind_ms", gust = "gust_ms", windDir = "wind_dir", pressure = "pressure_hpa",
    cloud = "cloud_pct", rain = "rain_mm", code = "weather_code", length = "length_cm", waterSource = "water_source" }

local function cell(value)
    if value == nil then return "" end
    if type(value) == "number" or type(value) == "boolean" then return tostring(value) end
    value = tostring(value)
    if value:find('[,"\n\r]') then return '"' .. value:gsub('"', '""') .. '"' end
    return value
end

function Log.csv()
    local lines, header = {}, {}
    for index, column in ipairs(Log.COLUMNS) do header[index] = HEADER[column] or column end
    lines[1] = table.concat(header, ",")
    local entries = data().entries
    for index = #entries, 1, -1 do
        local row = {}
        for c, column in ipairs(Log.COLUMNS) do row[c] = cell(entries[index][column]) end
        lines[#lines + 1] = table.concat(row, ",")
    end
    return table.concat(lines, "\n") .. "\n"
end

-- Writes the CSV next to the log (atomically) and returns its path.
function Log.export()
    local dir = store().dir
    if not dir then return nil, "There is no folder to write to." end
    if not store():save(Log.FILE, data()) then return nil, "Cannot write to " .. dir end
    local path = dir .. "/" .. Log.FILE .. ".csv"
    local file = io.open(path .. ".tmp", "wb")
    if not file then return nil, "Cannot write " .. path end
    local written, closed = file:write(Log.csv()), file:close()
    if not (written and closed and os.rename(path .. ".tmp", path)) then
        os.remove(path .. ".tmp")
        return nil, "Cannot write " .. path
    end
    return path
end

-- The bands a catch falls in, for patterns.
local BANDS = {
    { key = "pressure", title = "pressure", band = function(e)
        local t = e.trend3
        if t == nil then return nil end
        return t <= -0.5 and "falling" or t >= 0.5 and "rising" or "steady"
    end },
    { key = "wind", title = "wind", band = function(e)
        local w = e.wind
        if w == nil then return nil end
        return w < 1.5 and "calm" or w <= 5 and "light" or w <= 8 and "fresh" or "strong"
    end },
    { key = "light", title = "time of day", band = function(e)
        local h = Sky.hour(e.time)
        if h == nil then return nil end
        return h < 5 and "night" or h < 9 and "early morning" or h < 17 and "day" or h < 21 and "evening" or "night"
    end },
    { key = "water", title = "water", band = function(e)
        local w = e.water
        if w == nil then return nil end
        return w < 8 and "under 8°" or w < 14 and "8–14°" or w < 20 and "14–20°" or "20° and over"
    end },
}
Log.BANDS = BANDS

-- What the angler's own catches share: per band kind, the band most
-- catches fell in and how many, from at least PATTERN_MINIMUM catches.
-- Returns { count, ready, patterns = { { key, title, band, count, of } } }.
function Log.patterns()
    local catches = {}
    for _, entry in ipairs(data().entries) do
        if entry.kind == "catch" then catches[#catches + 1] = entry end
    end
    local result = { count = #catches, ready = #catches >= Log.PATTERN_MINIMUM, patterns = {} }
    if not result.ready then return result end
    for _, kind in ipairs(BANDS) do
        local counts, of = {}, 0
        for _, entry in ipairs(catches) do
            local band = kind.band(entry)
            if band then counts[band] = (counts[band] or 0) + 1; of = of + 1 end
        end
        local best, most = nil, 0
        for band, count in pairs(counts) do
            if count > most or (count == most and best and band < best) then best, most = band, count end
        end
        if best and of > 0 then
            result.patterns[#result.patterns + 1] = { key = kind.key, title = kind.title, band = best, count = most, of = of }
        end
    end
    return result
end

return Log
