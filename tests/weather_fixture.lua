-- Shared harness for the weather app's tests (tests/test_weather*.lua):
-- ORCA started headless on apps/Weather, a check counter, hand-built hours,
-- a synthetic Open-Meteo week with a fake fetch, a fixed clock, a scratch
-- storage folder, and the app driven through its real pages.
--
--   local F = dofile("tests/weather_fixture.lua").init { edition = "surf" }
--   F.check(F.verdict(Surf, { swellHeight = 1.5 }, { facing = 270 }).verdict == "good", "…")
--   local ui = F.launch { places = F.PLACES }   -- ui.app, ui.find, ui.text, ui.press, ui.settle
--   F.done("weather-surf")                      -- prints "PASS: weather-surf (n checks)"
--
-- `edition` runs the whole file as that edition (as EDITION=<key> bundles
-- Weather/build.lua), so call init before requiring any Weather module.
local Fixture = {}

Fixture.NOW = 1794981600 -- 2026-11-18 06:00 UTC, a Wednesday

function Fixture.init(options)
    options = options or {}
    local F = setmetatable({}, { __index = Fixture })
    if options.edition then package.loaded["Weather/build"] = { edition = options.edition } end
    local orca = require "orca"
    orca.init()
    local fs = require "orca.filesystem"
    require "orca.UIKit"
    require("orca.renderer").init(390, 844, true)
    fs.init("apps/Weather")
    require("orca.core.startup").load_plugins()
    F.orca, F.core = orca, require "orca.core"
    F.savedAsync = orca.async
    orca.async = function(fn, ...) return fn(...) end
    F.checks = 0
    function F.check(value, message) assert(value, message); F.checks = F.checks + 1 end
    -- Default units are metric whatever the machine's region.
    F.region = options.region or "NL"
    return F
end

function Fixture.done(name)
    local F = Fixture.current
    if F and F.savedAsync then F.orca.async = F.savedAsync end
    print(("PASS: %s (%d checks)"):format(name, F and F.checks or 0))
end

-- An hour at Lake-Tahoe-ish dawn; `fields` change conditions.
function Fixture.hour(fields)
    local h = { time = "2026-11-18T07:00", date = "2026-11-18", temp = 12, feels = 11, code = 2, wind = 3, gust = 6,
        windDir = 90, pressure = 1015, pressureTrend = -1, cloud = 50, rainChance = 10, isDay = true, uv = 1,
        day = { date = "2026-11-18", sunrise = "2026-11-18T06:45", sunset = "2026-11-18T17:00" } }
    for k, v in pairs(fields or {}) do h[k] = v end
    return h
end

-- The forecast table `verdict` rates against; tests may set fields on it.
Fixture.forecast = { epoch = Fixture.NOW }

-- An activity's result for one hand-built hour at a spot.
function Fixture.verdict(profile, fields, spot)
    Fixture.forecast.spot = spot
    return profile.rate(Fixture.hour(fields), Fixture.forecast):result()
end

-- Whether any reason's text matches a Lua pattern.
function Fixture.says(result, pattern)
    for _, reason in ipairs(result.reasons) do
        if reason.text:find(pattern) then return true end
    end
    return false
end

