-- New Message uses the real page, item templates and data scope over an offline client.
local orca = require "orca"
orca.init()
local fs, core = require "orca.filesystem", require "orca.core"
require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/chat")
require("orca.core.startup").load_plugins()
local App, Chat = require "Chat/Scripts/App", require "api.chat"

local savedAsync, pending = orca.async, {}
orca.async = function(fn, ...)
    local thread = coroutine.create(fn)
    local ok, err = coroutine.resume(thread, ...)
    assert(ok, err)
    if coroutine.status(thread) ~= "dead" then pending[#pending + 1] = thread end
end
local function completeRequest()
    local thread = assert(table.remove(pending, 1), "Expected a suspended request")
    local ok, err = coroutine.resume(thread)
    assert(ok, err)
    assert(coroutine.status(thread) == "dead")
end
local client = Chat.mock()
assert(client:signIn("demo@example.invalid", "password123"))
local app = App { client = client }
app.show = function(self, route) return self:navigate(route or "/") end
app.reportError = function(_, err) error(tostring(err), 0) end
app.openRoom = function(self, room) self.opened = room end
app.contactsNative = false
local creates, searches, createError, delayed = {}, 0, false, false
app.models.Rooms.create = function(_, values)
    creates[#creates + 1] = values
    if createError then return nil, { message = "Please retry" } end
    return { id = "room", kind = values.kind, peer_id = values.member_ids[1] }
end
client.findPeople = function(_, search)
    searches = searches + 1
    if delayed then coroutine.yield() end
    if search == "@alex" then return { { id = "alex", display_name = "Alex", username = "alex" } } end
    return {}
end
client.findChannels = function() return { { id = "news", name = "News" } } end
client.joinPublicChannel = function(_, id) return { id = id, kind = "channel" } end
client.joinInvite = function(_, code)
    if code ~= "valid" then return nil, { message = "Invalid invitation" } end
    return { id = "invited", kind = "group" }
end

local function settle()
    while true do
        local message = orca.system.peekMessage()
        if not message then break end
        if not message:is("Window.Paint") then orca.system.dispatchMessage(nil, message) end
    end
    core.advanceFrame()
    core.runAllPrograms(app.screen)
    app.screen:UpdateLayout(390, 844)
end
local function find(name) return app.screen:findChild(name, true) end
local function node(name) return assert(find(name), "Missing " .. name) end
local function click(name) node(name):send("Node.LeftButtonUp"); settle() end
local function text(name, value) node(name).Text = value; settle() end
local function back() node("NewMessagePage"):post("Pivot.BackRequested"); settle() end
local function submitSearch()
    local input = node("FindPerson")
    input:post("Input.Submit", { Text = input.Text })
    settle()
end

app:show("/new")
settle()
local contact = node("Contactalex")
local heading = contact:getParent():getParent():getFirstChild()
assert(heading:getClassName() == "TextBlock" and heading.Text == "a", "Contacts appear under letter headings")
click("Contactalex")
assert(app.opened.peer_id == "alex" and creates[1].kind == "direct" and creates[1].visibility == "private")
createError, app.opened = true, nil
click("Contactalex")
assert(not app.opened and node("NewChatStatus"):getTextContent() == "Please retry")
createError = false

text("FindPerson", "   ")
click("FindPeople")
assert(searches == 0)
text("FindPerson", "missing")
click("FindPeople")
assert(node("NoPeopleResults").Visible and node("NoPeopleResults"):getTextContent():find("public username", 1, true))
text("FindPerson", "@alex")
submitSearch()
local result = node("FoundPersonalex")
submitSearch()
assert(node("FoundPersonalex"):getUniqueID() == result:getUniqueID(), "Refreshing the same search retains its row")
click("FoundPersonalex")
assert(app.opened.kind == "direct")

click("KindGroup")
assert(not node("NewChatActions").Visible and not node("ConversationSetup").Visible)
click("Contactalex")
assert(app.newSelection.alex)
click("Contactalex")
assert(not app.newSelection.alex)
click("Contactalex")
text("FindPerson", "@alex")
click("FindPeople")
assert(node("FoundPersonalex").StateClasses == "chat-soft" and node("Contactalex").StateClasses == "chat-soft",
    "Contact and discovery rows share group-selection state")
assert(node("Contactalex"):getUniqueID() == contact:getUniqueID(), "Selection keeps the mounted contact row")
click("GroupNext")
assert(node("ConversationSetup").Visible and not node("PeopleSearch").Visible and not node("ConversationVisibility").Visible)
text("RoomName", "Draft group")
back()
assert(node("Contacts").Visible and app.newSelection.alex and node("RoomName").Text == "Draft group")
click("GroupNext")
text("RoomName", "")
local count = #creates
click("CreateConversation")
assert(#creates == count and node("NewChatStatus"):getTextContent() == "Enter a group name.")
text("RoomName", "Our group")
createError = true
click("CreateConversation")
assert(app.newSelection.alex and node("RoomName").Text == "Our group", "Failed creation retains the draft")
createError = false
click("CreateConversation")
assert(creates[#creates].kind == "group" and creates[#creates].member_ids[1] == "alex")
assert(node("NewChatActions").Visible and not next(app.newSelection))

click("KindChannel")
assert(node("ConversationSetup").Visible and not node("Contacts").Visible and node("ConversationVisibility").Visible)
click("ConversationVisibility")
text("RoomName", "News")
click("CreateConversation")
assert(creates[#creates].kind == "channel" and creates[#creates].visibility == "public" and #creates[#creates].member_ids == 0)
click("SearchChannels")
text("FindPerson", "News")
submitSearch()
click("JoinChannelnews")
assert(app.opened.id == "news")
back()
click("ShowInvite")
text("InviteCode", "bad")
click("JoinInvite")
assert(node("NewChatStatus"):getTextContent() == "Invalid invitation")
text("InviteCode", "valid")
click("JoinInvite")
assert(app.opened.id == "invited")

-- Editing, changing steps, leaving the retained page or replacing the account
-- while a request is suspended must never publish the old results.
delayed = true
for _, interrupt in ipairs {
    function() text("FindPerson", "edited while waiting") end,
    function() click("KindChannel"); back() end,
    function() app:show("/"); app:show("/new"); settle() end,
    function() app:resetSessionViews(); app:show("/new"); settle() end,
} do
    text("FindPerson", "@alex")
    click("FindPeople")
    assert(node("DiscoveryLoading"):getParent().Visible)
    interrupt()
    completeRequest()
    settle()
    assert(not find("FoundPersonalex") and not node("DiscoveryLoading"):getParent().Visible)
end
orca.async = savedAsync
print("PASS: Chat New Message lists, group selection, channel/invite flows and stale discovery")
