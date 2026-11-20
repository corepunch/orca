-- Turns a place's forecast into what the pages show. Every row is a
-- WeatherRow (Data/Weather.Schema.xml) told apart by Kind, so one request
-- feeds every section of a page:
--
--   hub (Report.hub)        card, now, answer, age, detail, strip, activity ("good for"), hour, day
--   activity (Report.rows)  now, age, card, reason, hour, day, metric
--
-- Every number goes through Units, so the text follows the user's units.
local Sky = require "Weather/sky"
local Score = require "Weather/score"
local Units = require "Weather/units"
local Settings = require "Weather/settings"
local Report = {}

local HOURS_SHOWN = 24
local REASONS_SHOWN = 4
local STRIP_DAYS = 5
-- Beyond a week the models drift apart: later days say "less certain".
local CERTAIN_DAYS = 7
local function certainty(index) return index > CERTAIN_DAYS and "less certain · " or "" end
Report.certainty = certainty

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
    return best.result, Units.span(from.time, to.time), best.hour
end

-- The days from today on; a forecast with `pastDays` also holds earlier ones.
local function daysAhead(forecast)
    local days = {}
    for index = forecast.today or 1, #forecast.days do days[#days + 1] = forecast.days[index] end
    return days
end
Report.daysAhead = daysAhead

-- Now against this time yesterday ("4° colder than yesterday"), from a
-- forecast with `pastDays`; nil without yesterday's hour.
function Report.yesterday(forecast)
    local now, before = forecast.hours[forecast.now], forecast.hours[forecast.now - 24]
    if not (now and before and now.temp and before.temp) or before.date >= now.date then return nil end
    local change = now.temp - before.temp
    if math.abs(change) < 1.5 then return "about as warm as yesterday" end
    return ("%s %s than yesterday"):format(Units.tempDelta(math.abs(change)), change > 0 and "warmer" or "colder")
end

