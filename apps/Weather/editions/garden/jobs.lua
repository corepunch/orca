-- Job windows (GA4): the next good hours for the jobs gardeners time by the
-- weather, over the next three days. Rules of thumb (research, feature gap
-- 8), each saying why:
--
--   spray      daylight, wind 1–4 m/s (no drift, no inversion), gusts under
--              7, 5–25 °C, dry this hour and the six after          2 h
--   mow        daylight, dry for 12 h before, air 2° above the dew point
--              (a rough sign of dry grass), 5 °C or more             2 h
--   feed       daylight, not frozen, under 10 mm of rain in the next
--              24 h so it isn't washed away                          1 h
--   plant out  a day whose next three nights stay above 10 °C and frost
--              free, with light wind
--   harvest    before frost, a downpour (25 mm in a day), a gale or hail
--              in the next three days
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Plot = require "Weather/editions/garden/plot"
local Frost = require "Weather/editions/garden/frost"
local Crops = require "Weather/editions/garden/crops"
local Jobs = {}

local HOUR, DAY = 3600, 86400

-- The hours from a day ago (for rain already fallen) to three days ahead.
local function window(forecast)
    local now = Plot.nowEpoch(forecast)
    return Plot.between(forecast, now - DAY, now + 3 * DAY), now
end

local function sumBetween(rows, from, to, field)
    local total, top = 0, 0
    for _, row in ipairs(rows) do
        if row.epoch >= from and row.epoch < to then
            total = total + (row[field] or 0)
            top = math.max(top, row[field] or 0)
        end
    end
    return total, top
end

local function dryAhead(rows, hour, hours)
    local rain = sumBetween(rows, hour.epoch, hour.epoch + (hours + 1) * HOUR, "rain")
    local _, chance = sumBetween(rows, hour.epoch, hour.epoch + (hours + 1) * HOUR, "rainChance")
    return rain < 0.2 and chance < 40
end

Jobs.RULES = {
    { id = "spray", title = "spray", run = 2, why = "light wind and dry for 6 h after",
        ok = function(hour, rows)
            return hour.isDay and not Sky.isStorm(hour.code) and (hour.wind or 9) >= 1 and (hour.wind or 9) <= 4
                and (hour.gust or 0) < 7 and (hour.temp or 0) >= 5 and (hour.temp or 0) <= 25 and dryAhead(rows, hour, 6)
        end },
    { id = "mow", title = "mow", run = 2, why = "dry grass: no rain for 12 h",
        ok = function(hour, rows)
            return hour.isDay and (hour.temp or 0) >= 5 and (hour.temp - (hour.dewPoint or hour.temp)) >= 2
                and sumBetween(rows, hour.epoch - 12 * HOUR, hour.epoch, "rain") < 0.2 and dryAhead(rows, hour, 0)
        end },
    { id = "feed", title = "feed", run = 1, why = "no heavy rain to wash it away",
        ok = function(hour, rows)
            return hour.isDay and (hour.temp or 0) > 2 and (hour.soilTemp or 5) > 0 and (hour.rain or 0) < 0.2
                and sumBetween(rows, hour.epoch, hour.epoch + DAY, "rain") < 10
                and select(2, sumBetween(rows, hour.epoch, hour.epoch + DAY, "rain")) < 4
        end },
}

-- The first run of `rule.run` good hours from now: its first and last hour.
local function firstRun(rule, rows, now)
    local start, last, count = nil, nil, 0
    for _, hour in ipairs(rows) do
        if hour.epoch >= now then
            local good = rule.ok(hour, rows)
            if good and last and hour.epoch == last.epoch + HOUR then last, count = hour, count + 1
            elseif count >= rule.run then break
            elseif good then start, last, count = hour, hour, 1
            else start, last, count = nil, nil, 0 end
        end
    end
    if count >= rule.run then return start, last end
end

-- "today 10–13h"
local function when(forecast, from, to)
    return ("%s %s"):format(Plot.dayName(from.date, forecast), Units.span(from.time, to.time))
end

