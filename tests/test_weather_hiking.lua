-- Hikers' Weather: is the trail safe and pleasant? The hiking activity's
-- rules on hand-built hours, its extra requests on recorded-shape JSON
-- (summit and trailhead, models, air, NWS alerts), its big answers (storm
-- clock, turnaround, trail), then the hiking edition running on its own
-- places (tests/weather_fixture.lua). This file belongs to the hiking
-- edition. No network: every answer is built here.
local F = dofile("tests/weather_fixture.lua").init { edition = "hiking" }
local check, hour, verdict, forecast = F.check, F.hour, F.verdict, F.forecast
local Hiking = require "Weather/activities/hiking"
local Forecast = require "Weather/forecast"
local Activities = require "Weather/activities"
local Settings = require "Weather/settings"
local Sources = require "Weather/editions/hiking/sources"
local Trail = require "Weather/editions/hiking/trail"
local Peaks = require "Weather/editions/hiking/peaks"

-- The rules on single hours.
check(verdict(Hiking, { feels = 14, cloud = 10 }).verdict == "good", "Mild, clear and calm is good hiking")
check(verdict(Hiking, { code = 96 }).verdict == "poor", "Thunder is poor hiking")
check(verdict(Hiking, { gust = 25 }).score <= 30, "Knock-down gusts are capped")
check(verdict(Hiking, { visibility = 500 }).score < verdict(Hiking).score, "Fog lowers the score")
forecast.elevation = 3000
local icy = Hiking.rate(hour { freezingLevel = 2500 }, forecast):result()
forecast.elevation = nil
check(F.says(icy, "ice"), "A freezing level below the spot warns of ice")
check(F.says(verdict(Hiking, { cape = 1500, lifted_index = -4, rainChance = 60 }), "Thunder likely"),
    "Storm energy with showers about reads as thunder likely")
check(verdict(Hiking, { wet_bulb_temperature_2m = 27 }).score <= 30, "Heat stress is capped")
check(F.says(verdict(Hiking, { aqi = 170 }), "Unhealthy air"), "Smoke lowers the score")
check(F.says(verdict(Hiking, { snow_depth = 0.2 }), "Snow lying"), "Lying snow is a reason")

-- Trail.thunder: an estimate in two words, never a percentage.
check(Trail.thunder(hour { code = 95 }) == "likely" and Trail.thunder(hour { cape = 600, rainChance = 30 }) == "possible"
    and Trail.thunder(hour { cape = 600, lifted_index = 2, rainChance = 30 }) == nil
    and Trail.thunder(hour { cape = 50 }) == nil, "Thunder is likely, possible or not shown")

-- Ridge wind (HI2): linear in height between pressure levels.
local speed, dir = Sources.ridge({ { height = 3000, speed = 20, dir = 270 }, { height = 1500, speed = 10, dir = 250 } }, 2250)
check(math.abs(speed - 15) < 1e-9 and dir == 270, "Ridge wind is interpolated by height")
check(Sources.ridge({ { height = 1500, speed = 10 } }, 900) == nil, "Below the lowest level there is no ridge wind")
check(Sources.ridge({ { height = 1500, speed = 10 }, { height = 3000, speed = 20 } }, 5000) == 20,
    "Above the highest level the top level's wind stands")

-- Every hiking place has a top above its start, with the US ones on NWS.
local Places = require "Weather/places"
for _, place in ipairs(Places) do
    local peak = Peaks[place.id]
    check(peak and peak.top.elevation > peak.start.elevation and peak.top.name and peak.start.name and peak.official,
        place.title .. " has a top and a start")
end

-- The requests: one multi-point request at both heights, models, air, and
-- alerts only where NWS covers.
local tahoe = { id = "tahoe", title = "Lake Tahoe", lat = 39.09, lon = -120.04, activities = { "hiking" } }
local request = Activities.request(tahoe)
check(request.pastDays == 3 and request.forecastDays == 16 and #request.sources == 4,
    "Hiking asks for three past days, sixteen ahead and four extra requests")
