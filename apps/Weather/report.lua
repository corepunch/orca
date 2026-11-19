-- Turns a place's forecast into what the pages show. Every row is a
-- WeatherRow (Data/Weather.Schema.xml) told apart by Kind, so one request
-- feeds every section of a page:
--
--   hub (Report.hub)        now, detail, strip, activity ("good for"), hour, day
--   activity (Report.rows)  now, reason, hour, day, metric
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Report = {}

local HOURS_SHOWN = 24
local REASONS_SHOWN = 4
local STRIP_DAYS = 5

local function rate(activity, hour, forecast)
    local result = activity.rate(hour, forecast):result()
    local labels = activity.labels or {}
    result.word = labels[result.verdict] or result.verdict
    return result
end
Report.rate = rate

local function adder(rows, prefix)
    return function(kind, id, fields)
        fields.id = prefix .. ":" .. kind .. ":" .. id
        fields.kind = kind
        fields.order = #rows + 1
        rows[#rows + 1] = fields
        return fields
    end
end

-- The best stretch of a list of rated hours: the run around the top score
-- that stays within ten points of it.
local function bestWindow(rated)
    local top
    for index, entry in ipairs(rated) do
        if not top or entry.result.score > rated[top].result.score then top = index end
    end
    if not top then return nil end
    local floor = rated[top].result.score - 10
    local first, last = top, top
    while first > 1 and rated[first - 1].result.score >= floor
        and Sky.hour(rated[first - 1].hour.time) == Sky.hour(rated[first].hour.time) - 1 do first = first - 1 end
    while last < #rated and rated[last + 1].result.score >= floor
        and Sky.hour(rated[last + 1].hour.time) == Sky.hour(rated[last].hour.time) + 1 do last = last + 1 end
    return rated[top], rated[first].hour, rated[last].hour
end

-- An activity's best window on a date, from now on: the top-rated hour's
-- result and the window as text ("06–08h"), or nil when no hour counts.
function Report.bestOfDay(activity, forecast, date)
    local now, rated = forecast.hours[forecast.now], {}
    for _, hour in ipairs(forecast.hours) do
        if hour.date == date and hour.time >= now.time and (not activity.window or activity.window(hour, forecast)) then
            rated[#rated + 1] = { hour = hour, result = rate(activity, hour, forecast) }
        end
    end
    local best, from, to = bestWindow(rated)
    if not best then return nil end
    local span = from == to and Sky.clock(from.time)
        or Sky.clock(from.time):sub(1, 2) .. "–" .. Sky.clock(to.time):sub(1, 2) .. "h"
    return best.result, span, best.hour
end

-- The activity page: the verdict now and why, the next hours, the days with
-- their best window, and the activity's own conditions.
function Report.rows(activity, place, forecast)
    local rows, hours = {}, forecast.hours
    local add = adder(rows, place.id .. ":" .. activity.id)
    local now = hours[forecast.now]
    if not now then return rows end
    -- The verdict card shows the current hour, or the hour the activity
    -- cares about next (tonight's first dark hour for stargazing).
    local focus = hours[activity.focus and activity.focus(forecast) or forecast.now] or now
    local result = rate(activity, focus, forecast)
    add("now", "now", {
        title = result.word, value = Score.fmt(focus.temp, "°"), score = result.score, tone = result.verdict,
        icon = Sky.icon(focus.code, focus.isDay),
        detail = Sky.describe(focus.code) .. " · feels like " .. Score.fmt(focus.feels, "°"),
        note = focus == now and place.title or place.title .. " · from " .. Sky.clock(focus.time),
    })
    for index = 1, math.min(REASONS_SHOWN, #result.reasons) do
        local reason = result.reasons[index]
        add("reason", tostring(index), { title = reason.text, tone = reason.tone, score = 0 })
    end
    for index = forecast.now, math.min(#hours, forecast.now + HOURS_SHOWN - 1) do
        local hour = hours[index]
        local rated = rate(activity, hour, forecast)
        local charts = activity.charts or {}
        add("hour", hour.time, {
            title = index == forecast.now and "now" or Sky.clock(hour.time), value = rated.word,
            detail = activity.note(hour, forecast), score = rated.score, tone = rated.verdict,
            icon = Sky.icon(hour.code, hour.isDay), short = Report.tick(hour),
            metric1 = charts[1] and charts[1].value(hour, forecast) or 0,
            metric2 = charts[2] and charts[2].value(hour, forecast) or 0,
        })
    end
    for _, day in ipairs(forecast.days) do
        local best, span = Report.bestOfDay(activity, forecast, day.date)
        if best then
            add("day", day.date, {
                title = day.date == now.date and "today" or Sky.weekday(day.date), value = best.word,
                detail = ("best %s · %s / %s · %s"):format(span, Score.fmt(day.max, "°"), Score.fmt(day.min, "°"),
                    Sky.describe(day.code)),
                score = best.score, tone = best.verdict, icon = Sky.icon(day.code),
            })
        end
    end
    for index, metric in ipairs(activity.metrics(focus, forecast)) do
        add("metric", tostring(index), {
            title = metric.label, value = metric.value, detail = metric.note or "", tone = metric.tone or "info", score = 0,
        })
    end
    return rows
end

-- The place hub, after Amazing Weather's "now" page and The Weather
-- Channel's activity cards: conditions, details, a five-day strip, what the
-- place is good for, the next hours and the week.
function Report.hub(place, forecast, activities)
    local rows, hours = {}, forecast.hours
    local add = adder(rows, place.id)
    local now = hours[forecast.now]
    if not now then return rows end
    local today = now.date
    add("now", "now", {
        title = Sky.describe(now.code), value = Score.fmt(now.temp, "°"), icon = Sky.icon(now.code, now.isDay),
        detail = "feels like " .. Score.fmt(now.feels, "°"),
        note = ("wind %s %s · uv %s"):format(Score.fmt(now.wind, " m/s"), Sky.compass(now.windDir), Score.fmt(now.uv)),
        score = 0, tone = "info",
    })
    local day = now.day or {}
    local moon = Sky.moon(forecast.epoch)
    local details = {
        { "humidity", Score.fmt(now.humidity, "%") }, { "pressure", Score.fmt(now.pressure, " hPa") },
        { "visibility", Score.fmt(now.visibility and now.visibility / 1000, " km") },
        { "precipitation", Score.fmt(now.rain, " mm", 1) }, { "dew point", Score.fmt(now.dewPoint, "°") },
        { "sunrise", Sky.clock(day.sunrise) }, { "sunset", Sky.clock(day.sunset) },
        { "moon", moon.name },
    }
    for index, detail in ipairs(details) do
        add("detail", tostring(index), { title = detail[1], value = detail[2], score = 0, tone = "info" })
    end
    for index = 1, math.min(STRIP_DAYS, #forecast.days) do
        local d = forecast.days[index]
        add("strip", d.date, {
            title = Sky.weekday(d.date):sub(1, 3), value = Score.fmt(d.max, "°"), detail = Score.fmt(d.min, "°"),
            note = Score.fmt(d.rainChance, "%"), icon = Sky.icon(d.code), score = 0, tone = "info",
        })
    end
    -- Good for: each activity's best window today, or tomorrow once today's
    -- has passed, with the reason that matters most then.
    local tomorrow = forecast.days[2] and forecast.days[2].date
    local bestPerDay = {}
    for _, activity in ipairs(activities) do
        local best, span, hour = Report.bestOfDay(activity, forecast, today)
        local when = ""
        if not best and tomorrow then
            best, span, hour = Report.bestOfDay(activity, forecast, tomorrow)
            when = "tomorrow "
        end
        if best then
            local reason = best.reasons[1]
            add("activity", activity.id, {
                title = activity.title, value = best.word, activity = activity.id, score = best.score, tone = best.verdict,
                detail = ("best %s%s%s"):format(when, span, reason and " · " .. reason.text or ""),
                icon = Sky.icon(hour.code, hour.isDay),
            })
        end
    end
    for index = forecast.now, math.min(#hours, forecast.now + HOURS_SHOWN - 1) do
        local hour = hours[index]
        add("hour", hour.time, {
            short = Report.tick(hour), temp = hour.temp or 0, wind = hour.wind or 0, rain_chance = hour.rainChance or 0,
            title = index == forecast.now and "now" or Sky.clock(hour.time), value = Score.fmt(hour.temp, "°"),
            detail = ("%s · rain %s · wind %s"):format(Sky.describe(hour.code), Score.fmt(hour.rainChance, "%"),
                Score.fmt(hour.wind, " m/s")),
            icon = Sky.icon(hour.code, hour.isDay), score = 0, tone = "info",
        })
    end
    -- The week: each day's best activity and window.
    for _, d in ipairs(forecast.days) do
        local top
        for _, activity in ipairs(activities) do
            local best, span = Report.bestOfDay(activity, forecast, d.date)
            if best and (not top or best.score > top.best.score) then top = { activity = activity, best = best, span = span } end
        end
        bestPerDay[d.date] = top
        add("day", d.date, {
            title = d.date == today and "today" or Sky.weekday(d.date),
            value = Score.fmt(d.max, "°") .. " / " .. Score.fmt(d.min, "°"),
            detail = top and ("%s for %s · %s"):format(top.best.word, top.activity.title, top.span) or Sky.describe(d.code),
            score = top and top.best.score or 0, tone = top and top.best.verdict or "info", icon = Sky.icon(d.code),
        })
    end
    return rows
end

-- The hour as a chart tick ("06") every three hours, empty between.
function Report.tick(hour)
    local h = Sky.hour(hour.time)
    return h and h % 3 == 0 and hour.time:sub(12, 13) or ""
end

-- Conditions most activities show: pressure and its trend, sun times, the moon.
function Report.pressure(hour)
    local trend = hour.pressureTrend or 0
    local arrow = trend > 0.5 and " rising" or trend < -0.5 and " falling" or " steady"
    return { label = "pressure", value = Score.fmt(hour.pressure, " hPa"),
        note = arrow:sub(2) .. " (" .. Score.fmt(trend, " hPa", 1) .. " in 3 h)" }
end

function Report.sun(hour)
    local day = hour.day or {}
    return { label = "sunrise · sunset", value = Sky.clock(day.sunrise) .. " · " .. Sky.clock(day.sunset) }
end

function Report.moon(forecast)
    local moon = Sky.moon(forecast.epoch)
    return { label = "moon", value = moon.name, note = Score.fmt(moon.lit * 100, "% lit") }
end

function Report.wind(hour)
    return { label = "wind", value = Score.fmt(hour.wind, " m/s") .. " " .. Sky.compass(hour.windDir),
        note = "gusts " .. Score.fmt(hour.gust, " m/s") }
end

return Report