-- The day to plant tender plants out: its next three nights mild and frost
-- free, and the day itself not windy.
local function plantOut(forecast, plot)
    local nights = Frost.nights(forecast, 6, plot)
    for index = 1, #nights - 2 do
        local mild = true
        for k = index, index + 2 do
            local n = nights[k]
            mild = mild and n.level == 0 and (n.air or -99) >= Crops.plantOutNight
        end
        local date = nights[index].date
        local windy = false
        for _, hour in ipairs(forecast.hours) do
            if hour.date == date and hour.isDay and (hour.wind or 0) > 6 then windy = true end
        end
        if mild and not windy then return date end
    end
    local coldest
    for index = 1, math.min(3, #nights) do coldest = math.min(coldest or math.huge, nights[index].air or math.huge) end
    return nil, coldest
end

-- What could spoil a crop in the next three days, and when it comes.
local function threat(forecast, plot)
    local rows, now = window(forecast)
    for _, night in ipairs(Frost.nights(forecast, 3, plot)) do
        if night.level == 2 then return night.date, "frost" end
    end
    for _, day in ipairs(Plot.byDate(rows)) do
        local rain, gust, hail = 0, 0, false
        for _, hour in ipairs(day.hours) do
            if hour.epoch >= now then
                rain = rain + (hour.rain or 0)
                gust = math.max(gust, hour.gust or 0)
                hail = hail or hour.code == 96 or hour.code == 99
            end
        end
        if hail then return day.date, "hail" end
        if gust >= 17 then return day.date, "a gale" end
        if rain >= 25 then return day.date, ("%s of rain"):format(Units.rain(rain)) end
    end
end

-- Every job: { id, title, value, note, tone }.
function Jobs.list(forecast, plot)
    plot = plot or Plot.current()
    local rows, now = window(forecast)
    local jobs = {}
    for _, rule in ipairs(Jobs.RULES) do
        local from, to = firstRun(rule, rows, now)
        jobs[#jobs + 1] = from and { id = rule.id, title = rule.title, value = when(forecast, from, to), note = rule.why,
            tone = "good", date = from.date, short = Plot.dayName(from.date, forecast), span = Units.span(from.time, to.time) }
            or { id = rule.id, title = rule.title, value = "not in 3 days", note = "no window: " .. rule.why, tone = "poor" }
    end
    local date, coldest = plantOut(forecast, plot)
    jobs[#jobs + 1] = date and { id = "plant", title = "plant out", value = Plot.dayName(date, forecast), tone = "good",
        note = ("nights above %s and frost free"):format(Units.temp(Crops.plantOutNight)), date = date }
        or { id = "plant", title = "plant out", value = "not yet", tone = "poor",
            note = ("tender plants wait for nights above %s (coming nights %s)"):format(Units.temp(Crops.plantOutNight),
                Units.temp(coldest)) }
    local by, what = threat(forecast, plot)
    -- Harvest the day before the threat, or today when it comes today.
    local today = forecast.hours[forecast.now].date
    local pickBy = by and Plot.addDays(by, -1)
    if pickBy and pickBy < today then pickBy = today end
    jobs[#jobs + 1] = by and { id = "harvest", title = "harvest", value = "before " .. Plot.dayName(by, forecast),
        tone = "fair", note = ("%s %s: lift and pick what it would spoil"):format(what, by == today and "today"
            or "comes " .. Plot.dayName(by, forecast)), urgent = true,
        short = pickBy == today and "today" or "by " .. Plot.dayName(pickBy, forecast) }
        or { id = "harvest", title = "harvest", value = "no rush", tone = "good", note = "no frost, gale or downpour in 3 days" }
    return jobs
end

-- The "garden jobs" card: the soonest job in big type, the rest in a line.
function Jobs.card(forecast, plot)
    local jobs = Jobs.list(forecast, plot)
    local lead
    for _, job in ipairs(jobs) do
        if job.urgent then lead = job; break end
    end
    if not lead then
        for _, job in ipairs(jobs) do
            if job.date and (not lead or job.date < lead.date) then lead = job end
        end
    end
    if not lead then return { id = "jobs", title = "garden jobs", value = "wait", tone = "poor",
        detail = "no good window for spraying, mowing or feeding in the next 3 days" }, jobs end
    local parts = { lead.span and lead.span .. ": " .. lead.note or lead.note }
    for _, job in ipairs(jobs) do
        if job ~= lead then parts[#parts + 1] = job.title .. " " .. job.value end
    end
    -- The value stays short enough for one line of big type.
    return { id = "jobs", title = "garden jobs", value = lead.title .. " " .. (lead.short or lead.value), tone = lead.tone,
        detail = table.concat(parts, " · ") }, jobs
end

return Jobs
