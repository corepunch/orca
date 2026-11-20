-- The surfer's own log (SU10): each session with a one-tap rating and the
-- conditions the forecast gave for it, kept on this device only (Storage,
-- "surf-sessions"). Later sessions at the same break in like conditions
-- say how such days went. This is the start of personal tuning: for now the
-- log only speaks up, it does not move the score.
local Storage = require "Weather/storage"
local Logbook = { NAME = "surf-sessions", RATINGS = { good = true, fair = true, poor = true } }

local loaded = setmetatable({}, { __mode = "k" })

-- Every session, oldest first, from the app's store (read once per store).
function Logbook.sessions()
    local store = Storage.default()
    if not loaded[store] then
        local saved = store:load(Logbook.NAME)
        loaded[store] = type(saved) == "table" and type(saved.sessions) == "table" and saved.sessions or {}
    end
    return loaded[store]
end

-- Logs a session at a place for a forecast hour, rated "good", "fair" or
-- "poor". `conditions` is what the rules read then: { face, period, wind
-- ("offshore"…), tide ("low"…) }. Returns the entry, or nil and why.
function Logbook.add(place, hour, rating, conditions)
    if not Logbook.RATINGS[rating] then return nil, "rate it good, fair or poor" end
    local sessions = Logbook.sessions()
    local entry = { place = place.id, time = hour.time, epoch = hour.epoch, rating = rating }
    for key, value in pairs(conditions or {}) do entry[key] = value end
    sessions[#sessions + 1] = entry
    local ok, err = Storage.default():save(Logbook.NAME, { sessions = sessions })
    if not ok then return nil, err end
    return entry
end

-- Sessions at a place in like conditions: a similar size (within about a
-- third), period within 3 s, and the same wind against the beach.
function Logbook.similar(placeId, conditions)
    local list = {}
    for _, entry in ipairs(Logbook.sessions()) do
        if entry.place == placeId and entry.face and conditions.face
            and math.abs(entry.face - conditions.face) <= math.max(0.3, conditions.face / 3)
            and (not (entry.period and conditions.period) or math.abs(entry.period - conditions.period) <= 3)
            and entry.wind == conditions.wind then
            list[#list + 1] = entry
        end
    end
    return list
end

-- "3 sessions like this in your log, mostly good", or nil under two.
function Logbook.summary(sessions)
    if #sessions < 2 then return nil end
    local count = { good = 0, fair = 0, poor = 0 }
    for _, entry in ipairs(sessions) do count[entry.rating] = count[entry.rating] + 1 end
    local top = "good"
    for _, rating in ipairs { "fair", "poor" } do
        if count[rating] > count[top] then top = rating end
    end
    local word = count[top] == #sessions and "all" or count[top] * 2 > #sessions and "mostly" or "often"
    return ("%d sessions like this in your log, %s %s"):format(#sessions, word, top)
end

return Logbook
