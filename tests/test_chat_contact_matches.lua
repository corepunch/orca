-- Native app pages over deterministic, isolated offline providers.
local orca = require "orca"
orca.init()
local fs, core = require "orca.filesystem", require "orca.core"
require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/chat")
require("orca.core.startup").load_plugins()
local App, Chat = require "Chat/App", require "api.chat"

local savedAsync = orca.async
orca.async = function(fn, ...) return fn(...) end
local function settle(app)
    while true do
        local message = orca.system.peekMessage()
        if not message then break end
        if not message:is("Window.Paint") then orca.system.dispatchMessage(nil, message) end
    end
    core.advanceFrame()
    core.runAllPrograms(app.screen)
    app.screen:UpdateLayout(app.screen.Width, app.screen.Height)
end
local function press(app, name)
    assert(app.screen:findChild(name, true), "Missing " .. name):send("Node.LeftButtonUp")
    settle(app)
end
local function newApp(signedIn)
    local client = Chat.mock { clock = function() return 1700000000 end }
    if signedIn then assert(client:signIn("demo@example.invalid", "password123")) end
    local app = App { client = client }
    -- Keep route construction/lifecycle real; animations are separately tested.
    app.show = function(self, route) return self:navigate(route or "/") end
    app.reportError = function(_, err) error(tostring(err), 0) end
    app:show("/")
    settle(app)
    return app
end
-- Find friends from contacts: the device's addresses are matched once against
-- people who opted in to "find me by email"; a match opens a direct chat.
local function nodeText(node) return node and (node.Text ~= "" and node.Text or node:getTextContent()) end
local function hasText(root, text)
    local found = false
    local function visit(node)
        if found or not node then return end
        if (node.Text == text) or (node.getTextContent and node:getTextContent() == text) then found = true; return end
        local child = node:getFirstChild()
        while child do visit(child); child = child:getNext() end
    end
    visit(root)
    return found
end

-- Unsupported devices show no command.
do
    local app = newApp(true)
    app.contactsNative = { supported = function() return false end }
    app:show("/new"); settle(app)
    assert(not app.screen:findChild("FromContacts", true), "No contacts command where contacts cannot be read")
end

local app = newApp(true)
local reads = 0
app.contactsNative = {
    supported = function() return true end,
    readEmails = function()
        reads = reads + 1
        return { { name = "Alex from work", emails = { "ALEX@example.invalid", "alex@example.invalid" } },
            { name = "Jules", emails = { "jules@example.invalid" } }, { name = "Stranger", emails = { "nobody@example.invalid" } } }
    end,
}
app:show("/new"); settle(app)
press(app, "FromContacts")
assert(reads == 1, "The command reads the device's contacts once")
local match = assert(app.screen:findChild("FoundPersonContactalex", true), "An opted-in contact is found")
assert(not app.screen:findChild("FoundPersonContactjules", true), "A contact who did not opt in stays hidden")
assert(hasText(match, "Alex from work · @alex"), "The match shows the name from the device's contacts")
press(app, "FoundPersonContactalex")
assert(app.currentRoute == "/thread" and app.room and app.room.peer_id == "alex", "A match opens the direct chat")

-- Nobody found, and a refused permission, are explained.
app:show("/new"); settle(app)
app.contactsNative.readEmails = function() return { { name = "Stranger", emails = { "nobody@example.invalid" } } } end
press(app, "FromContacts")
assert(app.screen:findChild("NoContactMatches", true), "An empty match explains how friends appear")
app.contactsNative.readEmails = function() return nil, { code = "permission", message = "Allow access to contacts in Settings to find friends from them." } end
press(app, "FromContacts")
local status = app.screen:findChild("NewChatStatus", true)
assert(status.Visible and nodeText(status) == "Allow access to contacts in Settings to find friends from them.",
    "A refused permission says how to allow it")
orca.async = savedAsync
print("Chat contact matches: opted-in friends found, others hidden, chat opens, empty and refused states explained")
