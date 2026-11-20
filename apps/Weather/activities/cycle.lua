-- Cycle, an everyday lens (W6): wind and wet roads matter most on a bike,
-- then the cold the ride's own wind adds, then the dark.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Report = require "Weather/report"

local Cycle = { id = "cycle", title = "cycle", needsMarine = false, everywhere = true }

function Cycle.rate(hour)
    local card = Score.card(55)
    if Sky.isStorm(hour.code) then card:cap(15, "Thunderstorm: stay off the bike.") end
    local wind, gust = hour.wind or 0, hour.gust or 0
    if gust >= 17 then card:cap(30, "Gusts strong enough to push you sideways.")
    elseif wind > 11 then card:add(-30, "Very windy: hard going into it.")
    elseif wind > 7 then card:add(-15, "Windy: plan the headwind first.")
    elseif wind < 4 then card:add(8, "Light wind.") end
    local feels = hour.feels or hour.temp
    if feels == nil then card:note("No temperature reading.")
    elseif feels < 3 then card:add(-12, "Cold: gloves and a windproof layer.")
    elseif feels < 8 then card:add(-4, "Cool: the ride feels colder.")
    elseif feels <= 24 then card:add(10, "Comfortable.")
    elseif feels > 30 then card:add(-15, "Hot: carry water.") end
    if (feels or 10) < 1 and (Sky.isWet(hour.code) or Sky.isSnow(hour.code)) then card:cap(30, "Icy roads.")
    elseif Sky.isHeavy(hour.code) then card:add(-25, "Heavy rain: poor grip and spray.")
    elseif Sky.isWet(hour.code) then card:add(-15, "Wet roads: brake early.") end
    if hour.sunAltitude and hour.sunAltitude < -0.833 then card:add(-8, "Dark: lights on.") end
    return card
end

function Cycle.window(hour) local h = Sky.hour(hour.time) or 0 return h >= 6 and h <= 21 end

function Cycle.note(hour)
    return ("wind %s %s · %s · rain %s"):format(Units.gust(hour.wind, hour.gust), Sky.compass(hour.windDir),
        Units.temp(hour.feels), Score.fmt(hour.rainChance, "%"))
end

function Cycle.metrics(hour)
    return {
        Report.wind(hour),
        { label = "feels like", value = Units.temp(hour.feels), note = "air " .. Units.temp(hour.temp) },
        { label = "rain", value = Score.fmt(hour.rainChance, "%"), note = Units.rain(hour.rain) .. " this hour" },
        Report.sun(hour),
    }
end

Cycle.charts = {
    { title = "wind", unit = "wind", kind = "column", value = function(hour) return hour.wind or 0 end },
    { title = "chance of rain", unit = "percent", kind = "column", min = 0, max = 100,
        value = function(hour) return hour.rainChance or 0 end },
}

return Cycle
