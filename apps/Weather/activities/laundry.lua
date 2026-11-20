-- Laundry on the line, an everyday lens (W6): dry, breezy, sunny and not
-- damp. Evaporation (`et0`, mm an hour) says how fast water leaves, which
-- is just what drying is.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Report = require "Weather/report"

local Laundry = { id = "laundry", title = "laundry", needsMarine = false, everywhere = true,
    followed = false } -- off in "good for" until chosen

function Laundry.rate(hour)
    local card = Score.card(45)
    if not hour.isDay then card:cap(25, "After dark: it will only get damper.") end
    if Sky.isWet(hour.code) or Sky.isStorm(hour.code) then card:cap(10, "Rain: bring it in.")
    elseif (hour.rainChance or 0) >= 40 then card:add(-20, "Showers are possible: keep an eye out.") end
    local humidity = hour.humidity or 70
    if humidity >= 85 then card:add(-20, "Damp air: slow drying.")
    elseif humidity <= 60 then card:add(10, "Dry air.") end
    local wind = hour.wind or 0
    if wind > 12 then card:add(-10, "Gale: peg it well.")
    elseif wind >= 2 then card:add(10, "A breeze helps.") end
    if (hour.et0 or 0) >= 0.3 then card:add(8, "Good drying.") end
    if hour.isDay and (hour.cloud or 100) < 40 then card:add(6, "Sunny.") end
    if (hour.temp or 10) < 2 then card:add(-10, "Near freezing: it may stiffen, not dry.") end
    return card
end

function Laundry.window(hour) return hour.isDay end

function Laundry.note(hour)
    return ("humidity %s · wind %s · rain %s"):format(Score.fmt(hour.humidity, "%"), Units.wind(hour.wind),
        Score.fmt(hour.rainChance, "%"))
end

function Laundry.metrics(hour)
    return {
        { label = "humidity", value = Score.fmt(hour.humidity, "%") },
        Report.wind(hour),
        { label = "rain", value = Score.fmt(hour.rainChance, "%"), note = Units.rain(hour.rain) .. " this hour" },
        { label = "evaporation", value = Units.rain(hour.et0, 2), note = "an hour" },
        Report.sun(hour),
    }
end

Laundry.charts = {
    { title = "humidity", unit = "percent", kind = "line", min = 0, max = 100, value = function(hour) return hour.humidity or 0 end },
    { title = "chance of rain", unit = "percent", kind = "column", min = 0, max = 100,
        value = function(hour) return hour.rainChance or 0 end },
}

return Laundry