local byId = {}
for _, source in ipairs(request.sources) do byId[source.id] = source end
local pointsUrl = byId["hiking-points"].url(tahoe, Forecast)
check(pointsUrl:find("latitude=38.9227,38.9058", 1, true) and pointsUrl:find("elevation=1960,2968", 1, true)
    and pointsUrl:find("wind_speed_700hPa", 1, true) and pointsUrl:find("geopotential_height_600hPa", 1, true),
    "The trailhead and the top come in one request, at their own heights")
check(byId["hiking-models"].url(tahoe, Forecast):find("models=", 1, true), "The models request names several models")
check(byId["hiking-air"].url(tahoe, Forecast):find("air%-quality%-api"), "Air quality comes from its own API")
check(byId["hiking-alerts"].url(tahoe):find("api.weather.gov/alerts/active?point=38.9058,-120.0991", 1, true)
    and byId["hiking-alerts"].url({ id = "zermatt" }) == nil, "Warnings are asked for at US places only")
check(byId["hiking-points"].url({ id = "nowhere", lat = 1, lon = 2 }, Forecast) == nil, "A place without a top asks nothing")

-- Recorded-shape answers. The synthetic week starts 2026-11-18 00:00 UTC
-- (offset 0); the clock is 06:00, so the current hour is the seventh.
local series, iso, HOURS = F.series, F.iso, 7 * 24
local function point(elevation, columns)
    local hourly = { time = series(HOURS, iso) }
    local defaults = { temperature_2m = 6, apparent_temperature = 5, weather_code = 2, wind_speed_10m = 3,
        wind_gusts_10m = 6, wind_direction_10m = 90, cloud_cover = 20, visibility = 20000,
        precipitation_probability = 10, precipitation = 0, freezing_level_height = 2200, cape = 50, lifted_index = 3,
        snow_depth = 0, snowfall = 0 }
    for name, value in pairs(defaults) do hourly[name] = series(HOURS, function() return value end) end
    for name, fn in pairs(columns or {}) do hourly[name] = series(HOURS, fn) end
    return { latitude = 0, longitude = 0, utc_offset_seconds = 0, timezone = "GMT", elevation = elevation, hourly = hourly }
end
local brecon = { id = "brecon", title = "Brecon Beacons", lat = 51.88, lon = -3.44, activities = { "hiking" } }
local function week(extra) return Forecast.parse(F.weather(51.88, extra), nil, F.NOW, brecon) end

-- Both ends (HI1): the hour carries `top` and `start`, and the harder end rates.
local fc = week()
byId["hiking-points"].parse({ point(440), point(886, {
    wind_gusts_10m = function() return 24 end, apparent_temperature = function() return -3 end,
    wind_speed_925hPa = function() return 8 end, geopotential_height_925hPa = function() return 750 end,
    wind_speed_850hPa = function() return 16 end, geopotential_height_850hPa = function() return 1450 end,
    wind_direction_850hPa = function() return 250 end,
}) }, fc)
fc.spot = brecon
local now = fc.hours[fc.now]
check(now.top.elevation == 886 and now.start.elevation == 440 and now.top.feels == -3 and now.top.isDay == now.isDay,
    "Each hour has the top and the start, reading through to the hour")
check(math.abs(now.top.ridge - (8 + 8 * (886 - 750) / 700)) < 1e-6, "The top has a ridge wind from the levels")
local rated = Hiking.rate(now, fc):result()
check(rated.score <= 30 and F.says(rated, "Rated for the top"), "The windy top decides the verdict")
local cards = Hiking.cards(fc, brecon)
local byCard = {}
for _, card in ipairs(cards) do byCard[card.id] = card end
check(byCard.points and byCard.points.value == "-3° · 5°" and byCard.points.detail:find("Pen y Fan 886 m", 1, true)
    and byCard.points.detail:find("Storey Arms 440 m", 1, true), "The top · start card names both heights")
