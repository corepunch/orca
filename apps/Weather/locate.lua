-- "Use my location" (N1): one fix from orca.location, named by the system
-- geocoder, made a draft with MyPlaces.fromLocation. The user then sees it
-- like a search result and saves it from the editor, so nothing is kept
-- unless they choose to. Denied, unsupported, no fix or no name each give a
-- calm line and never block search. `location` is the module (tests pass a
-- fake; `false` means none).
local MyPlaces = require "Weather/myplaces"
local Locate = {}

function Locate.module(location)
    if location ~= nil then return location or nil end
    local ok, module = pcall(require, "orca.location")
    return ok and type(module) == "table" and module or nil
end

-- Whether to offer it: the build can ask and the device's policy allows
-- it. A person who said no still sees it, and a tap says how to turn it on.
function Locate.available(location)
    location = Locate.module(location)
    if not (location and location.supported and location.supported()) then return false end
    return location.status() ~= "restricted"
end

Locate.NOTES = {
    permission = "Location is off for this app, so search for a place instead. You can allow it in Settings.",
    unsupported = "This device cannot say where you are. Search for a place instead.",
    timeout = "Your position took too long to find. Try again outdoors, or search for a place.",
    failed = "Your position could not be found just now. Search for a place instead.",
}

-- Finds where the device is (yields; run it in an operation). Returns a
-- draft and the fix, or nil and the line to show.
function Locate.run(location)
    location = Locate.module(location)
    if not (location and location.supported and location.supported()) then return nil, Locate.NOTES.unsupported end
    local ok, fix, err = pcall(location.current)
    if not ok or type(fix) ~= "table" then
        local code = ok and type(err) == "table" and err.code
        return nil, Locate.NOTES[code] or Locate.NOTES.failed
    end
    -- A name helps but is not needed: offline the place is "here" at its
    -- coordinates, and the user can rename nothing they did not ask for.
    local named = location.reverse and select(2, pcall(location.reverse, fix.lat, fix.lon))
    local name, region, country
    if type(named) == "table" then
        name = named.name or named.locality
        local parts = {}
        for _, part in ipairs { named.region, named.country } do
            if type(part) == "string" and part ~= "" and part ~= name and part ~= parts[#parts] then parts[#parts + 1] = part end
        end
        region = #parts > 0 and table.concat(parts, ", ") or nil
        country = type(named.countryCode) == "string" and named.countryCode:upper() or nil
    end
    local draft = MyPlaces.fromLocation(fix.lat, fix.lon, name, region, country)
    if not draft then return nil, Locate.NOTES.failed end
    return draft, fix
end

-- The search page's rows for a fix: the place (a result to open in the
-- editor), or a note.
function Locate.rows(location, drafts)
    local draft, fix = Locate.run(location)
    if not draft then return { { id = "locate:note", kind = "note", title = fix } } end
    drafts[draft.id] = draft
    local place = MyPlaces.resolveDraft(draft)
    local line = "your location"
    if MyPlaces.isSaved(draft.id) then line = "in your places · " .. line end
    if draft.base.region then line = line .. " · " .. draft.base.region end
    if fix.approximate then line = line .. " · approximate" end
    return { { id = draft.id, title = place and place.title or draft.base.title, region = draft.base.region, kind = "result",
        line = line } }
end

return Locate
