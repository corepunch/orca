-- Exercise delayed requests against real Chat controls and route constructors.
local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
filesystem.init("apps/chat")
require("orca.core.startup").load_plugins()
local App = require "Chat/App"
local Chat = require "api.chat"
local savedAsync, pending = orca.async, {}
orca.async = function(fn) pending[#pending + 1] = coroutine.create(fn) end
local function resume(co)
    local ok, value = coroutine.resume(co)
    assert(ok, value)
    return value
end
local function nextRequest()
    local co = assert(table.remove(pending, 1), "Expected scheduled content load")
    resume(co)
    return co
end
local function settle()
    while #pending > 0 do
        local co = nextRequest()
        assert(coroutine.status(co) == "dead", "Unexpected pending request")
    end
    core.flushQueue()
end
local client = Chat.mock()
assert(client:signIn("demo@example.invalid", "password123"))
local app = App { client = client }
app.activate_controller = function(self, controller) self.screen = controller.view; return controller end
local failures = {}
app.reportError = function(_, err) failures[#failures + 1] = tostring(err) end
local roomsSelect, contactsSelect = app.models.Rooms.select, app.models.Contacts.select
app.models.Rooms.select = function(self, ...)
    coroutine.yield("rooms")
    return roomsSelect(self, ...)
end
app.models.Contacts.select = function(self, ...)
    coroutine.yield("contacts")
    return contactsSelect(self, ...)
end
app:show("/")
local shell = app.screen
assert(shell:findChild("Search", true) and shell:findChild("AppBarNew", true), "Inbox chrome must mount before requests")
assert(shell:findChild("InboxLoadingAll", true).Visible and not shell:findChild("Room1", true))
shell:UpdateLayout(390, 844)
local indicator = shell:findChild("InboxLoadingAll", true)
local area = indicator:getParent()
assert(indicator.ActualWidth > 0 and indicator.ActualHeight > 0, "Pending content must show a nonzero indicator")
assert(math.abs(indicator.ActualX + indicator.ActualWidth / 2 - area.ActualWidth / 2) < 1
    and math.abs(indicator.ActualY + indicator.ActualHeight / 2 - area.ActualHeight / 2) < 1,
    "The loading indicator must be centered in the allocated list area")
local inbox = nextRequest()
assert(coroutine.status(inbox) == "suspended")
app.searchInput.Text = "Alex"
app:updateSearch()
assert(app.query == "Alex", "Search remains editable while inbox loads")
app:show("/new")
assert(app.screen == shell and app.shell.active == "New" and shell:findChild("CreateConversation", true),
    "Navigation and new conversation controls must work while the inbox request is suspended")
settle()
resume(inbox)
assert(coroutine.status(inbox) == "suspended" and app.rooms and shell:findChild("Room1", true),
    "Room rows must appear before the contacts request finishes")
resume(inbox)
assert(app.shell.active == "New" and not shell:findChild("InboxLoadingAll", true).Visible,
    "A late inbox response must update its retained list without changing the active tab")
app.models.Rooms.select, app.models.Contacts.select = roomsSelect, contactsSelect

local getProfile = client.getProfile
client.getProfile = function(self) coroutine.yield("profile"); return getProfile(self) end
app:show("/profile")
assert(shell:findChild("ContentLoading", true) and not shell:findChild("SaveProfile", true))
local profile = nextRequest()
app:show("/new")
local status = app.status
resume(profile)
assert(app.shell.active == "New" and app.status == status and shell:findChild("SaveProfile", true),
    "Profile completion must retain the user's current tab and status")
client.getProfile = getProfile

local room = app.rooms[1]
local messagesSelect = app.models.Messages.select
app.models.Messages.select = function(self, ...) coroutine.yield("messages"); return messagesSelect(self, ...) end
app:openRoom(room)
local thread = app.screen
assert(thread:findChild("MessagesLoading", true).Visible and thread:findChild("ThreadPage", true).BackIcon ~= ""
    and thread:findChild("Message", true), "Thread header and composer must mount before loading messages")
local messages = nextRequest()
app.composer.Text = "Draft written during loading"
resume(messages)
assert(app.composer.Text == "Draft written during loading" and not thread:findChild("MessagesLoading", true).Visible,
    "Loading messages must preserve new composer edits")
app.models.Messages.select = messagesSelect

local listMembers = client.listMembers
client.listMembers = function(self, ...) coroutine.yield("members"); return listMembers(self, ...) end
app:show("/details")
local details = app.screen
assert(details:findChild("ContentLoading", true) and details:findChild("BackToThread", true))
local members = nextRequest()
app:show("/thread")
local replacement = app.screen
resume(members)
assert(app.screen == replacement and not details:findChild("Memberme", true),
    "Member results must not populate a discarded details route")
settle()
client.listMembers = listMembers

-- Failure replaces the spinner with a retry action rather than an empty state.
local other = App { client = client }
other.activate_controller = app.activate_controller
other.reportError = app.reportError
local get = client.getProfile
client.getProfile = function() return nil, { message = "Offline" } end
other:show("/profile")
settle()
assert(other.screen:findChild("RetryContent", true) and not other.screen:findChild("ContentLoading", true))
client.getProfile = get
other.screen:findChild("RetryContent", true):send("Node.LeftButtonUp")
core.flushQueue()
while true do
    local message = orca.system.peekMessage()
    if not message then break end
    if not message:is("Window.Paint") then orca.system.dispatchMessage(other.screen, message) end
end
settle()
assert(other.screen:findChild("SaveProfile", true), "Retry must replace the failed pending page with its form")

-- Signing out retires a pending retained tab, even if its request succeeds later.
local late = App { client = client }
late.activate_controller, late.reportError = app.activate_controller, app.reportError
client.getProfile = function(self) local result = get(self); coroutine.yield("profile"); return result end
late:show("/profile")
local oldScreen, request = late.screen, nextRequest()
late:signOut()
local login = late.screen
resume(request)
assert(late.screen == login and not late.shell and login:findChild("RequestCode", true)
    and not oldScreen:findChild("SaveProfile", true), "A late profile result must not rebuild a signed-out session")
client.getProfile = get
-- Restoring a saved login must return a drawable screen before checking Appwrite.
local restoring = Chat.mock()
restoring.restoreSession = function(self)
    coroutine.yield("restore")
    return self:signIn("demo@example.invalid", "password123")
end
local restored = App { client = restoring }
restored.activate_controller, restored.reportError = app.activate_controller, app.reportError
restored.restorePending = true
restored:show("/")
assert(restored.screen:findChild("SessionLoading", true) and not restoring:getSession())
local restoration = nextRequest()
assert(coroutine.status(restoration) == "suspended" and restored.restorePending)
resume(restoration)
assert(restored.screen:findChild("InboxLoadingAll", true) and not restored.restorePending)
settle()
assert(restored.screen:findChild("Room1", true), "Restoration must transition through the usable inbox shell")

-- Inbox failures are retryable failures, never successful empty results.
local failed = App { client = restoring }
failed.activate_controller, failed.reportError = app.activate_controller, app.reportError
local selectRooms = failed.models.Rooms.select
failed.models.Rooms.select = function() return nil, { message = "Offline" } end
failed:show("/")
settle()
assert(failed.rooms == nil and failed.screen:findChild("RetryInboxAll", true)
    and not failed.screen:findChild("InboxLoadingAll", true).Visible)
failed.models.Rooms.select = selectRooms
failed:show("/")
settle()
assert(failed.screen:findChild("Room1", true) and not failed.screen:findChild("RetryInboxAll", true))

assert(#failures == 0, table.concat(failures, "\n"))
orca.async = savedAsync
print("Chat loading: shell-first routes, editable search/composer, retained-tab races, details cancellation, retry and logout passed")
