-- Hikers' Weather: is the trail safe and pleasant? The hiking activity's
-- rules on hand-built hours, its extra requests on recorded-shape JSON
-- (summit and trailhead, models, air, NWS alerts), its big answers (storm
-- clock, turnaround, trail), the other services (avalanche, fire, snow,
-- park alerts, lightning; tests/fixtures/hiking/), then the hiking edition running on its own
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
check(request.pastDays == 3 and request.forecastDays == 16 and #request.sources == 11,
    "Hiking asks for three past days, sixteen ahead and eleven optional extra requests")
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

-- Existing services (Research/services-hiking.md), each optional, on
-- synthetic answers in their documented shapes (tests/fixtures/hiking/).
local Services = require "Weather/editions/hiking/services"
local Avalanche = require "Weather/editions/hiking/avalanche"
local Provider = require "Weather/provider"
local fixture = function(name) return dofile("tests/fixtures/hiking/" .. name .. ".lua") end
local US, SLF, EUREGIO, SERVICES = fixture "avalanche-org", fixture "caaml-slf", fixture "caaml-euregio", fixture "services"
local function cardOf(fc, place, id)
    for _, card in ipairs(Hiking.cards(fc, place)) do if card.id == id then return card end end
end
local function snowyWeek() return week { snow_depth = function() return 0.5 end } end
local yosemite = { id = "yosemite", title = "Yosemite Valley", lat = 37.75, lon = -119.59, activities = { "hiking" } }

-- SV-HI1: avalanche.org, off until the National Avalanche Center agrees.
check(byId["hiking-avalanche"].url(tahoe, Forecast) == nil, "avalanche.org is not asked before its terms are settled")
Services.CLEARED.avalancheOrg = true
check(byId["hiking-avalanche"].url(tahoe, Forecast) == "https://api.avalanche.org/v2/public/products/map-layer/SAC"
    and byId["hiking-avalanche"].url(tahoe, Forecast, week()) == nil
    and byId["hiking-avalanche"].url(tahoe, Forecast, snowyWeek()), "Once cleared, one centre's zones, only with snow")
check(Avalanche.inside({ lat = 38.9, lon = -120.1 }, US.features[1].geometry)
    and not Avalanche.inside({ lat = 38.9, lon = -120.1 }, US.features[2].geometry)
    and not Avalanche.inside({ lat = 0.5, lon = 0.5 }, { type = "Polygon", coordinates = {
        { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 } }, { { 0.4, 0.4 }, { 0.6, 0.4 }, { 0.6, 0.6 }, { 0.4, 0.6 }, { 0.4, 0.4 } } } }),
    "Point in polygon: inside, outside, and in a hole")
fc = week()
byId["hiking-avalanche"].parse(US, fc)
local avalanche = cardOf(fc, tahoe, "avalanche")
check(avalanche and avalanche.value == "3 considerable · Central Sierra Nevada" and avalanche.tone == "poor"
    and avalanche.detail:find("Sierra Avalanche Center · until 18:00", 1, true)
    and avalanche.detail:find("read the full forecast: https://www.sierraavalanchecenter.org/forecasts/#/central-sierra-nevada", 1, true),
    "A current US rating keeps its level and word, the zone, the centre, until when and the link: " .. tostring(avalanche and avalanche.detail))
local later = Forecast.parse(F.weather(51.88, { snow_depth = function() return 0.5 end }), nil, F.NOW + 13 * 3600, brecon)
byId["hiking-avalanche"].parse(US, later)
avalanche = cardOf(later, tahoe, "avalanche")
check(avalanche.value == "no current rating" and avalanche.tone == "info" and avalanche.detail:find("expired", 1, true)
    and not (avalanche.value .. avalanche.detail):find("considerable"), "An expired rating never shows the last level")
local mono = { id = "mono", top = { name = "a Mono top", lat = 45, lon = 0, elevation = 3000 }, start = Peaks.tahoe.start,
    avalanche = { "Sierra Avalanche Center", "sierraavalanchecenter.org", center = "SAC", zone = "Mono" } }
check(Avalanche.mapLayer(US, mono).zone == "Mono" and not Avalanche.current(Avalanche.mapLayer(US, mono), fc),
    "Outside every polygon the named zone is used, and its expired rating is no rating")
