-- What each surf break needs (SU3), keyed by place id. places.lua is shared
-- by every edition, so the surf-only facts live here:
--
--   window   the swell directions the break opens to, { from, to } clockwise,
--            in degrees the swell comes from
--   wind     the wind direction that blows offshore here (degrees it comes from)
--   size     the breaking (face) height range it handles well, metres
--   tide     "low", "low-mid", "mid", "mid-high", "high" or "all"
--   level    who it suits: "beginner", "intermediate" or "advanced"
--   exposure how much of the open-ocean swell reaches the break (a point that
--            wraps the swell loses size; a canyon that focuses it gains)
--   runoff   rain in 72 h, mm, after which the water may be unsafe (a river
--            mouth or storm drains nearby); nil when that is not known
--
-- These are starter profiles from public surf guides, not surveys: a local
-- will know better, so the place editor lets people tune them (P3): a
-- place's own `breakLevel` and `breakTide`, and a `facing` the user set
-- (`place.own.facing`), come before the table.
local Sky = require "Weather/sky"
local Breaks = {}

Breaks.list = {
    -- Open, heavy beach break with strong rips and cold water; best on an
    -- incoming mid tide in the autumn's NW groundswell and morning east wind.
    ["ocean-beach"] = { window = { 200, 320 }, wind = 90, size = { 0.8, 2.5 }, tide = "mid", level = "advanced",
        exposure = 1.2, runoff = 2.5 },
    -- Surfrider: a long, mellow right point. Point Dume and the Channel
    -- Islands block the west; it lights up on south swell and north wind.
    -- LA County advises staying out of the ocean for 72 h after 0.1 in of rain.
    malibu = { window = { 165, 235 }, wind = 0, size = { 0.5, 2 }, tide = "low-mid", level = "intermediate",
        exposure = 0.9, runoff = 2.5 },
    -- Encuentro's reefs face the North Atlantic; the trade wind turns
    -- onshore by late morning, so the early southerly land breeze matters.
    cabarete = { window = { 320, 60 }, wind = 180, size = { 0.6, 2.5 }, tide = "mid-high", level = "intermediate",
        exposure = 1.0, runoff = 10 },
    -- La Gravière: thick, hollow beach break. The Gouf de Capbreton canyon
    -- brings deep water close to the shore and focuses the swell.
    hossegor = { window = { 250, 330 }, wind = 95, size = { 1, 3 }, tide = "mid", level = "advanced",
        exposure = 1.4, runoff = 10 },
    -- Reefs and points (Ribamar, Coxos) that want NW swell, an east wind
    -- and low to mid tide; Foz do Lizandro is a river mouth.
    ericeira = { window = { 265, 330 }, wind = 100, size = { 0.8, 2.5 }, tide = "low-mid", level = "intermediate",
        exposure = 1.0, runoff = 10 },
    -- Los Lances: gentle sand-bottom waves from Atlantic swell squeezed
    -- by the Strait; Morocco blocks the south. The levante is cross-offshore.
    tarifa = { window = { 200, 280 }, wind = 30, size = { 0.4, 1.6 }, tide = "mid", level = "beginner",
        exposure = 0.8 },
    -- A patrolled, sheltered beach for learners at the north end; south
    -- swell wraps in at the south end. Sydney's Beachwatch warns that
    -- stormwater fouls ocean beaches after heavy rain.
    bondi = { window = { 40, 170 }, wind = 290, size = { 0.4, 2 }, tide = "mid", level = "beginner",
        exposure = 0.9, runoff = 10 },
}

-- A profile guessed from the beach's facing: open 60° either side of it,
-- offshore straight off the land.
local function guessed(facing)
    return { window = facing and { (facing - 60) % 360, (facing + 60) % 360 }, wind = facing and (facing + 180) % 360,
        size = { 0.5, 2.5 }, tide = "all", level = "intermediate", exposure = 1.0, guessed = true }
end

-- A break's profile: the user's own values first (level, tide, and a
-- facing they set, which moves the window), then the table, then a guess
-- from the beach's facing.
function Breaks.of(spot)
    spot = spot or {}
    local known = spot.id and Breaks.list[spot.id]
    local own = spot.own or {}
    if not (own.facing or spot.breakLevel or spot.breakTide) then return known or guessed(spot.facing) end
    local profile = {}
    for key, value in pairs(known or guessed(spot.facing)) do profile[key] = value end
    if own.facing then
        local turned = guessed(own.facing)
        profile.window, profile.wind = turned.window, turned.wind
    end
    profile.level = Breaks.LEVELS[spot.breakLevel] and spot.breakLevel or profile.level
    profile.tide = Breaks.TIDE[spot.breakTide] and spot.breakTide or profile.tide
    profile.own = true
    return profile
end

Breaks.LEVELS = { beginner = true, intermediate = true, advanced = true }

-- Whether a bearing lies in a clockwise window, and how far outside it is.
function Breaks.outside(window, bearing)
    if not (window and bearing) then return 0 end
    local from, to = window[1], window[2]
    if (bearing - from) % 360 <= (to - from) % 360 then return 0 end
    return math.min(Sky.angle(bearing, from), Sky.angle(bearing, to))
end

-- "W to NW", the window as a surfer says it.
function Breaks.windowText(window)
    if not window then return "any direction" end
    local from, to = Sky.compass(window[1]), Sky.compass(window[2])
    return from == to and from or from .. " to " .. to
end

Breaks.TIDE = {
    low = "low", ["low-mid"] = "low to mid", mid = "mid", ["mid-high"] = "mid to high", high = "high", all = "any",
}

-- How well a tide level (0 low … 1 high) fits the break: 0 when it fits,
-- else how far off it is (up to 1).
function Breaks.tideMiss(tide, level)
    local ranges = { low = { 0, 0.35 }, ["low-mid"] = { 0, 0.65 }, mid = { 0.3, 0.7 }, ["mid-high"] = { 0.35, 1 },
        high = { 0.65, 1 } }
    local range = ranges[tide or "all"]
    if not (range and level) then return 0 end
    if level < range[1] then return range[1] - level end
    if level > range[2] then return level - range[2] end
    return 0
end

return Breaks
