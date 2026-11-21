-- The user's own places (P2): saved on the device with Storage ("places",
-- one file per edition), in the user's order, the first one home. The
-- curated places (places.lua) are suggestions: the first run shows them so
-- the app is useful at once, and a suggestion kept becomes a saved place
-- with the same id, so the editions' tables keyed by id (peaks, spots,
-- breaks, rules) still find it.
--
--   MyPlaces.init { storage = store, curated = Places }
--   MyPlaces.all()          -- saved places, then suggestions (the hub's list)
--   MyPlaces.mine()         -- saved places, or the suggestions before any is saved
--   MyPlaces.add(draft)     -- a draft from search (fromResult) or location (fromLocation)
--   MyPlaces.set(id, key, value), MyPlaces.move(id, delta), MyPlaces.remove(id)
--
-- A saved entry keeps the user's own values apart (`own`): a curated place
-- stores only its id and what the user changed, a found place also its
-- coordinates and names (`base`). A resolved place is a plain table with the
-- curated or found fields, the own values on top (`place.own` lists them),
-- `saved = true`, and `lake`/`river` from its kind.
--
-- The editor's rows come from declarations (P3): the place's kind, in the
-- full app a "good for" switch per activity, and each activity's own
-- `placeFields` = { { key, title, kind = "compass" | "choice" | "toggle" |
-- "number", options, default (a value or a function of the place), min,
-- max, step, format, detail } }. Keys are shared between activities (surf,
-- sailing and fishing all read `facing`); the first declaration wins.
local Activities = require "Weather/activities"
local Sky = require "Weather/sky"
local MyPlaces = { FILE = "places" }

local state = { entries = {}, curated = {}, revision = 0 }
local resolved -- cached { list, byId }

-- The kinds a place can be, and what each is likely good for (P4).
MyPlaces.KINDS = {
    { "town", "a town" }, { "coast", "a coast or a beach" }, { "lake", "a lake" },
    { "river", "a river" }, { "mountain", "a mountain or a trail" },
}
local GOOD_FOR = {
    town = { "garden", "stargazing" }, coast = { "surf", "sailing", "fishing", "stargazing" },
    lake = { "fishing", "sailing", "stargazing" }, river = { "fishing" }, mountain = { "hiking", "stargazing" },
}
-- GeoNames feature codes (Open-Meteo geocoding `feature_code`) → kind.
local FEATURES = {
    coast = { "BCH", "BCHS", "BAY", "BAYS", "CAPE", "COVE", "PT", "PTS", "PEN", "ISL", "ISLS", "HBR", "LGN", "SD",
        "STRT", "GULF", "INLT", "FJD", "RF", "SHOR", "CST", "SPIT" },
    lake = { "LK", "LKS", "LKN", "LKO", "RSV", "PND", "PNDS", "LKSN" },
    river = { "STM", "STMS", "STMM", "STMX", "RVN", "CNL", "ESTY" },
    mountain = { "MT", "MTS", "PK", "PKS", "HLL", "HLLS", "VLC", "PASS", "RDGE", "UPLD", "SPUR", "CRQ", "GLCR" },
}
-- What a place found without a telling feature is, by edition: a surfer
-- or a sailor searches for beaches, a hiker for mountains.
MyPlaces.EDITION_KIND = { surf = "coast", sailing = "coast", hiking = "mountain" }
local FEATURE = {}
for kind, codes in pairs(FEATURES) do for _, code in ipairs(codes) do FEATURE[code] = kind end end
MyPlaces.FEATURE = FEATURE
-- A word for a found place's line ("peak · Valais, Switzerland").
local WORDS = { BCH = "beach", BAY = "bay", CAPE = "cape", PT = "point", ISL = "island", HBR = "harbour",
    LK = "lake", RSV = "reservoir", STM = "river", MT = "mountain", PK = "peak", HLL = "hill", VLC = "volcano",
    PASS = "pass", PPLC = "capital", ADM1 = "region", ADM2 = "district", PCLI = "country" }
MyPlaces.HIGH = 1000 -- metres: from here a place is guessed good for hiking
MyPlaces.CITY = 200000 -- people: above it city light spoils the stars

local function copy(t)
    local out = {}
    for k, v in pairs(t or {}) do out[k] = type(v) == "table" and copy(v) or v end
    return out
end

local function offers(place, id)
    for _, other in ipairs(place.activities or {}) do if other == id then return true end end
    return false
end
MyPlaces.offers = offers

-- A curated place's kind, from its facts.
local function kindOf(place)
    if place.kind then return place.kind end
    if place.lake then return "lake" end
    if place.river then return "river" end
    if place.facing then return "coast" end
    if offers(place, "hiking") and not offers(place, "garden") then return "mountain" end
    return "town"
end

-- The activities a new place is guessed good for (P4): its kind's, hiking
-- from `HIGH` up, the stars away from big cities. An edition adds a place
-- for its own activity.
function MyPlaces.guess(kind, elevation, population)
    if Activities.edition then return { Activities.edition } end
    local list = {}
    for _, id in ipairs(GOOD_FOR[kind] or GOOD_FOR.town) do
        if not (id == "stargazing" and (population or 0) > MyPlaces.CITY) then list[#list + 1] = id end
    end
    if (elevation or 0) >= MyPlaces.HIGH and not offers({ activities = list }, "hiking") then list[#list + 1] = "hiking" end
    local known, result = {}, {}
    for _, activity in ipairs(Activities.list) do known[activity.id] = true end
    for _, id in ipairs(list) do if known[id] then result[#result + 1] = id end end
    return result
end

-- An entry → a place: the base (curated or found), then the user's own
-- values; `lake` and `river` follow the kind the user chose.
local function resolve(entry)
    local base = entry.curated and state.curatedById[entry.id] or entry.base
    if not base then return nil end
    local place = copy(base)
    place.id, place.own = entry.id, copy(entry.own)
    for key, value in pairs(entry.own or {}) do place[key] = copy({ value })[1] end
    place.kind = (entry.own and entry.own.kind) or kindOf(base)
    if (entry.own and entry.own.kind) or not entry.curated then
        place.lake, place.river = place.kind == "lake" or nil, place.kind == "river" or nil
    end
    if Activities.edition and not offers(place, Activities.edition) then
        place.activities = copy(place.activities or {})
        table.insert(place.activities, Activities.edition)
    end
    place.saved, place.curated = true, entry.curated or nil
    return place
end

local function list()
    if resolved then return resolved end
    resolved = { list = {}, byId = {} }
    for _, entry in ipairs(state.entries) do
        local place = resolve(entry)
        if place then resolved.list[#resolved.list + 1] = place; resolved.byId[place.id] = place end
    end
    return resolved
end

local function save()
    resolved = nil
    state.revision = state.revision + 1
    if state.storage then state.storage:save(MyPlaces.FILE, { version = 1, places = state.entries }) end
end

-- options.storage (a Storage store; nil keeps them in memory), options.curated
-- (the suggestions, default places.lua).
function MyPlaces.init(options)
    options = options or {}
    state.storage = options.storage
    state.curated = options.curated or require "Weather/places"
    state.curatedById = {}
    for _, place in ipairs(state.curated) do state.curatedById[place.id] = place end
    local saved = state.storage and state.storage:load(MyPlaces.FILE)
    state.entries = {}
    for _, entry in ipairs(type(saved) == "table" and type(saved.places) == "table" and saved.places or {}) do
        if type(entry) == "table" and type(entry.id) == "string" then
            entry.own = type(entry.own) == "table" and entry.own or {}
            entry.base = type(entry.base) == "table" and entry.base or nil
            state.entries[#state.entries + 1] = entry
        end
    end
    resolved = nil
    state.revision = state.revision + 1
end

-- Bumps on every change, so pages know to redraw.
function MyPlaces.revision() return state.revision end
function MyPlaces.saved() return list().list end
function MyPlaces.isSaved(id) return list().byId[id] ~= nil end

function MyPlaces.suggestions()
    local out = {}
    for _, place in ipairs(state.curated) do
        if not list().byId[place.id] then out[#out + 1] = place end
    end
    return out
end

function MyPlaces.mine()
    local saved = list().list
    return #saved > 0 and saved or state.curated
end

function MyPlaces.all()
    local out = {}
    for _, place in ipairs(list().list) do out[#out + 1] = place end
    for _, place in ipairs(MyPlaces.suggestions()) do out[#out + 1] = place end
    return out
end

function MyPlaces.find(id) return list().byId[id] or state.curatedById[id] end

local function indexOf(id)
    for index, entry in ipairs(state.entries) do if entry.id == id then return index end end
end

-- An id no saved or curated place has: the draft's own, else with "-2"…
local function freeId(id)
    local function taken(candidate) return indexOf(candidate) or state.curatedById[candidate] end
    if not taken(id) then return id end
    local n = 2
    while taken(id .. "-" .. n) do n = n + 1 end
    return id .. "-" .. n
end

-- Saves a draft (or keeps a suggestion by id) at the end of the list, or
-- first with `home`. Returns the place's id; a place already saved stays
-- where it is.
function MyPlaces.add(draft, home)
    if type(draft) == "string" then draft = state.curatedById[draft] and { id = draft, curated = true } end
    if not draft then return nil end
    if draft.curated and indexOf(draft.id) then return draft.id end
    local entry
    if draft.curated then
        entry = { id = draft.id, curated = true, own = copy(draft.own) }
    else
        entry = { id = freeId(draft.id), base = copy(draft.base), own = copy(draft.own) }
    end
    table.insert(state.entries, home and 1 or #state.entries + 1, entry)
    save()
    return entry.id
end

-- Forgets a place, and the forecast kept for it (private by default). Logs
-- keep their entries.
function MyPlaces.remove(id)
    local index = indexOf(id)
    if not index then return false end
    table.remove(state.entries, index)
    save()
    if state.storage then state.storage:remove("forecast-" .. id) end
    return true
end

-- Moves a place up (-1) or down (+1); to the top is home.
function MyPlaces.move(id, delta)
    local index = indexOf(id)
    if not index then return false end
    local target = math.max(1, math.min(#state.entries, index + delta))
    if target == index then return false end
    table.insert(state.entries, target, table.remove(state.entries, index))
    save()
    return true
end
function MyPlaces.makeHome(id) return MyPlaces.move(id, -#state.entries) end

-- Sets one own value on an entry or a draft (nil clears it). A new kind
-- brings its guessed activities with it (the full app; an edition keeps
-- its own).
function MyPlaces.apply(entry, key, value)
    entry.own = entry.own or {}
    entry.own[key] = value
    if key == "kind" and not Activities.edition then
        local base = entry.curated and state.curatedById[entry.id] or entry.base or {}
        entry.own.activities = MyPlaces.guess(value, base.elevation, base.population)
    end
end

-- The user's own value for a saved place.
function MyPlaces.set(id, key, value)
    local index = indexOf(id)
    if not index then return false end
    MyPlaces.apply(state.entries[index], key, value)
    save()
    return true
end

-- A draft not saved yet: { id, base, own } for a found place, { id,
-- curated = true, own } for a suggestion; resolved like a saved entry.
function MyPlaces.draft(id, base, own)
    if base == nil and state.curatedById[id] then return { id = id, curated = true, own = own or {} } end
    return { id = id, base = base, own = own or {} }
end
function MyPlaces.resolveDraft(draft)
    local place = resolve(draft)
    if place then place.saved, place.draft = nil, true end
    return place
end

-- A draft from one Open-Meteo geocoding result: its names, coordinates,
-- height and country, and the guessed kind and activities. `query`, when
-- it is one of the result's postcodes, is kept to show.
function MyPlaces.fromResult(result, query)
    if type(result) ~= "table" or not (tonumber(result.latitude) and tonumber(result.longitude)) then return nil end
    local name = tostring(result.name or "")
    local parts = {}
    for _, part in ipairs { result.admin1, result.country } do
        if type(part) == "string" and part ~= "" and part ~= name and part ~= parts[#parts] then parts[#parts + 1] = part end
    end
    local kind = FEATURE[result.feature_code] or MyPlaces.EDITION_KIND[Activities.edition] or "town"
    local postcode
    for _, code in ipairs(type(result.postcodes) == "table" and result.postcodes or {}) do
        if query and tostring(code):upper() == tostring(query):upper() then postcode = tostring(code) end
    end
    local base = { title = name, region = table.concat(parts, ", "), lat = tonumber(result.latitude),
        lon = tonumber(result.longitude), elevation = tonumber(result.elevation), country = result.country_code,
        population = tonumber(result.population), feature = result.feature_code, timezone = result.timezone,
        postcode = postcode }
    base.activities = MyPlaces.guess(kind, base.elevation, base.population)
    local own = { kind = kind }
    -- A peak's own height is the top of the hike.
    if kind == "mountain" and base.elevation then own.summit = math.floor(base.elevation + 0.5) end
    local id = result.id and ("geo-" .. math.floor(tonumber(result.id) or 0)) or
        ("at-%.3f_%.3f"):format(base.lat, base.lon)
    return MyPlaces.draft(id, base, own)
end

-- A draft for where the device is (locate.lua calls this with the fix and,
-- when it has one, the geocoded name, region and ISO country code). Nothing
-- here asks for location.
function MyPlaces.fromLocation(lat, lon, name, region, country)
    lat, lon = tonumber(lat), tonumber(lon)
    if not (lat and lon) or lat < -90 or lat > 90 or lon < -180 or lon > 180 then return nil end
    local base = { title = name or "here", region = region or ("%.2f°, %.2f°"):format(lat, lon),
        lat = lat, lon = lon, located = true, country = country }
    base.activities = MyPlaces.guess("town")
    return MyPlaces.draft(("at-%.3f_%.3f"):format(lat, lon), base, { kind = "town" })
end

-- A found place's line: what it is and where ("peak · Valais, Switzerland").
function MyPlaces.line(result)
    local word = WORDS[result.feature] or (result.feature and result.feature:match("^PPL") and "town")
    local parts = {}
    if word then parts[#parts + 1] = word end
    if result.postcode then parts[#parts + 1] = result.postcode end
    if result.region and result.region ~= "" then parts[#parts + 1] = result.region end
    return table.concat(parts, " · ")
end

-- The editor's declarations for a place: its kind, the full app's "good
-- for" switches and the place fields of each activity it offers.
function MyPlaces.fields(place)
    local fields, seen = {}, {}
    local function add(field, group)
        if seen[field.key] then return end
        seen[field.key] = true
        local entry = copy(field)
        entry.default, entry.format = field.default, field.format
        entry.group = group
        fields[#fields + 1] = entry
    end
    add({ key = "kind", title = "kind of place", kind = "choice", options = MyPlaces.KINDS,
        detail = Activities.edition and "" or "changing it suggests its activities again" }, "place")
    if not Activities.edition then
        for _, activity in ipairs(Activities.list) do
            if activity.id ~= "outdoors" and not activity.everywhere then
                add({ key = "for:" .. activity.id, title = activity.title, kind = "toggle", activity = activity.id },
                    "good for")
            end
        end
    end
    for _, activity in ipairs(Activities.at(place)) do
        for _, field in ipairs(activity.placeFields or {}) do add(field, activity.id) end
    end
    return fields
end

local function field(place, key)
    for _, entry in ipairs(MyPlaces.fields(place)) do if entry.key == key then return entry end end
end
MyPlaces.field = field

-- The value a field shows: the place's own, else its default.
function MyPlaces.value(place, entry)
    if type(entry) == "string" then entry = field(place, entry) end
    if not entry then return nil end
    if entry.activity then return offers(place, entry.activity) end
    local value = place[entry.key]
    if value == nil then
        value = entry.default
        if type(value) == "function" then value = value(place) end
    end
    return value
end

local function optionOf(option)
    if type(option) == "table" then return option[1], option[2] or option[1] end
    return option, tostring(option)
end

function MyPlaces.text(place, entry)
    if type(entry) == "string" then entry = field(place, entry) end
    if not entry then return "" end
    local value = MyPlaces.value(place, entry)
    if entry.format then return entry.format(value, place) end
    if entry.kind == "toggle" then return value and "yes" or "no" end
    if value == nil then return "not set" end
    if entry.kind == "compass" then return ("%s (%d°)"):format(Sky.compass(value), math.floor(value + 0.5)) end
    for _, option in ipairs(entry.options or {}) do
        local v, title = optionOf(option)
        if v == value then return title end
    end
    return tostring(value)
end

-- One tap on a field: the next choice or compass point (`direction` -1
-- back), a toggle flipped, a number a step on. Returns the key and value
-- to store (an activity switch stores the new `activities` list).
function MyPlaces.next(place, entry, direction)
    direction = direction or 1
    local value = MyPlaces.value(place, entry)
    if entry.activity then
        local activities, on = {}, not value
        for _, id in ipairs(place.activities or {}) do if id ~= entry.activity then activities[#activities + 1] = id end end
        if on then activities[#activities + 1] = entry.activity end
        return "activities", activities
    end
    if entry.kind == "toggle" then return entry.key, not value end
    if entry.kind == "compass" then
        local step = entry.step or 45
        if value == nil then return entry.key, direction > 0 and 0 or 360 - step end
        local snapped = math.floor(value / step + 0.5) * step
        return entry.key, math.floor((snapped + step * direction) % 360)
    end
    if entry.kind == "number" then
        local step = entry.step or 1
        local start = value or entry.start or entry.min or 0
        local nextValue = value and start + step * direction or start
        nextValue = math.max(entry.min or -math.huge, math.min(entry.max or math.huge, nextValue))
        return entry.key, tonumber(("%.6g"):format(nextValue))
    end
    local options = entry.options or {}
    for index, option in ipairs(options) do
        if optionOf(option) == value then
            return entry.key, (optionOf(options[(index - 1 + direction) % #options + 1]))
        end
    end
    return entry.key, options[1] and (optionOf(options[1]))
end

-- Until the app starts, the curated places and nothing saved.
MyPlaces.init()

return MyPlaces
