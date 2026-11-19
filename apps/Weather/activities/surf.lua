-- Surf: is it worth paddling out? Surfers read swell size and period, the
-- wind against the beach and the light. Every spot says which way its beach
-- faces (`facing`, degrees toward the sea), because the same wind is clean
-- offshore at one beach and blown-out onshore at the next.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Report = require "Weather/report"

local Surf = {
    id = "surf", title = "surf", needsMarine = true,
}

local function swell(hour) return hour.swellHeight or hour.waveHeight, hour.swellPeriod or hour.wavePeriod,
    hour.swellDir or hour.waveDir end

-- offshore, onshore or cross-shore for the spot.
local function windTo(hour, spot)
    if not (hour.windDir and spot.facing) then return "cross-shore" end
    if Sky.angle(hour.windDir, spot.facing + 180) <= 45 then return "offshore" end
    if Sky.angle(hour.windDir, spot.facing) <= 45 then return "onshore" end
    return "cross-shore"
end

function Surf.rate(hour, forecast)
    local card, spot = Score.card(40), forecast.spot or {}
    if Sky.isStorm(hour.code) then card:cap(5, "Lightning: get out of the water.") end
    if not hour.isDay then card:cap(30, "Dark: wait for first light.") end

    local height, period, direction = swell(hour)
    if height == nil then card:cap(20, "No swell data for this spot.")
    elseif height < 0.4 then card:add(-30, "Flat: under knee high.")
    elseif height < 0.8 then card:add(5, "Small waves: good for longboards and beginners.")
    elseif height <= 2.5 then card:add(25, "Waist high to overhead: a proper swell.")
    elseif height <= 4 then card:add(10, "Big swell: experienced surfers only.")
    else card:cap(30, "Huge swell: too heavy for most spots.") end

    if period then
        if period < 7 then card:add(-12, "Short period: weak, choppy windswell.")
        elseif period < 10 then card:add(2, "Medium-period swell.")
        else card:add(math.floor(Score.curve(period, { { 10, 10 }, { 15, 15 } })),
            "Long-period groundswell: powerful, lined-up waves.") end
    end

    if direction and spot.facing and Sky.angle(direction, spot.facing) > 100 then
        card:add(-15, "The swell is heading away from this beach.")
    end

    local wind, relation = hour.wind or 0, windTo(hour, spot)
    if wind < 2 then card:add(12, "Glassy: almost no wind.")
    elseif relation == "offshore" then
        if wind <= 8 then card:add(10, "Offshore wind grooms the wave faces.")
        else card:add(-8, "Strong offshore: hard to paddle into waves.") end
    elseif relation == "onshore" then card:add(-math.floor(wind * 2.5 + 0.5), "Onshore wind blows the waves out.")
    else card:add(-math.floor(wind + 0.5), "Cross-shore wind adds chop.") end
    return card
end

function Surf.window(hour) return hour.isDay end

function Surf.note(hour, forecast)
    local height, period = swell(hour)
    return ("%s · %s · %s %s"):format(Score.fmt(height, " m", 1), Score.fmt(period, " s"),
        windTo(hour, forecast.spot or {}), Score.fmt(hour.wind, " m/s"))
end

local function wetsuit(temp)
    if temp == nil then return nil end
    if temp < 12 then return "5/4 mm, boots and hood" end
    if temp < 15 then return "4/3 mm wetsuit" end
    if temp < 18 then return "3/2 mm wetsuit" end
    if temp < 22 then return "spring suit" end
    return "boardshorts"
end

function Surf.metrics(hour, forecast)
    local height, period, direction = swell(hour)
    local wind = Report.wind(hour)
    wind.note = windTo(hour, forecast.spot or {}) .. " · " .. wind.note
    return {
        { label = "swell", value = Score.fmt(height, " m", 1) .. " · " .. Score.fmt(period, " s"),
            note = "from " .. Sky.compass(direction) },
        { label = "waves", value = Score.fmt(hour.waveHeight, " m", 1), note = "wind sea and swell together" },
        wind,
        { label = "water", value = Score.fmt(hour.seaTemp, "°"), note = wetsuit(hour.seaTemp) },
        Report.sun(hour),
        Report.moon(forecast),
    }
end

-- The two charts on the activity's "today" view, over the next 24 hours.
Surf.charts = {
    { title = "swell m", kind = "column", value = function(hour) return hour.swellHeight or hour.waveHeight or 0 end },
    { title = "wind m/s", kind = "line", value = function(hour) return hour.wind or 0 end },
}

return Surf
