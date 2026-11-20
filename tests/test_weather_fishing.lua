-- Fishers' Weather: will the fish bite? The fishing activity's rules on hand-
-- built hours, its edition modules on synthetic forecasts, then the fishing
-- edition running on its own places (tests/weather_fixture.lua). No network.
-- This file belongs to the fishing edition.
local F = dofile("tests/weather_fixture.lua").init { edition = "fishing" }
local check, verdict, says = F.check, F.verdict, F.says
local Activities = require "Weather/activities"
local Settings = require "Weather/settings"
local Forecast = require "Weather/forecast"
local Sky = require "Weather/sky"
local Fishing = require "Weather/activities/fishing"
local Fronts = require "Weather/editions/fishing/fronts"
local Water = require "Weather/editions/fishing/water"
local Moon = require "Weather/editions/fishing/moon"
local Tides = require "Weather/editions/fishing/tides"
local Log = require "Weather/editions/fishing/log"
local River = require "Weather/editions/fishing/river"
local Compare = require "Weather/editions/fishing/compare"
local Page = require "Weather/editions/fishing/page"

-- Runs `fn` with a fishing setting changed, then puts the default back.
local function with(key, value, fn)
    Settings.set("fishing", key, value)
    local ok, err = pcall(fn)
    for _, setting in ipairs(Fishing.settings) do
        if setting.key == key then Settings.set("fishing", key, setting.default) end
    end
    assert(ok, err)
end

local LAKE = { id = "lake", title = "Test Lake", lat = 39.09, lon = -120.04, lake = true, facing = 90,
    activities = { "fishing" } }
local COAST = { id = "coast", title = "Test Coast", lat = 24.92, lon = -80.63, facing = 135, activities = { "fishing" } }

-- A parsed synthetic week (`extra` hourly columns) at a place, as of `epoch`.
local function forecastAt(place, extra, epoch, marine)
    local forecast = Forecast.parse(F.weather(place.lat, extra), marine, epoch or F.NOW, place)
    forecast.spot = place
    return forecast
end

-- The rules as they were, now worded as anglers' experience.
local dawn = verdict(Fishing)
check(dawn.verdict == "good", "Dawn, light breeze and slowly falling pressure is good fishing: " .. dawn.score)
check(dawn.reasons[1].text:find("Dawn"), "The strongest reason leads: " .. dawn.reasons[1].text)
check(verdict(Fishing, { code = 95 }).verdict == "poor", "Thunder means poor fishing")
check(verdict(Fishing, { pressureTrend = 2.5, time = "2026-11-18T12:00", cloud = 0, uv = 7 }).verdict == "poor",
    "Rising pressure under a bright noon sun is poor")
check(verdict(Fishing, { wind = 13, gust = 20 }).score <= 35, "Gusts cap the score")

-- FI3: honest scoring. Lore says whose lore it is; the moon is off by default.
check(says(dawn, "[Aa]nglers"), "Pressure and light rules are worded as anglers' experience")
check(Settings.get("fishing", "moon") == false, "The moon is out of the score by default")
local fullMoon
for day = 0, 30 do
    local epoch = F.NOW + day * 86400
    local moon = Sky.moon(epoch)
    if moon.daysFromSyzygy < 1 and moon.phase > 0.25 and moon.phase < 0.75 then fullMoon = epoch; break end
end
F.forecast.epoch = fullMoon
check(not says(verdict(Fishing), "moon"), "Near the full moon, the default score says nothing of the moon")
with("moon", true, function()
    local result = verdict(Fishing)
    check(says(result, "full moon: by solunar tradition") and says(result, "not proven"),
        "With the moon on, its reason is labelled as tradition")
end)
F.forecast.epoch = F.NOW
check(Settings.text("fishing", "moon") == "off", "The settings page shows the moon toggle off")

-- FI4: safety caps the hour, and depends on where the angler stands.
with("from", "boat", function()
    check(verdict(Fishing, { gust = 12 }).score <= 45, "Gusts of 12 m/s cap a small boat")
    local offshore = verdict(Fishing, { windDir = 270, wind = 7, gust = 9 }, { facing = 90 })
    check(offshore.score <= 45 and offshore.reasons[1].text:find("Offshore"), "An offshore wind caps a small boat")
    check(verdict(Fishing, { visibility = 400 }).score <= 40, "Fog caps a boat: don't run it blind")
    check(says(verdict(Fishing, { seaTemp = 7 }, COAST), "Cold water, 7°: .*life jacket"), "Cold water warns a boat")
    check(verdict(Fishing, { waveHeight = 1.8 }, COAST).score <= 35, "Waves cap a small boat on the coast")
end)
check(verdict(Fishing, { gust = 12 }).score > 45, "From the bank the same gusts do not cap")
check(says(verdict(Fishing, { windDir = 90, wind = 3 }, { facing = 90 }), "onto this bank"),
    "A wind onto the bank is the windward shore anglers fish")
