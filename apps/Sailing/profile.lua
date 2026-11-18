-- Sailing and kitesurf: is there a steady, usable wind? Sailors want enough
-- wind to move and not so much they reef or capsize; kiters need a band
-- of steady wind and never an offshore one, which carries a fallen rider
-- out to sea. Every spot says which way its shore faces (`facing`,
-- degrees toward the water); lakes have no waves to read.
local Sky = require "WeatherKit/sky"
local Score = require "WeatherKit/score"
local Report = require "WeatherKit/report"

local Sailing = {
    id = "sailing", title = "sailing", needsMarine = "optional",
    spots = {
        { id = "tarifa", title = "Tarifa", region = "Spain", lat = 36.01, lon = -5.61, facing = 210 },
        { id = "garda", title = "Lake Garda", region = "Italy", lat = 45.87, lon = 10.87, facing = 180, lake = true },
        { id = "spo", title = "Sankt Peter-Ording", region = "Germany", lat = 54.30, lon = 8.60, facing = 270 },
        { id = "cabarete", title = "Cabarete", region = "Dominican Republic", lat = 19.76, lon = -70.41, facing = 0 },
        { id = "crissy", title = "Crissy Field", region = "San Francisco Bay", lat = 37.81, lon = -122.46, facing = 0 },
    },
}

-- offshore, onshore or cross-shore for the spot's shore.
local function windTo(hour, spot)
    if not (hour.windDir and spot.facing) then return "cross-shore" end
    if Sky.angle(hour.windDir, spot.facing + 180) <= 45 then return "offshore" end
    if Sky.angle(hour.windDir, spot.facing) <= 45 then return "onshore" end
    return "cross-shore"
end
Sailing.windTo = windTo

-- How much stronger the gusts are than the wind; 1.0 is perfectly steady.
local function gustiness(hour)
    if not (hour.wind and hour.gust) or hour.wind < 1 then return 1 end
    return hour.gust / hour.wind
end

function Sailing.rate(hour, forecast)
    local card, spot = Score.card(40), forecast.spot or {}
    if Sky.isStorm(hour.code) then card:cap(5, "Lightning: get off the water.") end
    if not hour.isDay then card:cap(25, "Dark: nobody can see you if you need help.") end

    local wind = hour.wind or 0
    if wind < 3 then card:add(-25, "Too little wind to sail or fly a kite.")
    elseif wind < 5 then card:add(5, "Light wind: big kites and dinghies.")
    elseif wind <= 11 then card:add(25, "A good sailing and kiting breeze.")
    elseif wind <= 15 then card:add(0, "Strong wind: small kites, reefed sails, experts.")
    else card:cap(20, "Gale: stay ashore.") end

    local ratio = gustiness(hour)
    if wind >= 3 and ratio >= 1.6 then card:add(-15, "Very gusty: lulls and sudden pulls.")
    elseif wind >= 3 and ratio >= 1.35 then card:add(-6, "Gusty wind.")
    elseif wind >= 3 then card:add(6, "Steady wind.") end

    local relation = windTo(hour, spot)
    if wind >= 3 and relation == "offshore" then card:cap(30, "Offshore wind: it blows you away from the shore.")
    elseif wind >= 3 and relation == "cross-shore" then card:add(6, "Cross-shore wind: the safest for kiting.") end

    if hour.waveHeight and hour.waveHeight > 2.5 then card:add(-10, "Big waves at the shore break.") end
    if (hour.temp or 15) < 8 then card:add(-6, "Cold: wear a thick wetsuit or a drysuit.") end
    return card
end

function Sailing.window(hour) return hour.isDay end

function Sailing.note(hour, forecast)
    return ("%s gusts %s · %s %s"):format(Score.fmt(hour.wind, " m/s"), Score.fmt(hour.gust), windTo(hour, forecast.spot or {}),
        Sky.compass(hour.windDir))
end

function Sailing.metrics(hour, forecast)
    local spot, ratio = forecast.spot or {}, gustiness(hour)
    return {
        { label = "wind", value = Score.fmt(hour.wind, " m/s") .. " " .. Sky.compass(hour.windDir),
            note = ("%s · about %d knots"):format(windTo(hour, spot), math.floor((hour.wind or 0) * 1.944 + 0.5)) },
        { label = "gusts", value = Score.fmt(hour.gust, " m/s"),
            note = ratio >= 1.35 and "gusty" or "steady" },
        { label = "waves", value = spot.lake and "lake" or Score.fmt(hour.waveHeight, " m", 1),
            note = spot.lake and "no sea swell" or "period " .. Score.fmt(hour.wavePeriod, " s") },
        { label = "water", value = Score.fmt(hour.seaTemp, "°"), note = "air " .. Score.fmt(hour.temp, "°") },
        Report.sun(hour),
    }
end

return Sailing
