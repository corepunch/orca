-- The weather app (apps/Weather). The sky helpers and each activity's rules
-- are checked on hand-built hours; then the app runs on a synthetic
-- Open-Meteo forecast through its real pages, scopes and templates: the
-- place hub, an activity page, Back, and switching places.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/Weather")
require("orca.core.startup").load_plugins()
local Lint = require "orca.data.lint"
local savedAsync = orca.async
orca.async = function(fn, ...) return fn(...) end
local checks = 0
local function check(value, message) assert(value, message); checks = checks + 1 end

local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Fishing = require "Weather/activities/fishing"
local Surf = require "Weather/activities/surf"
local Sailing = require "Weather/activities/sailing"
local Hiking = require "Weather/activities/hiking"
local Garden = require "Weather/activities/garden"
local Stargazing = require "Weather/activities/stargazing"
local Forecast = require "Weather/forecast"

local NOW = 1794981600 -- 2026-11-18 06:00 UTC, a Wednesday

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

-- A day hour by hour; `tweak(hour, index)` changes conditions.
local function hour(fields)
    local h = { time = "2026-11-18T07:00", date = "2026-11-18", temp = 12, feels = 11, code = 2, wind = 3, gust = 6,
        windDir = 90, pressure = 1015, pressureTrend = -1, cloud = 50, rainChance = 10, isDay = true, uv = 1,
        day = { date = "2026-11-18", sunrise = "2026-11-18T06:45", sunset = "2026-11-18T17:00" } }
    for k, v in pairs(fields or {}) do h[k] = v end
    return h
end
local forecast = { epoch = NOW }
local function verdict(profile, fields, spot)
    forecast.spot = spot
    return profile.rate(hour(fields), forecast):result()
end

-- Fishing
local dawn = verdict(Fishing)
check(dawn.verdict == "good", "Dawn, light breeze and slowly falling pressure is good fishing: " .. dawn.score)
check(dawn.reasons[1].text:find("Dawn"), "The strongest reason leads: " .. dawn.reasons[1].text)
check(verdict(Fishing, { code = 95 }).verdict == "poor", "Thunder means poor fishing")
check(verdict(Fishing, { pressureTrend = 2.5, time = "2026-11-18T12:00", cloud = 0, uv = 7 }).verdict == "poor",
    "Rising pressure under a bright noon sun is poor")
check(verdict(Fishing, { wind = 13, gust = 20 }).score <= 35, "Gusts cap the score")

-- Surf
local beach = { facing = 270 }
local clean = { swellHeight = 1.5, swellPeriod = 12, swellDir = 280, windDir = 90, wind = 4 }
check(verdict(Surf, clean, beach).verdict == "good", "Offshore wind on a long-period swell is good surf")
check(verdict(Surf, { swellHeight = 1.5, swellPeriod = 12, swellDir = 280, windDir = 270, wind = 9 }, beach).verdict ~= "good",
    "The same swell blown out onshore is not")
check(verdict(Surf, { swellHeight = 0.2, swellPeriod = 6, swellDir = 280 }, beach).verdict == "poor", "Flat is poor")
check(verdict(Surf, { swellHeight = 1.5, swellPeriod = 12, swellDir = 280, isDay = false }, beach).score <= 30,
    "Dark caps the score")
check(verdict(Surf, { swellHeight = 1.5, swellPeriod = 12, swellDir = 90, windDir = 90, wind = 4 }, beach).score
    < verdict(Surf, clean, beach).score, "Swell pointed away from the beach scores lower")

-- Sailing and kitesurf
local shore = { facing = 270 }
local breeze = { wind = 8, gust = 9.5, windDir = 0 }
check(verdict(Sailing, breeze, shore).verdict == "good", "A steady cross-shore 8 m/s is good sailing")
check(verdict(Sailing, { wind = 8, gust = 9.5, windDir = 90 }, shore).score <= 30, "Offshore wind is capped")
check(verdict(Sailing, { wind = 1.5, gust = 2 }, shore).verdict == "poor", "No wind is poor")
check(verdict(Sailing, { wind = 18, gust = 25 }, shore).score <= 20, "A gale is capped")
check(verdict(Sailing, { wind = 8, gust = 14, windDir = 0 }, shore).score < verdict(Sailing, breeze, shore).score,
    "Gusty wind scores lower than steady wind")
check(Sailing.windTo({ windDir = 90 }, shore) == "offshore" and Sailing.windTo({ windDir = 270 }, shore) == "onshore",
    "Wind relative to the shore")

-- Hiking
check(verdict(Hiking, { feels = 14, cloud = 10 }).verdict == "good", "Mild, clear and calm is good hiking")
check(verdict(Hiking, { code = 96 }).verdict == "poor", "Thunder is poor hiking")
check(verdict(Hiking, { gust = 25 }).score <= 30, "Knock-down gusts are capped")
check(verdict(Hiking, { visibility = 500 }).score < verdict(Hiking).score, "Fog lowers the score")
forecast.elevation = 3000
local icy = Hiking.rate(hour { freezingLevel = 2500 }, forecast):result()
forecast.elevation = nil
local saysIce = false
for _, reason in ipairs(icy.reasons) do saysIce = saysIce or reason.text:find("ice") ~= nil end
check(saysIce, "A freezing level below the spot warns of ice")