fc = week()
byId["hiking-avalanche"].parse(US, fc)
check(not cardOf(fc, mono, "avalanche"), "No rating and no snow: no card")
fc = snowyWeek()
fc.missing["hiking-avalanche"] = "The forecast service is not answering."
avalanche = cardOf(fc, tahoe, "avalanche")
check(avalanche.value == "no current rating" and avalanche.detail:find("did not load", 1, true),
    "A bulletin that did not load says so, never 'safe'")
Services.CLEARED.avalancheOrg = false

-- SV-HI2: an EAWS bulletin at the top's height.
check(byId["hiking-bulletin"].url(zermatt, Forecast) == nil, "Without a micro-region id no bulletin is asked")
local matterhorn = { id = "matterhorn", top = Peaks.zermatt.top, start = Peaks.zermatt.start,
    avalanche = { "SLF bulletin", "slf.ch", caaml = { provider = "slf", region = "CH-TEST-1" } } }
local slfUrl, slfOptions = byId["hiking-bulletin"].url(matterhorn, Forecast), Forecast.requestOptions(byId["hiking-bulletin"].headers)
check(slfUrl and slfUrl:find("aws.slf.ch/api/bulletin/caaml", 1, true) and slfOptions.headers.Accept == "application/json",
    "SLF (CC BY 4.0) is asked as JSON")
check(Avalanche.aspects { "N", "NE", "E" } == "N–E" and Avalanche.aspects { "NW", "N", "NE" } == "NW–NE"
    and Avalanche.aspects { "N", "S" } == "N, S" and Avalanche.aspects { "N", "NE", "E", "SE", "S", "SW", "W", "NW" } == "all aspects",
    "Aspects read as runs")
fc = week()
byId["hiking-bulletin"].parse(SLF, fc)
avalanche = cardOf(fc, matterhorn, "avalanche")
check(avalanche and avalanche.value == "3 considerable above 2200 m" and avalanche.tone == "poor"
    and avalanche.detail:find("wind slab, persistent weak layers NW–E", 1, true) and not avalanche.detail:find("gliding")
    and avalanche.detail:find("published tue 17 16:00", 1, true) and avalanche.detail:find("next update 07:00", 1, true)
    and avalanche.detail:find("slf.ch", 1, true), "The top's band, its problems and aspects, the times and the link: "
    .. tostring(avalanche and avalanche.detail))
local low = { id = "low", top = { name = "a low top", lat = 46, lon = 7.7, elevation = 1900 }, start = Peaks.zermatt.start,
    avalanche = matterhorn.avalanche }
fc = snowyWeek()
byId["hiking-bulletin"].parse(SLF, fc)
avalanche = cardOf(fc, low, "avalanche")
check(avalanche.value == "2 moderate below 2200 m" and avalanche.detail:find("gliding snow S", 1, true),
    "A lower top reads the lower band")
local nowhere = { id = "nowhere", top = Peaks.zermatt.top, start = Peaks.zermatt.start,
    avalanche = { "SLF bulletin", "slf.ch", caaml = { provider = "slf", region = "CH-NONE" } } }
avalanche = cardOf(fc, nowhere, "avalanche")
check(avalanche.value == "no current rating" and avalanche.detail:find("no bulletin for this region", 1, true),
    "A region the bulletin does not cover is no rating")
fc = Forecast.parse(F.weather(51.88), nil, F.NOW + 11 * 3600, brecon)
byId["hiking-bulletin"].parse(SLF, fc)
check(not Avalanche.current(Avalanche.caaml(SLF, matterhorn), fc), "Past its valid time a bulletin is no rating")

-- Euregio: off until its licence is settled; am/pm split and treeline bounds.
local altissimo = { id = "altissimo", top = Peaks.garda.top, start = Peaks.garda.start,
    avalanche = { "Euregio avalanche report", "avalanche.report", caaml = { provider = "euregio", region = "IT-32-TN-TEST" } } }
