-- Small local files: settings, the last forecast per place, logs. Each file
-- holds one plain Lua table (strings, numbers, booleans and nested tables)
-- written as Lua source, so it is readable and needs no JSON encoder.
--
--   local store = Storage.new { dir = "/tmp/weather-test" }
--   store:save("settings", { units = { wind = "kn" } })
--   store:load("settings")          --> the table, or nil, err
--
-- Writes go to a temporary file that is then renamed over the old one, so a
-- crash never leaves half a file. Reads run the text with no environment
-- and an instruction budget, so a file can hold data but no code. Without
-- `dir` the folder is `$HOME/Library/Application Support/<app name>/`, one
-- per edition, so editions never share files. `Storage.load/save` use that
-- default folder; `Storage.use(store)` replaces it (tests).
local Storage = {}
local Store = {}
Store.__index = Store

local MAX_BYTES = 8 * 1024 * 1024
local BUDGET = 50 * 1000 * 1000 -- VM instructions a file may take to read

local function validName(name)
    return type(name) == "string" and name:match("^[%w][%w%-_%.]*$") ~= nil and not name:find("%.%.")
end

-- Lua source for a value. Keys are sorted so the same table always writes
-- the same text; functions, userdata, NaN and cycles are left out.
local function serialize(value, out, indent, seen)
    local kind = type(value)
    if kind == "string" then out[#out + 1] = ("%q"):format(value)
    elseif kind == "boolean" then out[#out + 1] = tostring(value)
    elseif kind == "number" then
        if value ~= value then out[#out + 1] = "nil"
        elseif value == math.huge then out[#out + 1] = "1e9999"
        elseif value == -math.huge then out[#out + 1] = "-1e9999"
        elseif math.type(value) == "integer" then out[#out + 1] = ("%d"):format(value)
        else out[#out + 1] = ("%.17g"):format(value) end
    elseif kind == "table" and not seen[value] then
        seen[value] = true
        local keys = {}
        for key, item in pairs(value) do
            local k, v = type(key), type(item)
            if (k == "string" or k == "number") and (v == "string" or v == "number" or v == "boolean" or v == "table") then
                keys[#keys + 1] = key
            end
        end
        table.sort(keys, function(a, b)
            if type(a) == type(b) then return a < b end
            return type(a) == "number"
        end)
        local inner = indent .. "  "
        out[#out + 1] = "{"
        for _, key in ipairs(keys) do
            out[#out + 1] = "\n" .. inner
            if type(key) == "string" and key:match("^[%a_][%w_]*$") then out[#out + 1] = key
            else out[#out + 1] = "["; serialize(key, out, inner, seen); out[#out + 1] = "]" end
            out[#out + 1] = " = "
            serialize(value[key], out, inner, seen)
            out[#out + 1] = ","
        end
        out[#out + 1] = #keys > 0 and "\n" .. indent .. "}" or "}"
        seen[value] = nil
    else out[#out + 1] = "nil" end
end

function Storage.serialize(value)
    local out = { "return " }
    serialize(value, out, "", {})
    out[#out + 1] = "\n"
    return table.concat(out)
end

-- The table a file's text describes, or nil, err. No globals, no code.
function Storage.deserialize(text, name)
    if type(text) ~= "string" or #text > MAX_BYTES then return nil, "too large" end
    local chunk, err = load(text, "=" .. (name or "storage"), "t", {})
    if not chunk then return nil, err end
    local hooked = debug and debug.sethook and debug.gethook
    local previous = hooked and { debug.gethook() }
    if hooked then debug.sethook(function() error("takes too long to read", 2) end, "", BUDGET) end
    local ok, value = pcall(chunk)
    if hooked then debug.sethook(table.unpack(previous)) end
    if not ok then return nil, value end
    if type(value) ~= "table" then return nil, "not a table" end
    return value
end

local function exists(path)
    local file = io.open(path, "r+b")
    if file then file:close(); return true end
    return false
end

-- Makes every missing folder along an absolute path.
local function makeDirectories(path)
    local ok, fs = pcall(require, "orca.filesystem")
    if not (ok and type(fs) == "table" and fs.makeDirectory) then return false end
    local current = ""
    for part in path:gmatch("[^/]+") do
        current = current .. "/" .. part
        if not (fs.fileExists and fs.fileExists(current .. "/.")) then pcall(fs.makeDirectory, current) end
    end
    return fs.fileExists == nil or fs.fileExists(path .. "/.")
end

-- `$HOME/Library/Application Support/<name>` on iOS and macOS; elsewhere
-- `$XDG_DATA_HOME/<name>` or `$HOME/.local/share/<name>`.
function Storage.defaultDir(name)
    local home = os.getenv("HOME")
    if not home or home == "" then return nil end
    local ok, fs = pcall(require, "orca.filesystem")
    local apple = PLATFORM == "ios" or PLATFORM == "macos"
        or (ok and fs.fileExists and fs.fileExists(home .. "/Library/."))
    if apple then return home .. "/Library/Application Support/" .. name end
    local xdg = os.getenv("XDG_DATA_HOME")
    return (xdg and xdg:sub(1, 1) == "/" and xdg or home .. "/.local/share") .. "/" .. name
end

-- options.dir: the folder; options.name: the app's name for the default
-- folder ("Weather", "Surfers' Weather").
function Storage.new(options)
    options = options or {}
    local dir = options.dir or Storage.defaultDir(options.name or "Weather")
    return setmetatable({ dir = dir and (dir:gsub("/+$", "")) }, Store)
end

function Store:path(name)
    if not (self.dir and validName(name)) then return nil end
    return self.dir .. "/" .. name .. ".lua"
end

function Store:load(name)
    local path = self:path(name)
    if not path then return nil, "no folder or bad name" end
    -- Update mode gets a native handle: ORCA's io.open wrapper serves plain
    -- reads from its package file system, which may hold an older copy.
    local file = io.open(path, "r+b")
    if not file then return nil, "missing" end
    local text = file:read("a")
    file:close()
    return Storage.deserialize(text, name)
end

function Store:save(name, value)
    local path = self:path(name)
    if not path or type(value) ~= "table" then return false, "no folder or bad name" end
    if not self.ready then self.ready = makeDirectories(self.dir) end
    if not self.ready then return false, "cannot make " .. self.dir end
    local temporary = path .. ".tmp"
    local file = io.open(temporary, "wb")
    if not file then return false, "cannot write " .. temporary end
    local written = file:write(Storage.serialize(value))
    local closed = file:close()
    if not (written and closed and os.rename(temporary, path)) then
        os.remove(temporary)
        return false, "cannot write " .. path
    end
    return true
end

function Store:remove(name)
    local path = self:path(name)
    return path ~= nil and exists(path) and os.remove(path) ~= nil
end

-- The app's shared default store (Storage.use replaces it).
local default
function Storage.use(store) default = store end
function Storage.default()
    if not default then
        local ok, Activities = pcall(require, "Weather/activities")
        default = Storage.new { name = ok and Activities.appName or "Weather" }
    end
    return default
end
function Storage.load(name) return Storage.default():load(name) end
function Storage.save(name, value) return Storage.default():save(name, value) end

return Storage