local metrics = Hiking.metrics(now, fc)
local labels = {}
for _, metric in ipairs(metrics) do labels[metric.label] = metric end
check(labels.top and labels.start and labels["ridge wind"] and labels["official word"].value == "MWIS · Met Office",
    "Metrics show both ends, the ridge wind and where to read the official word")

-- Storm clock (HI3): the first daylight hour with thunder signs today.
fc = week { cape = function(i) return i % 24 >= 14 and 1500 or 50 end,
    lifted_index = function(i) return i % 24 >= 14 and -4 or 3 end,
    precipitation_probability = function(i) return i % 24 >= 14 and 60 or 10 end }
local day = Trail.planDay(fc)
local storm = Trail.storm(fc, day)
check(storm and storm.hour.time == "2026-11-18T14:00" and storm.level == "likely" and storm.offBy == 13 * 60,
    "The storm clock finds 14:00 and says be off by 13:00")
cards = Hiking.cards(fc, brecon)
check(cards[1].id == "storm" and cards[1].value == "about 14:00" and cards[1].detail:find("estimate", 1, true)
    and cards[1].detail:find("off the summit by 13:00", 1, true), "The storm card is an estimate with a time")
local calm = Hiking.cards(week(), brecon)
check(calm[1].id == "storm" and calm[1].value == "no thunder signs" and not calm[1].value:find("%%"),
    "A calm day says no thunder signs, never 0 %")

-- Daylight budget and turnaround (HI4): defaults start 08:00, 3 h up, 2 h down.
local light = Trail.light(fc.days[1])
check(light.first > 6 * 60 and light.first < 8 * 60 and light.last > 16 * 60 and light.last < 18 * 60,
    "First and last light are civil twilight at Brecon in November")
local plan = Trail.plan(week())
local turn = (light.last - 120) - (light.last - 120) % 15
check(plan.today and plan.top == 11 * 60 and plan.turn == turn and not plan.late and plan.why == "dark",
    "The turnaround is last light minus the way down, to the quarter hour")
check(plan.left == light.last - light.first, "Before first light the whole day's light is left")
plan = Trail.plan(fc)
check(plan.turn == 13 * 60 and plan.why == "storm", "The storm clock brings the turnaround forward")
Settings.set("hiking", "start", 12)
plan = Trail.plan(week())
check(plan.late and plan.top == 15 * 60, "A late start reaches the top after the turnaround")
local turnCard
for _, card in ipairs(Hiking.cards(week(), brecon)) do if card.id == "turnaround" then turnCard = card end end
check(turnCard.tone == "poor" and turnCard.detail:find("start earlier", 1, true), "A late plan says start earlier")
Settings.set("hiking", "start", 6)
check(Trail.plan(week()).beforeLight, "A start before first light is noted")
check(Settings.text("hiking", "start") == "06:00" and Settings.text("hiking", "up") == "3 h", "The settings read as times")
Settings.set("hiking", "start", 8)
local evening = Forecast.parse(F.weather(51.88), nil, F.NOW + 13 * 3600, brecon)
check(not Trail.plan(evening).today, "After the hike's end today the plan is for tomorrow")

-- Trail conditions (HI5): snow at the top, what fell, freeze and thaw, rain
-- (precipitation that fell as snow is not rain).
fc = week { precipitation = function(i) return i < 6 and 4 or i == 6 and 6.1 or 0 end,
    snowfall = function(i) return i == 6 and 1.47 or 0 end }
byId["hiking-points"].parse({ point(440), point(886, {
    snow_depth = function() return 0.4 end, snowfall = function(i) return i <= 6 and 2 or 0 end,
    temperature_2m = function(i) local h = i % 24; return h <= 8 and -3 or h >= 11 and 3 or 0 end,
}) }, fc)
local trail = Trail.conditions(fc)
check(trail.snowDepth == 0.4 and trail.snowfall == 14 and math.abs(trail.rain24 - 28) < 1e-6 and trail.thaw.time == "2026-11-18T11:00",
    "Trail conditions read the top's snow, the last days and the thaw")
