-- Public guest snapshots and immutable image variants only. Session credentials
-- and authenticated timelines belong in memory, never in this optional cache.
local json = require "Octopus/api/json"
local Cache = {}
Cache.__index = Cache
local MAX_ENTRIES, MAX_BYTES, MAX_JSON = 128, 50 * 1024 * 1024, 1024 * 1024
local IMAGE_AGE = 7 * 86400
local extensions = { ".json", ".png", ".jpg", ".bin" }

local function hash(value)
    local result = 0xcbf29ce484222325
    for i = 1, #value do result = (result ~ value:byte(i)) * 0x100000001b3 end
    return string.format("%016x", result)
end

local function directoryExists(path)
    local fs = package.loaded["orca.filesystem"]
    if type(fs) == "table" and type(fs.fileExists) == "function" then return fs.fileExists(path .. "/.") end
    local file = io.open(path .. "/.", "r")
    if file then file:close(); return true end
    return false
end

local function defaultDirectory()
    local override = os.getenv("OCTOPUS_CACHE_DIR")
    if override and override ~= "" then return override end
    local xdg, home = os.getenv("XDG_CACHE_HOME"), os.getenv("HOME")
    if xdg and xdg:sub(1, 1) == "/" then return xdg .. "/octopus" end
    if not home or home == "" then return nil end
    local library = home .. "/Library"
    return home .. (directoryExists(library) and "/Library/Caches/octopus" or "/.cache/octopus")
end

local function makeDirectory(path)
    -- Native mkdir reports false for EEXIST. Check directory existence without
    -- renaming it: an existing parent can be readable but not writable.
    if directoryExists(path) then return true end
    local ok, fs = pcall(require, "orca.filesystem")
    return ok and fs.makeDirectory(path)
end

local function prepare(directory, mkdir)
    if type(directory) ~= "string" or directory:sub(1, 1) ~= "/" or directory:find("[%z\r\n]") then return nil end
    local path = ""
    for part in directory:gmatch("[^/]+") do
        if part == "." or part == ".." then return nil end
        path = path .. "/" .. part
        local ok, made = pcall(mkdir, path)
        if not ok or not made then return nil end
    end
    return path ~= "" and path or nil
end

local function readFile(path, limit)
    -- Update mode keeps a native file handle after ORCA installs its virtual
    -- read-only io.open wrapper, which has no seek and preloads whole files.
    local file = io.open(path, "r+b")
    if not file then return nil end
    local size = file:seek("end")
    if not size or size > limit then file:close(); return nil end
    file:seek("set", 0)
    local bytes = file:read("a")
    file:close()
    return bytes
end

local function validKey(key)
    return type(key) == "string" and #key > 0 and #key <= 512
end

function Cache:_filename(entry)
    return self.prefix .. string.format("%03d-%d", entry.slot, entry.generation) .. entry.extension
end

function Cache:_atomic(bytes, path)
    local ok, temporary = pcall(os.tmpname)
    if not ok then self.disabled = true; return nil end
    local staging = self.prefix .. "staging"
    -- tmpname creates a private inode. Moving it before writing preserves 0600
    -- permissions; a cross-filesystem move safely disables optional caching.
    if not os.rename(temporary, staging) then
        os.remove(temporary)
        self.disabled = true
        return nil
    end
    local file = io.open(staging, "wb")
    local written = file and file:write(bytes)
    local closed = file and file:close()
    if not written or not closed or not os.rename(staging, path) then
        os.remove(staging)
        self.disabled = true
        return nil
    end
    return path
end

function Cache:_save()
    local entries = {}
    for key, entry in pairs(self.entries) do
        entries[#entries + 1] = { key = key, slot = entry.slot, generation = entry.generation,
            extension = entry.extension, size = entry.size, at = entry.at, used = entry.used }
    end
    return self:_atomic(json.encode { version = 1, entries = entries }, self.prefix .. "index.json")
end

function Cache:_remove(key)
    local entry = self.entries[key]
    if not entry then return end
    os.remove(self:_filename(entry))
    self.entries[key] = nil
end

