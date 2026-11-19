-- Every activity the app rates, in the order the hub lists them. Each is a
-- lens over the same forecast: rules that rate an hour good, fair or poor
-- and say why (see README.md, "Write another activity"). `accent` colours
-- the activity's own page; the hub keeps the app accent.
local Activities = { list = {}, byId = {} }

local ORDER = {
    { "outdoors", "#1BA1E2" }, { "fishing", "#60A917" }, { "surf", "#00ABA9" }, { "sailing", "#6A00FF" },
    { "hiking", "#C46A2B" }, { "garden", "#008A00" }, { "stargazing", "#AA00FF" },
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

for _, entry in ipairs(ORDER) do
    if not Activities.edition or entry[1] == "outdoors" or entry[1] == Activities.edition then
        local activity = require("Weather/activities/" .. entry[1])
        activity.accent = entry[2]
        Activities.list[#Activities.list + 1] = activity
        Activities.byId[activity.id] = activity
    end
end
assert(not Activities.edition or Activities.byId[Activities.edition], "Unknown edition: " .. tostring(Activities.edition))

-- The activities a place offers, in hub order; everyday "outdoors" fits everywhere.
function Activities.at(place)
    local wanted, result = { outdoors = true }, {}
    for _, id in ipairs(place.activities or {}) do wanted[id] = true end
    for _, activity in ipairs(Activities.list) do
        if wanted[activity.id] then result[#result + 1] = activity end
    end
    return result
end

-- Whether any of a place's activities reads the sea.
function Activities.needsMarine(place)
    for _, activity in ipairs(Activities.at(place)) do
        if activity.needsMarine then return true end
    end
    return false
end

return Activities
