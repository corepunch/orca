-- Gardeners' Weather: good for garden work, and what does the garden need?
-- The garden activity's rules on hand-built hours (frost, water, soil,
-- jobs, blight, warnings, frost dates), then the garden edition running on
-- its own places (tests/weather_fixture.lua). This file belongs to the
-- garden edition.
local F = dofile("tests/weather_fixture.lua").init { edition = "garden" }
local check, NOW, hour, verdict = F.check, F.NOW, F.hour, F.verdict
local Activities = require "Weather/activities"
local Garden = require "Weather/activities/garden"
local Settings = require "Weather/settings"
local Storage = require "Weather/storage"
local Units = require "Weather/units"
local Forecast = require "Weather/forecast"
local Report = require "Weather/report"
local Plot = require "Weather/editions/garden/plot"
local Frost = require "Weather/editions/garden/frost"
local Water = require "Weather/editions/garden/water"
local Soil = require "Weather/editions/garden/soil"
local Jobs = require "Weather/editions/garden/jobs"
local Protect = require "Weather/editions/garden/protect"
local Climate = require "Weather/editions/garden/climate"

local LEEDS = { id = "leeds", title = "Leeds", lat = 53.8, lon = -1.55, activities = { "garden" } }
local store = F.storage("garden-rules", { LEEDS }, Climate.key)
Storage.use(store)
Settings.init { edition = "garden", region = "NL" }

