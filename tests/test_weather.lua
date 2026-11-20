-- The weather app (apps/Weather): the shared modules and the full app. The
-- sky, storage, units, settings and forecast parsing are checked directly;
-- then the app runs on a synthetic Open-Meteo forecast through its real
-- pages, scopes and templates: the place hub, an activity page, Back,
-- switching places, settings and an offline start. Each activity's own
-- rules are in tests/test_weather_<activity>.lua; the shared harness is
-- tests/weather_fixture.lua.
local F = dofile("tests/weather_fixture.lua").init()
local check, NOW = F.check, F.NOW
local Lint = require "orca.data.lint"

local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Forecast = require "Weather/forecast"
local Storage = require "Weather/storage"
local Units = require "Weather/units"
local Settings = require "Weather/settings"
local Report = require "Weather/report"

-- Sky
check(Sky.moon(1795531980).name == "full moon", "2026-11-24 is a full moon")
check(Sky.moon(1794207720).name == "new moon" and Sky.moon(1794207720).lit < 0.01, "2026-11-09 is a new moon")
check(math.abs(Sky.moon(NOW).daysFromSyzygy - 6.4) < 0.5, "Mid-November is about six days from full")
check(Sky.weekday("2026-11-18") == "wed 18" and Sky.weekday("2027-01-01") == "fri 1", "Weekdays")
check(Sky.angle(350, 10) == 20 and Sky.angle(90, 270) == 180, "Bearings wrap")
check(Sky.moonUp(0.5, 0) and not Sky.moonUp(0.5, 12) and Sky.moonUp(0, 12) and not Sky.moonUp(0, 0),
    "A full moon is up at midnight, a new moon at noon")
check(Sky.icon(0, false):find("/moon%.svg$") and Sky.icon(2, false):find("/cloud%-moon%.svg$")
    and Sky.icon(0, true):find("/sun%.svg$") and Sky.icon(0):find("/sun%.svg$") and Sky.icon(63, false):find("/rain%.svg$"),
    "Clear nights show the moon; rain is rain day or night")
check(Sky.isSnow(75) and not Sky.isSnow(65) and Sky.isHeavy(75), "Snow codes")
check(Sky.compass(0) == "N" and Sky.compass(225) == "SW" and Sky.compass(350) == "N", "Compass points")

-- The sun (F6), against NOAA's solar calculator: Greenwich at the June
-- solstice's solar noon stands at 90 - 51.48 + 23.44 = 61.96 degrees.
check(Sky.epochOf("2026-11-18") == NOW - 6 * 3600 and Sky.epochOf("2000-01-01") == 946684800, "Dates to epochs")
check(math.abs(Sky.sunAltitude(Sky.epochOf("2026-06-21") + 12 * 3600 + 2 * 60, 51.4779, 0) - 61.96) < 0.05,
    "Solar noon at Greenwich in June")
check(Sky.sunAltitude(Sky.epochOf("2026-06-21"), 51.4779, 0) < -14, "Midnight is dark at Greenwich")
local function minutes(iso) return Sky.minutes(iso) end
local function near(iso, hhmm, tolerance)
    local h, m = hhmm:match("(%d+):(%d+)")
    return iso and math.abs(minutes(iso) - (tonumber(h) * 60 + tonumber(m))) <= (tolerance or 3)
end
local london = Sky.twilight("2026-11-18", 51.5074, -0.1278, 0)
-- NOAA: London 18 Nov 2026 sunrise 07:25, sunset 16:06, civil dusk 16:45.
check(near(london.sunrise, "07:25") and near(london.sunset, "16:06") and near(london.civil.dusk, "16:45")
    and london.astronomical.dawn < london.nautical.dawn and london.nautical.dawn < london.civil.dawn,
    "London's November sunrise, sunset and twilights: " .. london.sunrise .. " " .. london.sunset)
local newYork = Sky.twilight("2026-11-18", 40.7128, -74.006, -5 * 3600)
check(near(newYork.sunrise, "06:47") and near(newYork.sunset, "16:35"), "New York, in local time: " .. newYork.sunrise)
local midsummer = Sky.twilight("2026-06-21", 51.5074, -0.1278, 3600)
check(near(midsummer.sunrise, "04:43") and near(midsummer.sunset, "21:21") and midsummer.nautical
    and midsummer.astronomical == nil, "No full darkness over London at midsummer")
local tromso = Sky.twilight("2026-06-21", 69.65, 18.96, 7200)
check(tromso.polar == "day" and tromso.civil == nil and tromso.sunrise == nil, "Midnight sun in Tromsø")
local polarNight = Sky.twilight("2026-12-21", 69.65, 18.96, 3600)
check(polarNight.polar == "night" and polarNight.sunrise == nil and polarNight.civil.dawn ~= nil,
    "Polar night still has a civil twilight at noon")
local dusky = F.hour { time = "2026-11-18T17:00", day = { date = "2026-11-18", sunrise = "2026-11-18T07:25",
    sunset = "2026-11-18T16:06", twilight = london } }
check(Score.twilight(dusky) == 54 and math.abs(Score.twilight(dusky, "civil") - 15) <= 3
    and Score.twilight(F.hour { day = { twilight = midsummer } }, "astronomical") == math.huge,
    "Score.twilight measures to sunset or to a twilight's edge")
check(Score.darkness { sunAltitude = 10 } == "day" and Score.darkness { sunAltitude = -3 } == "civil"
    and Score.darkness { sunAltitude = -15 } == "astronomical" and Score.darkness { sunAltitude = -30 } == "night"
    and Score.darkness {} == nil, "How dark the sky is")

-- Storage (F1): plain tables, atomic writes, no code.
local scratch = F.storage("storage")
local sample = { name = "Lake \"Tahoe\"\nshore", count = 3, ratio = 0.1, on = true, off = false, list = { 1, 2, [4] = 4 },
    nested = { deep = { [10] = "ten", ["with space"] = -2.5 } }, skipped = function() end }
