-- Every activity the app rates, in the order the hub lists them. Each is a
-- lens over the same forecast: rules that rate an hour good, fair or poor
-- and say why (see README.md, "Write another activity"). `accent` colours
-- the activity's own page; the hub keeps the app accent.
local Activities = { list = {}, byId = {} }

local ORDER = {
    { "outdoors", "#1BA1E2" }, { "fishing", "#60A917" }, { "surf", "#00ABA9" }, { "sailing", "#6A00FF" },
    { "hiking", "#C46A2B" }, { "garden", "#008A00" }, { "stargazing", "#AA00FF" },
}

for _, entry in ipairs(ORDER) do
    local activity = require("Weather/activities/" .. entry[1])
    activity.accent = entry[2]
    Activities.list[#Activities.list + 1] = activity
    Activities.byId[activity.id] = activity
end

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