for _, card in ipairs(Hiking.cards(fc, brecon)) do byCard[card.id] = card end
check(byCard.trail.value == "snow at the top" and byCard.trail.tone == "poor"
    and byCard.trail.detail:find("40 cm", 1, true) and byCard.trail.detail:find("14 cm fell", 1, true)
    and byCard.trail.detail:find("streams high", 1, true) and byCard.trail.detail:find("thawing from about 11:00", 1, true),
    "The trail card leads with snow and lists the rest")
check(Trail.snow(40) == "40 cm", "Snow is in centimetres")
local zermatt = { id = "zermatt", title = "Zermatt", lat = 46.02, lon = 7.75, activities = { "hiking" } }
byCard.avalanche = nil
for _, card in ipairs(Hiking.cards(fc, zermatt)) do byCard[card.id] = card end
check(byCard.avalanche and byCard.avalanche.value == "SLF bulletin", "Snow on the mountain links the avalanche bulletin")
byCard.avalanche = nil
for _, card in ipairs(Hiking.cards(week(), zermatt)) do byCard[card.id] = card end
check(not byCard.avalanche, "No snow, no avalanche card")

-- Models (HI6): three models agree, then part ways.
check(Sources.agreement({ 10, 11, 10.5 }, { 0, 0.2, 0 }, { 8, 9, 10 }) == "sure"
    and Sources.agreement({ 10, 14 }, { 0, 0 }, { 8, 9 }) == "likely"
    and Sources.agreement({ 10, 17, 12 }, { 0, 0, 0 }, { 8, 9, 10 }) == "uncertain"
    and Sources.agreement({ 10, 10 }, { 0, 12 }, { 8, 9 }) == "uncertain"
    and Sources.agreement({ 10 }, { 0 }, { 8 }) == nil, "Model agreement is sure, likely or uncertain")
fc = week()
local daily = { time = series(7, function(d) return ("2026-11-%02d"):format(18 + d) end) }
for _, model in ipairs { "ecmwf_ifs025", "gfs_seamless", "icon_seamless" } do
    daily["temperature_2m_max_" .. model] = series(7, function(d) return d < 3 and 10 or (model == "gfs_seamless" and 18 or 9) end)
    daily["precipitation_sum_" .. model] = series(7, function() return 0 end)
    daily["wind_gusts_10m_max_" .. model] = series(7, function() return 8 end)
end
byId["hiking-models"].parse({ daily = daily }, fc)
local runs = Trail.confidence(fc)
check(#runs == 2 and runs[1].word == "sure" and runs[1].to == "2026-11-20" and runs[2].word == "uncertain",
    "The days run sure, then uncertain")
for _, card in ipairs(Hiking.cards(fc, brecon)) do byCard[card.id] = card end
check(byCard.confidence.value == "sure to fri 20" and byCard.confidence.detail:find("uncertain sat 21–tue 24", 1, true)
    and byCard.confidence.detail:find("3 models", 1, true), "The confidence card says how sure, and until when")
local far = fc.hours[fc.now + 4 * 24]
check(F.says(Hiking.rate(far, fc):result(), "models disagree"), "An uncertain day says so in its reasons")

-- Air (HI8): smoke at the trailhead.
fc = week()
local air = { time = series(5 * 24, iso), us_aqi = series(5 * 24, function(i) return i == 9 and 165 or 40 end),
    pm2_5 = series(5 * 24, function(i) return i == 9 and 80 or 8 end) }
byId["hiking-air"].parse({ hourly = air }, fc)
check(fc.hours[10].aqi == 165 and fc.hours[10].pm25 == 80, "Air quality lands on the hours")
for _, card in ipairs(Hiking.cards(fc, brecon)) do byCard[card.id] = card end
check(byCard.air and byCard.air.value == "unhealthy air" and byCard.air.detail:find("around 09:00", 1, true),
    "Smoke today shows as a card with its hour")
check(F.says(Hiking.rate(fc.hours[10], fc):result(), "Unhealthy air"), "The smoky hour rates lower")