check(scratch:save("sample", sample), "A table saves")
local loaded = scratch:load("sample")
check(loaded.name == sample.name and loaded.count == 3 and math.type(loaded.count) == "integer" and loaded.ratio == 0.1
    and loaded.on and loaded.off == false and loaded.list[4] == 4 and loaded.list[3] == nil
    and loaded.nested.deep[10] == "ten" and loaded.nested.deep["with space"] == -2.5 and loaded.skipped == nil,
    "and loads back as it was")
check(not io.open(scratch:path("sample") .. ".tmp", "r+b"), "No temporary file is left behind")
check(Storage.serialize { b = 1, a = 2 } == "return {\n  a = 2,\n  b = 1,\n}\n", "The same table writes the same text")
check(Storage.deserialize("return os.exit()") == nil and Storage.deserialize("return { x = print }").x == nil,
    "A file can hold no code")
check(Storage.deserialize("while true do end return {}") == nil, "nor run forever")
check(Storage.deserialize("return 5") == nil and scratch:load("missing") == nil, "Only tables load")
check(not scratch:save("../escape", {}) and not scratch:save("a/b", {}), "Names stay inside the folder")
check(scratch:remove("sample") and scratch:load("sample") == nil, "Files can be removed")
check(Storage.new { name = "Surfers' Weather" }.dir:find("Surfers' Weather$"), "Each edition has its own folder")

-- Units (F2): metric inside, converted only in what is shown.
check(Units.regionOf("en-US") == "US" and Units.regionOf("en_GB.UTF-8") == "GB" and Units.regionOf("zh-Hant-TW") == "TW"
    and Units.regionOf("en") == nil and Units.regionOf(nil) == nil, "Regions from language tags")
local us, uk, sailors = Units.defaults(nil, "US"), Units.defaults(nil, "GB"), Units.defaults("sailing", "US")
check(us.temp == "f" and us.wind == "mph" and us.rain == "in" and us.clock == "12" and uk.temp == "c" and uk.wind == "mph"
    and sailors.wind == "kn" and sailors.temp == "f" and Units.defaults(nil, nil).wind == "ms", "Defaults by region and edition")
Units.use {}
check(Units.temp(12.4) == "12°" and Units.wind(5) == "5 m/s" and Units.gust(5, 7.2) == "5 g 7 m/s" and Units.height(1.46) == "1.5 m"
    and Units.height(2200) == "2200 m" and Units.rain(2) == "2.0 mm" and Units.pressure(1013.2) == "1013 hPa"
    and Units.pressure(-1, 1) == "-1.0 hPa" and Units.distance(20) == "20 km" and Units.clock("2026-11-18T06:00") == "06:00"
    and Units.span("2026-11-18T06:00", "2026-11-18T08:00") == "06–08h" and Units.temp(nil) == "—", "Metric text")
Units.use { temp = "f", wind = "kmh", height = "ft", rain = "in", pressure = "inhg", distance = "mi", clock = "12" }
check(Units.temp(10) == "50°" and Units.tempDelta(5) == "9°" and Units.wind(5) == "18 km/h" and Units.gust(5, 7.2) == "18 g 26 km/h"
    and Units.height(1.5) == "4.9 ft" and Units.rain(2) == "0.08 in" and Units.pressure(1013.25) == "29.92 inHg"
    and Units.distance(20) == "12 mi" and Units.clock("06:05") == "6:05 am" and Units.clock("00:00") == "12:00 am"
    and Units.clock("13:00") == "1:00 pm" and Units.span("x T06:00", "x T08:00") == "6–8 am"
    and Units.span("x T11:00", "x T13:00") == "11 am–1 pm", "Imperial text and a 12-hour clock")
check(Units.label("temp") == "°F" and Units.label("percent") == "%" and Units.value("wind", 10) == 36
    and Units.value("percent", 40) == 40, "Chart labels and values")
Units.use { wind = "kn" }
check(Units.wind(10) == "19 kn" and Units.gust(9.26, 13.4) == "18 g 26 kn", "Knots for wind sports")
Units.use { wind = "bft" }
check(Units.wind(5) == "3 Bft" and Units.wind(0.2) == "0 Bft" and Units.wind(40) == "12 Bft", "Beaufort forces")
Units.use { wind = "nonsense" }
check(Units.current("wind") == "ms", "An unknown unit falls back to metric")

-- Settings (F2): declarations, defaults by region, stored with Storage.
local settingsStore = F.storage("settings")
local probe = { id = "probe", title = "probe", settings = {
    { key = "mode", title = "mode", kind = "choice", default = "b", options = { "a", "b", { "c", "see" } } },
    { key = "on", title = "on", kind = "toggle", default = true },
    { key = "size", title = "size", kind = "number", default = 5, min = 1, max = 9, step = 2,
        format = function(v) return v .. " m" end },
} }
Settings.register(probe)
Settings.init { storage = settingsStore, region = "US" }
check(Settings.get("units", "temp") == "f" and Units.temp(10) == "50°", "A US region starts in Fahrenheit")
check(Settings.get("probe", "mode") == "b" and Settings.get("probe", "on") == true and Settings.get("probe", "size") == 5,
    "An activity's settings start at their defaults")
check(Settings.step("probe", "mode") and Settings.get("probe", "mode") == "c" and Settings.text("probe", "mode") == "see"
    and Settings.step("probe", "mode") and Settings.get("probe", "mode") == "a", "Choices cycle")
check(Settings.step("probe", "on") and Settings.get("probe", "on") == false and Settings.text("probe", "on") == "off",
    "Toggles flip, false included")
check(Settings.step("probe", "size", -1) and Settings.get("probe", "size") == 3 and Settings.set("probe", "size", 40)
    and Settings.get("probe", "size") == 9 and Settings.text("probe", "size") == "9 m", "Numbers step and clamp")