check(says(verdict(Fishing, { visibility = 400 }), "Fog on the water"), "Fog is mentioned from the bank")
check(verdict(Fishing, { waveHeight = 3 }, COAST).score <= 40, "Big waves cap anyone at the water's edge")
check(verdict(Fishing, { waveHeight = 3 }, LAKE).score > 40, "A lake has no sea waves to cap")
with("from", "rocks", function()
    check(verdict(Fishing, { waveHeight = 1.6 }, COAST).score <= 30, "Waves cap rock anglers lower")
end)
do
    local gusty = forecastAt(LAKE, { wind_gusts_10m = function(i) return i < 9 and 6 or 16 end })
    local now = gusty.hours[gusty.now]
    check(says(Fishing.rate(now, gusty):result(), "Wind builds: gusts 16 m/s by 09:00"),
        "Calm now with gusts in two hours warns in the reasons")
    local card = Fishing.windCard(gusty)
    check(card and card.tone == "poor" and card.value == "gusts from 09:00", "A wind-later card names the hour")
    check(Fishing.windCard(forecastAt(LAKE)) == nil, "No wind-later card on a steady day")
end

-- FI1: water temperature, from the best source and always labelled.
check(Fishing.needsMarine == "optional", "Fishing asks for the sea when there is one, and stands without it")
check(Activities.request(LAKE).pastDays == 2, "Fishing asks for two past days")
local sea = Water.at(F.hour { seaTemp = 13.5 }, { spot = COAST })
check(sea and sea.kind == "sea" and sea.temp == 13.5 and sea.source:find("marine model"), "A coast reads the sea surface")
check(Water.at(F.hour { seaTemp = 13.5 }, { spot = LAKE }) == nil, "A lake never claims a sea temperature")
local lake = forecastAt(LAKE)
local estimate = Water.at(lake.hours[lake.now], lake)
check(estimate and estimate.kind == "lake" and estimate.temp == 6 and estimate.source:find("estimate"),
    "A steady water cell gives a labelled lake estimate")
local land = forecastAt(LAKE, { soil_temperature_0cm = function(i) return i % 24 < 12 and 0 or 14 end })
check(Water.at(land.hours[land.now], land) == nil, "A cell that swings like land gives no lake estimate")
local reading = Water.at(lake.hours[lake.now], lake, { temp = 9, epoch = F.NOW - 3600 })
check(reading.kind == "reading" and reading.temp == 9, "The angler's own reading wins")
check(Water.at(lake.hours[lake.now], lake, { temp = 9, epoch = F.NOW - 4 * 86400 }).kind == "lake",
    "A reading older than three days gives way to the estimate")
check(Fishing.waterCard(lake).value == "6°" and Fishing.waterCard(lake).tone == "fair",
    "The water card shows the estimate and the cold-water warning")
check(says(Fishing.rate(lake.hours[lake.now], lake):result(), "Cold water, 6°"), "Cold lake water is in the score")

-- FI2: 48 hours of pressure behind, fronts named in words.
local TROUGH = {
    pressure_msl = function(i) return i <= 20 and 1016 - 0.4 * i or 1008 + 0.5 * (i - 20) end,
    wind_direction_10m = function(i) return i <= 20 and 180 or 290 end,
}
local coming = forecastAt(LAKE, TROUGH)
local fronts = Fronts.list(coming)
check(#fronts == 1 and fronts[1].hour.time == "2026-11-18T20:00" and fronts[1].kind == "front"
    and fronts[1].from == "S" and fronts[1].to == "W", "A trough with a wind swing is a front at 20:00")
local frontCard = Fishing.frontCard(coming)
check(frontCard.title == "front coming" and frontCard.value == "passes ~20:00" and frontCard.detail:find("S to W"),
    "The card says when the front passes: " .. frontCard.value)
