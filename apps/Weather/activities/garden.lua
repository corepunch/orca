-- Gardening: is it a good hour for garden work, and what does the garden
-- need? Gardeners watch for frost on tender plants (ground frost, not only
-- air frost), soil too wet to dig or frozen, wind that ruins spraying, and
-- whether the week's rain makes up for the water plants lose to sun and
-- wind (evapotranspiration). The Gardeners' Weather edition leads its hub
-- with the answers (`cards`): frost tonight, water this week, the best time
-- today, what the soil is warm enough to sow, and the jobs the weather
-- times. The rules live in editions/garden/ (see Research/garden.md).
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Report = require "Weather/report"
local Storage = require "Weather/storage"
local Plot = require "Weather/editions/garden/plot"
local Frost = require "Weather/editions/garden/frost"
local Water = require "Weather/editions/garden/water"
local Soil = require "Weather/editions/garden/soil"
local Jobs = require "Weather/editions/garden/jobs"
local Protect = require "Weather/editions/garden/protect"
local Climate = require "Weather/editions/garden/climate"

local Garden = {
    id = "garden", title = "garden", needsMarine = false,
    -- A week of history for the water balance, soil at sowing and root
    -- depth, snowfall for the snow-load warning, and 16 days ahead.
    fields = {
        hourly = { "soil_temperature_6cm", "soil_temperature_18cm", "soil_moisture_3_to_9cm", "soil_moisture_9_to_27cm",
            "snowfall" },
        pastDays = 7, forecastDays = 16,
    },
    sources = { Climate.source },
    settings = Plot.SETTINGS,
}

