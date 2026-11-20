-- Picnic, an everyday lens (W6): daylight, a dry sky, warmth and little
-- wind; a picnic is spoiled by any one of them going wrong.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Report = require "Weather/report"

local Picnic = { id = "picnic", title = "picnic", needsMarine = false, everywhere = true,
    followed = false } -- off in "good for" until chosen

function Picnic.rate(hour)
    local card = Score.card(50)
    if Sky.isStorm(hour.code) then card:cap(10, "Thunderstorm.") end
    if not hour.isDay then card:cap(20, "After dark.") end
    if Sky.isWet(hour.code) then card:cap(25, "Wet: not on a blanket.")
    elseif (hour.rainChance or 0) >= 40 then card:add(-15, "Rain is quite likely: pick a spot with cover.") end
    local feels = hour.feels or hour.temp
    if feels == nil then card:note("No temperature reading.")
    elseif feels < 14 then card:add(-15, "Too cool to sit for long.")
    elseif feels <= 28 then card:add(15, "Warm enough to sit out.")
    else card:add(-10, "Hot: find shade.") end
    if (hour.wind or 0) > 7 then card:add(-15, "Breezy: the napkins will fly.") end
    if hour.isDay and (hour.cloud or 100) < 50 then card:add(8, "Sunny.") end
    if (hour.uv or 0) >= 6 then card:note("High UV: sunscreen and a hat.") end
    return card
end

function Picnic.window(hour) return hour.isDay end

function Picnic.note(hour)
    return ("%s · rain %s · wind %s"):format(Units.temp(hour.feels), Score.fmt(hour.rainChance, "%"), Units.wind(hour.wind))
end

function Picnic.metrics(hour)
    return {
        { label = "feels like", value = Units.temp(hour.feels), note = "air " .. Units.temp(hour.temp) },
        { label = "rain", value = Score.fmt(hour.rainChance, "%"), note = Units.rain(hour.rain) .. " this hour" },
        Report.wind(hour),
        { label = "uv index", value = Score.fmt(hour.uv) },
        Report.sun(hour),
    }
end

Picnic.charts = {
    { title = "feels like", unit = "temp", kind = "column", value = function(hour) return hour.feels or hour.temp or 0 end },
    { title = "chance of rain", unit = "percent", kind = "column", min = 0, max = 100,
        value = function(hour) return hour.rainChance or 0 end },
}

return Picnic
