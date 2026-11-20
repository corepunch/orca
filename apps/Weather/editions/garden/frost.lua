-- Frost on the plot: air frost and ground frost (GA1). On a clear, still
-- night the ground and the plants on it radiate their warmth to the sky
-- and can sit 2–5 °C below the air two metres up, where the forecast is
-- measured. So "air 3°" can still blacken runner beans. The estimate:
--
--   ground ≈ air − 4° × clear × calm × dry
--
-- each factor 0–1 from cloud cover (clear below 20 %), wind (calm below
-- 1.5 m/s) and the gap to the dew point (dew forming slows the cooling).
-- The model's own surface temperature counts when it is colder. Then:
--
--   likely    air ≤ 0° or ground ≤ −2°  (on a clear, calm night: air ≤ 2°)
--   possible  air ≤ 2° or ground ≤ 0°   (on a clear, calm night: air ≤ 4°)
--
-- A frost pocket runs 2° colder, a sheltered plot 1° warmer; growing hardy
-- plants only moves both thresholds 3° colder. These are rules of thumb
-- (research, feature gap 2) and need tuning against observations.
local Score = require "Weather/score"
local Units = require "Weather/units"
local Plot = require "Weather/editions/garden/plot"
local Frost = {}

local LEVELS = { [0] = "no frost", "possible", "likely" }
Frost.LEVELS = LEVELS

-- One hour's frost: { level 0–2, air, ground, clear, calm, dry } or nil.
function Frost.hour(hour, plot)
    if not hour.temp then return nil end
    plot = plot or Plot.current()
    local offset = plot.offset or 0
    local air = hour.temp + offset
    local clear = Score.curve(hour.cloud or 50, { { 20, 1 }, { 70, 0 } })
    local calm = Score.curve(hour.wind or 3, { { 1.5, 1 }, { 5, 0 } })
    local dry = Score.curve(hour.temp - (hour.dewPoint or hour.temp - 3), { { 0.5, 0.4 }, { 4, 1 } })
    local ground = air - 4 * clear * calm * dry
    if hour.soilTemp and hour.soilTemp + offset < ground then ground = hour.soilTemp + offset end
    local shift = plot.protect == "hardy" and -3 or 0
    local level = (air <= 0 + shift or ground <= -2 + shift) and 2 or (air <= 2 + shift or ground <= 0 + shift) and 1 or 0
    return { level = level, air = air, ground = ground, clear = clear >= 0.6, calm = calm >= 0.6, dry = dry >= 0.8,
        hour = hour }
end

