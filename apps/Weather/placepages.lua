-- The places pages (wave 3): the hub's place rows, "add a place" (search)
-- and the place editor. `PlacePages.install(app)` adds their operations,
-- filters and handlers to the app's provider; App.lua only routes. What the
-- editor edits is `app.editing`: { id } for a saved place, { draft } for a
-- place found or suggested and not saved yet.
local MyPlaces = require "Weather/myplaces"
local Search = require "Weather/search"
local Locate = require "Weather/locate"
local Activities = require "Weather/activities"
local Units = require "Weather/units"
local PlacePages = {}

local function kind(name) return function(record) return record.values.Kind == name end end

-- A place as a WeatherPlace row: saved ones first (the first is home).
function PlacePages.rows(places)
    local rows, first = {}, true
    for _, place in ipairs(places) do
        local saved = MyPlaces.isSaved(place.id)
        local line = place.region or ""
        if saved and first then line = line ~= "" and "home · " .. line or "home"; first = false end
        rows[#rows + 1] = { id = place.id, title = place.title, region = place.region, kind = saved and "mine" or "suggestion",
            line = line, link = saved and "edit" or "keep" }
    end
    return rows
end

local function where(place)
    local parts = {}
    if place.lat and place.lon then
        parts[#parts + 1] = ("%.2f° %s, %.2f° %s"):format(math.abs(place.lat), place.lat >= 0 and "N" or "S",
            math.abs(place.lon), place.lon >= 0 and "E" or "W")
    end
    if place.elevation then parts[#parts + 1] = Units.height(place.elevation, 0) .. " up" end
    return table.concat(parts, " · ")
end

local KIND_WORDS = { town = "a town", coast = "a coast", lake = "a lake", river = "a river", mountain = "a mountain" }

-- The editor's rows for a place: where it is, its fields under their
-- activity's heading, and the links (save for a draft; home, move and
-- remove for a saved place).
function PlacePages.editorRows(place, options)
    options = options or {}
    local rows = {}
    local function row(fields)
        fields.order, fields.tone, fields.score = #rows + 1, fields.tone or "info", 0
        rows[#rows + 1] = fields
    end
    row { id = "place:about", kind = "about", title = place.region ~= "" and place.region or place.title,
        detail = where(place) }
    if options.draft and not place.curated then
        row { id = "place:guess", kind = "about", title = "guessed: " .. (KIND_WORDS[place.kind] or "a place"),
            detail = "Check what it is and what it is good for below; you can change it later." }
    end
    local group
    for _, field in ipairs(MyPlaces.fields(place)) do
        local heading = ""
        if field.group ~= group then
            group = field.group
            local activity = Activities.byId[group]
            heading = activity and activity.title or (group ~= "place" and group) or ""
        end
        local detail = field.detail or ""
        if type(detail) == "function" then detail = detail(place) end
        local value = MyPlaces.value(place, field)
        if value == nil and field.needed then detail = field.needed end
        row { id = "place:field:" .. field.key, kind = "field", title = field.title or field.key,
            value = MyPlaces.text(place, field), detail = detail, note = heading, setting = field.key,
            stepper = field.kind == "number" or field.kind == "compass" }
    end
    local actions = {}
    if options.draft then
        actions[#actions + 1] = { "save", MyPlaces.isSaved(place.id) and "done" or "save this place" }
        if #MyPlaces.saved() > 0 then actions[#actions + 1] = { "save-home", "save as home" } end
    else
        local list, index = MyPlaces.saved(), 0
        for i, other in ipairs(list) do if other.id == place.id then index = i end end
        if index > 1 then
            actions[#actions + 1] = { "home", "make this home" }
            actions[#actions + 1] = { "up", "move up" }
        end
        if index > 0 and index < #list then actions[#actions + 1] = { "down", "move down" } end
        actions[#actions + 1] = { "remove", options.confirmRemove and "tap again to remove " .. place.title
            or "remove this place" }
    end
    for _, action in ipairs(actions) do
        row { id = "place:action:" .. action[1], kind = "placeaction", title = action[2], action1 = action[1],
            tone = action[1] == "remove" and options.confirmRemove and "poor" or "info" }
    end
    return rows
end

-- A search's rows: the places found, then the app's own that match (or,
-- offline, only those), and a note when there is something to say.
function PlacePages.searchRows(text, fetch, drafts)
    local clean = Search.clean(text)
    if not clean then
        return { { id = "note", kind = "note", title = "Type at least two letters: a town, a lake, a peak or a postcode." } }
    end
    local found, err = Search.run(clean, fetch)
    local rows, seen = {}, {}
    for _, draft in ipairs(found) do
        local place = MyPlaces.resolveDraft(draft)
        if place and not seen[draft.id] then
            seen[draft.id] = true
            drafts[draft.id] = draft
            local line = MyPlaces.line(draft.base)
            if MyPlaces.isSaved(draft.id) then line = "in your places · " .. line end
            rows[#rows + 1] = { id = draft.id, title = place.title, region = place.region, kind = "result", line = line }
        end
    end
    for _, place in ipairs(Search.known(clean)) do
        if not seen[place.id] then
            seen[place.id] = true
            rows[#rows + 1] = { id = place.id, title = place.title, region = place.region, kind = "result",
                line = (MyPlaces.isSaved(place.id) and "in your places · " or "suggested · ") .. (place.region or "") }
        end
    end
    if err then
        table.insert(rows, 1, { id = "note", kind = "note", title = #rows > 0
            and "No connection, so only the app's own places are searched."
            or "No connection: search needs the network. Your saved places still work offline." })
    elseif #rows == 0 then
        rows[1] = { id = "note", kind = "note", title = ("Nothing found for “%s”. Try the nearest town, or a postcode."):format(clean) }
    end
    return rows
end

function PlacePages.install(app)
    local provider = app.provider
    local drafts = {}
    provider.filters.minePlaces = kind("mine")
    provider.filters.suggestedPlaces = kind("suggestion")
    provider.filters.resultRows = kind("result")
    provider.filters.noteRows = kind("note")
    provider.filters.fieldRows = kind("field")
    provider.filters.placeActionRows = kind("placeaction")
    provider.computed.hasLink = function(values) return (values.Link or "") ~= "" end

    provider.operations.places = function() return PlacePages.rows(app.places) end
    provider.operations.searchPlaces = function(params) return PlacePages.searchRows(params.Query, app.fetch, drafts) end
    -- "use my location" (N1): offered where the build can ask; the fix
    -- shows on the search page as a result, or a calm note.
    provider.filters.offerRows = kind("offer")
    provider.operations.locationOffer = function()
        if not Locate.available(app.location) then return {} end
        return { { id = "locate", kind = "offer", title = "use my location", line = "" } }
    end
    provider.operations.locatePlace = function() return Locate.rows(app.location, drafts) end
    provider.operations.suggestedPlaces = function()
        local rows = {}
        for _, place in ipairs(MyPlaces.suggestions()) do
            rows[#rows + 1] = { id = place.id, title = place.title, region = place.region, kind = "result",
                line = place.region or "" }
        end
        return rows
    end
    provider.operations.placeEditor = function()
        local place, draft = app:editedPlace()
        if not place then return nil, { code = "no_place", message = "This place is gone." } end
        return PlacePages.editorRows(place, { draft = draft, confirmRemove = app.editing and app.editing.confirmRemove })
    end

    local handlers = provider.handlers
    handlers.addPlace = function() return app:openSearch() end
    -- From the hub's places: open search and look there.
    handlers.locateFromHub = function()
        app.locateOnOpen = true
        return app:openSearch()
    end
    handlers.useLocation = function(scope) scope:member("Located"):refresh { force = true } end
    handlers.placeLink = function(scope, params)
        local values = params.item and params.item.values
        if not values then return end
        if values.Kind == "mine" then return app:editPlace(values.Id, "/") end
        return app:keepPlace(values.Id)
    end
    handlers.runSearch = function(scope, params)
        local sender = params.sender
        local field = sender and (sender.Name == "SearchInput" and sender or sender:getParent():findChild("SearchInput", true))
        local text = field and field.Text or ""
        local query = scope:member("Results")
        if not query:set("Query", text) then query:refresh { force = true } end
    end
    handlers.chooseResult = function(_, params)
        local id = params.item and params.item.values.Id
        if not id then return end
        if MyPlaces.isSaved(id) then return app:editPlace(id, "/search") end
        local draft = drafts[id] or MyPlaces.draft(id)
        return app:editDraft(draft, "/search")
    end
    handlers.changeField = function(scope, params) return app:changeField(scope, params.item, 1) end
    handlers.lowerField = function(scope, params) return app:changeField(scope, params.item, -1) end
    handlers.placeAction = function(scope, params)
        local id = params.item and params.item.values.Action1
        return id and app:placeAction(scope, id)
    end
end

return PlacePages
