-- Turns a forecast into what the pages show for one profile: the verdict
-- now and why, the next hours, the days with their best window, and the
-- profile's own conditions. Every row is a WeatherRow (Data/Weather.Schema.xml)
-- told apart by Kind, so one request feeds every panorama section.
local Sky = require "WeatherKit/sky"
local Score = require "WeatherKit/score"
local Report = {}

local HOURS_SHOWN = 24
local REASONS_SHOWN = 4

local function rate(profile, hour, forecast)
    local result = profile.rate(hour, forecast):result()
    local labels = profile.labels or {}
    result.word = labels[result.verdict] or result.verdict
    return result
end

local function row(rows, spot, kind, id, fields)
    fields.id = spot.id .. ":" .. kind .. ":" .. id
    fields.kind = kind
    fields.order = #rows + 1
    rows[#rows + 1] = fields
    return fields
end

-- The best stretch of a day: the run of hours around its top score that stay
-- within ten points of it.
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

function Report.rows(profile, spot, forecast)
    local rows, hours = {}, forecast.hours
    local now = hours[forecast.now]
    if not now then return rows end
    -- The verdict card shows the current hour, or the hour the profile
    -- cares about next (tonight's first dark hour for stargazing).
    local focusIndex = profile.focus and profile.focus(forecast) or forecast.now
    local focus = hours[focusIndex] or now
    local result = rate(profile, focus, forecast)
    row(rows, spot, "now", "now", {
        title = result.word, value = Score.fmt(focus.temp, "°"), score = result.score, tone = result.verdict,
        icon = Sky.icon(focus.code, focus.isDay),
        detail = Sky.describe(focus.code) .. " · feels like " .. Score.fmt(focus.feels, "°"),
        note = focus == now and spot.title or spot.title .. " · from " .. Sky.clock(focus.time),
    })
    for index = 1, math.min(REASONS_SHOWN, #result.reasons) do
        local reason = result.reasons[index]
        row(rows, spot, "reason", tostring(index), { title = reason.text, tone = reason.tone, score = 0 })
    end
    for index = forecast.now, math.min(#hours, forecast.now + HOURS_SHOWN - 1) do
        local hour = hours[index]
        local rated = rate(profile, hour, forecast)
        row(rows, spot, "hour", hour.time, {
            title = index == forecast.now and "now" or Sky.clock(hour.time), value = rated.word,
            detail = profile.note(hour, forecast), score = rated.score, tone = rated.verdict, icon = Sky.icon(hour.code, hour.isDay),
        })
    end
    local today = now.date
    for _, day in ipairs(forecast.days) do
        local rated = {}
        for _, hour in ipairs(hours) do
            if hour.date == day.date and (day.date ~= today or hour.time >= now.time)
                and (not profile.window or profile.window(hour, forecast)) then
                rated[#rated + 1] = { hour = hour, result = rate(profile, hour, forecast) }
            end
        end
        local best, from, to = bestWindow(rated)
        if best then
            local span = from == to and Sky.clock(from.time)
                or Sky.clock(from.time):sub(1, 2) .. "–" .. Sky.clock(to.time):sub(1, 2) .. "h"
            row(rows, spot, "day", day.date, {
                title = day.date == today and "today" or Sky.weekday(day.date), value = best.result.word,
                detail = ("best %s · %s / %s · %s"):format(span, Score.fmt(day.max, "°"), Score.fmt(day.min, "°"),
                    Sky.describe(day.code)),
                score = best.result.score, tone = best.result.verdict, icon = Sky.icon(day.code),
            })
        end
    end
    for index, metric in ipairs(profile.metrics(focus, forecast)) do
        row(rows, spot, "metric", tostring(index), {
            title = metric.label, value = metric.value, detail = metric.note or "", tone = metric.tone or "info", score = 0,
        })
    end
    return rows
end

-- Conditions most profiles show: pressure and its trend, sun times, the moon.
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
