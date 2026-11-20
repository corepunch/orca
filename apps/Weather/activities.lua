-- Every activity the app rates, in the order the hub lists them. Each is a
-- lens over the same forecast: rules that rate an hour good, fair or poor
-- and say why (see README.md, "Write another activity"). `accent` colours
-- the activity's own page; the hub keeps the app accent.
local Settings = require "Weather/settings"
local Activities = { list = {}, byId = {} }

local ORDER = {
    { "outdoors", "#1BA1E2" }, { "fishing", "#60A917" }, { "surf", "#00ABA9" }, { "sailing", "#6A00FF" },
    { "hiking", "#C46A2B" }, { "garden", "#008A00" }, { "stargazing", "#AA00FF" },
    -- Everyday lenses, full app only (`everywhere`: they fit every place).
    -- Metro colours kept clear of the fair and poor tones, each at least
    -- 4.5:1 on black.
    { "run", "#A4C400" }, { "cycle", "#F472D0" }, { "dogwalk", "#87794E" }, { "picnic", "#6D8764" },
    { "laundry", "#6E8094" },
}

-- An edition is a one-activity build of this app: EDITION=surf bundles a
-- `build.lua` (see package.lua, `Editions`) that keeps only everyday
-- "outdoors" and surf, at the places that offer surf. ORCA_EDITION does the
-- same for a desktop run. Without either, every activity is in.
local function edition()
    local ok, build = pcall(require, "Weather/build")
    local id = ok and type(build) == "table" and build.edition or os.getenv("ORCA_EDITION")
    return id ~= "" and id or nil
end
Activities.edition = edition()

-- The app's name, as package.lua `Editions` gives it (tests keep the two in
-- step): the title of the settings page, an edition's activity page and the
-- folder its files live in.
local NAMES = {
    fishing = "Fishers' Weather", surf = "Surfers' Weather", sailing = "Sailors' Weather",
    hiking = "Hikers' Weather", garden = "Gardeners' Weather", stargazing = "Stargazers' Weather",
}
Activities.names = NAMES
Activities.appName = NAMES[Activities.edition] or "Weather"

for _, entry in ipairs(ORDER) do
    if not Activities.edition or entry[1] == "outdoors" or entry[1] == Activities.edition then
        local activity = require("Weather/activities/" .. entry[1])
        activity.accent = entry[2]
        Activities.list[#Activities.list + 1] = activity
        Activities.byId[activity.id] = activity
        Settings.register(activity)
    end
end
assert(not Activities.edition or Activities.byId[Activities.edition], "Unknown edition: " .. tostring(Activities.edition))

-- The activities a place offers, in hub order; everyday "outdoors" and the
-- `everywhere` lenses fit every place.
function Activities.at(place)
    local wanted, result = { outdoors = true }, {}
    for _, id in ipairs(place.activities or {}) do wanted[id] = true end
    for _, activity in ipairs(Activities.list) do
        if wanted[activity.id] or activity.everywhere then result[#result + 1] = activity end
    end
    return result
end

-- Whether an activity reads the sea at a place: `needsMarine` is a value
-- or a function of the place (a lake has no waves to ask for).
function Activities.marineFor(activity, place)
    local need = activity.needsMarine
    if type(need) == "function" then need = need(place) end
    return need
end

-- Whether any of a place's activities reads the sea.
function Activities.needsMarine(place)
    for _, activity in ipairs(Activities.at(place)) do
        if Activities.marineFor(activity, place) then return true end
    end
    return false
end

-- The activity whose cards lead the hub: the edition's own, or everyday
-- outdoors in the full app.
function Activities.lead()
    return Activities.byId[Activities.edition or "outdoors"]
end

-- What one forecast request for a place must ask for: the union of its
-- activities' `fields` (hourly, daily, minutely15 and marine columns, the
-- most pastDays, forecastDays and minutely15Steps, the weather `models`)
-- and their `sources`, one per id. Every required source comes along, but
-- an optional one only from the lead activity and from `focus`, the
-- activity whose page is open (an activity or its id; "all" lists every
-- source, as replaying a saved forecast needs).
function Activities.request(place, focus)
    local request = { hourly = {}, daily = {}, minutely15 = {}, marine = {}, models = {}, sources = {} }
    local seen, sources, lead = {}, {}, Activities.lead()
    local function wanted(activity, source)
        return not source.optional or focus == "all" or activity == lead or activity == focus or activity.id == focus
    end
    for _, activity in ipairs(Activities.at(place)) do
        local fields = activity.fields or {}
        local models = fields.models
        if type(models) == "function" then models = models(place) end
        for _, block in ipairs { "hourly", "daily", "minutely15", "marine", "models" } do
            for _, name in ipairs(block == "models" and models or fields[block] or {}) do
                local key = block .. ":" .. name
                if not seen[key] then seen[key] = true; table.insert(request[block], name) end
            end
        end
        for _, name in ipairs { "pastDays", "forecastDays", "minutely15Steps" } do
            if fields[name] then request[name] = math.max(request[name] or 0, fields[name]) end
        end
        for _, source in ipairs(activity.sources or {}) do
            if not sources[source.id] and wanted(activity, source) then
                sources[source.id] = true; table.insert(request.sources, source)
            end
        end
    end
    return request
end

return Activities
