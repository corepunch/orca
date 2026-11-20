-- One night as a stargazer counts it: from sunset to sunrise, across
-- midnight, with the stretch of full darkness inside it, the moon's rise
-- and set, and the dark, moon-free windows. Times come from the sun's and
-- moon's real positions every ten minutes, so they hold at any latitude:
-- a June night in Wales has no full darkness, a Lofoten January has no day.
local Sky = require "Weather/sky"
local Moon = require "Weather/editions/stargazing/moon"
local Night = {}

local STEP = 600
-- The sun's height at each edge: sunset, and full (astronomical) darkness.
Night.SUNSET, Night.DARK = Sky.TWILIGHT.sunrise, Sky.TWILIGHT.astronomical

-- How dark the sky gets with the sun at a height: the twilight it is in.
function Night.level(altitude)
    if altitude > Sky.TWILIGHT.sunrise then return "day" end
    if altitude > Sky.TWILIGHT.civil then return "civil" end
    if altitude > Sky.TWILIGHT.nautical then return "nautical" end
    if altitude > Sky.TWILIGHT.astronomical then return "astronomical" end
    return "night"
end

-- Local clock text for a UTC epoch.
function Night.iso(epoch, offset)
    return epoch and os.date("!%Y-%m-%dT%H:%M", math.floor(epoch + (offset or 0) + 30))
end

-- The first time after `from` (before, when `step` is negative) that
-- `margin` changes sign, within `limit` seconds; linear between samples.
local function crossing(from, step, limit, margin)
    local previous = margin(from)
    for index = 1, limit // math.abs(step) do
        local epoch = from + index * step
        local value = margin(epoch)
        if (previous > 0) ~= (value > 0) then return epoch - step + step * previous / (previous - value) end
        previous = value
    end
end

-- The stretches of [from, to) where `margin` is positive, as { from, to }.
local function spans(from, to, margin)
    local list, open, previous = {}, nil, nil
    local epoch = from
    while epoch <= to do
        local value = margin(epoch)
        if previous ~= nil and (previous > 0) ~= (value > 0) then
            local edge = epoch - STEP + STEP * previous / (previous - value)
            if value > 0 then open = edge else list[#list + 1] = { from = open or from, to = edge }; open = nil end
        elseif previous == nil and value > 0 then open = from end
        previous = value
        epoch = epoch + STEP
    end
    if open then list[#list + 1] = { from = open, to = to } end
    return list
end
Night.spans = spans

local frames = {}

-- The night going on at `epoch`, or the next one, at a spot:
--
--   { from, to, sunset, sunrise,            sunset to sunrise (epochs, local text)
--     polar = nil | "day" | "night",       "day": no night at all; "night": no sunrise within a day
--     darkest, level,                      the sun's lowest height and the twilight that leaves
--     dark = { from, to } | nil,           full darkness (sun 18° down), first to last
--     moonFree = { { from, to }, … },      dark with the moon below the horizon
--     moonRise, moonSet,                   epochs inside the night, if it rises or sets
--     moonUp = "all" | "none" | nil,       the moon's place all night, when it neither rises nor sets
--     moonHigh,                            its greatest height in degrees
--     moon = Moon.phase at the night's middle }
function Night.frame(spot, epoch, offset)
    offset = offset or 0
    local key = ("%.3f|%.3f|%d|%d"):format(spot.lat, spot.lon, offset, epoch // STEP)
    if frames[key] then return frames[key] end
    local function sunDown(e) return Night.SUNSET - Sky.sunAltitude(e, spot.lat, spot.lon) end
    local function moonDown(e) return Moon.HORIZON - Moon.altitude(e, spot.lat, spot.lon) end
    local frame = {}
    local from, to
    if sunDown(epoch) > 0 then
        from = crossing(epoch, -STEP, 86400, sunDown)
        if not from then from, frame.polar = epoch, "night" end
    else
        from = crossing(epoch, STEP, 26 * 3600, sunDown)
        if not from then
            frame.polar = "day"
            frames[key] = frame
            return frame
        end
    end
    to = crossing(from + 60, STEP, 86400, sunDown)
    if not to then to, frame.polar = from + 86400, "night" end
    frame.from, frame.to, frame.offset = from, to, offset
    frame.sunset = not (frame.polar == "night" and from == epoch) and Night.iso(from, offset) or nil
    frame.sunrise = frame.polar ~= "night" and Night.iso(to, offset) or nil

    local darkest, moonHigh = math.huge, -90
    local e = from
    while e <= to do
        darkest = math.min(darkest, Sky.sunAltitude(e, spot.lat, spot.lon))
        moonHigh = math.max(moonHigh, Moon.altitude(e, spot.lat, spot.lon))
        e = e + STEP
    end
    frame.darkest, frame.level, frame.moonHigh = darkest, Night.level(darkest), moonHigh

    local function dark(e2) return Sky.sunAltitude(e2, spot.lat, spot.lon) - Night.DARK end
    local darkSpans = spans(from, to, function(e2) return -dark(e2) end)
    if #darkSpans > 0 then frame.dark = { from = darkSpans[1].from, to = darkSpans[#darkSpans].to } end
    frame.moonFree = spans(from, to, function(e2) return math.min(-dark(e2), moonDown(e2)) end)

    local up = spans(from, to, function(e2) return -moonDown(e2) end)
    for _, span in ipairs(up) do
        if span.from > from then frame.moonRise = frame.moonRise or span.from end
        if span.to < to then frame.moonSet = frame.moonSet or span.to end
    end
    if #up == 0 then frame.moonUp = "none"
    elseif #up == 1 and up[1].from == from and up[1].to == to then frame.moonUp = "all" end
    frame.moon = Moon.phase((from + to) / 2)
    frames[key] = frame
    return frame
end

-- The forecast hours that overlap a night, from the hour holding `now` on.
function Night.hours(forecast, frame, now)
    local list = {}
    if not frame.from then return list end
    local start = math.max(frame.from, now or frame.from)
    for _, hour in ipairs(forecast.hours) do
        if hour.epoch and hour.epoch + 3600 > start and hour.epoch < frame.to then list[#list + 1] = hour end
    end
    return list
end

-- Tonight at the forecast's place: the night going on now or the next one.
function Night.tonight(forecast)
    local spot = forecast.spot
    if not (spot and spot.lat and spot.lon and forecast.epoch) then return nil end
    return Night.frame(spot, forecast.epoch, forecast.offset)
end

-- The nights the forecast covers, from tonight on: one frame per sunset.
function Night.week(forecast)
    local first = Night.tonight(forecast)
    if not first then return {} end
    local last = forecast.hours[#forecast.hours]
    local nights, frame = {}, first
    while frame and frame.from and last and frame.from < last.epoch do
        nights[#nights + 1] = frame
        local nextFrom = frame.to + 3600
        frame = Night.frame(forecast.spot, nextFrom, forecast.offset)
        if frame.polar == "day" or not frame.from or frame.from <= nights[#nights].from then break end
    end
    return nights
end

return Night
