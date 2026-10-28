local orca = require "orca"
orca.init()
local ui = require "orca.UIKit"
require("orca.renderer").init(200, 200, true)
require("orca.filesystem").init("apps/chat")
local Loader = require "Chat/photo_loader"
local Media = require "Chat/media"
local f = assert(io.open("tests/fixtures/chat/photo.jpg", "rb"))
local jpeg = f:read("*a"); f:close()
local invalidJpeg = "\255\216\255\226\0\255\217"
local workers, calls = {}, {}
local app = { screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }, sessionEpoch = 1,
    media = Media.new(), client = {} }
function app:task(fn) workers[#workers + 1] = coroutine.create(fn) end
function app:getMedia() return self.media end
function app:repaint() end
function app.client:getPhoto(id, thumbnail, onPreview)
    assert(thumbnail == false, "Every staged load must finish with the full JPEG")
    calls[id] = (calls[id] or 0) + 1
    coroutine.yield()
    if onPreview then
        if onPreview(id == "broken-preview" and invalidJpeg or jpeg) == false then return nil, { code = "cancelled" } end
        coroutine.yield()
    end
    if id == "bad" then return nil, { message = "Download failed" } end
    if id == "corrupt-full" then return invalidJpeg end
    return jpeg
end
local loader = Loader.new(app)
local list = ui.StackView { Overflow = "hidden scroll" }
app.screen:addChild(list)
local frames, results = {}, {}
for i, id in ipairs { "same", "same", "third", "fourth", "bad" } do
    local frame = ui.Grid { Height = 60 }
    list:addChild(frame)
    frames[i] = frame
    loader:watch(frame, { id = id }, function(path) results[i] = path end)
end
local function finish()
    local i = 1
    while i <= #workers do
        local worker = workers[i]
        while coroutine.status(worker) ~= "dead" do
            local ok, err = coroutine.resume(worker); assert(ok, err)
        end
        i = i + 1
    end
end
loader:loadVisible()
assert(#workers == 0, "No requests before layout")
app.screen:UpdateLayout(200,200)
collectgarbage("collect")
loader:loadVisible()
assert(#workers == 2 and loader.active == 2, "Only two preview downloads may run")
local nextWorker = 1
while not loader:preview { id = "same" } do
    local worker = assert(workers[nextWorker])
    while coroutine.status(worker) ~= "dead" and not loader:preview { id = "same" } do
        local ok, err = coroutine.resume(worker); assert(ok, err)
    end
    nextWorker = nextWorker + 1
end
local preview = assert(loader:preview { id = "same" })
assert(frames[1]:findChild("Photosame").Src == preview and frames[2]:findChild("Photosame").Src == preview,
    "One thumbnail is displayed in every visible duplicate while the full image is suspended")
assert(not results[1] and not loader:cached { id = "same" } and loader.active > 0 and loader.active <= 2,
    "A preview does not finish the request, notify completion or enter the full-image cache")
finish()
assert(calls.same == 1 and calls.third == 1 and calls.fourth == 1 and not calls.bad,
    "Visible duplicate previews share one fetch; offscreen photos wait")
assert(results[1] == results[2] and frames[1]:findChild("Photosame"), "All duplicate visible frames receive the preview")
assert(loader:cached { id = "same" } == results[1], "The viewer can reuse the full JPEG without another download")
assert(results[1] ~= preview and frames[1]:findChild("Photosame").Src == results[1],
    "The final JPEG replaces the preview using a separate immutable file")
assert(not loader:cached { id = "same", updated_at = "new-version" }, "Cache lookups respect attachment revisions")
list.ContentOffset = { X = 0, Y = -100 }
app.screen:UpdateLayout(200,200)
loader:loadVisible(); finish()
assert(calls.bad == 1 and frames[5]:findChild("RetryPhotobad"), "Visible failures expose explicit retry")
local failedPreview = assert(frames[5]:findChild("Photobad")).Src
assert(failedPreview == loader:preview { id = "bad" } and not loader:cached { id = "bad" },
    "A failed full image retains the usable thumbnail without caching it as final")
loader:loadVisible(); finish()
assert(calls.bad == 1, "Painting cannot repeatedly retry a failed photo")
frames[5]:findChild("RetryPhotobad"):send("Node.LeftButtonUp")
require("orca.core").flushQueue(); finish()
assert(calls.bad == 2)
-- Rebuilt rows share the versioned file cache, but changed versions must fetch.
local cached = ui.Grid { Height = 60 }
list:addChild(cached)
loader:watch(cached, { id = "same" })
list.ContentOffset = { X = 0, Y = -200 }
app.screen:UpdateLayout(200,200)
loader:loadVisible(); finish()
assert(cached:findChild("Photosame") and calls.same == 1)
local changed = ui.Grid { Height = 60 }
list:addChild(changed)
loader:watch(changed, { id = "same", updated_at = "new-version" })
app.screen:UpdateLayout(200,200)
loader:loadVisible(); finish()
assert(calls.same == 2)
-- A session reset prevents old completions from recreating cleared private files.
local late = ui.Grid { Height = 60 }
list:addChild(late)
loader:watch(late, { id = "late" })
list.ContentOffset = { X = 0, Y = -300 }
app.screen:UpdateLayout(200,200)
loader:loadVisible()
local worker = workers[#workers]
local ok, err = coroutine.resume(worker); assert(ok,err)
assert(calls.late == 1)
ok, err = coroutine.resume(worker); assert(ok, err)
assert(late:findChild("Photolate") and not loader:cached { id = "late" },
    "The final image is still pending after the preview arrives")
app.sessionEpoch = 2
loader:clear(); app.media:clear()
finish()
assert(next(app.media.keys) == nil, "Late full responses cannot recreate cleared private files")
assert(not loader:cached { id = "same" }, "Session reset clears full-size photos")
assert(not loader:preview { id = "same" }, "Session reset clears thumbnail references")
-- Invalid image data in either stage must not suppress the usable stage.
app.screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
loader = Loader.new(app)
local stack = ui.StackView {}
app.screen:addChild(stack)
local brokenPreview, corruptFull = ui.Grid { Height = 60 }, ui.Grid { Height = 60 }
stack:addChild(brokenPreview); stack:addChild(corruptFull)
loader:watch(brokenPreview, { id = "broken-preview" })
loader:watch(corruptFull, { id = "corrupt-full" })
app.screen:UpdateLayout(200, 200)
loader:loadVisible(); finish()
assert(brokenPreview:findChild("Photobroken-preview") and loader:cached { id = "broken-preview" },
    "An undecodable optional thumbnail must still allow the final image")
assert(corruptFull:findChild("Photocorrupt-full") and corruptFull:findChild("RetryPhotocorrupt-full")
    and not loader:cached { id = "corrupt-full" },
    "An undecodable final image must retain the good preview, offer retry and leave the full cache empty")
app.media:clear()
print("PASS: staged Chat photos, thumbnail replacement, concurrency, deduplication, retry and session cleanup")