check(not Settings.set("probe", "mode", "z") and not Settings.set("probe", "missing", 1), "Unknown values are refused")
local revision = Settings.revision()
Settings.set("units", "wind", "kn")
check(Settings.revision() > revision and Units.wind(10) == "19 kn", "Changing a unit applies it")
Settings.init { storage = settingsStore, region = "NL" }
check(Settings.get("units", "wind") == "kn" and Settings.get("units", "temp") == "c" and Settings.get("probe", "on") == false,
    "Saved choices come back; the rest follow the region")
local probeRows = Settings.rows { probe }
check(#probeRows == 10 and probeRows[1].setting == "units/temp" and probeRows[8].note == "probe"
    and probeRows[10].stepper and probeRows[10].value == "9 m", "Rows for the settings page")
settingsStore:remove("settings")
Settings.init { region = "NL" }

-- A synthetic week from Open-Meteo, the shape the forecast API returns.
local weather, marine = F.weather, F.marine

-- Hours carry their UTC time; the extra fields arrive on every hour.
local parsed = Forecast.parse(weather(1), marine(), NOW + 1800)
check(parsed.hours[parsed.now].time == "2026-11-18T06:00" and parsed.hours[parsed.now].epoch == NOW
    and parsed.hours[parsed.now + 5].epoch == NOW + 5 * 3600, "Hour epochs count from the current hour")
check(parsed.elevation == 1200 and parsed.hours[1].freezingLevel == 2200 and parsed.hours[1].soilTemp == 6
    and parsed.hours[1].cloudHigh == 10 and parsed.hours[1].et0 == 0.1, "Extra fields parse")
check(Forecast.plausible(75, 11, 5) == 65 and Forecast.plausible(75, 1, 5) == 75 and Forecast.plausible(63, 20, 5) == 63,
    "Snow reported at 11 °C is rain; cold snow and plain rain stay")
local warmSnow = weather(1)
warmSnow.hourly.weather_code[7] = 75
warmSnow.daily.weather_code[1] = 75
local warmParsed = Forecast.parse(warmSnow, nil, NOW)
check(warmParsed.hours[7].code == 65 and warmParsed.days[1].code == 65, "Warm snow is corrected for hours and days")
local surfLoad, surfErr = Forecast.load({ lat = 1, lon = 1 }, true, function(url)
    if url:find("marine%-api") then return nil, "down" end return weather(1) end, NOW)
check(not surfLoad and surfErr == "down", "Surf needs the marine forecast")
local sailLoad = Forecast.load({ lat = 1, lon = 1 }, "optional", function(url)
    if url:find("marine%-api") then return nil, "down" end return weather(1) end, NOW)
check(sailLoad and not sailLoad.hasMarine, "Sailing goes on without waves")
local sunny = Forecast.parse(weather(39.09), nil, NOW, { lat = 39.09, lon = -120.04 })
check(sunny.days[1].twilight and sunny.days[1].twilight.civil.dawn and sunny.hours[sunny.now].sunAltitude < 0
    and sunny.hours[sunny.now + 12].sunAltitude ~= nil, "Each day gets its twilights and each hour the sun's height")

-- Forecast extension points (F4): extra columns, past days, quarter hours,
-- extra sources and one table of endpoints.
local request = { hourly = { "soil_temperature_6cm" }, daily = { "uv_index_max" }, minutely15 = { "precipitation" },
    marine = { "secondary_swell_wave_height" }, pastDays = 2, forecastDays = 16 }
local weatherUrl, marineUrl = Forecast.urls({ lat = 1, lon = 2 }, true, request)
check(weatherUrl:find("^https://api%.open%-meteo%.com/v1/forecast%?latitude=1&longitude=2")
    and weatherUrl:find("et0_fao_evapotranspiration,soil_temperature_6cm&", 1, true) and weatherUrl:find("sunset,uv_index_max", 1, true)
    and weatherUrl:find("minutely_15=precipitation", 1, true) and weatherUrl:find("forecast_days=16", 1, true)
    and weatherUrl:find("past_days=2", 1, true) and marineUrl:find("secondary_swell_wave_height", 1, true),
    "A request asks for its activities' columns: " .. weatherUrl)
Forecast.endpoints.apiKey = "secret"
local keyed = Forecast.urls({ lat = 1, lon = 2 }, false)
check(keyed:find("^https://customer%-api%.open%-meteo%.com/") and keyed:find("&apikey=secret$"), "A key moves to the customer host")
Forecast.endpoints.apiKey = nil
local function pastWeek()
    local base = Sky.epochOf("2026-11-16")
    local function at(i) return os.date("!%Y-%m-%dT%H:00", base + i * 3600) end
    local w = weather(1, {})
    w.hourly = { time = F.series(96, at), temperature_2m = F.series(96, function(i) return i end),
        soil_temperature_6cm = F.series(96, function() return 4.5 end) }
    w.daily = { time = { "2026-11-16", "2026-11-17", "2026-11-18", "2026-11-19" }, uv_index_max = { 1, 2, 3, 4 } }
    w.minutely_15 = { time = { "2026-11-18T06:00", "2026-11-18T06:15" }, precipitation = { 0, 0.4 } }
    return w
end
local past = Forecast.parse(pastWeek(), nil, NOW, { lat = 1, lon = 2 })
check(past.now == 55 and past.hours[past.now].temp == 54 and past.hours[1].epoch == NOW - 54 * 3600
    and past.hours[1].time == "2026-11-16T00:00", "Past hours keep their order before now")
check(past.today == 3 and past.days[3].uv_index_max == 3 and #Report.daysAhead(past) == 2
    and Report.daysAhead(past)[1].date == "2026-11-18", "Past days stay out of the days ahead")
check(past.hours[1].soil_temperature_6cm == 4.5, "An unknown column lands under its API name")
check(#past.quarters == 2 and past.quarters[2].precipitation == 0.4 and past.quarters[2].epoch == NOW + 900,
    "Quarter hours carry their time")
local airQuality = { id = "air", optional = true,
    url = function(spot, forecast) return forecast.url("airQuality", "latitude=" .. spot.lat) end,
    parse = function(json, forecast) forecast.aqi = json.aqi end }