check(byId["hiking-bulletin"].url(altissimo, Forecast) == nil, "Euregio is not asked before its licence is settled")
fc = week()
byId["hiking-bulletin"].parse(EUREGIO, fc)
avalanche = cardOf(fc, altissimo, "avalanche")
check(avalanche.value == "2 moderate above the treeline" and avalanche.detail:find("treeline height not known", 1, true)
    and avalanche.detail:find("new snow N", 1, true) and avalanche.detail:find("rising to 3 considerable later in the day", 1, true),
    "Morning: the higher band where the treeline is unknown, and the afternoon's rise: " .. avalanche.detail)
altissimo.avalanche.caaml.treeline = 1800
avalanche = cardOf(fc, altissimo, "avalanche")
check(avalanche.value == "2 moderate above 1800 m" and not avalanche.detail:find("not known"), "A known treeline is a height")
local afternoon = Forecast.parse(F.weather(51.88), nil, F.NOW + 7 * 3600, brecon)
byId["hiking-bulletin"].parse(EUREGIO, afternoon)
avalanche = cardOf(afternoon, altissimo, "avalanche")
check(avalanche.value == "3 considerable above 1800 m" and avalanche.detail:find("wet snow E–SW", 1, true),
    "In the afternoon the later rating and its problems")

-- SV-HI3: fires near US trailheads, from public-domain WFIGS layers.
local fireUrl = byId["hiking-fire"].url(tahoe)
check(fireUrl:find("WFIGS_Interagency_Perimeters_Current/FeatureServer/0/query?geometry=-120.0714,38.9227", 1, true)
    and fireUrl:find("distance=30&units=esriSRUnit_Kilometer", 1, true) and fireUrl:find("returnCentroid=true", 1, true)
    and fireUrl:find("poly_IncidentName", 1, true) and not fireUrl:find("Date%%3E") and byId["hiking-fire"].url(zermatt) == nil,
    "One point-and-distance query at US trailheads, no relative date filter")
check(byId["hiking-fire-starts"].url(tahoe):find("WFIGS_Incident_Locations_Current", 1, true), "New starts come from incident points")
fc = week()
byId["hiking-fire"].parse(SERVICES.fire, fc)
local fireCard = cardOf(fc, tahoe, "fire")
check(fireCard and fireCard.value == "fire 12 km NE" and fireCard.tone == "fair"
    and fireCard.detail:find("Cascade · 931 ha · 45 % contained · updated 06:00", 1, true)
    and fireCard.detail:find("check Lake Tahoe Basin forest orders, fs.usda.gov/ltbmu", 1, true),
    "A fire 12 km off is fair, with its size, containment, update and the forest order: " .. tostring(fireCard and fireCard.value))
byId["hiking-fire-starts"].parse(SERVICES.starts, fc)
fireCard = cardOf(fc, tahoe, "fire")
check(fireCard.value == "fire 5.0 km W" and fireCard.tone == "poor" and fireCard.detail:find("Fallen Leaf (new start)", 1, true)
    and fireCard.detail:find("2 fires within", 1, true), "A new start under 10 km is poor; the same fire is not counted twice")
fc = week()
byId["hiking-fire"].parse(SERVICES.fireNone, fc)
check(not cardOf(fc, tahoe, "fire"), "Nothing within 30 km: no card, and nothing says 'no fires'")
check(not pcall(byId["hiking-fire"].parse, SERVICES.fireError, week()), "An ArcGIS error is a missing source, not 'no fires'")

-- SV-HI4: measured snow from a SNOTEL station near the top.
local snowUrl = byId["hiking-snow"].url(tahoe)
check(snowUrl:find("awdbRestApi/services/v1/data?stationTriplets=463:CA:SNTL&elements=SNWD&duration=DAILY&beginDate=", 1, true)
    and byId["hiking-snow"].url(zermatt) == nil, "SNOTEL is asked for the curated station's snow depth")
fc = week()
byId["hiking-snow"].parse(SERVICES.snotel, fc)
local trailCard = cardOf(fc, tahoe, "trail")
check(trailCard.value == "snow measured nearby" and trailCard.tone == "fair"
    and trailCard.detail:find("measured 84 cm at Echo Peak (2377 m), yesterday; +20 cm in 3 days", 1, true),
    "The trail card gains the measured depth, its station and height, its day and the change: " .. trailCard.detail)
