-- Compare my waters (FI10): every place the angler fishes, ranked by its
-- best hours this morning (the rest of today after noon, tomorrow morning
-- after dark). Each place costs one forecast, which the provider caches for
-- ten minutes, so the comparison respects the call budget.
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Compare = {}

-- The date and the hours that count from `now` (a forecast hour): this
-- morning until noon, else the rest of today, else tomorrow morning.
function Compare.target(now)
    local hour = Sky.hour(now.time) or 0
    if hour < 12 then return now.date, "this morning", 12 end
    if hour < 18 then return now.date, "later today", 24 end
    local next = os.date("!%Y-%m-%d", (Sky.epochOf(now.date) or 0) + 86400)
    return next, "tomorrow morning", 12
end

-- One place's best stretch on a date before `untilHour`: the top hour's
-- result and the run around it within ten points, or nil.
function Compare.best(activity, forecast, date, untilHour)
    local now = forecast.hours[forecast.now]
    local rated = {}
    for _, hour in ipairs(forecast.hours) do
        local h = Sky.hour(hour.time)
        if hour.date == date and hour.time >= now.time and h and h < untilHour
            and (not activity.window or activity.window(hour, forecast)) then
            rated[#rated + 1] = { hour = hour, result = activity.rate(hour, forecast):result() }
        end
    end
    local top
    for index, entry in ipairs(rated) do
        if not top or entry.result.score > rated[top].result.score then top = index end
    end
    if not top then return nil end
    local floor, first, last = rated[top].result.score - 10, top, top
    while first > 1 and rated[first - 1].result.score >= floor do first = first - 1 end
    while last < #rated and rated[last + 1].result.score >= floor do last = last + 1 end
    return rated[top].result, Units.span(rated[first].hour.time, rated[last].hour.time), rated[top].hour
end

-- Every place offering `activity`, best first: { place, result, span, hour,
-- label } or { place, failed = message } when its forecast is missing.
-- `forecastFor(placeId)` returns the forecast or nil, err.
function Compare.rank(activity, places, forecastFor)
    local ranked, failed = {}, {}
    for _, place in ipairs(places) do
        local offers = false
        for _, id in ipairs(place.activities or {}) do offers = offers or id == activity.id end
        if offers then
            local forecast, err = forecastFor(place.id)
            local now = forecast and forecast.hours[forecast.now]
            if now then
                local date, label, untilHour = Compare.target(now)
                local result, span, hour = Compare.best(activity, forecast, date, untilHour)
                if result then
                    ranked[#ranked + 1] = { place = place, result = result, span = span, hour = hour, label = label }
                end
            else
                failed[#failed + 1] = { place = place, failed = type(err) == "table" and err.message or tostring(err or "") }
            end
        end
    end
    table.sort(ranked, function(a, b)
        if a.result.score ~= b.result.score then return a.result.score > b.result.score end
        return a.place.title < b.place.title
    end)
    for _, entry in ipairs(failed) do ranked[#ranked + 1] = entry end
    return ranked
end

return Compare
