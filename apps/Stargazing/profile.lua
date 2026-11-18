-- Stargazing: will the night sky be dark and clear? Stargazers need a
-- fully dark sky (well after sunset), no clouds, the moon down or thin,
-- dry air and little wind to shake a telescope. Its best windows are at
-- night, unlike every other profile.
local Sky = require "WeatherKit/sky"
local Score = require "WeatherKit/score"
local Report = require "WeatherKit/report"

local Stargazing = {
    id = "stargazing", title = "stars", needsMarine = false,
    spots = {
        { id = "atacama", title = "San Pedro de Atacama", region = "Chile", lat = -22.91, lon = -68.20 },
        { id = "mauna-kea", title = "Mauna Kea", region = "Hawaii", lat = 19.76, lon = -155.46 },
        { id = "tekapo", title = "Lake Tekapo", region = "New Zealand", lat = -44.00, lon = 170.48 },
        { id = "cherry-springs", title = "Cherry Springs", region = "Pennsylvania", lat = 41.66, lon = -77.82 },
        { id = "brecon", title = "Brecon Beacons", region = "Wales", lat = 51.88, lon = -3.44 },
    },
}

local function moonAt(hour, forecast)
    local moon = Sky.moon(hour.epoch or forecast.epoch)
    moon.up = Sky.moonUp(moon.phase, Sky.hour(hour.time) or 0)
    return moon
end

function Stargazing.rate(hour, forecast)
    local card = Score.card(50)
    if Sky.isStorm(hour.code) then card:cap(5, "Thunderstorm.") end
    if hour.isDay then card:cap(5, "Daylight.")
    elseif Score.twilight(hour) <= 90 then card:cap(35, "Twilight: the sky is not fully dark yet.") end

    local cloud = hour.cloud or 100
    if cloud < 10 then card:add(25, "Clear sky.")
    elseif cloud < 30 then card:add(10, "A few clouds.")
    elseif cloud < 60 then card:add(-15, "Partly cloudy: stars only in the gaps.")
    else card:add(-35, "Cloudy: nothing to see.") end
    if Sky.isWet(hour.code) or Sky.isSnow(hour.code) then card:add(-15, "Rain or snow: keep the telescope covered.") end

    local moon = moonAt(hour, forecast)
    if moon.up and moon.lit > 0.6 then card:add(-20, "Bright moon up: faint stars wash out.")
    elseif moon.up and moon.lit > 0.25 then card:add(-8, "The moon is up.")
    else card:add(10, "Moon down or thin: a dark sky.") end

    if (hour.humidity or 0) >= 90 then card:add(-6, "Damp air: dew on lenses.") end
    if (hour.visibility or 20000) < 10000 then card:add(-8, "Hazy air dims the stars.") end
    if (hour.wind or 0) > 8 then card:add(-6, "Windy: a shaky telescope.") end
    if (hour.temp or 10) < 0 then card:note("Below freezing: dress warmly.") end
    return card
end

-- The card shows the first dark hour from now: tonight, or right now.
function Stargazing.focus(forecast)
    for index = forecast.now, #forecast.hours do
        if Stargazing.window(forecast.hours[index]) then return index end
    end
    return forecast.now
end

-- Dark hours only: more than an hour and a half from sunset or sunrise.
function Stargazing.window(hour) return not hour.isDay and Score.twilight(hour) > 90 end

function Stargazing.note(hour, forecast)
    local moon = moonAt(hour, forecast)
    return ("clouds %s · moon %s · %s"):format(Score.fmt(hour.cloud, "%"), moon.up and "up" or "down",
        Score.fmt(hour.temp, "°"))
end

function Stargazing.metrics(hour, forecast)
    local moon = moonAt(hour, forecast)
    return {
        { label = "clouds", value = Score.fmt(hour.cloud, "%"),
            note = ("low %s · middle %s · high %s"):format(Score.fmt(hour.cloudLow, "%"), Score.fmt(hour.cloudMid, "%"),
                Score.fmt(hour.cloudHigh, "%")) },
        { label = "moon", value = moon.name, note = Score.fmt(moon.lit * 100, "% lit · ") .. (moon.up and "about up now"
            or "about below the horizon") },
        { label = "humidity", value = Score.fmt(hour.humidity, "%"), note = "dew point " .. Score.fmt(hour.dewPoint, "°") },
        { label = "visibility", value = Score.fmt((hour.visibility or 0) / 1000, " km") },
        Report.wind(hour),
        Report.sun(hour),
    }
end

return Stargazing
