-- Fishing: is it a good hour to fish here? Anglers read the barometer, the
-- wind on the water, the light, the water's warmth, the tide and, some of
-- them, the moon. The rules below are that experience, worded as anglers
-- tell it, not as proven science (Research/fishing.md, "Presenting lore as
-- science"). Safety comes first and caps the hour: lightning, gusts, an
-- offshore wind for a small boat, fog, cold water and waves on the rocks.
-- The edition's own modules live in editions/fishing/.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Report = require "Weather/report"
local Settings = require "Weather/settings"
local Fronts = require "Weather/editions/fishing/fronts"
local Water = require "Weather/editions/fishing/water"
local Moon = require "Weather/editions/fishing/moon"
local Tides = require "Weather/editions/fishing/tides"
local River = require "Weather/editions/fishing/river"
local Species = require "Weather/editions/fishing/species"
local Rules = require "Weather/editions/fishing/rules"
local Log = require "Weather/editions/fishing/log"

local Fishing = {
    id = "fishing", title = "fishing",
    -- The sea surface and waves where there is sea, and the forecast stands
    -- without them; a lake does not ask.
    needsMarine = function(place) return not (place and place.lake) and "optional" end,
    -- Two past days for the barometer's history and fronts (FI2).
    fields = { pastDays = 2 },
}

-- The edition's own page: the private catch log, my waters and patterns
-- (editions/fishing/page.lua), opened from the fishing page's link.
Fishing.page = { routes = "Weather/editions/fishing/Data/Routes.xml", path = "/fishing/log", link = "log a catch",
    provider = function(options) return require("Weather/editions/fishing/page").provider(options) end }

Fishing.sources = Tides.sources()
Fishing.sources[#Fishing.sources + 1] = River.source

Fishing.settings = {
    { key = "from", title = "I fish from", kind = "choice", default = "shore",
        options = { { "shore", "the bank or a beach" }, { "boat", "a small boat" }, { "rocks", "rocks or a pier" } },
        detail = "the safety rules for wind, waves and cold water follow this" },
    { key = "species", title = "target fish", kind = "choice", default = "any", options = Species.OPTIONS,
        detail = "anglers' rules of thumb for that fish's water and light" },
    { key = "moon", title = "moon in the score", kind = "toggle", default = false,
        detail = "solunar times are anglers' tradition; studies have not found the effect" },
}

local DEFAULTS = {}
for _, setting in ipairs(Fishing.settings) do DEFAULTS[setting.key] = setting.default end
local function setting(key)
    local value = Settings.get("fishing", key)
    if value == nil then return DEFAULTS[key] end
    return value
end
Fishing.setting = setting

-- Where the wind blows relative to the bank: "onto" it from the water,
-- "off" it out over the water, or "along" it. `facing` points from the
-- shore toward the water; a wind's direction is where it comes from.
function Fishing.bank(hour, spot)
    if not (hour.windDir and spot and spot.facing) then return nil end
    if Sky.angle(hour.windDir, spot.facing) <= 45 then return "onto" end
    if Sky.angle(hour.windDir, spot.facing + 180) <= 45 then return "off" end
    return "along"
end

local function coastal(spot) return spot and spot.lat and not spot.lake and not spot.river end

-- The water at an hour (Water.at with the place's own reading).
function Fishing.water(hour, forecast)
    local spot = forecast and forecast.spot
    return Water.at(hour, forecast, spot and Log.reading(spot.id))
end

-- The highest gust in the `hours` after an hour, and when it comes.
local function gustAhead(forecast, hour, hours)
    local index = Fronts.indexOf(forecast, hour)
    if not index then return nil end
    local top, at
    for other = index + 1, math.min(#forecast.hours, index + hours) do
        local gust = forecast.hours[other].gust
        if gust and (not top or gust > top) then top, at = gust, forecast.hours[other] end
    end
    return top, at
end

-- Rain over the day before an hour, mm.
local function rainBefore(forecast, hour)
    local index = Fronts.indexOf(forecast, hour)
    if not index or index <= 24 then return nil end
    local sum = 0
    for other = index - 24, index - 1 do sum = sum + (forecast.hours[other].rain or 0) end
    return sum
end

local function moonDay(forecast, date)
    local spot = forecast and forecast.spot
    if not (spot and spot.lat and spot.lon and date) then return nil end
    return Moon.cached(date, spot.lat, spot.lon, forecast.offset or 0)
end

-- Safety: these cap the hour whatever the fish do, and lead the reasons.
local function safety(card, hour, forecast, spot, from, water)
    if Sky.isStorm(hour.code) then card:cap(10, "Lightning risk: stay off the water and away from the shore.") end
    local gust = hour.gust or 0
    if gust > 15 then
        card:cap(35, "Gusts over " .. Units.wind(15) .. ": dangerous on open water, hard to stand and cast.")
    elseif from == "boat" and gust > 11 then
        card:cap(45, "Gusts over " .. Units.wind(11) .. ": rough going in a small boat.")
    end
    local later, at = gustAhead(forecast, hour, 3)
    if later and later >= 13 and gust < 10 then
        card:add(from == "boat" and -12 or -6, ("Wind builds: gusts %s by %s. Be off the water before then.")
            :format(Units.wind(later), Units.clock(at.time)))
    end
    local bank = Fishing.bank(hour, spot)
    if bank == "off" and from == "boat" and (hour.wind or 0) >= 6 then
        card:cap(45, "Offshore wind: it pushes a small boat away from this bank and makes it hard to row back.")
    end
    if (hour.visibility and hour.visibility < 1000) or hour.code == 45 or hour.code == 48 then
        if from == "boat" then card:cap(40, "Fog: less than " .. Units.distance(1) .. " to see. Don't run a boat blind.")
        else card:add(-4, "Fog on the water: hard to see your line and your footing.") end
    end
    if water and Water.danger(water.temp) == "cold" then
        card:add(from == "shore" and -3 or -6, ("Cold water, %s: a fall in takes your breath away. Wear a life jacket.")
            :format(Units.temp(water.temp)))
    end
    local wave = coastal(spot) and (hour.waveHeight or hour.swellHeight)
    if wave then
        if from == "rocks" and wave >= 1.5 then
            card:cap(30, ("Waves of %s can sweep over rocks. Fish from higher ground or stay off them."):format(Units.height(wave)))
        elseif from == "boat" and wave >= 1.5 then
            card:cap(35, ("Waves of %s: rough water for a small boat."):format(Units.height(wave)))
        elseif wave >= 2.5 then
            card:cap(40, ("Big waves, %s: keep well back from the water's edge."):format(Units.height(wave)))
        end
    end
    return bank, wave
end

-- The barometer: fronts first, then the three-hour trend.
local function pressure(card, hour, forecast)
    local last, next = Fronts.around(forecast, hour, 36, 6)
    local trend = hour.pressureTrend or 0
    if last and Fronts.since(forecast, hour, last) < 3 then
        card:add(-6, "A front is passing: anglers say the bite often switches off for a while.")
    elseif next and trend < 0 then
        card:add(10, ("A front arrives about %s: anglers say fish feed hard before it."):format(Units.clock(next.hour.time)))
    elseif last and trend > 0 then
        card:add(-8, "The day after a front: anglers often find fish tight-lipped under rising pressure.")
    elseif trend <= -3 then card:add(-6, "Pressure dropping fast: a front is close, and anglers say the bite fades.")
    elseif trend <= -0.5 then card:add(12, "Pressure falling slowly: anglers say fish feed ahead of the weather.")
    elseif trend < 0.5 then card:add(6, "Steady pressure: in anglers' experience fish keep their routine.")
    elseif trend < 1.5 then card:add(-4, "Pressure creeping up: anglers often find bites slow down.")
    else card:add(-12, "Pressure rising fast after a front: anglers say fish go deep.") end
end

function Fishing.rate(hour, forecast)
    forecast = forecast or {}
    local spot = forecast.spot or {}
    local from = setting("from")
    local card = Score.card(50)
    local water = Fishing.water(hour, forecast)
    local bank, wave = safety(card, hour, forecast, spot, from, water)

    pressure(card, hour, forecast)

    local wind = hour.wind or 0
    if wind < 1.5 then card:add(-4, "Flat calm: clear water lets fish see you and your line.")
    elseif wind <= 5 then card:add(8, "Light breeze ripples the water and hides you.")
    elseif wind <= 8 then card:add(-2, "Fresh breeze: try the sheltered bank.")
    else card:add(math.floor(Score.curve(wind, { { 8, -8 }, { 12, -25 } })), "Strong wind: hard casting, rough water.") end
    if bank == "onto" and wind >= 2 and wind <= 8 then
        card:add(5, "Wind blows onto this bank: anglers say food, and fish, drift in on it.")
    elseif bank == "off" and wind >= 2 and wind <= 8 and from ~= "boat" then
        card:add(2, "Wind at your back on this bank: easy casting, calm water at the edge.")
    end

    local twilight = Score.twilight(hour)
    if twilight <= 60 then card:add(15, "Dawn or dusk: anglers' favourite feeding hour.")
    elseif twilight <= 120 then card:add(7, "Close to dawn or dusk.")
    elseif not hour.isDay then card:add(-10, "Dark: fewer bites, and hard to see.") end

    local bright = hour.isDay and (hour.cloud or 0) < 20 and (hour.uv or 0) >= 6
    if bright then card:add(-8, "Bright sun: anglers find fish hold deep and in shade.")
    elseif hour.isDay and (hour.cloud or 0) >= 40 and (hour.cloud or 0) <= 90 then
        card:add(5, "Clouds take the glare off the water.")
    end

    if Sky.isSnow(hour.code) then card:add(Sky.isHeavy(hour.code) and -15 or -6, "Snowing: cold hands, poor visibility.")
    elseif Sky.isHeavy(hour.code) then card:add(-12, "Heavy rain muddies the water.")
    elseif Sky.isWet(hour.code) then card:add(4, "Light rain stirs the surface and washes food in.") end
    local fallen = rainBefore(forecast, hour)
    if fallen and fallen >= 20 then
        card:add(-4, ("%s of rain in the last day: inflows run muddy. Anglers fish the clearer edges."):format(Units.rain(fallen)))
    end

    -- The water's warmth when it is known, else the air's.
    if water then
        local t = water.temp
        if t < 4 then card:add(-8, ("Water near freezing, %s: fish are slow and deep."):format(Units.temp(t)))
        elseif t < 8 then card:add(-3, ("Cold water, %s: fish move little. Anglers slow their baits down."):format(Units.temp(t)))
        elseif t > 26 then card:add(-6, ("Warm water, %s: little oxygen, fish go deep. Go early or late."):format(Units.temp(t))) end
    else
        local temp = hour.temp or 10
        if temp < 2 then card:add(-8, "Near freezing: cold hands, and anglers find fish slow.")
        elseif temp > 30 then card:add(-6, "Hot: fish go deep. Go early or late.") end
    end

    if coastal(spot) then
        local tide = Tides.of(forecast)
        local stage = tide and Tides.stage(tide, hour.time)
        if stage and stage.stage == "slack" then
            card:add(-4, "Slack water: anglers often wait for the tide to turn.")
        elseif stage and stage.running then
            card:add(8, ("Tide running (%s): anglers say moving water brings the bite."):format(stage.stage))
        elseif stage then
            card:add(3, ("Tide %s: the water is on the move."):format(stage.stage))
        end
    end

    local river = forecast.river and River.on(forecast, hour.date)
    if river and river.state == "high" then
        card:add(-15, "River running high and coloured: anglers call it blown out.")
    elseif river and river.trend == "falling" and river.ratio > 1.2 then
        card:add(6, "River dropping and clearing after a rise: a time anglers like.")
    end

    Species.apply(card, setting("species"), { hour = hour, water = water and water.temp, coast = coastal(spot),
        wave = wave, twilight = twilight, bright = bright })

    -- The moon only when the angler asks for it, and always as tradition.
    if setting("moon") then
        local moon = Sky.moon(hour.epoch or forecast.epoch or os.time())
        if moon.daysFromSyzygy <= 2 then
            card:add(4, ("Near the %s: by solunar tradition a strong day (not proven)."):format(
                moon.phase > 0.25 and moon.phase < 0.75 and "full moon" or "new moon"))
        end
        local day = moonDay(forecast, hour.date)
        local period = day and Moon.periodAt(Moon.solunar(day), hour.time)
        if period then
            card:add(period.kind == "major" and 4 or 2, ("Solunar %s period: tradition, not proven."):format(period.kind))
        end
    end
    return card
end

-- Anglers fish daylight and the hour or so either side of it.
function Fishing.window(hour) return hour.isDay or Score.twilight(hour) <= 90 end

local function trendWord(trend)
    -- Words, not arrows: the phone font has no arrow glyphs.
    return trend > 0.5 and "rising" or trend < -0.5 and "falling" or "steady"
end

function Fishing.note(hour, forecast)
    local text = ("wind %s · %s %s · rain %s"):format(Units.wind(hour.wind), Units.pressure(hour.pressure),
        trendWord(hour.pressureTrend or 0), Score.fmt(hour.rainChance, "%"))
    local spot = forecast and forecast.spot
    local tide = coastal(spot) and Tides.of(forecast)
    local stage = tide and Tides.stage(tide, hour.time)
    return stage and text .. " · tide " .. stage.stage or text
end

-- "14:00", or "tomorrow 03:00" / "yesterday 22:00" off the current day.
local function when(time, today)
    local date = time:sub(1, 10)
    if date == today then return Units.clock(time) end
    local delta = ((Sky.epochOf(date) or 0) - (Sky.epochOf(today) or 0)) // 86400
    local word = delta == 1 and "tomorrow" or delta == -1 and "yesterday" or Sky.weekday(date)
    return word .. " " .. Units.clock(time)
end

-- The pressure card: a front coming or just gone, else the trend.
function Fishing.frontCard(forecast)
    local now = forecast.hours and forecast.hours[forecast.now]
    if not now then return nil end
    local last, next = Fronts.around(forecast, now, 48, 24)
    local change6 = Fronts.change(forecast, now, 6)
    local trend = ("%s in 3 h"):format(Units.pressure(now.pressureTrend or 0, 1))
        .. (change6 and (", %s in 6 h"):format(Units.pressure(change6, 1)) or "")
    if next then
        local wind = next.from and next.to and next.from ~= next.to and (", wind turning %s to %s"):format(next.from, next.to) or ""
        return { id = "front", title = next.kind == "front" and "front coming" or "low pressure coming",
            value = (next.kind == "front" and "passes ~" or "lowest ~") .. when(next.hour.time, now.date), tone = "info",
            detail = ("Pressure falls %s, then rises%s. Anglers say fish feed before %s and go quiet after.")
                :format(Units.pressure(next.drop, 1), wind, next.kind == "front" and "a front" or "the weather") }
    end
    if last then
        local days = Fronts.since(forecast, now, last) // 24 + 1
        local clear = (now.cloud or 100) < 25 and " Clear \"bluebird\" skies after a front are the slowest, anglers say." or ""
        return { id = "front", title = "after a front", value = ("day %d after it"):format(days), tone = "info",
            detail = ("Lowest %s; pressure up %s since.%s"):format(when(last.hour.time, now.date),
                Units.pressure(now.pressure - last.hour.pressure, 1), clear) }
    end
    return { id = "front", title = "pressure", value = trendWord(now.pressureTrend or 0), tone = "info",
        detail = trend .. ". No front in the last two days or the next one." }
end

function Fishing.waterCard(forecast)
    local now = forecast.hours and forecast.hours[forecast.now]
    local water = now and Fishing.water(now, forecast)
    if not water then
        return { id = "water", title = "water", value = "unknown", tone = "info",
            detail = "No water temperature here. Log your own thermometer reading to use it." }
    end
    local danger = Water.danger(water.temp)
    return { id = "water", title = "water", value = Units.temp(water.temp), tone = danger == "cold" and "fair" or "info",
        detail = water.source .. (danger == "cold" and ". Cold: wear a life jacket." or danger == "cool"
            and ". Cool: a fall in still chills fast." or "") }
end

-- Calm now, but the wind builds later today: off the water in time (story 25).
function Fishing.windCard(forecast)
    local now = forecast.hours and forecast.hours[forecast.now]
    if not now or (now.gust or 0) >= 9 then return nil end
    local top, at = gustAhead(forecast, now, 12)
    if not (top and top >= 13) then return nil end
    for index = forecast.now + 1, math.min(#forecast.hours, forecast.now + 12) do
        local hour = forecast.hours[index]
        if (hour.gust or 0) >= 13 then at = hour; break end
    end
    return { id = "wind", title = "wind later", value = "gusts from " .. when(at.time, now.date), tone = "poor",
        detail = ("Up to %s. Plan to be off the water before then."):format(Units.wind(top)) }
end

function Fishing.tideCard(forecast)
    local now = forecast.hours and forecast.hours[forecast.now]
    local tide = now and coastal(forecast.spot) and Tides.of(forecast)
    if not tide then return nil end
    local parts = {}
    for _, turn in ipairs(Tides.upcoming(tide, now.time, 2)) do
        parts[#parts + 1] = (turn.high and "high " or "low ") .. when(turn.time, now.date)
    end
    if #parts == 0 then return nil end
    local stage = Tides.stage(tide, now.time)
    local distance = tide.distance and (", %s away"):format(Units.distance(tide.distance)) or ""
    local moon = Sky.moon(forecast.epoch or os.time())
    local springs = moon.daysFromSyzygy <= 2 and " Spring tides: a big range." or ""
    return { id = "tides", title = tide.approximate and "tides (approximate)" or "tides", value = table.concat(parts, " · "),
        tone = "info",
        detail = ("%s%s%s.%s%s"):format(stage and "Tide " .. stage.stage .. " now. " or "", tide.source, distance, springs,
            tide.approximate and " Not for navigation." or "") }
end

function Fishing.solunarCard(forecast)
    local now = forecast.hours and forecast.hours[forecast.now]
    local day = now and moonDay(forecast, now.date)
    if not day then return nil end
    local major, minor = {}, {}
    for _, period in ipairs(Moon.solunar(day)) do
        table.insert(period.kind == "major" and major or minor, Units.clock(period.at))
    end
    return { id = "solunar", title = "solunar times (tradition)",
        value = ("major %s"):format(#major > 0 and table.concat(major, " · ") or "—"), tone = "info",
        detail = ("Minor %s. Anglers' tradition from the moon; studies have not found the effect.")
            :format(#minor > 0 and table.concat(minor, " · ") or "—") }
end

function Fishing.riverCard(forecast)
    local now = forecast.hours and forecast.hours[forecast.now]
    local river = now and River.on(forecast, now.date)
    if not river then return nil end
    return { id = "river", title = "river", value = river.state == "high" and "high: blown out" or river.trend,
        tone = river.state == "high" and "poor" or "info",
        detail = ("%d m³/s, %d%% of the week's usual. GloFAS model, about 5 km: a small stream may read as a bigger river.")
            :format(math.floor(river.discharge + 0.5), math.floor(river.ratio * 100 + 0.5)) }
end

-- The big answers: wind building, the front, the water, the tide, the
-- river and, when the angler wants it, the solunar times.
function Fishing.cards(forecast)
    local cards = {}
    for _, make in ipairs { Fishing.windCard, Fishing.frontCard, Fishing.waterCard, Fishing.tideCard, Fishing.riverCard } do
        local card = make(forecast)
        if card then cards[#cards + 1] = card end
    end
    if setting("moon") then cards[#cards + 1] = Fishing.solunarCard(forecast) end
    return cards
end

function Fishing.metrics(hour, forecast)
    local spot = forecast.spot or {}
    local pressureRow = Report.pressure(hour)
    local change6 = Fronts.change(forecast, hour, 6)
    if change6 then pressureRow.note = pressureRow.note:gsub("%)$", ", " .. Units.pressure(change6, 1) .. " in 6 h)") end
    local windRow = Report.wind(hour)
    local bank = Fishing.bank(hour, spot)
    if bank then windRow.note = windRow.note .. " · " .. (bank == "along" and "along" or bank) .. " this bank" end
    local water = Fishing.water(hour, forecast)
    local rows = {
        pressureRow, windRow,
        { label = "water", value = water and Units.temp(water.temp) or "—", note = water and water.short or "no source here" },
    }
    local tide = coastal(spot) and Tides.of(forecast)
    local stage = tide and Tides.stage(tide, hour.time)
    if stage then
        rows[#rows + 1] = { label = "tide", value = stage.stage, note = (stage.next.high and "high " or "low ")
            .. Units.clock(stage.next.time) .. (tide.approximate and " · approximate" or "") }
    end
    local moon = Report.moon(forecast)
    local day = moonDay(forecast, hour.date)
    if day then
        moon.note = moon.note .. " · rises " .. Units.clock(day.rise) .. " · sets " .. Units.clock(day.set)
    end
    rows[#rows + 1] = moon
    local sun = Report.sun(hour)
    local civil = hour.day and hour.day.twilight and hour.day.twilight.civil
    if civil and civil.dawn then sun.note = "first light " .. Units.clock(civil.dawn) .. " · last light " .. Units.clock(civil.dusk) end
    rows[#rows + 1] = sun
    rows[#rows + 1] = { label = "cloud cover", value = Score.fmt(hour.cloud, "%") }
    rows[#rows + 1] = { label = "air", value = Units.temp(hour.temp), note = "feels like " .. Units.temp(hour.feels) }
    rows[#rows + 1] = Rules.metric(spot)
    return rows
end

-- The two charts on the activity's "today" view: two days back and one
-- ahead (`chartHours`), so the barometer's history shows the front.
-- `value` is metric; `unit` (a Units kind) converts it and ends the title.
function Fishing.chartHours(forecast) return forecast.now - 48, forecast.now + 23 end
Fishing.charts = {
    { title = "pressure", note = "2 days back", unit = "pressure", kind = "line",
        value = function(hour) return hour.pressure or 0 end },
    { title = "wind", unit = "wind", kind = "column", value = function(hour) return hour.wind or 0 end },
}

return Fishing
