-- Real native Contacts composition, retained collection rows and offline races.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
require "orca.core"
require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/chat")
require("orca.core.startup").load_plugins()
local App = require "Chat/App"
local Chat = require "api.chat"
local savedAsync = orca.async
orca.async = function(fn, ...) return fn(...) end
local client = Chat.mock()
assert(client:signIn("demo@example.invalid", "password123"))
local listContacts = client.listContacts
client.listContacts = function(self)
    local people = listContacts(self)
    for i = 1, 28 do people[#people + 1] = { id = "extra" .. i, display_name = "Contact " .. i, username = "contact" .. i } end
    return people
end
local app = App { client = client }
app.activate_controller = function(self, controller) self.screen = controller.view; return controller end
app.show = function(self, route)
    self.navigationEpoch = (self.navigationEpoch or 0) + 1
    self.screen = self:dispatch(route or "/").view
end
local function node(name) return assert(app.screen:findChild(name, true), "Missing " .. name) end
-- Layout assertions need native fonts and textures, but recursive GPU paint
-- requests can keep a queue drain alive while replacing an entire shell.
local function flushQueue()
    while true do
        local event = orca.system.peekMessage()
        if not event then return end
        if not event:is("Window.Paint") then orca.system.dispatchMessage(nil, event) end
    end
end
local function drain()
    flushQueue()
    app.screen:UpdateLayout(390, 844)
    app:afterPaint()
    flushQueue()
end
app:show("/contacts")
drain()
assert(app.shell.active == "Contacts" and node("AppBarContacts").IsEnabled == false)
assert(node("AppBarNew") and node("AppBarProfile") and node("AppBarChats"))
assert(node("ContactsPanorama").Title == "contacts")
assert(node("ContactsRecent").Orientation == "Horizontal")
assert(node("ContactsMosaic").ActualWidth == 720 and node("ContactsMosaic").ActualHeight == 360)
assert(node("ContactsTilealex").ActualWidth > 70 and node("ContactsTilealex").ActualHeight > 70)
assert(node("ContactsTilealex"):findChild("AvatarPhoto", true), "The portrait wall must use real avatar loading")
local portraitContent = node("ContactsTilealex"):getFirstChild()
assert(portraitContent.Name == "ContactAvatar" and portraitContent:getNext() == nil,
    "Portraits must occupy the full tile without captions extending into the next row")
assert(node("ContactsTilealex").DataContext.Label ~= "", "Portrait identity stays available in presentation data")
local page = node("ContactsPage")
assert(not node("ContactsFeedback").Visible
    and math.abs(node("ContactsPanorama").ActualHeight - (page.ActualHeight
        - page.AdjustedContentInsetTop - page.AdjustedContentInsetBottom)) < 1,
    "Healthy Contacts must not reserve feedback space")
local state, panorama, portrait = app.contactsState, app.contactsPanorama, node("ContactsTilealex")
local listRow = node("ContactsRowalex")
app.contactsPanorama.SelectedIndex = 1
drain()
node("ContactsSearch").Text = "alex"
drain()
assert(node("ContactsRowalex") == listRow and not node("ContactsEmpty").Visible)
node("ContactsSearch").Text = "unlikely name"
drain()
assert(node("ContactsEmpty").Visible and node("ContactsEmpty"):getTextContent() == "No matching contacts.", tostring(app.contactsState.query) .. " / " .. tostring(node("ContactsEmpty").Visible) .. " / " .. node("ContactsEmpty"):getTextContent())
node("ContactsSearch").Text = ""
app:show("/new"); drain()
assert(app.shell.active == "New" and node("KindGroup") and node("KindChannel"))
app:show("/contacts"); drain()
assert(app.contactsState == state and app.contactsPanorama == panorama and app.contactsPanorama.SelectedIndex == 1)
assert(node("ContactsTilealex") == portrait, "Refreshing a retained Contacts page must preserve native tiles")
node("ContactsRowalex"):send("Node.LeftButtonUp")
drain()
assert(app.room and app.room.peer_id == "alex" and app.currentRoute == "/thread", "Contacts open real model-backed direct conversations")
app:show("/contacts"); drain()
local create = app.models.Rooms.create
app.models.Rooms.create = function() return nil, { message = "Try opening again" } end
node("ContactsRowalex"):send("Node.LeftButtonUp"); drain()
assert(node("ContactsStatus"):getTextContent() == "Try opening again" and node("ContactsStatus").Visible)
assert(not node("ContactsOpening"):getParent().Visible)
assert(node("ContactsStatus"):getParent() == node("ContactsFeedback"), "Feedback must stay outside horizontally moving sections")
assert(node("ContactsFeedback").ActualHeight > 0 and node("ContactsFeedback").ActualWidth <= 390)
app.models.Rooms.create = function(self, values)
    app:show("/new")
    return create(self, values)
end
local before = app.room
app:show("/contacts"); drain()
node("ContactsRowalex"):send("Node.LeftButtonUp"); drain()
assert(app.shell.active == "New" and app.room == before, "A delayed create must not navigate away from a newer tab")
app.models.Rooms.create = create

-- Delayed initial data mounts searchable UI first; finishing offscreen only
-- populates its retained page, while session invalidation discards responses.
local originalContacts, pending = client.listContacts, {}
client.listContacts = function(self) coroutine.yield("contacts"); return originalContacts(self) end
orca.async = function(fn) pending[#pending + 1] = coroutine.create(fn) end
app:resetSessionViews()
app:show("/contacts")
assert(node("ContactsLoading"):getParent().Visible and node("ContactsSearch"))
local request = assert(table.remove(pending, 1))
assert(coroutine.resume(request))
assert(coroutine.status(request) == "suspended")
app:show("/new")
local status = app.status
assert(coroutine.resume(request))
assert(coroutine.status(request) == "dead" and app.shell.active == "New" and app.status == status)
assert(app.contactsState.portraits.byKey.alex, "A late load may populate a retained inactive page")
client.listContacts = originalContacts
orca.async = function(fn, ...) return fn(...) end

-- A failed refresh retains usable portraits and provides retry in both sections.
client.listContacts = function() return nil, { message = "Contacts unavailable" } end
app:show("/contacts"); drain()
assert(node("ContactsTilealex") and node("ContactsRetry").Visible)
assert(not node("ContactsLoading"):getParent().Visible)
client.listContacts = originalContacts
node("ContactsRetry"):send("Node.LeftButtonUp"); drain()
assert(not node("ContactsRetry").Visible and not node("ContactsStatus").Visible)

client.listContacts = function(self) coroutine.yield("contacts"); return originalContacts(self) end
orca.async = function(fn) pending[#pending + 1] = coroutine.create(fn) end
pending = {}
app.contactsState.refresh()
request = assert(table.remove(pending, 1)); assert(coroutine.resume(request))
app:resetSessionViews()
assert(coroutine.resume(request))
assert(coroutine.status(request) == "dead" and app.contactsState == nil, "A previous session cannot repopulate Contacts")
client.listContacts = originalContacts
orca.async = function(fn, ...) return fn(...) end
app:show("/contacts"); drain()
app.models.Rooms.create = function()
    client.session = nil
    return nil, { code = "unauthenticated", message = "Sign in to continue." }
end
node("ContactsRowalex"):send("Node.LeftButtonUp"); drain()
assert(not app.client:getSession() and not app.contactsState and not app.shell,
    "Session expiry during contact activation must clear retained authenticated records")
assert(app.screen:findChild("RequestCode", true), "Expired contact activation must return to authentication")
orca.async = savedAsync
print("PASS: Contacts panorama, photos, search, retained items, direct messages, failures and navigation/session races")
