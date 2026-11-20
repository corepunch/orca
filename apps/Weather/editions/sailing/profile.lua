-- The rider (SA2): what they sail, how well, their own wind range and their
-- gear. Everything is a setting (Settings, group "sailing"), so the settings
-- page lists it and the rating reads it. The list follows the sport: a
-- kiter sees kites and weight, a keelboat sailor neither. Each sport keeps
-- its own wind range ("min-kite", "min-dinghy"…), so switching between the
-- dinghy and the keelboat keeps both.
--
-- The wind ranges and gear sizes are rules of thumb, not physics; the page
-- and the cards say so.
local Settings = require "Weather/settings"
local Units = require "Weather/units"
local Profile = {}

local KN = 0.514444 -- m/s in a knot
Profile.KN = KN

Profile.SPORTS = { { "kite", "kitesurf" }, { "windsurf", "windsurf" }, { "wing", "wing foil" }, { "dinghy", "dinghy" },
    { "keelboat", "keelboat" } }
Profile.LEVELS = { { "beginner", "learning" }, { "intermediate", "intermediate" }, { "expert", "expert" } }

-- Rule of thumb, knots: the wind each rider can use, from school and club
-- guidance (a learner kiter 12–20, a planing windsurfer from about 12, a
-- dinghy reefs or stays in above about 18–24).
Profile.BANDS = {
    kite = { beginner = { 12, 20 }, intermediate = { 12, 28 }, expert = { 10, 35 } },
    windsurf = { beginner = { 6, 14 }, intermediate = { 12, 25 }, expert = { 12, 35 } },
    wing = { beginner = { 12, 20 }, intermediate = { 12, 25 }, expert = { 10, 30 } },
    dinghy = { beginner = { 5, 12 }, intermediate = { 6, 18 }, expert = { 6, 24 } },
    keelboat = { beginner = { 6, 15 }, intermediate = { 6, 20 }, expert = { 8, 28 } },
}

-- Gear the rider owns, sizes in m², and the rule of thumb that sizes it:
-- size ≈ weight (kg) × factor ÷ wind (kn). The kite factor 2.2 is the one
-- kite schools quote; 1.34 for windsurf sails and 1.2 for wings are the
-- same idea scaled to those rigs. All three are estimates.
Profile.GEAR = {
    kite = { key = "kites", title = "kites", factor = 2.2, unit = " m", default = "9 / 12",
        options = { "9 / 12", "7 / 9 / 12", "8 / 10 / 12", "6 / 8 / 10 / 12 / 14", "5 / 7 / 9 / 11 / 13", "10 / 14" } },
    windsurf = { key = "sails", title = "sails", factor = 1.34, unit = " m²", default = "4.7 / 5.8 / 7.0",
        options = { "4.7 / 5.8 / 7.0", "4.2 / 5.0 / 5.8 / 6.6", "5.3 / 6.5 / 7.8", "3.7 / 4.2 / 4.7 / 5.3" } },
    wing = { key = "wings", title = "wings", factor = 1.2, unit = " m", default = "4 / 5 / 6",
        options = { "4 / 5 / 6", "3 / 4.5 / 6", "5 / 6.5", "2.5 / 3.5 / 4.5 / 5.5" } },
}

local function raw(key, fallback)
    local value = Settings.get("sailing", key)
    if value == nil then return fallback end
    return value
end

local BASE = {
    { key = "sport", title = "sport", kind = "choice", default = "kite", options = Profile.SPORTS },
    { key = "level", title = "level", kind = "choice", default = "intermediate", options = Profile.LEVELS },
}

local function knots(v) return Units.wind(v * KN) end

-- The declarations for one sport and level: the base, the sport's own wind
-- range (its default from the level), weight and gear for boards and kites.
local function build(sport, level)
    local band = (Profile.BANDS[sport] or Profile.BANDS.kite)[level] or Profile.BANDS.kite.intermediate
    local list = { BASE[1], BASE[2],
        { key = "min-" .. sport, title = "lowest wind", kind = "number", default = band[1], min = 1, max = 40, step = 1,
            format = knots, detail = ("the least you can use; %s for your sport and level (a rule of thumb)")
                :format(knots(band[1])) },
        { key = "max-" .. sport, title = "highest wind", kind = "number", default = band[2], min = 5, max = 50, step = 1,
            format = knots, detail = ("the most you want; %s for your sport and level"):format(knots(band[2])) },
    }
    local gear = Profile.GEAR[sport]
    if gear then
        list[#list + 1] = { key = "weight", title = "your weight", kind = "number", default = 75, min = 30, max = 150,
            step = 5, format = function(v) return ("%d kg"):format(v) end, detail = "sizes the kite, sail or wing" }
        list[#list + 1] = { key = gear.key, title = gear.title, kind = "choice", default = gear.default,
            options = gear.options, detail = "sizes you own, m²" }
    end
    list[#list + 1] = { key = "range", title = "compare spots within", kind = "choice", default = 900,
        detail = "for \"this weekend\": the spots you would drive to",
        options = { { 300, Units.distance(300) }, { 900, Units.distance(900) }, { 20000, "everywhere" } } }
    return list
