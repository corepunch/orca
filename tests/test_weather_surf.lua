-- Surfers' Weather: is it worth paddling out? The surf activity's rules on
-- hand-built hours, then the surf edition running on its own places
-- (tests/weather_fixture.lua). This file belongs to the surf edition.
local F = dofile("tests/weather_fixture.lua").init { edition = "surf" }
local check, verdict, says = F.check, F.verdict, F.says
local Surf = require "Weather/activities/surf"
local Breaks = require "Weather/editions/surf/breaks"
local Waves = require "Weather/editions/surf/waves"
local Tides = require "Weather/editions/surf/tides"
local Outlook = require "Weather/editions/surf/outlook"
local Forecast = require "Weather/forecast"
local Activities = require "Weather/activities"
local Settings = require "Weather/settings"
local Report = require "Weather/report"
local Places = require "Weather/places"

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

local function with(fields)
    local hour = {}
    for k, v in pairs(clean) do hour[k] = v end
    for k, v in pairs(fields) do hour[k] = v end
    return hour
end

-- SU1: every swell train apart, plus the peak period.
do
    local hour = F.hour(with { secondary_swell_wave_height = 0.6, secondary_swell_wave_period = 9,
        secondary_swell_wave_direction = 200, wind_wave_height = 0.3, wind_wave_period = 4, wind_wave_direction = 270,
        swell_wave_peak_period = 14 })
    local trains = Waves.trains(hour)
    check(#trains == 3 and trains[1].name == "swell" and trains[2].name == "secondary swell"
        and trains[3].name == "wind sea", "The primary, secondary swell and wind sea are told apart")
    check(trains[1].period == 14, "The primary swell is rated on its peak period")
    F.forecast.spot = beach
    local labels = {}
    for _, metric in ipairs(Surf.metrics(hour, F.forecast)) do labels[metric.label] = metric end
    check(labels.swell and labels["secondary swell"] and labels["wind sea"]
        and labels.swell.note:find("peak 14 s") and labels["wind sea"].note:find("local chop"),
        "The conditions list each train, with the peak period")
    check(#Waves.trains(F.hour { waveHeight = 1.2, wavePeriod = 9, waveDir = 280 }) == 1,
        "Without separate trains the combined sea stands in")
    local url = select(2, Forecast.urls(Places[1], true, Activities.request(Places[1])))
    check(url:find("secondary_swell_wave_height") and url:find("wind_wave_period") and url:find("swell_wave_peak_period"),
        "The marine request asks for every train")
end

-- SU2: an estimated breaking height in body words.
do
    check(Waves.words(0.9, 1.2) == "waist to chest high" and Waves.words(1.6, 2.3) == "head high to overhead"
        and Waves.words(0.1, 0.2) == "flat" and Waves.word(1.0) == "waist high", "Sizes read in body words")
    local short = Waves.face({ height = 1.5, period = 6, direction = 270 }, Breaks.of(beach))
    local long = Waves.face({ height = 1.5, period = 16, direction = 270 }, Breaks.of(beach))
    check(long > short * 1.5, "A long-period swell breaks bigger than windswell of the same height")
    check(says(verdict(Surf, clean, beach), "%(estimate%)"), "The size reason says it is an estimate")
    local metrics = Surf.metrics(F.hour(clean), F.forecast)
    check(metrics[1].label == "surf (estimate)" and metrics[1].value:find("–") and metrics[1].note:find("high"),
        "The conditions open with the estimated surf as a range and in words")
end

-- SU3: a profile per break.
do
    for _, place in ipairs(Places) do
        local profile = Breaks.list[place.id]
        check(profile and profile.window and profile.wind and profile.size and Breaks.TIDE[profile.tide]
            and profile.level, place.title .. " has a break profile")
    end
    local malibu = { id = "malibu", facing = 190 }
    local south = { swellHeight = 1.2, swellPeriod = 14, swellDir = 200, windDir = 0, wind = 3 }
    local west = { swellHeight = 1.2, swellPeriod = 14, swellDir = 280, windDir = 0, wind = 3 }
    check(verdict(Surf, south, malibu).score > verdict(Surf, west, malibu).score + 20
        and says(verdict(Surf, west, malibu), "misses this break"), "Malibu wants south swell; a west swell misses it")
    check(says(verdict(Surf, south, malibu), "Offshore"), "North wind is offshore at Malibu")
    check(Breaks.outside({ 320, 60 }, 10) == 0 and Breaks.outside({ 320, 60 }, 90) == 30, "A window may wrap north")
    -- Skill: a beginner on a big day, and on a heavy break.
    local big = with { swellHeight = 1.3, swellPeriod = 14 }
    check(verdict(Surf, big, beach).verdict == "good", "Head high and clean is good for an intermediate")
    Settings.set("surf", "level", "beginner")
    check(Surf.level() == "beginner" and verdict(Surf, big, beach).score <= 40
        and says(verdict(Surf, big, beach), "too big for a beginner"), "The same day is too big for a beginner")
    -- The same small, clean swell straight into each beach.
    local atBondi = with { swellHeight = 0.6, swellPeriod = 10, swellDir = 110, windDir = 290 }
    local atOcean = with { swellHeight = 0.6, swellPeriod = 10, swellDir = 270, windDir = 90 }
    local gentle, heavy = verdict(Surf, atBondi, { id = "bondi", facing = 110 }),
        verdict(Surf, atOcean, { id = "ocean-beach", facing = 270 })
    check(gentle.verdict == "good" and heavy.score <= 55 and says(heavy, "experienced surfers"),
        "A beginner is sent to a gentle beach, and a heavy break says so")
    Settings.set("surf", "level", "advanced")
    check(verdict(Surf, with { swellHeight = 1.6, swellPeriod = 14 }, beach).verdict == "good",
        "Overhead is good for an advanced surfer")
    local row = Surf.breakRow(Breaks.of { id = "ericeira" })
    check(row.value == "needs W to NW swell" and row.note:find("low to mid tide") and row.note:find("you: advanced"),
        "The break's needs are listed: " .. row.value .. " · " .. row.note)
