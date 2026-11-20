-- Tides from Open-Meteo's marine sea level (`sea_level_height_msl`, SU5).
-- The model runs at about 8 km and is "not suitable for coastal
-- navigation" (PLAN.md 0.4): good enough for the shape of the day and the
-- times of high and low water, rough on heights. The screens call it
-- "approximate" for that reason.
local Tides = {}

Tides.COLUMN = "sea_level_height_msl"
local MIN_RANGE = 0.1 -- metres; smaller wiggles are noise, not a tide

local cache = setmetatable({}, { __mode = "k" })

local function at(epoch, offset, format) return os.date("!" .. format, math.floor(epoch + (offset or 0) + 30)) end

-- Highs and lows in a forecast's hours, each refined between its hourly
-- neighbours: { kind = "high" | "low", height, epoch, time (local ISO), date }.
local function extremes(forecast)
    local hours, list = forecast.hours or {}, {}
    -- Where the sea barely moves (an enclosed sea, or no data) there is no tide.
    local low, high = math.huge, -math.huge
    for _, hour in ipairs(hours) do
        local height = hour[Tides.COLUMN]
        if height then low, high = math.min(low, height), math.max(high, height) end
    end
    if high - low < 2 * MIN_RANGE then return list end
    for index = 2, #hours - 1 do
        local a, b, c = hours[index - 1][Tides.COLUMN], hours[index][Tides.COLUMN], hours[index + 1][Tides.COLUMN]
        if a and b and c and hours[index].epoch then
            local kind = (b > a and b >= c) and "high" or (b < a and b <= c) and "low" or nil
            if kind then
                -- The top of the parabola through the three hours.
                local bend, shift = a - 2 * b + c, 0
                if bend ~= 0 then shift = math.max(-0.5, math.min(0.5, 0.5 * (a - c) / bend)) end
                local epoch = hours[index].epoch + shift * 3600
                list[#list + 1] = { kind = kind, height = b - 0.25 * (a - c) * shift, epoch = epoch,
                    time = at(epoch, forecast.offset, "%Y-%m-%dT%H:%M"), date = at(epoch, forecast.offset, "%Y-%m-%d") }
            end
        end
    end
    -- A high and a low closer than MIN_RANGE are a wobble: drop both.
    local kept = {}
    for _, extreme in ipairs(list) do
        local last = kept[#kept]
        if last and last.kind == extreme.kind then
            if (extreme.kind == "high") == (extreme.height > last.height) then kept[#kept] = extreme end
        elseif last and math.abs(last.height - extreme.height) < MIN_RANGE then
            kept[#kept] = nil
        else
            kept[#kept + 1] = extreme
        end
    end
    return kept
end

local function lookup(forecast)
    local entry = cache[forecast]
    if entry and entry.hours == forecast.hours then return entry end
    entry = { hours = forecast.hours, extremes = extremes(forecast), states = {} }
    cache[forecast] = entry
    return entry
end

-- Every high and low in the forecast.
function Tides.extremes(forecast) return lookup(forecast).extremes end

-- The tide at an hour: { level = 0 (low) … 1 (high), rising, height, range,
-- word = "low" | "mid" | "high", next = the next extreme }, or nil without
-- sea-level data or a high and low around it.
function Tides.at(forecast, hour)
    local height = hour[Tides.COLUMN]
    if height == nil or not hour.epoch then return nil end
    local entry = lookup(forecast)
    local known = entry.states[hour.time]
    if known ~= nil then return known or nil end
    local before, after
    for _, extreme in ipairs(entry.extremes) do
        if extreme.epoch <= hour.epoch then before = extreme
        elseif not after then after = extreme end
    end
    local state = false
    if before and after and before.kind ~= after.kind then
        local low, high = before.kind == "low" and before or after, before.kind == "high" and before or after
        local range = high.height - low.height
        local level = math.max(0, math.min(1, (height - low.height) / range))
        state = { level = level, rising = after.kind == "high", height = height, range = range, next = after,
            word = level < 1 / 3 and "low" or level > 2 / 3 and "high" or "mid" }
    end
    entry.states[hour.time] = state
    return state or nil
end

-- "mid, rising"
function Tides.text(state)
    if not state then return "—" end
    return state.word .. ", " .. (state.rising and "rising" or "falling")
end

-- The highs and lows on a local date from `fromTime` on.
function Tides.onDate(forecast, date, fromTime)
    local list = {}
    for _, extreme in ipairs(Tides.extremes(forecast)) do
        if extreme.date == date and (not fromTime or extreme.time >= fromTime) then list[#list + 1] = extreme end
    end
    return list
end

return Tides
