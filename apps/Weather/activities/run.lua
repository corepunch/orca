-- Run, an everyday lens (W6): cool, dry enough and calm. Runners warm up
-- fast, so the comfortable band sits lower than for a stroll, and muggy
-- heat and poor air count for more.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Report = require "Weather/report"
local Outdoors = require "Weather/activities/outdoors"

local Run = { id = "run", title = "run", needsMarine = false, everywhere = true }

function Run.rate(hour)
    local card = Score.card(55)
    if Sky.isStorm(hour.code) then card:cap(15, "Thunderstorm: run another time.") end
    local feels = hour.feels or hour.temp
    if feels == nil then card:note("No temperature reading.")
    elseif feels < -5 then card:add(-20, "Bitter cold: cover your hands and ears.")
    elseif feels < 3 then card:add(-6, "Cold: layers you can take off.")
    elseif feels <= 18 then card:add(15, "Cool: good running weather.")
    elseif feels <= 24 then card:add(2, "Warm: take it a little easier.")
    elseif feels <= 29 then card:add(-15, "Hot: slow down and drink.")
    else card:cap(35, "Too hot to run hard.") end
    if (hour.dewPoint or 0) >= 18 then card:add(-8, "Muggy: sweat won't cool you.") end
    if (feels or 10) < 1 and (Sky.isWet(hour.code) or Sky.isSnow(hour.code)) then card:add(-20, "Icy paths.")
    elseif Sky.isHeavy(hour.code) then card:add(-15, "Heavy rain.")
    elseif Sky.isWet(hour.code) then card:add(-4, "Light rain: fine once you are moving.") end
    if (hour.wind or 0) > 9 then card:add(-10, "Strong wind: plan the headwind first.") end
    if hour.sunAltitude and hour.sunAltitude < -0.833 then card:note("After dark: wear something bright.") end
    Outdoors.rateAir(card, hour)
    return card
end

function Run.window(hour) local h = Sky.hour(hour.time) or 0 return h >= 6 and h <= 21 end

function Run.note(hour)
    return ("feels %s · dew point %s · wind %s"):format(Units.temp(hour.feels), Units.temp(hour.dewPoint), Units.wind(hour.wind))
end

function Run.metrics(hour, forecast)
    local list = {
        { label = "feels like", value = Units.temp(hour.feels), note = "air " .. Units.temp(hour.temp) },
        { label = "dew point", value = Units.temp(hour.dewPoint), note = (hour.dewPoint or 0) >= 18 and "muggy" or "comfortable" },
        Report.wind(hour),
        { label = "rain", value = Score.fmt(hour.rainChance, "%"), note = Units.rain(hour.rain) .. " this hour" },
        Report.sun(hour),
    }
    for index, metric in ipairs(Outdoors.airMetrics(hour, forecast)) do table.insert(list, 2 + index, metric) end
    return list
end

Run.charts = {
    { title = "feels like", unit = "temp", kind = "column", value = function(hour) return hour.feels or hour.temp or 0 end },
    { title = "dew point", unit = "temp", kind = "line", value = function(hour) return hour.dewPoint or 0 end },
}

return Run