local stale = { { stationTriplet = "463:CA:SNTL", data = { { values = { { date = "2026-11-15", value = 40 } } } } } }
fc = week()
byId["hiking-snow"].parse(stale, fc)
check(fc.hiking.measured.stale and not cardOf(fc, tahoe, "trail").detail:find("measured"), "A reading older than two days is dropped")

-- SV-HI5: NPS alerts, only with a key, sent as a header.
check(byId["hiking-park"].url(yosemite) == nil, "Without an NPS key no park alerts are asked")
Forecast.endpoints.npsKey = "test-key"
local parkUrl = byId["hiking-park"].url(yosemite)
local parkOptions = Forecast.requestOptions(byId["hiking-park"].headers, yosemite)
check(parkUrl == "https://developer.nps.gov/api/v1/alerts?parkCode=yose&limit=20" and not parkUrl:find("test%-key")
    and parkOptions.headers["X-Api-Key"] == "test-key" and byId["hiking-park"].url(tahoe) == nil,
    "With a key, the park's alerts, the key in a header")
local credits = {}
for _, about in ipairs(Provider.about()) do credits[about[1]] = true end
check(credits["Park alerts: National Park Service"], "With a key the NPS is credited")
Forecast.endpoints.npsKey = nil
fc = week()
byId["hiking-park"].parse(SERVICES.nps, fc)
local parkCard = cardOf(fc, yosemite, "park")
check(parkCard and parkCard.value == "park closure" and parkCard.tone == "poor"
    and parkCard.detail:find("Half Dome cables are down", 1, true) and parkCard.detail:find("nps.gov/yose/planyourvisit/halfdome.htm", 1, true),
    "A park closure is poor, with its title and link")
fc = week()
byId["hiking-park"].parse({ data = { SERVICES.nps.data[2] } }, fc)
check(not cardOf(fc, yosemite, "park"), "Information alone gives no card")

-- SV-HI7: ICON-D2 lightning potential names the storm clock's hour.
check(byId["hiking-lightning"].url(zermatt, Forecast):find("models=icon_d2", 1, true)
    and byId["hiking-lightning"].url(zermatt, Forecast):find("lightning_potential", 1, true)
    and byId["hiking-lightning"].url(tahoe, Forecast) == nil, "Lightning potential is asked inside ICON-D2's domain only")
fc = week()
byId["hiking-lightning"].parse(SERVICES.lightning(series, iso), fc)
storm = Trail.storm(fc, Trail.planDay(fc))
local stormCard = cardOf(fc, zermatt, "storm")
check(storm and storm.hour.time == "2026-11-18T14:00" and storm.level == "likely"
    and stormCard.detail:find("model estimate (lightning potential): thunder likely from about 14:00", 1, true),
    "A lightning spike sets the storm clock, worded as a model estimate")

-- SV-HI8: links where no open API can be used.
check(Peaks.garda.official[2][1] == "MeteoAlarm" and Peaks.zermatt.official[2][1] == "MeteoAlarm"
    and Peaks.garda.avalanche[2] == "avalanche.report" and Peaks.lofoten.avalanche[2] == "varsom.no"
    and Peaks.tahoe.forest[2] == "fs.usda.gov/ltbmu", "Official links stand in where terms say ask first")

-- Credit: the services that are on join the settings' "about".
credits = {}
for _, about in ipairs(Provider.about()) do credits[about[1]] = true end
check(credits["Fire data: NIFC / WFIGS"] and credits["Snow: USDA NRCS SNOTEL"]
    and credits["Avalanche bulletin: WSL Institute for Snow and Avalanche Research SLF"]
    and not credits["Park alerts: National Park Service"] and not credits["Avalanche bulletin: Euregio Tyrol, South Tyrol, Trentino"],
    "About credits the services in use, and only those")

-- The whole request on a fake network, and offline replay.
local net = F.network()
net.answers["elevation=1960"] = function() return { point(1960), point(2968, { snow_depth = function() return 0.3 end }) } end
net.answers["models="] = function() return { daily = daily } end
net.answers["air%-quality"] = function() return { hourly = air } end
net.answers["weather%.gov"] = function() return nws end
net.answers["Perimeters_Current"] = function() return SERVICES.fire end
net.answers["Incident_Locations_Current"] = function() return SERVICES.fireNone end
net.answers["awdbRestApi"] = function() return SERVICES.snotel end
local loaded, err, raw = Forecast.load(tahoe, false, net.fetch, F.NOW, request)
check(loaded and not err and next(loaded.missing) == nil and loaded.hours[loaded.now].top.snow_depth == 0.3
    and loaded.hiking.confidence and #loaded.hiking.alerts == 2 and #loaded.hiking.fires == 1
    and loaded.hiking.measured.cm, "Every hiking source loads")
