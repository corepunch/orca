-- Where to read the local fishing rules (FI12): seasons, limits and
-- licences. There is no free worldwide source, so these are curated per
-- place; a place may also bring its own (`rules = { title, url }` in
-- places.lua). Without one, the row only reminds the angler to check.
local Rules = {}

Rules.BY_PLACE = {
    tahoe = { title = "California; Nevada: ndow.org", url = "wildlife.ca.gov" },
    islamorada = { title = "Florida saltwater", url = "myfwc.com" },
    lofoten = { title = "Norway, sea fishing", url = "fiskeridir.no" },
    bodensee = { title = "Lake Constance", url = "ibkf.org" },
}

-- { title, url } for a place, or nil.
function Rules.of(place)
    if not place then return nil end
    return place.rules or Rules.BY_PLACE[place.id]
end

-- A condition row for the activity page.
function Rules.metric(place)
    local rules = Rules.of(place)
    if not rules then
        return { label = "local rules", value = "check before you go", note = "seasons, size limits and licences" }
    end
    return { label = "local rules", value = rules.url, note = rules.title .. ": seasons, limits and licences" }
end

return Rules