-- The hours after `hour`, up to `count` of them.
local function ahead(forecast, hour, count)
    local rows = {}
    for _, other in ipairs(forecast.hours or {}) do
        if other.epoch and hour.epoch and other.epoch > hour.epoch and other.epoch <= hour.epoch + count * 3600 then
            rows[#rows + 1] = other
        end
    end
    return rows
end

-- The worst frost in a list of hours (Frost.hour), or nil.
local function worstFrost(rows, plot)
    local worst
    for _, row in ipairs(rows) do
        local f = Frost.hour(row, plot)
        if f and (not worst or f.level > worst.level or (f.level == worst.level and f.ground < worst.ground)) then worst = f end
    end
    return worst
end

function Garden.rate(hour, forecast)
    local plot = Plot.current()
    local card = Score.card(55)
    if Sky.isStorm(hour.code) then card:cap(10, "Thunderstorm: stay indoors.") end
    if not hour.isDay then card:cap(25, "Dark: garden work waits for daylight.") end

    if (hour.temp or 5) <= 0 or (hour.soilTemp or 5) <= 0 then card:add(-20, "Frozen ground: leave the soil alone.") end
    local frost = worstFrost(ahead(forecast, hour, 12), plot)
    if frost and frost.level == 2 then card:add(-5, "Frost likely tonight: cover tender plants.")
    elseif frost and frost.level == 1 then card:add(-3, "Frost possible tonight: have fleece ready.") end

    local full = Water.fullness(hour, plot)
    if Sky.isWet(hour.code) or Sky.isSnow(hour.code) then card:add(-15, "Wet: digging now packs the soil hard.")
    elseif full and full >= 1.1 then card:add(-8, "Soggy soil: wait for it to drain.")
    elseif full and full < 0.3 then card:note("Dry at the roots: water in the evening.") end

    local wind, gust = hour.wind or 0, hour.gust or 0
    if wind > 8 or gust >= 15 then card:add(-10, "Windy: no spraying; stake tall plants.")
    elseif wind < 4 then card:add(4, "Calm: good for spraying and sowing.") end

    local temp = hour.temp
    if temp and temp >= 8 and temp <= 24 then card:add(12, "Pleasant weather for garden work.")
    elseif temp and temp > 28 then card:add(-10, "Hot: work in the morning, water at dusk.") end
    return card
end

function Garden.window(hour) return hour.isDay end

function Garden.note(hour)
    local soil = hour.soil_temperature_6cm or hour.soilTemp
    return ("%s · soil %s · rain %s"):format(Units.temp(hour.temp), Units.temp(soil), Score.fmt(hour.rainChance, "%"))
end

-- What the frost card said last time per place, for the all-clear.
local function remembered(place)
    local saved = Storage.load("garden-frost")
    return type(saved) == "table" and saved[place.id] or nil, type(saved) == "table" and saved or {}
end

local function remember(place, card)
    local last, all = remembered(place)
    if card.level > 0 and not (last and last.date == card.date and last.level == card.level) then
        all[place.id] = { date = card.date, level = card.level }
        Storage.save("garden-frost", all)
    end
    return last
end

-- The best hours for garden work today (tomorrow once today's are past).
local function bestTime(forecast)
    local now = forecast.hours[forecast.now]
    local best, span = Report.bestOfDay(Garden, forecast, now.date)
    local title = "best time today"
    if not best then
        best, span = Report.bestOfDay(Garden, forecast, Plot.addDays(now.date, 1))
        title = "best time tomorrow"
    end
    if not best then return nil end
    local reason = best.reasons[1]
    return { id = "best", title = title, value = span, tone = best.verdict,
        detail = best.word .. (reason and " · " .. reason.text or "") }
end

-- Tradition, labelled as such (GA13): sow what crops above ground while
-- the moon waxes, roots while it wanes. No trial has shown an effect.
local function moonCard(forecast)
    local moon = Sky.moon(forecast.epoch)
    local waxing = moon.phase < 0.5
    return { id = "moon", title = "moon gardening (tradition)", value = moon.name, tone = "info",
        detail = (waxing and "tradition: sow leaves, flowers and fruiting crops while the moon waxes"
            or "tradition: sow roots, prune and weed while the moon wanes") .. " · no evidence backs it; sow by the soil" }
end

-- The edition's answers, in the order a gardener asks: frost tonight, water
-- this week, the best time today, then sowing, jobs and warnings.
function Garden.cards(forecast, place)
    if not (forecast.hours and forecast.hours[forecast.now]) then return {} end
    place = place or forecast.spot or {}
    forecast.spot = forecast.spot or place
    local plot = Plot.current()
    Climate.want(place)
    local cards = {}
    local function add(card) if card then cards[#cards + 1] = card end end
    local frost = Frost.card(forecast, plot, place.id and remembered(place))
    if frost and place.id then remember(place, frost) end
    add(frost)
    add((Water.card(forecast, plot)))
    add(bestTime(forecast))
    for _, card in ipairs(Protect.cards(forecast)) do add(card) end
    add((Soil.card(forecast)))
    if plot.lawn then add(Soil.lawnCard(forecast)) end
    add((Jobs.card(forecast, plot)))
    if plot.moon then add(moonCard(forecast)) end
    return cards
end

function Garden.metrics(hour, forecast)
    local plot = Plot.current()
    local rows = {}
    local nights = Frost.nights(forecast, 1, plot)
    local tonight = nights[1]
    if tonight then
        rows[#rows + 1] = { label = "frost tonight", value = Frost.LEVELS[tonight.level],
            tone = tonight.level == 2 and "poor" or tonight.level == 1 and "fair" or "good",
            note = ("lowest: air %s, ground about %s"):format(Units.temp(tonight.air), Units.temp(tonight.ground)) }
    end
    local b = Water.balance(forecast, plot)
    if b then
        rows[#rows + 1] = { label = "rain · water lost, 7 days",
            value = Units.rain(b.rainPast) .. " · " .. Units.rain(b.lostPast),
            note = ("next 7 days: %s · %s"):format(Units.rain(b.rainNext), Units.rain(b.lostNext)) }
    end
    rows[#rows + 1] = Water.metric(hour, plot)
    local soil = Soil.state(forecast)
    rows[#rows + 1] = { label = "soil at 6 cm", value = Units.temp(soil and soil.today or hour.soil_temperature_6cm),
        note = soil and (Soil.trendText(soil.trend) or "daily mean") or "daily mean" }
    for _, job in ipairs(Jobs.list(forecast, plot)) do
        rows[#rows + 1] = { label = job.title, value = job.value, note = job.note, tone = job.tone }
    end
    rows[#rows + 1] = Protect.blightMetric(forecast)
    for _, row in ipairs(Climate.metrics(forecast, forecast.spot)) do rows[#rows + 1] = row end
    rows[#rows + 1] = Report.wind(hour)
    rows[#rows + 1] = Report.sun(hour)
    return rows
end

-- The two charts on the activity's "today" view, over the next 24 hours.
-- `value` is metric; `unit` (a Units kind) converts it and ends the title.
Garden.charts = {
    { title = "air", unit = "temp", kind = "line", value = function(hour) return hour.temp or 0 end },
    { title = "rain", unit = "rain", kind = "column", min = 0, value = function(hour) return hour.rain or 0 end },
}

return Garden
