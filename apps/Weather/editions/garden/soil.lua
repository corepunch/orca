-- Soil warmth at sowing depth (GA3). The forecast gives the soil at 6 and
-- 18 cm (`soil_temperature_6cm`, `_18cm`); seed sits at about 5 cm, so
-- 6 cm stands for sowing depth and 10 cm (for lawns) lies a third of the
-- way from 6 to 18. Daily means, not afternoon peaks: seed feels the cold
-- night as much as the warm day.
local Units = require "Weather/units"
local Plot = require "Weather/editions/garden/plot"
local Crops = require "Weather/editions/garden/crops"
local Soil = {}

local function at10(hour)
    local a, b = hour.soil_temperature_6cm, hour.soil_temperature_18cm
    if a and b then return a + (b - a) / 3 end
    return a
end

-- Daily means of a field per local date, for dates with 18 hours or more.
function Soil.daily(forecast, value)
    local days = {}
    for _, day in ipairs(Plot.byDate(forecast.hours or {})) do
        local total, count = 0, 0
        for _, hour in ipairs(day.hours) do
            local v = value(hour)
            if v then total, count = total + v, count + 1 end
        end
        if count >= 18 then days[#days + 1] = { date = day.date, mean = total / count } end
    end
    return days
end

local function sowingDepth(hour) return hour.soil_temperature_6cm end

local function mean(days, first, last)
    local total, count = 0, 0
    for index = math.max(1, first), math.min(#days, last) do total, count = total + days[index].mean, count + 1 end
    return count > 0 and total / count or nil
end

-- Today's mean at 6 cm, the coolest of the next three days, and the trend
-- over the week: the next three days against the last three (or, with no
-- history, later in the week against now).
function Soil.state(forecast)
    local now = forecast.hours and forecast.hours[forecast.now]
    if not now then return nil end
    local days = Soil.daily(forecast, sowingDepth)
    local today
    for index, day in ipairs(days) do if day.date == now.date then today = index end end
    if not today then return nil end
    local coolest = math.huge
    for index = today, math.min(#days, today + 2) do coolest = math.min(coolest, days[index].mean) end
    local trend
    if today >= 4 then trend = mean(days, today + 1, today + 3) - mean(days, today - 3, today - 1)
    elseif #days >= today + 5 then trend = mean(days, today + 4, today + 6) - mean(days, today, today + 2) end
    return { today = days[today].mean, coolest = coolest, trend = trend, days = days, index = today }
end

function Soil.trendText(trend)
    if not trend then return nil end
    if trend >= 0.5 then return "warming " .. Units.tempDelta(trend) .. " this week" end
    if trend <= -0.5 then return "cooling " .. Units.tempDelta(-trend) .. " this week" end
    return "steady this week"
end

-- The crops warm enough to sow now: the coolest of the next three days
-- reaches their minimum. Also the next crop the soil is short of.
function Soil.sowable(state)
    local ready, nextCrop = {}, nil
    for _, crop in ipairs(Crops.sowing) do
        if state.coolest >= crop.min then ready[#ready + 1] = crop
        elseif not nextCrop then nextCrop = crop end
    end
    return ready, nextCrop
end

-- The "warm enough to sow" card.
function Soil.card(forecast)
    local state = Soil.state(forecast)
    if not state then return nil end
    local ready, nextCrop = Soil.sowable(state)
    local card = { id = "sow", title = "warm enough to sow" }
    if #ready == 0 then card.value, card.tone = "not yet", "poor"
    elseif not nextCrop then card.value, card.tone = "everything", "good"
    else
        -- The two most demanding, to fit a line; the hardier ones go too.
        local names = {}
        for index = #ready, math.max(1, #ready - 1), -1 do names[#names + 1] = ready[index].name end
        card.value, card.tone = table.concat(names, ", "), "good"
    end
    local parts = { ("soil %s at 6 cm"):format(Units.temp(state.today)) }
    if #ready > 2 then parts[1] = parts[1] .. ", fine for hardier crops too" end
    parts[#parts + 1] = Soil.trendText(state.trend)
    if nextCrop then parts[#parts + 1] = ("next: %s at %s"):format(nextCrop.detail, Units.temp(nextCrop.min)) end
    card.detail = table.concat(parts, " · ")
    return card, state
end

-- The lawn (only with "I have a lawn"): pre-emergent once 10 cm holds
-- 13 °C for three days in spring; seeding while it sits between 10 and
-- 21 °C in late summer and autumn.
function Soil.lawnCard(forecast)
    local now = forecast.hours and forecast.hours[forecast.now]
    if not now then return nil end
    local days = Soil.daily(forecast, at10)
    local today
    for index, day in ipairs(days) do if day.date == now.date then today = index end end
    if not today then return nil end
    local lawn, season = Crops.lawn, Plot.season(forecast)
    local soil = days[today].mean
    local card = { id = "lawn", title = "lawn", tone = "info" }
    local run, first = 0, nil
    for index = math.max(1, today - 3), #days do
        if days[index].mean >= lawn.preEmergent then
            run = run + 1
            if run >= lawn.preEmergentDays and not first then first = days[index - lawn.preEmergentDays + 1].date end
        else run = 0 end
    end
    if season == "spring" and first then
        local already = first <= now.date
        card.value, card.tone = already and "pre-emergent now" or "pre-emergent " .. Plot.dayName(first, forecast), "good"
        card.detail = ("soil %s at 10 cm, %s for %d days in a row: weeds start to sprout"):format(Units.temp(soil),
            Units.temp(lawn.preEmergent), lawn.preEmergentDays)
    elseif (season == "summer" or season == "autumn") and soil >= lawn.seedMin and soil <= lawn.seedMax then
        card.value, card.tone = "good for seeding", "good"
        card.detail = ("soil %s at 10 cm: grass seed takes between %s and %s"):format(Units.temp(soil),
            Units.temp(lawn.seedMin), Units.temp(lawn.seedMax))
    else
        card.value = "soil " .. Units.temp(soil)
        card.detail = season == "spring" and ("pre-emergent once 10 cm holds %s for %d days"):format(
            Units.temp(lawn.preEmergent), lawn.preEmergentDays)
            or ("at 10 cm · seed grass between %s and %s"):format(Units.temp(lawn.seedMin), Units.temp(lawn.seedMax))
    end
    return card
end

return Soil