end

-- A forecast for the tests below: the fixture week at a spot on the Greenwich
-- meridian (so local time is UTC and dawn is near 06:40), as of `epoch`.
local spot = { id = "test-beach", title = "Test Beach", lat = 43.67, lon = 0, facing = 270, activities = { "surf" } }
local function marine(fields)
    local m = F.marine()
    for name, fn in pairs(fields or {}) do m.hourly[name] = F.series(#m.hourly.time, fn) end
    return m
end
local function forecastAt(epoch, extraWeather, extraMarine, place)
    local f = Forecast.parse(F.weather(43.67, extraWeather), marine(extraMarine), epoch, place or spot)
    f.spot = place or spot
    return f
end
-- A semi-diurnal tide: 1 m either side, highs at 03:00 and about 15:25.
local function tide(i) return math.cos((i - 3) * 2 * math.pi / 12.42) end

-- SU4: the dawn-patrol card.
do
    Settings.set("surf", "level", "advanced")
    local f = forecastAt(F.NOW)
    local card = Surf.dawnPatrol(f)
    local dawn = f.days[f.today].twilight.civil.dawn
    check(card.title == "dawn patrol this morning" and card.value == "first light " .. dawn:sub(12, 16),
        "Before first light the card is about this morning: " .. card.value)
    check(card.tone == "good" and card.detail:find("^good from 07:00") and card.detail:find("estimate"),
        "It names the first good hour: " .. card.detail)
    local evening = forecastAt(F.NOW + 12 * 3600)
    check(Surf.dawnPatrol(evening).title == "dawn patrol tomorrow", "In the evening it is about tomorrow")
    check(Surf.focus(evening) > evening.now and Surf.light(evening.hours[Surf.focus(evening)]),
        "In the dark the verdict shows the next light hour")
    local twilight = with { sunAltitude = -3, isDay = false }
    check(Surf.light(twilight) and not Surf.light(with { sunAltitude = -8, isDay = false })
        and says(verdict(Surf, twilight, beach), "First light") and verdict(Surf, twilight, beach).score > 30,
        "Civil twilight before sunrise counts as light for dawn patrol")
end

-- SU5: tides, approximate.
do
    local f = forecastAt(F.NOW, nil, { sea_level_height_msl = tide })
    local extremes = Tides.extremes(f)
    check(extremes[1].kind == "high" and extremes[1].time == "2026-11-18T03:00" and extremes[2].kind == "low"
        and extremes[2].time:find("T09:1"), "Highs and lows are found and timed between hours: " .. extremes[2].time)
    local state = Tides.at(f, f.hours[f.now])
    check(state.word == "mid" and not state.rising and math.abs(state.range - 2) < 0.05,
        "Three hours after high water the tide is mid and falling")
    check(Tides.at(f, f.hours[f.now + 3]).word == "low", "Near low water it is low")
    local card = Surf.tideCard(f)
    check(card.title == "tides (approximate)" and card.value == "low 09:12 · high 15:25"
        and card.detail:find("not for navigation") and card.detail:find("now mid, falling"),
        "The tide card gives the next low and high, approximate: " .. card.value)
    -- Ericeira works at low to mid tide.
    local ericeira = { id = "ericeira", title = "Ericeira", lat = 43.67, lon = 0, facing = 290 }
    local g = forecastAt(F.NOW, nil, { sea_level_height_msl = tide }, ericeira)
    local high, low = Report.rate(Surf, g.hours[4], g), Report.rate(Surf, g.hours[10], g)
    check(says(high, "works best at low to mid") and says(low, "when this break works best") and low.score > high.score,
        "The break's best tide counts in the rating")
    check(Surf.note(g.hours[10], g):find("low tide$"), "Each hour's line names the tide")
    local flat = forecastAt(F.NOW, nil, { sea_level_height_msl = function(i) return 0.05 * tide(i) end }, ericeira)
    check(#Tides.extremes(flat) == 0 and not Surf.tideCard(flat), "A wobble of a few centimetres is no tide")
end

-- SU6: the best break among those with a forecast.
do
    local here, there = { id = "ocean-beach", title = "Ocean Beach", facing = 270 }, { id = "bondi", title = "Bondi", facing = 110 }
    local good = forecastAt(F.NOW, nil, nil, here)
    local away = forecastAt(F.NOW, nil, { swell_wave_direction = function() return 300 end }, there)
    local ranked = Surf.compare { { place = there, forecast = away }, { place = here, forecast = good } }
    check(#ranked == 2 and ranked[1].place == here and ranked[1].result.score > ranked[2].result.score,
        "The break with the swell in its window wins")
end

-- SU7: the 16-day outlook with a confidence word.
do
    check(Surf.fields.forecastDays == 16 and Surf.sources[1].id == "surf-outlook", "Surf asks for 16 days")
    local url = Surf.sources[1].url({ lat = 37.76, lon = -122.52, facing = 270 }, Forecast)
    check(url:find("models=ncep_gfswave025") and url:find("cell_selection=sea") and url:find("forecast_days=16")
        and tonumber(url:match("longitude=([%-%d%.]+)")) < -122.7, "The outlook asks GFS Wave out at sea: " .. url)
    local f = forecastAt(F.NOW)
    for _, hour in ipairs(f.hours) do
        if hour.date >= "2026-11-22" then
            hour.swellHeight, hour.swellPeriod, hour.swellDir, hour.waveHeight = nil, nil, nil, nil
        end
    end
    local gfs = { hourly = { time = F.series(#f.hours, F.iso),
        wave_height = F.series(#f.hours, function(i) return i < 24 and 3.6 or 1.9 end),
        swell_wave_height = F.series(#f.hours, function() return 1.6 end),
        swell_wave_period = F.series(#f.hours, function() return 13 end),
        swell_wave_direction = F.series(#f.hours, function() return 280 end) } }
    Outlook.merge(gfs, f)
    local far = f.hours[5 * 24 + 10]
    check(far.oneModel and far.swellHeight == 1.6 and far.waveHeight == 1.9, "GFS Wave fills the days the default model lacks")
    check(Report.rate(Surf, far, f).verdict ~= "poor" and says(Report.rate(Surf, far, f), "one wave model"),
        "Far hours are rated and say they rest on one model")
    local c0, why0 = Outlook.confidence(f, "2026-11-18")
    local c1, why1 = Outlook.confidence(f, "2026-11-19")
    local c5, why5 = Outlook.confidence(f, "2026-11-23")
    check(c0 == "medium" and why0:find("disagree"), "Two models far apart lower today's confidence: " .. why0)
    check(c1 == "high" and why1 == "1 day out", "Tomorrow with agreeing models is high confidence: " .. why1)
    check(c5 == "low" and why5:find("one wave model"), "One model only is low confidence: " .. why5)
    local card = Surf.outlookCard(f)
    check(card.title == "best day ahead" and card.detail:find("confidence"), "The outlook card says how sure it is")
    local land = forecastAt(F.NOW)
    land.missing = {}
    Outlook.merge({ hourly = { time = { F.iso(0) }, wave_height = { 0 } } }, land)
    check(land.missing["surf-outlook"] and not land.hours[1].gfsWave, "A land cell in the wave model is ignored")
end

-- SU8: water quality after heavy rain.
do
    local later = F.NOW + 3 * 86400
    local rain = function(i) return i >= 24 and i < 48 and 1 or 0 end -- 24 mm on Thursday
    local wet = forecastAt(later, { precipitation = rain }, nil, { id = "malibu", lat = 43.67, lon = 0, facing = 190 })
    check(math.abs(Surf.rainBefore(wet, wet.hours[wet.now]) - 24) < 0.01, "Rain of the last 72 h is added up")
    local card = Surf.waterCard(wet)
    check(card and card.tone == "poor" and card.value == "runoff risk" and card.detail:find("24 mm"),
        "Heavy rain near drains warns about the water")
    check(says(Report.rate(Surf, wet.hours[wet.now], wet), "runoff"), "and lowers the rating")
    check(not Surf.waterCard(forecastAt(later, nil, nil, { id = "malibu", lat = 43.67, lon = 0, facing = 190 })),
        "Dry days give no warning")
    check(not Surf.waterCard(forecastAt(later, { precipitation = rain }, nil,
        { id = "tarifa", lat = 43.67, lon = 0, facing = 210 })), "A break without drains or a river mouth gives none")
    check(Surf.fields.pastDays >= 3, "Surf asks for three past days")
end

-- SU10: a session log with a one-tap rating, on this device only.
do
    local Storage = require "Weather/storage"
    local Logbook = require "Weather/editions/surf/logbook"
    local store = F.storage("surf-log")
    store:remove(Logbook.NAME)
    Storage.use(store)
    local f = forecastAt(F.NOW)
    check(not Surf.log(spot, f, f.hours[f.now + 1], "epic"), "A rating is good, fair or poor")
    local entry = Surf.log(spot, f, f.hours[f.now + 1], "good")
    Surf.log(spot, f, f.hours[f.now + 2], "good")
    check(entry and entry.face and entry.wind == "offshore" and entry.place == spot.id, "A session keeps its conditions")
    check(#store:load(Logbook.NAME).sessions == 2, "The log is saved at once")
    check(says(Report.rate(Surf, f.hours[f.now + 3], f), "2 sessions like this in your log, all good"),
        "Like conditions recall the log")
    local elsewhere = forecastAt(F.NOW, nil, nil, { id = "elsewhere", lat = 43.67, lon = 0, facing = 270 })
    check(not says(Report.rate(Surf, elsewhere.hours[elsewhere.now + 3], elsewhere), "your log"),
        "Another break has its own log")
    check(Logbook.summary { { rating = "good" }, { rating = "poor" }, { rating = "poor" } }
        == "3 sessions like this in your log, mostly poor", "The summary counts the ratings")
end

-- SU11: the wind drawn against the beach.
do
    local west = Breaks.of(beach)
    check(Surf.windIcon({ windDir = 90 }, west):find("/0%.svg$") and Surf.windIcon({ windDir = 270 }, west):find("/180%.svg$")
        and Surf.windIcon({ windDir = 0 }, west):find("/270%.svg$"), "Offshore points out to sea, onshore at the shore")
    for turn = 0, 315, 45 do
        local file = io.open("apps/Weather/editions/surf/wind/" .. turn .. ".svg", "r")
        check(file, "There is a drawing for " .. turn .. "°")
        if file then file:close() end
    end
    check(Surf.dawnPatrol(forecastAt(F.NOW)).icon:find("wind/0%.svg$"), "The dawn card draws the wind on the beach")
end

-- Existing services (Research/services-surf.md). Synthetic answers in each
-- service's documented shape; the network to these hosts is not reached.
local Hazards = require "Weather/editions/surf/hazards"
local Buoys = require "Weather/editions/surf/buoys"
local Comfort = require "Weather/editions/surf/comfort"
local WaterQuality = require "Weather/editions/surf/waterquality"
local usBeach = { id = "ocean-beach", title = "Ocean Beach", lat = 37.76, lon = -122.52, facing = 270, activities = { "surf" } }
local function sourceIds()
    local ids = {}
    for _, source in ipairs(Surf.sources) do ids[source.id] = source end
    return ids
end
local function loadAt(place, answers)
    local net = F.network()
    for pattern, answer in pairs(answers or {}) do net.answers[pattern] = answer end
    local f = Forecast.load(place, true, net.fetch, F.NOW, Activities.request(place))
    f.spot = place
    return f, net
end
local function cardOf(f, id)
    for _, card in ipairs(Surf.cards(f, f.spot)) do if card.id == id then return card end end
end
local function iso(epoch, offset) return os.date("!%Y-%m-%dT%H:%M:%S", epoch) .. (offset or "Z") end

-- api.weather.gov alerts/active?point= (GeoJSON), trimmed to what is read.
local function alerts(features) return function() return { type = "FeatureCollection", features = features } end end
local RIP = { properties = { event = "Rip Current Statement", status = "Actual", severity = "Moderate",
    senderName = "NWS San Francisco CA", onset = "2026-11-17T21:00:00-08:00", ends = "2026-11-19T03:00:00-08:00" } }
local CRAFT = { properties = { event = "Small Craft Advisory", status = "Actual", severity = "Minor",
    senderName = "NWS San Francisco CA", ends = "2026-11-19T03:00:00-08:00" } }

-- SV-SU1: beach warnings from the National Weather Service.
do
    Settings.set("surf", "level", "intermediate")
    local ids = sourceIds()
    check(ids["surf-hazards"] and ids["surf-hazards"].optional, "Beach warnings are an optional source")
    check(Hazards.source.url(usBeach):find("^https://api%.weather%.gov/alerts/active%?point=37%.7600,%-122%.5200")
        and not Hazards.source.url({ lat = -33.89, lon = 151.28 }), "Warnings are asked for US beaches only")
    local f, net = loadAt(usBeach, { ["api%.weather%.gov"] = alerts { RIP, CRAFT } })
    local asked
    for index, url in ipairs(net.requests) do
        if url:find("api%.weather%.gov") then asked = net.options[index].headers end
    end
    check(asked and asked.Accept == "application/geo+json" and asked["User-Agent"], "The request names the app")
    check(#f.beachHazards == 1 and f.beachHazards[1].event == "Rip Current Statement",
        "Only the beach's warnings are kept: a small craft advisory is not")
    local card = cardOf(f, "hazard")
    check(card and card.value == "rip current statement" and card.tone == "poor" and card.rank == 0
        and card.detail:find("until thu 19 11:00") and card.detail:find("lifeguard")
        and card.detail:find("NWS San Francisco"), "A rip current statement leads, with its end: "
        .. tostring(card and card.detail))
    check(Surf.cards(f, usBeach)[1].id == "hazard", "The warning comes first among the cards")
    local during, after = f.hours[f.now + 2], f.hours[f.now + 30]
    check(Report.rate(Surf, during, f).score <= 60 and says(Report.rate(Surf, during, f), "Rip Current Statement in force")
        and not says(Report.rate(Surf, after, f), "in force"), "While it is in force it caps the hour")
    Settings.set("surf", "level", "beginner")
    check(Report.rate(Surf, during, f).score <= 40 and says(Report.rate(Surf, during, f), "surf near a lifeguard"),
        "A beginner is told to surf near a lifeguard")
    Settings.set("surf", "level", "intermediate")
    local quiet = loadAt(usBeach, { ["api%.weather%.gov"] = alerts {} })
    check(#quiet.beachHazards == 0 and not cardOf(quiet, "hazard") and not quiet.missing["surf-hazards"],
        "No beach warning in force: no card")
    local broken = loadAt(usBeach, { ["api%.weather%.gov"] = function() return { title = "Unexpected Problem" } end })
    check(broken.missing["surf-hazards"] and not broken.beachHazards and not cardOf(broken, "hazard"),
        "An answer without an alert list fails the source")
    for _, c in ipairs(Surf.cards(broken, usBeach)) do
        check(not (c.value .. " " .. (c.detail or "")):lower():find("no warning")
            and not (c.value .. " " .. (c.detail or "")):lower():find("safe"),
            "A failed warning service never reads as safe: " .. c.id)
    end
    local high = { properties = { event = "High Surf Warning", status = "Actual", ends = "2026-11-19T03:00:00-08:00" } }
    local worst = Hazards.parse { features = { RIP, high } }
    check(worst[1].event == "High Surf Warning", "The worst warning comes first")
end

-- CoastWatch ERDDAP cwwcNDBCMet.json: a table of columns and rows.
local function erddap(rows)
    return function()
        local list = {}
        for _, row in ipairs(rows) do
            list[#list + 1] = { "46026", iso(F.NOW - row[1] * 60), row[2], row[3], row[4], row[5], row[6] }
        end
        return { table = { columnNames = { "station", "time", "wvht", "dpd", "apd", "mwd", "wtmp" },
            columnTypes = { "String", "String", "float", "float", "float", "short", "float" },
            columnUnits = { nil, "UTC", "m", "s", "s", "degrees_true", "degree_C" }, rows = list } }
    end
end

-- SV-SU2: the live buoy, against the forecast.
do
    check(Breaks.of(usBeach).buoy == "46026", "Ocean Beach reads buoy 46026")
    local url = Buoys.source.url(usBeach)
    check(url:find("^https://coastwatch%.pfeg%.noaa%.gov/erddap/tabledap/cwwcNDBCMet%.json%?")
        and url:find("station=%%2246026%%22") and url:find("time%%3E=now%-6hours")
        and not Buoys.source.url({ id = "tarifa" }), "The buoy is asked for six hours, only where a break has one")
    -- Minutes before now: 23 min ago a long-period swell, 3 h before that a short one.
    local f = loadAt(usBeach, { coastwatch = erddap {
        { 310, 1.6, 9, 7.5, 290, 12.4 }, { 190, 1.7, 9, 7.6, 290, 12.4 }, { 23, 2.1, 14, 9.1, 292, 12.3 } } })
    local card = cardOf(f, "buoy")
    check(card and card.title == "buoy 46026" and card.value == "2.1 m · 14 s" and card.tone == "info",
        "The buoy card reads the last wave: " .. tostring(card and card.value))
    check(card.detail:find("^23 min ago, from W") and card.detail:find("0%.3 m bigger than forecast")
        and card.detail:find("swell arriving: the period rose from 9 s to 14 s")
        and card.detail:find("National Data Buoy Center"), "It compares with the forecast: " .. card.detail)
    local same = loadAt(usBeach, { coastwatch = erddap { { 15, 1.85, 11, 8, 280, 12 } } })
    check(cardOf(same, "buoy").detail:find("as forecast") and not cardOf(same, "buoy").detail:find("arriving"),
        "Close to the forecast it says so")
    local stale = loadAt(usBeach, { coastwatch = erddap { { 310, 1.6, 9, 7.5, 290, 12.4 } } })
    card = cardOf(stale, "buoy")
    check(card.value == "no recent reading" and card.detail:find("5 h ago") and card.hub == false,
        "A reading five hours old is not now")
    local silent = loadAt(usBeach, { coastwatch = erddap { { 30, nil, nil, nil, nil, 12.2 }, { 90, "NaN", nil, nil, nil, 12 } } })
    check(cardOf(silent, "buoy").value == "no reading" and cardOf(silent, "buoy").detail:find("may be down"),
        "A buoy without wave readings says it may be down")
    local down = loadAt(usBeach)
    check(down.missing["surf-buoy"] and cardOf(down, "buoy").value == "no reading"
        and cardOf(down, "buoy").detail:find("not answering"), "A feed that fails says so, on the page")
    check(not cardOf(loadAt({ id = "tarifa", title = "Tarifa", lat = 36.01, lon = -5.61, facing = 210,
        activities = { "surf" } }), "buoy"), "A break without a buoy shows none")
end

-- SV-SU3: the water temperature measured at a NOAA station.
do
    local url = Buoys.waterSource.url(usBeach, Forecast)
    check(url:find("^https://api%.tidesandcurrents%.noaa%.gov/api/prod/datagetter%?product=water_temperature")
        and url:find("station=9414290") and url:find("date=latest") and url:find("format=json"),
        "Ocean Beach asks CO-OPS station 9414290: " .. url)
    local function waterRow(f, ahead)
        for _, row in ipairs(Surf.metrics(f.hours[f.now + (ahead or 0)], f)) do
            if row.label == "water" then return row end
        end
    end
    local f = loadAt(usBeach, { ["datagetter.*water_temperature"] = function()
        return { metadata = { id = "9414290", name = "San Francisco", lat = "37.8063", lon = "-122.4659" },
            data = { { t = "2026-11-18 05:54", v = "10.8", f = "0,0,0" } } }
    end })
    local row = waterRow(f)
    check(f.waterTemp and f.waterTemp.value == 10.8 and row.value == "11°" and row.note:find("^5/4 mm")
        and row.note:find("measured at San Francisco"), "The measured water picks the wetsuit: " .. row.note)
    check(waterRow(f, 24).value == "14°" and waterRow(f, 24).note:find("model"),
        "Tomorrow's hours keep the model's sea, labelled model")
    local failed = loadAt(usBeach, { ["datagetter.*water_temperature"] = function()
        return { error = { message = "No data was found. This product may not be offered at this station." } }
    end })
    row = waterRow(failed)
    check(failed.missing["surf-water"] and row.value == "14°" and row.note == "4/3 mm wetsuit · model",
        "Without a measurement the model's sea stands, labelled model")
    local buoyWater = loadAt(usBeach, { coastwatch = erddap { { 20, 1.8, 11, 8, 280, 12.6 } } })
    check(waterRow(buoyWater).value == "13°" and waterRow(buoyWater).note:find("measured at buoy 46026"),
        "The buoy's water temperature stands in for a station")
end

-- SV-SU4: official bathing-water status (Ireland's EPA), before the rain rule.
do
    local code = "IESWBWC090_0000_0300"
    Breaks.list["test-ie"] = { window = { 200, 320 }, wind = 90, size = { 0.5, 2.5 }, tide = "all",
        level = "intermediate", exposure = 1, runoff = 10, bathingWater = { ie = code } }
    local irish = { id = "test-ie", title = "Test Strand", lat = 43.67, lon = 0, facing = 270 }
    check(WaterQuality.ireland.url(irish) == "https://data.epa.ie/bw/api/v1/alerts"
        and not WaterQuality.ireland.url(usBeach), "Ireland's list is asked only for a break with an Irish ID")
    local f = forecastAt(F.NOW, nil, nil, irish)
    WaterQuality.ireland.parse({ { bathingWaterCode = "IENWBWC160_0000_0100", alertType = "Prohibition" },
        { bathingWaterCode = code, alertType = "Advice Against Bathing", reason = "Sewage overflow",
            endDate = "2026-11-20T00:00:00" } }, f)
    local card = Surf.waterCard(f)
    check(card.value == "advice against bathing" and card.tone == "poor" and card.detail:find("sewage overflow")
        and card.detail:find("until 2026%-11%-20") and card.detail:find("EPA Ireland"),
        "An EPA alert for the break leads the water card: " .. card.detail)
    check(says(Report.rate(Surf, f.hours[f.now + 2], f), "Official advice against bathing"),
        "and lowers today's rating")
    local clear = forecastAt(F.NOW, nil, nil, irish)
    WaterQuality.ireland.parse({ alerts = { { bathingWaterCode = "IENWBWC160_0000_0100", alertType = "Prohibition" } } },
        clear)
    card = Surf.waterCard(clear)
    check(card.value == "no restriction listed" and card.tone == "info" and card.hub == false
        and not card.detail:find("safe"), "No alert for the break: none listed, never safe")
    local later = F.NOW + 3 * 86400
    local wet = forecastAt(later, { precipitation = function(i) return i >= 24 and i < 48 and 1 or 0 end }, nil, irish)
    WaterQuality.ireland.parse({}, wet)
    card = Surf.waterCard(wet)
    check(card.value == "runoff risk" and card.detail:find("no official restriction listed yet"),
        "Heavy rain still warns when the list is quiet")
    local ok = pcall(WaterQuality.ireland.parse, "garbled", forecastAt(F.NOW, nil, nil, irish))
    check(not ok, "An answer without a list fails the source")
    local about = Surf.attribution()
    check(about[#about][1]:find("EPA Ireland"), "A break that reads the EPA credits it")
    Breaks.list["test-ie"] = nil
    for _, line in ipairs(Surf.attribution()) do
        check(not line[1]:find("EPA"), "No Irish break, no Irish credit")
    end
    local malibu = forecastAt(later, { precipitation = function(i) return i >= 24 and i < 48 and 1 or 0 end }, nil,
        { id = "malibu", lat = 43.67, lon = 0, facing = 190 })
    check(Surf.waterCard(malibu).detail:find("no official reading here"), "Elsewhere the rain rule says it is not official")
end

-- SV-SU5: the sun and the current, on the surf page.
do
    local f = forecastAt(F.NOW, { uv_index = function(i) return i % 24 == 13 and 8 or 1 end })
    check(Comfort.source.url(spot, Forecast):find("ocean_current_velocity,ocean_current_direction"),
        "The current is asked from the marine model")
    Comfort.source.parse({ hourly_units = { ocean_current_velocity = "km/h", ocean_current_direction = "°" },
        hourly = { time = F.series(#f.hours, F.iso), ocean_current_velocity = F.series(#f.hours, function() return 3.24 end),
            ocean_current_direction = F.series(#f.hours, function() return 0 end) } }, f)
    check(math.abs(f.hours[f.now].current - 0.9) < 0.001, "km/h is read as m/s")
    local card = Comfort.card(f, f.hours[f.now + 2])
    check(card and card.id == "comfort" and card.hub == false and card.value == "strong current" and card.tone == "fair"
        and card.detail:find("strong current along the beach") and card.detail:find("not rips"),
        "A 0.9 m/s current along the beach is strong: " .. tostring(card and card.detail))
    check(card.detail:find("UV 8 around 13:00: sunscreen"), "UV 8 at noon asks for sunscreen")
    local calm = forecastAt(F.NOW)
    check(not Comfort.card(calm), "Low UV and no current data: no card")
end

-- SV-SU6: pages about the break, named and never copied.
do
    local card = Surf.linksCard(forecastAt(F.NOW, nil, nil, { id = "bondi", lat = 43.67, lon = 0, facing = 110 }))
    check(card and card.hub == false and card.detail:find("Beachwatch NSW: beachwatch%.nsw%.gov%.au")
        and card.detail:find("SharkSmart NSW"), "Bondi names Beachwatch and SharkSmart")
    check(not Surf.linksCard(forecastAt(F.NOW, nil, nil, { id = "tarifa", lat = 43.67, lon = 0, facing = 210 })),
        "A break without pages shows none")
end

-- Credit for every service on the settings' "about".
do
    local Provider = require "Weather/provider"
    local titles = {}
    for _, line in ipairs(Provider.about()) do titles[#titles + 1] = line[1] end
    local all = table.concat(titles, "|")
    check(all:find("Warnings: U.S. National Weather Service", 1, true) and all:find("NOAA National Data Buoy Center", 1, true)
        and all:find("Water temperature: NOAA CO-OPS", 1, true) and titles[1]:find("Open%-Meteo"),
        "About credits the NWS, NDBC and CO-OPS after Open-Meteo: " .. all)
end

-- On the hub: a warning in force leads.
do
    local first = Places[1]
    check(first.id == "ocean-beach", "The edition opens on Ocean Beach")
    local net = F.network()
    net.answers["api%.weather%.gov"] = alerts { RIP }
    local ui = F.launch { places = Places, net = net, storage = F.storage("surf-hazard", Places) }
    local cards = ui.cards()
    check(cards[1] == "Card" .. first.id .. ":card:hazard" and #cards == 3,
        "The hub leads with the rip current statement: " .. table.concat(cards, " "))
    check(ui.text("Card" .. first.id .. ":card:hazard", "CardValue") == "rip current statement", "and names it")
    ui.app.hubScope:dispose()
end

-- The edition on its own places, with the cards on the hub and the page.
F.checkEdition("surf")
do
    local ui = F.launch { places = Places, storageName = "surf-panorama" }
    local screen, panorama, section = ui.app.screen, ui.find("HubPanorama"), ui.find("NowSection")
    local photo = F.checkHubPhoto(ui)

    local function pointer(message, x)
        F.orca.system.dispatchMessage { target = screen, message = message, x = x, y = 420 }
        screen:UpdateLayout(390, 844)
    end
    local start, crop = section.ActualX, photo.Viewbox
    pointer("LeftButtonDown", 240)
    pointer("LeftButtonDragged", 150)
    local panelTravel = start - section.ActualX
    local photoTravel = (photo.Viewbox.X - crop.X) * photo.ActualWidth / crop.Z
    check(panelTravel > 0 and math.abs(photoTravel - panelTravel / 8) < 0.5,
        "The photo follows the finger at one-eighth of the panel speed")
    local draggedCrop = photo.Viewbox.X
    pointer("LeftButtonUp", 150)
    check(panorama.SelectedIndex == 1 and math.abs(photo.Viewbox.X - draggedCrop) < 0.0001,
        "A swipe changes sections and begins settling from the current crop")
    ui.app.hubScope:dispose()
end
do
    local store = F.storage("surf-cards", Places)
    local net = F.network()
    net.answers["marine%-api.*latitude=%-33"] = function(url)
        return marine { swell_wave_direction = function() return 100 end, wave_direction = function() return 100 end }
    end
    local bondi
    for _, place in ipairs(Places) do if place.id == "bondi" then bondi = place end end
    local _, _, raw = Forecast.load(bondi, "optional", net.fetch, F.NOW, Activities.request(bondi))
    store:save("forecast-bondi", raw)
    local ui = F.launch { places = Places, net = net, storage = store }
    local id = Places[1].id
    -- I2: at most three cards on the hub, the first-run setup first (`rank`).
    local cards = ui.cards()
    check(#cards == 3 and cards[1] == "Card" .. id .. ":card:setup", "A first run asks for the surfer's level first: "
        .. table.concat(cards, " "))
    check(ui.has("Card" .. id .. ":card:dawn") and ui.text("Card" .. id .. ":card:dawn", "CardValue"):find("^first light"),
        "The hub shows dawn patrol")
    local outlook
    for _, url in ipairs(net.requests) do outlook = outlook or url:find("ncep_gfswave025") end
    check(outlook, "The outlook is fetched with the forecast")
    -- I3: the setup card's links choose the level in one tap.
    local setup = ui.find("Card" .. id .. ":card:setup")
    check(setup:findChild("CardAction1", true).Text == "beginner" and setup:findChild("CardAction3", true).Visible,
        "The setup card offers the three levels")
    setup:findChild("CardAction1", true):send("Node.LeftButtonUp")
    ui.settle()
    check(Surf.level() == "beginner" and select(2, Surf.level()), "A tap chooses beginner")
    check(not ui.has("Card" .. id .. ":card:setup") and not ui.has("Card" .. id .. ":card:log"),
        "Once chosen, the setup card goes away; the session log stays off the hub")
    ui.press("OpenSettings")
    check(ui.text("Settingsettings:surf:level", "SettingValue") == "beginner", "Settings show the level chosen")
    ui.back("SettingsPivot")
    ui.open(id, "surf")
    check(ui.text("Card" .. id .. ":surf:card:dawn", "CardValue"):find("^first light")
        and ui.text("Card" .. id .. ":surf:card:best-break", "CardValue") == "bondi" and ui.has("Card" .. id .. ":surf:card:outlook"),
        "The activity page shows every card; the best break today is another saved break")
    -- I7: the week says how sure each day is; a third chart draws the tide.
    check(ui.text("Day" .. id .. ":surf:day:2026-11-18", "Why"):find("^high confidence · best"),
        "Days carry the outlook's confidence: " .. ui.text("Day" .. id .. ":surf:day:2026-11-18", "Why"))
    check(ui.find("ChartC").Visible and ui.find("ChartCLine").Title == "tide (approximate) m", "A third chart draws the tide")
    -- I3: one tap logs the session in the current hour's conditions.
    local Logbook = require "Weather/editions/surf/logbook"
    local before = #Logbook.sessions()
    local log = "Card" .. id .. ":surf:card:log"
    check(ui.text(log, "CardValue") == "how was it?" and ui.find(log):findChild("CardAction2", true).Text == "fair",
        "The page asks how the session was")
    ui.find(log):findChild("CardAction1", true):send("Node.LeftButtonUp")
    ui.settle()
    check(#Logbook.sessions() == before + 1 and Logbook.sessions()[before + 1].rating == "good"
        and Logbook.sessions()[before + 1].place == id, "One tap logs a good session here")
    check(ui.text(log, "CardValue") == "saved" and ui.text(log, "CardDetail"):find("^saved a good session at"),
        "The card says what it saved: " .. ui.text(log, "CardDetail"))
    ui.back()
    ui.app.hubScope:dispose()
end
-- Wave 3: a beach the surfer found, saved for surf (the edition's activity),
-- its shore set in the editor, flowing through the surf rules; then the
-- comparison of my breaks (Weather/compare.lua) sees it.
do
    local MyPlaces = require "Weather/myplaces"
    local Compare = require "Weather/compare"
    local net = F.network()
    -- Recorded from Open-Meteo's geocoding (November 2026), trimmed.
    net.answers["geocoding%-api"] = function()
        return { results = { { id = 3018384, name = "Plage des Estagnots", latitude = 43.69, longitude = -1.44,
            elevation = 4, feature_code = "BCH", country_code = "FR", timezone = "Europe/Paris", country = "France",
            admin1 = "New Aquitaine" } } }
    end
    local store = F.storage("surf-places", Places)
    store:remove("forecast-geo-3018384")
    local ui = F.launch { places = Places, net = net, storage = store }
    local function child(row, name) return ui.find(row):findChild(name, true) end
    local function tap(row, name, times) for _ = 1, times or 1 do child(row, name):send("Node.LeftButtonUp"); ui.settle() end end
    ui.press("AddPlace")
    ui.find("SearchInput").Text = "estagnots"
    ui.press("RunSearch")
    check(child("Resultgeo-3018384", "ResultLine").Text == "beach · New Aquitaine, France", "A beach is found")
    ui.press("Resultgeo-3018384")
    check(ui.find("PlacePage").StateClasses:find("surf") and not ui.has("Fieldplace:field:for:surf")
        and child("Fieldplace:field:kind", "SettingValue").Text == "a coast or a beach"
        and child("Fieldplace:field:facing", "SettingValue").Text == "not set"
        and child("Fieldplace:field:facing", "SettingDetail").Text:find("^not set: tap"),
        "In Surfers' Weather a place is added for surf, its shore to confirm")
    check(child("Fieldplace:field:facing", "SettingHeading").Text == "surf"
        and child("Fieldplace:field:breakLevel", "SettingValue").Text == "intermediate surfers",
        "Surf's own fields: the shore, who the break suits, the tide")
    tap("Fieldplace:field:facing", "SettingLess", 2)
    check(child("Fieldplace:field:facing", "SettingValue").Text == "W (270°)", "The shore turns to face west")
    tap("Fieldplace:field:breakLevel", "SettingChange")
    ui.press("Actionplace:action:save")
    local place = MyPlaces.find("geo-3018384")
    check(ui.find("HubPanorama").Title == "Plage des Estagnots" and ui.has("GoodForgeo-3018384:activity:surf")
        and table.concat(place.activities, ",") == "surf", "Saved, it is good for surf")
    local profile = Breaks.of(place)
    check(profile.window[1] == 210 and profile.window[2] == 330 and profile.wind == 90 and profile.level == "advanced",
        "The break's profile comes from what the user set")
    check(table.concat(net.requests, "\n"):find("marine%-api[^\n]*latitude=43.69"), "A coast asks for the sea")
    ui.open("geo-3018384", "surf")
    check(ui.text("Hourgeo-3018384:surf:hour:2026-11-18T08:00", "Why"):find("offshore"),
        "The east wind is offshore on a west-facing beach")
    ui.back()
    check(#Compare.places("surf") == 1 and Compare.places("surf")[1] == place, "My breaks are the saved ones")
    child("Placebondi", "PlaceLink"):send("Node.LeftButtonUp")
    ui.settle()
    local mine = Compare.places("surf")
    check(#mine == 2 and mine[2].id == "bondi", "A kept suggestion joins them")
    local own = MyPlaces.find("bondi")
    check(Breaks.of(own) == Breaks.list.bondi, "A break the user has not changed keeps its table")
    MyPlaces.set("bondi", "breakTide", "low")
    check(Breaks.of(MyPlaces.find("bondi")).tide == "low" and Breaks.of(MyPlaces.find("bondi")).window[1] == 40,
        "The user's tide comes first; the table keeps the rest")
    check(MyPlaces.fromResult({ id = 3032797, name = "Biarritz", latitude = 43.48, longitude = -1.56, feature_code = "PPL" })
        .own.kind == "coast", "In Surfers' Weather a town found is guessed a coast")
    ui.app.hubScope:dispose()
end
F.done("weather-surf")
