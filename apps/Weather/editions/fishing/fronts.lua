-- Pressure history and fronts for Fishers' Weather (FI2). The forecast asks
-- for two past days, so the barometer reads 48 hours back and the week
-- ahead. A front shows as a trough: pressure falls, bottoms out and climbs
-- again, usually with the wind swinging round or the air turning colder.
-- What anglers make of it is lore; the trough itself is measured.
local Sky = require "Weather/sky"
local Fronts = {}

local DROP, RISE = 2.5, 2      -- hPa a trough must fall before and climb after, within 12 h
local SHIFT, CHILL = 45, 2     -- a wind swing (degrees) or a fall in air temperature (°C) that marks a front
local cache = setmetatable({}, { __mode = "k" })

local function memo(forecast)
    local entry = cache[forecast]
    if not entry then
        entry = { index = {} }
        for index, hour in ipairs(forecast.hours or {}) do entry.index[hour] = index end
        cache[forecast] = entry
    end
    return entry
end

-- The hour's position in forecast.hours, or nil for a hand-built hour.
function Fronts.indexOf(forecast, hour) return forecast and forecast.hours and memo(forecast).index[hour] end

-- Pressure change over the `hours` before an hour, hPa; nil without data.
function Fronts.change(forecast, hour, hours)
    local index = Fronts.indexOf(forecast, hour)
    local before = index and forecast.hours[index - hours]
    if not (before and before.pressure and hour.pressure) then return nil end
    return hour.pressure - before.pressure
end

local function meanDirection(hours, from, to)
    local x, y, n = 0, 0, 0
    for index = math.max(1, from), math.min(#hours, to) do
        local dir = hours[index].windDir
        if dir then x, y, n = x + math.cos(math.rad(dir)), y + math.sin(math.rad(dir)), n + 1 end
    end
    return n > 0 and (math.deg(math.atan(y, x)) % 360) or nil
end

local function meanTemp(hours, from, to)
    local sum, n = 0, 0
    for index = math.max(1, from), math.min(#hours, to) do
        if hours[index].temp then sum, n = sum + hours[index].temp, n + 1 end
    end
    return n > 0 and sum / n or nil
end

-- Every trough in the forecast, oldest first: { index, hour, kind = "front"
-- (the wind swings or the air cools) or "low", drop, rise, from, to (the
-- wind's compass points before and after) }.
function Fronts.list(forecast)
    local entry = memo(forecast)
    if entry.fronts then return entry.fronts end
    local hours, fronts = forecast.hours or {}, {}
    for index, hour in ipairs(hours) do
        local p = hour.pressure
        local lowest = p ~= nil
        for other = math.max(1, index - 6), math.min(#hours, index + 6) do
            local q = hours[other].pressure
            if lowest and q and (q < p or (q == p and other < index)) then lowest = false end
        end
        if lowest then
            local before, after = p, p
            for other = math.max(1, index - 12), index do before = math.max(before, hours[other].pressure or p) end
            for other = index, math.min(#hours, index + 12) do after = math.max(after, hours[other].pressure or p) end
            if before - p >= DROP and after - p >= RISE then
                local dirBefore, dirAfter = meanDirection(hours, index - 6, index - 1), meanDirection(hours, index + 1, index + 6)
                local tempBefore, tempAfter = meanTemp(hours, index - 6, index - 1), meanTemp(hours, index + 1, index + 6)
                local swung = dirBefore and dirAfter and Sky.angle(dirBefore, dirAfter) >= SHIFT
                local chilled = tempBefore and tempAfter and tempBefore - tempAfter >= CHILL
                fronts[#fronts + 1] = { index = index, hour = hour, kind = (swung or chilled) and "front" or "low",
                    drop = before - p, rise = after - p, from = dirBefore and Sky.compass(dirBefore),
                    to = dirAfter and Sky.compass(dirAfter) }
            end
        end
    end
    entry.fronts = fronts
    return fronts
end

-- The trough nearest an hour on either side: the last one before it (within
-- `back` hours) and the next one after it (within `ahead` hours).
function Fronts.around(forecast, hour, back, ahead)
    local index = Fronts.indexOf(forecast, hour)
    if not index then return nil, nil end
    local last, next
    for _, front in ipairs(Fronts.list(forecast)) do
        if front.index <= index and index - front.index <= back then last = front end
        if front.index > index and front.index - index <= ahead and not next then next = front end
    end
    return last, next
end

-- Hours since a past trough became the hour (0 at the trough itself).
function Fronts.since(forecast, hour, front) return Fronts.indexOf(forecast, hour) - front.index end

return Fronts
