-- The Chat inbox scope (Data/Inbox.xml + providers/chat.lua) against a fake
-- client: request order, contacts, polling, coalescing, local unread/pin
-- state, search and session isolation. No UI or network.
local orca = require "orca"
orca.init()
local filesystem = require "orca.filesystem"
filesystem.init("apps/chat")
local data = require "orca.data"
local Provider = require "Chat/providers/chat"
local checks = 0
local function check(value, message) assert(value, message); checks = checks + 1 end

local function host()
    local h = { pending = {}, timers = {}, reports = {} }
    function h.run(fn)
        local co = coroutine.create(fn)
        assert(coroutine.resume(co))
        if coroutine.status(co) ~= "dead" then h.pending[#h.pending + 1] = co end
    end
    function h.resume() local co = table.remove(h.pending, 1); assert(coroutine.resume(co)) end
    function h.after(ms, fn)
        local timer = { ms = ms, fn = fn }
        function timer:cancel() self.canceled = true end
        h.timers[#h.timers + 1] = timer
        return timer
    end
    function h.tick()
        local due = h.timers; h.timers = {}
        for _, timer in ipairs(due) do if not timer.canceled then timer.fn() end end
    end
    function h.report(err) h.reports[#h.reports + 1] = err end
    return h
end

local function fixture(user)
    local f = { reads = 0, contactReads = 0, rows = {}, directory = {}, opened = {}, read = {}, pinned = {},
        session = { user = { id = user or "me" } } }
    f.client = { getSession = function() return f.session end,
        getAvatars = function() return {} end, getAvatar = function() end }
    f.models = { Rooms = {}, Contacts = {} }
    function f.models.Rooms.select()
        f.reads = f.reads + 1
        if f.yieldRooms then coroutine.yield() end
        if f.roomError then return nil, f.roomError end
        return f.rows
    end
    function f.models.Contacts.select()
        f.contactReads = f.contactReads + 1
        return f.directory
    end
    f.host = host()
    f.scope = data.scope("Chat/Data/Inbox.xml", {
        provider = Provider.new { client = f.client, models = f.models, read = f.read, pinned = f.pinned,
            avatarPaths = {}, openRoom = function(room) f.opened[#f.opened + 1] = room.id end,
            sessionLost = function() f.lost = true end },
        host = f.host, identity = "chat:" .. (user or "me"),
        valid = function() return f.session ~= nil and f.session.user.id == (user or "me") end })
    return f
end
local function room(id, name, unread, subtitle)
    return { id = id, name = name, kind = "direct", peer_id = id, unread = unread or 0, subtitle = subtitle,
        created_at = "2025-01-01T09:30:00Z" }
end

-- Contacts follow rooms: an empty inbox asks once and keeps no directory.
local empty = fixture()
empty.scope:activate()
local rooms, contacts = empty.scope:member("Rooms"), empty.scope:member("Contacts")
check(empty.reads == 1 and empty.contactReads == 0 and rooms.state.ShowEmpty and contacts.state.Count == 0,
    "An empty inbox makes one room request and no contacts request")
check(empty.scope:member("AllRooms").state.ShowEmpty, "The empty inbox shows its empty state")
empty.rows, empty.directory = { room("new", "New room") }, { { id = "peer", display_name = "Peer" } }
rooms:refresh()
check(empty.contactReads == 1 and contacts.state.Count == 1, "Contacts load when the inbox gains a room")
empty.rows = {}
rooms:refresh()
check(empty.contactReads == 1 and contacts.state.Count == 0, "Losing the last room clears contacts without a request")

-- Polling, identity and quiet failures.
local f = fixture()
f.rows = { room("alex", "Alex Morgan", 2, "See you?"), room("weekend", "Weekend people", 0, "Maya: plan"),
    room("jules", "Jules Park", 0) }
f.pinned.alex, f.pinned.weekend = true, true
f.scope:activate()
local all = f.scope:member("AllRooms")
local alex = f.scope.store:get("ChatRoom", "alex")
local row = all.state:findChild("Items"):findChild("alex")
check(row.Title == "Alex Morgan" and row.UnreadText == "2" and row.HasUnread and row.State == "unread"
    and row.Time == "09:30" and row.Initials == "AM" and row.Tone:match("^tone%d$"), "Rows carry presentation fields")
check(f.scope:member("Rooms").def.poll == 15000 and #f.host.timers == 1 and f.host.timers[1].ms == 15000,
    "The inbox polls every 15 seconds while active")
f.rows[1].subtitle = "New message"
f.host.tick()
check(f.reads == 2 and all.state:findChild("Items"):findChild("alex") == row and row.Preview == "New message",
    "A poll updates rows in place")
f.roomError = { code = "network", message = "Offline" }
f.host.tick()
check(f.scope:member("Rooms").state.HasError and all.state.Count == 3 and not all.state.ShowError,
    "A failed poll keeps the rows")
check(f.host.timers[1].ms == 30000, "Failures back off")
f.roomError = nil
f.scope:member("Rooms"):refresh()
check(not f.scope:member("Rooms").state.HasError and f.host.timers[#f.host.timers].ms == 15000, "Success resets the cadence")
f.scope:deactivate()
check(#f.host.timers == 0 or f.host.timers[#f.host.timers].canceled, "Leaving the inbox stops polling")

-- Local state: views filter by unread, pins and search; opening reads.
local unread, pinned, matching = f.scope:member("UnreadRooms"), f.scope:member("PinnedRooms"), f.scope:member("Matching")
check(unread.items:keys()[1] == "alex" and #unread.items:keys() == 1, "Unread lists rooms with unread messages")
check(#pinned.items:keys() == 2 and pinned.state:findChild("Items"):findChild("weekend").ShowPin
    and not pinned.state:findChild("Items"):findChild("alex").ShowPin, "Pins show unless a badge does")
matching:set("Search", "jul")
check(all.items:keys()[1] == "jules" and #all.items:keys() == 1 and unread.state.ShowEmpty,
    "Search filters every list")
matching:set("Search", "")
row = all.state:findChild("Items"):findChild("alex")
local open = f.scope:member("OpenRoom")
open:execute { item = alex }
check(f.opened[1] == "alex" and f.read.alex and alex.values.UnreadCount == 0 and unread.state.Count == 0
    and all.state:findChild("Items"):findChild("alex") == row, "Opening a room marks it read in place")

-- Manual refresh and polling share one in-flight request across yields.
f.yieldRooms = true
local reads = f.reads
f.scope:member("Rooms"):refresh()
f.scope:member("Rooms"):refresh()
check(f.reads == reads + 1 and #f.host.pending == 1, "Overlapping refreshes coalesce")
f.yieldRooms = false
f.host.resume()
check(f.reads == reads + 2, "The coalesced refresh runs once afterwards")

-- A replacement session never receives the old session's response.
local old = fixture("old-user")
old.rows = { room("secret", "Old account") }
old.yieldRooms = true
old.scope:activate()
old.session = { user = { id = "new-user" } }
old.host.resume()
check(old.scope:member("Rooms").state.Count == 0 and not old.scope.store:get("ChatRoom", "secret"),
    "A response for a replaced account is discarded")

-- Validation of the declaration happens at load.
local ok, err = pcall(data.scope, "Chat/Data/Inbox.xml", { provider = {}, host = host() })
check(not ok and err:find("Inbox.xml", 1, true) and err:find("unknown operation 'rooms'", 1, true),
    "A missing provider operation fails at load with the resource and name")

print(("PASS: Chat inbox scope (%d checks)"):format(checks))
