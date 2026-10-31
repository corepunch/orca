local orca = require "orca"
orca.init()
local ui = require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
local Avatars = require "orca.avatars"
assert(#Avatars.entries == 16, "Both apps offer all sixteen characters")
local seen = {}
for _, entry in ipairs(Avatars.entries) do
    assert(not seen[entry.id] and entry.label ~= "", "Characters have unique stable IDs and labels")
    seen[entry.id] = true
    local photo, err = Avatars.load(entry.id)
    assert(photo, err and err.message)
    assert(photo.byte_count == #photo.jpeg and photo.byte_count < 250000)
    local image = ui.ImageView { Src = entry.path }
    image:send("Node.LoadView")
    assert(image.Source and image.Source.Width == photo.width and image.Source.Height == photo.height,
        "Picker portraits decode from the same bundled file that is uploaded")
end
local photo, err = Avatars.load("../../other-file")
assert(not photo and err.code == "validation", "Only catalog IDs may load files")
local open = io.open
io.open = function() return nil end
photo, err = Avatars.load("orca")
io.open = open
assert(not photo and err.code == "file", "A missing resource is a retryable error")
assert(Avatars.load("orca"), "A failed load does not poison the next attempt")
print("Default avatar catalog: 16 portraits decode; invalid and missing resources handled")
