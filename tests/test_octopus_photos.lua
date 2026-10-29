-- Timeline photos through the Photos media member: real PostCard rows, the
-- visibility walk and Octopus file storage, with a network double whose
-- downloads the test resumes explicitly.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
local ui = require "orca.UIKit"
require("orca.renderer").init(200, 450, true)
fs.init("apps/octopus")
require("orca.core.startup").load_plugins()
local data = require "orca.data"
local Visibility = require "orca.data.visibility"
local Media = require "Octopus/media"
local OctopusProvider = require "Octopus/providers/octopus"
local f = assert(io.open("tests/fixtures/chat/photo.jpg", "rb"))
local jpeg = f:read("*a"); f:close()

local workers, calls, valid = {}, {}, true
local posts = {}
for i, key in ipairs { "same", "same", "third", "fourth", "fifth", "sixth", "bad", "corrupt", "late" } do
    posts[i] = { id = "p" .. i, postType = "photo", title = "", content = "", authorId = "u", authorName = "U",
        createdAt = "2025-10-25T00:00:00Z", imageId = key, imageWidth = 400, imageHeight = 300, tags = {} }
end
local client = {
    listPosts = function() return posts end,
    stats = function() return {} end,
    avatars = function() return {} end,
    getImage = function(_, key)
        calls[key] = (calls[key] or 0) + 1
        coroutine.yield()
        if key == "bad" then return nil, { message = "Download failed" } end
        if key == "corrupt" and calls[key] == 1 then return "\255\216\255\226\0" end
        return jpeg
    end,
}
local media = Media.new()
local provider = OctopusProvider.new { client = client, media = media, now = function() return 1761400000 end }
local store = data.Store.new { schema = data.schema("Octopus/Data/Octopus.Schema.xml"), scope = "octopus:test",
    computed = provider.computed }