-- Official warnings (HI7), in the shape api.weather.gov answers.
check(Sources.epoch("2026-11-18T10:00:00-08:00") == F.NOW + 12 * 3600 and Sources.epoch("2026-11-18T06:00:00Z") == F.NOW,
    "Alert times with an offset become UTC")
local nws = { type = "FeatureCollection", features = {
    { properties = { event = "Winter Storm Warning", severity = "Severe", senderName = "NWS Reno NV", status = "Actual",
        onset = "2026-11-17T20:18:00-08:00", ends = "2026-11-18T10:00:00-08:00",
        headline = "Winter Storm Warning issued November 17 by NWS Reno NV" } },
    { properties = { event = "Wind Advisory", severity = "Moderate", senderName = "NWS Reno NV",
        onset = "2026-11-16T05:00:00-08:00", expires = "2026-11-17T04:00:00-08:00" } },
} }
fc = week()
byId["hiking-alerts"].parse(nws, fc)
local active = Trail.alerts(fc)
check(#active == 1 and active[1].event == "Winter Storm Warning", "Expired warnings drop out")
for _, card in ipairs(Hiking.cards(fc, tahoe)) do byCard[card.id] = card end
check(Hiking.cards(fc, tahoe)[1].id == "warning" and byCard.warning.value == "winter storm warning"
    and byCard.warning.tone == "poor" and byCard.warning.detail:find("NWS Reno NV", 1, true),
    "An active warning leads the cards")
check(F.says(Hiking.rate(fc.hours[fc.now], fc):result(), "Official warning"), "A severe warning lowers the hours it covers")
check(not F.says(Hiking.rate(fc.hours[fc.now + 30], fc):result(), "Official warning"), "Hours after it ends are clear")
check(not pcall(byId["hiking-alerts"].parse, { hourly = {} }, week()), "An answer without a list is not 'no warnings'")

-- The whole request on a fake network, and offline replay.
local net = F.network()
net.answers["elevation=1960"] = function() return { point(1960), point(2968, { snow_depth = function() return 0.3 end }) } end
net.answers["models="] = function() return { daily = daily } end
net.answers["air%-quality"] = function() return { hourly = air } end
net.answers["weather%.gov"] = function() return nws end
local loaded, err, raw = Forecast.load(tahoe, false, net.fetch, F.NOW, request)
check(loaded and not err and next(loaded.missing) == nil and loaded.hours[loaded.now].top.snow_depth == 0.3
    and loaded.hiking.confidence and #loaded.hiking.alerts == 2, "Every hiking source loads")
local restored = Forecast.restore(raw, tahoe, request, F.NOW + 3600)
check(restored and restored.hours[restored.now].top.elevation == 2968, "Offline, the saved top and start come back")
net.answers["elevation=1960"] = function() return nil, "No connection to the forecast service." end
loaded = Forecast.load(tahoe, false, net.fetch, F.NOW, request)
check(loaded and loaded.missing["hiking-points"] and not loaded.hours[loaded.now].top
    and Hiking.rate(loaded.hours[loaded.now], loaded), "Without the points the place's own forecast still rates")

-- The edition, then its activity page on the recorded answers.
F.checkEdition("hiking")
net.answers["elevation=1960"] = function() return { point(1960), point(2968) } end
local ui = F.launch { places = { tahoe }, net = net, storageName = "hiking-page" }
check(ui.text("Cardtahoe:card:warning", "CardValue") == "winter storm warning"
    and ui.has("Cardtahoe:card:storm") and ui.has("Cardtahoe:card:turnaround"), "The hub opens on hiking's answers")
ui.open("tahoe", "hiking")
check(ui.text("Cardtahoe:hiking:card:points", "CardValue") == "5° · 5°"
    and ui.text("Cardtahoe:hiking:card:turnaround", "CardValue"):find("^turn by"), "The activity page shows the answers")
ui.back()
ui.app.hubScope:dispose()
F.done("weather-hiking")
