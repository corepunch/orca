-- The fishers' log page (editions/fishing/Pages/Log.xml over Data/Log.xml):
-- a Pivot of "log" (one tap for a catch or a blank, the water reading, the
-- entries, undo and export), "waters" (my places compared, FI10) and
-- "patterns" (what my catches share, FI7). Rows are WeatherRows, so the
-- shared templates show them.
--
-- The activity declares it as `Fishing.page`; the app opens the route with
--   Page.provider { forecastFor = provider.forecastFor, places = places,
--       place = function() return app.place end, clock = os.time, back = function() … end }
local Sky = require "Weather/sky"
local Units = require "Weather/units"
local Fishing = require "Weather/activities/fishing"
local Log = require "Weather/editions/fishing/log"
local Compare = require "Weather/editions/fishing/compare"
local Page = {}

local function kind(name) return function(record) return record.values.Kind == name end end

-- An entry as a row: when and what, then the conditions it saved.
function Page.entryRow(entry, order)
    local title = entry.kind == "skunk" and "no fish" or (entry.species and entry.species ~= "any" and entry.species or "a catch")
    local parts = { Units.pressure(entry.pressure) }
    if entry.trend3 then parts[1] = parts[1] .. " " .. (entry.trend3 > 0.5 and "rising" or entry.trend3 < -0.5 and "falling" or "steady") end
    parts[#parts + 1] = "wind " .. Units.wind(entry.wind)
    if entry.water then parts[#parts + 1] = "water " .. Units.temp(entry.water) end
    if entry.tide then parts[#parts + 1] = "tide " .. entry.tide end
    parts[#parts + 1] = entry.moon
    return { id = "log:entry:" .. entry.id, kind = "entry", order = order, title = title,
        value = Sky.weekday(entry.time:sub(1, 10)) .. " " .. Units.clock(entry.time),
        detail = (entry.placeTitle or "") .. " · " .. table.concat(parts, " · "),
        tone = entry.kind == "skunk" and "info" or "good", icon = entry.code and Sky.icon(entry.code, entry.isDay) or "",
        score = entry.score or 0 }
end

function Page.patternRows()
    local rows = {}
    local patterns = Log.patterns()
    if not patterns.ready then
        rows[1] = { id = "log:pattern:wait", kind = "pattern", order = 1, tone = "info", score = 0,
            title = ("%d of %d catches"):format(patterns.count, Log.PATTERN_MINIMUM),
            detail = "Log a few more and this page shows the conditions your own catches share. Blanks count too." }
        return rows
    end
    for index, pattern in ipairs(patterns.patterns) do
        rows[#rows + 1] = { id = "log:pattern:" .. pattern.key, kind = "pattern", order = index, tone = "info", score = 0,
            title = pattern.title .. ": " .. pattern.band,
            detail = ("%d of your %d catches. Your own water beats anyone's lore."):format(pattern.count, pattern.of) }
    end
    return rows
end

function Page.provider(options)
    local status = ""
    local clock = options.clock or os.time
    local function place() return options.place and options.place() end
    local function forecast()
        local p = place()
        if not p then return nil, "Choose a place first." end
        local f, err = options.forecastFor(p.id)
        if not f then return nil, type(err) == "table" and err.message or err end
        return f
    end
    local function refresh(scope) scope:member("Log"):refresh { force = true } end
    local function add(scope, entry)
        local f, err = forecast()
        local saved
        if f then saved, err = Log.add(f, place(), entry, clock()) end
        status = saved and ("saved %s at %s: %s, wind %s"):format(saved.kind == "skunk" and "a blank" or "a catch",
            Units.clock(saved.time), Units.pressure(saved.pressure), Units.wind(saved.wind)) or ("not saved: " .. tostring(err))
        refresh(scope)
    end
    -- The reading moves by one degree in the user's own scale.
    local function stepReading(scope, direction)
        local p = place()
        if not p then return end
        local now = clock()
        local reading = Log.reading(p.id, now)
        local start = reading and reading.temp
        if not start then
            local f = forecast()
            local water = f and Fishing.water(f.hours[f.now], f)
            start = water and water.temp or 10
            direction = 0
        end
        local step = Units.current("temp") == "f" and 5 / 9 or 1
        Log.setReading(p.id, math.floor((start + direction * step) * 10 + 0.5) / 10, now)
        status = ""
        refresh(scope)
    end

    return {
        computed = require("Weather/provider").computed,
        filters = { statusRows = kind("status"), readingRows = kind("reading"), entryRows = kind("entry"),
            patternRows = kind("pattern"), waterRows = kind("water") },
        handlers = {
            logCatch = function(scope) add(scope, { kind = "catch", species = Fishing.setting("species") }) end,
            logSkunk = function(scope) add(scope, { kind = "skunk" }) end,
            undoLog = function(scope)
                local removed = Log.undo()
                status = removed and "took back the last entry" or "nothing to take back"
                refresh(scope)
            end,
            exportLog = function(scope)
                local path, err = Log.export()
                status = path and ("%d entries saved as %s"):format(#Log.entries(), path) or ("not exported: " .. tostring(err))
                refresh(scope)
            end,
            warmerWater = function(scope) stepReading(scope, 1) end,
            coolerWater = function(scope) stepReading(scope, -1) end,
            clearWater = function(scope)
                local p = place()
                if p then Log.setReading(p.id, nil) end
                refresh(scope)
            end,
            goBack = function() return options.back and options.back() end,
        },
        operations = {
            fishingLog = function()
                local rows, p, now = {}, place(), clock()
                local function push(row) row.order = #rows + 1; rows[#rows + 1] = row end
                push { id = "log:status", kind = "status", title = p and p.title or "", detail = status, tone = "info", score = 0 }
                local reading = p and Log.reading(p.id, now)
                local f = forecast()
                local model = f and Fishing.water(f.hours[f.now], f)
                if model and model.kind == "reading" then model = nil end
                push { id = "log:reading", kind = "reading", title = "my water reading",
                    value = reading and Units.temp(reading.temp) or "not set", tone = "info", score = 0,
                    detail = (reading and "kept for three days and used in the score" or "tap warmer or cooler to set it")
                        .. (model and (" · %s says %s"):format(model.short, Units.temp(model.temp)) or "") }
                local entries = Log.entries()
                for index, entry in ipairs(entries) do push(Page.entryRow(entry, index)) end
                for _, row in ipairs(Page.patternRows()) do push(row) end
                return rows
            end,
            fishingWaters = function()
                local rows = {}
                for index, entry in ipairs(Compare.rank(Fishing, options.places or {}, options.forecastFor)) do
                    if entry.failed then
                        rows[#rows + 1] = { id = "waters:" .. entry.place.id, kind = "water", order = index,
                            title = entry.place.title, value = "no forecast", detail = entry.failed, tone = "info", score = 0 }
                    else
                        local reason = entry.result.reasons[1]
                        rows[#rows + 1] = { id = "waters:" .. entry.place.id, kind = "water", order = index,
                            title = entry.place.title, value = entry.result.verdict, score = entry.result.score,
                            tone = entry.result.verdict, icon = Sky.icon(entry.hour.code, entry.hour.isDay),
                            detail = ("%s %s%s"):format(entry.label, entry.span, reason and " · " .. reason.text or "") }
                    end
                end
                return rows
            end,
        },
    }
end

return Page
