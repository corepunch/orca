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
-- Bindings run on paint; this suite does not paint.
local function programs(screen)
    core.advanceFrame()
    core.runAllPrograms(screen)
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
programs(shell)
assert(shell:findChild("Search", true) and shell:findChild("AppBarNew", true), "Inbox chrome must mount before requests")
assert(shell:findChild("InboxLoadingAll", true).Visible and not shell:findChild("Roomalex", true))
shell:UpdateLayout(390, 844)
local indicator = shell:findChild("InboxLoadingAll", true)
local area = indicator:getParent()
assert(indicator.ActualWidth > 0 and indicator.ActualHeight > 0, "Pending content must show a nonzero indicator")
assert(math.abs(indicator.ActualX + indicator.ActualWidth / 2 - area.ActualWidth / 2) < 1
    and math.abs(indicator.ActualY + indicator.ActualHeight / 2 - area.ActualHeight / 2) < 1,
    "The loading indicator must be centered in the allocated list area")
local inbox = nextRequest()
assert(coroutine.status(inbox) == "suspended")
shell:findChild("Search", true).Text = "Alex"
core.flushQueue()
assert(app.scope:member("Matching"):get("Search") == "Alex", "Search remains editable while inbox loads")
app:show("/new")
assert(app.screen == shell and app.shell.active == "New" and shell:findChild("CreateConversation", true),
    "Navigation and new conversation controls must work while the inbox request is suspended")
settle()
resume(inbox)
local contactsRequest = nextRequest()
assert(coroutine.status(contactsRequest) == "suspended" and shell:findChild("Roomalex", true),
    "Room rows must appear before the contacts request finishes")
assert(not shell:findChild("Roomweekend", true), "The search typed while loading applies to the late rows")
resume(contactsRequest)
programs(shell)
assert(app.shell.active == "New" and not shell:findChild("InboxLoadingAll", true).Visible and app.contactsById.alex,
    "A late inbox response must update its retained list without changing the active tab")
shell:findChild("Search", true).Text = ""
core.flushQueue()
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

local room = app.scope.store:get("ChatRoom", "alex").raw
local messagesPage = app.models.Messages.page
app.models.Messages.page = function(self, ...) coroutine.yield("messages"); return messagesPage(self, ...) end
app:openRoom(room)
-- The page in view turns out; its completion handler then shows the thread.
core.flushQueue()
nextRequest()
local thread = app.screen
assert(thread:findChild("MessagesLoading", true).Visible and thread:findChild("ThreadPage", true).BackIcon ~= ""
    and thread:findChild("Message", true), "Thread header and composer must mount before loading messages")
local messages = nextRequest()
assert(coroutine.status(messages) == "suspended", "The initial message page must remain pending while composing")
app.composer.Text = "Draft written during loading"
resume(messages)
programs(thread)
assert(app.composer.Text == "Draft written during loading" and not thread:findChild("MessagesLoading", true).Visible,
    "Loading messages must preserve new composer edits")
app.models.Messages.page = messagesPage

local listMembers = client.listMembers
client.listMembers = function(self, ...) coroutine.yield("members"); return listMembers(self, ...) end
app:show("/details")
local details = app.screen
assert(details:findChild("ContentLoading", true) and details:findChild("DetailsPage", true).BackIcon ~= "")
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
assert(restored.screen:findChild("Roomalex", true), "Restoration must transition through the usable inbox shell")

-- Inbox failures are retryable failures, never successful empty results.
local failed = App { client = restoring }
failed.activate_controller, failed.reportError = app.activate_controller, app.reportError
local selectRooms = failed.models.Rooms.select
failed.models.Rooms.select = function() return nil, { message = "Offline" } end
failed:show("/")
settle()
programs(failed.screen)
assert(not failed.screen:findChild("Roomalex", true) and failed.screen:findChild("InboxErrorAll", true).Visible
    and not failed.screen:findChild("InboxEmptyAll", true).Visible
    and not failed.screen:findChild("InboxLoadingAll", true).Visible, "A failed first load shows retry, not empty")
failed.models.Rooms.select = selectRooms
failed.screen:findChild("RetryInboxAll", true):send("Node.LeftButtonUp")
core.flushQueue()
settle()
programs(failed.screen)
assert(failed.screen:findChild("Roomalex", true) and not failed.screen:findChild("InboxErrorAll", true).Visible,
    "Retry loads the inbox")

assert(#failures == 0, table.concat(failures, "\n"))
orca.async = savedAsync
print("Chat loading: shell-first routes, editable search/composer, retained-tab races, details cancellation, retry and logout passed")
