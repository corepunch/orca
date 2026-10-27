local orca = require "orca"
orca.init()
local ui = require "orca.UIKit"
require("orca.renderer").init(200, 200, true)
require("orca.filesystem").init("apps/chat")
local Loader = require "Chat/photo_loader"
local Media = require "Chat/media"
local f = assert(io.open("tests/fixtures/chat/photo.jpg", "rb"))
local jpeg = f:read("*a"); f:close()
local workers, calls = {}, {}
local app = { screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }, sessionEpoch = 1,
    media = Media.new(), client = {} }
function app:task(fn) workers[#workers + 1] = coroutine.create(fn) end
function app:getMedia() return self.media end
function app:repaint() end
function app.client:getPhoto(id)
    calls[id] = (calls[id] or 0) + 1
    coroutine.yield()
    if id == "bad" then return nil, { message = "Download failed" } end
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
finish()
assert(calls.same == 1 and calls.third == 1 and calls.fourth == 1 and not calls.bad,
    "Visible duplicate previews share one fetch; offscreen photos wait")
assert(results[1] == results[2] and frames[1]:findChild("Photosame"), "All duplicate visible frames receive the preview")
list.ContentOffset = { X = 0, Y = -100 }
app.screen:UpdateLayout(200,200)
loader:loadVisible(); finish()
assert(calls.bad == 1 and frames[5]:findChild("RetryPhotobad"), "Visible failures expose explicit retry")
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
app.sessionEpoch = 2
loader:clear(); app.media:clear()
finish()
assert(not late:findChild("Photolate") and next(app.media.keys) == nil)
print("PASS: viewport chat previews, concurrency, deduplication, retry, versioned cache and session cleanup")
