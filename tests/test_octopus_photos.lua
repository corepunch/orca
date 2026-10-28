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
local colors = require "Octopus/theme"
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
    if key == "corrupt" and calls[key] == 1 then return "\255\216\255\226\0" end
    return jpeg
end }
local loader = Loader.new(app)
local list = ui.StackView { Overflow = "hidden scroll" }
app.screen:addChild(list)
local frames = {}
for i, key in ipairs { "same", "same", "third", "fourth", "fifth", "sixth", "bad" } do
    local frame = ui.Grid { Height = 100, Background = colors.raised }
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
assert(not frames[1]:findChild("LoadingPhoto1"), "Unlaid-out photos must not animate a spinner")
app.screen:UpdateLayout(200, 450)
loader:loadVisible()
assert(#workers == 2 and loader.active == 2, "Only two image coroutines may run concurrently")
for i = 1, 5 do
    assert(frames[i]:findChild("LoadingPhoto" .. i).IsActive,
        "Visible queued and active requests need a loading indicator")
end
assert(not frames[6]:findChild("LoadingPhoto6"), "Offscreen photos must not animate a spinner")
for _, worker in ipairs(workers) do resume(worker) end
assert(calls.same == 1 and calls.third == 1 and not calls.sixth,
    "Visible duplicates share one fetch and offscreen frames do not fetch")
frames[1].Visible = false
loader.observer:update()
assert(not frames[1]:findChild("LoadingPhoto1") and loader.active == 2,
    "Scrolling or hiding a photo must stop its spinner without restarting the shared download")
frames[1].Visible = true
loader.observer:update()
assert(frames[1]:findChild("LoadingPhoto1").IsActive and calls.same == 1,
    "Returning to an active request must restore its spinner without fetching again")
finishWorkers()
assert(calls.same == 1 and calls.fourth == 1 and calls.fifth == 1 and not calls.sixth)
for i = 1, 5 do
    local image = assert(frames[i]:findChild("Photo" .. i))
    assert(image.Source and image.Source.Width == 32, "Downloaded bytes must reach the native ImageView resource")
    assert(frames[i].BackgroundColor.A == 0 and frames[i].Height == 100,
        "Successful photos must remove the placeholder background without changing the reserved height")
    assert(not frames[i]:findChild("LoadingPhoto" .. i), "Successful photos must remove the loading indicator")
end
app.screen:UpdateLayout(200, 450)
local fitted = frames[1]:findChild("Photo1")
assert(math.abs(fitted.ActualX) < 1 and math.abs(fitted.ActualY) < 1
    and fitted.ActualWidth <= frames[1].ActualWidth and fitted.ActualHeight <= frames[1].ActualHeight,
    "Aspect-fitted photos must sit in the top-left corner of their transparent reserved frame")
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
assert(not duplicate:findChild("LoadingCached"), "Cache hits must not leave a loading indicator")
assert(calls.sixth == 1 and calls.bad == 1, "Scrolling exposes only newly visible photos")
local retry = assert(frames[7]:findChild("RetryPhoto7"), "Failures need an explicit retry control")
assert(not frames[7]:findChild("LoadingPhoto7") and frames[7].BackgroundColor.A > 0,
    "Failures must replace the spinner with a visible retry placeholder")
local unavailable = assert(retry:findChild("UnavailablePhoto7"), "Failed photos need a broken-image icon")
assert(unavailable.Source == "Octopus/assets/icons/image-off.svg" and (retry.Text or "") == "",
    "Failed photos show the broken-image icon without retry text")
app.screen:UpdateLayout(200, 450)
assert(retry.ActualWidth == frames[7].ActualWidth and retry.ActualHeight == frames[7].ActualHeight,
    "The retry area must fill the reserved photo frame")
assert(math.abs(unavailable.ActualX + unavailable.ActualWidth / 2 - retry.ActualWidth / 2) < 1
    and math.abs(unavailable.ActualY + unavailable.ActualHeight / 2 - retry.ActualHeight / 2) < 1,
    "The broken-image icon must stay centered in the reserved photo frame")
if native then app.screen:send("Screen.RenderScreen", { width = 200, height = 450 }) end
loader:loadVisible()
assert(calls.bad == 1, "Paints must not spin on failed downloads")
retry:send("Node.LeftButtonUp")
core.flushQueue()
loader:loadVisible()
assert(frames[7]:findChild("LoadingPhoto7"), "Retry must show progress while the new request is pending")
finishWorkers()
assert(calls.bad == 2, "Retry should start a fresh request")
-- HTTP success is not sufficient: corrupt photo bytes must keep the retry
-- placeholder, and retry must fetch new bytes instead of reusing the bad file.
local corrupt = ui.Grid { Height = 100 }
list:addChild(corrupt)
loader:watch(corrupt, "corrupt", "Corrupt")
list.ContentOffset = { X = 0, Y = -450 }
app.screen:UpdateLayout(200, 450)
loader:loadVisible()
finishWorkers()
assert(corrupt:findChild("RetryCorrupt") and not corrupt:findChild("Corrupt"),
    "Decode failures need the same retry placeholder as download failures")
assert(not app.media:path("photo:corrupt"), "Failed decodes must evict corrupt cached bytes")
corrupt:findChild("RetryCorrupt"):send("Node.LeftButtonUp")
core.flushQueue()
loader:loadVisible()
finishWorkers()
assert(calls.corrupt == 2 and corrupt:findChild("Corrupt"), "Retry must recover with a newly downloaded photo")
-- A response arriving after sign-out must not update the view or its cache.
local late = ui.Grid { Height = 100 }
list:addChild(late)
loader:watch(late, "late", "Late")
list.ContentOffset = { X = 0, Y = -550 }
app.screen:UpdateLayout(200, 450)
loader:loadVisible()
local worker = workers[#workers]
resume(worker)
assert(calls.late == 1)
app.sessionEpoch = app.sessionEpoch + 1
loader:loadVisible()
assert(not late:findChild("LoadingLate"), "Changing sessions must stop stale loading indicators")
resume(worker)
assert(not late:findChild("Late") and not app.media:path("photo:late"), "Stale session results must be discarded")
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
-- Known dimensions reserve the final display frame before any network work.
workers, calls = {}, {}
app.sessionEpoch = app.sessionEpoch + 1
app.screen = ui.Screen { Width = 200, Height = 450, ResizeMode = "NoResize" }
loader = Loader.new(app)
assert(math.abs(loader:aspect("known", 1600, 900) - 16 / 9) < 0.001)
assert(loader:aspect("portrait", 600, 1200) == 4 / 5 and loader:aspect("wide", 4000, 1000) == 1.91,
    "Metadata preserves the established portrait and landscape crop limits")
assert(loader:aspect("invalid", 0, 100) == 4 / 3 and loader:aspect("invalid", 1 / 0, 1) == 4 / 3
    and loader:aspect("invalid", 1.5, 2) == 4 / 3, "Invalid dimensions cannot poison layout")
local known = ui.Grid { AspectRatio = loader:aspect("known"), Background = colors.raised }
app.screen:addChild(known)
loader:watch(known, "known", "Known")
app.screen:UpdateLayout(200, 450)
local reserved = known.ActualHeight
assert(math.abs(reserved - 112.5) < 1 and known.BackgroundColor.A > 0 and #workers == 0,
    "A gray rectangle has the final height before image requests start")
loader:loadVisible()
local pending = workers[1]
resume(pending)
app.screen:UpdateLayout(200, 450)
assert(not known:findChild("Known") and math.abs(known.ActualHeight - reserved) < 0.1,
    "A pending download must preserve the metadata-defined frame")
resume(pending)
app.screen:UpdateLayout(200, 450)
assert(known:findChild("Known") and math.abs(known.ActualHeight - reserved) < 0.1,
    "Decoding must not replace metadata dimensions or shift the feed")
assert(calls.known == 1, "Photos load once without a separate thumbnail request")
app.media:clear()
print("PASS: stable metadata layout, bounded viewport loading, native resources, cache, retry and stale sessions")