function Cache:_load()
    local bytes = readFile(self.prefix .. "index.json", MAX_JSON)
    local ok, index = pcall(json.decode, bytes)
    local live, count, total = {}, 0, 0
    if ok and type(index) == "table" and index.version == 1 and type(index.entries) == "table" then
        for _, entry in ipairs(index.entries) do
            if type(entry) ~= "table" then ok = false; break end
            local allowed = false
            for _, extension in ipairs(extensions) do allowed = allowed or entry.extension == extension end
            if not validKey(entry.key) or type(entry.slot) ~= "number" or entry.slot % 1 ~= 0
                    or entry.slot < 1 or entry.slot > self.maxEntries or self.generations[entry.slot] ~= nil
                    or (entry.generation ~= 0 and entry.generation ~= 1) or not allowed
                    or type(entry.size) ~= "number" or entry.size < 1 or entry.size % 1 ~= 0
                    or entry.size > self.maxBytes or type(entry.at) ~= "number" or entry.at < 0
                    or entry.at ~= entry.at or entry.at == math.huge or self.entries[entry.key] then
                ok = false
                break
            end
            entry.used = type(entry.used) == "number" and entry.used == entry.used and math.abs(entry.used) ~= math.huge and entry.used or entry.at
            count, total = count + 1, total + entry.size
            self.entries[entry.key], self.generations[entry.slot] = entry, entry.generation
            live[self:_filename(entry)] = true
        end
    else ok = false end
    if not ok or count > self.maxEntries or total > self.maxBytes then
        self.entries, self.generations, live = {}, {}, {}
    end
    -- Fixed slot names keep disk use bounded even after an interrupted write or
    -- corrupt index. Never use a filename supplied by cache JSON.
    for slot = 1, MAX_ENTRIES do
        for generation = 0, 1 do
            for _, extension in ipairs(extensions) do
                local path = self:_filename { slot = slot, generation = generation, extension = extension }
                if not live[path] then os.remove(path) end
            end
        end
    end
    os.remove(self.prefix .. "staging")
end

function Cache.new(options)
    options = options or {}
    local self = setmetatable({ entries = {}, generations = {}, clock = options.clock or os.time,
        maxEntries = math.min(MAX_ENTRIES, options.maxEntries or MAX_ENTRIES),
        maxBytes = math.min(MAX_BYTES, options.maxBytes or MAX_BYTES) }, Cache)
    local directory = prepare(options.directory or options.root or defaultDirectory(), options.mkdir or makeDirectory)
    if not directory or not validKey(options.namespace) or self.maxEntries < 1 or self.maxBytes < 1 then
        self.disabled = true
        return self
    end
    self.prefix = directory .. "/octopus-" .. hash(options.namespace) .. "-"
    self:_load()
    return self
end

function Cache:path(key, maxAge)
    if self.disabled or not validKey(key) then return nil end
    local entry, now = self.entries[key], self.clock()
    if not entry then return nil end
    local file = io.open(self:_filename(entry), "r+b")
    local size = file and file:seek("end")
    if file then file:close() end
    if size ~= entry.size or now < entry.at or now - entry.at > (maxAge or IMAGE_AGE) then
        self:discard(key)
        return nil
    end
    entry.used = now
    return self:_filename(entry)
end

function Cache:discard(key)
    if self.disabled or not self.entries[key] then return end
    self:_remove(key)
    self:_save()
end

function Cache:_put(bytes, key, extension)
    if self.disabled or not validKey(key) or type(bytes) ~= "string" or #bytes == 0 or #bytes > self.maxBytes then return nil end
    local old, used, count, total = self.entries[key], {}, 0, 0
    for name, entry in pairs(self.entries) do
        if name ~= key then
            used[entry.slot] = true
            count, total = count + 1, total + entry.size
        end
    end
    while count >= self.maxEntries or total + #bytes > self.maxBytes do
        local oldestKey, oldest
        for name, entry in pairs(self.entries) do
            if name ~= key and (not oldest or entry.used < oldest.used
                    or (entry.used == oldest.used and entry.slot < oldest.slot)) then oldestKey, oldest = name, entry end
        end
        if not oldest then return nil end
        used[oldest.slot] = nil
        count, total = count - 1, total - oldest.size
        self:_remove(oldestKey)
    end
    local slot = old and old.slot
    if not slot then for candidate = 1, self.maxEntries do if not used[candidate] then slot = candidate; break end end end
    local now = self.clock()
    local entry = { slot = slot, generation = 1 - (self.generations[slot] or 0),
        extension = extension, size = #bytes, at = now, used = now }
    local path = self:_atomic(bytes, self:_filename(entry))
    if not path then return nil end
    self.entries[key], self.generations[slot] = entry, entry.generation
    if not self:_save() then return nil end
    if old then os.remove(self:_filename(old)) end
    return path
end

function Cache:store(bytes, key)
    local extension = ".bin"
    if type(bytes) == "string" and bytes:sub(1, 3) == "\255\216\255" then extension = ".jpg"
    elseif type(bytes) == "string" and bytes:sub(1, 8) == "\137PNG\r\n\26\n" then extension = ".png" end
    return self:_put(bytes, key, extension)
end

function Cache:writeJSON(key, value)
    local ok, bytes = pcall(json.encode, value)
    if not ok or type(value) ~= "table" or #bytes > MAX_JSON then return nil end
    return self:_put(bytes, key, ".json")
end

function Cache:readJSON(key, maxAge)
    if self.disabled or not self.entries[key] or self.entries[key].extension ~= ".json" then return nil end
    local path = self:path(key, maxAge or 86400)
    local bytes = path and readFile(path, MAX_JSON)
    local ok, value = pcall(json.decode, bytes)
    if not ok or type(value) ~= "table" then self:discard(key); return nil end
    return value
end

return Cache
