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
        local epoch = (spec.at or NOW) + k * 3600
        local time = os.date("!%Y-%m-%dT%H:00", epoch)
        local hod = tonumber(time:sub(12, 13))
        local h = { epoch = epoch, time = time, date = time:sub(1, 10), temp = 10, dewPoint = 5, cloud = 80, wind = 3,
            gust = 5, rain = 0, rainChance = 10, humidity = 70, et0 = 0.05, code = 3, isDay = hod >= 8 and hod <= 16 }
        if spec.each then spec.each(h, k, hod) end
        hours[#hours + 1] = h
    end
    return { hours = hours, now = past + 1, epoch = spec.at or NOW, spot = spec.spot or LEEDS, missing = {} }
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

-- Existing services (Research/services-garden.md). Every answer here is a
-- synthetic one in the service's documented shape; no test touches the network.
local Warnings = require "Weather/editions/garden/warnings"
local Degree = require "Weather/editions/garden/degree"
local Sky = require "Weather/sky"
local DAN = { id = "columbus", title = "Columbus", lat = 39.96, lon = -83.0, activities = { "garden" } }
local SOUTH = { id = "canberra", title = "Canberra", lat = -35.3, lon = 149.1, activities = { "garden" } }
for _, place in ipairs { LEEDS, DAN, SOUTH } do store:remove(Degree.key(place)) end
store:remove("garden-drought")
Degree.reset()
local function iso(epoch) return os.date("!%Y-%m-%dT%H:%M:00+00:00", epoch) end
local function cardsOf(forecast, place)
    local byId, ids = {}, {}
    for _, c in ipairs(Garden.cards(forecast, place)) do byId[c.id], ids[#ids + 1] = c, c.id end
    return byId, table.concat(ids, ",")
end

-- SV-GA1: NWS alerts for US places, shown as issued.
check(Warnings.nws.url(DAN):find("^https://api%.weather%.gov/alerts/active%?point=39%.9600,%-83%.0000")
    and Warnings.nws.url(LEEDS) == nil and Warnings.nws.headers.Accept == "application/geo+json",
    "NWS is asked for US places only, as GeoJSON")
local function nwsAnswer(...)
    local features = {}
    for _, p in ipairs { ... } do features[#features + 1] = { type = "Feature", properties = p } end
    return { type = "FeatureCollection", features = features }
end
local frostAdvisory = { event = "Frost Advisory", status = "Actual", messageType = "Alert", severity = "Minor",
    senderName = "NWS Wilmington OH", headline = "Frost Advisory issued November 18 at 3:12AM EST until November 19 at 9:00AM EST by NWS Wilmington OH",
    onset = iso(NOW + 12 * 3600), ends = iso(NOW + 27 * 3600) }
local function usNight(temp)
    local f = evening(temp)
    f.spot, f.days = DAN, {}
    return f
end
local fc = usNight(1.5)
Warnings.nws.parse(nwsAnswer(frostAdvisory,
    { event = "Winter Storm Warning", status = "Actual", headline = "not for the garden" },
    { event = "Freeze Warning", status = "Test", headline = "a test" },
    { event = "High Wind Warning", status = "Actual", headline = "old", ends = iso(NOW - 3600) }), fc)
check(#fc.official.list == 2, "Kept: frost, freeze, heat and wind; tests dropped")
local byId, ids = cardsOf(fc, DAN)
check(ids:find("^official,frost,water") and byId.official.value == "Frost Advisory"
    and byId.official.title == "official warning (NWS)" and byId.official.rank == 0
    and byId.official.detail:find(frostAdvisory.headline, 1, true) and byId.official.detail:find("NWS Wilmington OH", 1, true),
    "The NWS advisory leads, unmodified, with its issuer: " .. ids)
check(byId.frost.detail:find("NWS: Frost Advisory in force", 1, true) and byId.frost.title == "frost tonight",
    "The app's own estimate stays its own card and names the advisory")
local hubIds = {}
for _, row in ipairs(Report.hub(DAN, fc, { Garden }, nil, Garden)) do
    if row.kind == "card" then hubIds[#hubIds + 1] = row.id:match("([^:]+)$") end
end
check(table.concat(hubIds, ",") == "official,frost,water", "The hub leads with NWS, then frost and water")
check(Warnings.active(fc, NOW + 30 * 3600)[1] == nil and select(1, Warnings.state(fc, DAN, NOW + 30 * 3600)) == "quiet",
    "An advisory that has ended is gone")
local quiet = usNight(1.5)
Warnings.nws.parse(nwsAnswer(), quiet)
byId = cardsOf(quiet, DAN)
check(not byId.official and byId.frost.detail:find("no NWS frost alert in force", 1, true)
    and byId.frost.detail:find("our estimate", 1, true), "NWS quiet on a frosty night: this is our estimate")
local failed = usNight(1.5)
check(not pcall(Warnings.nws.parse, { title = "Service Unavailable" }, failed), "An answer without alerts fails the source")
failed.missing.nws = "HTTP 503"
byId = cardsOf(failed, DAN)
check(not byId.official and byId.frost.detail:find("NWS alerts not reached", 1, true)
    and not byId.frost.detail:find("no NWS", 1, true), "A failed NWS never reads as no warning: " .. byId.frost.detail)
check(Warnings.metric(failed, DAN).value == "not reached" and Warnings.metric(quiet, DAN).value == "none for the garden"
    and Warnings.metric(fc, DAN).value == "Frost Advisory", "The activity page says what NWS said, or that it was not heard")
check(Warnings.metric(build {}, LEEDS).note:find("Met Office Weather Warnings", 1, true)
    and Warnings.metric(build {}, { lat = 52.5, lon = 13.4 }) == nil, "Britain gets a Met Office link; elsewhere nothing")
check(not cardsOf(evening(1.5), LEEDS).frost.detail:find("NWS", 1, true), "No NWS talk outside the US")

-- SV-GA2: growing degree days (GA10).
check(Degree.day(25, 12) == 8.5 and Degree.day(35, 20) == 15 and Degree.day(8, 2) == 0 and Degree.day(14, 4) == 2,
    "A day's degree days: base 10°, capped at 30°")
check(Degree.start(DAN, "2026-11-18") == "2026-01-01" and Degree.start(LEEDS, "2026-11-18") == "2026-04-01"
    and Degree.start(SOUTH, "2026-03-01") == "2025-07-01", "Seasons: 1 January, 1 April in Britain, 1 July south")
local CLEVELAND = { lat = 41.5, lon = -81.7 }
store:remove(Degree.key(CLEVELAND))
check(Degree.url(CLEVELAND, NOW) == nil, "An unseen place asks nothing")
Degree.want(DAN)
url = Degree.url(DAN, NOW)
check(url and url:find("archive%-api") and url:find("start_date=2026%-01%-01&end_date=2026%-11%-13", 1)
    and url:find("temperature_2m_max,temperature_2m_min", 1, true), "Then this season to five days ago: " .. tostring(url))
check(Degree.url(DAN, NOW + 3600) == nil, "A try waits six hours")
-- A season of days: `fn(date)` gives max and min.
local function season(first, last, fn)
    local dates, maxs, mins = {}, {}, {}
    local e, stop = Sky.epochOf(first), Sky.epochOf(last)
    while e <= stop do
        local date = os.date("!%Y-%m-%d", e)
        local mx, mn = fn(date)
        local i = #dates + 1
        dates[i], maxs[i], mins[i] = date, mx, mn
        e = e + 86400
    end
    return { latitude = DAN.lat, longitude = DAN.lon, daily = { time = dates, temperature_2m_max = maxs, temperature_2m_min = mins } }
end
local warm = build { past = 7 * 24, hours = 7 * 24, spot = DAN }
warm.days = {}
for d = -7, 7 do
    local date = os.date("!%Y-%m-%d", NOW + d * 86400)
    warm.days[#warm.days + 1] = { date = date, max = 20, min = 10 }
end
Degree.source.parse(season("2026-01-01", "2026-11-13", function() return 18.4, 10 end), warm)
check(warm.gdd.through == "2026-11-13" and math.abs(warm.gdd.total - 317 * 4.2) < 1e-6, "The archive's days are added up")
check(Degree.url(DAN, NOW + 7 * 3600) == nil and Degree.url(DAN, NOW + 86400 + 7 * 3600) ~= nil, "Asked again a day later")
local total = Degree.total(warm, DAN)
check(total and math.abs(total.total - (317 * 4.2 + 5 * 5)) < 1e-6 and total.index == 322,
    "Then the forecast's days to today")
local gap = build { spot = DAN }
gap.days = { { date = "2026-11-17", max = 20, min = 10 }, { date = "2026-11-18", max = 20, min = 10 } }
gap.gdd = warm.gdd
check(Degree.total(gap, DAN) == nil, "A missing day gives no total, not a wrong one")
local broken = build { spot = DAN }
Degree.source.parse(season("2026-01-01", "2026-03-10", function(date) if date == "2026-02-01" then return nil, nil end return 20, 10 end), broken)
check(broken.gdd.through == "2026-01-31", "The total stops before a missing archive day")
-- 30 years at 4 °C-days a day: today's total is reached 18 days later in an average year.
local normals = season("1996-01-01", "2025-12-31", function() return 18, 10 end)
local normal = Degree.normal(normals.daily.time, normals.daily.temperature_2m_max, normals.daily.temperature_2m_min, DAN)
check(normal and normal[1] == 4 and normal[322] == 1288, "The average season by day")
check(Degree.ahead(total, normal) == 18, "18 days ahead of an average year")
local seasonCard = Degree.card(warm, DAN, { gddNormal = normal, from = 1996, to = 2025 }, Plot.current())
check(seasonCard.value == "18 days ahead" and seasonCard.detail:find("1356 degree days since 1 jan (base 10°)", 1, true)
    and seasonCard.detail:find("sweetcorn", 1, true) and seasonCard.detail:find("average year has 1288 by now (1996–2025)", 1, true)
    and seasonCard.hub == false, "The growing season card: " .. seasonCard.detail)
Units.use { temp = "f" }
check(Degree.card(warm, DAN, { gddNormal = normal }, Plot.current()).detail:find("2442 degree days since jan 1 (base 50°)", 1, true),
    "°F-days where the US reads °F")
Units.use {}
local flat = {}
for k = 1, 366 do flat[k] = k < 300 and 4 * k or 1196 + (k - 299) * 0.2 end
check(select(2, Degree.ahead(total, flat)) and Degree.card(warm, DAN, { gddNormal = flat }, Plot.current()).value
    == "156 degree days above average", "Late in the season it counts degree days, not days")
check(Degree.card(build { spot = DAN }, DAN, nil, Plot.current()).value == "not yet", "Without records: not yet")
check(Degree.card(build { at = NOW - 240 * 86400, spot = LEEDS }, LEEDS, nil, Plot.current()).value == "from 1 apr",
    "Britain counts from 1 April")
check(Degree.crabgrass({ total = 120, start = "2026-01-01", today = "2026-04-10" }, DAN):find("crabgrass preventer: now", 1, true)
    and Degree.crabgrass({ total = 60, start = "2026-01-01", today = "2026-04-01" }, DAN):find("not yet", 1, true)
    and Degree.crabgrass({ total = 120, start = "2026-04-01", today = "2026-04-10" }, LEEDS) == nil,
    "Lawns: the crabgrass window by °F-days from 1 January")
-- The frost dates' archive now brings the average season too; an older
-- copy without it is asked once more and keeps its frost dates meanwhile.
local cfc = build { spot = DAN }
Climate.source.parse(normals, cfc)
check(cfc.climate.gddNormal and cfc.climate.gddNormal[322] == 1288 and Climate.url(DAN, NOW + 9 * 86400) == nil,
    "Frost dates and the average season in one request")
store:save(Climate.key(SOUTH), { frost = true, climate = { zone = "9b" } })
Climate.reset()
local again = Climate.url(SOUTH, NOW)
check(again and again:find("temperature_2m_max", 1, true) and Climate.get(nil, SOUTH).zone == "9b",
    "An older copy is asked once more and still shows its zone")
store:remove(Climate.key(SOUTH))
Climate.reset()

-- SV-GA3: pollen for mowing, Europe only.
check(Jobs.pollenSource.url(LEEDS):find("air%-quality") and Jobs.pollenSource.url(LEEDS):find("grass_pollen,birch_pollen,mugwort_pollen", 1, true)
    and Jobs.pollenSource.url(DAN) == nil, "CAMS pollen is asked for European places only")
local function pollenDay(fn)
    local f = build { hours = 72, each = function(h) h.wind, h.gust, h.temp, h.dewPoint = 2, 4, 12, 6 end }
    local times, grass, birch = {}, {}, {}
    for index, h in ipairs(f.hours) do
        times[index], grass[index], birch[index] = h.time, fn(tonumber(h.time:sub(12, 13)), h.date), 5
    end
    Jobs.pollenSource.parse({ hourly = { time = times, grass_pollen = grass, birch_pollen = birch } }, f)
    return f
end
local pollen = pollenDay(function(hod, date) return date == "2026-11-18" and hod <= 13 and 80 or 10 end)
check(pollen.hours[pollen.now + 2].grass_pollen == 80 and Jobs.pollen(pollen)
    == "grass pollen high until 14:00: mow and weed after, if you have hay fever", "High grass pollen until 14:00")
for _, job in ipairs(Jobs.list(pollen)) do jobs[job.id] = job end
check(jobs.mow.note:find("grass pollen high until 14:00", 1, true), "It is a reason on mowing: " .. jobs.mow.note)
byId = cardsOf(pollen, LEEDS)
check(byId.best.detail:find("grass pollen high", 1, true), "And on the best time today: " .. byId.best.detail)
check(F.says(Garden.rate(pollen.hours[pollen.now + 3], pollen):result(), "High grass pollen"), "The hour notes it")
check(Jobs.pollen(pollenDay(function(hod) return hod >= 12 and 60 or 30 end)):find("from 12:00: mow and weed before", 1, true),
    "Pollen rising at noon: mow before")
check(Jobs.pollen(pollenDay(function() return 60 end)):find("rest of the day", 1, true), "High all day")
check(Jobs.pollen(pollenDay(function() return 30 end)) == nil and Jobs.pollen(build {}) == nil, "Medium, or no data: nothing said")

-- SV-GA4: the U.S. Drought Monitor's category.
local droughtUrl = Water.droughtSource.url(DAN, nil, NOW)
check(droughtUrl:find("USDM_current/FeatureServer/0/query?geometry=-83.0000,39.9600&geometryType=esriGeometryPoint", 1, true)
    and droughtUrl:find("outFields=DM", 1, true) and Water.droughtSource.url(LEEDS, nil, NOW) == nil,
    "The current map is asked at the point, US only")
local dryUS = week { }
dryUS.spot = DAN
Water.droughtSource.parse({ features = { { attributes = { DM = 1 } }, { attributes = { DM = 2 } } } }, dryUS)
check(Water.card(dryUS).detail:find("Severe drought (D2) here (U.S. Drought Monitor): check your utility's watering days", 1, true),
    "D2 at the point: the water card says so")
check(Water.droughtSource.url(DAN, nil, NOW + 3600) == nil and Water.droughtSource.url(DAN, nil, NOW + 2 * 86400) ~= nil,
    "Weekly map, asked at most once a day")
local later = week {}
later.spot = DAN
check(Water.drought(later) == 2 and Water.card(later).detail:find("Severe drought", 1, true), "Kept for the next day's forecast")
local wet = week {}
wet.spot = DAN
Water.droughtSource.parse({ features = {} }, wet)
check(Water.drought(wet) == -1 and not Water.card(wet).detail:find("drought", 1, true), "Outside the drought areas: nothing said")
check(not pcall(Water.droughtSource.parse, { error = { code = 400 } }, build { spot = DAN }), "A refused query fails the source")
store:remove("garden-drought")
local unheard = week {}
unheard.spot, unheard.missing.drought = DAN, "HTTP 500"
check(not Water.card(unheard).detail:find("drought", 1, true), "A failed map says nothing, not 'no drought'")

-- SV-GA5: a hosepipe-ban pointer in a dry British summer, never a claim.
local July = NOW - 126 * 86400 -- 2026-07-15 06:00 UTC
local function summer(spot)
    local f = build { at = July, past = 7 * 24, hours = 7 * 24, spot = spot, each = function(h) h.et0 = 0.15 end }
    return Water.card(f)
end
local hose = summer(LEEDS)
check(hose.value:find("^water") and hose.detail:find("is there a hosepipe ban? ask your water company (Water UK: water.org.uk)", 1, true)
    and not hose.detail:find("there is a ban", 1, true), "A dry British July: where to check for a ban")
check(not Water.card(dry).detail:find("hosepipe", 1, true) and not summer(DAN).detail:find("hosepipe", 1, true),
    "Not in November, not in the US")

-- SV-GA8: where the official ratings live, by name.
local cmetrics = {}
for _, row in ipairs(Climate.metrics({ climate = c }, LEEDS)) do cmetrics[row.label] = row end
check(cmetrics.hardiness.note:find("rhs.org.uk", 1, true), "Britain: RHS hardiness ratings")
cmetrics = {}
for _, row in ipairs(Climate.metrics({ climate = c }, DAN)) do cmetrics[row.label] = row end
check(cmetrics.hardiness.note:find("planthardiness.ars.usda.gov", 1, true), "US: the official USDA map")
local gb = {}
for _, metric in ipairs(Garden.metrics(parsed.hours[parsed.now], parsed)) do gb[metric.label] = metric end
check(gb["blight (Hutton)"].note:find("BlightWatch", 1, true) and gb["official warnings"].value == "Met Office",
    "Leeds: BlightWatch and the Met Office link")

-- Credit on the settings' about.
local Provider = require "Weather/provider"
local credits = {}
for _, about in ipairs(Provider.about()) do credits[#credits + 1] = about[1] end
credits = table.concat(credits, "|")
check(credits:find("Alerts: National Weather Service", 1, true) and credits:find("Copernicus Atmosphere Monitoring Service", 1, true)
    and credits:find("U.S. Drought Monitor", 1, true), "Each service is credited: " .. credits)

-- Through the forecast load: each source asked where it covers, and a
-- failed one leaves the rest standing.
local net = F.network()
net.answers["api%.weather%.gov"] = function() return nwsAnswer(frostAdvisory) end
net.answers["USDM_current"] = function() return { features = { { attributes = { DM = 3 } } } } end
local loaded = Forecast.load(DAN, false, net.fetch, NOW, Activities.request(DAN))
local nwsOptions
for index, u in ipairs(net.requests) do if u:find("api.weather.gov", 1, true) then nwsOptions = net.options[index] end end
check(loaded and loaded.official and #loaded.official.list == 1 and loaded.drought.dm == 3
    and nwsOptions and nwsOptions.headers.Accept == "application/geo+json" and nwsOptions.headers["User-Agent"],
    "A US load asks NWS (with its headers) and the drought map")
local askedPollen = false
for _, u in ipairs(net.requests) do askedPollen = askedPollen or u:find("air%-quality") ~= nil end
check(not askedPollen, "…and no European pollen")
store:remove("garden-drought")
local down = F.network()
down.answers["api%.weather%.gov"] = function() return nil, "HTTP 503" end
down.answers["USDM_current"] = function() return nil, "HTTP 500" end
local standing = Forecast.load(DAN, false, down.fetch, NOW, Activities.request(DAN))
check(standing and standing.missing.nws and standing.missing.drought and #Garden.cards(standing, DAN) > 0,
    "NWS and the drought map down: the forecast and cards stand")
store:remove("garden-drought")
for _, place in ipairs { LEEDS, DAN, SOUTH } do store:remove(Degree.key(place)) end
Degree.reset()

-- The edition: its places, and the archive request never holding up the hub.
local Places = require "Weather/places"
local appStore = F.storage("garden", Places, Climate.key)
Climate.reset()
for _, place in ipairs(Places) do appStore:remove(Degree.key(place)) end
appStore:remove("garden-drought")
Degree.reset()
local ui = F.checkEdition("garden")
local first = Places[1]
check(Climate.saved(first) and Climate.saved(first).wanted, "The first place is marked for frost dates")
local asked = false
for _, u in ipairs(ui.net.requests) do asked = asked or u:find("archive%-api") ~= nil end
check(not asked, "The first load asks nothing of the archive")
-- The archive serves both: 30 years for the frost dates, this season for
-- the degree days.
ui.net.answers["archive%-api"] = function(u)
    if u:find("models=era5_land", 1, true) then
        return { latitude = first.lat, longitude = first.lon, daily = { time = dates, temperature_2m_min = mins } }
    end
    local answer = season(u:match("start_date=([%d%-]+)"), u:match("end_date=([%d%-]+)"), function() return 20, 10 end)
    answer.latitude, answer.longitude = first.lat, first.lon
    return answer
end
local refreshed = ui.app.provider.forecastFor(first.id, true)
asked = false
for _, u in ipairs(ui.net.requests) do asked = asked or u:find("archive%-api") ~= nil end
check(asked and refreshed and Climate.get(refreshed, first).zone == "8a", "The next load fetches and keeps the frost dates")
check(Degree.saved(first) and Degree.saved(first).through and Degree.saved(first).total > 0,
    "…and this season's degree days")
F.done("weather-garden")