local sourced, _, raw = Forecast.load({ lat = 1, lon = 2 }, false, function(url)
    if url:find("air%-quality") then return { aqi = 12 } end return weather(1) end, NOW, { sources = { airQuality } })
check(sourced.aqi == 12 and raw.sources.air.aqi == 12 and raw.weather, "A source adds its answer")
local missing = Forecast.load({ lat = 1, lon = 2 }, false, function(url)
    if url:find("air%-quality") then return nil, "down" end return weather(1) end, NOW, { sources = { airQuality } })
check(missing and missing.missing.air == "down", "An optional source may fail")
local needed = { id = "tides", url = function() return "https://tides.example/x" end, parse = function() end }
check(Forecast.load({ lat = 1, lon = 2 }, false, function(url)
    if url:find("tides") then return nil, "down" end return weather(1) end, NOW, { sources = { needed } }) == nil,
    "A required source must answer")
local restored = Forecast.restore(raw, { lat = 1, lon = 2 }, { sources = { airQuality } }, NOW + 3 * 3600)
check(restored.updated == NOW and restored.hours[restored.now].time == "2026-11-18T09:00" and restored.aqi == 12,
    "A saved answer replays as of a later hour")
check(Forecast.restore(raw, { lat = 1, lon = 2 }, nil, NOW + 9 * 86400) == nil, "A copy that no longer reaches now is dropped")
local Activities = require "Weather/activities"
local tahoe = F.PLACES[1]
-- Merged over the activities' own declarations (each edition grows its own),
-- so only what this test adds is checked, and theirs come back after.
local declared = {}
for _, activity in ipairs(Activities.list) do declared[activity] = { activity.fields, activity.sources } end
local function only(list, wanted)
    local count = 0
    for _, name in ipairs(list) do if name == wanted then count = count + 1 end end
    return count == 1
end
for _, activity in ipairs(Activities.list) do activity.fields, activity.sources = nil, nil end
Activities.byId.fishing.fields = { hourly = { "soil_temperature_6cm" }, pastDays = 2 }
Activities.byId.hiking.fields = { hourly = { "soil_temperature_6cm", "snow_depth" }, pastDays = 3, forecastDays = 10 }
Activities.byId.fishing.sources, Activities.byId.hiking.sources = { airQuality }, { airQuality }
local merged = Activities.request(tahoe)
check(table.concat(merged.hourly, ",") == "soil_temperature_6cm,snow_depth" and merged.pastDays == 3
    and merged.forecastDays == 10 and #merged.sources == 1 and only(merged.hourly, "snow_depth"),
    "One request merges the activities at a place")
for activity, saved in pairs(declared) do activity.fields, activity.sources = saved[1], saved[2] end

