-- Standalone, offline persistence tests; no native engine or shell commands.
package.preload["Octopus/api/json"] = function() return dofile("apps/octopus/api/json.lua") end
local Cache = dofile("apps/octopus/cache.lua")
local Media = dofile("apps/octopus/media.lua")
-- The same file also runs inside ORCA, including its virtual io.open wrapper.
local nativeOK, nativeFS = pcall(require, "orca.filesystem")
local now, caches = 1000000, {}
local directory = os.getenv("TMPDIR") or "/tmp"
local unique = os.tmpname()
os.remove(unique)
local function new(suffix, options)
    options = options or {}
    options.namespace, options.directory = unique .. suffix, directory
    options.clock, options.mkdir = function() return now end, function() return true end
    local cache = Cache.new(options)
    caches[#caches + 1] = cache
    return cache
end
local function read(path)
    local file = assert(io.open(path, "r+b"))
    local bytes = file:read("a")
    file:close()
    return bytes
end
local function rewrite(path, bytes)
    local file = assert(io.open(path, "wb"))
    assert(file:write(bytes))
    assert(file:close())
end
local png = "\137PNG\r\n\26\n" .. "a test image"
local jpg = "\255\216\255" .. "a different image"
local cache = new("public")
local avatar = assert(cache:store(png, "avatar:same-file"))
local photo = assert(cache:store(jpg, "photo:same-file"))
assert(avatar ~= photo and avatar:sub(-4) == ".png" and photo:sub(-4) == ".jpg", "Image variants need separate keys and native extensions")
assert(read(avatar) == png and read(photo) == jpg)
assert(cache:writeJSON("discover", { items = { { id = "post", authorName = "alice" } }, cursor = "post" }))
local restarted = new("public")
assert(restarted:path("avatar:same-file") == avatar and restarted:path("photo:same-file") == photo, "Cache must survive another instance")
local saved = assert(restarted:readJSON("discover", 60))
assert(saved.cursor == "post" and saved.items[1].authorName == "alice")
saved.items[1].authorName = "changed"
assert(restarted:readJSON("discover", 60).items[1].authorName == "alice", "Reads must return fresh JSON tables")
local other = new("another-project")
assert(not other:path("avatar:same-file") and not other:readJSON("discover"), "Projects/endpoints must not share cached data")
local media = Media.new { cache = restarted }
assert(media:path("avatar:same-file") == avatar)
media:clear()
assert(restarted:path("avatar:same-file") == avatar, "Media cleanup must preserve persistent files")
media:discard("avatar:same-file")
assert(not restarted:path("avatar:same-file") and not io.open(avatar, "rb"), "Decode failure must discard persistent bytes")
local ephemeral = Media.new()
local temporary = assert(ephemeral:store(png, "temporary"))
ephemeral:clear()
assert(not io.open(temporary, "rb"), "Default Media instances must remain temporary")
now = now + 61
assert(not restarted:readJSON("discover", 60), "Feed snapshots must expire")
now = now + 7 * 86400
assert(not restarted:path("photo:same-file"), "Images expire after seven days")

local bounded = new("bounded", { maxEntries = 2, maxBytes = 16 })
assert(bounded:store("12345678", "one"))
now = now + 1
local two = assert(bounded:store("abcdefgh", "two"))
now = now + 1
assert(bounded:path("one"))
now = now + 1
assert(bounded:store("87654321", "three"))
assert(not bounded:path("two") and not io.open(two, "rb"), "Entry eviction must remove the least recently used bytes")
assert(bounded:path("one") and bounded:path("three"))
assert(not bounded:store(string.rep("x", 17), "oversize"), "Oversize writes must not exceed the disk bound")
local reopened = new("bounded", { maxEntries = 2, maxBytes = 16 })
assert(reopened:path("one") and reopened:path("three"), "Eviction must persist")
local bytes = new("bytes", { maxEntries = 3, maxBytes = 12 })
assert(bytes:store("12345678", "one"))
assert(bytes:store("abcdefgh", "two"))
assert(not bytes:path("one") and bytes:path("two"), "Byte bounds must evict even below the entry bound")

local corrupt = new("corrupt")
local invalid = assert(corrupt:writeJSON("discover", { title = "valid" }))
rewrite(invalid, string.rep("!", #read(invalid)))
assert(not corrupt:readJSON("discover"), "Invalid JSON must be a cache miss")
local image = assert(corrupt:store(png, "photo"))
rewrite(corrupt.prefix .. "index.json", '{"version":1,"entries":[true]}')
local repaired = new("corrupt")
assert(not repaired:path("photo") and not io.open(image, "rb"), "Corrupt metadata must remove only known cache slots")
assert(repaired:store(png, "fresh"), "A corrupt index may safely recover as an empty cache")
assert(not repaired:writeJSON("large", { bytes = string.rep("x", 1024 * 1024) }), "Snapshot JSON is capped at one MiB")
local cycle = {}; cycle.self = cycle
assert(not repaired:writeJSON("cycle", cycle), "Invalid JSON values must not throw")
local dated = assert(repaired:store(png, "clock"))
now = now - 1
assert(not repaired:path("clock") and not io.open(dated, "rb"), "Clock rollback must not retain data forever")

local atomic = new("atomic")
local old = assert(atomic:store("old bytes", "photo"))
local originalRename = os.rename
os.rename = function(from, to)
    if to == atomic.prefix .. "index.json" then return nil end
    return originalRename(from, to)
end
assert(not atomic:store("new bytes", "photo") and atomic.disabled)
os.rename = originalRename
local recovered = new("atomic")
assert(recovered:path("photo") == old and read(old) == "old bytes", "Interrupted index writes must preserve old bytes and never mislabel a replacement")

local nativeRoot
if nativeOK and type(nativeFS.makeDirectory) == "function" then
    nativeRoot = unique .. "-cache-folder"
    local nested = Cache.new { namespace = unique .. "native-folders", directory = nativeRoot .. "/native/nested" }
    caches[#caches + 1] = nested
    local path = assert(nested:store(png, "photo"))
    local again = Cache.new { namespace = unique .. "native-folders", directory = nativeRoot .. "/native/nested" }
    caches[#caches + 1] = again
    assert(again:path("photo") == path, "Native mkdir must create missing folders and accept existing parents on restart")
end

package.preload["orca.filesystem"] = function()
    return { makeDirectory = function() return false end }
end
local nativeMkdir = Cache.new { namespace = unique .. "existing-parents", directory = directory }
caches[#caches + 1] = nativeMkdir
assert(not nativeMkdir.disabled and nativeMkdir:store(png, "photo"), "Existing parents must work even when native mkdir returns false for EEXIST")

local unavailable = Cache.new { namespace = unique, directory = "/not/a/cache", mkdir = function() return false end }
assert(not unavailable:store(png, "image") and not unavailable:path("image") and not unavailable:writeJSON("discover", {}))
local blocked = Cache.new { namespace = unique, directory = unique .. "/absent", mkdir = function() return true end }
assert(not blocked:store(png, "image") and blocked.disabled, "Unwritable or missing directories must disable caching gracefully")
local fallback = Media.new { cache = blocked }
local privateTemporary = assert(fallback:store(png, "fallback"))
assert(read(privateTemporary) == png, "Disabled persistent storage must retain the private temporary fallback")
fallback:clear()
assert(not io.open(privateTemporary, "rb"))

for _, item in ipairs(caches) do
    local keys = {}
    for key in pairs(item.entries) do keys[#keys + 1] = key end
    for _, key in ipairs(keys) do item:discard(key) end
    os.remove(item.prefix .. "index.json")
    os.remove(item.prefix .. "staging")
end
if nativeRoot then
    os.remove(nativeRoot .. "/native/nested")
    os.remove(nativeRoot .. "/native")
    os.remove(nativeRoot)
end
print("PASS: bounded public snapshot/image persistence, namespace isolation, TTL, corruption recovery and private temporary fallback")
