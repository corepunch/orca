-- Everyday weather: is it nice to be outside? Comfortable temperature, a
-- dry sky, a gentle wind and some sun make a good hour.
local Sky = require "WeatherKit/sky"
local Score = require "WeatherKit/score"
local Report = require "WeatherKit/report"

local Weather = {
    id = "weather", title = "weather", needsMarine = false,
    spots = {
        { id = "san-francisco", title = "San Francisco", region = "California", lat = 37.7749, lon = -122.4194 },
        { id = "new-york", title = "New York", region = "New York", lat = 40.7128, lon = -74.0060 },
        { id = "berlin", title = "Berlin", region = "Germany", lat = 52.5200, lon = 13.4050 },
        { id = "london", title = "London", region = "United Kingdom", lat = 51.5072, lon = -0.1276 },
        { id = "tokyo", title = "Tokyo", region = "Japan", lat = 35.6762, lon = 139.6503 },
        { id = "sydney", title = "Sydney", region = "Australia", lat = -33.8688, lon = 151.2093 },
    },
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

return Weather
