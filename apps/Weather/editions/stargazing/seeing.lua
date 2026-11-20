-- Seeing and transparency, the two numbers observers plan with, on the
-- five-step scale of the Clear Sky Chart (1 bad … 5 excellent). These are
-- our estimates from model columns, not a measured or validated seeing
-- model, and every text says "estimate":
--
--   seeing        how steady the air is, for sharp planets and double stars:
--                 the jet stream overhead (above about 35 m/s is usually
--                 poor), a steep, churning low layer, and ground wind;
--   transparency  how clear and clean it is, for faint galaxies: high and
--                 middle cloud, smoke and dust (aerosol optical depth),
--                 water vapour in the whole column, damp air and haze.
--
-- Pure functions of one hour.
local Seeing = {}

Seeing.WORDS = { "bad", "poor", "average", "good", "excellent" }

local LEVELS = { { "850hPa", 1450 }, { "700hPa", 3000 }, { "500hPa", 5600 } }

-- The temperature fall per 100 m through the lowest pressure layer that is
-- clear of the ground (the forecast's `elevation`): near 1 °C is a churning,
-- well-mixed layer; small or negative is calm, layered air.
function Seeing.lapse(hour, elevation)
    for index = 1, #LEVELS - 1 do
        local lower, upper = LEVELS[index][1], LEVELS[index + 1][1]
        local zl = hour["geopotential_height_" .. lower] or LEVELS[index][2]
        local zu = hour["geopotential_height_" .. upper] or LEVELS[index + 1][2]
        local tl, tu = hour["temperature_" .. lower], hour["temperature_" .. upper]
        if zl > (elevation or 0) + 100 and tl and tu and zu > zl then return (tl - tu) / (zu - zl) * 100 end
    end
end

local function scale(points) return math.max(1, math.min(5, math.floor(points + 0.5))) end

-- { value = 1..5, word, why = "jet stream 38 m/s overhead" } or nil when
-- the upper winds are missing.
function Seeing.seeing(hour, elevation)
    local jet = hour.wind_speed_250hPa
    if not jet then return nil end
    local points, why = 5, nil
    if jet >= 45 then points, why = points - 2.5, "a strong jet stream overhead"
    elseif jet >= 35 then points, why = points - 2, "the jet stream overhead"
    elseif jet >= 25 then points, why = points - 1, "brisk winds high up"
    elseif jet >= 18 then points = points - 0.5 end
    local lapse = Seeing.lapse(hour, elevation)
    if lapse and lapse >= 0.9 then points, why = points - 1, why or "churning air near the ground"
    elseif lapse and lapse >= 0.75 then points = points - 0.5 end
    local wind = hour.wind or 0
    if wind > 8 then points, why = points - 1, why or "wind at the ground"
    elseif wind > 5 then points = points - 0.5 end
    local value = scale(points)
    return { value = value, word = Seeing.WORDS[value], why = why or (value >= 4 and "steady air" or nil), jet = jet }
end

-- { value = 1..5, word, why } from what dims the sky.
function Seeing.transparency(hour)
    local points, why = 5, nil
    local function lose(amount, reason) points = points - amount; why = why or reason end
    local high, mid = hour.cloudHigh or 0, hour.cloudMid or 0
    if high >= 50 then lose(2, "a veil of high cloud") elseif high >= 20 then lose(1, "thin high cloud")
    elseif high >= 8 then lose(0.5, "a little high cloud") end
    if mid >= 30 then lose(1, "middle cloud") end
    local aod = hour.aod
    if aod and aod >= 0.4 then lose(2, "smoke or dust") elseif aod and aod >= 0.2 then lose(1, "haze")
    elseif aod and aod >= 0.12 then lose(0.5, "some haze") end
    local water = hour.total_column_integrated_water_vapour
    if water and water >= 30 then lose(1.5, "very moist air") elseif water and water >= 20 then lose(1, "moist air")
    elseif water and water >= 12 then lose(0.5, "some moisture") end
    if (hour.humidity or 0) >= 90 then lose(0.5, "damp air") end
    if hour.visibility and hour.visibility < 10000 then lose(1, "haze near the ground") end
    local value = scale(points)
    return { value = value, word = Seeing.WORDS[value], why = why or (value >= 4 and "clean, dry air" or nil) }
end

return Seeing
