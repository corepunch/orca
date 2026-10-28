local orca = require "orca"
orca.init()
local ui = require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
require("orca.filesystem").init("apps/chat")
local Viewer = require "Chat/views/photo_viewer"
local Media = require "Chat/media"
local file = assert(io.open("tests/fixtures/chat/photo.jpg", "rb"))
local jpeg = file:read("*a"); file:close()
local workers, calls, stored = {}, 0, 0
local session = { user = { id = "alice" } }
local app = { screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize", ClearColor = "#ffffff" },
    room = {}, sessionEpoch = 1, navigationEpoch = 1, client = {}, media = Media.new() }
local store = app.media.storePreview
function app.media:storePreview(bytes, key) stored = stored + 1; return store(self, bytes, key) end
function app:getMedia() return self.media end
function app:load(fn) workers[#workers + 1] = coroutine.create(fn) end
function app.client:getSession() return session end
function app.client:getPhoto(id, thumbnail)
    assert(thumbnail == false, "The viewer must fetch the full JPEG")
    calls = calls + 1
    coroutine.yield()
    if id == "bad" then return nil, { message = "Photo download failed" } end
    return jpeg
end
local function step(worker)
    local ok, err = coroutine.resume(worker); assert(ok, err)
end
local function finish(worker)
    while coroutine.status(worker) ~= "dead" do step(worker) end
end
local function click(name)
    assert(app.screen:findChild(name, true), "Missing " .. name):send("Node.LeftButtonUp")
    require("orca.core").flushQueue()
end

local composer = ui.Input { Name = "Composer", Text = "unsent draft" }
app.screen:addChild(composer)
local underlyingClicks = 0
app.screen:addChild(ui.Button { Name = "UnderlyingAction", Click = function() underlyingClicks = underlyingClicks + 1 end })
composer:setFocus()
local root = Viewer.open(app, { id = "photo" })
assert(not composer:isFocused() and root:findChild("ClosePhotoViewer", true):isFocused(),
    "Opening a photo must release the composer and dismiss its keyboard")
assert(root and app.screen:findChild("PhotoViewerLoading", true), "Opening an uncached photo shows loading")
assert(app.screen:findChild("ClosePhotoViewer", true), "Loading photos can be closed")
local worker = workers[#workers]
step(worker)
assert(calls == 1 and stored == 0)
finish(worker)
require("orca.core").flushQueue()
app.screen:UpdateLayout(390, 844)
local image = assert(app.screen:findChild("PhotoViewerImage", true))
assert(image.Stretch == "Uniform" and image.Source and image.Source.Width == 32,
    "The full JPEG must be fitted without cropping")
for _, size in ipairs { { 390, 844 }, { 834, 390 } } do
    app.screen.Width, app.screen.Height = size[1], size[2]
    require("orca.core").advanceFrame()
    require("orca.core").runAllPrograms(app.screen)
    app.screen:UpdateLayout(size[1], size[2])
    local host = image:getParent()
    assert(root.ActualWidth == size[1] and root.ActualHeight == size[2], "The viewer covers the screen")
    assert(image.ActualWidth <= host.ActualWidth and image.ActualHeight <= host.ActualHeight
        and math.abs(image.ActualWidth / image.ActualHeight - 4/3) < 0.01,
        "The whole photo must fit in portrait and landscape layouts")
    assert(math.abs(image.ActualWidth - host.ActualWidth) < 2
        or math.abs(image.ActualHeight - host.ActualHeight) < 2,
        "Even small photos enlarge until one dimension fills the viewport")
    assert(math.abs(image.ActualX * 2 + image.ActualWidth - host.ActualWidth) < 2
        and math.abs(image.ActualY * 2 + image.ActualHeight - host.ActualHeight) < 2,
        "The fitted image must be centered with letterboxing on either axis")
end
app.screen.Width, app.screen.Height, app.screen.RespectSafeArea = 390, 844, false
app.screen.SafeAreaTop, app.screen.SafeAreaBottom = 44, 34
require("orca.core").advanceFrame()
require("orca.core").runAllPrograms(app.screen)
app.screen:UpdateLayout(390, 844)
assert(root.AdjustedContentInsetTop == 44 and root.AdjustedContentInsetBottom == 34,
    "An edge-to-edge thread must keep the viewer controls clear of system safe areas")
for _, message in ipairs { "LeftButtonDown", "LeftButtonUp" } do
    orca.system.dispatchMessage { target = app.screen, message = message, x = 195, y = 422 }
end
require("orca.core").flushQueue()
assert(underlyingClicks == 0 and app.photoViewer, "Photo taps cannot activate the composer or controls underneath")
local cached = image.Src
click("ClosePhotoViewer")
assert(not app.photoViewer and not app.screen:findChild("PhotoViewer", true))
assert(composer.Text == "unsent draft", "Closing the viewer preserves the conversation and draft")

app.messagePhotos = { cached = function(_, attachment) return attachment.id == "cached" and cached or nil end }
Viewer.open(app, { id = "cached" })
assert(app.screen:findChild("PhotoViewerImage", true).Src == cached and calls == 1,
    "A displayed full JPEG is reused without a second request")
Viewer.close(app)

Viewer.open(app, { id = "bad" })
finish(workers[#workers])
assert(app.screen:findChild("PhotoViewerError", true) and app.screen:findChild("RetryPhotoViewer", true),
    "A failed photo offers retry without losing the close action")
click("RetryPhotoViewer")
assert(app.screen:findChild("PhotoViewerLoading", true))
finish(workers[#workers])
assert(calls == 3, "Retry requests the full photo again")
Viewer.close(app)

-- A closed viewer and a replacement viewer must not receive an old request.
Viewer.open(app, { id = "late" })
worker = workers[#workers]; step(worker)
Viewer.open(app, { id = "cached" })
finish(worker)
assert(stored == 1 and app.screen:findChild("PhotoViewerImage", true).Src == cached)
Viewer.close(app)

-- Navigation and session changes are checked before writing private files.
for _, invalidate in ipairs {
    function() app.navigationEpoch = app.navigationEpoch + 1 end,
    function() app.sessionEpoch = app.sessionEpoch + 1; app.media:clear() end,
    function() session = { user = { id = "bob" } } end,
} do
    Viewer.open(app, { id = "late" })
    worker = workers[#workers]; step(worker)
    invalidate(); finish(worker)
    assert(stored == 1 and not app.screen:findChild("PhotoViewerImage", true),
        "Stale requests must not recreate private files or change the viewer")
    Viewer.close(app)
end
app.media:clear()
print("PASS: letterboxed full-photo viewer, cached JPEG, close, retry and stale request guards")
