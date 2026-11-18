-- The weather kit (apps/WeatherKit) and its profile apps. The sky helpers and
-- each profile's rules are checked on hand-built hours; then the Fishing and
-- Surf apps run on a synthetic Open-Meteo forecast through the real page,
-- scope and templates, including switching spots.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/Fishing")
fs.init("apps/Surf")
fs.init("apps/Weather")
require("orca.core.startup").load_plugins()
local Lint = require "orca.data.lint"
local savedAsync = orca.async
orca.async = function(fn, ...) return fn(...) end
local checks = 0
local function check(value, message) assert(value, message); checks = checks + 1 end

local Sky = require "WeatherKit/sky"
local Score = require "WeatherKit/score"
local Fishing = require "Fishing/profile"
local Surf = require "Surf/profile"

local NOW = 1794981600 -- 2026-11-18 06:00 UTC, a Wednesday

-- Sky
check(Sky.moon(1795531980).name == "full moon", "2026-11-24 is a full moon")
check(Sky.moon(1794207720).name == "new moon" and Sky.moon(1794207720).lit < 0.01, "2026-11-09 is a new moon")
check(math.abs(Sky.moon(NOW).daysFromSyzygy - 6.4) < 0.5, "Mid-November is about six days from full")
check(Sky.weekday("2026-11-18") == "wed 18" and Sky.weekday("2027-01-01") == "fri 1", "Weekdays")
check(Sky.angle(350, 10) == 20 and Sky.angle(90, 270) == 180, "Bearings wrap")
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

-- A synthetic week from Open-Meteo, the shape the forecast API returns.
local function series(n, fn) local t = {} for i = 1, n do t[i] = fn(i - 1) end return t end
local function iso(i) return ("2026-11-%02dT%02d:00"):format(18 + i // 24, i % 24) end
local function weather(lat)
    local hours = 7 * 24
    return {
        utc_offset_seconds = 0, latitude = lat,
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
local failing = false
local function fetch(url)
    requests[#requests + 1] = url
    if failing then return nil, "No connection to the forecast service." end
    if url:find("marine%-api") then return marine() end
    return weather(tonumber(url:match("latitude=([%-%d%.]+)")))
end

local function run(App, problems)
    check(#problems == 0, Lint.format(problems))
    local app = App { fetch = fetch, clock = function() return NOW end }
    local screen = app:dispatch("/").view
    local function settle()
        for _ = 1, 3 do
            core.flushQueue()
            core.advanceFrame()
            core.runAllPrograms(screen)
            core.flushQueue()
        end
        screen:UpdateLayout(390, 844)
    end
    settle()
    local function find(name) return assert(screen:findChild(name, true), "Missing " .. name) end
    return app, screen, find, function(node) node:send("Node.LeftButtonUp"); settle() end
end

-- The Fishing app
local lint = Lint.page("WeatherKit/Pages/Forecast", "WeatherKit/Data/Forecast.xml")
local app, screen, find, press = run(require "Fishing/App", lint)
check(find("WeatherPanorama").Title == "fishing", "The panorama is titled by the profile")
check(#requests == 1 and requests[1]:find("latitude=39.09") and not requests[1]:find("marine"),
    "Fishing asks for the first spot's forecast only")
local nowRow = find("Nowtahoe:now:now")
check(nowRow:findChild("NowVerdict", true).Text == "good", "06:00 before a 06:45 sunrise is good fishing")
check(nowRow:findChild("NowSpot", true).Text == "Lake Tahoe" and nowRow:findChild("NowTemp", true).Text == "10°",
    "The now card shows the spot and temperature")
check(find("Reasontahoe:reason:1"):findChild("ReasonText", true).Text:find("Dawn"), "Reasons list why")
check(find("Hourtahoe:hour:2026-11-18T06:00"):findChild("When", true).Text == "now"
    and find("Hourtahoe:hour:2026-11-19T05:00"), "Twenty-four hours from now")
check(not screen:findChild("Hourtahoe:hour:2026-11-19T06:00", true), "No more than twenty-four hours")
check(find("Daytahoe:day:2026-11-18"):findChild("When", true).Text == "today"
    and find("Daytahoe:day:2026-11-19"):findChild("When", true).Text == "thu 19", "Days are named")
check(find("Daytahoe:day:2026-11-20"):findChild("Verdict", true).Text == "poor", "A thunderstorm day is poor")
check(find("Daytahoe:day:2026-11-19"):findChild("Why", true).Text:find("^best 0[67]"), "Days give the best window: "
    .. find("Daytahoe:day:2026-11-19"):findChild("Why", true).Text)
check(find("Conditiontahoe:metric:1"):findChild("MetricValue", true).Text == "1018 hPa", "Conditions show pressure")
check(find("Spottahoe") and find("Spotlofoten"), "Spots are listed")
press(find("Spotlofoten"))
check(requests[#requests]:find("latitude=68.23") and find("Nowlofoten:now:now")
    and find("Nowlofoten:now:now"):findChild("NowSpot", true).Text == "Lofoten", "Choosing a spot forecasts there")
failing = true
press(find("RefreshReport"))
check(find("Nowlofoten:now:now"), "A failed refresh keeps the last report")
failing = false
app.scope:dispose()

-- The Surf app
requests = {}
app, screen, find, press = run(require "Surf/App", lint)
check(find("WeatherPanorama").Title == "surf", "Surf has its own title")
check(#requests == 2 and requests[2]:find("marine%-api") and requests[2]:find("latitude=37.76"),
    "Surf adds the marine forecast")
-- Ocean Beach faces west; the synthetic wind is from the east: offshore.
check(find("Hourocean-beach:hour:2026-11-18T08:00"):findChild("Verdict", true).Text == "good",
    "Offshore 1.5 m at 13 s is good surf in daylight")
check(find("Hourocean-beach:hour:2026-11-18T06:00"):findChild("Verdict", true).Text == "poor", "But not in the dark")
check(find("Hourocean-beach:hour:2026-11-18T08:00"):findChild("Why", true).Text == "1.5 m · 13 s · offshore 3 m/s",
    "Hours say swell, period and wind")
check(find("Conditionocean-beach:metric:4"):findChild("MetricValue", true).Text == "14°", "Water temperature")
press(find("Spotbondi"))
-- Bondi faces east: the same east wind is onshore and the westerly swell misses it.
check(find("Hourbondi:hour:2026-11-18T08:00"):findChild("Verdict", true).Text ~= "good", "Wind and swell depend on the beach")
app.scope:dispose()

-- The everyday Weather app runs on the same kit.
requests = {}
app, screen, find, press = run(require "Weather/App", lint)
check(find("WeatherPanorama").Title == "weather" and requests[1]:find("latitude=37.7749"), "Weather uses the kit too")
check(find("Daysan-francisco:day:2026-11-19"):findChild("Verdict", true).Text ~= "", "Weather rates the days")
app.scope:dispose()

orca.async = savedAsync
print(("PASS: weather profiles (%d checks)"):format(checks))