-- Garden
check(verdict(Garden, { time = "2026-11-18T11:00", temp = 16, wind = 2 }).verdict == "good",
    "A mild, calm, dry hour is good for the garden")
check(verdict(Garden, { temp = -2, soilTemp = -1 }).score < 45, "Frozen ground is poor")
check(verdict(Garden, { code = 63 }).score < verdict(Garden).score, "Rain lowers the score")
local frostAhead = { epoch = NOW, hours = { { epoch = NOW + 3600 * 5, temp = -1 } } }
local frosty = Garden.rate(hour { epoch = NOW }, frostAhead):result()
local warns = false
for _, reason in ipairs(frosty.reasons) do warns = warns or reason.text:find("Frost") ~= nil end
check(warns, "Frost in the next hours is a reason")
local metrics = Garden.metrics(hour { epoch = NOW }, { epoch = NOW, hours = {
    { epoch = NOW + 3600, temp = 0.5, et0 = 0.3, rain = 0 }, { epoch = NOW + 7200, temp = 3, et0 = 0.3, rain = 0 },
    { epoch = NOW + 3600 * 30, temp = -9, et0 = 5, rain = 0 } } })
check(metrics[1].value == "frost risk" and metrics[1].note == "lowest in 24 h: 1°", "Frost looks 24 hours ahead only")
check(metrics[2].value == "no", "Little water loss needs no watering: " .. metrics[2].note)

-- Stargazing
local night = { time = "2026-11-18T23:00", isDay = false, cloud = 0, humidity = 50, epoch = 1794207720 + 11 * 3600 }
check(verdict(Stargazing, night).verdict == "good", "A clear night near new moon is good stargazing")
check(verdict(Stargazing, { isDay = true, cloud = 0 }).score <= 5, "Daylight is capped")
check(verdict(Stargazing, { time = "2026-11-18T18:00", isDay = false, cloud = 0 }).score <= 35, "Twilight is capped")
check(verdict(Stargazing, { time = "2026-11-18T23:00", isDay = false, cloud = 90, epoch = night.epoch }).verdict == "poor",
    "Clouds are poor")
local fullNight = { time = "2026-11-25T00:00", isDay = false, cloud = 0, humidity = 50, epoch = 1795531980 + 9 * 3600 }
check(verdict(Stargazing, fullNight).score < verdict(Stargazing, night).score, "A full moon overhead dims the sky")

