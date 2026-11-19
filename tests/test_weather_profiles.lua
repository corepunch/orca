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
for _, name in ipairs { "Sailing", "Hiking", "Garden", "Stargazing" } do fs.init("apps/" .. name) end
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
local Sailing = require "Sailing/profile"
local Hiking = require "Hiking/profile"
local Garden = require "Garden/profile"
local Stargazing = require "Stargazing/profile"
local Forecast = require "WeatherKit/forecast"

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

-- The everyday Weather app runs on the same kit.
requests = {}
app, screen, find, press = run(require "Weather/App", lint)
check(find("WeatherPanorama").Title == "weather" and requests[1]:find("latitude=37.7749"), "Weather uses the kit too")
check(find("Daysan-francisco:day:2026-11-19"):findChild("Verdict", true).Text ~= "", "Weather rates the days")
app.scope:dispose()

-- The four activity apps load, rate and switch spots.
local apps = {
    { "Sailing", "sailing", "tarifa", "garda" }, { "Hiking", "hiking", "zermatt", "fuji" },
    { "Garden", "garden", "london", "melbourne" }, { "Stargazing", "stars", "atacama", "brecon" },
}
for _, spec in ipairs(apps) do
    local name, title, first, other = spec[1], spec[2], spec[3], spec[4]
    requests = {}
    marineFailing = name == "Sailing"
    app, screen, find, press = run(require(name .. "/App"), lint)
    check(find("WeatherPanorama").Title == title, name .. " is titled " .. title)
    check(not find("ReportFailed").Visible and find("Now" .. first .. ":now:now"), name .. " loads a report")
    check(find("Now" .. first .. ":now:now"):findChild("NowVerdict", true).Text:match("^%a+$"), name .. " has a verdict")
    check(find("Reason" .. first .. ":reason:1") and find("Hour" .. first .. ":hour:2026-11-18T06:00")
        and find("Day" .. first .. ":day:2026-11-19") and find("Condition" .. first .. ":metric:1"),
        name .. " fills every section")
    for index = 1, 8 do
        local condition = screen:findChild("Condition" .. first .. ":metric:" .. index, true)
        if condition then
            local value = condition:findChild("MetricValue", true).Text
            check(value ~= "" and not value:find("nil") and not value:find("e%+"), name .. " condition " .. index .. ": " .. value)
        end
    end
    if name == "Garden" then
        -- Toned conditions take their verdict colour; plain ones stay white.
        -- Every app's package shares this process, so `accent` is whichever
        -- loaded last: check that the tone applies, not which accent it is.
        local frost = find("Condition" .. first .. ":metric:1"):findChild("MetricValue", true).ForegroundColor
        local rain = find("Condition" .. first .. ":metric:3"):findChild("MetricValue", true).ForegroundColor
        check(not (frost.R > 0.99 and frost.G > 0.99 and frost.B > 0.99), "\"no frost\" takes the good tone")
        check(rain.R > 0.99 and rain.G > 0.99 and rain.B > 0.99, "A plain condition is white")
    end
    press(find("Spot" .. other))
    check(find("Now" .. other .. ":now:now"), name .. " switches spots")
    app.scope:dispose()
end
marineFailing = false
check(#requests == 2, "Stargazing asks for one forecast per spot")

-- Stargazing's card looks ahead to the first dark hour.
app, screen, find = run(require "Stargazing/App", lint)
check(find("Nowatacama:now:now"):findChild("NowSpot", true).Text == "San Pedro de Atacama · from 19:00",
    "At 06:00 the stars card shows tonight from 19:00")
app.scope:dispose()

orca.async = savedAsync
print(("PASS: weather profiles (%d checks)"):format(checks))