-- A synthetic week from Open-Meteo, the shape the forecast API returns.
function Fixture.series(n, fn) local t = {} for i = 1, n do t[i] = fn(i - 1) end return t end
function Fixture.iso(i) return ("2026-11-%02dT%02d:00"):format(18 + i // 24, i % 24) end

-- `extra` adds hourly columns: { name = function(i) … end }.
function Fixture.weather(lat, extra)
    local series, iso, hours = Fixture.series, Fixture.iso, 7 * 24
    local result = {
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
    for name, fn in pairs(extra or {}) do result.hourly[name] = series(hours, fn) end
    return result
end

function Fixture.marine()
    local series, hours = Fixture.series, 7 * 24
    return { hourly = {
        time = series(hours, Fixture.iso),
        wave_height = series(hours, function() return 1.8 end), wave_period = series(hours, function() return 11 end),
        wave_direction = series(hours, function() return 280 end),
        swell_wave_height = series(hours, function() return 1.5 end),
        swell_wave_period = series(hours, function() return 13 end),
        swell_wave_direction = series(hours, function() return 280 end),
        sea_surface_temperature = series(hours, function() return 13.5 end),
    } }
end

-- A fake network: `net.fetch(url, options)` answers from the synthetic week
-- and records each URL in `net.requests` (its options in `net.options`). Set `net.failing` (everything) or
-- `net.marineFailing`; `net.answers[pattern] = fn(url)` adds other hosts.
function Fixture.network()
    local net = { requests = {}, options = {}, failing = false, marineFailing = false, answers = {} }
    function net.fetch(url, options)
        net.requests[#net.requests + 1] = url
        net.options[#net.requests] = options
        if net.failing or (net.marineFailing and url:find("marine%-api")) then
            return nil, "No connection to the forecast service."
        end
        for pattern, answer in pairs(net.answers) do
            if url:find(pattern) then return answer(url) end
        end
        if url:find("marine%-api") then return Fixture.marine() end
        return Fixture.weather(tonumber(url:match("latitude=([%-%d%.]+)")))
    end
    return net
end

-- A scratch folder for Storage, emptied of the files the app writes.
function Fixture.storage(name, places)
    local Storage = require "Weather/storage"
    local base = (os.getenv("TMPDIR") or "/tmp/"):gsub("/?$", "/")
    local store = Storage.new { dir = base .. "orca-weather-tests/" .. name }
    store:remove("settings")
    for _, place in ipairs(places or {}) do store:remove("forecast-" .. place.id) end
    return store
end

-- A few places with every kind of shore.
Fixture.PLACES = {
    { id = "tahoe", title = "Lake Tahoe", region = "California", lat = 39.09, lon = -120.04, lake = true, facing = 90,
        activities = { "fishing", "hiking", "stargazing" } },
    { id = "ocean-beach", title = "Ocean Beach", region = "San Francisco", lat = 37.76, lon = -122.52, facing = 270,
        activities = { "surf" } },
    { id = "bondi", title = "Bondi", region = "Sydney", lat = -33.89, lon = 151.28, facing = 110, activities = { "surf" } },
}

-- Starts the app on the fake network and fixed clock at "/". Options:
-- places (default PLACES), net (default a new Fixture.network()), clock,
-- storage, region. Returns helpers bound to it.
function Fixture.launch(options)
    options = options or {}
    local F = Fixture.current or {}
    local places = options.places or Fixture.PLACES
    local net = options.net or Fixture.network()
    local App = require "Weather/App"
    local app = App { places = places, fetch = net.fetch, clock = options.clock or function() return Fixture.NOW end,
        storage = options.storage or Fixture.storage(options.storageName or "app", places),
        region = options.region or F.region or "NL" }
    app:activate_route("/")
    local core = require "orca.core"
    local ui = { app = app, net = net }
    function ui.settle()
        for _ = 1, 3 do
            core.flushQueue()
            core.advanceFrame()
            core.runAllPrograms(app.screen)
            core.flushQueue()
        end
        app.screen:UpdateLayout(390, 844)
    end
    function ui.find(name) return assert(app.screen:findChild(name, true), "Missing " .. name) end
    function ui.has(name) return app.screen:findChild(name, true) ~= nil end
    function ui.text(row, child) return ui.find(row):findChild(child, true).Text end
    function ui.press(name) ui.find(name):send("Node.LeftButtonUp"); ui.settle() end
    function ui.back(pivot) ui.find(pivot or "ActivityPivot"):post("Pivot.BackRequested"); ui.settle() end
    -- The names of the card rows on screen ("Cardtahoe:card:frost"), in order.
    function ui.cards()
        local list, node = {}, ui.find("CardItems"):getFirstChild()
        while node do
            if node.Visible ~= false then list[#list + 1] = node.Name end
            node = node:getNext()
        end
        return list
    end
    -- Opens an activity from the selected place's "good for" row.
    function ui.open(placeId, activityId) ui.press("GoodFor" .. placeId .. ":activity:" .. activityId) end
    ui.settle()
    return ui
end

-- The checks every edition shares: it runs on its own places, rates
-- everyday outdoors and its activity, titles its activity page with the
-- app's name when that fits (F9), and its settings name it.
function Fixture.checkEdition(key)
    local check = Fixture.current.check
    local Activities = require "Weather/activities"
    local Places = require "Weather/places"
    local activity = Activities.byId[key]
    check(Activities.edition == key and #Activities.list == 2 and Activities.list[1].id == "outdoors" and activity,
        "The " .. key .. " edition rates outdoors and " .. key)
    check(Activities.appName == Activities.names[key], "The edition is called " .. Activities.appName)
    local ui = Fixture.launch { places = Places, storageName = key, region = "NL" }
    local place = Places[1]
    -- The edition's class; its own page classes (red mode's "night") may follow.
    check(ui.find("HubPanorama").Title == place.title and ui.find("HubPage").StateClasses:match("^%S+") == key,
        "The edition opens on " .. place.title .. " in its accent")
    check(ui.has("GoodFor" .. place.id .. ":activity:outdoors") and ui.has("GoodFor" .. place.id .. ":activity:" .. key),
        place.title .. " is good for outdoors and " .. key)
    for _, other in ipairs { "fishing", "surf", "sailing", "hiking", "garden", "stargazing" } do
        if other ~= key then check(not ui.has("GoodFor" .. place.id .. ":activity:" .. other), "No " .. other .. " here") end
    end
    ui.open(place.id, key)
    local title = ui.find("ActivityPivot").Title
    local named = Activities.appName:lower() .. " · " .. place.title
    check(title == (utf8.len(named) <= 34 and named or activity.title .. " · " .. place.title),
        "The activity page is titled " .. title)
    ui.back()
    ui.press("OpenSettings")
    check(ui.find("SettingsPivot").Title == Activities.appName:lower()
        and ui.find("ActivitySettingsSection").Header == activity.title, "Settings carry the edition's name")
    ui.back("SettingsPivot")
    ui.app.hubScope:dispose()
    return ui
end

-- `init` remembers its harness for done and launch.
local init = Fixture.init
function Fixture.init(options)
    Fixture.current = init(options)
    return Fixture.current
end

return Fixture