-- How old the forecast is, as a row: "updated 25 min ago", and a clear line
-- when it is the copy saved before the network went away.
function Report.age(forecast, now)
    local minutes = math.max(0, math.floor(((now or os.time()) - (forecast.updated or forecast.epoch or now)) / 60))
    local title = minutes < 1 and "updated just now" or minutes < 60 and ("updated %d min ago"):format(minutes)
        or minutes < 48 * 60 and ("updated %d h ago"):format(minutes // 60)
        or ("updated %d days ago"):format(minutes // 1440)
    local old = minutes >= 6 * 60
    local detail = forecast.stale and "offline: this is the last forecast saved on this device"
        or old and "this forecast is old: pull to refresh" or ""
    return { title = title, detail = detail, tone = forecast.stale and "poor" or old and "fair" or "info", score = 0 }
end

-- An activity's cards (`cards(forecast, place, clock)`), added as "card"
-- rows. `clock` is the time now; it may be nil.
local function addCards(add, activity, forecast, place, clock, kind)
    if not (activity and activity.cards) then return end
    for index, card in ipairs(activity.cards(forecast, place, clock) or {}) do
        add(kind or "card", tostring(card.id or index), {
            title = card.title or "", value = card.value or "", detail = card.detail or "", tone = card.tone or "info",
            icon = card.icon or "", activity = activity.id, score = 0,
        })
    end
end

-- A chart's value for an hour, in the user's units.
local function chartValue(chart, hour, forecast)
    if not chart then return 0 end
    return Units.value(chart.unit, chart.value(hour, forecast)) or 0
end
Report.chartValue = chartValue

-- The activity page: the verdict now and why, the next hours, the days with
-- their best window, and the activity's own conditions. `clock` is the time
-- for the age row.
function Report.rows(activity, place, forecast, clock)
    local rows, hours = {}, forecast.hours
    local add = adder(rows, place.id .. ":" .. activity.id)
    local now = hours[forecast.now]
    if not now then return rows end
    -- The verdict card shows the current hour, or the hour the activity
    -- cares about next (tonight's first dark hour for stargazing).
    local focus = hours[activity.focus and activity.focus(forecast) or forecast.now] or now
    local result = rate(activity, focus, forecast)
    add("now", "now", {
        title = result.word, value = Units.temp(focus.temp), score = result.score, tone = result.verdict,
        icon = Sky.icon(focus.code, focus.isDay),
        detail = Sky.describe(focus.code) .. " · feels like " .. Units.temp(focus.feels),
        note = focus == now and place.title or place.title .. " · from " .. Units.clock(focus.time),
    })
    add("age", "age", Report.age(forecast, clock))
    addCards(add, activity, forecast, place, clock)
    for index = 1, math.min(REASONS_SHOWN, #result.reasons) do
        local reason = result.reasons[index]
        add("reason", tostring(index), { title = reason.text, tone = reason.tone, score = 0 })
    end
    for index = forecast.now, math.min(#hours, forecast.now + HOURS_SHOWN - 1) do
        local hour = hours[index]
        local rated = rate(activity, hour, forecast)
        local charts = activity.charts or {}
        add("hour", hour.time, {
            title = index == forecast.now and "now" or Units.clock(hour.time), value = rated.word,
            detail = activity.note(hour, forecast), score = rated.score, tone = rated.verdict,
            icon = Sky.icon(hour.code, hour.isDay), short = Report.tick(hour),
            metric1 = chartValue(charts[1], hour, forecast), metric2 = chartValue(charts[2], hour, forecast),
        })
    end
    for index, day in ipairs(daysAhead(forecast)) do
        local best, span = Report.bestOfDay(activity, forecast, day.date)
        if best then
            add("day", day.date, {
                title = day.date == now.date and "today" or Sky.weekday(day.date), value = best.word,
                detail = ("%sbest %s · %s / %s · %s"):format(certainty(index), span, Units.temp(day.max),
                    Units.temp(day.min), Sky.describe(day.code)),
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
-- Channel's activity cards: the lead activity's cards (an edition's own
-- answers first), conditions, details, a five-day strip, what the place is
-- good for, the next hours and the week. `clock` is the time for the age
-- row; `lead` the activity whose cards come first, if the place offers it.
function Report.hub(place, forecast, activities, clock, lead)
    local rows, hours = {}, forecast.hours
    local add = adder(rows, place.id)
    local now = hours[forecast.now]
    if not now then return rows end
    local today = now.date
    -- An edition's own answers lead; in the full app everyday outdoors'
    -- (the next two hours, the day in one line, what to bring) follow the
    -- temperature as compact "answer" rows, so it stays on top.
    local everyday = lead and lead.id == "outdoors"
    for _, activity in ipairs(activities) do
        if activity == lead and not everyday then addCards(add, activity, forecast, place, clock) end
    end
    local yesterday = Report.yesterday(forecast)
    local gusts = (now.gust or 0) >= (now.wind or 0) + 3 and ", gusts " .. Units.wind(now.gust) or ""
    add("now", "now", {
        title = Sky.describe(now.code), value = Units.temp(now.temp), icon = Sky.icon(now.code, now.isDay),
        detail = "feels like " .. Units.temp(now.feels) .. (yesterday and " · " .. yesterday or ""),
        note = ("wind %s %s%s · uv %s"):format(Units.wind(now.wind), Sky.compass(now.windDir), gusts, Score.fmt(now.uv)),
        score = 0, tone = "info",
    })
    if everyday then addCards(add, lead, forecast, place, clock, "answer") end
    add("age", "age", Report.age(forecast, clock))
    local day = now.day or {}
    local moon = Sky.moon(forecast.epoch)
    local details = {
        { "humidity", Score.fmt(now.humidity, "%") }, { "pressure", Units.pressure(now.pressure) },
        { "visibility", Units.distance(now.visibility and now.visibility / 1000) },
        { "precipitation", Units.rain(now.rain) }, { "dew point", Units.temp(now.dewPoint) },
        { "sunrise", Units.clock(day.sunrise) }, { "sunset", Units.clock(day.sunset) },
        { "moon", moon.name },
    }
    if now.aqi then
        details[#details + 1] = { "air quality", ("%d %s"):format(math.floor(now.aqi + 0.5), now.aqiScale == "eu" and "EU AQI" or "US AQI") }
    end
    for index, detail in ipairs(details) do
        add("detail", tostring(index), { title = detail[1], value = detail[2], score = 0, tone = "info" })
    end
    local ahead = daysAhead(forecast)
    for index = 1, math.min(STRIP_DAYS, #ahead) do
        local d = ahead[index]
        add("strip", d.date, {
            title = Sky.weekday(d.date):sub(1, 3), value = Units.temp(d.max), detail = Units.temp(d.min),
            note = Score.fmt(d.rainChance, "%"), icon = Sky.icon(d.code), score = 0, tone = "info",
        })
    end
    -- Good for: each followed activity's best window today, or tomorrow
    -- once today's has passed, with the reason that matters most then; in
    -- hub order or, if the user chose it, the best first.
    activities = Report.followed(activities)
    local tomorrow = ahead[2] and ahead[2].date
    local bestPerDay, goodFor = {}, {}
    for _, activity in ipairs(activities) do
        local best, span, hour = Report.bestOfDay(activity, forecast, today)
        local when = ""
        if not best and tomorrow then
            best, span, hour = Report.bestOfDay(activity, forecast, tomorrow)
            when = "tomorrow "
        end
        if best then
            local reason = best.reasons[1]
            goodFor[#goodFor + 1] = { id = activity.id, rank = #goodFor + (when == "" and 0 or 1000), fields = {
                title = activity.title, value = best.word, activity = activity.id, score = best.score, tone = best.verdict,
                detail = ("best %s%s%s"):format(when, span, reason and " · " .. reason.text or ""),
                icon = Sky.icon(hour.code, hour.isDay),
            } }
        end
    end
    if Settings.get("outdoors", "order") == "best" then
        -- Today's windows before tomorrow's, then the higher score.
        table.sort(goodFor, function(a, b)
            if (a.rank >= 1000) ~= (b.rank >= 1000) then return a.rank < b.rank end
            if a.fields.score ~= b.fields.score then return a.fields.score > b.fields.score end
            return a.rank < b.rank
        end)
    end
    for _, entry in ipairs(goodFor) do add("activity", entry.id, entry.fields) end
    for index = forecast.now, math.min(#hours, forecast.now + HOURS_SHOWN - 1) do
        local hour = hours[index]
        add("hour", hour.time, {
            short = Report.tick(hour), temp = Units.value("temp", hour.temp or 0), wind = Units.value("wind", hour.wind or 0),
            rain_chance = hour.rainChance or 0,
            title = index == forecast.now and "now" or Units.clock(hour.time), value = Units.temp(hour.temp),
            detail = ("%s · rain %s · wind %s"):format(Sky.describe(hour.code), Score.fmt(hour.rainChance, "%"),
                Units.wind(hour.wind)),
            icon = Sky.icon(hour.code, hour.isDay), score = 0, tone = "info",
        })
    end
    -- The week: each day's best activity and window.
    for index, d in ipairs(ahead) do
        local top
        for _, activity in ipairs(activities) do
            local best, span = Report.bestOfDay(activity, forecast, d.date)
            if best and (not top or best.score > top.best.score) then top = { activity = activity, best = best, span = span } end
        end
        bestPerDay[d.date] = top
        add("day", d.date, {
            title = d.date == today and "today" or Sky.weekday(d.date),
            value = Units.temp(d.max) .. " / " .. Units.temp(d.min),
            detail = certainty(index) .. (top and ("%s for %s · %s"):format(top.best.word, top.activity.title, top.span)
                or Sky.describe(d.code)),
            score = top and top.best.score or 0, tone = top and top.best.verdict or "info", icon = Sky.icon(d.code),
        })
    end
    return rows
end

-- The activities the user follows (W7): an "outdoors" setting
-- `follow_<id>` switches each off; everyday outdoors always stays.
function Report.followed(activities)
    local list = {}
    for _, activity in ipairs(activities) do
        if activity.id == "outdoors" or Settings.get("outdoors", "follow_" .. activity.id) ~= false then
            list[#list + 1] = activity
        end
    end
    return list
end

-- One day of a place, opened from the hub's week: the sky, high and low,
-- the day in one line (an activity's `daySummary(forecast, date)`), its
-- readings, what each activity is good for then, and its hours.
function Report.day(place, forecast, date, activities)
    local rows = {}
    local add = adder(rows, place.id .. ":" .. date)
    local now = forecast.hours[forecast.now]
    local index, day
    for i, d in ipairs(daysAhead(forecast)) do
        if d.date == date then index, day = i, d end
    end
    if not (now and day) then return rows end
    activities = Report.followed(activities)
    add("now", "day", {
        title = Sky.describe(day.code), value = Units.temp(day.max) .. " / " .. Units.temp(day.min),
        icon = Sky.icon(day.code), detail = certainty(index) .. "rain " .. Score.fmt(day.rainChance, "%"),
        note = date == now.date and "today" or Sky.weekday(date), score = 0,
        tone = index > CERTAIN_DAYS and "fair" or "info",
    })
    for _, activity in ipairs(activities) do
        for i, card in ipairs(activity.daySummary and activity.daySummary(forecast, date) or {}) do
            add("card", tostring(card.id or i), { title = card.title or "", value = card.value or "",
                detail = card.detail or "", tone = card.tone or "info", activity = activity.id, score = 0 })
        end
    end
    local details = {
        { "rain", Units.rain(day.rain) .. (day.precipitation_hours and (" in %s h"):format(Score.fmt(day.precipitation_hours)) or "") },
        { "wind", Units.wind(day.windMax) .. (day.wind_gusts_10m_max and ", gusts " .. Units.wind(day.wind_gusts_10m_max) or "") },
        { "uv index", Score.fmt(day.uv_index_max) },
        { "sunrise", Units.clock(day.sunrise) }, { "sunset", Units.clock(day.sunset) },
    }
    for i, detail in ipairs(details) do
        add("detail", tostring(i), { title = detail[1], value = detail[2], score = 0, tone = "info" })
    end
    for _, activity in ipairs(activities) do
        local best, span, hour = Report.bestOfDay(activity, forecast, date)
        if best then
            local reason = best.reasons[1]
            add("activity", activity.id, {
                title = activity.title, value = best.word, activity = activity.id, score = best.score, tone = best.verdict,
                detail = ("best %s%s"):format(span, reason and " · " .. reason.text or ""),
                icon = Sky.icon(hour.code, hour.isDay),
            })
        end
    end
    for _, hour in ipairs(forecast.hours) do
        if hour.date == date then
            add("hour", hour.time, {
                short = Report.tick(hour), temp = Units.value("temp", hour.temp or 0),
                wind = Units.value("wind", hour.wind or 0), rain_chance = hour.rainChance or 0,
                title = Units.clock(hour.time), value = Units.temp(hour.temp),
                detail = ("%s · rain %s · wind %s"):format(Sky.describe(hour.code), Score.fmt(hour.rainChance, "%"),
                    Units.wind(hour.wind)),
                icon = Sky.icon(hour.code, hour.isDay), score = 0, tone = "info",
            })
        end
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
    return { label = "pressure", value = Units.pressure(hour.pressure),
        note = arrow:sub(2) .. " (" .. Units.pressure(trend, 1) .. " in 3 h)" }
end

function Report.sun(hour)
    local day = hour.day or {}
    return { label = "sunrise · sunset", value = Units.clock(day.sunrise) .. " · " .. Units.clock(day.sunset) }
end

function Report.moon(forecast)
    local moon = Sky.moon(forecast.epoch)
    return { label = "moon", value = moon.name, note = Score.fmt(moon.lit * 100, "% lit") }
end

function Report.wind(hour)
    return { label = "wind", value = Units.wind(hour.wind) .. " " .. Sky.compass(hour.windDir),
        note = "gusts " .. Units.wind(hour.gust) }
end

return Report
