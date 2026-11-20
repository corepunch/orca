-- What the user chose: units, the clock, and each activity's own values.
-- Every setting is a declaration, so the settings page builds itself:
--
--   Fishing.settings = {
--       { key = "moon", title = "moon in the score", kind = "toggle", default = false },
--       { key = "species", title = "species", kind = "choice", default = "bass",
--         options = { { "bass", "bass" }, { "trout", "trout" } } },
--       { key = "depth", title = "depth", kind = "number", default = 2, min = 0, max = 20, step = 0.5,
--         format = function(v) return Units.height(v) end },
--   }
--   Settings.get("fishing", "moon")   --> false until the user turns it on
--
-- Options are `{ value, title }` pairs or plain strings. A choice steps to the
-- next option, a toggle flips, a number moves by `step` between `min` and
-- `max`. The shared units and clock are the group "units"
-- (`Settings.get("units", "wind")`). Values are stored with Storage under
-- "settings"; changing one saves at once and applies the units.
local Units = require "Weather/units"
local Settings = { UNITS = "units" }

local state = { values = {}, registry = {}, order = {}, revision = 0 }

-- The shared units, in the order the page lists them.
local SHARED = {
    { key = "temp", title = "temperature" }, { key = "wind", title = "wind" },
    { key = "height", title = "waves and heights" }, { key = "rain", title = "rain" },
    { key = "pressure", title = "pressure" }, { key = "distance", title = "distance" },
    { key = "clock", title = "clock" },
}
for _, setting in ipairs(SHARED) do setting.kind, setting.options = "choice", Units.CHOICES[setting.key] end

local function option(entry)
    if type(entry) == "table" then
        local value = entry.value
        if value == nil then value = entry[1] end
        return value, tostring(entry.title or entry[2] or value)
    end
    return entry, tostring(entry)
end

-- An activity whose `settings` the page lists (activities.lua registers each).
-- The declarations are read when used, so they may change at run time.
function Settings.register(activity)
    if not state.registry[activity.id] then state.order[#state.order + 1] = activity.id end
    state.registry[activity.id] = activity
end

function Settings.declarations(group)
    if group == Settings.UNITS then return SHARED end
    local activity = state.registry[group]
    return activity and activity.settings or {}
end

local function declaration(group, key)
    for _, setting in ipairs(Settings.declarations(group)) do
        if setting.key == key then return setting end
    end
end

local function default(group, key, setting)
    if group == Settings.UNITS then return Units.defaults(state.edition, state.region)[key] end
    return setting and setting.default
end

-- A stored value if it still fits its declaration, else nil.
local function valid(setting, value)
    if not setting or value == nil then return nil end
    if setting.kind == "number" then
        if type(value) ~= "number" then return nil end
        return math.max(setting.min or -math.huge, math.min(setting.max or math.huge, value))
    end
    for _, entry in ipairs(setting.options or {}) do
        if option(entry) == value then return value end
    end
    return nil
end

local function applyUnits()
    local choice = {}
    for _, setting in ipairs(SHARED) do choice[setting.key] = Settings.get(Settings.UNITS, setting.key) end
    Units.use(choice)
end

-- options.storage (a Storage store; nil keeps settings in memory only),
-- options.edition and options.region pick the defaults. Loads what was
-- saved and applies the units.
function Settings.init(options)
    options = options or {}
    state.storage, state.edition, state.region = options.storage, options.edition, options.region
    local saved = state.storage and state.storage:load("settings")
    state.values = type(saved) == "table" and saved or {}
    state.revision = state.revision + 1
    applyUnits()
end

function Settings.get(group, key)
    local setting = declaration(group, key)
    local stored = state.values[group] and state.values[group][key]
    if setting and setting.kind == "toggle" then
        if type(stored) == "boolean" then return stored end
    else
        local value = valid(setting, stored)
        if value ~= nil then return value end
    end
    return default(group, key, setting)
end

-- Bumps whenever a value changes, so pages know to redraw.
function Settings.revision() return state.revision end

function Settings.set(group, key, value)
    local setting = declaration(group, key)
    if not setting then return false end
    if setting.kind == "toggle" then value = value == true
    else
        value = valid(setting, value)
        if value == nil then return false end
    end
    state.values[group] = state.values[group] or {}
    state.values[group][key] = value
    state.revision = state.revision + 1
    if group == Settings.UNITS then applyUnits() end
    if state.storage then state.storage:save("settings", state.values) end
    return true
end

-- One tap: the next option, the other side of a toggle, or a number one
-- step up (`direction` -1 steps down).
function Settings.step(group, key, direction)
    local setting = declaration(group, key)
    if not setting then return false end
    local value = Settings.get(group, key)
    if setting.kind == "toggle" then return Settings.set(group, key, not value) end
    if setting.kind == "number" then
        local next = (value or 0) + (setting.step or 1) * (direction or 1)
        return Settings.set(group, key, tonumber(("%.6g"):format(next)))
    end
    local options = setting.options or {}
    for index, entry in ipairs(options) do
        if option(entry) == value then
            return Settings.set(group, key, (option(options[(index - 1 + (direction or 1)) % #options + 1])))
        end
    end
    return #options > 0 and Settings.set(group, key, (option(options[1])))
end

-- The value as the page shows it.
function Settings.text(group, key)
    local setting, value = declaration(group, key), Settings.get(group, key)
    if not setting then return "" end
    if setting.format then return setting.format(value) end
    if setting.kind == "toggle" then return value and "on" or "off" end
    if setting.kind == "number" then return value == nil and "—" or tostring(value) end
    for _, entry in ipairs(setting.options or {}) do
        local v, title = option(entry)
        if v == value then return title end
    end
    return tostring(value)
end

-- Rows for the settings page (WeatherRow, Kind "setting"): the units, then
-- each listed activity's settings under its title (`note` on its first row).
function Settings.rows(activities)
    local rows = {}
    local function add(group, setting, heading)
        rows[#rows + 1] = {
            id = "settings:" .. group .. ":" .. setting.key, kind = "setting", order = #rows + 1,
            title = setting.title or setting.key, value = Settings.text(group, setting.key), detail = setting.detail or "",
            note = heading or "", activity = group, setting = group .. "/" .. setting.key,
            stepper = setting.kind == "number", tone = "info", score = 0,
        }
    end
    for _, setting in ipairs(SHARED) do add(Settings.UNITS, setting) end
    for _, activity in ipairs(activities or {}) do
        for index, setting in ipairs(activity.settings or {}) do add(activity.id, setting, index == 1 and activity.title) end
    end
    return rows
end

-- "fishing/moon" → "fishing", "moon".
function Settings.split(path) return (path or ""):match("^([^/]+)/(.+)$") end

return Settings