local restored = Forecast.restore(raw, tahoe, request, F.NOW + 3600)
check(restored and restored.hours[restored.now].top.elevation == 2968, "Offline, the saved top and start come back")
net.answers["elevation=1960"] = function() return nil, "No connection to the forecast service." end
loaded = Forecast.load(tahoe, false, net.fetch, F.NOW, request)
check(loaded and loaded.missing["hiking-points"] and not loaded.hours[loaded.now].top
    and Hiking.rate(loaded.hours[loaded.now], loaded), "Without the points the place's own forecast still rates")

-- Wave 3: the heights a hiker sets in the place editor come before the
-- table; a place found by search needs both, and a US one gets warnings.
do
    local MyPlaces = require "Weather/myplaces"
    MyPlaces.init { curated = require "Weather/places" }
    local draft = MyPlaces.fromResult({ id = 5706152, name = "Matterhorn", latitude = 41.81074, longitude = -115.37534,
        elevation = 3250, feature_code = "MT", country_code = "US", country = "United States", admin1 = "Nevada" })
    local found = MyPlaces.resolveDraft(draft)
    check(table.concat(found.activities, ",") == "hiking" and found.summit == 3250 and Sources.peak(found) == nil,
        "A found peak in Hikers' Weather is for hiking; with no trailhead yet it asks no points")
    local field = MyPlaces.field(found, "trailhead")
    MyPlaces.apply(draft, MyPlaces.next(found, field, 1))
    found = MyPlaces.resolveDraft(draft)
    local peak = Sources.peak(found)
    check(peak and peak.top.elevation == 3250 and peak.start.elevation == 500 and peak.top.lat == found.lat and peak.nws,
        "Both heights make the two points, at the place itself; a US place has warnings")
    check(byId["hiking-points"].url(found, Forecast):find("elevation=500,3250", 1, true)
        and byId["hiking-alerts"].url(found):find("api.weather.gov", 1, true), "and asks for them")
    local id = MyPlaces.add("tahoe")
    MyPlaces.set(id, "summit", 3000)
    local own = Sources.peak(MyPlaces.find("tahoe"))
    check(own.top.elevation == 3000 and own.top.name == "Mount Tallac" and own.start.elevation == 1960
        and own.avalanche and Peaks.tahoe.top.elevation == 2968, "A curated peak keeps its table under the user's height")
    check(MyPlaces.text(MyPlaces.find("tahoe"), "trailhead") == "1960 m", "The editor shows the table's height as the default")
    MyPlaces.init { curated = require "Weather/places" }
end

-- The edition, then its activity page on the recorded answers.
F.checkEdition("hiking")
net.answers["elevation=1960"] = function() return { point(1960), point(2968) } end
local ui = F.launch { places = { tahoe }, net = net, storageName = "hiking-page" }
-- I2: the hub keeps the three most urgent answers, the warning first; the
-- measured snow brings the avalanche link up, so a calm storm clock drops off.
check(ui.text("Cardtahoe:card:warning", "CardValue") == "winter storm warning" and ui.cards()[1] == "Cardtahoe:card:warning"
    and ui.has("Cardtahoe:card:avalanche") and not ui.has("Cardtahoe:card:storm") and #ui.cards() == 3, "The hub opens on hiking's answers: " .. table.concat(ui.cards(), " "))
ui.open("tahoe", "hiking")
check(ui.has("Cardtahoe:hiking:card:warning") and ui.has("Cardtahoe:hiking:card:storm") and #ui.cards() > 3,
    "Every answer stays on the activity page")
check(ui.text("Cardtahoe:hiking:card:points", "CardValue") == "5° · 5°"
    and ui.text("Cardtahoe:hiking:card:turnaround", "CardValue"):find("^turn by"), "The activity page shows the answers")
ui.back()
ui.app.hubScope:dispose()
F.done("weather-hiking")
