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

-- Algal blooms on a lake or river (SV-FI7). No service reports them for
-- every water (EPA CyAN has no documented API), so this is a link and a
-- reminder: most waters are never tested, so no report is not "safe".
-- After warm, calm days, when blooms grow, the row says so first.
Rules.BLOOM_LINKS = { US = "mywaterway.epa.gov" }

local function warmAndCalm(forecast)
    local hours, now = forecast and forecast.hours, forecast and forecast.now
    if not (hours and now) then return false end
    local warm, wind, n = 0, 0, 0
    for index = math.max(1, now - 47), now do
        local hour = hours[index]
        if hour.temp and hour.temp >= 25 then warm = warm + 1 end
        wind, n = wind + (hour.wind or 0), n + 1
    end
    return warm >= 6 and n > 0 and wind / n < 3
end

function Rules.blooms(place, forecast)
    if not (place and (place.lake or place.river)) then return nil end
    local link = Rules.BLOOM_LINKS[place.country or ""]
    local where = link and "check " .. link or "check the local water authority"
    local value = warmAndCalm(forecast) and "warm and calm: look for scum" or "check for blooms"
    return { label = "algae", value = value,
        note = where .. ". Most waters are never tested, so no report does not mean safe. Keep dogs out of scummy "
            .. "or discoloured water, and wait a week or two after a bloom before eating fish from it." }
end

return Rules