check(says(Fishing.rate(coming.hours[16], coming):result(), "A front arrives about 20:00"),
    "Hours just before a front say so")
local after = forecastAt(LAKE, TROUGH, F.NOW + 30 * 3600)
local afterCard = Fishing.frontCard(after)
check(afterCard.title == "after a front" and afterCard.value == "day 1 after it", "A day later: " .. afterCard.value)
check(says(Fishing.rate(after.hours[after.now], after):result(), "The day after a front"),
    "The day after a front is scored as anglers find it")
check(Fronts.change(after, after.hours[after.now], 6) == 3, "The six-hour change reads back through past hours")
check(Fishing.frontCard(lake).title == "pressure", "Without a trough the card shows the trend")
local metrics = Fishing.metrics(after.hours[after.now], after)
check(metrics[1].note:find("in 6 h"), "The pressure row adds the six-hour change: " .. metrics[1].note)

-- FI8: the moon's real rise and set, solunar times as tradition.
local newMoon = Moon.position(947182440)
check(math.abs(newMoon.longitude - 285.8) < 1.5, "At new moon the moon stands by the sun: " .. newMoon.longitude)
local fullDate = os.date("!%Y-%m-%d", fullMoon)
local full = Moon.day(fullDate, 51.5, 0, 0)
local fullRise = Sky.minutes(full.rise)
check(fullRise and fullRise > 14 * 60 and fullRise < 18 * 60, "A full moon rises near sunset: " .. tostring(full.rise))
local periods = Moon.solunar(Moon.day("2026-11-18", 51.5, 0, 0))
local majors = 0
for _, period in ipairs(periods) do majors = majors + (period.kind == "major" and 1 or 0) end
check(majors >= 1 and #periods >= 3, "A day has solunar majors and minors")
check(Fishing.metrics(lake.hours[lake.now], lake)[4].note:find("rises %d%d:%d%d"), "The moon row gives moonrise")
with("moon", true, function()
    local cards = Fishing.cards(lake)
    check(cards[#cards].id == "solunar" and cards[#cards].title:find("tradition"), "Solunar times show as tradition")
end)
for _, card in ipairs(Fishing.cards(lake)) do check(card.id ~= "solunar", "No solunar card with the moon off") end

-- FI5: tides on the coast, approximate unless a NOAA station answers.
local function seaLevel(lat, lon)
    return { latitude = lat, longitude = lon, hourly = {
        time = F.series(7 * 24, F.iso),
        sea_level_height_msl = F.series(7 * 24, function(i) return 0.6 * math.cos(2 * math.pi * (i - 3) / 12.42) end) } }
end
local model = Tides.parseSeaLevel(seaLevel(24.95, -80.6))
check(model.turns[1].high and model.turns[1].time == "2026-11-18T03:00", "High water at 03:00: " .. model.turns[1].time)
check(not model.turns[2].high and model.turns[2].time:find("^2026%-11%-18T09:1"), "Low about 09:13: " .. model.turns[2].time)
local running = Tides.stage(model, "2026-11-18T06:00")
check(running.stage == "falling" and running.running, "Mid-ebb the tide runs")
check(Tides.stage(model, "2026-11-18T09:00").stage == "slack", "Near low water it is slack")
local coastForecast = forecastAt(COAST, nil, nil, F.marine())
coastForecast.tidesModel = model
local tideCard = Fishing.tideCard(coastForecast)
check(tideCard.title == "tides (approximate)" and tideCard.detail:find("Not for navigation")
    and tideCard.detail:find("km away"), "Model tides are labelled approximate, with the cell's distance")
check(says(Fishing.rate(coastForecast.hours[coastForecast.now], coastForecast):result(), "Tide running"),
    "Running water is in the score")
local noaa = Tides.parseNoaa { predictions = { { t = "2026-11-18 04:12", v = "0.7", type = "H" },
    { t = "2026-11-18 10:30", v = "0.1", type = "L" }, { t = "2026-11-18 16:40", v = "0.8", type = "H" } } }
local station = { id = "keys", lat = 24.92, lon = -80.63, facing = 135,
    tideStation = { noaa = "0000000", name = "Test Harbor", lat = 24.95, lon = -80.6 } }
check(Tides.noaaUrl(station, F.NOW):find("station=0000000") and Tides.noaaUrl(COAST) == nil
    and Tides.noaaUrl(LAKE) == nil, "NOAA is asked only for a place with a station")
local stationTide = Tides.of { spot = station, tidesNoaa = noaa, tidesModel = model }
check(stationTide.kind == "noaa" and not stationTide.approximate and stationTide.source == "NOAA station Test Harbor"
    and stationTide.distance < 5, "A NOAA station wins, named, with its distance")
check(Tides.of { spot = LAKE, tidesModel = model } == nil, "A lake has no tide")

-- FI11: river flow for river places.
local riverJson = { daily = { time = F.series(10, function(d) return ("2026-11-%02d"):format(11 + d) end),
    river_discharge = { 10, 11, 10, 9, 10, 12, 11, 30, 28, 20 } } }
check(River.url(LAKE) == nil and River.url({ river = true, lat = 1, lon = 2 }):find("river_discharge"),
    "Only a river place asks for discharge")
local riverForecast = forecastAt({ id = "r", title = "R", lat = 45, lon = 6, river = true })
riverForecast.river = River.parse(riverJson)
check(River.on(riverForecast, "2026-11-18").state == "high", "Three times the usual flow is blown out")
check(says(Fishing.rate(riverForecast.hours[riverForecast.now], riverForecast):result(), "blown out"),
    "A blown-out river is in the score")
check(Fishing.riverCard(riverForecast).detail:find("GloFAS"), "The river card names its model")

-- FI9: species profiles change the rules.
with("species", "trout", function()
    check(says(verdict(Fishing, { seaTemp = 22 }, COAST), "trout are stressed"), "Warm water stresses trout")
end)
with("species", "walleye", function()
    check(says(verdict(Fishing, { wind = 5 }), "walleye chop"), "Walleye anglers like a chop")
end)
check(not says(verdict(Fishing, { seaTemp = 22 }, COAST), "trout"), "Any fish: no trout rule")

-- FI12: where the local rules are.
local tahoeRules = Fishing.metrics(F.hour(), { spot = { id = "tahoe" }, epoch = F.NOW })
check(tahoeRules[#tahoeRules].value:find("wildlife.ca.gov"), "Lake Tahoe links California's rules")

-- FI6: the private log saves the hour's conditions, offline, and exports CSV.
local store = F.storage("fishing-log")
store:remove(Log.FILE)
Log.use(store)
local saved = Log.add(coastForecast, COAST, { kind = "catch", species = "snook" }, F.NOW + 600)
check(saved and saved.pressure and saved.wind == 3 and saved.water == 13.5 and saved.tide == "falling"
    and saved.trend3 and saved.score and saved.moon, "A catch keeps the hour's conditions")
check(Log.add(coastForecast, COAST, { kind = "skunk" }, F.NOW - 8 * 3600) == nil,
    "An hour the forecast does not hold is refused")
local earlier = forecastAt(COAST, nil, F.NOW + 6 * 3600)
check(Log.add(earlier, COAST, { kind = "skunk" }, F.NOW + 3600).time == "2026-11-18T07:00",
    "A trip logged later still gets that hour's weather")
Log.use(store) -- read back from the file
check(#Log.entries() == 2 and Log.entries()[1].kind == "skunk" and Log.entries()[2].species == "snook",
    "The log survives a restart, newest first")
local csv = Log.csv()
check(csv:find("^time,kind,place,species") and select(2, csv:gsub("\n", "")) == 3 and csv:find("snook"),
    "CSV has a header and a line per entry")
local path = Log.export()
local file = path and io.open(path, "r+b")
check(file and file:read("a") == csv, "Export writes the CSV beside the log")
if file then file:close() end
check(Log.undo().kind == "skunk" and #Log.entries() == 1, "Undo takes back the newest entry")
Log.setReading("coast", 18, F.NOW)
check(Fishing.water(coastForecast.hours[coastForecast.now], coastForecast).kind == "reading",
    "A logged water reading feeds the score")
Log.setReading("coast", nil)

-- FI7: patterns from the angler's own catches, once there are enough.
check(Page.entryRow(Log.entries()[1], 1).detail:find("^Test Coast · .*hPa falling · wind 3 m/s · water 14°"),
    "An entry row lists its saved conditions: " .. Page.entryRow(Log.entries()[1], 1).detail)
check(not Log.patterns().ready, "One catch is not a pattern")
for index = 1, Log.PATTERN_MINIMUM do
    Log.add(coastForecast, COAST, { kind = index % 3 == 0 and "skunk" or "catch" }, F.NOW)
end
for index = 1, 6 do Log.add(coastForecast, COAST, { kind = "catch" }, F.NOW) end
local patterns = Log.patterns()
check(patterns.ready and patterns.patterns[1].title == "pressure" and patterns.patterns[1].band == "falling",
    "Enough catches show where they cluster")

-- FI10: compare my waters.
local calm, gale = forecastAt(LAKE), forecastAt(COAST, { wind_gusts_10m = function() return 20 end })
local ranked = Compare.rank(Fishing, { COAST, LAKE, { id = "x", title = "X", activities = { "surf" } } },
    function(id) return id == "lake" and calm or gale end)
check(#ranked == 2 and ranked[1].place.id == "lake" and ranked[1].label == "this morning"
    and ranked[2].result.score <= 35, "The calm lake ranks above the gale-swept coast")

-- The edition on its own places, then the log page as the app would open it.
F.checkEdition("fishing")
local net = F.network()
net.answers["sea_level_height_msl"] = function(url)
    return seaLevel(tonumber(url:match("latitude=([%-%d%.]+)")), tonumber(url:match("longitude=([%-%d%.]+)")))
end
local ui = F.launch { places = { LAKE, COAST }, net = net, storageName = "fishing-app" }
local asked = table.concat(net.requests, "\n")
check(asked:find("past_days=2") and not asked:find("sea_level_height_msl"), "A lake asks for past days, no tide")
check(ui.has("Cardlake:card:water") and ui.has("Cardlake:card:front"), "The lake's hub leads with water and pressure")
ui.press("Placecoast")
asked = table.concat(net.requests, "\n")
check(asked:find("marine%-api[^\n]*latitude=24.92") and asked:find("sea_level_height_msl"), "A coast asks for the sea and the tide")
check(ui.has("Cardcoast:card:tides"), "The coast's hub has a tide card")

check(Fishing.page.link == "log a catch" and Fishing.page.path == "/fishing/log", "Fishing declares its log page")
Log.use(nil)
local appStore = ui.app.storage
appStore:remove(Log.FILE)
ui.app.place = LAKE
local Routes = require "orca.data.routes"
local wentBack = false
local provider = Fishing.page.provider { forecastFor = ui.app.provider.forecastFor, places = { LAKE, COAST },
    place = function() return ui.app.place end, clock = function() return F.NOW end,
    back = function() wentBack = true end }
local page, scope = Routes.load(Fishing.page.routes):open(Fishing.page.path, ui.app, { provider = provider, host = ui.app.host })
local screen = ui.app:screenFor(page)
scope:activate()
local core = require "orca.core"
local function settle()
    for _ = 1, 3 do core.flushQueue(); core.advanceFrame(); core.runAllPrograms(screen); core.flushQueue() end
    screen:UpdateLayout(390, 844)
end
local function press(name) page:findChild(name, true):send("Node.LeftButtonUp"); settle() end
settle()
check(page:findChild("NoEntries", true).Visible, "An empty log says it stays on the phone")
press("LogCatch")
check(#Log.entries() == 1 and Log.entries()[1].place == "lake", "One tap logs a catch at the place")
check(page:findChild("Entrylog:entry:1", true) ~= nil, "The entry shows in the list")
check(page:findChild("Statuslog:status", true):findChild("AboutDetail", true).Text:find("saved a catch at 06:00"),
    "The page confirms what it saved")
press("LogSkunk")
check(Log.entries()[1].kind == "skunk", "A blank is one tap too")
press("WarmerWater")
check(Log.reading("lake").temp == 6, "The first tap on the reading starts from the estimate")
press("WarmerWater")
check(Log.reading("lake").temp == 7 and page:findChild("Readinglog:reading", true):findChild("MetricValue", true).Text == "7°",
    "Warmer moves the reading by a degree")
press("UndoLog")
check(#Log.entries() == 1, "Undo from the page")
press("ExportLog")
check(io.open(appStore.dir .. "/fishing-log.csv", "r+b") ~= nil, "Export from the page")
check(page:findChild("Waterlake", true) ~= nil or page:findChild("Waterwaters:lake", true) ~= nil,
    "The waters view lists the places")
page:findChild("FishingLogPivot", true):post("Pivot.BackRequested")
settle()
check(wentBack, "Back leaves the log")
scope:dispose()
Log.setReading("lake", nil)
ui.app.hubScope:dispose()

F.done("weather-fishing")
