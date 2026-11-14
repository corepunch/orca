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
-- Native grading is Apple-only; the stub marks which filter graded which bytes.
local graded = {}
media.filters = function() return { "normal", "clarendon", "inkwell" } end
media.filter = function(_, bytes, name)
    graded[#graded + 1] = name
    return { jpeg = bytes, thumbnail = "graded:" .. name, width = 7 }
end
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

-- Filters grade the focused photo from its original and publish the graded bytes.
press("Kindphoto"); press("ChoosePhoto"); press("ChoosePhoto")
local first, second = provider.draftPhotos("photo")[1], provider.draftPhotos("photo")[2]
assert(edit.draft.HasFilters and not edit.draft.FiltersOpen and edit.draft.FilterName == "normal")
assert(#graded == 0, "Closed filters grade nothing")
press("ToggleFilters")
assert(edit.draft.FiltersOpen and #graded == 2, "Opening the grid grades one preview per filter")
screen:UpdateLayout(390, 844); settle()
local tile = assert(form:findChild("Filterclarendon", true))
local choices = assert(form:findChild("FilterChoices", true))
assert(tile.ActualWidth > 60 and math.abs(tile.ActualHeight - tile.ActualWidth) < 2
    and tile.ActualWidth < choices.ActualWidth / 4 + 1, "Filter tiles are squares in a four-column grid")
local function pressIn(item, name)
    if type(item) == "string" then item = assert(form:findChild(item, true), item) end
    item:findChild(name, true):send("Node.LeftButtonUp"); settle()
end
-- Draft tiles are keyed by photo identity, so address them by position.
local function photoTile(index)
    local tile = assert(form:findChild("PhotoPreview", true)):getFirstChild():getFirstChild()
    for _ = 2, index do tile = tile:getNext() end
    return tile
end
assert(form:findChild("Filterinkwell", true), "Filter tiles are named by filter")
pressIn("Filterclarendon", "SelectFilter")
assert(second.filter == "clarendon" and second.thumbnail == "graded:clarendon" and first.filter == "normal",
    "A filter grades only the newest, focused photo")
assert(edit.draft.FilterName == "clarendon")
pressIn(photoTile(1), "FocusSelectedPhoto")
assert(edit.draft.FilterName == "normal" and #graded == 5, "Focusing another photo grades its own previews")
pressIn("Filterinkwell", "SelectFilter")
pressIn(photoTile(2), "FocusSelectedPhoto")
pressIn("Filterinkwell", "SelectFilter")
assert(second.filter == "inkwell" and graded[#graded] == "inkwell" and second.source.jpeg == jpeg,
    "Changing filters regrades the original rather than the previous grade")
pressIn("Filternormal", "SelectFilter")
assert(second.filter == "normal" and second.thumbnail == nil and second.width == nil, "Normal restores the original")
pressIn("Filterinkwell", "SelectFilter")
press("Publish")
assert(captured.photos[1] == first and captured.photos[2] == second
    and first.thumbnail == "graded:inkwell" and second.thumbnail == "graded:inkwell",
    "Publishing uploads each photo with its own filter")
assert(not edit.draft.HasFilters, "A cleared draft hides the filters")
scope:dispose(); media:clear()
print("PASS: Octopus album composer limit, ordered per-tile removal, independent drafts, failed publish, successful clear and photo filters")
