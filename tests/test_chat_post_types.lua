-- Native type-picker geometry, pointer navigation and per-type draft retention.
local orca = require "orca"
orca.init()
local fs, core, system = require "orca.filesystem", require "orca.core", require "orca.system"
require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/chat")
require("orca.core.startup").load_plugins()
local App, Chat = require "Chat/App", require "api.chat"
local savedAsync, savedGetenv = orca.async, os.getenv
orca.async = function(fn, ...) return fn(...) end
local function settle(app)
    while true do
        local message = system.peekMessage()
        if not message then break end
        if not message:is("Window.Paint") then system.dispatchMessage(nil, message) end
    end
    core.advanceFrame()
    core.runAllPrograms(app.screen)
    app.screen:UpdateLayout(app.screen.Width, app.screen.Height)
end
local function visit(node, callback)
    callback(node)
    local child = node:getFirstChild()
    while child do visit(child, callback); child = child:getNext() end
end
local function position(node)
    local x, y = node.ActualWidth / 2, node.ActualHeight / 2
    while node do
        local transform = node.RenderTransformTranslation
        x, y = x + (node.ActualX or 0) + (transform and transform.X or 0),
            y + (node.ActualY or 0) + (transform and transform.Y or 0)
        local parent = node:getParent()
        local offset = parent and parent.ContentOffset
        if offset then x, y = x + offset.X, y + offset.Y end
        node = parent
    end
    return x, y
end
local function tap(app, name)
    local node = assert(app.screen:findChild(name, true), "Missing " .. name)
    local x, y = position(node)
    for _, message in ipairs { "LeftButtonDown", "LeftButtonUp" } do
        system.dispatchMessage { target = app.screen, message = message, x = x, y = y }
    end
    settle(app)
end
local function back(app)
    for _, message in ipairs { "LeftButtonDown", "LeftButtonUp" } do
        system.dispatchMessage { target = app.screen, message = message, x = 26, y = 26 }
    end
    settle(app)
end

local client = Chat.mock { clock = function() return 1700000000 end }
assert(client:signIn("demo@example.invalid", "password123"))
local app = App { client = client }
app.show = function(self, route) return self:navigate(route or "/") end
app.reportError = function(_, err) error(tostring(err), 0) end
local function showPicker(width)
    app:show("/posts/write")
    app.screen.Width = width or 390
    settle(app)
    assert(not app.screen:findChild("Publish", true) and not app.screen:findChild("PostTitle", true),
        "Step one contains type choices, not compose fields")
end
local palette = {}
for _, appearance in ipairs { "dark", "light" } do
    os.getenv = function(name) return name == "CHAT_APPEARANCE" and appearance or savedGetenv(name) end
    app:updateAppearance()
    for _, width in ipairs { 320, 390, 768 } do
        showPicker(width)
        for _, kind in ipairs { "text", "photo", "quote", "link" } do
            local tile = assert(app.screen:findChild("Kind" .. kind, true))
            assert(tile.ActualWidth >= 44 and math.abs(tile.ActualWidth - tile.ActualHeight) < 1,
                string.format("%s must be a square at %d: %.1f x %.1f", kind, width, tile.ActualWidth, tile.ActualHeight))
            local color = tile.BackgroundColor
            assert(color.A > 0.99, "Tiles use solid colors")
            local key = string.format("%.3f,%.3f,%.3f", color.R, color.G, color.B)
            if appearance == "dark" then palette[kind] = key
            else assert(palette[kind] == key, "Type colors remain stable in light mode") end
            local labels, icons = 0, 0
            visit(tile, function(node)
                if node.className == "TextBlock" or node.className == "VectorView" then
                    local white = node.ForegroundColor
                    assert(white.R > 0.99 and white.G > 0.99 and white.B > 0.99, "Tile icons and text must be white")
                    if node.className == "TextBlock" then labels = labels + 1 else icons = icons + 1 end
                end
            end)
            assert(labels > 0 and icons > 0, "Each type has an icon and a label")
        end
    end
end
local distinct = {}
for _, color in pairs(palette) do assert(not distinct[color], "Each post type has its own color"); distinct[color] = true end
local fields = { text = "PostTitle", photo = "PhotoCaption", quote = "QuoteText", link = "LinkUrl" }
for _, kind in ipairs { "text", "photo", "quote", "link" } do
    showPicker()
    tap(app, "Kind" .. kind)
    assert(app.currentRoute == "/posts/compose", "A tile opens step two")
    assert(not app.screen:findChild("PostKinds", true) and not app.screen:findChild("Kindtext", true),
        "The editor does not repeat the type picker")
    assert(app:posts().app.composeScope:exposedRecord("Composer").values.Kind == kind,
        "The editor uses the selected type")
    app.screen:findChild(fields[kind], true).Text = "Draft for " .. kind
    settle(app)
    back(app)
    assert(app.currentRoute == "/posts/write", "Editor Back returns to type choice")
end
for _, kind in ipairs { "text", "photo", "quote", "link" } do
    showPicker()
    tap(app, "Kind" .. kind)
    assert(app.screen:findChild(fields[kind], true).Text == "Draft for " .. kind, "Each type keeps its own draft")
    back(app)
end
back(app)
assert(app.currentRoute == "/posts", "Picker Back returns to Posts")
os.getenv, orca.async = savedGetenv, savedAsync
print("Chat two-step public composer: square colored tiles, white content, pointer navigation and independent drafts passed")
