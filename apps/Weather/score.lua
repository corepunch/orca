-- The scoring kit a profile rates an hour with. A card starts at a base
-- score; every factor adds or takes points and says why in plain words.
-- The verdict is good, fair or poor; a hard limit (lightning, dark, gale)
-- caps the score whatever else is going well.
local Sky = require "Weather/sky"
local Score = {}

local Card = {}
Card.__index = Card

function Score.card(base) return setmetatable({ score = base or 50, reasons = {}, limit = 100 }, Card) end

-- Adds points (negative takes them) with the reason a person would give.
function Card:add(points, text)
    if points == 0 then return self end
    self.score = self.score + points
    self.reasons[#self.reasons + 1] = { points = points, text = text, tone = points > 0 and "good" or "poor" }
    return self
end

-- Caps the score: nothing outweighs this reason.
function Card:cap(limit, text)
    if limit < self.limit then self.limit = limit end
    self.reasons[#self.reasons + 1] = { points = -1000, text = text, tone = "poor" }
    return self
end

-- A note that neither helps nor hurts.
function Card:note(text)
    self.reasons[#self.reasons + 1] = { points = 0, text = text, tone = "info" }
    return self
end

function Score.verdict(score)
    if score >= 70 then return "good" end
    if score >= 45 then return "fair" end
    return "poor"
end

function Card:result()
    local score = math.max(0, math.min(self.limit, math.floor(self.score + 0.5)))
    local reasons = {}
    for index, reason in ipairs(self.reasons) do reasons[index] = reason end
    -- Strongest reasons first; a cap leads; notes last.
    table.sort(reasons, function(a, b)
        local wa, wb = a.points == 0 and -1 or math.abs(a.points), b.points == 0 and -1 or math.abs(b.points)
        if wa ~= wb then return wa > wb end
        return a.text < b.text
    end)
    return { score = score, verdict = Score.verdict(score), reasons = reasons }
end

-- Piecewise-linear lookup: points for `value` from {{x, points}, ...}
-- sorted by x; clamps at both ends.
function Score.curve(value, points)
    if value == nil then return 0 end
    if value <= points[1][1] then return points[1][2] end
    for index = 2, #points do
        local x1, y1, x0, y0 = points[index][1], points[index][2], points[index - 1][1], points[index - 1][2]
        if value <= x1 then return y0 + (y1 - y0) * (value - x0) / (x1 - x0) end
    end
    return points[#points][2]
end

-- Minutes from this hour to the nearest sunrise or sunset of its day, or,
-- with a `level` ("civil", "nautical", "astronomical"), to the nearest dawn
-- or dusk of that twilight (Sky.twilight, on `hour.day.twilight`). math.huge
-- when the day has no such edge, as in a northern summer night.
function Score.twilight(hour, level)
    local day, now = hour.day, Sky.minutes(hour.time)
    if not (day and now) then return math.huge end
    local rise, set
    if level then
        local edges = day.twilight and day.twilight[level]
        rise, set = Sky.minutes(edges and edges.dawn), Sky.minutes(edges and edges.dusk)
    else
        local computed = day.twilight or {}
        rise, set = Sky.minutes(day.sunrise or computed.sunrise), Sky.minutes(day.sunset or computed.sunset)
    end
    return math.min(rise and math.abs(now - rise) or math.huge, set and math.abs(now - set) or math.huge)
end

-- How dark the sky is at an hour, from the sun's height (`hour.sunAltitude`,
-- set by Forecast.parse): "day", "civil", "nautical", "astronomical" (the
-- twilight it is in) or "night". nil when the height is unknown.
function Score.darkness(hour)
    local altitude = hour.sunAltitude
    if altitude == nil then return nil end
    if altitude > Sky.TWILIGHT.sunrise then return "day" end
    if altitude > Sky.TWILIGHT.civil then return "civil" end
    if altitude > Sky.TWILIGHT.nautical then return "nautical" end
    if altitude > Sky.TWILIGHT.astronomical then return "astronomical" end
    return "night"
end

-- Rounded number with a unit, or a dash.
function Score.fmt(value, unit, decimals)
    if value == nil then return "—" end
    local text = decimals and ("%." .. decimals .. "f"):format(value) or tostring(math.floor(value + 0.5))
    return text .. (unit or "")
end

return Score
