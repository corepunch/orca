-- Hiking and mountains: is it a safe, pleasant hour on the trail? Hikers
-- watch for thunder (and the storm energy that brings it in the
-- afternoon), wind on the ridges, cold, fog and ice. The place's forecast
-- is often a town or valley; Hikers' Weather also asks for the trailhead
-- and the top at their own heights (editions/hiking/peaks.lua) and rates
-- the harder of the two. Avalanche danger, fires, measured snow and park
-- alerts come from existing services (editions/hiking/services.lua) as
-- cards; they never change the rating.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Report = require "Weather/report"
local Sources = require "Weather/editions/hiking/sources"
local Trail = require "Weather/editions/hiking/trail"
local Services = require "Weather/editions/hiking/services"
local Avalanche = require "Weather/editions/hiking/avalanche"

local Hiking = {
    id = "hiking", title = "hiking", needsMarine = false,
    hubBackground = "Weather/editions/hiking/alpine-trail.jpg",
}

-- Snow, ice and heat columns for the place, three days back for what fell
-- (HI5), and 16 days ahead (HI6).
Hiking.fields = {
    hourly = { "lifted_index", "snowfall", "snow_depth", "wet_bulb_temperature_2m" },
    pastDays = 3, forecastDays = 16,
}
-- Open-Meteo's points, models and air and the NWS warnings, then the
-- other services (editions/hiking/services.lua), each optional.
Hiking.sources = {}
for _, list in ipairs { Sources.list, Services.list } do
    for _, source in ipairs(list) do Hiking.sources[#Hiking.sources + 1] = source end
end
Hiking.attribution = Services.attribution

local function hourText(v)
    local h = math.floor(v)
    return Units.clock(("%02d:%02d"):format(h, math.floor((v - h) * 60 + 0.5)))
end

-- What the place editor asks of a hike (P3): the heights where it starts and
-- tops out. Sources.peak reads them before the edition's table.
local function height(value) return value and Units.height(value, 0) or "not set" end
local function peakHeight(which)
    return function(place) local peak = Sources.peak(place); return peak and peak[which] and peak[which].elevation end
end
Hiking.placeFields = {
    { key = "summit", title = "height of the top", kind = "number", min = 0, max = 8850, step = 50, start = 1500,
        format = height, default = peakHeight("top"), detail = "with the trailhead's height, the top and the start get their own forecast" },
    { key = "trailhead", title = "height of the trailhead", kind = "number", min = 0, max = 6000, step = 50, start = 500,
        format = height, default = peakHeight("start"), detail = "where you set off" },
}

-- The planned hike, for the turnaround (HI4).
Hiking.settings = {
    { key = "start", title = "start time", kind = "number", default = 8, min = 3, max = 14, step = 0.5,
        format = hourText, detail = "when you set off from the trailhead" },
    { key = "up", title = "time to the top", kind = "number", default = 3, min = 0.5, max = 12, step = 0.5,
        format = function(v) return Trail.duration(v * 60) end },
    { key = "down", title = "time back down", kind = "number", default = 2, min = 0.5, max = 10, step = 0.5,
        format = function(v) return Trail.duration(v * 60) end },
}

-- One end of the hike: `at` is the hour itself, or its `top` or `start`
-- (which read through to the hour for what they lack).
local function rateAt(at, forecast, height)
    local card = Score.card(55)
    if Sky.isStorm(at.code) then card:cap(5, "Thunderstorm: get below the ridges.") end
    if not at.isDay then card:cap(25, "Dark: hike in daylight.") end

    local thunder = not Sky.isStorm(at.code) and Trail.thunder(at)
    if thunder == "likely" then card:add(-20, "Thunder likely about now (estimate): stay off ridges and tops.")
    elseif thunder == "possible" then card:add(-10, "Storm energy building: watch for towering clouds.")
    elseif (at.cape or 0) >= 300 then card:add(-5, "Some storm energy: watch for towering clouds.") end

    if Sky.isSnow(at.code) then card:add(-12, "Snow on the trail.")
    elseif Sky.isHeavy(at.code) then card:add(-25, "Heavy rain: slippery rock and swollen streams.")
    elseif Sky.isWet(at.code) then card:add(-12, "Wet: rain gear on.") end

    -- Free-air wind at the top's height reads high for a sheltered top,
    -- so it counts like a gust, not more.
    local gust, ridge = at.gust or at.wind or 0, at.ridge or 0
    local wind = math.max(gust, ridge)
    local what = ridge > gust and "Ridge wind (estimate)" or "Gusts"
    if wind > 20 then card:cap(30, what .. " strong enough to knock you over on ridges.")
    elseif wind > 12 then card:add(-12, what .. " strong on exposed ridges.") end

    local feels = at.feels or at.temp
    if feels == nil then card:note("No temperature reading.")
    elseif feels < -10 then card:add(-15, "Bitter cold: frostbite risk.")
    elseif feels < 0 then card:add(-5, "Below freezing: layers, hat and gloves.")
    elseif feels >= 5 and feels <= 22 then card:add(12, "Good walking temperature.")
    elseif feels > 28 then card:add(-12, "Hot: start early and carry extra water.") end

    local wetBulb = at.wet_bulb_temperature_2m
    if wetBulb and wetBulb >= 26 then card:cap(30, "Heat stress: too hot and humid to hike hard.")
    elseif wetBulb and wetBulb >= 24 then card:add(-12, "Hot and humid: rest often and drink.") end

    if (at.visibility or 20000) < 1000 then card:add(-15, "Fog: the trail is hard to follow.")
    elseif (at.visibility or 20000) < 5000 then card:add(-5, "Hazy: short views.")
    elseif at.isDay and (at.cloud or 100) < 30 then card:add(6, "Clear views from the top.") end

    if height and at.freezingLevel and at.freezingLevel < height then
        card:add(-6, "Freezing level below you: watch for ice on rocks.")
    end
    if (at.snow_depth or 0) >= 0.05 and not Sky.isSnow(at.code) then
        card:add(-6, "Snow lying on the trail: spikes or poles may help.")
    end

    local aqi = at.aqi
    if aqi and aqi > 150 then card:add(-20, "Unhealthy air (smoke or dust): shorten the hike.")
    elseif aqi and aqi > 100 then card:add(-8, "Air unhealthy for sensitive lungs.") end
    return card
end

-- Official warnings that cover the hour.
local function warned(card, hour, forecast)
    for _, alert in ipairs(Trail.alerts(forecast) or {}) do
        local from, to = alert.onset or 0, alert.ends or math.huge
        if hour.epoch and hour.epoch + 3600 > from and hour.epoch < to
            and (alert.severity == "Severe" or alert.severity == "Extreme") then
            card:add(-15, "Official warning: " .. alert.event .. ".")
            return
        end
    end
end

function Hiking.rate(hour, forecast)
    local card
    if hour.top and hour.start then
        -- The harder end of the hike decides; ties go to the top.
        local top = rateAt(hour.top, forecast, hour.top.elevation)
        local start = rateAt(hour.start, forecast, hour.start.elevation)
        local atTop = top:result().score <= start:result().score
        card = atTop and top or start
        card:note(atTop and "Rated for the top, the harder end." or "Rated for the start, the harder end.")
    else
        card = rateAt(hour, forecast, forecast.elevation)
    end
    warned(card, hour, forecast)
    local word = forecast.hiking and forecast.hiking.confidence and forecast.hiking.confidence[hour.date]
    if word and word.word == "uncertain" then card:note("The models disagree about this day: look again nearer the time.") end
    return card
end

function Hiking.window(hour) return hour.isDay end

function Hiking.note(hour)
    local at = hour.top or hour
    return ("%s · gusts %s · rain %s"):format(Units.temp(at.feels), Units.wind(math.max(at.gust or 0, at.ridge or 0)),
        Score.fmt(hour.rainChance, "%"))
end

local function stormCard(forecast, day, today)
    local storm = Trail.storm(forecast, day)
    local when = today and "today" or "tomorrow"
    if not storm then
        return { id = "storm", title = "storm clock · " .. when, value = "no thunder signs", tone = "good",
            detail = "estimate from storm energy and showers · mountain weather changes fast" }
    end
    local at = Units.clock(storm.hour.time)
    local estimate = (storm.hour.lightning or 0) >= Trail.LIGHTNING and "model estimate (lightning potential)"
        or "estimate"
    if storm.now then
        return { id = "storm", title = "storm clock · now", value = "thunder " .. storm.level, tone = "poor",
            detail = estimate .. " · get below the ridges and off the top" }
    end
    return { id = "storm", title = "storm clock · " .. when, value = "about " .. at,
        tone = storm.level == "likely" and "poor" or "fair",
        detail = ("%s: thunder %s from about %s · be off the summit by %s"):format(estimate, storm.level, at,
            Trail.clock(storm.offBy)) }
end

local function turnCard(forecast)
    local plan = Trail.plan(forecast)
    if not plan then return nil end
    local title = "turnaround · " .. (plan.today and "today" or "tomorrow")
    if plan.dark then
        return { id = "turnaround", title = title, value = "no daylight", tone = "poor",
            detail = "the sun stays below the horizon all day" }
    end
    local light = plan.light.always and "light all night"
        or ("light %s–%s"):format(Trail.clock(plan.light.first), Trail.clock(plan.light.last))
    local parts = { light }
    if plan.left then parts[#parts + 1] = Trail.duration(plan.left) .. " left" end
    local tone = "good"
    if plan.late then
        tone = "poor"
        parts[#parts + 1] = ("you'd reach the top about %s, after it: start earlier or shorten the hike")
            :format(Trail.clock(plan.top))
    else
        if plan.turn - plan.top < 30 then tone = "fair" end
        parts[#parts + 1] = ("start %s · top about %s · down about %s"):format(Trail.clock(plan.start),
            Trail.clock(plan.top), Trail.clock(plan.back))
    end
    if plan.why == "storm" then parts[#parts + 1] = "off the top before the storm clock" end
    if plan.beforeLight then parts[#parts + 1] = "you start before first light: take a headtorch" end
    return { id = "turnaround", title = title, value = "turn by " .. Trail.clock(plan.turn), tone = tone,
        detail = table.concat(parts, " · ") }
end

local function trailCard(forecast, place)
    local c = Trail.conditions(forecast)
    local measured, measuredCm = Services.measuredSnow(forecast, Sources.peak(place))
    local parts, value, tone = {}, nil, "good"
    local function say(text, word, worse)
        parts[#parts + 1] = text
        if word and not value then value = word end
        if worse and tone ~= "poor" then tone = worse end
    end
    local where = forecast.hiking and forecast.hiking.top and " at the top" or ""
    if c.snowDepth >= 0.05 then say("about " .. Trail.snow(c.snowDepth * 100) .. " of snow" .. where, "snow" .. where,
        c.snowDepth >= 0.3 and "poor" or "fair") end
    -- A station's reading beside the modelled depth (SV-HI4).
    if measured then say(measured, measuredCm >= 5 and "snow measured nearby" or nil, measuredCm >= 5 and "fair" or nil) end
    if c.snowfall >= 1 then say("about " .. Trail.snow(c.snowfall) .. " fell in 3 days", "fresh snow", "fair") end
    if c.thaw then say("frozen early, thawing from about " .. Units.clock(c.thaw.time), "icy early", "fair") end
    -- Rain alone: snow (cm, Open-Meteo's 7:1) is taken out of the precipitation.
    if c.rain24 >= 20 then say(Units.rain(c.rain24, 0) .. " of rain in 24 h: streams high", "streams high", "fair")
    elseif c.rain72 >= 5 then say(Units.rain(c.rain72, 0) .. " of rain in 3 days", "wet underfoot", "fair") end
    if c.wetBulb and c.wetBulb >= 24 then say("hot and humid: heat stress", "heat", "poor")
    elseif c.hot and c.hot > 30 then say("feels " .. Units.temp(c.hot) .. ": carry extra water", "hot", "fair") end
    if c.uv >= 6 then say(("uv %d, stronger up high and on snow"):format(math.floor(c.uv + 0.5))) end
    if #parts == 0 then parts[1] = "no snow and little rain lately" end
    return { id = "trail", title = "trail conditions", value = value or "dry trail", tone = tone,
        detail = table.concat(parts, " · ") }
end

local SURE = { sure = "good", likely = "fair", uncertain = "info" }
local function span(run)
    return run.from == run.to and Sky.weekday(run.from) or Sky.weekday(run.from) .. "–" .. Sky.weekday(run.to)
end

local function confidenceCard(forecast)
    local runs = Trail.confidence(forecast)
    if not runs then return nil end
    local first = runs[1]
    local today = Trail.now(forecast).date
    local value = #runs == 1 and first.word .. " all " .. (#forecast.days - (forecast.today or 1) + 1 > 7 and "fortnight"
        or "week") or first.to == today and first.word .. " today" or first.word .. " to " .. Sky.weekday(first.to)
    local parts = {}
    for index = 2, math.min(#runs, 4) do parts[#parts + 1] = runs[index].word .. " " .. span(runs[index]) end
    parts[#parts + 1] = ("%d models compared"):format(first.models)
    return { id = "confidence", title = "do the models agree?", value = value, tone = SURE[first.word] or "info",
        detail = table.concat(parts, " · ") }
end

local function pointsCard(forecast, place)
    local now, peak = Trail.now(forecast), Sources.peak(place)
    local top, start = now.top, now.start
    if not (peak and top and start) then return nil end
    local ridge = top.ridge and (" · ridge wind about %s (open-air estimate)"):format(Units.wind(top.ridge)) or ""
    return { id = "points", title = "top · start", value = Units.temp(top.feels) .. " · " .. Units.temp(start.feels),
        tone = "info",
        detail = ("%s %s: gusts %s%s · %s %s: gusts %s"):format(peak.top.name, Units.height(top.elevation, 0),
            Units.wind(top.gust), ridge, peak.start.name, Units.height(start.elevation, 0), Units.wind(start.gust)) }
end

local function alertCard(forecast)
    local active = Trail.alerts(forecast)
    if not (active and active[1]) then return nil end
    local alert = active[1]
    local parts = { alert.sender or "National Weather Service" }
    if alert.ends then parts[#parts + 1] = "until " .. Sky.weekday(os.date("!%Y-%m-%d", alert.ends + forecast.offset))
        .. " " .. Units.clock(os.date("!%H:%M", alert.ends + forecast.offset)) end
    if #active > 1 then parts[#parts + 1] = ("%d more"):format(#active - 1) end
    return { id = "warning", title = "official warning", value = alert.event:lower(),
        tone = (alert.severity == "Severe" or alert.severity == "Extreme") and "poor" or "fair",
        detail = table.concat(parts, " · ") }
end

-- The centre's own rating when its service answered (SV-HI1, SV-HI2): a
-- current rating always shows; "no current rating", a failed bulletin or
-- the plain link only when there is snow on the mountain.
local function avalancheCard(forecast, place)
    local peak = Sources.peak(place)
    if not (peak and peak.avalanche) then return nil end
    local result, missing = Services.avalanche(forecast, peak), Services.avalancheMissing(forecast, peak)
    if Avalanche.current(result, forecast) then return Avalanche.card(result, peak, forecast) end
    local c = Trail.conditions(forecast)
    local _, measured = Services.measuredSnow(forecast, peak)
    if c.snowDepth < 0.1 and c.snowfall < 10 and (measured or 0) < 10 then return nil end
    if result or missing then return Avalanche.card(result, peak, forecast, missing) end
    return { id = "avalanche", title = "avalanche", value = peak.avalanche[1], tone = "fair",
        detail = peak.avalanche[2] .. " · snow on the mountain: read the bulletin before you go" }
end

local function airCard(forecast)
    local worst
    for index = forecast.now, math.min(#forecast.hours, forecast.now + 23) do
        local hour = forecast.hours[index]
        if hour.aqi and (not worst or hour.aqi > worst.aqi) then worst = hour end
    end
    if not (worst and worst.aqi > 100) then return nil end
    local when = (worst.date ~= Trail.now(forecast).date and "tomorrow " or "") .. Units.clock(worst.time)
    return { id = "air", title = "smoke and air", value = worst.aqi > 150 and "unhealthy air" or "poor air",
        tone = worst.aqi > 150 and "poor" or "fair",
        detail = ("US AQI about %d around %s · PM2.5 %s µg/m³ · %s"):format(math.floor(worst.aqi + 0.5), when,
            Score.fmt(worst.pm25), worst.aqi > 150 and "shorten the hike" or "sensitive lungs: take it easy") }
end

-- The big answers, most urgent first: warnings and park alerts, the storm
-- clock, the turnaround, avalanche danger, fire, both ends of the hike, the
-- trail, smoke, models.
function Hiking.cards(forecast, place)
    if not Trail.now(forecast) then return {} end
    local day, today = Trail.planDay(forecast)
    local peak = Sources.peak(place)
    local cards = {}
    for _, card in ipairs {
        alertCard(forecast) or false, Services.parkCard(forecast, peak) or false,
        day and stormCard(forecast, day, today) or false, turnCard(forecast) or false,
        avalancheCard(forecast, place) or false, Services.fireCard(forecast, peak) or false,
        pointsCard(forecast, place) or false, trailCard(forecast, place), airCard(forecast) or false,
        confidenceCard(forecast) or false,
    } do
        if card then cards[#cards + 1] = card end
    end
    return cards
end

function Hiking.metrics(hour, forecast)
    local place, top, start = forecast.spot, hour.top, hour.start
    local peak = Sources.peak(place)
    local list = {}
    if top and start and peak then
        list[#list + 1] = { label = "top", value = Units.height(top.elevation, 0),
            note = ("%s · feels %s"):format(peak.top.name, Units.temp(top.feels)) }
        list[#list + 1] = { label = "start", value = Units.height(start.elevation, 0),
            note = ("%s · feels %s"):format(peak.start.name, Units.temp(start.feels)) }
    else
        list[#list + 1] = { label = "height", value = Units.height(forecast.elevation, 0),
            note = "the forecast is for this height" }
    end
    local height, at = top and top.elevation or forecast.elevation, top or hour
    list[#list + 1] = { label = "freezing level", value = Units.height(at.freezingLevel, 0),
        note = (height and at.freezingLevel and at.freezingLevel < height) and "below the top: ice likely"
            or "above you" }
    if top and top.ridge then
        list[#list + 1] = { label = "ridge wind", value = Units.wind(top.ridge) .. " " .. Sky.compass(top.ridgeDir),
            note = "open air at the top's height: less on a sheltered top" }
    end
    if not top then
        list[#list + 1] = { label = "feels like", value = Units.temp(hour.feels), note = "air " .. Units.temp(hour.temp) }
    end
    local wind = Report.wind(at)
    if top then wind.label = "wind at the top" end
    list[#list + 1] = wind
    list[#list + 1] = { label = "visibility", value = Units.distance((at.visibility or 0) / 1000),
        note = Sky.describe(at.code) }
    local thunder = Trail.thunder(at)
    list[#list + 1] = { label = "storm energy", value = Score.fmt(at.cape, " J/kg"),
        note = thunder and "thunder " .. thunder .. " (estimate)" or (at.cape or 0) >= 300 and "some risk" or "low" }
    if (at.snow_depth or 0) > 0 then
        list[#list + 1] = { label = "snow depth", value = Trail.snow(at.snow_depth * 100), note = "modelled, not measured" }
    end
    if hour.aqi then
        list[#list + 1] = { label = "air quality", value = Score.fmt(hour.aqi, " AQI"),
            note = "PM2.5 " .. Score.fmt(hour.pm25, " µg/m³") }
    end
    local light = Trail.light(hour.day)
    if light and not light.always and not light.never then
        list[#list + 1] = { label = "first · last light", value = Trail.clock(light.first) .. " · " .. Trail.clock(light.last),
            note = "civil twilight" }
    else
        list[#list + 1] = Report.sun(hour)
    end
    if peak and peak.official then
        local names, wheres = {}, {}
        for _, entry in ipairs(peak.official) do names[#names + 1], wheres[#wheres + 1] = entry[1], entry[2] end
        local alerts = Trail.alerts(forecast)
        local note = table.concat(wheres, " · ")
        if alerts and #alerts == 0 then note = note .. " · no active warnings" end
        list[#list + 1] = { label = "official word", value = table.concat(names, " · "), note = note }
    end
    return list
end

-- The two charts on the activity's "today" view, over the next 24 hours.
-- `value` is metric; `unit` (a Units kind) converts it and ends the title.
-- With both ends known, "feels like" is the top's.
Hiking.charts = {
    { title = "feels like", unit = "temp", kind = "line",
        value = function(hour) local at = hour.top or hour; return at.feels or at.temp or 0 end },
    { title = "chance of rain", unit = "percent", kind = "column", min = 0, max = 100,
        value = function(hour) return hour.rainChance or 0 end },
}

return Hiking
