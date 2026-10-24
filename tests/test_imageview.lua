local orca = require "orca"
orca.init()
local core = require "orca.core"
local filesystem = require "orca.filesystem"
local ui = require "orca.UIKit"
local renderer = require "orca.renderer"
if os.getenv("ORCA_IMAGEVIEW_NATIVE") == "1" then renderer.init(64, 64, true) end
filesystem.init("apps/chat")

local fixture = assert(io.open("tests/fixtures/chat/photo.jpg", "rb"))
local jpeg = fixture:read("*a")
fixture:close()
local paths = {}
for i = 1, 2 do
    local temporary = os.tmpname()
    paths[i] = temporary .. ".jpg"
    assert(os.rename(temporary, paths[i]))
    local file = assert(io.open(paths[i], "wb"))
    assert(file:write(jpeg)); file:close()
end

local screen = ui.Screen { Width = 64, Height = 64, ResizeMode = "NoResize" }
local function collect()
    for _ = 1, 3 do collectgarbage("collect"); core.flushQueue() end
end
local function assertLoaded(image)
    local source = assert(image.Source, "The local image must expose its texture")
    assert(source.Width == 32 and source.Height == 24,
        string.format("The owned image resource must start before use: %sx%s", source.Width, source.Height))
    return source
end
local function exercise()
    local image = ui.ImageView { Src = paths[1], Width = 32, Height = 24 }
    screen:addChild(image)
    core.flushQueue()
    local first = assertLoaded(image)
    first:send("Object.Start")
    assertLoaded(image)
    for _ = 1, 3 do
        image:send("Node.LoadView")
        assertLoaded(image)
    end
    image.Src = paths[2]
    core.flushQueue()
    assertLoaded(image)
    assert(first.Width == 32 and first.Height == 24,
        "A caller-held source must remain alive after the view replaces its owned resource")
    image.Src = ""
    core.flushQueue()
    assert(image.Source == nil, "Clearing Src must release and clear the owned texture")
    image:removeFromParent()
end

exercise()
collect()
local before = core.objectCount()
exercise()
collect()
assert(core.objectCount() == before, "Local image replacement and destruction must not leak owned resources")
for _, path in ipairs(paths) do assert(os.remove(path)) end
print("PASS: local ImageView resource startup, replacement, retained references and cleanup")
