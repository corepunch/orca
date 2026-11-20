-- The hiking edition's big answers, worked out from a forecast: the storm
-- clock (HI3), the daylight budget and turnaround (HI4), trail conditions
-- (HI5) and whether the models agree (HI6). Times are local minutes from
-- midnight; every heuristic is worded as an estimate where it is shown.
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Trail = {}

local DAY = 24 * 60

-- Minutes → "HH:MM" as the clock reads it.
function Trail.clock(minutes)
    minutes = math.floor(minutes) % DAY
    return Units.clock(("%02d:%02d"):format(minutes // 60, minutes % 60))
end

-- A length of time: "45 min", "3 h", "6 h 10".
function Trail.duration(minutes)
    minutes = math.max(0, math.floor(minutes + 0.5))
    if minutes < 60 then return minutes .. " min" end
    local h, m = minutes // 60, minutes % 60
    return m == 0 and h .. " h" or ("%d h %02d"):format(h, m)
end

-- Snow as hikers say it: centimetres, or inches where rain is in inches.
function Trail.snow(cm)
    if Units.current("rain") == "in" then return ("%d in"):format(math.floor(cm / 2.54 + 0.5)) end
    return ("%d cm"):format(math.floor(cm + 0.5))
end

function Trail.now(forecast) return forecast.hours[forecast.now] end
function Trail.nowMinutes(forecast) return (((forecast.epoch or 0) + (forecast.offset or 0)) // 60) % DAY end

local function dayOf(forecast, date)
    for _, day in ipairs(forecast.days or {}) do
        if day.date == date then return day end
    end
end

-- The next day after `date` in the forecast, or nil.
local function nextDay(forecast, date)
    for _, day in ipairs(forecast.days or {}) do
        if day.date > date then return day end
    end
end

-- First and last light (civil twilight) on a day: minutes, or nil for a
-- day with no edge. `always` when it never gets dark, `never` in polar night.
function Trail.light(day)
    local civil = day and day.twilight and day.twilight.civil
    if day and day.twilight and civil == nil then return { first = 0, last = DAY, always = true } end
    if not civil then
        local rise, set = Sky.minutes(day and day.sunrise), Sky.minutes(day and day.sunset)
        return rise and set and { first = rise, last = set } or nil
    end
    if civil.dark then return { never = true } end
    return { first = Sky.minutes(civil.dawn) or 0, last = Sky.minutes(civil.dusk) or DAY }
end

-- How likely thunder is in an hour (an estimate): "likely" with a storm
-- code or lots of unstable storm energy and showers about, "possible" with
-- some, else nil. CAPE is the energy, a negative lifted index the push.
function Trail.thunder(hour)
    if Sky.isStorm(hour.code) then return "likely" end
    local cape, lifted, rain = hour.cape or 0, hour.lifted_index, hour.rainChance or 0
    if cape >= 1000 and (lifted == nil or lifted <= -2) and rain >= 40 then return "likely" end
    if cape >= 500 and (lifted == nil or lifted <= 0) and rain >= 20 then return "possible" end
    return nil
end

-- The worse of an hour's own thunder and its top's.
local function thunderAt(hour)
    local a, b = Trail.thunder(hour), hour.top and Trail.thunder(hour.top)
    if a == "likely" or b == "likely" then return "likely" end
    return a or b
end

-- The day a hike is planned for: today while daylight is left, else tomorrow.
function Trail.planDay(forecast)
    local now = Trail.now(forecast)
    local today = dayOf(forecast, now.date)
    local light = Trail.light(today)
    if light and not light.never and Trail.nowMinutes(forecast) < light.last then return today, true end
    return nextDay(forecast, now.date) or today, false
end

-- The storm clock for a day: the first daylight hour from now on with
-- thunder likely or possible, and when to be off the summit (an hour
-- earlier). nil when no hour shows the signs.
function Trail.storm(forecast, day)
    local now = Trail.now(forecast)
    for index = forecast.now, #forecast.hours do
        local hour = forecast.hours[index]
        if hour.date > day.date then break end
        if hour.date == day.date and hour.isDay then
            local level = thunderAt(hour)
            if level then
                return { hour = hour, level = level, minutes = Sky.minutes(hour.time), now = hour == now,
                    offBy = Sky.minutes(hour.time) - 60 }
            end
        end
    end
end

local function hours(key)
    local h = tonumber(require("Weather/settings").get("hiking", key))
    return h or 0
end

-- The turnaround for the planned hike (settings: start time, time to the
-- top, time back down): the latest time to turn back so you are down by
-- last light and off the top before the storm clock. `late` when the top
-- comes after it.
function Trail.plan(forecast)
    local today = Trail.now(forecast).date
    local day = dayOf(forecast, today)
    local start, up, down = hours("start") * 60, hours("up") * 60, hours("down") * 60
    local top, back = start + up, start + up + down
    -- Today while the hike is still ahead or under way, else tomorrow.
    local light = Trail.light(day)
    if not light or light.never or Trail.nowMinutes(forecast) >= math.min(back, light.last) then
        day = nextDay(forecast, today)
        light = Trail.light(day)
    end
    if not (day and light) then return nil end
    local plan = { day = day, today = day.date == today, start = start, top = top, back = back, light = light,
        up = up, down = down }
    if light.never then plan.dark = true; return plan end
    local turn, why = light.last - down, "dark"
    local storm = Trail.storm(forecast, day)
    if storm and storm.offBy < turn then turn, why = storm.offBy, "storm" end
    plan.storm = storm
    -- Round down to a quarter hour: the times are estimates.
    plan.turn = turn - turn % 15
    plan.why = why
    plan.late = top > plan.turn
    plan.beforeLight = start < light.first
    if plan.today then plan.left = math.max(0, light.last - math.max(Trail.nowMinutes(forecast), light.first)) end
    return plan
end

-- What the trail is like: snow lying at the top, snow and rain of the last
-- three days (rain apart from snow), a freeze overnight and thaw later,
-- heat and UV today.
function Trail.conditions(forecast)
    local list, now = forecast.hours, forecast.now
    local result = { snowDepth = 0, snowfall = 0, rain24 = 0, rain72 = 0, uv = 0 }
    for index = math.max(1, now - 72), now do
        local hour = list[index]
        local at = hour.top or hour
        result.snowfall = result.snowfall + (at.snowfall or 0)
        local rain = math.max(0, (hour.rain or 0) - (hour.snowfall or 0) / 0.7)
        result.rain72 = result.rain72 + rain
        if index > now - 24 then result.rain24 = result.rain24 + rain end
    end
    for index = now, math.min(#list, now + 23) do
        local hour = list[index]
        local at = hour.top or hour
        result.snowDepth = math.max(result.snowDepth, at.snow_depth or 0)
        if hour.date == list[now].date then
            result.uv = math.max(result.uv, hour.uv or 0)
            if hour.wet_bulb_temperature_2m then
                result.wetBulb = math.max(result.wetBulb or -99, hour.wet_bulb_temperature_2m)
            end
            if hour.feels then result.hot = math.max(result.hot or -99, hour.feels) end
        end
    end
    -- Freeze and thaw at the top on the planned day: below zero in the
    -- small hours, above it later; the first thawing hour.
    local day = Trail.planDay(forecast)
    local frozen
    for _, hour in ipairs(list) do
        if day and hour.date == day.date then
            local temp = (hour.top or hour).temp
            local h = Sky.hour(hour.time)
            if temp and temp < 0 and h <= 8 then frozen = true end
            if frozen and temp and temp > 1 and h > 8 and not result.thaw then result.thaw = hour end
        end
    end
    return result
end

-- The models' word for each day ahead, as runs: { { word, from, to } }.
function Trail.confidence(forecast)
    local words = forecast.hiking and forecast.hiking.confidence
    if not words then return nil end
    local runs = {}
    for index = forecast.today or 1, #forecast.days do
        local day = forecast.days[index]
        local entry = words[day.date]
        if not entry then break end
        local last = runs[#runs]
        if last and last.word == entry.word then last.to = day.date
        else runs[#runs + 1] = { word = entry.word, from = day.date, to = day.date, models = entry.models } end
    end
    return #runs > 0 and runs or nil
end

-- Official warnings in force now or later (US places).
function Trail.alerts(forecast)
    local list = forecast.hiking and forecast.hiking.alerts
    if not list then return nil end
    local active = {}
    for _, alert in ipairs(list) do
        if not alert.ends or alert.ends > forecast.epoch then active[#active + 1] = alert end
    end
    local rank = { Extreme = 4, Severe = 3, Moderate = 2, Minor = 1 }
    table.sort(active, function(a, b) return (rank[a.severity] or 0) > (rank[b.severity] or 0) end)
    return active
end

return Trail
