-- Gardening: is it a good hour for garden work, and what does the garden
-- need? Gardeners watch for frost on tender plants, soil too wet to dig or
-- frozen, wind that ruins spraying, and whether today's rain makes up for
-- the water plants lose to sun and wind (evapotranspiration).
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Report = require "Weather/report"

local Garden = {
    id = "garden", title = "garden", needsMarine = false,
}

-- The hours after `hour`, up to `count` of them.
local function ahead(forecast, hour, count)
    local rows = {}
    for _, other in ipairs(forecast.hours or {}) do
        if other.epoch and hour.epoch and other.epoch > hour.epoch and other.epoch <= hour.epoch + count * 3600 then
            rows[#rows + 1] = other
        end
    end
    return rows
end

local function lowest(rows)
    local low
    for _, row in ipairs(rows) do if row.temp and (not low or row.temp < low) then low = row.temp end end
    return low
end

local function sum(rows, field)
    local total = 0
    for _, row in ipairs(rows) do total = total + (row[field] or 0) end
    return total
end

function Garden.rate(hour, forecast)
    local card = Score.card(55)
    if Sky.isStorm(hour.code) then card:cap(10, "Thunderstorm: stay indoors.") end
    if not hour.isDay then card:cap(25, "Dark: garden work waits for daylight.") end

    if (hour.temp or 5) <= 0 or (hour.soilTemp or 5) <= 0 then card:add(-20, "Frozen ground: leave the soil alone.") end
    local low = lowest(ahead(forecast, hour, 12))
    if low and low <= 1 then card:add(-5, "Frost on the way: cover tender plants.") end

    if Sky.isWet(hour.code) or Sky.isSnow(hour.code) then card:add(-15, "Wet: digging now packs the soil hard.")
    elseif (hour.soilMoisture or 0) > 0.4 then card:add(-8, "Soggy soil: wait for it to drain.")
    elseif (hour.soilMoisture or 0.3) < 0.15 then card:note("Dry soil: water in the evening.") end

    local wind = hour.wind or 0
    if wind > 8 then card:add(-10, "Windy: no spraying; stake tall plants.")
    elseif wind < 4 then card:add(4, "Calm: good for spraying and sowing.") end

    local temp = hour.temp
    if temp and temp >= 8 and temp <= 24 then card:add(12, "Pleasant weather for garden work.")
    elseif temp and temp > 28 then card:add(-10, "Hot: work in the morning, water at dusk.") end
    return card
end

function Garden.window(hour) return hour.isDay end

function Garden.note(hour)
    return ("%s · soil %s · rain %s"):format(Score.fmt(hour.temp, "°"), Score.fmt(hour.soilTemp, "°"),
        Score.fmt(hour.rainChance, "%"))
end

function Garden.metrics(hour, forecast)
    local next24 = ahead(forecast, hour, 24)
    local low = lowest(next24)
    local need = sum(next24, "et0") - sum(next24, "rain")
    local frost = low and low <= 1
    return {
        { label = "frost", value = frost and "frost risk" or "no frost", tone = frost and "poor" or "good",
            note = "lowest in 24 h: " .. Score.fmt(low, "°") },
        { label = "water today?", value = need > 1 and "yes" or "no", tone = need > 1 and "fair" or "good",
            note = need > 1 and ("plants lose about %s more than the rain gives"):format(Score.fmt(need, " mm", 1))
                or "rain covers what plants lose" },
        { label = "rain next 24 h", value = Score.fmt(sum(next24, "rain"), " mm", 1) },
        { label = "soil", value = Score.fmt(hour.soilTemp, "°"),
            note = "moisture " .. Score.fmt((hour.soilMoisture or 0) * 100, "%") .. " near the surface" },
        Report.wind(hour),
        Report.sun(hour),
    }
end

-- The two charts on the activity's "today" view, over the next 24 hours.
Garden.charts = {
    { title = "air °C", kind = "line", value = function(hour) return hour.temp or 0 end },
    { title = "rain mm", kind = "column", min = 0, value = function(hour) return hour.rain or 0 end },
}

return Garden
