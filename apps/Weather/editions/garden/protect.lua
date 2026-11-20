-- Protecting plants: potato and tomato blight (GA6), mildew, and heat,
-- gust, hail and snow warnings (GA7).
--
-- Blight, the Hutton criteria (James Hutton Institute, 2017, which replaced
-- the Smith period in the UK): a "Hutton period" is two days in a row,
-- each with a minimum of 10 °C or more and at least six hours of relative
-- humidity of 90 % or more. One such day, or two days with five humid
-- hours, is a near miss. Model humidity can run high, so this is a risk,
-- never a diagnosis.
--
-- Powdery mildew (courgettes, roses): warm days (20 °C or more) with humid
-- nights (four hours at 90 % or more) on two of the next three days. A
-- gardeners' rule of thumb, labelled "estimate".
local Units = require "Weather/units"
local Plot = require "Weather/editions/garden/plot"
local Protect = {}

local DAY = 86400

-- Each local day's minimum and humid hours, from yesterday on; days with
-- fewer than 20 hours are left out.
local function days(forecast)
    local now = Plot.nowEpoch(forecast)
    local list = {}
    for _, day in ipairs(Plot.byDate(Plot.between(forecast, now - 2 * DAY, now + 8 * DAY))) do
        if #day.hours >= 20 then
            local low, high, humid, humidNight = math.huge, -math.huge, 0, 0
            for _, hour in ipairs(day.hours) do
                if hour.temp then low, high = math.min(low, hour.temp), math.max(high, hour.temp) end
                if (hour.humidity or 0) >= 90 then
                    humid = humid + 1
                    if not hour.isDay then humidNight = humidNight + 1 end
                end
            end
            list[#list + 1] = { date = day.date, low = low, high = high, humid = humid, humidNight = humidNight }
        end
    end
    return list
end
Protect.days = days

-- The first Hutton period (or near miss) from yesterday on.
function Protect.blight(forecast)
    local list = days(forecast)
    local today = forecast.hours[forecast.now] and forecast.hours[forecast.now].date
    local near
    for index = 1, #list - 1 do
        local a, b = list[index], list[index + 1]
        if b.date >= (today or "") or a.date >= (today or "") then
            local fullA, fullB = a.low >= 10 and a.humid >= 6, b.low >= 10 and b.humid >= 6
            if fullA and fullB then return { level = 2, from = a.date, to = b.date } end
            if not near and a.low >= 10 and b.low >= 10 and a.humid >= 5 and b.humid >= 5 then
                near = { level = 1, from = a.date, to = b.date }
            elseif not near and (fullA or fullB) and (fullA and a.date or b.date) >= (today or "") then
                near = { level = 1, from = fullA and a.date or b.date }
            end
        end
    end
    return near or { level = 0 }
end

-- Powdery mildew weather in the next three days.
function Protect.mildew(forecast)
    local today = forecast.hours[forecast.now] and forecast.hours[forecast.now].date
    local count, first = 0, nil
    for _, day in ipairs(days(forecast)) do
        if day.date >= (today or "") and day.date <= Plot.addDays(today, 2) and day.high >= 20 and day.humidNight >= 4 then
            count, first = count + 1, first or day.date
        end
    end
    return count >= 2 and first or nil
end

-- Weather that hurts plants in the next 48 hours, worst first: each
-- { id, value, detail, date }.
function Protect.warnings(forecast)
    local now = Plot.nowEpoch(forecast)
    local rows = Plot.between(forecast, now, now + 2 * DAY)
    local hot, gust, hail, snow, snowFirst
    for _, hour in ipairs(rows) do
        if (hour.temp or 0) >= 30 and not hot then hot = hour end
        if (hour.gust or 0) >= 15 and (not gust or hour.gust > gust.gust) then gust = hour end
        if (hour.code == 96 or hour.code == 99) and not hail then hail = hour end
        if (hour.snowfall or 0) > 0 then snow = (snow or 0) + hour.snowfall; snowFirst = snowFirst or hour end
    end
    local list = {}
    local function at(hour) return Plot.dayName(hour.date, forecast) .. " " .. Units.clock(hour.time) end
    if hail then list[#list + 1] = { id = "hail", value = "hail " .. at(hail), date = hail.date,
        detail = "hail with thunder: cover what you can, prop cloches and cold-frame lids shut" } end
    if gust then list[#list + 1] = { id = "gust", value = "gusts " .. Units.wind(gust.gust), date = gust.date,
        detail = at(gust) .. ": stake tall plants, weigh down fleece and netting, close the cold frame" } end
    if snow and snow >= 5 then list[#list + 1] = { id = "snow", value = ("snow %d cm"):format(math.floor(snow + 0.5)),
        date = snowFirst.date, detail = "from " .. at(snowFirst) .. ": brush snow off fleece, netting and greenhouse roofs" } end
    if hot then list[#list + 1] = { id = "heat", value = "heat " .. Units.temp(hot.temp), date = hot.date,
        detail = at(hot) .. ": shade the greenhouse, water pots in the evening, move them out of the sun" } end
    return list
end

-- Cards: plant warnings (when any) and blight or mildew (when there is a risk).
function Protect.cards(forecast)
    local cards = {}
    local warnings = Protect.warnings(forecast)
    if #warnings > 0 then
        local lead, others = warnings[1], {}
        for index = 2, #warnings do others[#others + 1] = warnings[index].value end
        cards[#cards + 1] = { id = "warn", title = "protect plants", value = lead.value, tone = "poor",
            detail = lead.detail .. (#others > 0 and " · also " .. table.concat(others, ", ") or "") }
    end
    local blight = Protect.blight(forecast)
    if blight.level > 0 then
        local span = blight.to and (Plot.dayName(blight.from, forecast) .. "–" .. Plot.dayName(blight.to, forecast))
            or Plot.dayName(blight.from, forecast)
        cards[#cards + 1] = { id = "blight", title = "blight risk", tone = blight.level == 2 and "poor" or "fair",
            value = blight.level == 2 and "high" or "watch",
            detail = (blight.level == 2 and "a Hutton period %s: warm, humid days blight spreads in · check potatoes "
                .. "and tomatoes, protect or spray now" or "near a Hutton period %s · check potato and tomato leaves")
                :format(span) .. " · risk from model humidity" }
    end
    local mildew = Protect.mildew(forecast)
    if mildew then
        cards[#cards + 1] = { id = "mildew", title = "mildew risk", value = "watch", tone = "fair",
            detail = ("warm days, damp nights from %s: water the soil, not the leaves; watch courgettes and roses · estimate")
                :format(Plot.dayName(mildew, forecast)) }
    end
    return cards
end

-- Blight as a metric row (always shown on the activity page).
function Protect.blightMetric(forecast)
    local blight = Protect.blight(forecast)
    return { label = "blight (Hutton)", value = blight.level == 2 and "high" or blight.level == 1 and "watch" or "low",
        tone = blight.level == 2 and "poor" or blight.level == 1 and "fair" or "good",
        note = ("two days at %s or more with 6 h at 90%% humidity · risk"):format(Units.temp(10)) }
end

return Protect
