local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
local ui = require "orca.UIKit"
local renderer = require "orca.renderer"
local native = os.getenv("ORCA_OCTOPUS_NATIVE") == "1"
if native then renderer.init(200, 450, true) end
fs.init("apps/octopus")
local Loader = require "Octopus/photo_loader"
local Media = require "Octopus/media"
local f = assert(io.open("tests/fixtures/chat/photo.jpg", "rb"))
local jpeg = f:read("*a"); f:close()
local workers, calls = {}, {}
local app = { sessionEpoch = 1, screen = ui.Screen { Width = 200, Height = 450, ResizeMode = "NoResize" }, media = Media.new() }
function app:task(fn) workers[#workers + 1] = coroutine.create(fn) end
function app:repaint() end
app.client = { getImage = function(_, key)
    calls[key] = (calls[key] or 0) + 1
    coroutine.yield()
    if key == "bad" then return nil, { message = "Download failed" } end
    return jpeg
end }
local loader = Loader.new(app)
local list = ui.StackView { Overflow = "hidden scroll" }
app.screen:addChild(list)
local frames = {}
for i, key in ipairs { "same", "same", "third", "fourth", "fifth", "sixth", "bad" } do
    local frame = ui.Grid { Height = 100 }
    list:addChild(frame)
    frames[i] = frame
    loader:watch(frame, key, "Photo" .. i)
end
local function resume(worker)
    local ok, err = coroutine.resume(worker)
    assert(ok, err)
end
local function finishWorkers()
    local index = 1
    while index <= #workers do
        local worker = workers[index]
        while coroutine.status(worker) ~= "dead" do resume(worker) end
        index = index + 1
    end
    core.flushQueue()
end
loader:loadVisible()
assert(#workers == 0, "Unlaid-out views must not request photos")
app.screen:UpdateLayout(200, 450)
loader:loadVisible()
assert(#workers == 2 and loader.active == 2, "Only two image coroutines may run concurrently")
for _, worker in ipairs(workers) do resume(worker) end
assert(calls.same == 1 and calls.third == 1 and not calls.sixth,
    "Visible duplicates share one fetch and offscreen frames do not fetch")
finishWorkers()
assert(calls.same == 1 and calls.fourth == 1 and calls.fifth == 1 and not calls.sixth)
for i = 1, 5 do
    local image = assert(frames[i]:findChild("Photo" .. i))
    assert(image.Source and image.Source.Width == 32, "Downloaded bytes must reach the native ImageView resource")
end
if native then app.screen:send("Screen.RenderScreen", { width = 200, height = 450 }) end
-- Returning to a retained page should reuse the file without refetching it.
local duplicate = ui.Grid { Height = 100 }
list:addChild(duplicate)
loader:watch(duplicate, "same", "Cached")
list.ContentOffset = { X = 0, Y = -350 }
app.screen:UpdateLayout(200, 450)
loader:loadVisible()
finishWorkers()
assert(duplicate:findChild("Cached") and calls.same == 1)
assert(calls.sixth == 1 and calls.bad == 1, "Scrolling exposes only newly visible photos")
local retry = assert(frames[7]:findChild("RetryPhoto7"), "Failures need an explicit retry control")
loader:loadVisible()
assert(calls.bad == 1, "Paints must not spin on failed downloads")
retry:send("Node.LeftButtonUp")
core.flushQueue()
loader:loadVisible()
finishWorkers()
assert(calls.bad == 2, "Retry should start a fresh request")
-- A response arriving after sign-out must not update the view or its cache.
local late = ui.Grid { Height = 100 }
list:addChild(late)
loader:watch(late, "late", "Late")
list.ContentOffset = { X = 0, Y = -450 }
app.screen:UpdateLayout(200, 450)
loader:loadVisible()
local worker = workers[#workers]
resume(worker)
assert(calls.late == 1)
app.sessionEpoch = app.sessionEpoch + 1
resume(worker)
assert(not late:findChild("Late") and not app.media:path("late"), "Stale session results must be discarded")
-- Hidden retained pages cannot start downloads.
local hidden = ui.Grid { Height = 100, Visible = false }
list:addChild(hidden)
loader:watch(hidden, "hidden", "Hidden")
app.screen:UpdateLayout(200, 450)
loader:loadVisible()
assert(not calls.hidden)
-- Thread/profile routes temporarily replace the current screen while the shell
-- keeps its timeline. Those offscreen registrations must survive the visit.
app.shell = { screen = app.screen }
app.screen = ui.Screen { Width = 200, Height = 450, ResizeMode = "NoResize" }
loader:loadVisible()
assert(loader.watching[hidden], "Retained shell photos must survive navigation to another screen")
app.screen = app.shell.screen
-- Scene ownership does not keep a Lua userdata wrapper alive. A photo built
-- without an external Lua frame reference must survive GC before its first paint.
app.sessionEpoch = app.sessionEpoch + 1
local gcList = ui.StackView { Overflow = "hidden scroll" }
app.screen:addChild(gcList)
do
    local frame = ui.Grid { Name = "CollectedFrame", Height = 100 }
    gcList:addChild(frame)
    loader:watch(frame, "gc-photo", "AfterGC")
end
collectgarbage("collect")
collectgarbage("collect")
app.screen:UpdateLayout(200, 450)
loader:loadVisible()
finishWorkers()
assert(calls["gc-photo"] == 1 and gcList:findChild("CollectedFrame"):findChild("AfterGC"),
    "Native-owned visible photos must remain registered across Lua GC")
-- Detached frames and old sessions must release their strong registrations.
local detached = ui.Grid { Height = 100 }
gcList:addChild(detached)
loader:watch(detached, "detached", "Detached")
detached:removeFromParent()
loader:loadVisible()
assert(not loader.watching[detached] and not calls.detached)
assert(next(loader.watching) == nil, "Completed, detached and stale frames must leave the registry")
app.media:clear()
print("PASS: viewport photo loading, bounded concurrency, deduplication, native resources, cache, retry and stale sessions")
