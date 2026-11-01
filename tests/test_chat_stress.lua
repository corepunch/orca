-- Offline long-conversation stress through the real scope, XML rows and native
-- layout. Override ORCA_CHAT_STRESS_ROWS for manual larger runs; no service or
-- image downloads are used. Timings are diagnostic, work counts are asserted.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/chat")
require("orca.core.startup").load_plugins()
local data = require "orca.data"
local App = require "Chat/App"
local Chat = require "api.chat"
local Media = require "Chat/media"
local count = tonumber(os.getenv("ORCA_CHAT_STRESS_ROWS")) or 1000
assert(count >= 60 and count % 1 == 0)
local pending, savedAsync, savedHost = {}, orca.async, data.Scope.defaultHost
orca.async = function(fn, ...)
    local args = table.pack(...)
    pending[#pending + 1] = coroutine.create(function() fn(table.unpack(args, 1, args.n)) end)
end
data.Scope.defaultHost = function()
    local host = savedHost()
    function host.after() return { cancel = function() end } end
    return host
end
local function settle()
    repeat
        core.flushQueue()
        local ran = #pending > 0
        while #pending > 0 do
            local co = table.remove(pending, 1)
            while coroutine.status(co) ~= "dead" do
                local ok, err = coroutine.resume(co)
                assert(ok, err)
            end
        end
    until not ran
end
local client = Chat.mock { clock = function() return 1700000000 end }
assert(client:signIn("demo@example.invalid", "password123"))
local me, rows = client:getSession().user.id, {}
local function message(index)
    return { id = tostring(index), room_id = "room1", sender_id = index % 3 == 0 and "other" or me,
        created_at = "2025-10-25T12:00:00Z",
        body = "Message " .. index .. ": " .. string.rep("A long conversation keeps its place while new messages arrive. ", index % 4 + 1) }
end
client.listMessagePage = function() return { messages = rows, has_more = false } end
local app = App { client = client, media = Media.new() }
app.activate_controller = function(self, controller) self.screen = controller.view; return controller end
app.show = function(self, route) self:activate_controller(self:dispatch(route or "/")) end
app:show("/"); settle()
app.room = { id = "room1", kind = "group", name = "Stress conversation", can_post = true, role = "owner" }
app:show("/thread"); settle()
app.screen.ResizeMode = "NoResize"
local function refresh()
    app:refreshMessages { quiet = true }; settle()
    core.advanceFrame(); core.runAllPrograms(app.screen)
    app.screen:UpdateLayout(390, 844)
end
local function timed(fn)
    local start = os.clock(); fn(); return (os.clock() - start) * 1000
end
for index = 1, count do rows[index] = message(index) end
local initial = timed(refresh)
local list = app.messageList
local items = assert(list:findChild("MessageItems", true))
local function childCount()
    local total, child = 0, items:getFirstChild()
    while child do total = total + 1; child = child:getNext() end
    return total
end
assert(childCount() == count, "The long conversation retains every requested message")
local bottom = function() return math.min(0, list.ActualHeight - list.ScrollHeight) end
assert(math.abs(list.ContentOffset.Y - bottom()) < 1, "Long conversations open at the latest message")
local retained = assert(items:findChild("MessageEntry" .. count))
assert(items.ActualHeight >= retained.ActualY + retained.ActualHeight - 1,
    "The full native conversation extent includes its last message beyond 16-bit sizes")
assert(list.ScrollHeight >= items.ActualY + items.ActualHeight - 1,
    "The entire conversation remains reachable through the scroll range")
local store, updates = app.threadScope.store, 0
local update = store.update
store.update = function(self, record, values)
    if record.entity.name == "ChatMessage" or record.entity.name == "ChatAttachment" then updates = updates + 1 end
    return update(self, record, values)
end
local unchanged = timed(refresh)
local unchangedUpdates = updates
updates = 0
list.ContentOffset = { X = 0, Y = bottom() / 2 }
local anchor, anchorY, child = nil, nil, items:getFirstChild()
while child do
    local y = items.ActualY + child.ActualY + list.ContentOffset.Y
    if y + child.ActualHeight > list.AdjustedContentInsetTop then anchor, anchorY = child, y; break end
    child = child:getNext()
end
assert(anchor, "The middle of the long conversation has a reading anchor")
rows[#rows + 1] = message(count + 1)
local append = timed(refresh)
local appendUpdates = updates
assert(childCount() == count + 1 and items:findChild("MessageEntry" .. count) == retained,
    "Appending retains existing native rows")
assert(math.abs(items.ActualY + anchor.ActualY + list.ContentOffset.Y - anchorY) < 1,
    "Appending below the middle reader preserves their visible message")
local scroll = timed(function()
    for step = 0, 20 do
        list.ContentOffset = { X = 0, Y = bottom() * step / 20 }
        core.advanceFrame(); core.runAllPrograms(app.screen)
        app.screen:UpdateLayout(390, 844)
        app:afterPaint(); settle()
    end
end)
print(("CHAT STRESS rows=%d initial_ms=%.1f unchanged_ms=%.1f append_ms=%.1f scroll_21_ms=%.1f unchanged_updates=%d append_updates=%d")
    :format(count, initial, unchanged, append, scroll, unchangedUpdates, appendUpdates))
-- The native view remains exercised above even when these work bounds fail.
assert(unchangedUpdates == 0, "An unchanged poll does not re-decorate the retained conversation")
assert(appendUpdates <= 3, "Appending decorates only the new message and changed sender-run boundary")
store.update = update
app.threadScope:dispose()
orca.async, data.Scope.defaultHost = savedAsync, savedHost
print("PASS: long chat native rows, refresh work, identity, reading position and scrolling")