-- The night that starts on the evening of `date`: its hours from 18:00 to
-- 09:00 next morning, when the ground is coldest around sunrise.
local function nightHours(forecast, date, fromEpoch)
    local nextDate = Plot.addDays(date, 1)
    local rows = {}
    for _, hour in ipairs(forecast.hours or {}) do
        local time = hour.time or ""
        local inNight = (time:sub(1, 10) == date and time:sub(12, 13) >= "18")
            or (time:sub(1, 10) == nextDate and time:sub(12, 13) <= "09")
        if inNight and (not fromEpoch or not hour.epoch or hour.epoch >= fromEpoch) then rows[#rows + 1] = hour end
    end
    return rows
end

-- A night's frost: the worst hour, when the risk starts and ends, and the
-- lowest air and ground temperatures. nil when the night has no hours.
function Frost.night(forecast, date, plot, fromEpoch)
    plot = plot or Plot.current()
    local hours = nightHours(forecast, date, fromEpoch)
    if #hours == 0 then return nil end
    local night = { date = date, level = 0, hours = #hours }
    for _, hour in ipairs(hours) do
        local f = Frost.hour(hour, plot)
        if f then
            if not night.worst or f.level > night.worst.level or (f.level == night.worst.level and f.ground < night.worst.ground) then
                night.worst = f
            end
            night.air = math.min(night.air or f.air, f.air)
            night.ground = math.min(night.ground or f.ground, f.ground)
            if f.level > 0 then
                night.from = night.from or hour
                night.to = hour
            end
        end
    end
    night.level = night.worst and night.worst.level or 0
    return night
end

-- Tonight's date: before 05:00 the night that began yesterday evening.
function Frost.tonight(forecast)
    local now = forecast.hours and forecast.hours[forecast.now]
    if not (now and now.date) then return nil end
    local hourOfDay = tonumber(now.time:sub(12, 13)) or 12
    return hourOfDay < 5 and Plot.addDays(now.date, -1) or now.date
end

-- The coming nights, tonight first: up to `count` of them.
function Frost.nights(forecast, count, plot)
    local first = Frost.tonight(forecast)
    local list = {}
    if not first then return list end
    local fromEpoch = Plot.nowEpoch(forecast)
    for index = 0, (count or 3) - 1 do
        local night = Frost.night(forecast, Plot.addDays(first, index), plot, fromEpoch)
        if not night or night.hours < 6 then break end
        list[#list + 1] = night
    end
    return list
end

-- How hard the frost bites, and what to do about it.
function Frost.severity(night)
    if night.air and night.air <= -5 then return "severe", "severe frost: bring pots in and fleece twice; tender plants won't survive outside" end
    if night.air and night.air <= -2 then return "hard", "hard frost: bring pots in; fleece alone may not save tender plants" end
    return "light", "light frost: fleece tender plants before dusk"
end

-- Why the night is cold, in the order a gardener reads it.
local function why(f)
    local parts = {}
    if f.clear then parts[#parts + 1] = "clear sky" end
    if f.calm then parts[#parts + 1] = "still air" end
    if f.dry then parts[#parts + 1] = "dry air" end
    return table.concat(parts, ", ")
end
Frost.why = why

-- The "frost tonight" card, with the next frosty night in the forecast
-- (up to 16 days, GA12) and an all-clear when an earlier forecast frost has gone.
-- `remembered` is what the card said last time ({ date, level }).
function Frost.card(forecast, plot, remembered)
    plot = plot or Plot.current()
    local nights = Frost.nights(forecast, 16, plot)
    local tonight = nights[1]
    if not tonight then return nil end
    local card = { id = "frost", title = "frost tonight", level = tonight.level, date = tonight.date }
    local parts = {}
    if tonight.level > 0 then
        local f = tonight.worst
        card.value = LEVELS[tonight.level]
        card.tone = tonight.level == 2 and "poor" or "fair"
        local reasons = why(f)
        parts[#parts + 1] = ("ground about %s, air %s%s"):format(Units.temp(tonight.ground), Units.temp(tonight.air),
            reasons ~= "" and " · " .. reasons or "")
        if tonight.from then parts[#parts + 1] = Units.span(tonight.from.time, tonight.to.time) end
        local _, advice = Frost.severity(tonight)
        parts[#parts + 1] = advice
        if plot.protect == "blossom" and Plot.season(forecast) == "spring" and tonight.ground <= -1 then
            parts[#parts + 1] = "open blossom is hurt from about " .. Units.temp(-2)
        end
    else
        card.value, card.tone = "no frost", "good"
        parts[#parts + 1] = ("lowest about %s on the ground"):format(Units.temp(tonight.ground))
        if remembered and remembered.date == tonight.date and (remembered.level or 0) > 0 then
            parts[#parts + 1] = "all clear: the frost forecast earlier has gone"
        end
    end
    for index = 2, #nights do
        if nights[index].level > 0 then
            parts[#parts + 1] = ("next: %s night, %s%s"):format(Plot.dayName(nights[index].date, forecast),
                LEVELS[nights[index].level], index > 4 and " (far ahead, so uncertain)" or "")
            break
        end
    end
    if plot.offset ~= 0 then
        parts[#parts + 1] = plot.offset < 0 and ("frost pocket: %s colder than the forecast"):format(Units.tempDelta(-plot.offset))
            or ("sheltered: %s warmer than the forecast"):format(Units.tempDelta(plot.offset))
    end
    card.detail = table.concat(parts, " · ")
    return card
end

-- The first frosty night within `days`, for longer lead times (S14).
function Frost.next(forecast, days, plot)
    for _, night in ipairs(Frost.nights(forecast, days, plot)) do
        if night.level > 0 then return night end
    end
end

return Frost
