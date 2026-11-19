-- Hiking and mountains: is it a safe, pleasant hour on the trail? Hikers
-- watch for thunder (and the storm energy that brings it in the
-- afternoon), wind on the ridges, cold, fog and ice. Open-Meteo forecasts
-- for each spot's own height, so a summit gets summit weather.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Report = require "Weather/report"

local Hiking = {
    id = "hiking", title = "hiking", needsMarine = false,
}

function Hiking.rate(hour, forecast)
    local card = Score.card(55)
    if Sky.isStorm(hour.code) then card:cap(5, "Thunderstorm: get below the ridges.") end
    if not hour.isDay then card:cap(25, "Dark: hike in daylight.") end

    local cape = hour.cape or 0
    if cape >= 1000 then card:add(-15, "Lots of storm energy: thunder can build fast.")
    elseif cape >= 300 then card:add(-5, "Some storm energy: watch for towering clouds.") end

    if Sky.isSnow(hour.code) then card:add(-12, "Snow on the trail.")
    elseif Sky.isHeavy(hour.code) then card:add(-25, "Heavy rain: slippery rock and swollen streams.")
    elseif Sky.isWet(hour.code) then card:add(-12, "Wet: rain gear on.") end

    local wind = hour.gust or hour.wind or 0
    if wind > 20 then card:cap(30, "Gusts strong enough to knock you over on ridges.")
    elseif wind > 12 then card:add(-12, "Strong gusts on exposed ridges.") end

    local feels = hour.feels or hour.temp
    if feels == nil then card:note("No temperature reading.")
    elseif feels < -10 then card:add(-15, "Bitter cold: frostbite risk.")
    elseif feels < 0 then card:add(-5, "Below freezing: layers, hat and gloves.")
    elseif feels >= 5 and feels <= 22 then card:add(12, "Good walking temperature.")
    elseif feels > 28 then card:add(-12, "Hot: start early and carry extra water.") end

    if (hour.visibility or 20000) < 1000 then card:add(-15, "Fog: the trail is hard to follow.")
    elseif (hour.visibility or 20000) < 5000 then card:add(-5, "Hazy: short views.")
    elseif hour.isDay and (hour.cloud or 100) < 30 then card:add(6, "Clear views from the top.") end

    if forecast.elevation and hour.freezingLevel and hour.freezingLevel < forecast.elevation then
        card:add(-6, "Freezing level below you: watch for ice on rocks.")
    end
    return card
end

function Hiking.window(hour) return hour.isDay end

function Hiking.note(hour)
    return ("%s · gusts %s · rain %s"):format(Score.fmt(hour.feels, "°"), Score.fmt(hour.gust, " m/s"),
        Score.fmt(hour.rainChance, "%"))
end

function Hiking.metrics(hour, forecast)
    local height = forecast.elevation
    return {
        { label = "height", value = Score.fmt(height, " m"), note = "the forecast is for this height" },
        { label = "freezing level", value = Score.fmt(hour.freezingLevel, " m"),
            note = (height and hour.freezingLevel and hour.freezingLevel < height) and "below you: ice likely"
                or "above you" },
        { label = "feels like", value = Score.fmt(hour.feels, "°"), note = "air " .. Score.fmt(hour.temp, "°") },
        Report.wind(hour),
        { label = "visibility", value = Score.fmt((hour.visibility or 0) / 1000, " km"), note = Sky.describe(hour.code) },
        { label = "storm energy", value = Score.fmt(hour.cape, " J/kg"),
            note = (hour.cape or 0) >= 1000 and "thunder likely" or (hour.cape or 0) >= 300 and "some risk" or "low" },
        Report.sun(hour),
    }
end

-- The two charts on the activity's "today" view, over the next 24 hours.
Hiking.charts = {
    { title = "feels like °C", kind = "line", value = function(hour) return hour.feels or hour.temp or 0 end },
    { title = "chance of rain %", kind = "column", min = 0, max = 100, value = function(hour) return hour.rainChance or 0 end },
}

return Hiking
