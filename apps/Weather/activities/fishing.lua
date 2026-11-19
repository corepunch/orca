-- Fishing: is it a good hour to fish here? Anglers read the barometer, the
-- wind on the water, the light and the moon. Each rule below is one piece
-- of that lore with the reason an angler would give.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Report = require "Weather/report"

local Fishing = {
    id = "fishing", title = "fishing", needsMarine = false,
}

function Fishing.rate(hour, forecast)
    local card = Score.card(50)
    if Sky.isStorm(hour.code) then card:cap(10, "Lightning risk: stay off the water.") end
    if (hour.gust or 0) > 15 then card:cap(35, "Gusts over 15 m/s: unsafe in a small boat.") end

    local trend = hour.pressureTrend or 0
    if trend <= -3 then card:add(-6, "Pressure dropping fast: a front is close.")
    elseif trend <= -0.5 then card:add(12, "Pressure falling slowly: fish feed before the front.")
    elseif trend < 0.5 then card:add(6, "Steady pressure: fish keep their routine.")
    elseif trend < 1.5 then card:add(-4, "Pressure creeping up: bites slow down.")
    else card:add(-12, "Pressure rising fast after a front: fish go deep.") end

    local wind = hour.wind or 0
    if wind < 1.5 then card:add(-4, "Flat calm: fish can see you and the line.")
    elseif wind <= 5 then card:add(8, "Light breeze ripples the water and hides you.")
    elseif wind <= 8 then card:add(-2, "Fresh breeze: fish the sheltered bank.")
    else card:add(math.floor(Score.curve(wind, { { 8, -8 }, { 12, -25 } })), "Strong wind: hard casting, rough water.") end

    local twilight = Score.twilight(hour)
    if twilight <= 60 then card:add(15, "Dawn or dusk: the best feeding hour.")
    elseif twilight <= 120 then card:add(7, "Close to dawn or dusk.")
    elseif not hour.isDay then card:add(-10, "Dark: fewer bites, harder to see.") end

    if hour.isDay and (hour.cloud or 0) < 20 and (hour.uv or 0) >= 6 then
        card:add(-8, "Bright sun: fish hide in deep shade.")
    elseif hour.isDay and (hour.cloud or 0) >= 40 and (hour.cloud or 0) <= 90 then
        card:add(5, "Clouds take the glare off the water.")
    end

    if Sky.isSnow(hour.code) then card:add(Sky.isHeavy(hour.code) and -15 or -6, "Snowing: cold hands, poor visibility.")
    elseif Sky.isHeavy(hour.code) then card:add(-12, "Heavy rain muddies the water.")
    elseif Sky.isWet(hour.code) then card:add(4, "Light rain stirs the surface and washes food in.") end

    local moon = Sky.moon(forecast.epoch)
    if moon.daysFromSyzygy <= 2 then card:add(6, ("Near the %s: a strong feeding day."):format(
        moon.phase > 0.25 and moon.phase < 0.75 and "full moon" or "new moon")) end

    local temp = hour.temp or 10
    if temp < 2 then card:add(-8, "Near freezing: fish are sluggish.")
    elseif temp > 30 then card:add(-6, "Hot: fish go deep. Go early or late.") end
    return card
end

-- Anglers fish daylight and the hour or so either side of it.
function Fishing.window(hour) return hour.isDay or Score.twilight(hour) <= 90 end

function Fishing.note(hour)
    local trend = hour.pressureTrend or 0
    -- Words, not arrows: the phone font has no arrow glyphs.
    local arrow = trend > 0.5 and "rising" or trend < -0.5 and "falling" or "steady"
    return ("wind %s · %s %s · rain %s"):format(Score.fmt(hour.wind, " m/s"), Score.fmt(hour.pressure, " hPa"), arrow,
        Score.fmt(hour.rainChance, "%"))
end

function Fishing.metrics(hour, forecast)
    return {
        Report.pressure(hour),
        Report.wind(hour),
        Report.moon(forecast),
        Report.sun(hour),
        { label = "cloud cover", value = Score.fmt(hour.cloud, "%") },
        { label = "air", value = Score.fmt(hour.temp, "°"), note = "feels like " .. Score.fmt(hour.feels, "°") },
    }
end

return Fishing
