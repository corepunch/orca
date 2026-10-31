-- Album selections and per-kind drafts through the actual XML write form.
local orca = require "orca"
orca.init()
local fs, core = require "orca.filesystem", require "orca.core"
require("orca.renderer").init(390, 844, true)
fs.init("apps/octopus")
require("orca.core.startup").load_plugins()
local data = require "orca.data"
local Provider, Media = require "Octopus/providers/octopus", require "Octopus/media"
local file = assert(io.open("tests/fixtures/chat/photo.jpg", "rb"))
local jpeg = file:read("*a"); file:close()
local media, notice, captured, rejected = Media.new(), "", nil, true
media.pick = function() return { jpeg = jpeg } end
local provider = Provider.new {
    media = media, notice = function(value) notice = value end,
    client = { createPost = function(_, values)
        captured = values
        if rejected then return nil, { code = "network", message = "Offline." } end
        return { id = "new-post" }
    end },
}
local scope = data.scope("Octopus/Data/Compose.xml", { provider = provider })
scope:expose("Reader", scope.store:put("OctopusReader", { id = "reader", signedIn = true }))
scope:expose("Composer", scope.store:put("OctopusComposer", { id = "composer", kind = "photo" }))
Provider.selectKind(scope, "photo")
local screen = require("orca.UIKit").Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
local form = assert(fs.loadObject("Octopus/Pages/Write"))
screen:addChild(form); scope:attach(form)
local function settle()
    core.flushQueue(); core.advanceFrame(); core.runAllPrograms(screen); core.flushQueue()
end
local function press(name)
    assert(form:findChild(name, true), name):send("Node.LeftButtonUp"); settle()
end
settle()
assert(not scope:member("Draft"):dirty(), "An empty selection must keep a new draft clean")
for index = 1, 9 do press("ChoosePhoto") end
assert(#provider.draftPhotos("photo") == 9)
screen:UpdateLayout(390, 844); settle()
local preview = assert(form:findChild("PhotoPreview", true))
local grid = preview:getFirstChild()
assert(grid.ActualWidth > 0 and math.abs(grid.ActualHeight - grid.ActualWidth) < 2,
    "Nine composer thumbnails reserve a compact square grid")
press("ChoosePhoto")
assert(#provider.draftPhotos("photo") == 9 and notice == "You can add up to 9 photos.")
local original = provider.draftPhotos("photo")[2]
press("RemoveSelectedPhoto")
assert(#provider.draftPhotos("photo") == 8 and provider.draftPhotos("photo")[1] == original,
    "Removing the first tile preserves selection order")
press("Kindquote"); press("ChoosePhoto")
assert(#provider.draftPhotos("quote") == 1 and #provider.draftPhotos("photo") == 8)
press("Kindphoto")
local edit = scope:member("Draft")
assert(edit.draft.HasPhoto and edit.draft.PhotoCount == 8)
edit.draft.Content = "Eight views"
press("Publish")
assert(#captured.photos == 8 and captured.photos[1] == original and captured.content == "Eight views")
assert(#provider.draftPhotos("photo") == 8 and edit.draft.Content == "Eight views" and notice:find("draft is safe", 1, true),
    "Failed publication retains every selected image and the caption")
rejected = false
press("Publish")
assert(#provider.draftPhotos("photo") == 0 and not edit.draft.HasPhoto and edit.draft.Content == "")
press("Kindquote")
assert(#provider.draftPhotos("quote") == 1 and edit.draft.HasPhoto,
    "Publishing a photo album leaves the other kind's selection intact")
scope:dispose(); media:clear()
print("PASS: Octopus album composer limit, ordered per-tile removal, independent drafts, failed publish and successful clear")
