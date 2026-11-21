-- Growing degree days (GA10, SV-GA2): how much warmth plants and insects
-- have had this season, and whether the season runs ahead of an average
-- year. No new service: the days come from Open-Meteo's Historical Weather
-- API (the archive climate.lua already asks), up to about five days ago,
-- then from the forecast's own past and present days.
--
--   a day's GDD   max(0, (min(max, 30) + max(min, 10)) / 2 − 10)   °C
--                 (base 10 °C / 50 °F, the "modified" method: no credit
--                 above 30 °C / 86 °F or below the base)
--   season        from 1 January; 1 April in Britain, where gardeners
--                 count from spring; 1 July south of the equator
--   ahead         the day an average year (climate.lua's 30 years) had
--                 this much warmth, against today: "18 days ahead"
--
-- Lawns (US): crabgrass germinates as the soil warms, and extension
-- services time the pre-emergent by base-50 °F GDD from 1 January, about
-- 200–250 °F-days (Michigan State University Turfgrass, GDD Tracker,
-- gddtracker.net; *verify the window before shipping*). Labelled as a rule
-- of thumb.
--
-- The archive answer is boiled down to a total and kept with Storage
-- ("gdd-<lat>_<lon>"), asked at most once a day and only after a place's
-- cards have been drawn once, so it never holds up the hub.
local Forecast = require "Weather/forecast"
local Storage = require "Weather/storage"
local Units = require "Weather/units"
local Sky = require "Weather/sky"
local Degree = {}

Degree.BASE, Degree.CAP = 10, 30
Degree.LAG = 5             -- days the archive runs behind
Degree.RETRY = 6 * 3600
Degree.CRABGRASS = { 200, 250 } -- °F-days, base 50 °F, from 1 January

local DAY = 86400

function Degree.day(max, min)
    if not (max and min) then return nil end
    return math.max(0, (math.min(max, Degree.CAP) + math.max(min, Degree.BASE)) / 2 - Degree.BASE)
end

local function british(spot)
    return spot and spot.lat and spot.lat >= 49.8 and spot.lat <= 61 and spot.lon >= -11 and spot.lon <= 2
end

local function dateOf(epoch) return os.date("!%Y-%m-%d", epoch) end
local function addDays(date, n) local e = Sky.epochOf(date); return e and dateOf(e + n * DAY) end
local function daysBetween(from, to) return math.floor((Sky.epochOf(to) - Sky.epochOf(from)) / DAY + 0.5) end

-- The first day of the season that holds `date`.
function Degree.start(spot, date)
    local y, m = tonumber(date:sub(1, 4)), tonumber(date:sub(6, 7))
    if spot and spot.lat and spot.lat < 0 then return ("%d-07-01"):format(m >= 7 and y or y - 1) end
    if british(spot) then return ("%d-04-01"):format(y) end
    return ("%d-01-01"):format(y)
end

-- "1 jan", or "jan 1" where the US reads °F.
local MONTHS = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" }
function Degree.dateText(date)
    local m, d = tonumber(date:sub(6, 7)), tonumber(date:sub(9, 10))
    return Units.current("temp") == "f" and ("%s %d"):format(MONTHS[m], d) or ("%d %s"):format(d, MONTHS[m])
end

-- Degree days in the user's units: °F-days are 1.8 °C-days.
function Degree.show(c) return math.floor((Units.current("temp") == "f" and c * 1.8 or c) + 0.5) end

local memory = {}
function Degree.key(spot)
    if not (spot and spot.lat and spot.lon) then return nil end
    return ("gdd-%.1f_%.1f"):format(spot.lat, spot.lon)
end
function Degree.saved(spot)
    local key = Degree.key(spot)
    if not key then return nil end
    if memory[key] == nil then memory[key] = Storage.load(key) or false end
    return memory[key] or nil
end
local function keep(spot, value)
    local key = Degree.key(spot)
    if not key then return end
    memory[key] = value
    Storage.save(key, value)
end
function Degree.reset() memory = {} end

-- Marks a place wanted the first time its cards are drawn.
function Degree.want(spot)
    if Degree.key(spot) and not Degree.saved(spot) then keep(spot, { wanted = true }) end
end

-- The archive URL from the season's start to about five days ago, or nil
-- when the total is current, the place unseen, or a try failed recently.
function Degree.url(spot, now)
    local saved = Degree.saved(spot)
    if not saved then return nil end
    now = now or os.time()
    local start, stop = Degree.start(spot, dateOf(now)), dateOf(now - Degree.LAG * DAY)
    if stop < start then return nil end
    if saved.start == start and saved.through and saved.through >= stop then return nil end
    if saved.tried and now - saved.tried < Degree.RETRY then return nil end
    local again = {}
    for k, v in pairs(saved) do again[k] = v end
    again.tried = now
    keep(spot, again)
    return Forecast.url("archive", ("latitude=%s&longitude=%s&start_date=%s&end_date=%s"
        .. "&daily=temperature_2m_max,temperature_2m_min&timezone=auto"):format(spot.lat, spot.lon, start, stop))
end

-- The forecast has no `spot` while its sources parse, and the archive
-- answers with its grid point, so the place asked is remembered from `url`
-- (sources run one at a time). A replayed answer has none and is not kept.
local asking
-- The place asked, if the answer's grid point is near it (a replay after
-- a failed request must not borrow another place's).
local function near(spot, json)
    if not (spot and json.latitude and json.longitude) then return spot end
    return math.abs(spot.lat - json.latitude) < 0.5 and math.abs(spot.lon - json.longitude) < 0.5 and spot or nil
end
Degree.source = {
    id = "gdd", optional = true,
    url = function(spot) local url = Degree.url(spot); asking = url and spot; return url end,
    parse = function(json, forecast)
        local spot = forecast.spot or near(asking, json)
        asking = nil
        local daily = json.daily or {}
        local dates, maxs, mins = daily.time or {}, daily.temperature_2m_max or {}, daily.temperature_2m_min or {}
        assert(#dates > 0, "no days")
        local start = Degree.start(spot or { lat = json.latitude, lon = json.longitude }, dates[#dates])
        local total, through = 0, nil
        for index, date in ipairs(dates) do
            local g = Degree.day(maxs[index], mins[index])
            if date >= start and g then
                -- A missing day breaks the run: the total stops before it.
                if through and date ~= addDays(through, 1) then break end
                total, through = total + g, date
            end
        end
        assert(through, "no whole days this season")
        local value = { wanted = true, start = start, through = through, total = total }
        if spot then keep(spot, value) end
        forecast.gdd = value
    end,
}

-- This season's total to today: the archive's total and the forecast's
-- days after it. { total (°C-days), start, today, index (day of season) },
-- `before = true` before the season starts; nil while a day is missing.
function Degree.total(forecast, spot)
    spot = spot or forecast.spot
    local now = forecast.hours and forecast.hours[forecast.now]
    local today = now and now.date
    if not today then return nil end
    local start = Degree.start(spot, today)
    if british(spot) and today < start then return { before = true, start = start, today = today } end
    -- The newer of this forecast's answer and the one kept.
    local saved, kept = forecast.gdd, Degree.saved(spot)
    if kept and kept.through and not (saved and saved.through and saved.through >= kept.through) then saved = kept end
    if not (saved and saved.start == start and saved.through) then return nil end
    local total, last = saved.total, saved.through
    for _, day in ipairs(forecast.days or {}) do
        if day.date > last and day.date <= today then
            local g = Degree.day(day.max, day.min)
            if day.date ~= addDays(last, 1) or not g then return nil end
            total, last = total + g, day.date
        end
    end
    if last ~= today then return nil end
    return { total = total, start = start, today = today, index = daysBetween(start, today) + 1 }
end

-- The average season (for climate.lua's 30 years): the mean total by each
-- day of the season, 1 to 366. nil without daily maximums.
function Degree.normal(dates, maxs, mins, spot)
    if not (dates and maxs and mins) then return nil end
    local seasons, order = {}, {}
    for index, date in ipairs(dates) do
        local g = Degree.day(maxs[index], mins[index])
        local start = Degree.start(spot, date)
        if g and date >= start then
            local s = seasons[start]
            if not s then s = { days = {}, count = 0 }; seasons[start] = s; order[#order + 1] = start end
            s.days[daysBetween(start, date) + 1] = g
            s.count = s.count + 1
        end
    end
    local sums, n = {}, 0
    for _, start in ipairs(order) do
        local s = seasons[start]
        -- Whole seasons only: Britain's runs April to December.
        if s.count >= (british(spot) and 270 or 360) then
            n = n + 1
            local running = 0
            for k = 1, 366 do
                running = running + (s.days[k] or 0)
                sums[k] = (sums[k] or 0) + running
            end
        end
    end
    if n < 10 then return nil end
    local curve = {}
    for k = 1, 366 do curve[k] = math.floor(sums[k] / n * 10 + 0.5) / 10 end
    return curve
end

-- Days ahead (+) or behind (−) an average season: the day the average
-- year reached this total, against today. nil when the average is
-- unknown, too little warmth has come yet to tell (under 30 °C-days), or
-- the average year barely warms now (under 1 °C-day a day over the last
-- two weeks, late autumn and winter), when days would exaggerate a small
-- gap; the second value is then the gap in °C-days.
function Degree.ahead(total, normal)
    if not (total and total.index and normal and normal[total.index]) then return nil end
    local index, mean = total.index, normal[total.index]
    if mean < 30 then return nil end
    if (mean - normal[math.max(1, index - 14)]) / 14 < 1 then return nil, total.total - mean end
    for k = 1, #normal do
        if normal[k] >= total.total then return k - index end
    end
    -- Past the whole average season: at least this many days ahead.
    return #normal - index + 1
end

-- The "growing season" card. `climate` is climate.lua's result, or nil.
function Degree.card(forecast, spot, climate, plot)
    spot = spot or forecast.spot
    local t = Degree.total(forecast, spot)
    if not t then
        return { id = "season", title = "growing season", value = "not yet", tone = "info", hub = false,
            detail = "degree days are added up from this season's records after the next refresh" }
    end
    if t.before then
        return { id = "season", title = "growing season", value = "from " .. Degree.dateText(t.start), tone = "info",
            hub = false, detail = "degree days count from " .. Degree.dateText(t.start) .. " in Britain" }
    end
    local card = { id = "season", title = "growing season", tone = "info", hub = false }
    local parts = { ("%d degree days since %s (base %s)"):format(Degree.show(t.total), Degree.dateText(t.start),
        Units.temp(Degree.BASE)) }
    local normal = climate and climate.gddNormal or nil
    local ahead, gap = Degree.ahead(t, normal)
    if gap then
        local shown = Degree.show(math.abs(gap))
        if math.abs(gap) < math.max(10, 0.05 * normal[t.index]) then card.value = "about average"
        else card.value = ("%d degree days %s average"):format(shown, gap > 0 and "above" or "below") end
    elseif ahead and ahead >= 4 then
        card.value = ("%d days ahead"):format(ahead)
        parts[#parts + 1] = ("crops such as sweetcorn ripen by warmth: about %d days early this year"):format(ahead)
    elseif ahead and ahead <= -4 then
        card.value = ("%d days behind"):format(-ahead)
        parts[#parts + 1] = ("crops such as sweetcorn ripen by warmth: about %d days late this year"):format(-ahead)
    elseif ahead then
        card.value = "about average"
    else
        card.value = Degree.show(t.total) .. " degree days"
    end
    if normal and normal[t.index] then
        parts[#parts + 1] = ("an average year has %d by now%s"):format(Degree.show(normal[t.index]),
            climate.from and (" (%d–%d)"):format(climate.from, climate.to) or "")
    elseif not climate then
        parts[#parts + 1] = "the average year comes with the frost dates"
    end
    local line = plot and plot.lawn and Degree.crabgrass(t, spot)
    if line then parts[#parts + 1] = line end
    card.detail = table.concat(parts, " · ")
    return card
end

-- The crabgrass pre-emergent line for a lawn, in spring, where the season
-- counts from 1 January (the US rule), or nil.
function Degree.crabgrass(t, spot)
    if not (t and t.total and t.start:sub(6) == "01-01") or british(spot) then return nil end
    local month = tonumber(t.today:sub(6, 7))
    if month < 2 or month > 5 then return nil end
    local f = t.total * 1.8
    local low, high = Degree.CRABGRASS[1], Degree.CRABGRASS[2]
    local window = ("about %d–%d °F degree days, base 50 °F"):format(low, high)
    if f < low - 50 then return "crabgrass preventer: not yet (" .. window .. "; rule of thumb)" end
    if f < low then return "crabgrass preventer: soon (" .. window .. "; rule of thumb)" end
    if f <= high then return "crabgrass preventer: now (" .. window .. "; rule of thumb)" end
    return "crabgrass preventer: the window has passed (" .. window .. "; rule of thumb)"
end

return Degree
