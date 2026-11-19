-- Outdoors, the everyday activity: is it nice to be outside? Comfortable temperature, a
-- dry sky, a gentle wind and some sun make a good hour.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Report = require "Weather/report"

local Weather = {
    id = "outdoors", title = "outdoors", needsMarine = false,
}

function Weather.rate(hour)
    local card = Score.card(55)
    if Sky.isStorm(hour.code) then card:cap(15, "Thunderstorm: stay indoors.") end
    local feels = hour.feels or hour.temp
    if feels == nil then card:note("No temperature reading.")
    elseif feels < 0 then card:add(-25, "Freezing: wrap up warm.")
    elseif feels < 10 then card:add(-10, "Chilly: bring a jacket.")
    elseif feels <= 26 then card:add(12, "Comfortable temperature.")
    elseif feels <= 32 then card:add(-6, "Warm: find some shade.")
    else card:add(-20, "Hot: drink water and stay out of the sun.") end
    if Sky.isSnow(hour.code) then card:add(Sky.isHeavy(hour.code) and -25 or -10, "Snow: boots and a warm coat.")
    elseif Sky.isHeavy(hour.code) then card:add(-25, "Heavy rain.")
    elseif Sky.isWet(hour.code) then card:add(-12, "Wet: take an umbrella.")
    elseif (hour.rainChance or 0) >= 50 then card:add(-6, "Rain is likely.") end
    if (hour.wind or 0) > 10 then card:add(-15, "Windy.") elseif (hour.wind or 0) < 5 then card:add(4, "Gentle wind.") end
    if hour.isDay and (hour.cloud or 100) < 50 and not Sky.isWet(hour.code) then card:add(6, "Sunny spells.") end
    if (hour.uv or 0) >= 8 then card:note("Very high UV: use sunscreen.") end
    return card
end

function Weather.window(hour) return hour.isDay end

function Weather.note(hour)
    return ("%s · rain %s · wind %s"):format(Score.fmt(hour.temp, "°"), Score.fmt(hour.rainChance, "%"),
        Score.fmt(hour.wind, " m/s"))
end

function Weather.metrics(hour, forecast)
    return {
        { label = "feels like", value = Score.fmt(hour.feels, "°"), note = "air " .. Score.fmt(hour.temp, "°") },
        { label = "rain", value = Score.fmt(hour.rainChance, "%"), note = Score.fmt(hour.rain, " mm", 1) .. " this hour" },
        Report.wind(hour),
        { label = "humidity", value = Score.fmt(hour.humidity, "%") },
        { label = "uv index", value = Score.fmt(hour.uv) },
        Report.pressure(hour),
        Report.sun(hour),
        Report.moon(forecast),
    }
end

-- The two charts on the activity's "today" view, over the next 24 hours.
Weather.charts = {
    { title = "temperature °C", kind = "column", value = function(hour) return hour.temp or 0 end },
    { title = "chance of rain %", kind = "column", min = 0, max = 100, value = function(hour) return hour.rainChance or 0 end },
}

return Weather