-- A hand-built forecast around NOW (06:00 UTC, offset 0): `past` hours
-- before now, `hours` from now; `each(h, k, hourOfDay)` sets conditions
-- (k counts hours from now). Defaults: a mild, cloudy, breezy, dry spell.
local function build(spec)
    local hours = {}
    local past = spec.past or 0
    for k = -past, (spec.hours or 72) - 1 do
        local epoch = NOW + k * 3600
        local time = os.date("!%Y-%m-%dT%H:00", epoch)
        local hod = tonumber(time:sub(12, 13))
        local h = { epoch = epoch, time = time, date = time:sub(1, 10), temp = 10, dewPoint = 5, cloud = 80, wind = 3,
            gust = 5, rain = 0, rainChance = 10, humidity = 70, et0 = 0.05, code = 3, isDay = hod >= 8 and hod <= 16 }
        if spec.each then spec.each(h, k, hod) end
        hours[#hours + 1] = h
    end
    return { hours = hours, now = past + 1, epoch = NOW, spot = spec.spot or LEEDS }
end
local function night(hod) return hod >= 18 or hod <= 9 end
local function set(key, value) check(Settings.set("garden", key, value), "Set " .. key) end
local function default() for _, s in ipairs(Plot.SETTINGS) do Settings.set("garden", s.key, s.default) end end

-- The hour's rules, as before.
check(verdict(Garden, { time = "2026-11-18T11:00", temp = 16, wind = 2 }).verdict == "good",
    "A mild, calm, dry hour is good for the garden")
check(verdict(Garden, { temp = -2, soilTemp = -1 }).score < 45, "Frozen ground is poor")
check(verdict(Garden, { code = 63 }).score < verdict(Garden).score, "Rain lowers the score")
local frosty = Garden.rate(hour { epoch = NOW }, { epoch = NOW, hours = { { epoch = NOW + 3600 * 5, temp = -1 } } }):result()
check(F.says(frosty, "Frost likely"), "Frost in the next hours is a reason")

-- GA1 frost tonight: ground frost on clear, calm nights.
local clearCalm = { cloud = 5, wind = 0.5, dewPoint = -6 }
local function frostAt(temp, fields)
    local h = { temp = temp }
    for k, v in pairs(fields or {}) do h[k] = v end
    return Frost.hour(h, Plot.current()).level
end
check(frostAt(3.5, clearCalm) == 1 and frostAt(1.8, clearCalm) == 2, "Clear and calm: possible from 4°, likely from 2°")
check(frostAt(4.5, clearCalm) == 0, "Clear and calm at 4.5° is no frost")
check(frostAt(3.5, { cloud = 95, wind = 6 }) == 0 and frostAt(1.5, { cloud = 95, wind = 6 }) == 1
    and frostAt(-0.5, { cloud = 95, wind = 6 }) == 2, "Cloudy and windy: only air frost counts")
check(frostAt(3.5, { cloud = 5, wind = 0.5, dewPoint = 3.3 }) == 0, "Humid air (dew forming) slows the cooling")
check(frostAt(6, { cloud = 95, wind = 6, soilTemp = -0.5 }) == 1, "The model's own cold surface counts")
local function evening(temp, fields)
    return build { hours = 60, each = function(h, k, hod)
        if night(hod) and k > 6 then
            h.temp = temp
            for key, v in pairs(fields or clearCalm) do h[key] = v end
        end
    end }
end
local card = Frost.card(evening(1.5), Plot.current())
check(card.title == "frost tonight" and card.value == "likely" and card.tone == "poor", "A clear 1.5° night: frost likely")
check(card.detail:find("clear sky, still air, dry air", 1, true) and card.detail:find("fleece", 1, true)
    and card.detail:find("18–09h", 1, true), "It says why, how long and what to do: " .. card.detail)
check(Frost.card(evening(3.5), Plot.current()).value == "possible", "A clear 3.5° night: frost possible")
check(Frost.card(evening(-3), Plot.current()).detail:find("hard frost", 1, true), "Severity: hard frost below −2°")
check(Frost.card(evening(6), Plot.current()).value == "no frost", "A mild night: no frost")
-- Lead time: the night after next is the cold one.
local later = Frost.card(build { hours = 80, each = function(h, k, hod)
    if k >= 60 and night(hod) then h.temp, h.cloud, h.wind, h.dewPoint = 0.5, 5, 0.5, -6 end
end }, Plot.current())
check(later.value == "no frost" and later.detail:find("next: fri 20 night, likely", 1, true), "Warns nights ahead: " .. later.detail)
check(Frost.card(evening(6), Plot.current(), { date = "2026-11-18", level = 2 }).detail:find("all clear", 1, true),
    "An earlier frost forecast that has gone says all clear")
-- GA11: a frost pocket runs colder, a sheltered plot warmer; hardy plants only warn later.
check(Frost.card(evening(5), Plot.current()).value == "no frost", "Open ground: 5° is no frost")
set("site", "pocket")
local pocket = Frost.card(evening(5), Plot.current())
check(pocket.value == "possible" and pocket.detail:find("frost pocket", 1, true), "A frost pocket: 5° is possible")
set("site", "sheltered")
check(Frost.card(evening(3.5), Plot.current()).value == "no frost", "Sheltered: 3.5° is no frost")
set("site", "open"); set("protect", "hardy")
check(Frost.card(evening(1.5), Plot.current()).value == "no frost" and Frost.card(evening(-3), Plot.current()).value == "likely",
    "Hardy plants only: warns at hard frost")
default()

-- GA2 water this week: the last seven days' rain against what plants lost.
local function week(spec)
    return build { past = spec.past or 7 * 24, hours = 7 * 24, each = function(h, k, hod)
        h.et0 = spec.et0 or 0.15
        if k < 0 then h.rain = spec.rainPast or 0 end
        if spec.each then spec.each(h, k, hod) end
    end }
end
local dry = week {}
local card, b = Water.card(dry)
check(b.verdict == "water" and card.value == "water 25 L/m²" and card.tone == "fair",
    "A dry week: water, in litres per square metre: " .. card.value)
check(card.detail:find("last 7 days: rain 0.0 mm, plants lost 25.2 mm", 1, true) and card.detail:find("deep soak", 1, true),
    "It explains itself: " .. card.detail)
check(Water.card(week { rainPast = 0.2 }).value == "no need", "A wet week (34 mm of rain): no need to water")
local coming = Water.card(week { each = function(h, k) if k >= 18 and k < 26 then h.rain, h.rainChance = 1.5, 80 end end })
check(coming.value == "rain coming" and coming.detail:find("wait: tomorrow brings about 12.0 mm", 1, true),
    "Rain tomorrow: wait for it: " .. coming.detail)
local unlikely = Water.card(week { each = function(h, k) if k >= 18 and k < 26 then h.rain, h.rainChance = 1.5, 20 end end })
check(unlikely.value:find("^water"), "Unlikely rain does not stop watering")
local twelve = { et0 = 12 / 168 }
set("soil", "sandy")
check(Water.balance(week(twelve)).verdict == "water", "Sandy soil needs water after a 12 mm shortfall")
set("soil", "clay")
check(Water.balance(week(twelve)).verdict == "skip", "Clay holds out after a 12 mm shortfall")
default()
Units.use { rain = "in" }
check(Water.card(dry).value == "water 0.98 in", "Inches where rain is read in inches")
Units.use {}
check(Water.card(week { past = 6 }).detail:find("no rain record", 1, true), "Without last week it says so")
-- GA8: the root zone overrules the sums.
local roots = function(upper, lower) return function(h) h.soil_moisture_3_to_9cm, h.soil_moisture_9_to_27cm = upper, lower end end
check(Water.balance(week { rainPast = 0.2, each = roots(0.10, 0.12) }).verdict == "water",
    "Dry at the roots: water, whatever the rain")
check(Water.balance(week { each = roots(0.31, 0.30) }).verdict == "skip", "Moist at the roots: skip")
check(math.abs(Water.rootMoisture { soil_moisture_3_to_9cm = 0.2, soil_moisture_9_to_27cm = 0.3 } - 0.275) < 1e-9,
    "Root moisture weighs the layers by thickness")
check(F.says(verdict(Garden, { time = "2026-11-18T11:00", soil_moisture_3_to_9cm = 0.36, soil_moisture_9_to_27cm = 0.34 }),
    "Soggy"), "Waterlogged roots: too wet to dig")
check(Water.metric({ soil_moisture_3_to_9cm = 0.08, soil_moisture_9_to_27cm = 0.1 }).value == "dry", "The metric says dry")

-- GA3 soil at sowing depth: daily means, a trend, crops warm enough.
local function soil(fn, spec)
    spec = spec or {}
    return build { past = spec.past or 7 * 24 + 6, hours = 7 * 24, spot = spec.spot, each = function(h, k)
        h.soil_temperature_6cm = fn(k)
        h.soil_temperature_18cm = fn(k)
    end }
end
local warming = Soil.card(soil(function(k) return 8 + k / 24 end))
check(warming.value == "beetroot, carrots" and warming.detail:find("warming 4° this week", 1, true)
    and warming.detail:find("next: french and runner beans at 12°", 1, true), "Warming soil: " .. warming.value .. " · "
    .. warming.detail)
check(Soil.card(soil(function() return 3 end)).value == "not yet", "Cold soil: not yet")
check(Soil.card(soil(function() return 16 end)).value == "everything", "Warm soil: everything")
check(Soil.card(soil(function(k) return 10 - k / 24 end)).detail:find("cooling", 1, true), "Cooling soil says so")
local nightCold = Soil.state(soil(function(k) return (k % 24 < 12) and 2 or 14 end))
check(nightCold.today > 7 and nightCold.today < 9, "Daily means, not afternoon peaks")
local spring = { lat = -35, lon = 149 }
check(Soil.lawnCard(soil(function() return 14 end, { spot = spring })).value == "pre-emergent now",
    "Lawn: 13° at 10 cm for three spring days means pre-emergent now")
check(Soil.lawnCard(soil(function() return 15 end)).value == "good for seeding", "Lawn: autumn soil at 15° is good for seeding")

-- GA4 job windows over the next three days.
local calm = build { hours = 72, each = function(h) h.wind, h.gust, h.temp, h.dewPoint = 2, 4, 12, 6 end }
local jobs = {}
for _, job in ipairs(Jobs.list(calm)) do jobs[job.id] = job end
check(jobs.spray.value == "today 08–16h" and jobs.mow.value == "today 08–16h" and jobs.feed.value == "today 08–16h",
    "A calm, dry day: spray, mow and feed today: " .. jobs.spray.value)
check(jobs.harvest.value == "no rush" and jobs.plant.value == "today", "Mild nights: plant out today, no rush to harvest")
local wetToday = build { hours = 72, each = function(h, k)
    h.wind, h.gust, h.temp, h.dewPoint = 2, 4, 12, 6
    if k < 18 then h.rain, h.rainChance = 0.8, 90 end
end }
for _, job in ipairs(Jobs.list(wetToday)) do jobs[job.id] = job end
check(jobs.spray.value == "tomorrow 08–16h" and jobs.mow.value == "tomorrow 12–16h",
    "Rain all day: spray tomorrow, mow once the grass has had 12 dry hours: " .. jobs.mow.value)
check(jobs.feed.value:find("^tomorrow") or jobs.feed.value:find("^today 1"), "Feed waits for the heavy rain to pass")
local windy = build { hours = 72, each = function(h) h.wind, h.gust = 7, 12 end }
for _, job in ipairs(Jobs.list(windy)) do jobs[job.id] = job end
check(jobs.spray.value == "not in 3 days", "Windy for days: no spraying window")
local frostComing = build { hours = 72, each = function(h, k, hod)
    if k > 30 and night(hod) then h.temp, h.cloud, h.wind, h.dewPoint = -1, 5, 0.5, -8 end
end }
for _, job in ipairs(Jobs.list(frostComing)) do jobs[job.id] = job end
check(jobs.harvest.value == "before tomorrow" and jobs.harvest.note:find("frost", 1, true), "Harvest before the frost: " .. jobs.harvest.value .. " " .. jobs.harvest.note)
check(jobs.plant.value == "not yet", "No planting out before a frost")
local function harvestWith(mmPerHour)
    for _, job in ipairs(Jobs.list(build { hours = 72, each = function(h, k) if k >= 20 and k < 30 then h.rain = mmPerHour end end })) do
        if job.id == "harvest" then return job.value end
    end
end
check(harvestWith(2) == "no rush" and harvestWith(3) == "before tomorrow", "A downpour of 25 mm or more means harvest first")
local jobsCard = Jobs.card(frostComing)
check(jobsCard.value == "harvest today" and jobsCard.detail:find("frost comes tomorrow", 1, true), "The jobs card leads with the urgent one: " .. jobsCard.value)

-- GA6 blight (Hutton) and mildew.
local function humid(dates)
    return build { past = 6, hours = 5 * 24, each = function(h, _, hod)
        if dates[h.date] then
            h.temp = 14
            if hod < dates[h.date] then h.humidity = 95 end
        end
    end }
end
local hutton = Protect.blight(humid { ["2026-11-19"] = 8, ["2026-11-20"] = 7 })
check(hutton.level == 2 and hutton.from == "2026-11-19" and hutton.to == "2026-11-20", "Two warm, humid days: a Hutton period")
check(Protect.blight(humid { ["2026-11-19"] = 8 }).level == 1, "One such day is a near miss")
check(Protect.blight(humid { ["2026-11-19"] = 4, ["2026-11-20"] = 4 }).level == 0, "Four humid hours are not enough")
check(Protect.blight(build { hours = 96, each = function(h) h.temp, h.humidity = 6, 99 end }).level == 0, "Cold and humid is no blight")
local blightCard
for _, c in ipairs(Protect.cards(humid { ["2026-11-19"] = 8, ["2026-11-20"] = 7 })) do
    if c.id == "blight" then blightCard = c end
end
check(blightCard and blightCard.value == "high" and blightCard.detail:find("risk", 1, true), "Blight is a labelled risk")
check(Protect.mildew(build { past = 6, hours = 96, each = function(h, _, hod)
    h.temp = h.isDay and 24 or 15
    if not h.isDay then h.humidity = 95 end
end }) == "2026-11-18", "Warm days and damp nights: mildew weather")

-- GA7 warnings for plants in the next 48 hours, worst first.
local stormy = build { hours = 72, each = function(h, k)
    if k == 10 then h.gust = 19 end
    if k == 12 then h.code = 96 end
    if k >= 20 and k < 26 then h.snowfall = 1 end
    if k == 30 then h.temp = 31 end
    if k == 60 then h.gust = 30 end
end }
local warnings = Protect.warnings(stormy)
check(#warnings == 4 and warnings[1].id == "hail" and warnings[2].value == "gusts 19 m/s" and warnings[3].value == "snow 6 cm"
    and warnings[4].value == "heat 31°", "Hail, gusts, snow and heat, only the next 48 h")
check(#Protect.warnings(build {}) == 0, "Calm weather: no warnings")

-- GA9 frost dates and hardiness from 30 years of minimums.
-- Every date of the years with `fn(year, day)`, the day counted as in a
-- non-leap year (29 February shares 1 March's number).
local function years(first, last, fn)
    local dates, mins = {}, {}
    for y = first, last do
        local start = os.time { year = y, month = 1, day = 1, hour = 12 }
        local leap = (y % 4 == 0 and y % 100 ~= 0) or y % 400 == 0
        for d = 1, leap and 366 or 365 do
            dates[#dates + 1] = os.date("%Y-%m-%d", start + (d - 1) * 86400)
            mins[#mins + 1] = fn(y, (leap and d > 59) and d - 1 or d)
        end
    end
    return dates, mins
end
-- Frost until day 100–109 (10–19 April) and from day 290–299 (17–26 October).
local dates, mins = years(1996, 2025, function(y, d)
    if d == 20 then return -10 end
    return (d < 100 + y % 10 or d >= 290 + y % 10) and -1 or 8
end)
local c = Climate.compute(dates, mins, 53.8)
check(c and c.years == 30 and c.lastSpring.p50 == 103 and c.lastSpring.p90 == 107 and c.lastSpring.p10 == 99,
    "Last spring frost: 50 % by 13 april, 10 % later than 17 april")
check(c.firstAutumn.p50 == 294 and c.firstAutumn.p10 == 290, "First autumn frost: 50 % by 21 october")
check(Climate.dateText(c.lastSpring.p50) == "13 apr" and Climate.dateText(294) == "21 oct", "Dates read as days and months")
check(c.zone == "8a" and c.rhs == "H4" and math.abs(c.coldest + 10) < 1e-9, "−10° winters: zone 8a, RHS H4")
check(c.season >= 185 and c.season <= 195, "About 190 frost-free days")
local sd, sm = years(1996, 2025, function(_, d) return (d >= 160 and d < 240) and -1 or 9 end)
local southern = Climate.compute(sd, sm, -35)
check(southern and southern.lastSpring.p50 == 239 and southern.firstAutumn.p50 == 160,
    "South: last frost in late august, first in june")
check(Climate.compute(years(2020, 2025, function() return -1 end)) == nil, "Fewer than ten years: no dates")
check(Climate.usda(-20) == "6b" and Climate.usda(-23.3) == "6a" and Climate.usda(-1) == "10a" and Climate.rhs(-25) == "H7" and Climate.rhs(12) == "H1b",
    "Zones and RHS ratings")
-- One request, cached, and only once the place has been seen.
Climate.reset()
check(Climate.url(LEEDS, NOW) == nil, "An unseen place asks nothing")
Climate.want(LEEDS)
local url = Climate.url(LEEDS, NOW)
check(url and url:find("archive%-api") and url:find("start_date=1996%-01%-01") and url:find("end_date=2025%-12%-31"),
    "Then 1996–2025 from the archive: " .. tostring(url))
check(Climate.url(LEEDS, NOW + 3600) == nil and Climate.url(LEEDS, NOW + 2 * 86400) ~= nil, "Retried a day later at most")
local fc = build {}
Climate.source.parse({ latitude = 53.8, longitude = -1.55, daily = { time = dates, temperature_2m_min = mins } }, fc)
check(fc.climate and fc.climate.from == 1996 and fc.climate.to == 2025, "Parsed onto the forecast")
Climate.reset()
check(Climate.url(LEEDS, NOW + 9 * 86400) == nil and Climate.get(build {}, LEEDS).zone == "8a", "Kept on the device")
local metrics = {}
for _, row in ipairs(Climate.metrics(build {}, LEEDS)) do metrics[row.label] = row end
check(metrics["last spring frost"].value == "13 apr" and metrics["last spring frost"].note:find("10 % chance of frost after 17 apr",
    1, true) and metrics.hardiness.value == "zone 8a" and metrics.hardiness.note:find("rated H4", 1, true),
    "Frost-date odds and, in Britain, the RHS rating: " .. metrics.hardiness.note)

-- GA11 settings, GA12 16 days, GA13 moon.
local request = Activities.request(LEEDS)
check(request.pastDays == 7 and request.forecastDays == 16, "A week back and 16 days ahead")
local forecastUrl = Forecast.urls(LEEDS, false, request)
check(forecastUrl:find("past_days=7", 1, true) and forecastUrl:find("forecast_days=16", 1, true)
    and forecastUrl:find("soil_temperature_6cm", 1, true) and forecastUrl:find("soil_moisture_9_to_27cm", 1, true),
    "The request asks for soil and history")
check(Settings.step("garden", "site") and Plot.current().offset == -2, "The plot setting steps to frost pocket")
default()
local function cardIds(forecast)
    local ids = {}
    for _, card in ipairs(Garden.cards(forecast, LEEDS)) do ids[#ids + 1] = card.id end
    return table.concat(ids, ",")
end
local parsed = Forecast.parse(F.weather(53.8, {
    soil_temperature_6cm = function() return 9 end, soil_temperature_18cm = function() return 10 end }), nil, NOW, LEEDS)
check(cardIds(parsed):find("^frost,water,best,") and cardIds(parsed):find("sow", 1, true) and cardIds(parsed):find("jobs", 1, true)
    and not cardIds(parsed):find("moon", 1, true) and not cardIds(parsed):find("lawn", 1, true),
    "GA5: frost tonight, water this week and the best time lead: " .. cardIds(parsed))
do
    -- The hub keeps three cards, and frost tonight and water this week lead.
    local Report = require "Weather/report"
    local hub = {}
    for _, row in ipairs(Report.hub(LEEDS, parsed, { Garden }, nil, Garden)) do
        if row.kind == "card" then hub[#hub + 1] = row.id:match("([^:]+)$") end
    end
    check(#hub == 3 and hub[1] == "frost" and hub[2] == "water", "The hub leads with frost and water: " .. table.concat(hub, ","))
end
set("moon", true); set("lawn", true)
local moon
for _, card in ipairs(Garden.cards(parsed, LEEDS)) do if card.id == "moon" then moon = card end end
check(moon and moon.title:find("tradition", 1, true) and moon.detail:find("no evidence", 1, true), "Moon gardening, labelled")
check(cardIds(parsed):find("lawn", 1, true), "The lawn card with a lawn")
default()
local hub = Report.hub(LEEDS, parsed, Activities.at(LEEDS), NOW, Activities.lead())
check(hub[1].id == "leeds:card:frost" and hub[2].id == "leeds:card:water" and hub[3].id == "leeds:card:best",
    "The hub opens on the garden's answers")
local labels = {}
for _, metric in ipairs(Garden.metrics(parsed.hours[parsed.now], parsed)) do labels[#labels + 1] = metric.label end
labels = table.concat(labels, ",")
check(labels:find("frost tonight,", 1, true) and labels:find("soil at the roots", 1, true) and labels:find("spray", 1, true)
    and labels:find("blight", 1, true) and labels:find("hardiness", 1, true), "The activity page lists it all: " .. labels)
-- All-clear memory: a frost forecast is remembered per place.
local frosted = evening(1)
Garden.cards(frosted, LEEDS)
local saved = Storage.load("garden-frost")
check(saved and saved.leeds and saved.leeds.level == 2, "A frost warning is remembered")
check(Garden.cards(evening(8), LEEDS)[1].detail:find("all clear", 1, true), "Then the all-clear when it goes")

-- The edition: its places, and the archive request never holding up the hub.
local Places = require "Weather/places"
local appStore = F.storage("garden", Places, Climate.key)
Climate.reset()
local ui = F.checkEdition("garden")
local first = Places[1]
check(Climate.saved(first) and Climate.saved(first).wanted, "The first place is marked for frost dates")
local asked = false
for _, u in ipairs(ui.net.requests) do asked = asked or u:find("archive%-api") ~= nil end
check(not asked, "The first load asks nothing of the archive")
ui.net.answers["archive%-api"] = function()
    return { latitude = first.lat, longitude = first.lon, daily = { time = dates, temperature_2m_min = mins } }
end
local refreshed = ui.app.provider.forecastFor(first.id, true)
asked = false
for _, u in ipairs(ui.net.requests) do asked = asked or u:find("archive%-api") ~= nil end
check(asked and refreshed and Climate.get(refreshed, first).zone == "8a", "The next load fetches and keeps the frost dates")
F.done("weather-garden")
