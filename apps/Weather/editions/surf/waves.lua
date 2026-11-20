-- The swell trains in an hour and the surf they make at a break (SU1, SU2).
-- Open-Meteo gives the open ocean: a primary and a secondary swell and the
-- local wind sea, each with its height, period and direction. What breaks
-- on the beach is an estimate: longer periods grow more as they reach
-- shallow water, a swell from outside the break's window mostly misses it,
-- and each break keeps more or less of the swell (`exposure`). Real
-- nearshore models (SWAN) are not available, so every number from here is
-- called an "estimate" on screen.
local Score = require "Weather/score"
local Units = require "Weather/units"
local Breaks = require "Weather/editions/surf/breaks"
local Waves = {}

-- The marine columns beyond the shared ones (forecast.lua names the rest).
Waves.COLUMNS = {
    "secondary_swell_wave_height", "secondary_swell_wave_period", "secondary_swell_wave_direction",
    "wind_wave_height", "wind_wave_period", "wind_wave_direction", "swell_wave_peak_period",
}

-- The trains in an hour, biggest first: { name, height, period, direction }.
-- Without separate trains, the combined sea stands in.
function Waves.trains(hour)
    local list = {}
    local function add(name, height, period, direction)
        if height and height >= 0.05 then
            list[#list + 1] = { name = name, height = height, period = period, direction = direction }
        end
    end
    add("swell", hour.swellHeight, hour.swell_wave_peak_period or hour.swellPeriod, hour.swellDir)
    add("secondary swell", hour.secondary_swell_wave_height, hour.secondary_swell_wave_period,
        hour.secondary_swell_wave_direction)
    add("wind sea", hour.wind_wave_height, hour.wind_wave_period, hour.wind_wave_direction)
    if #list == 0 then add("waves", hour.waveHeight, hour.wavePeriod, hour.waveDir) end
    table.sort(list, function(a, b) return a.height > b.height end)
    return list
end

-- How much a period grows the wave in shallow water: short windswell
-- breaks smaller than its open-ocean height, groundswell bigger.
local GROWTH = { { 5, 0.7 }, { 8, 1.0 }, { 12, 1.3 }, { 16, 1.6 }, { 20, 1.7 } }
-- How much of a swell reaches a break, by degrees outside its window.
local MISS = { { 0, 1 }, { 20, 0.7 }, { 45, 0.35 }, { 90, 0.05 } }

-- The estimated breaking face of one train at a break, metres.
function Waves.face(train, profile)
    local growth = train.period and Score.curve(train.period, GROWTH) or 1
    local reach = Score.curve(Breaks.outside(profile.window, train.direction), MISS)
    return train.height * growth * (profile.exposure or 1) * reach
end

-- The surf at a break: the train that makes the biggest waves there, its
-- estimated face and a range around it (the estimate is rough), or nil
-- without sea data.
function Waves.surf(hour, profile)
    local best
    for _, train in ipairs(Waves.trains(hour)) do
        local face = Waves.face(train, profile)
        if not best or face > best.face then best = { face = face, train = train } end
    end
    if best then best.low, best.high = best.face * 0.8, best.face * 1.2 end
    return best
end

-- Body words for a breaking height, the way surfers talk.
local WORDS = {
    { 0.25, "flat" }, { 0.45, "ankle" }, { 0.65, "knee" }, { 0.85, "thigh" }, { 1.05, "waist" }, { 1.3, "chest" },
    { 1.55, "shoulder" }, { 1.85, "head" }, { 2.6, "overhead" }, { 3.4, "well overhead" }, { 4.5, "double overhead" },
}
local function word(m)
    for _, entry in ipairs(WORDS) do
        if m < entry[1] then return entry[2] end
    end
    return "huge"
end
local BODY = { ankle = true, knee = true, thigh = true, waist = true, chest = true, shoulder = true, head = true }
local function said(w) return BODY[w] and w .. " high" or w end

-- "waist high", "chest to head high", "head high to overhead".
function Waves.words(low, high)
    local a, b = word(low), word(high or low)
    if a == b then return said(a) end
    if BODY[a] and BODY[b] then return a .. " to " .. b .. " high" end
    return said(a) .. " to " .. b
end

-- One word for an hour's line: "head high".
function Waves.word(m) return m and said(word(m)) or "—" end

-- "1.6–2.3 m" or "5–8 ft", the estimate's range in the user's units.
function Waves.range(surf)
    if not surf then return "—" end
    local decimals = Units.current("height") == "ft" and 0 or 1
    local low, high = Units.height(surf.low, decimals), Units.height(surf.high, decimals)
    return low:match("^[%d%.]+") .. "–" .. high
end

return Waves
