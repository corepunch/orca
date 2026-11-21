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
local usMetric = Units.defaults(nil, "US", { measurement = "metric", clock24 = true })
local nl12 = Units.defaults(nil, "NL", { measurement = "metric", clock24 = false })
local ukPhone = Units.defaults(nil, nil, { measurement = "uk", clock24 = true })
check(usMetric.temp == "c" and usMetric.wind == "ms" and usMetric.clock == "24" and nl12.clock == "12" and nl12.temp == "c"
    and ukPhone.wind == "mph" and ukPhone.temp == "c" and Units.defaults("sailing", "US", { measurement = "us" }).wind == "kn",
    "The phone's own measurement system and clock win over its region")
do
    -- The engine's region functions: ORCA_REGION decides when set (tests and
    -- previews); Units.device() then stays out of the way.
    local core = require "orca.core"
    check(type(core.getRegion()) == "string" and ({ metric = true, us = true, uk = true })[core.getMeasurementSystem()]
        and type(core.uses24HourClock()) == "boolean", "The engine says the device's region, measurement and clock")
    if os.getenv("ORCA_REGION") == nil then
        local device = Units.device()
        check(device == nil or (device.measurement and type(device.clock24) == "boolean"), "Units.device reads the engine")
    end
end
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
check(#probeRows == 11 and probeRows[1].setting == "units/temp" and probeRows[8].setting == "appearance/photos"
    and probeRows[8].value == "on" and probeRows[9].note == "probe"
    and probeRows[11].stepper and probeRows[11].value == "9 m", "Rows include the default photo choice for existing settings")
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
-- I8: every Open-Meteo host moves with the key; NOAA's stays as it is.
check(Forecast.url("flood", "x=1") == "https://customer-flood-api.open-meteo.com/v1/flood?x=1&apikey=secret"
    and Forecast.url("archive", "x=1"):find("^https://customer%-archive%-api%.open%-meteo%.com/")
    and Forecast.url("noaaTides", "x=1") == "https://api.tidesandcurrents.noaa.gov/api/prod/datagetter?x=1",
    "A key switches every Open-Meteo host, and only those")
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
-- Several models in the main call: the best match reads as the plain column.
local several = weather(1, {})
several.hourly.temperature_2m_best_match, several.hourly.temperature_2m = several.hourly.temperature_2m, nil
several.hourly.temperature_2m_gfs_seamless = F.series(#several.hourly.time, function() return -3 end)
several.daily.temperature_2m_max_best_match, several.daily.temperature_2m_max = several.daily.temperature_2m_max, nil
local modelled = Forecast.parse(several, nil, NOW, { lat = 1, lon = 2 })
check(modelled.hours[modelled.now].temp == weather(1, {}).hourly.temperature_2m[modelled.now]
    and modelled.hours[1].temperature_2m_gfs_seamless == -3 and modelled.days[1].max == 15,
    "The best match keeps the plain names; other models land under theirs")
-- I4: every request names the app, sends no cookies, and a source adds headers.
local seen = {}
local alerting = { id = "alerts", optional = true, headers = { Accept = "application/geo+json" },
    url = function() return "https://api.weather.gov/alerts/active?point=1,2" end, parse = function() end }
Forecast.load({ lat = 1, lon = 2 }, false, function(url, options)
    seen[#seen + 1] = options
    return weather(1) end, NOW, { sources = { alerting } })
check(#seen == 2 and seen[1].nocookies and seen[1].headers["User-Agent"] == Forecast.userAgent
    and seen[2].headers.Accept == "application/geo+json" and seen[2].headers["User-Agent"] == Forecast.userAgent
    and not Forecast.userAgent:find("@"), "Requests carry a generic User-Agent and a source's own headers")
check(Forecast.userAgent == "Weather/1.0 (ORCA)", "The full app's User-Agent is its name: " .. Forecast.userAgent)
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
local merged = Activities.request(tahoe, "all")
check(table.concat(merged.hourly, ",") == "soil_temperature_6cm,snow_depth" and merged.pastDays == 3
    and merged.forecastDays == 10 and #merged.sources == 1 and only(merged.hourly, "snow_depth"),
    "One request merges the activities at a place")
-- I1: an optional source comes only from the lead (outdoors here) and the
-- activity whose page is open; a required one always.
local tides = { id = "tides", url = function() return nil end, parse = function() end }
Activities.byId.fishing.sources, Activities.byId.hiking.sources = { airQuality }, { tides }
check(#Activities.request(tahoe).sources == 1 and Activities.request(tahoe).sources[1] == tides,
    "The hub asks only for required sources and the lead's")
check(#Activities.request(tahoe, "fishing").sources == 2 and #Activities.request(tahoe, Activities.byId.fishing).sources == 2,
    "An open activity page adds its optional sources")
Activities.byId.hiking.fields = { minutely15 = { "precipitation" }, minutely15Steps = 8,
    models = function(place) return { "gfs_seamless", place.lake and "icon_seamless" or nil } end }
Activities.byId.fishing.fields = { minutely15Steps = 12, models = { "gfs_seamless" } }
local stepped = Activities.request(tahoe)
check(stepped.minutely15Steps == 12 and table.concat(stepped.models, ",") == "gfs_seamless,icon_seamless",
    "Quarter-hour steps and weather models merge")
local steppedUrl = Forecast.urls(tahoe, false, stepped)
check(steppedUrl:find("&forecast_minutely_15=12&past_minutely_15=0", 1, true)
    and steppedUrl:find("&models=best_match,gfs_seamless,icon_seamless", 1, true), "They go into the main call: " .. steppedUrl)
for activity, saved in pairs(declared) do activity.fields, activity.sources = saved[1], saved[2] end

-- I5: a night activity groups hours by the evening their night began
-- (`nightOf`), and its best window runs on across midnight.
local nightly = { id = "nightly", nightOf = function(hour)
        local h = Sky.hour(hour.time)
        return h < 12 and os.date("!%Y-%m-%d", Sky.epochOf(hour.date) - 86400) or hour.date
    end,
    rate = function(hour) return { result = function()
        local h = Sky.hour(hour.time)
        local good = h >= 22 or h <= 1
        return { score = good and 90 or 20, verdict = good and "good" or "poor", reasons = {} }
    end } end }
local nightForecast = Forecast.parse(weather(39.09), nil, NOW, tahoe)
local _, nightSpan, nightBest = Report.bestOfDay(nightly, nightForecast, "2026-11-18")
check(nightSpan == Units.span("2026-11-18T22:00", "2026-11-19T01:00") and nightBest.time == "2026-11-18T22:00",
    "Tonight's window runs across midnight: " .. tostring(nightSpan))
local _, daySpan = Report.bestOfDay(setmetatable({ nightOf = false }, { __index = nightly }), nightForecast, "2026-11-18")
check(daySpan == Units.span("2026-11-18T22:00", "2026-11-18T23:00"), "Without nightOf the day ends at midnight")

-- I2: an edition's cards know where they show; the hub keeps the three most
-- urgent (by `rank`, else poor and warnings first), in the activity's order.
local wheres = {}
local lens = setmetatable({ id = "lens", cards = function(_, _, _, where)
    wheres[#wheres + 1] = where
    return { { id = "a", tone = "good" }, { id = "b", tone = "info" }, { id = "c", tone = "poor" },
        { id = "d", tone = "fair", hub = false }, { id = "e", tone = "fair" }, { id = "f", tone = "info", rank = 0,
        actions = { { "go", "go" }, { id = "stay", text = "stay" } } } }
end }, { __index = Activities.byId.outdoors })
local lensForecast = Forecast.parse(weather(39.09), nil, NOW, tahoe)
local hubCards = {}
for _, row in ipairs(Report.hub(tahoe, lensForecast, { lens }, NOW, lens)) do
    if row.kind == "card" then hubCards[#hubCards + 1] = row.id:match("[^:]+$") end
end
check(table.concat(hubCards, ",") == "c,e,f" and wheres[1] == "hub", "The hub keeps three cards: " .. table.concat(hubCards, ","))
local pageCards, actionRow = {}, nil
for _, row in ipairs(Report.rows(lens, tahoe, lensForecast, NOW)) do
    if row.kind == "card" then pageCards[#pageCards + 1] = row.id:match("[^:]+$"); actionRow = row end
end
check(table.concat(pageCards, ",") == "a,b,c,d,e,f" and wheres[2] == "activity", "The page keeps every card")
check(actionRow.action1 == "go" and actionRow.action2_text == "stay" and actionRow.action3 == nil,
    "A card's actions become its links")

-- The app over a few places, with every navigation step a real control event.
local Places = require "Weather/places"
local Outdoors = Activities.byId.outdoors
for _, page in ipairs { { "Weather/Pages/Hub", "Weather/Data/Hub.xml" }, { "Weather/Pages/Activity", "Weather/Data/Activity.xml" },
        { "Weather/Pages/Settings", "Weather/Data/Settings.xml" }, { "Weather/Pages/Day", "Weather/Data/Day.xml" },
        { "Weather/Pages/Search", "Weather/Data/Search.xml" }, { "Weather/Pages/Place", "Weather/Data/Place.xml" } } do
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
F.checkHubPhoto(ui)
check(find("HubPanorama").Title == "Lake Tahoe", "The hub is titled with the place")
check(requests[1] == forecasts()[1] and forecasts()[1]:find("latitude=39.09") and forecasts()[1]:find("v1/forecast"),
    "The place's forecast comes first")
-- I1: a lake asks no marine forecast, and the hub only outdoors' own sources
-- (air and warnings at a US place), not the other activities'.
check(#forecasts() == 1 and #requests == 3 and requests[2]:find("air%-quality") and requests[3]:find("api.weather.gov"),
    "The hub asks for three things at Lake Tahoe: " .. table.concat(requests, " "))
check(requests[1]:find("&forecast_minutely_15=12", 1, true), "The nowcast asks for three hours of quarter hours")
check(text("Nowtahoe:now:now", "HubTemp") == "10°" and text("Nowtahoe:now:now", "HubSky") == "partly cloudy"
    and text("Nowtahoe:now:now", "HubFeels") == "feels like 9°", "Now shows temperature, sky and feels like")
check(text("Detailtahoe:detail:1", "DetailValue") == "70%" and text("Detailtahoe:detail:6", "DetailValue") == "06:45",
    "Details list humidity and sunrise")
check(text("Striptahoe:strip:2026-11-18", "StripWeekday") == "wed" and find("Striptahoe:strip:2026-11-22")
    and not app.screen:findChild("Striptahoe:strip:2026-11-23", true), "The strip shows five days")
do
    local strip, details = find("StripItems"), find("DetailItems")
    local originalWidth = hub.Width
    for _, width in ipairs { 320, 390, 430 } do
        hub.Width = width
        hub:UpdateLayout(width, hub.Height)
        check(strip.ActualX == details.ActualX and strip.ActualWidth == details.ActualWidth,
            "The forecast strip fills the same panel width as the details")
        local day = strip:getFirstChild()
        local cellWidth = (strip.ActualWidth - 4 * strip.Spacing) / 5
        for index = 0, 4 do
            check(day and math.abs(day.ActualWidth - cellWidth) < 1
                and math.abs(day.ActualX - index * (cellWidth + strip.Spacing)) < 1,
                "The five forecast days share the available width equally")
            day = day:getNext()
        end
    end
    hub.Width = originalWidth
    settle()
end
for _, id in ipairs { "outdoors", "fishing", "hiking", "stargazing" } do
    check(find("GoodFortahoe:activity:" .. id), "Lake Tahoe is good for " .. id)
end
check(not app.screen:findChild("GoodFortahoe:activity:surf", true), "Surf does not fit a lake")
check(text("GoodFortahoe:activity:fishing", "ActivityName") == "fishing"
    and text("GoodFortahoe:activity:fishing", "ActivityVerdict") == "good"
    and text("GoodFortahoe:activity:fishing", "ActivityWhy"):find("^best 06"), "Fishing is good now, at dawn: "
    .. text("GoodFortahoe:activity:fishing", "ActivityWhy"))
-- I5: a night is one night, so tonight's best may come after midnight.
local starsFrom = tonumber(text("GoodFortahoe:activity:stargazing", "ActivityWhy"):match("^best (%d%d)"))
check(starsFrom and (starsFrom >= 18 or starsFrom < 6),
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
check(find("HubBandTitle").Text == "outdoors, next 24 hours" and find("Bandtahoe:hour:2026-11-18T06:00")
    and tickRow.values.Tone ~= "info" and find("Bandtahoe:hour:2026-11-18T06:00").StateClasses == tickRow.values.Tone,
    "The hourly section opens with outdoors' verdict band")
-- W11: VoiceOver reads a band cell's hour and verdict, a row as one
-- sentence with a hint, and the hub's visible elements in order.
do
    local UIKit = require "orca.UIKit"
    local band9, row9 = find("Bandtahoe:hour:2026-11-18T09:00"), app.store:get("WeatherRow", "tahoe:hour:2026-11-18T09:00")
    check(UIKit.getAccessibilityLabel(find("Bandtahoe:hour:2026-11-18T06:00")):find("^now ")
        and UIKit.getAccessibilityLabel(band9) == "09:00 " .. row9.values.Tone,
        "A band cell says its hour and verdict: " .. UIKit.getAccessibilityLabel(band9))
    local fishingRow = find("GoodFortahoe:activity:fishing")
    check(UIKit.getAccessibilityLabel(fishingRow):find("^fishing, good, best 06") and fishingRow.AccessibilityHint == "opens the activity",
        "A good-for row reads as one sentence: " .. UIKit.getAccessibilityLabel(fishingRow))
    check(UIKit.getAccessibilityLabel(find("HubTemp")) == "10°", "Plain text reads as itself")
    local spoken = UIKit.describeAccessibility(app.screen)
    check(spoken:find("^Header\tlake tahoe\nHeader\tnow\nStaticText\t10°\n") and spoken:find("\nNone\thumidity, 70%%\n")
        and spoken:find("\nHeader\tgood for\nButton\toutdoors, good, "),
        "The hub reads its title, each section's header, then the section:\n" .. spoken)
    check(find("TempChart").AccessibilityLabel:find("^chart of the next 24 hours: temperature"), "Charts are named for VoiceOver")
end
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
-- I10: every activity's accent is in the sheet, in step with activities.lua,
-- and poor text reads at 4.5:1 on black.
local sheet = assert(io.open("apps/Weather/Styles/weather.css", "r+b")):read("a")
for _, activity in ipairs(Activities.list) do
    check(sheet:find("." .. activity.id .. " .wx-accent { color: " .. activity.accent .. "; }", 1, true),
        activity.id .. " has its accent in the sheet")
end
check(sheet:find(".wx-poor { color: #f01a00; }", 1, true), "Poor text is the lighter red")
-- I1: the page adds only fishing's own optional sources; the forecast stays.
local added = {}
for index = hubRequests + 1, #requests do added[#added + 1] = requests[index] end
check(#forecasts() == 1 and not table.concat(added, " "):find("v1/forecast%?latitude=39.09&longitude=%-120.04&hourly=temperature"),
    "Opening an activity reuses the place's forecast: " .. table.concat(added, " "))
local fishingSources = 0
for _, source in ipairs(Activities.byId.fishing.sources or {}) do
    if source.url(F.PLACES[1], Forecast) then fishingSources = fishingSources + 1 end
end
check(#added == fishingSources, "and asks for fishing's own sources: " .. #added)
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
check(find("ChartALine").Visible and not find("ChartAColumns").Visible
    and find("ChartALine").Title == "pressure hPa · 2 days back", "Fishing charts pressure as a line: " .. find("ChartALine").Title)
check(find("ChartBColumns").Visible and find("ChartBColumns").Title == "wind m/s" and not find("ChartC").Visible,
    "and wind as columns; no third chart")
local fishingHour = app.store:get("WeatherRow", "tahoe:fishing:chart:2026-11-18T06:00")
check(fishingHour and math.abs(fishingHour.values.Metric1 - 1018.2) < 0.01 and fishingHour.values.Metric2 == 3,
    "Chart rows carry the activity's chart values")
-- I5: fishing charts two days back (`chartHours`): here, every hour the
-- synthetic week has before now (six), and the next 24.
local chartRows = app.activityScope:member("ChartHours").items:count()
check(chartRows == 30 and app.store:get("WeatherRow", "tahoe:fishing:chart:2026-11-18T00:00")
    and app.store:get("WeatherRow", "tahoe:fishing:chart:2026-11-18T06:00").values.Short == "now",
    "The chart hours follow the activity: " .. chartRows)
check(Report.tick({ time = "2026-11-18T09:00" }, 12) == "" and Report.tick({ time = "2026-11-18T12:00" }, 12) == "12",
    "Long charts tick every 12 h")
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
ui.choose("ocean-beach")
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
ui.choose("bondi")
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
local photoSetting = setting("appearance:photos")
local panorama = app.hubPage:findChild("HubPanorama", true)
local beforePhotos = #requests
find("SettingsPivot").SelectedIndex = 2
settle()
check(text(photoSetting, "SettingTitle") == "background photos" and text(photoSetting, "SettingValue") == "on",
    "Appearance offers a background photos setting")
tap(photoSetting, "SettingChange")
check(text(photoSetting, "SettingValue") == "off" and panorama.BackgroundImage == ""
    and store:load("settings").appearance.photos == false, "Turning photos off updates the hub and saves the choice")
ui.back("SettingsPivot")
check(panorama:getFirstChild():getClassName() == "PanoramaItem" and #requests == beforePhotos,
    "The plain hub opens without fetching a forecast for the appearance change")
press("OpenSettings")
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
check(later.find("HubPanorama"):getFirstChild():getClassName() == "PanoramaItem",
    "Photos remain off when the app is reopened")
later.press("OpenSettings")
check(later.text(photoSetting, "SettingValue") == "off", "Settings show the saved photo choice")
later.find(photoSetting):findChild("SettingChange", true):send("Node.LeftButtonUp")
later.settle()
later.back("SettingsPivot")
F.checkHubPhoto(later)
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

-- Air quality and pollen (W3) in the verdict, and "I run cold" (W8).
local smog = F.verdict(Outdoors, { aqi = 170, aqiScale = "us" })
check(F.says(smog, "unhealthy: keep effort low") and smog.score < F.verdict(Outdoors, {}).score, "Smog lowers outdoors")
check(F.verdict(Outdoors, { aqi = 350, aqiScale = "us" }).score <= 25 and not F.says(F.verdict(Outdoors, { aqi = 15,
    aqiScale = "eu" }), "Air"), "Hazardous air caps it; clean air goes unsaid")
local sneezy = F.verdict(Outdoors, { pollen = 180, pollenType = "grass" })
check(F.says(sneezy, "^Very high grass pollen") and sneezy.score == F.verdict(Outdoors, {}).score,
    "Pollen is a note unless you ask")
Settings.set("outdoors", "pollen", true)
check(F.verdict(Outdoors, { pollen = 180, pollenType = "grass" }).score < F.verdict(Outdoors, {}).score, "then it counts")
Settings.set("outdoors", "pollen", false)
check(F.says(F.verdict(Outdoors, { feels = 11 }), "Comfortable"), "11° feels comfortable to most")
Settings.set("outdoors", "comfort", "cold")
check(F.says(F.verdict(Outdoors, { feels = 11 }), "Chilly"), "but chilly to someone who runs cold")
Settings.set("outdoors", "comfort", "usual")
local air = Outdoors.airSource
check(air.url(BERLIN, Forecast):find("european_aqi,grass_pollen") and air.url({ lat = 39, lon = -120 }, Forecast):find("us_aqi"),
    "Europe reads the European index, the rest the US one")
local aired = Forecast.parse(weather(1), nil, NOW)
air.parse({ hourly = { time = { "2026-11-18T06:00", "2026-11-18T07:00" }, european_aqi = { 65, 30 },
    grass_pollen = { 2, 60 }, birch_pollen = { 0, 5 } } }, aired)
local airNow = aired.hours[aired.now]
check(airNow.aqi == 65 and airNow.aqiScale == "eu" and airNow.pollen == 2 and aired.hours[aired.now + 1].pollenType == "grass",
    "The air lands on its hours")
local conditions = {}
for _, metric in ipairs(Outdoors.metrics(airNow, aired)) do conditions[metric.label] = metric end
check(conditions["air quality"].value == "65 EU AQI" and conditions["air quality"].note == "poor"
    and conditions.pollen.value == "low", "Outdoors shows the air among its conditions")
local usAir = Forecast.parse(weather(1), nil, NOW)
air.parse({ hourly = { time = { "2026-11-18T06:00" }, us_aqi = { 40 }, grass_pollen = {} } }, usAir)
conditions = {}
for _, metric in ipairs(Outdoors.metrics(usAir.hours[usAir.now], usAir)) do conditions[metric.label] = metric end
check(conditions.pollen.value == "—" and conditions.pollen.note:find("Europe only"), "and says plainly when pollen is not forecast")

check(Outdoors.sources[1] == air, "The full app asks for the air; an edition keeps to its own requests")

-- Official warnings (W10) from the US National Weather Service.
local alerts = Outdoors.alertSource
check(alerts.url(F.PLACES[1]):find("^https://api%.weather%.gov/alerts/active%?point=39%.0900,%-120%.0400")
    and alerts.url(BERLIN) == nil and Outdoors.isoEpoch("2026-11-18T14:00:00-08:00") == NOW + 16 * 3600,
    "US places ask the NWS")
local warned = Forecast.parse(weather(39.09), nil, NOW)
warned.offset = -8 * 3600
alerts.parse({ features = {
    { properties = { event = "Wind Advisory", severity = "Moderate", ends = "2026-11-18T14:00:00-08:00",
        senderName = "NWS Reno NV" } },
    { properties = { event = "Winter Storm Warning", severity = "Severe", expires = "2026-11-19T18:00:00-08:00" } },
    { properties = { event = "Gone", severity = "Severe", ends = "2026-11-17T18:00:00-08:00" } },
    { properties = { event = "Test Message", status = "Test" } },
} }, warned)
local warnings = Outdoors.warningCards(warned, NOW)
check(#warnings == 2 and warnings[1].value == "winter storm warning" and warnings[1].tone == "poor"
    and warnings[1].detail == "until thu 19 18:00 · National Weather Service"
    and warnings[2].value == "wind advisory" and warnings[2].tone == "fair", "Warnings in force lead, the severe first")
check(Outdoors.cards(warned, F.PLACES[1], NOW)[1].id == "warning-1", "ahead of every other answer")
check(not pcall(alerts.parse, { title = "busy" }, warned), "An answer without a list is not \"no warnings\"")

-- Choose and order "good for" (W7): a switch per activity and the order.
local function goodFor(rows)
    local list = {}
    for _, row in ipairs(rows) do if row.kind == "activity" then list[#list + 1] = row.activity end end
    return table.concat(list, ",")
end
local tahoeWeek = Forecast.parse(weather(39.09), nil, NOW, F.PLACES[1])
local tahoeActivities = Activities.at(F.PLACES[1])
check(goodFor(Report.hub(F.PLACES[1], tahoeWeek, tahoeActivities, NOW)) == "outdoors,fishing,hiking,stargazing,run,cycle",
    "The place's activities and the run and cycle lenses at first, in hub order")
check(Settings.set("outdoors", "follow_hiking", false) and Settings.set("outdoors", "order", "best"), "Switch one off, best first")
local chosen = goodFor(Report.hub(F.PLACES[1], tahoeWeek, tahoeActivities, NOW))
check(not chosen:find("hiking") and chosen:find("stargazing$"), "Hidden ones go, tonight's stars wait at the end: " .. chosen)
local settingsRows = {}
for _, row in ipairs(Settings.rows(Activities.list)) do settingsRows[row.id] = row end
check(settingsRows["settings:outdoors:follow_hiking"].value == "off" and settingsRows["settings:outdoors:order"].value
    == "best first" and settingsRows["settings:outdoors:comfort"].note == "outdoors", "Settings list them under outdoors")
Settings.set("outdoors", "follow_hiking", true)
Settings.set("outdoors", "order", "listed")

-- Everyday lenses (W6): every place, full app only; three start switched off.
local Lens = Activities.byId
check(Lens.run.everywhere and Lens.laundry and #Activities.at(F.PLACES[2]) == 7, "Lenses fit every place")
check(Settings.set("outdoors", "follow_picnic", true)
    and goodFor(Report.hub(F.PLACES[2], tahoeWeek, Activities.at(F.PLACES[2]), NOW)):find("picnic"), "and can be followed")
Settings.set("outdoors", "follow_picnic", false)
check(F.says(F.verdict(Lens.run, { feels = 27, dewPoint = 20 }), "Muggy") and F.verdict(Lens.run, { feels = 12 }).verdict
    == "good", "Run: cool is good, muggy heat is not")
check(F.verdict(Lens.run, { aqi = 170, aqiScale = "us" }).score < F.verdict(Lens.run, {}).score, "and poor air counts")
check(F.verdict(Lens.cycle, { wind = 3, gust = 18 }).score <= 30 and F.says(F.verdict(Lens.cycle, { code = 61 }), "Wet roads"),
    "Cycle: gusts and wet roads")
check(F.verdict(Lens.dogwalk, { temp = 31 }).score <= 30 and F.verdict(Lens.dogwalk, { code = 51 }).verdict ~= "poor",
    "Dog walk: hot paws matter, drizzle hardly")
check(F.verdict(Lens.picnic, { isDay = false }).verdict == "poor" and F.verdict(Lens.picnic, { feels = 22, cloud = 10 })
    .verdict == "good", "Picnic: warm, sunny daylight")
check(F.verdict(Lens.laundry, { humidity = 50, wind = 4, cloud = 20, et0 = 0.4 }).verdict == "good"
    and F.verdict(Lens.laundry, { code = 63 }).score <= 10, "Laundry: breezy and dry, never in rain")

-- Places (wave 3): search through Open-Meteo's geocoding (recorded
-- answers, no network), saving, editing, reordering and removing.
local MyPlaces = require "Weather/myplaces"
local Search = require "Weather/search"
-- Recorded from geocoding-api.open-meteo.com (November 2026), trimmed.
local GEO = {
    matterhorn = { results = {
        { id = 2659729, name = "Matterhorn", latitude = 45.97641, longitude = 7.65861, elevation = 4478.0,
            feature_code = "MT", country_code = "CH", timezone = "Europe/Zurich", country = "Switzerland", admin1 = "Valais" },
        { id = 6637692, name = "Matterhorn", latitude = -77.66667, longitude = 162.45, elevation = 1600.0,
            feature_code = "MT", country_code = "AQ", timezone = "Antarctica/McMurdo" },
    }, generationtime_ms = 0.18 },
    ["10115"] = { results = {
        { id = 5128581, name = "New York", latitude = 40.71427, longitude = -74.00597, elevation = 10.0,
            feature_code = "PPL", country_code = "US", timezone = "America/New_York", population = 8804190,
            postcodes = { "10001", "10114", "10115", "10116" }, country = "United States", admin1 = "New York" },
    }, generationtime_ms = 0.37 },
    hossegor = { results = {
        { id = 3013139, name = "Hossegor", latitude = 43.69, longitude = -1.37, elevation = 19.0, feature_code = "PPL",
            country_code = "FR", timezone = "Europe/Paris", country = "France", admin1 = "New Aquitaine", admin2 = "Landes" },
    }, generationtime_ms = 0.13 },
    nothing = { generationtime_ms = 0.1 },
}
local function geocoding(url)
    local name = url:match("[?&]name=([^&]+)")
    return GEO[name and name:lower()] or GEO.nothing
end

check(Search.url("Zürich 8001"):find("^https://geocoding%-api%.open%-meteo%.com/v1/search%?name=Z%%C3%%BCrich%%208001&count=10")
    and not Search.url("x y"):find("apikey"), "Search asks Open-Meteo's geocoding, encoded")
Forecast.endpoints.apiKey = "KEY"
check(Search.url("bondi"):find("^https://customer%-geocoding%-api%.open%-meteo%.com/") and Search.url("bondi"):find("&apikey=KEY$"),
    "An API key moves search to the commercial host")
Forecast.endpoints.apiKey = nil
check(Search.clean("  b ") == nil and Search.clean(" Lake  Garda ") == "Lake Garda", "Too short a search is not sent")
do
    -- The device's country: a postcode is searched in it, a name everywhere
    -- with its places first.
    local before = Search.preferred()
    Search.prefer("fr")
    check(Search.url("40150"):find("&countryCode=FR") and not Search.url("hossegor"):find("countryCode"),
        "A postcode is searched in the device's country, a name everywhere")
    local mixed = Search.parse({ results = { GEO.matterhorn.results[1], GEO.hossegor.results[1] } }, "x")
    check(mixed[1].base.country == "FR" and mixed[2].base.country == "CH", "Places in the device's country come first")
    Search.prefer(nil)
    check(not Search.url("40150"):find("countryCode"), "Without a region nothing is filtered")
    Search.prefer(before)
end
local peak = MyPlaces.fromResult(GEO.matterhorn.results[1], "matterhorn")
check(peak.id == "geo-2659729" and peak.base.region == "Valais, Switzerland" and peak.own.kind == "mountain"
    and peak.own.summit == 4478 and table.concat(peak.base.activities, ",") == "hiking,stargazing",
    "A peak is guessed a mountain for hiking and the stars, its height the top")
local function guessed(code, elevation, population)
    return table.concat(MyPlaces.guess(MyPlaces.FEATURE[code] or "town", elevation, population), ",")
end
check(guessed("BCH") == "surf,sailing,fishing,stargazing" and guessed("LK") == "fishing,sailing,stargazing"
    and guessed("STM") == "fishing" and guessed("PPL", 1600) == "garden,stargazing,hiking"
    and guessed("PPL", 10, 8000000) == "garden", "Coast, lake, river, height and city light guess the activities")
local lake = MyPlaces.resolveDraft(MyPlaces.fromResult({ id = 1, name = "Tegernsee", latitude = 47.71, longitude = 11.76,
    feature_code = "LK" }))
check(lake.lake and not lake.river and lake.kind == "lake" and lake.draft, "A lake is a lake for the forecast")
check(MyPlaces.fromResult(GEO["10115"].results[1], "10115").base.postcode == "10115", "A postcode found is kept to show")
local here = MyPlaces.fromLocation(51.5, -0.12)
check(here and here.id == "at-51.500_-0.120" and here.base.located and here.own.kind == "town",
    "Device location has its hook (wave 4): a draft at the fix")

local placeNet = F.network()
placeNet.answers["geocoding%-api"] = geocoding
local placeStore = F.storage("places", F.PLACES, function(place) return "forecast-geo-" .. place.id end)
placeStore:remove("forecast-geo-2659729")
local pui = F.launch { net = placeNet, storage = placeStore }
local function child(row, name) return pui.find(row):findChild(name, true) end
local function tap(row, name) child(row, name):send("Node.LeftButtonUp"); pui.settle() end
-- First run: the curated places are suggestions, the first one shown.
check(#MyPlaces.saved() == 0 and pui.find("HubPanorama").Title == "Lake Tahoe" and pui.find("NoPlaces").Visible
    and child("Placetahoe", "PlaceLink").Text == "keep", "The first run shows the suggestions, useful at once")
pui.press("AddPlace")
check(pui.find("SearchPivot").Title == "weather" and pui.has("Suggestedtahoe") and not pui.has("Resulttahoe"),
    "Add a place lists the suggestions too")
pui.find("SearchInput").Text = "Matterhorn"
pui.press("RunSearch")
check(placeNet.requests[#placeNet.requests]:find("geocoding%-api%.open%-meteo%.com/v1/search%?name=Matterhorn"),
    "Search asks for what was typed")
check(child("Resultgeo-2659729", "ResultLine").Text == "mountain · Valais, Switzerland"
    and child("Resultgeo-6637692", "ResultLine").Text == "mountain", "Results say what and where each place is")
pui.press("Resultgeo-2659729")
check(pui.find("PlacePivot").Title == "Matterhorn" and pui.find("PlaceSection").Header == "new place"
    and child("Fieldplace:field:kind", "SettingValue").Text == "a mountain or a trail"
    and child("Fieldplace:field:for:hiking", "SettingValue").Text == "yes"
    and child("Fieldplace:field:for:garden", "SettingValue").Text == "no"
    and child("Fieldplace:field:summit", "SettingValue").Text == "4478 m"
    and child("Fieldplace:field:trailhead", "SettingValue").Text == "not set", "The editor shows the guess to confirm")
check(child("Fieldplace:field:summit", "SettingHeading").Text == "hiking" and not pui.has("Fieldplace:field:facing"),
    "Hiking's own fields follow, under its name; no shore for a mountain")
tap("Fieldplace:field:trailhead", "SettingMore")
tap("Fieldplace:field:trailhead", "SettingMore")
check(child("Fieldplace:field:trailhead", "SettingValue").Text == "550 m", "A height steps from a start")
tap("Fieldplace:field:for:garden", "SettingChange")
tap("Fieldplace:field:for:garden", "SettingChange")
check(child("Fieldplace:field:for:garden", "SettingValue").Text == "no", "An activity switches on and off")
check(#MyPlaces.saved() == 0, "Nothing is saved before save")
pui.press("Actionplace:action:save")
check(MyPlaces.saved()[1].id == "geo-2659729" and pui.find("HubPanorama").Title == "Matterhorn"
    and pui.find("HubPanorama").SelectedIndex == 0 and pui.has("GoodForgeo-2659729:activity:hiking"),
    "Saving shows the new place's hub, good for hiking")
local saved = placeStore:load("places")
check(saved and saved.places[1].id == "geo-2659729" and saved.places[1].own.trailhead == 550
    and saved.places[1].base.lat == 45.97641, "The place is saved on the device")
check(placeNet.requests[#placeNet.requests]:find("latitude=45.97641"), "and its forecast was asked")
local asked = #placeNet.requests
pui.open("geo-2659729", "hiking")
check(table.concat(placeNet.requests, "\n", asked + 1):find("elevation=550,4478"),
    "Hiking asks the trailhead and the top at the heights given")
pui.back()
check(placeStore:load("forecast-geo-2659729"), "Its forecast is kept for offline")
-- Keep a suggestion, make it home, turn its shore.
check(child("Placegeo-2659729", "PlaceLine").Text == "home · Valais, Switzerland", "The first place is home")
tap("Placebondi", "PlaceLink")
check(MyPlaces.saved()[2].id == "bondi" and child("Placebondi", "PlaceLink").Text == "edit" and not pui.find("NoPlaces").Visible
    and pui.find("PlaceItems"):findChild("Placebondi", true), "Keep makes a suggestion my place")
tap("Placebondi", "PlaceLink")
check(pui.find("PlaceSection").Header == "edit place" and pui.has("Actionplace:action:home")
    and not pui.has("Actionplace:action:down") and child("Fieldplace:field:facing", "SettingValue").Text == "E (110°)",
    "A saved place has its links; Bondi's shore faces east")
pui.press("Actionplace:action:home")
check(MyPlaces.saved()[1].id == "bondi" and not pui.has("Actionplace:action:home") and pui.has("Actionplace:action:down"),
    "Make home moves it first")
local before = #placeNet.requests
tap("Fieldplace:field:facing", "SettingMore")
local bondi = MyPlaces.find("bondi")
check(child("Fieldplace:field:facing", "SettingValue").Text == "SE (135°)" and bondi.facing == 135 and bondi.own.facing == 135,
    "The compass turns a step")
check(placeStore:load("places").places[1].own.facing == 135 and placeStore:load("places").places[1].curated
    and not placeStore:load("places").places[1].base, "A kept suggestion stores only what the user changed")
pui.back("PlacePivot")
check(child("Placebondi", "PlaceLine").Text == "home · Sydney" and #placeNet.requests == before,
    "Back on the hub, Bondi is home; a new facing asks nothing new")
-- Reorder and remove.
tap("Placegeo-2659729", "PlaceLink")
pui.press("Actionplace:action:up")
check(MyPlaces.saved()[1].id == "geo-2659729", "Move up reorders")
pui.press("Actionplace:action:down")
check(MyPlaces.saved()[2].id == "geo-2659729", "and move down")
pui.press("Actionplace:action:remove")
check(pui.find("Actionplace:action:remove").Text == "tap again to remove Matterhorn" and #MyPlaces.saved() == 2,
    "Remove asks for a second tap")
pui.press("Actionplace:action:remove")
check(#MyPlaces.saved() == 1 and not placeStore:load("forecast-geo-2659729") and pui.find("HubPanorama").Title == "Bondi"
    and not pui.has("Placegeo-2659729"), "Removing forgets the place and its forecast")
-- Offline, and nothing found.
pui.press("AddPlace")
placeNet.failing = true
pui.find("SearchInput").Text = "lake"
pui.press("RunSearch")
check(pui.find("Notenote").Text:find("No connection") and pui.has("Resulttahoe"),
    "Offline, search says so and still finds the app's own places")
pui.find("SearchInput").Text = "qwxz"
pui.press("RunSearch")
check(pui.find("Notenote").Text:find("search needs the network") and not pui.has("Resulttahoe"),
    "and says what still works")
placeNet.failing = false
pui.press("RunSearch")
check(pui.find("Notenote").Text:find("Nothing found for “qwxz”"), "Nothing found says so")
pui.find("SearchInput").Text = "10115"
pui.press("RunSearch")
check(child("Resultgeo-5128581", "ResultLine").Text == "town · 10115 · United States", "A postcode finds its town")
pui.press("Resultgeo-5128581")
check(child("Fieldplace:field:for:stargazing", "SettingValue").Text == "no", "A big city is not guessed for the stars")
pui.press("Actionplace:action:save-home")
check(MyPlaces.saved()[1].id == "geo-5128581" and MyPlaces.saved()[2].id == "bondi", "Save as home puts it first")
pui.app.hubScope:dispose()
-- The next launch keeps them, in order.
local again = F.launch { net = placeNet, storage = placeStore }
check(again.find("HubPanorama").Title == "New York" and #MyPlaces.saved() == 2 and again.has("Placebondi"),
    "Saved places come back on the next launch")
again.app.hubScope:dispose()
check(not again.has("Locatelocate"), "Where the device cannot say where it is, no 'use my location' is offered")

-- N1: "use my location" over a stand-in for orca.location. The fix is a
-- result like a search's; nothing is saved until the editor saves it.
local Locate = require "Weather/locate"
local located = { status = "allowed", asked = 0 }
local fakeLocation = {
    supported = function() return true end,
    status = function() return located.status, located.approximate end,
    current = function()
        located.asked = located.asked + 1
        if located.status ~= "allowed" then return nil, { code = "permission", message = "denied" } end
        return { lat = 52.3702, lon = 4.8952, accuracy = 3000, timestamp = NOW, approximate = true }
    end,
    reverse = function()
        if located.offline then return nil, { code = "network", message = "offline" } end
        return { name = "Amsterdam", locality = "Amsterdam", region = "North Holland", country = "Netherlands", countryCode = "NL" }
    end,
}
check(Locate.available(false) == false and Locate.available(fakeLocation) and select(2, Locate.run(false)):find("cannot say"),
    "Without orca.location, location is not offered and says so")
local loc = F.launch { net = placeNet, storage = placeStore, location = fakeLocation }
check(loc.find("Locatelocate").Text == "use my location", "The places section offers 'use my location'")
loc.press("Locatelocate")
check(loc.find("SearchPivot") and located.asked == 1
    and loc.find("Locatedat-52.370_4.895"):findChild("ResultLine", true).Text
        == "your location · North Holland, Netherlands · approximate"
    and loc.find("Locatedat-52.370_4.895"):findChild("ResultTitle", true).Text == "Amsterdam",
    "It opens search on where the device is, named, saying it is approximate")
check(#MyPlaces.saved() == 2, "Nothing is saved by looking")
loc.press("Locatedat-52.370_4.895")
check(loc.find("PlacePivot").Title == "Amsterdam" and loc.find("PlaceSection").Header == "new place",
    "The located place opens in the editor like a search result")
loc.press("Actionplace:action:save")
local mine = MyPlaces.find("at-52.370_4.895")
check(mine and mine.saved and mine.country == "NL" and mine.located and loc.find("HubPanorama").Title == "Amsterdam",
    "Saving keeps it with its country, and the hub shows it")
-- Offline naming falls back to coordinates; a denial is a calm line.
located.offline = true
local draft = Locate.run(fakeLocation)
check(draft and draft.base.title == "here" and draft.base.region == "52.37°, 4.90°", "Without a name the place is 'here'")
located.offline, located.status = nil, "denied"
loc.press("AddPlace")
loc.press("Locatelocate")
check(loc.find("LocatedNotelocate:note").Text:find("Location is off for this app") and not loc.has("Locatedat-52.370_4.895")
    and loc.has("SearchInput"), "Denied, it says so calmly and search still works")
located.status = "restricted"
check(not Locate.available(fakeLocation), "A device policy that forbids location hides the offer")
loc.app.hubScope:dispose()

-- P5: one comparison of my places for an activity, which surf, sailing
-- and fishing rank with.
local Compare = require "Weather/compare"
local A, B, C, D = { id = "a", title = "A", activities = { "surf" } }, { id = "b", title = "B", activities = { "surf" } },
    { id = "c", title = "C", activities = { "fishing" } }, { id = "d", title = "D", activities = { "surf" } }
check(#Compare.places("surf", { A, B, C, D }) == 3 and Compare.places("fishing", { A, C })[1] == C,
    "Only the places offering the activity are compared")
local scores = { a = { score = 50 }, b = { score = 80 }, d = { score = 90 } }
local ranked = Compare.rank({ { place = A }, { place = B, failed = "offline" }, { place = D, later = true }, { place = C } },
    function(entry)
        local result = scores[entry.place.id]
        return result and { result = result, later = entry.place.id == "d" }
    end)
check(#ranked == 3 and ranked[1].place == A and ranked[2].place == D and ranked[3].failed == "offline",
    "Sooner first, then the score; places without a forecast last")

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

-- W11: Dynamic Type. The content size category scales Weather's type,
-- held to what the Metro layout takes.
do
    local core, App = require "orca.core", require "Weather/App"
    check(core.getContentSizeCategory() == "large" and core.getTextScale() == 1, "Large is the default text size")
    core.setContentSizeCategory("extra-large")
    check(math.abs(core.getTextScale() - 19 / 17) < 1e-4 and math.abs(App.textScale() - 19 / 17) < 1e-4,
        "Extra large is the Body style's 19 points")
    core.setContentSizeCategory("accessibility-extra-extra-extra-large")
    check(core.getTextScale() > 3 and App.textScale() == App.TEXT_SCALE.max, "The largest sizes stop at Weather's maximum")
    core.setContentSizeCategory("extra-small")
    check(App.textScale() == App.TEXT_SCALE.min, "And the smallest at its minimum")
    core.setContentSizeCategory("bogus")
    check(core.getTextScale() == 1, "An unknown category scales as large")
    core.setContentSizeCategory("large")
end

-- Services: a source may ask for text, and an activity's attribution joins
-- the settings' "about" once, between Open-Meteo and the privacy line.
do
    local Forecast, Provider, Activities = require "Weather/forecast", require "Weather/provider", require "Weather/activities"
    check(Forecast.requestOptions(nil, nil, "text").format == "text", "A text source asks for text")
    check(Forecast.requestOptions().format == nil, "Other requests decode JSON")
    local seen
    local source = { id = "buoy-test", optional = true, format = "text",
        url = function() return "https://example.test/buoy.txt" end,
        parse = function(body, forecast) seen = body end }
    local forecast = { missing = {} }
    Forecast.extend(forecast, { id = "x", lat = 0, lon = 0 }, { source },
        function(url, options) return options.format == "text" and "#YY MM DD" or nil end, {})
    check(seen == "#YY MM DD", "The text reaches the source's parse")
    local first = Activities.list[1]
    local before, baseline = first.attribution, #Provider.about()
    first.attribution = { { "Test credit by Example", "Public domain." }, { "Weather data by Open-Meteo.com", "twice" } }
    local about = Provider.about()
    local function at(title) for index, line in ipairs(about) do if line[1] == title then return index end end end
    check(about[1][1] == "Weather data by Open-Meteo.com" and (at("Test credit by Example") or 0) > 1,
        "An activity's service follows Open-Meteo")
    check(about[#about][1] == "your data stays here" and #about == baseline + 1, "Each title shows once")
    first.attribution = before
end

F.done("weather")
