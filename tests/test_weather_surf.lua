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

-- The edition on its own places, with the cards on the hub and the page.
F.checkEdition("surf")
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
    check(ui.text("Card" .. id .. ":card:dawn", "CardValue"):find("^first light"), "The hub opens on dawn patrol")
    check(ui.has("Card" .. id .. ":card:setup"), "A first run asks for the surfer's level")
    check(ui.text("Card" .. id .. ":card:best-break", "CardValue") == "bondi",
        "The best break today is another saved break: " .. ui.text("Card" .. id .. ":card:best-break", "CardDetail"))
    check(ui.text("Card" .. id .. ":card:outlook", "CardTitle") ~= "", "The outlook card is on the hub")
    local outlook
    for _, url in ipairs(net.requests) do outlook = outlook or url:find("ncep_gfswave025") end
    check(outlook, "The outlook is fetched with the forecast")
    ui.press("OpenSettings")
    local row = "Settingsettings:surf:level"
    check(ui.text(row, "SettingValue") == "not set" and ui.text(row, "SettingTitle") == "your level",
        "Settings show the level is not set")
    ui.find(row):findChild("SettingChange", true):send("Node.LeftButtonUp")
    ui.settle()
    check(Surf.level() == "beginner" and select(2, Surf.level()), "A tap chooses beginner")
    ui.back("SettingsPivot")
    check(not ui.has("Card" .. id .. ":card:setup"), "Once chosen, the setup card goes away")
    ui.open(id, "surf")
    check(ui.text("Card" .. id .. ":surf:card:dawn", "CardValue"):find("^first light"), "The activity page shows the cards")
    ui.back()
    ui.app.hubScope:dispose()
end
F.done("weather-surf")