local host = data.Scope.defaultHost()
host.run = function(fn) workers[#workers + 1] = coroutine.create(fn) end
local scope = data.scope("Octopus/Data/Feeds.xml", { provider = provider, store = store, host = host,
    valid = function() return valid end })
local photos = scope:member("Photos")
local function resume(worker) assert(worker, "no worker " .. debug.traceback()); local ok, err = coroutine.resume(worker); assert(ok, err) end
local function finish()
    local index = 1
    while index <= #workers do
        local worker = workers[index]
        while coroutine.status(worker) ~= "dead" do resume(worker) end
        index = index + 1
    end
    workers = {}
end
local screen = assert(fs.loadObjectFromXmlString([[
<Screen Width="200" Height="450" ResizeMode="NoResize">
  <StackView Name="List" Overflow="hidden scroll" JustifyContent="Start">
    <ItemsStack Name="PostItems" Spacing="12" JustifyContent="Start" ItemNamePrefix="Post"
                ItemsSource="{Binding DataContext/Discover/Items}" ItemTemplate="Octopus/Templates/PostCard"/>
  </StackView>
</Screen>]]))
scope:attach(screen)
scope:member("Discover"):refresh()
finish()
local function settle()
    core.flushQueue(); core.advanceFrame(); core.runAllPrograms(screen); core.flushQueue()
end
settle()
local visibility = Visibility.new()
visibility:watch(screen, scope)
local function row(i) return assert(screen:findChild("Postp" .. i, true)) end
local function inRow(i, name) return assert(row(i):findChild(name, true), name .. " in row " .. i) end
local function shown(node)
    while node do
        if node.Visible == false then return false end
        node = node:getParent()
    end
    return true
end
local function visibleRows()
    local list, count = screen:findChild("List", true), 0
    for i = 1, #posts do
        local y = row(i).ActualY + list.ContentOffset.Y
        if y < 450 and y + row(i).ActualHeight > 0 then count = count + 1 end
    end
    return count
end

visibility:update()
assert(#workers == 0, "Unlaid-out views must not request photos")
screen:UpdateLayout(200, 450)
settle()
local first = visibleRows()
assert(first >= 3 and first < #posts, "The test needs both visible and offscreen rows")
visibility:update()
settle()
assert(photos.active == 2 and #photos.pending == 1, "Only two downloads run at once; the next one waits")
assert(shown(inRow(1, "PhotoLoading")) and shown(inRow(3, "PhotoLoading")), "Requested photos show progress")
for _, worker in ipairs(workers) do resume(worker) end
assert(calls.same == 1 and not calls.fourth and calls.third == 1 and not calls.sixth,
    "Visible duplicates share one download and offscreen photos do not download")
finish()
settle()
assert(calls.fourth == 1, "A waiting download starts when a slot frees")
local image = inRow(1, "Image")
assert(image.Src == media:path("photo:same") and inRow(2, "Image").Src == image.Src,
    "Every post showing the photo receives it")
image:send("Node.LoadView")
assert(image.Source and image.Source.Width == 32, "Downloaded bytes must reach the native ImageView resource")
assert(not shown(inRow(1, "PhotoLoading")) and not shown(inRow(1, "RetryPhoto")), "Loaded photos show neither progress nor retry")
assert(math.abs(store:get("OctopusPost", "p1").values.AspectRatio - 4 / 3) < 0.001, "Known dimensions reserve the frame")

-- Scrolling reaches the failing downloads; loaded photos are not fetched again.
local list = screen:findChild("List", true)
list.ContentOffset = { X = 0, Y = -(row(7).ActualY - 10) }
screen:UpdateLayout(200, 450)
visibility:update()
finish()
visibility:update()
finish()
settle()
assert(calls.same == 1 and calls.bad == 1, "Scrolling fetches only newly visible photos")
assert(shown(inRow(7, "RetryPhoto")) and not shown(inRow(7, "PhotoLoading")),
    "A failed download replaces progress with a retry placeholder")
assert(inRow(7, "PhotoUnavailable").Source == "Octopus/assets/icons/image-off.svg", "Failed photos show a broken-image icon")
visibility:update()
assert(calls.bad == 1 and #workers == 0, "Paints must not retry failed downloads by themselves")
inRow(7, "RetryPhoto"):send("Node.LeftButtonUp")
core.flushQueue()
settle()
assert(shown(inRow(7, "PhotoLoading")), "Retry shows progress while the new request runs")
finish()
assert(calls.bad == 2, "Retry starts a fresh download")
-- Bytes that do not decode are discarded; retry downloads them again.
assert(calls.corrupt == 1 and shown(inRow(8, "RetryPhoto")) and not media:path("photo:corrupt"),
    "Undecodable photos show retry and leave no cached file")
inRow(8, "RetryPhoto"):send("Node.LeftButtonUp")
core.flushQueue()
finish()
settle()
assert(calls.corrupt == 2 and inRow(8, "Image").Src == media:path("photo:corrupt"), "Retry recovers with new bytes")

-- A download that finishes for a replaced reader changes nothing.
list.ContentOffset = { X = 0, Y = -(row(9).ActualY - 10) }
screen:UpdateLayout(200, 450)
store:put("OctopusPost", { id = "p9", imageId = "stale" }, { partial = true })
visibility:update()
local late = workers[#workers]
resume(late)
valid = false
resume(late)
workers = {}
assert(calls.stale == 1 and not media:path("photo:stale"),
    "Results for a replaced reader are discarded")
valid = true

-- Hidden pages cannot start downloads.
list.Visible = false
photos:clear()
visibility:update()
assert(#workers == 0, "Hidden pages do not download")
list.Visible = true

-- Crop limits keep layout stable for any metadata.
local aspect = OctopusProvider.aspect
assert(math.abs(aspect(1600, 900) - 16 / 9) < 0.001)
assert(aspect(600, 1200) == 4 / 5 and aspect(4000, 1000) == 1.91,
    "Metadata preserves the established portrait and landscape crop limits")
assert(aspect(0, 100) == 4 / 3 and aspect(1 / 0, 1) == 4 / 3, "Invalid dimensions cannot poison layout")
assert(math.abs(aspect(nil, nil, 0.5) - 4 / 5) < 0.001, "A decoded shape is used when metadata is missing")
scope:dispose()
print("PASS: stable metadata layout, bounded viewport loading, native resources, cache, retry and stale sessions")