-- The app over a few places, with every navigation step a real control event.
local Places = require "Weather/places"
local Outdoors = Activities.byId.outdoors
for _, page in ipairs { { "Weather/Pages/Hub", "Weather/Data/Hub.xml" }, { "Weather/Pages/Activity", "Weather/Data/Activity.xml" },
        { "Weather/Pages/Settings", "Weather/Data/Settings.xml" }, { "Weather/Pages/Day", "Weather/Data/Day.xml" } } do
    local problems = Lint.page(page[1], page[2])
    check(#problems == 0, Lint.format(problems))
end
-- In the full app everyday outdoors' cards follow the temperature as
-- answers (F7); its own are checked below, after a stand-in here.
local everydayCards = Outdoors.cards
Outdoors.cards = function(_, place)
    return { { id = "today", title = "today", value = "a mild day", detail = "dry until " .. place.title .. "'s evening",
        tone = "good" } }
end
local net = F.network()
local requests = net.requests
-- The place's own forecast and marine requests; activities' extra sources
-- (tides, air, other models) come and go as the editions grow.
local function forecasts()
    local list = {}
    for _, url in ipairs(requests) do
        if url:find("v1/forecast%?latitude=[%-%d%.]+&longitude=[%-%d%.]+&hourly=temperature_2m,") or url:find("marine%-api")
        then list[#list + 1] = url end
    end
    return list
end
local store = F.storage("app", F.PLACES)
local ui = F.launch { net = net, storage = store }
local app, find, text, press, settle = ui.app, ui.find, ui.text, ui.press, ui.settle
local hub = app.screen

-- The hub: Amazing Weather's now page, what the place is good for, the week.
check(find("HubPanorama").Title == "Lake Tahoe", "The hub is titled with the place")
check(requests[1] == forecasts()[1] and forecasts()[1]:find("latitude=39.09") and forecasts()[1]:find("v1/forecast"),
    "The place's forecast comes first")
check(text("Nowtahoe:now:now", "HubTemp") == "10°" and text("Nowtahoe:now:now", "HubSky") == "partly cloudy"
    and text("Nowtahoe:now:now", "HubFeels") == "feels like 9°", "Now shows temperature, sky and feels like")
check(text("Detailtahoe:detail:1", "DetailValue") == "70%" and text("Detailtahoe:detail:6", "DetailValue") == "06:45",
    "Details list humidity and sunrise")
check(text("Striptahoe:strip:2026-11-18", "StripWeekday") == "wed" and find("Striptahoe:strip:2026-11-22")
    and not app.screen:findChild("Striptahoe:strip:2026-11-23", true), "The strip shows five days")
for _, id in ipairs { "outdoors", "fishing", "hiking", "stargazing" } do
    check(find("GoodFortahoe:activity:" .. id), "Lake Tahoe is good for " .. id)
end
check(not app.screen:findChild("GoodFortahoe:activity:surf", true), "Surf does not fit a lake")
check(text("GoodFortahoe:activity:fishing", "ActivityName") == "fishing"
    and text("GoodFortahoe:activity:fishing", "ActivityVerdict") == "good"
    and text("GoodFortahoe:activity:fishing", "ActivityWhy"):find("^best 06"), "Fishing is good now, at dawn: "
    .. text("GoodFortahoe:activity:fishing", "ActivityWhy"))
check(text("GoodFortahoe:activity:stargazing", "ActivityWhy"):find("^best 19") ~= nil,
    "Stars wait for tonight: " .. text("GoodFortahoe:activity:stargazing", "ActivityWhy"))
-- Hourly is charts first, as in Amazing Weather: temperature with wind, then rain.
local hourRows = app.hubScope:member("Hours").items:count()
check(hourRows == 24, "The hourly charts plot twenty-four hours: " .. hourRows)
check(find("TempChart").CategoryPath == "Short" and find("TempColumns").ValuePath == "Temp"
    and find("WindLine").ValuePath == "Wind" and find("RainChart").Maximum == 100, "Hub charts plot temperature, wind and rain")
local tickRow = app.store:get("WeatherRow", "tahoe:hour:2026-11-18T06:00")
check(tickRow and tickRow.values.Short == "06" and tickRow.values.Temp == 10 and tickRow.values.RainChance == 10,
    "Hour rows carry chart numbers and a tick every three hours")
check(app.store:get("WeatherRow", "tahoe:hour:2026-11-18T07:00").values.Short == "", "No tick between")
check(text("Daytahoe:day:2026-11-20", "Verdict") == "11° / 7°" or text("Daytahoe:day:2026-11-20", "Verdict"):find("/"),
    "Days show high and low")
check(text("Daytahoe:day:2026-11-19", "Why"):find(" for ") ~= nil, "Days name their best activity: "
    .. text("Daytahoe:day:2026-11-19", "Why"))
check(find("Placetahoe") and find("Placebondi"), "Places are listed")
-- Outdoors' answers follow the conditions; the age row comes next (F5, F7).
check(text("Answertahoe:answer:today", "AnswerValue") == "a mild day"
    and find("Answertahoe:answer:today").StateClasses == "good" and not app.screen:findChild("Cardtahoe:card:today", true),
    "Everyday outdoors answers under the temperature")
check(text("Agetahoe:age:age", "AgeText") == "updated just now" and not find("Agetahoe:age:age"):findChild("AgeWarning", true).Visible,
    "A fresh forecast says when it was updated")
check(find("TempColumns").Title == "temperature °C" and find("WindLine").Title == "wind m/s", "Hub charts name their units")

-- A day of the week opens its hours (W5), and Back comes home.
press("Daytahoe:day:2026-11-20")
check(app.screen ~= hub and find("DayPivot").Title == "fri 20 · Lake Tahoe", "A day opens its own page")
check(text("Headtahoe:2026-11-20:now:day", "DayTemps") == "15° / 7°" and text("Headtahoe:2026-11-20:now:day", "DaySky")
    == "thunderstorm" and text("Cardtahoe:2026-11-20:card:today", "AnswerValue") == "thunderstorms all day; up to 15°",
    "The day's sky and its line: " .. text("Cardtahoe:2026-11-20:card:today", "AnswerValue"))
check(app.activityScope:member("Hours").items:count() == 24 and find("TempColumns").Title == "temperature °C"
    and text("Hourtahoe:2026-11-20:hour:2026-11-20T13:00", "Verdict") == "12°", "and all of its hours")
check(text("GoodFortahoe:2026-11-20:activity:fishing", "ActivityVerdict") == "poor"
    and text("GoodFortahoe:2026-11-20:activity:outdoors", "ActivityVerdict") == "poor", "A thunderstorm day is good for nothing")
check(text("Detailtahoe:2026-11-20:detail:5", "DetailValue") == "17:00", "The day's readings")
press("GoodFortahoe:2026-11-20:activity:outdoors")
check(find("ActivityPivot").Title == "outdoors · Lake Tahoe", "A day's activity opens its page")
ui.back()
check(find("DayPivot") and app.activityScope, "and Back returns to the day")
ui.back("DayPivot")
check(app.screen == hub and not app.activityScope, "then to the hub")

-- An activity page reuses the hub's forecast and goes back to the same hub.
local hubRequests = #requests
press("GoodFortahoe:activity:fishing")
check(app.screen ~= hub and find("ActivityPivot").Title == "fishing · Lake Tahoe", "Fishing opens its own page")
check(find("ActivityPage").StateClasses == "fishing", "The page carries the activity's accent class")
check(#requests == hubRequests, "Opening an activity reuses the place's forecast")
check(text("Nowtahoe:fishing:now:now", "NowVerdict") == "good" and text("Nowtahoe:fishing:now:now", "NowSpot") == "Lake Tahoe",
    "The verdict card shows the activity's verdict")
local tile = find("Nowtahoe:fishing:now:now"):findChild("VerdictTile", true).BackgroundColor
check(math.abs(tile.R - 0x60 / 255) < 0.02 and math.abs(tile.G - 0xA9 / 255) < 0.02, "A good tile takes the fishing green")
check(text("Reasontahoe:fishing:reason:1", "ReasonText"):find("Dawn"), "Reasons list why")
check(text("Daytahoe:fishing:day:2026-11-20", "Verdict") == "poor", "A thunderstorm day is poor fishing")
check(text("Conditiontahoe:fishing:metric:1", "MetricValue") == "1018 hPa", "Conditions show pressure")
-- Today: a 24-hour verdict band and the two charts fishing watches.
check(find("Bandtahoe:fishing:hour:2026-11-18T06:00").StateClasses == "good"
    and find("Bandtahoe:fishing:hour:2026-11-19T05:00"), "The band colours twenty-four hours by verdict")
check(find("ChartALine").Visible and not find("ChartAColumns").Visible and find("ChartALine").Title == "pressure hPa",
    "Fishing charts pressure as a line")
check(find("ChartBColumns").Visible and find("ChartBColumns").Title == "wind m/s", "and wind as columns")
local fishingHour = app.store:get("WeatherRow", "tahoe:fishing:hour:2026-11-18T06:00")
check(fishingHour and math.abs(fishingHour.values.Metric1 - 1018.2) < 0.01 and fishingHour.values.Metric2 == 3,
    "Hour rows carry the activity's chart values")
find("ActivityPivot"):post("Pivot.BackRequested")
settle()
check(app.screen == hub and not app.activityScope, "Back returns to the same hub and closes the activity")
press("GoodFortahoe:activity:outdoors")
check(text("Cardtahoe:outdoors:card:today", "CardTitle") == "today" and find("Agetahoe:outdoors:age:age"),
    "An activity page shows its cards and the forecast's age")
find("ActivityPivot"):post("Pivot.BackRequested")
settle()
press("GoodFortahoe:activity:stargazing")
check(text("Nowtahoe:stargazing:now:now", "NowSpot") == "Lake Tahoe · from 19:00", "The stars card shows tonight")
check(find("ChartA").Minimum == 0 and find("ChartA").Maximum == 100 and find("ChartAColumns").Title == "clouds %",
    "Percent charts run from 0 to 100")
find("ActivityPivot"):post("Pivot.BackRequested")
settle()

-- Another place: its hub from the first section, with the sea for surf.
find("HubPanorama").SelectedIndex = 4
press("Placeocean-beach")
check(find("HubPanorama").Title == "Ocean Beach" and find("HubPanorama").SelectedIndex == 0,
    "Choosing a place shows its hub from the start")
check(forecasts()[#forecasts()]:find("marine%-api") and forecasts()[#forecasts()]:find("latitude=37.76"),
    "Surf adds the marine forecast")
check(find("GoodForocean-beach:activity:surf") and not app.screen:findChild("GoodForocean-beach:activity:fishing", true),
    "Ocean Beach is good for surf, not fishing")
press("GoodForocean-beach:activity:surf")
-- Ocean Beach faces west; the synthetic wind is from the east: offshore.
-- Surf's own verdicts are checked in tests/test_weather_surf.lua.
check(text("Hourocean-beach:surf:hour:2026-11-18T06:00", "Verdict") == "poor", "No surf in the dark")
check(text("Hourocean-beach:surf:hour:2026-11-18T08:00", "Why"):find("offshore"), "Hours say the wind on the beach")
check(text("Conditionocean-beach:surf:metric:4", "MetricValue") == "14°", "Water temperature")
find("ActivityPivot"):post("Pivot.BackRequested")
settle()
press("Placebondi")
press("GoodForbondi:activity:surf")
-- Bondi faces east: the same east wind is onshore and the westerly swell misses it.
check(text("Hourbondi:surf:hour:2026-11-18T08:00", "Verdict") ~= "good", "Wind and swell depend on the beach")
find("ActivityPivot"):post("Pivot.BackRequested")
settle()
net.failing = true
press("RefreshReport")
check(find("Nowbondi:now:now") and not find("ReportFailed").Visible, "A failed refresh keeps the last report")
check(text("Agebondi:age:age", "AgeWarning"):find("^offline") and find("Agebondi:age:age").StateClasses == "poor",
    "and says it is offline")
net.failing = false

-- Settings (F3): units from declarations, each activity's own, and about.
local Fishing = Activities.byId.fishing
Fishing.settings = {
    { key = "moon", title = "moon in the score", kind = "toggle", default = false, detail = "anglers' lore, not science" },
    { key = "depth", title = "depth", kind = "number", default = 2, min = 0, max = 3, step = 0.5 },
}
press("OpenSettings")
check(find("SettingsPivot").Title == "weather" and app.screen ~= hub, "Settings open from the places section")
local function setting(id) return "Settingsettings:" .. id end
check(text(setting("units:temp"), "SettingTitle") == "temperature" and text(setting("units:temp"), "SettingValue") == "°C"
    and text(setting("units:wind"), "SettingValue") == "m/s" and text(setting("units:clock"), "SettingValue") == "24-hour",
    "Units start metric in a metric region")
check(not find(setting("units:temp")):findChild("SettingLess", true).Visible, "Choices have no stepper")
check(text(setting("fishing:moon"), "SettingHeading") == "fishing" and text(setting("fishing:moon"), "SettingValue") == "off"
    and not find(setting("fishing:depth")):findChild("SettingHeading", true).Visible
    and find(setting("fishing:depth")):findChild("SettingMore", true).Visible, "Activity settings come under their name")
check(not find("NoActivitySettings").Visible, "The empty line hides when an activity has settings")
check(text("Aboutsettings:about:1", "AboutTitle") == "Weather data by Open-Meteo.com", "About credits Open-Meteo")
local function tap(row, button) find(row):findChild(button, true):send("Node.LeftButtonUp"); settle() end
tap(setting("units:temp"), "SettingChange")
check(text(setting("units:temp"), "SettingValue") == "°F" and Settings.get("units", "temp") == "f", "A tap picks the next unit")
tap(setting("fishing:moon"), "SettingChange")
tap(setting("fishing:depth"), "SettingMore")
tap(setting("fishing:depth"), "SettingMore")
tap(setting("fishing:depth"), "SettingMore")
check(Settings.get("fishing", "moon") == true and Settings.get("fishing", "depth") == 3
    and text(setting("fishing:depth"), "SettingValue") == "3", "Toggles flip; numbers step and stop at their max")
tap(setting("fishing:depth"), "SettingLess")
check(Settings.get("fishing", "depth") == 2.5, "less steps down")
local stored = store:load("settings")
check(stored and stored.units.temp == "f" and stored.fishing.moon == true, "Settings are saved at once")
local before = #requests
find("SettingsPivot"):post("Pivot.BackRequested")
settle()
check(app.screen == hub and text("Nowbondi:now:now", "HubTemp") == "50°" and find("TempColumns").Title == "temperature °F",
    "Back on the hub, temperatures follow the new unit")
check(#requests == before, "Redrawing for new units reuses the forecast")
Settings.set("units", "temp", "c")
Fishing.settings = nil
app.hubScope:dispose()

-- Offline (F5): a new start with no network shows the copy saved on the
-- device, with its age; the saved hub rows also show while a request runs.
net.failing = true
local later = F.launch { net = net, storage = store, clock = function() return NOW + 2 * 3600 end }
check(later.text("Nowtahoe:now:now", "HubTemp") == "11°" and later.text("Agetahoe:age:age", "AgeText") == "updated 2 h ago"
    and later.text("Agetahoe:age:age", "AgeWarning"):find("offline"), "Offline, the saved forecast shows with its age")
local savedRows = later.app.provider.caches.savedHub { PlaceId = "ocean-beach" }
check(savedRows and #savedRows > 0, "The saved hub can show before the first request")
net.failing = false
later.app.hubScope:dispose()
Outdoors.cards = everydayCards

-- Everyday answers (W1, W2): the next two hours of rain from the quarter
-- hours, the rest of the day in one line, and what to bring.
local BERLIN, TOKYO = { lat = 52.52, lon = 13.4 }, { lat = 35.68, lon = 139.7 }
-- A day at a spot with quarter-hour rain (mm per quarter from 06:00) and
-- hourly changes `hourly(i, w)`.
local function everyday(spot, rain, hourly, clock)
    local w = weather(spot.lat)
    local times, amounts, snow = {}, {}, {}
    for i = 0, 15 do
        times[i + 1] = ("2026-11-18T%02d:%02d"):format(6 + i * 15 // 60, i * 15 % 60)
        amounts[i + 1], snow[i + 1] = rain and rain[i + 1] or 0, 0
    end
    w.minutely_15 = { time = times, precipitation = amounts, snowfall = snow }
    if hourly then for i = 1, #w.hourly.time do hourly(i - 1, w) end end
    local forecast = Forecast.parse(w, nil, clock or NOW, spot)
    forecast.spot = spot
    return forecast, w
end
check(Outdoors.preciseNowcast(BERLIN) and Outdoors.preciseNowcast({ lat = 40.7, lon = -74 })
    and not Outdoors.preciseNowcast(TOKYO) and not Outdoors.preciseNowcast({ lat = -33.9, lon = 151.2 }),
    "Real quarter hours: North America and Central Europe")
local function nextHour(spot, rain, clock) return Outdoors.nextHour(everyday(spot, rain, nil, clock), clock or NOW) end
check(nextHour(BERLIN, {}).value == "dry for the next 2 hours" and nextHour(BERLIN, {}).tone == "good", "A dry outlook")
local soon = nextHour(BERLIN, { 0, 0, 0, 0.3, 0.4, 0.5, 0.2 })
check(soon.value == "rain starts in about 30 min" and soon.detail == "light, for about 1 h" and soon.tone == "fair",
    "Rain from 06:30 to 07:30: " .. soon.value .. " · " .. soon.detail)
check(nextHour(BERLIN, { 0, 0, 0, 0.3 }, NOW + 7 * 60).value == "rain starts in about 25 min",
    "Minutes count from the time now, not from the request")
local stopping = nextHour(BERLIN, { 0, 0.3, 0.4, 0.5, 0.2 })
check(stopping.value == "rain stops in about 1 h" and stopping.detail:find("^light"), "Rain now: when it stops")
local pouring = nextHour(BERLIN, { 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 })
check(pouring.value == "rain for the next 2 hours" and pouring.detail == "heavy" and pouring.tone == "poor", "A wet outlook")
local rough = nextHour(TOKYO, { 0, 0, 0, 0.3, 0.4, 0.5, 0.2 })
check(rough.value == "rain likely within the hour" and rough.detail:find("approximate") and not rough.value:find("min"),
    "Elsewhere the timing is rough, never in minutes: " .. rough.value)
check(Outdoors.nextHour(Forecast.parse(weather(52.5), nil, NOW), NOW) == nil, "No quarter hours, no next-hour answer")
local function answers(forecast)
    local byId = {}
    for _, card in ipairs(Outdoors.today(forecast)) do byId[card.id] = card end
    return byId
end
local plain = answers(everyday(BERLIN))
check(plain.today.title == "today" and plain.today.value == "dry all day; up to 15°" and plain.today.detail == "feels 9° to 14°",
    "Today in one line: " .. plain.today.value)
check(plain.bring.value == "a coat" and plain.bring.detail == "feels 9° at the coldest", "Bring a coat when it feels 9°")
local showers = answers(everyday(BERLIN, nil, function(i, w)
    if i >= 15 and i < 24 then w.hourly.weather_code[i + 1] = 81 end
    if i == 12 then w.hourly.uv_index[i + 1] = 5 end
end))
check(showers.today.value == "dry until 15:00, then showers; up to 15°" and showers.today.detail:find("uv up to 5"),
    "Dry, then showers: " .. showers.today.value)
check(showers.bring.value == "an umbrella, a coat, sunscreen" and showers.bring.detail:find("^showers from 15:00"),
    "Bring an umbrella and sunscreen: " .. showers.bring.value)
local gale = answers(everyday(BERLIN, nil, function(i, w)
    if i >= 8 and i < 11 then w.hourly.weather_code[i + 1] = 63 end
    if i >= 13 and i < 15 then w.hourly.weather_code[i + 1] = 63 end
    w.hourly.wind_gusts_10m[i + 1] = 17
    w.hourly.apparent_temperature[i + 1] = -2
end))
check(gale.today.value == "rain at times, most 08–10h; up to 15°, windy", "Rain on and off: " .. gale.today.value)
check(gale.bring.value == "a hood, a warm coat, hat and gloves" and gale.bring.detail:find("too windy for an umbrella"),
    "Gales call for a hood: " .. gale.bring.value)
local evening = answers(everyday(BERLIN, nil, nil, NOW + 15 * 3600))
check(evening.today.title == "tomorrow", "Late in the evening the line is about tomorrow")
-- Sixteen days (W5): after a week the days say they are less certain.
local function fortnight(days)
    local w = weather(1)
    for name, values in pairs(w.hourly) do
        for i = #values + 1, days * 24 do values[i] = name == "time" and F.iso(i - 1) or values[i - 24] end
    end
    for name, values in pairs(w.daily) do
        for d = #values + 1, days do
            values[d] = name == "time" and ("2026-11-%02d"):format(17 + d)
                or (name == "sunrise" or name == "sunset") and values[d - 1]:gsub("11%-%d%d", ("11-%02d"):format(17 + d))
                or values[d - 1]
        end
    end
    return w
end
local long = Forecast.parse(fortnight(10), nil, NOW)
long.spot = F.PLACES[1]
local hubRows = {}
for _, row in ipairs(Report.hub(F.PLACES[1], long, { Outdoors }, NOW)) do hubRows[row.id] = row end
check(Outdoors.fields.forecastDays == 16 and hubRows["tahoe:day:2026-11-24"].detail:find("^good for outdoors")
    and hubRows["tahoe:day:2026-11-25"].detail:find("^less certain · good for outdoors"),
    "Days after the first week are less certain: " .. hubRows["tahoe:day:2026-11-25"].detail)
local dayRows = {}
for _, row in ipairs(Report.day(F.PLACES[1], long, "2026-11-26", { Outdoors })) do dayRows[row.id] = row end
check(dayRows["tahoe:2026-11-26:now:day"].detail:find("^less certain") and dayRows["tahoe:2026-11-26:now:day"].tone == "fair",
    "and so does their page")

-- Yesterday (W4): `pastDays = 1` brings the same hour a day earlier.
local function withYesterday(change)
    local w = weather(1)
    for name, values in pairs(w.hourly) do
        local copy = {}
        for i = 1, 24 do copy[i] = name == "time" and values[i]:gsub("11%-18", "11-17") or values[i] end
        if name == "temperature_2m" then for i = 1, 24 do copy[i] = values[i] + change end end
        for i, v in ipairs(values) do copy[24 + i] = v end
        w.hourly[name] = copy
    end
    for name, values in pairs(w.daily) do
        table.insert(values, 1, name == "time" and "2026-11-17" or name == "sunrise" and "2026-11-17T06:45"
            or name == "sunset" and "2026-11-17T17:00" or values[1])
    end
    return w
end
check(Report.yesterday(Forecast.parse(withYesterday(4), nil, NOW)) == "4° colder than yesterday"
    and Report.yesterday(Forecast.parse(withYesterday(-2.6), nil, NOW)) == "3° warmer than yesterday"
    and Report.yesterday(Forecast.parse(withYesterday(1), nil, NOW)) == "about as warm as yesterday"
    and Report.yesterday(Forecast.parse(weather(1), nil, NOW)) == nil, "Now against this time yesterday")
check(Outdoors.fields.pastDays == 1, "Outdoors asks for yesterday")

-- On the hub, under the temperature.
local berlin = { id = "berlin", title = "Berlin", region = "Germany", lat = 52.52, lon = 13.4, activities = {} }
local everydayNet = F.network()
everydayNet.answers["v1/forecast"] = function() return (select(2, everyday(BERLIN, { 0, 0, 0, 0.3, 0.4, 0.5, 0.2 }))) end
local daily = F.launch { places = { berlin }, net = everydayNet, storageName = "everyday" }
check(everydayNet.requests[1]:find("minutely_15=precipitation,snowfall", 1, true)
    and everydayNet.requests[1]:find("precipitation_hours,uv_index_max", 1, true), "Outdoors asks for quarter hours")
check(daily.text("Answerberlin:answer:next-hour", "AnswerValue") == "rain starts in about 30 min"
    and daily.text("Answerberlin:answer:today", "AnswerValue") == "dry all day; up to 15°"
    and daily.text("Answerberlin:answer:bring", "AnswerValue") == "a coat", "The hub answers the door-step questions")
check(daily.find("AnswerItems").Parent == daily.find("NowItems").Parent, "beside the conditions")
daily.app.hubScope:dispose()

-- The real places: unique, known activities, and a shore for surf and sailing.
local seen = {}
for _, place in ipairs(Places) do
    check(not seen[place.id] and place.title and place.lat and place.lon, "Place " .. place.id .. " is complete")
    seen[place.id] = true
    for _, id in ipairs(place.activities) do
        check(Activities.byId[id], place.id .. " names a known activity " .. id)
        if id == "surf" or id == "sailing" then check(place.facing, place.id .. " says which way its shore faces") end
    end
end

-- An edition (EDITION=surf bundles Weather/build.lua) keeps everyday
-- outdoors and its activity, at the places that offer it.
local MODULES = { "Weather/build", "Weather/activities", "Weather/places" }
local saved = {}
for _, name in ipairs(MODULES) do saved[name] = package.loaded[name]; package.loaded[name] = nil end
package.loaded["Weather/build"] = { edition = "surf" }
local SurfActivities, SurfPlaces = require "Weather/activities", require "Weather/places"
check(SurfActivities.edition == "surf" and #SurfActivities.list == 2 and SurfActivities.list[1].id == "outdoors"
    and SurfActivities.byId.surf and not SurfActivities.byId.fishing, "A surf edition rates outdoors and surf")
check(#SurfPlaces > 0 and #SurfPlaces < #Places, "A surf edition keeps fewer places")
for _, place in ipairs(SurfPlaces) do
    local ids = {}
    for _, activity in ipairs(SurfActivities.at(place)) do ids[#ids + 1] = activity.id end
    check(table.concat(ids, ",") == "outdoors,surf", place.id .. " offers surf in the surf edition")
end
for _, name in ipairs(MODULES) do package.loaded[name] = saved[name] end

-- package.lua and activities.lua name the editions alike.
local package = assert(io.open("apps/Weather/package.lua", "r+b")):read("a")
for key, name in pairs(Activities.names) do
    check(package:find(('{ Key = "%s", Name = "%s" }'):format(key, name), 1, true), "package.lua names " .. key)
end
check(Activities.appName == "Weather", "The full app is plain Weather")

F.done("weather")
