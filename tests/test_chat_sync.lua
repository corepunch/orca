package.path = "apps/chat/?.lua;" .. package.path
local savedCore, savedPolling = package.loaded["orca.core"], package.loaded["Chat/polling"]
package.loaded["Chat/polling"] = require "polling"
local owner
package.loaded["orca.core"] = { Timer = function(props)
    owner = props
    owner.nextId, owner.delays, owner.canceled = 0, {}, {}
    function owner:setTimer(delay)
        self.nextId = self.nextId + 1
        self.delays[self.nextId] = delay
        return self.nextId
    end
    function owner:cancelTimer(id) self.canceled[id] = true end
    return owner
end }
local Sync = require "sync"
local function fixture()
    local session = { user = { id = "me" } }
    local screen = {}
    local app = { screen = screen, navigationEpoch = 1, sessionEpoch = 1, rooms = {}, contacts = {},
        rows = { { id = "one", name = "One" } }, directory = {}, reads = 0, contactReads = 0, renders = 0,
        query = "one", filter = "Unread", composer = { Text = "draft" } }
    app.client = { getSession = function() return session end }
    app.shell = { screen = screen, active = "Chats" }
    app.roomLists = { All = { ContentOffset = { X = 0, Y = -20 } } }
    app.models = { Rooms = {}, Contacts = {} }
    function app.models.Rooms:select()
        app.reads = app.reads + 1
        if app.readHook then app.readHook() end
        return app.rows, app.readError
    end
    function app.models.Contacts:select()
        app.contactReads = app.contactReads + 1
        return app.directory
    end
    function app:renderRoomList() self.renders = self.renders + 1; self.roomLists.All.ContentOffset = { X = 0, Y = 0 } end
    function app:notice() error("Quiet polling must not replace foreground status") end
    function app:resetSessionViews() self.sessionEpoch = self.sessionEpoch + 1 end
    function app:show(route) self.redirect = route end
    function app:logout() session = nil end
    return app
end
local function fire(id)
    id = id or owner.nextId
    if owner.Tick then owner.Tick(owner, { timerId = id }) end
end
local app = fixture()
local empty = fixture()
empty.rows, empty.rooms = {}, nil
empty.contacts, empty.contactsById = { { id = "old-peer" } }, { ["old-peer"] = { id = "old-peer" } }
assert(Sync.refreshInbox(empty, function() return true end, true))
assert(empty.reads == 1 and empty.contactReads == 0 and #empty.rooms == 0,
    "An empty inbox must finish with one room request and no contacts request")
assert(empty.renders == 1, "The first empty response must replace the inbox loading state")
assert(#empty.contacts == 0 and next(empty.contactsById) == nil,
    "Leaving the last shared room must discard the old contact directory")
empty.rows, empty.directory = { { id = "new-room", name = "New room" } }, { { id = "new-peer" } }
assert(Sync.refreshInbox(empty, function() return true end, true))
assert(empty.contactReads == 1 and empty.contactsById["new-peer"],
    "Contacts must still load when the inbox gains a room")
empty.rows = {}
assert(Sync.refreshInbox(empty, function() return true end, true))
assert(empty.contactReads == 1 and #empty.contacts == 0 and next(empty.contactsById) == nil,
    "Losing the last shared room must clear contacts without another directory request")
local sync = Sync.new(app)
sync:activate()
assert(owner.delays[1] == 15000 and app.reads == 0)
app.readHook = function() assert(owner.canceled[1], "Native repeating timer must stop before HTTP work") end
fire()
assert(app.reads == 1 and app.renders == 1 and owner.delays[2] == 15000)
assert(app.roomLists.All.ContentOffset.Y == -20 and app.query == "one" and app.filter == "Unread")
app.readHook = nil
app.rows = { { id = "one", name = "One" } }
fire()
assert(app.reads == 2 and app.renders == 1, "Unchanged inbox data must not rebuild rows")
fire(1)
assert(app.reads == 2, "Queued ticks from canceled timers must do nothing")
sync:setForeground(false)
fire()
assert(app.reads == 2)
sync:setForeground(true)
assert(owner.delays[owner.nextId] == 1, "Resume must schedule an immediate refresh")
fire()
assert(app.reads == 3)
sync:stop()
fire()
assert(app.reads == 3)

-- A response from a previous route is discarded before publishing rows.
app.rows = { { id = "changed", name = "Changed" } }
app.readHook = function() app.navigationEpoch = app.navigationEpoch + 1 end
sync:activate()
fire()
assert(app.rooms[1].id == "one" and app.renders == 1)
app.readHook = nil

-- Manual refresh and polling use the same lock across network yields.
app.readHook = function() coroutine.yield() end
local pending = coroutine.create(function() Sync.refreshInbox(app, function() return true end, true) end)
assert(coroutine.resume(pending))
local reads = app.reads
assert(Sync.refreshInbox(app, function() return true end, true))
assert(app.reads == reads)
assert(coroutine.resume(pending) and not app.inboxRefreshBusy)
app.readHook = nil

-- A replacement session starts immediately, and an old completion cannot unlock it.
local raced = fixture()
raced.readHook = function() coroutine.yield() end
local oldEpoch = raced.sessionEpoch
local old = coroutine.create(function()
    Sync.refreshInbox(raced, function() return raced.sessionEpoch == oldEpoch end, true)
end)
assert(coroutine.resume(old))
raced.sessionEpoch = oldEpoch + 1
local replacement = coroutine.create(function() Sync.refreshInbox(raced, function() return true end, true) end)
assert(coroutine.resume(replacement) and raced.reads == 2, "A new session must not wait on the old session's inbox lock")
local lock = raced.inboxRefreshBusy
assert(coroutine.resume(old) and raced.inboxRefreshBusy == lock and #raced.rooms == 0,
    "Old responses must neither publish rows nor release a newer session's lock")
assert(coroutine.resume(replacement) and not raced.inboxRefreshBusy and raced.rooms[1].id == "one")

-- Thread polling preserves scroll/drafts and uses its own three-second cadence.
app.room = { id = "thread" }
app.messageList = { getRoot = function() return app.screen end }
function app:renderMessages(options)
    assert(options.current() and options.preserveScroll and options.quiet)
    self.threadReads = (self.threadReads or 0) + 1
    return true
end
sync:activate()
assert(owner.delays[owner.nextId] == 3000)
fire()
assert(app.threadReads == 1 and app.composer.Text == "draft")
app.sending = true
fire()
assert(app.threadReads == 1, "Polling must not compete with an active send")
app.sending = false
function app:renderMessages() self:logout(); return nil, { status = 401 } end
fire()
assert(app.redirect == "/", "Revocation must remove authenticated views")
package.loaded["orca.core"], package.loaded["Chat/polling"] = savedCore, savedPolling
package.loaded.sync = nil
print("PASS: Chat timer bridge, foreground lifecycle, inbox updates and stale-read guards")
