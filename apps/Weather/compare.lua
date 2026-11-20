-- Compare my places for one activity (P5): which of the user's places is
-- best for it soon. Surfers' best break (Surf.compare), Sailors' weekend
-- (Rank) and Fishers' waters (editions/fishing/compare.lua) rank with it.
--
--   Compare.places("surf")               -- my places that offer surf
--   Compare.saved(place, epoch)          -- its forecast kept on the device, no request
--   Compare.rank(entries, best, options) -- entries rated by `best`, best first
--
-- "My places" are the saved ones, or the suggestions before any is saved
-- (MyPlaces.mine). Nothing here asks the network: each caller decides what
-- a comparison may cost (one cached forecast per place, the saved copies,
-- or one multi-point request).
local Compare = {}

local function offers(place, id)
    for _, other in ipairs(place.activities or {}) do if other == id then return true end end
    return false
end
Compare.offers = offers

-- The places offering an activity (an id or an activity), in the user's
-- order. `places` defaults to my places.
function Compare.places(activity, places)
    local id = type(activity) == "table" and activity.id or activity
    places = places or require("Weather/myplaces").mine()
    local list = {}
    for _, place in ipairs(places) do if offers(place, id) then list[#list + 1] = place end end
    return list
end

-- The forecast this device has kept for a place, as of `epoch`, replayed
-- from the saved answers (memoised for ten minutes per store); nil when
-- there is none or it no longer reaches now.
local memo = setmetatable({}, { __mode = "k" })
function Compare.saved(place, epoch, store)
    local Storage, Forecast, Activities = require "Weather/storage", require "Weather/forecast", require "Weather/activities"
    store = store or Storage.default()
    memo[store] = memo[store] or {}
    local kept = memo[store][place.id]
    if not (kept and kept.place == place and kept.epoch // 600 == epoch // 600) then
        local raw = store:load("forecast-" .. place.id)
        local forecast = raw and Forecast.restore(raw, place, Activities.request(place), epoch)
        if forecast then forecast.spot = place end
        kept = { epoch = epoch, place = place, forecast = forecast }
        memo[store][place.id] = kept
    end
    return kept.forecast
end

-- Rates each entry ({ place, forecast, … }) with `best(entry)`, which
-- returns a table { result, span, hour, later } or nil (nothing to rate).
-- The ranked entries carry the entry's fields and best's, sorted: what is
-- sooner first (`later` false before true), then the higher score, then
-- `options.tie(a, b)` or the title. `entry.failed` (a message) keeps a
-- place whose forecast is missing; such entries come last, in order.
function Compare.rank(entries, best, options)
    options = options or {}
    local ranked, failed = {}, {}
    for _, entry in ipairs(entries) do
        if entry.failed then failed[#failed + 1] = entry
        else
            local top = best(entry)
            if top and top.result then
                for key, value in pairs(entry) do if top[key] == nil then top[key] = value end end
                ranked[#ranked + 1] = top
            end
        end
    end
    table.sort(ranked, function(a, b)
        if (a.later or false) ~= (b.later or false) then return not a.later end
        if a.result.score ~= b.result.score then return a.result.score > b.result.score end
        if options.tie then
            local before, after = options.tie(a, b), options.tie(b, a)
            if before ~= after then return before end
        end
        return (a.place.title or a.place.id or "") < (b.place.title or b.place.id or "")
    end)
    for _, entry in ipairs(failed) do ranked[#ranked + 1] = entry end
    return ranked
end

return Compare