end

-- Cached per settings revision; reading "sport" while building sees only
-- the base list, so the lookup does not recurse.
local cache = {}
function Profile.declarations()
    if cache.building then return BASE end
    local revision = Settings.revision()
    if cache.revision ~= revision then
        cache.building = true
        local sport, level = raw("sport", "kite"), raw("level", "intermediate")
        cache.building = nil
        cache.list, cache.revision, cache.profile = build(sport, level), revision, nil
    end
    return cache.list
end

-- Sailing.settings: a list whose rows follow the current sport.
Profile.settings = setmetatable({}, { __index = function(_, index) return Profile.declarations()[index] end })

local function sizes(text)
    local list = {}
    for size in tostring(text or ""):gmatch("[%d%.]+") do list[#list + 1] = tonumber(size) end
    table.sort(list)
    return list
end
Profile.sizes = sizes

-- The rider now: sport, level, wind range in m/s, weight and gear sizes.
function Profile.current()
    Profile.declarations()
    if cache.profile then return cache.profile end
    local sport, level = raw("sport", "kite"), raw("level", "intermediate")
    local gear = Profile.GEAR[sport]
    local band = (Profile.BANDS[sport] or Profile.BANDS.kite)[level] or Profile.BANDS.kite.intermediate
    local profile = {
        sport = sport, level = level, board = gear ~= nil,
        min = raw("min-" .. sport, band[1]) * KN, max = raw("max-" .. sport, band[2]) * KN, gear = gear,
        weight = gear and raw("weight", 75), sizes = gear and sizes(raw(gear.key, gear.default)) or {},
        range = raw("range", 900),
    }
    if profile.max < profile.min then profile.max = profile.min end
    cache.profile = profile
    return profile
end

-- "kitesurf · intermediate": the profile in words.
function Profile.title(profile)
    local function find(list, value)
        for _, entry in ipairs(list) do if entry[1] == value then return entry[2] end end
        return value
    end
    return find(Profile.SPORTS, profile.sport) .. " · " .. find(Profile.LEVELS, profile.level)
end

local function size(value) return (value % 1 == 0 and "%d" or "%.1f"):format(value) end

-- What to rig in a wind (m/s): `text` ("the 9 m", "one reef") and `why`.
-- Boards and kites size by weight ÷ wind; boats by reefing points (rule of
-- thumb: a day keelboat reefs at about 15 and 20 kn; a dinghy depowers from
-- about 12 kn).
function Profile.rig(profile, wind)
    if not wind or wind <= 0 then return nil end
    local kn = wind / KN
    local gear = profile.gear
    if gear then
        local list = profile.sizes
        if #list == 0 then return nil end
        local ideal = profile.weight * gear.factor / kn
        local best = list[1]
        for _, s in ipairs(list) do
            if math.abs(math.log(s / ideal)) < math.abs(math.log(best / ideal)) then best = s end
        end
        local text = "the " .. size(best) .. gear.unit
        local why = ("about %s%s for %d kg in %s (weight × %.4g ÷ knots, a rule of thumb)")
            :format(size(math.floor(ideal * 2 + 0.5) / 2), gear.unit, profile.weight, Units.wind(wind), gear.factor)
        if ideal > list[#list] * 1.15 then
            return text, "even your biggest will feel underpowered; " .. why, "under"
        elseif ideal < list[1] * 0.85 then
            return text, "even your smallest will be a handful; " .. why, "over"
        end
        return text, why, "ok"
    end
    if profile.sport == "keelboat" then
        if kn < 15 then return "full main", "under about 15 kn (a rule of thumb)", "ok" end
        if kn < 20 then return "one reef", "about 15–20 kn (a rule of thumb)", "ok" end
        if kn < 25 then return "two reefs", "about 20–25 kn (a rule of thumb)", "ok" end
        return "storm sails or stay in", "over 25 kn", "over"
    end
    if kn < 12 then return "full power", "under about 12 kn", "ok" end
    if kn < 18 then return "depower", "kicker, cunningham and outhaul on, about 12–18 kn", "ok" end
    return "reef or a smaller rig", "over about 18 kn", "over"
end

return Profile
