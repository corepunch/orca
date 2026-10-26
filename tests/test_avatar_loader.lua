local orca = require "orca"
orca.init()
local ui = require "orca.UIKit"
local Loader = require "orca.UIKit.AvatarLoader"
local app = { sessionEpoch = 1, avatarPaths = {}, screen = ui.Screen { Width = 200, Height = 300, ResizeMode = "NoResize" } }
local list = ui.StackView { Overflow = "hidden scroll" }
app.screen:addChild(list)
local workers, lookups, calls, cache, paints = {}, {}, {}, {}, 0
local adapter = {
    lookup = function(ids)
        lookups[#lookups + 1] = ids
        local found = {}
        for _, id in ipairs(ids) do
            if id == "missing" then found[id] = false else found[id] = "file-" .. id end
        end
        return found
    end,
    cached = function(id) return cache[id] end,
    fetch = function(id)
        calls[id] = (calls[id] or 0) + 1
        coroutine.yield()
        if id == "file-bad" then return nil end
        return "jpeg-" .. id
    end,
    store = function(bytes, id) cache[id] = "/cache/" .. id; return cache[id] end,
    apply = function(node, path) node.Name = "loaded:" .. path end,
    schedule = function(fn) workers[#workers + 1] = coroutine.create(fn) end,
    repaint = function() paints = paints + 1 end,
}
local loader = Loader.new(app, adapter)
local nodes = {}
for _, id in ipairs { "a", "a", "b", "c", "d", "offscreen", "missing" } do
    local node = ui.Grid { Name = id, Height = 100 }
    list:addChild(node)
    loader:watch(node, id)
    nodes[#nodes + 1] = node
end
local function resume(worker)
    local ok, err = coroutine.resume(worker)
    assert(ok, err)
end
local function finish()
    local index = 1
    while index <= #workers do
        while coroutine.status(workers[index]) ~= "dead" do resume(workers[index]) end
        index = index + 1
    end
end
loader:loadVisible()
assert(#workers == 0, "Unlaid-out avatars must not request profiles")
app.screen:UpdateLayout(200, 300)
loader:loadVisible()
assert(#workers == 1, "Visible avatar metadata must use one batch")
resume(workers[1])
assert(#lookups == 1 and #lookups[1] == 4, "Only visible and nearby unique users belong in the batch")
assert(loader.active == 2 and #workers == 3, "Image concurrency is bounded to two")
resume(workers[2]); resume(workers[3])
assert(not calls["file-offscreen"] and not calls["file-missing"])
finish()
assert(calls["file-a"] == 1 and calls["file-b"] == 1 and calls["file-c"] == 1 and calls["file-d"] == 1)
assert(nodes[1].Name == nodes[2].Name and nodes[1].Name:match("^loaded:"), "Duplicates share one cached file")
assert(paints > 0)
local before = #workers
loader:loadVisible()
assert(#workers == before, "Completed avatar paints must not repeat network work")
-- Scrolling brings the next users into the prefetch band.
list.ContentOffset = { X = 0, Y = -300 }
app.screen:UpdateLayout(200, 300)
loader:loadVisible(); finish()
assert(calls["file-offscreen"] == 1 and app.avatarPaths.missing == false)
local cached = ui.Grid {}
loader:watch(cached, "a")
assert(cached.Name == nodes[1].Name, "Cached portraits apply immediately without another lookup")
-- Native scene ownership must survive Lua GC until a deferred request starts.
local gcList = ui.StackView { Overflow = "hidden scroll" }
app.screen:addChild(gcList)
do
    local node = ui.Grid { Height = 100 }
    gcList:addChild(node); loader:watch(node, "gc")
end
collectgarbage("collect"); collectgarbage("collect")
app.screen:UpdateLayout(200, 300)
loader:loadVisible(); finish()
assert(calls["file-gc"] == 1)
-- Failure keeps initials and does not spin on paints; a new view may retry.
local failed = ui.Grid { Height = 100 }
gcList:addChild(failed); loader:watch(failed, "bad")
app.screen:UpdateLayout(200, 300)
loader:loadVisible(); finish()
assert(calls["file-bad"] == 1 and not app.avatarPaths.bad)
loader:loadVisible(); finish()
assert(calls["file-bad"] == 1)
loader:watch(failed, "bad")
loader:loadVisible(); finish()
assert(calls["file-bad"] == 2)
-- Changing a profile photo invalidates already-running work for that identity.
local changed = ui.Grid { Height = 100 }
gcList:addChild(changed); loader:watch(changed, "changed")
app.screen:UpdateLayout(200, 300)
loader:loadVisible()
resume(workers[#workers])
local oldPhoto = workers[#workers]; resume(oldPhoto)
loader:invalidate("changed")
resume(oldPhoto)
assert(not app.avatarPaths.changed and not changed.Name:match("^loaded:"),
    "An old profile-photo response must not overwrite a changed profile")
-- A session change cannot populate a new user's media cache or tiles.
local stale = ui.Grid { Height = 100 }
gcList:addChild(stale); loader:watch(stale, "stale")
app.screen:UpdateLayout(200, 300)
loader:loadVisible()
local metadata = workers[#workers]; resume(metadata)
local fetch = workers[#workers]; resume(fetch)
assert(calls["file-stale"] == 1)
app.sessionEpoch = 2
resume(fetch)
assert(not cache["file-stale"] and not app.avatarPaths.stale and not stale.Name:match("^loaded:"))
loader:clear()
print("PASS: avatar visibility/prefetch, metadata batching, bounded downloads, deduplication, cache, GC, retry and stale sessions")