-- A synthetic week from Open-Meteo, the shape the forecast API returns.
local function series(n, fn) local t = {} for i = 1, n do t[i] = fn(i - 1) end return t end
local function iso(i) return ("2026-11-%02dT%02d:00"):format(18 + i // 24, i % 24) end
local function weather(lat)
    local hours = 7 * 24
    return {
        utc_offset_seconds = 0, latitude = lat, elevation = 1200,
        hourly = {
            time = series(hours, iso),
            temperature_2m = series(hours, function(i) return 8 + (i % 24) / 3 end),
            apparent_temperature = series(hours, function(i) return 7 + (i % 24) / 3 end),
            weather_code = series(hours, function(i) return i // 24 == 2 and 95 or 2 end),
            wind_speed_10m = series(hours, function() return 3 end),
            wind_gusts_10m = series(hours, function() return 6 end),
            wind_direction_10m = series(hours, function() return 90 end),
            pressure_msl = series(hours, function(i) return 1020 - i * 0.3 end),
            cloud_cover = series(hours, function() return 50 end),
            precipitation_probability = series(hours, function() return 10 end),
            precipitation = series(hours, function() return 0 end),
            relative_humidity_2m = series(hours, function() return 70 end),
            visibility = series(hours, function() return 20000 end),
            uv_index = series(hours, function() return 1 end),
            is_day = series(hours, function(i) return (i % 24 >= 7 and i % 24 <= 16) and 1 or 0 end),
            dew_point_2m = series(hours, function() return 3 end),
            cloud_cover_low = series(hours, function() return 20 end),
            cloud_cover_mid = series(hours, function() return 20 end),
            cloud_cover_high = series(hours, function() return 10 end),
            freezing_level_height = series(hours, function() return 2200 end),
            cape = series(hours, function() return 50 end),
            soil_moisture_0_to_1cm = series(hours, function() return 0.3 end),
            soil_temperature_0cm = series(hours, function() return 6 end),
            et0_fao_evapotranspiration = series(hours, function() return 0.1 end),
        },
        daily = {
            time = series(7, function(d) return ("2026-11-%02d"):format(18 + d) end),
            weather_code = series(7, function(d) return d == 2 and 95 or 2 end),
            temperature_2m_max = series(7, function() return 15 end),
            temperature_2m_min = series(7, function() return 7 end),
            precipitation_probability_max = series(7, function() return 10 end),
            precipitation_sum = series(7, function() return 0 end),
            wind_speed_10m_max = series(7, function() return 5 end),
            sunrise = series(7, function(d) return ("2026-11-%02dT06:45"):format(18 + d) end),
            sunset = series(7, function(d) return ("2026-11-%02dT17:00"):format(18 + d) end),
        },
    }
end
local function marine()
    local hours = 7 * 24
    return { hourly = {
        time = series(hours, iso),
        wave_height = series(hours, function() return 1.8 end), wave_period = series(hours, function() return 11 end),
        wave_direction = series(hours, function() return 280 end),
        swell_wave_height = series(hours, function() return 1.5 end),
        swell_wave_period = series(hours, function() return 13 end),
        swell_wave_direction = series(hours, function() return 280 end),
        sea_surface_temperature = series(hours, function() return 13.5 end),
    } }
end
local requests = {}
local failing, marineFailing = false, false
local function fetch(url)
    requests[#requests + 1] = url
    if failing or (marineFailing and url:find("marine%-api")) then return nil, "No connection to the forecast service." end
    if url:find("marine%-api") then return marine() end
    return weather(tonumber(url:match("latitude=([%-%d%.]+)")))
end

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

-- The app over a few places, with every navigation step a real control event.
local Activities = require "Weather/activities"
local Places = require "Weather/places"
local testPlaces = {
    { id = "tahoe", title = "Lake Tahoe", region = "California", lat = 39.09, lon = -120.04, lake = true, facing = 90,
        activities = { "fishing", "hiking", "stargazing" } },
    { id = "ocean-beach", title = "Ocean Beach", region = "San Francisco", lat = 37.76, lon = -122.52, facing = 270,
        activities = { "surf" } },
    { id = "bondi", title = "Bondi", region = "Sydney", lat = -33.89, lon = 151.28, facing = 110, activities = { "surf" } },
}
for _, page in ipairs { { "Weather/Pages/Hub", "Weather/Data/Hub.xml" }, { "Weather/Pages/Activity", "Weather/Data/Activity.xml" } } do
    local problems = Lint.page(page[1], page[2])
    check(#problems == 0, Lint.format(problems))
end
requests = {}
local App = require "Weather/App"
local app = App { places = testPlaces, fetch = fetch, clock = function() return NOW end }
app:activate_route("/")
local function settle()
    for _ = 1, 3 do
        core.flushQueue()
        core.advanceFrame()
        core.runAllPrograms(app.screen)
        core.flushQueue()
    end
    app.screen:UpdateLayout(390, 844)
end
local function find(name) return assert(app.screen:findChild(name, true), "Missing " .. name) end
local function text(row, child) return find(row):findChild(child, true).Text end
local function press(name) find(name):send("Node.LeftButtonUp"); settle() end
settle()
local hub = app.screen

-- The hub: Amazing Weather's now page, what the place is good for, the week.
check(find("HubPanorama").Title == "Lake Tahoe", "The hub is titled with the place")
check(#requests == 1 and requests[1]:find("latitude=39.09") and not requests[1]:find("marine"),
    "A lake asks for the forecast only")
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

-- An activity page reuses the hub's forecast and goes back to the same hub.
press("GoodFortahoe:activity:fishing")
check(app.screen ~= hub and find("ActivityPivot").Title == "fishing · Lake Tahoe", "Fishing opens its own page")
check(find("ActivityPage").StateClasses == "fishing", "The page carries the activity's accent class")
check(#requests == 1, "Opening an activity reuses the place's forecast")
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
check(requests[#requests]:find("marine%-api") and requests[#requests]:find("latitude=37.76"), "Surf adds the marine forecast")
check(find("GoodForocean-beach:activity:surf") and not app.screen:findChild("GoodForocean-beach:activity:fishing", true),
    "Ocean Beach is good for surf, not fishing")
press("GoodForocean-beach:activity:surf")
-- Ocean Beach faces west; the synthetic wind is from the east: offshore.
check(text("Hourocean-beach:surf:hour:2026-11-18T08:00", "Verdict") == "good", "Offshore 1.5 m at 13 s is good surf")
check(text("Hourocean-beach:surf:hour:2026-11-18T06:00", "Verdict") == "poor", "But not in the dark")
check(text("Hourocean-beach:surf:hour:2026-11-18T08:00", "Why") == "1.5 m · 13 s · offshore 3 m/s", "Hours say swell and wind")
check(text("Conditionocean-beach:surf:metric:4", "MetricValue") == "14°", "Water temperature")
find("ActivityPivot"):post("Pivot.BackRequested")
settle()
press("Placebondi")
press("GoodForbondi:activity:surf")
-- Bondi faces east: the same east wind is onshore and the westerly swell misses it.
check(text("Hourbondi:surf:hour:2026-11-18T08:00", "Verdict") ~= "good", "Wind and swell depend on the beach")
find("ActivityPivot"):post("Pivot.BackRequested")
settle()
failing = true
press("RefreshReport")
check(find("Nowbondi:now:now") and not find("ReportFailed").Visible, "A failed refresh keeps the last report")
failing = false
app.hubScope:dispose()

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

orca.async = savedAsync
print(("PASS: weather (%d checks)"):format(checks))
