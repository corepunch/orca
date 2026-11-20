-- Dog walk, an everyday lens (W6): almost any weather will do, except heat
-- (hot pavement burns paws), thunder, which frightens many dogs, and
-- bitter cold for small ones.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Report = require "Weather/report"

local DogWalk = { id = "dogwalk", title = "dog walk", needsMarine = false, everywhere = true,
    followed = false } -- off in "good for" until chosen

function DogWalk.rate(hour)
    local card = Score.card(65)
    if Sky.isStorm(hour.code) then card:cap(20, "Thunder frightens many dogs: a short one.") end
    local temp = hour.temp
    if temp == nil then card:note("No temperature reading.")
    elseif temp > 30 then card:cap(30, "Too hot for paws: walk early or late.")
    elseif temp > 25 then card:add(-20, "Hot pavement: keep to the shade and grass.")
    elseif temp >= 0 and temp <= 20 then card:add(8, "Comfortable for a dog.")
    elseif temp < -5 then card:add(-12, "Bitter cold: a short walk for small dogs.") end
    if Sky.isHeavy(hour.code) then card:add(-15, "Heavy rain.")
    elseif Sky.isWet(hour.code) then card:add(-5, "Drizzle: a towel by the door.") end
    if (hour.wind or 0) > 12 then card:add(-10, "Stormy wind.") end
    if hour.sunAltitude and hour.sunAltitude < -0.833 then card:note("After dark: a lit collar helps.") end
    return card
end

function DogWalk.window(hour) local h = Sky.hour(hour.time) or 0 return h >= 6 and h <= 22 end

function DogWalk.note(hour)
    return ("%s · rain %s · wind %s"):format(Units.temp(hour.temp), Score.fmt(hour.rainChance, "%"), Units.wind(hour.wind))
end

function DogWalk.metrics(hour)
    return {
        { label = "temperature", value = Units.temp(hour.temp), note = "feels like " .. Units.temp(hour.feels) },
        { label = "rain", value = Score.fmt(hour.rainChance, "%"), note = Units.rain(hour.rain) .. " this hour" },
        Report.wind(hour),
        Report.sun(hour),
    }
end

DogWalk.charts = {
    { title = "temperature", unit = "temp", kind = "column", value = function(hour) return hour.temp or 0 end },
    { title = "chance of rain", unit = "percent", kind = "column", min = 0, max = 100,
        value = function(hour) return hour.rainChance or 0 end },
}

return DogWalk
