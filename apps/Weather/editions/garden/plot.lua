-- The gardener's plot, from three choices instead of a plant list
-- (research: "no data entry"), and the small helpers every garden module
-- shares: hours between two times, days by date, and the text for a day.
local Settings = require "Weather/settings"
local Units = require "Weather/units"
local Sky = require "Weather/sky"
local Plot = {}

-- Declarations for the settings page (Garden.settings).
Plot.SETTINGS = {
    { key = "site", title = "my plot", kind = "choice", default = "open",
        options = { { "open", "open ground" }, { "pocket", "frost pocket" }, { "sheltered", "sheltered or walled" } },
        detail = "frost pockets run colder on clear, still nights" },
    { key = "protect", title = "I protect", kind = "choice", default = "tender",
        options = { { "tender", "tender plants" }, { "blossom", "tender plants and fruit blossom" },
            { "hardy", "hardy plants only" } },
        detail = "frost warnings start at the frost these plants feel" },
    { key = "soil", title = "soil", kind = "choice", default = "loam",
        options = { { "sandy", "sandy" }, { "loam", "loam" }, { "clay", "clay" } },
        detail = "how fast it dries and how long it stays too wet to dig" },
    { key = "lawn", title = "I have a lawn", kind = "toggle", default = false, detail = "lawn feed and seeding by soil warmth" },
    { key = "moon", title = "moon gardening", kind = "toggle", default = false, detail = "a tradition, not science" },
}

-- What a frost pocket or a sheltered corner does to the forecast low, °C.
Plot.SITE_OFFSET = { open = 0, pocket = -2, sheltered = 1 }

-- Soil water, as a volume fraction: field capacity (full after draining)
-- and wilting point (roots can draw no more). Textbook values, approximate.
Plot.SOILS = {
    sandy = { full = 0.15, wilt = 0.05, dry = 10 },
    loam = { full = 0.30, wilt = 0.12, dry = 15 },
    clay = { full = 0.40, wilt = 0.22, dry = 20 },
}

function Plot.get(key) return Settings.get("garden", key) end

-- The plot as one table: site offset, what is protected, soil, lawn, moon.
function Plot.current()
    local site, soil = Plot.get("site") or "open", Plot.get("soil") or "loam"
    return { site = site, offset = Plot.SITE_OFFSET[site] or 0, protect = Plot.get("protect") or "tender",
        soil = soil, soilInfo = Plot.SOILS[soil] or Plot.SOILS.loam, lawn = Plot.get("lawn") == true,
        moon = Plot.get("moon") == true }
end

-- The current hour's UTC time.
function Plot.nowEpoch(forecast)
    local now = forecast.hours and forecast.hours[forecast.now]
    return now and now.epoch or forecast.epoch
end

-- Hours with `from <= epoch < to`, in order.
function Plot.between(forecast, from, to)
    local rows = {}
    for _, hour in ipairs(forecast.hours or {}) do
        if hour.epoch and hour.epoch >= from and hour.epoch < to then rows[#rows + 1] = hour end
    end
    return rows
end

function Plot.sum(rows, field)
    local total = 0
    for _, row in ipairs(rows) do total = total + (row[field] or 0) end
    return total
end

-- The hours of each date, as { date, hours } in order.
function Plot.byDate(rows)
    local days, index = {}, {}
    for _, hour in ipairs(rows) do
        local date = hour.date or (hour.time and hour.time:sub(1, 10))
        if date then
            if not index[date] then index[date] = #days + 1; days[#days + 1] = { date = date, hours = {} } end
            table.insert(days[index[date]].hours, hour)
        end
    end
    return days
end

-- A date shifted by whole days ("2026-11-18", 1 → "2026-11-19").
function Plot.addDays(date, n)
    local epoch = Sky.epochOf(date)
    return epoch and os.date("!%Y-%m-%d", epoch + n * 86400)
end

-- "today", "tomorrow" or "fri 20" for a date, from the current hour's date.
function Plot.dayName(date, forecast)
    local now = forecast.hours and forecast.hours[forecast.now]
    local today = now and now.date
    if date == today then return "today" end
    if today and date == Plot.addDays(today, 1) then return "tomorrow" end
    return Sky.weekday(date)
end

-- Water as gardeners measure it: litres per square metre (1 mm of rain is
-- 1 L/m²), or inches where rain is read in inches.
function Plot.water(mm)
    if Units.current("rain") == "in" then return Units.rain(mm) end
    return ("%d L/m²"):format(math.floor((mm or 0) + 0.5))
end

-- A soil depth as the user reads lengths: "6 cm", or "2 in" with feet.
-- With a second depth, a range: "3–27 cm".
function Plot.depth(cm, to)
    local inches = Units.current("height") == "ft"
    local function n(v) return inches and math.floor(v / 2.54 + 0.5) or v end
    return (to and ("%d–%d"):format(n(cm), n(to)) or ("%d"):format(n(cm))) .. (inches and " in" or " cm")
end

-- The local month (1–12) of the current hour, and whether it is spring at
-- this latitude (March–May north, September–November south).
function Plot.month(forecast)
    local now = forecast.hours and forecast.hours[forecast.now]
    return tonumber(now and now.time and now.time:sub(6, 7))
end

function Plot.season(forecast)
    local month, spot = Plot.month(forecast), forecast.spot
    if not month then return nil end
    if spot and spot.lat and spot.lat < 0 then month = (month + 5) % 12 + 1 end
    return month >= 3 and month <= 5 and "spring" or month >= 6 and month <= 8 and "summer"
        or month >= 9 and month <= 11 and "autumn" or "winter"
end

return Plot
