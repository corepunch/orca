-- The open conversation through the real thread page: the Conversation
-- scope, XML templates, photo media, scroll policy and polling. The service
-- is the mock client with controllable pages and photos; background work and
-- timers run when the test says so, and nothing paints.
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
local WHEN = "2025-10-25T12:00:00Z"
local file = assert(io.open("tests/fixtures/chat/photo.jpg", "rb"))
local JPEG = file:read("*a"); file:close()

local queue, timers = {}, {}
local savedAsync, savedHost = orca.async, data.Scope.defaultHost
-- Event handlers arrive as orca.async(handler, self, args, sender).
orca.async = function(fn, ...)
    local args = table.pack(...)
    queue[#queue + 1] = coroutine.create(function() return fn(table.unpack(args, 1, args.n)) end)
end
data.Scope.defaultHost = function()
    local host = savedHost()
    function host.after(ms, fn)
        local timer = { ms = ms, fn = fn }
        function timer:cancel() self.canceled = true end
        timers[#timers + 1] = timer
        return timer
    end
    return host
end
local function resume(co)
    local ok, err = coroutine.resume(co)
    assert(ok, err)
end
-- Runs queued messages and work to completion, including what they queue;
-- event handlers run as queued work too.
local function settle()
    repeat
        core.flushQueue()
        local ran = #queue > 0
        while #queue > 0 do
            local co = table.remove(queue, 1)
            while coroutine.status(co) ~= "dead" do resume(co) end
        end
    until not ran
end
local function poll()
    for index = #timers, 1, -1 do
        local timer = timers[index]
        if not timer.canceled and timer.ms == 3000 then
            table.remove(timers, index)
            timer.fn()
            return true
        end
    end
end

local function children(node)
    local result, child = {}, node:getFirstChild()
    while child do result[#result + 1] = child; child = child:getNext() end
    return result
end
local function shown(node)
    while node do
        if node.Visible == false then return false end
        node = node:getParent()
    end
    return true
end
local function count(node, predicate)
    local total = predicate(node) and 1 or 0
    for _, child in ipairs(children(node)) do total = total + count(child, predicate) end
    return total
end
local function near(a, b, message) assert(math.abs(a - b) < 1, message .. ": " .. a .. " vs " .. b) end

local function fixture(options)
    options = options or {}
    -- Timers of earlier fixtures' apps never fire again.
    for index = #timers, 1, -1 do timers[index] = nil end
    local client = Chat.mock { clock = function() return 1700000000 end }
    assert(client:signIn("demo@example.invalid", "password123"))
    local me = client:getSession().user.id
    local f = { me = me, rows = {}, reads = {}, photos = {}, stages = {} }
    function f.message(id, body, sender)
        return { id = tostring(id), room_id = "room1", body = body, sender_id = sender or me, created_at = WHEN }
    end
    for i = 1, 6 do f.rows[i] = f.message(i, "Message " .. i) end
    client.listMessagePage = function(_, _, page)
        f.reads[#f.reads + 1] = { limit = page.limit, before = page.before }
        if f.readHook then f.readHook() end
        if f.error then return nil, f.error end
        local rows = page.before and f.older or f.rows
        return { messages = rows, has_more = (page.before and f.olderMore or f.hasMore) or false,
            next_cursor = f.nextCursor }
    end
    client.getPhoto = function(_, attachment, thumbnail)
        f.stages[#f.stages + 1] = thumbnail and "preview" or "full"
        if not thumbnail then
            f.photos[attachment.id] = (f.photos[attachment.id] or 0) + 1
            if f.photoHook then return f.photoHook(attachment) end
        end
        return JPEG
    end
    local app = App { client = client, media = Media.new() }
    app.activate_controller = function(self, controller) self.screen = controller.view; return controller end
    app.show = function(self, route) self:activate_controller(self:dispatch(route or "/")) end
    app:show("/")
    settle()
    app.room = { id = "room1", kind = options.kind or "direct", name = "Newsroom", can_post = true, role = "owner" }
    app:show("/thread")
    settle()
    function f.paint()
        local screen = app.screen
        core.advanceFrame(); core.runAllPrograms(screen)
        screen:UpdateLayout(screen.Width, screen.Height)
        app:afterPaint()
        settle()
        core.advanceFrame(); core.runAllPrograms(screen)
        screen:UpdateLayout(screen.Width, screen.Height)
    end
    function f.refresh(quiet)
        app:refreshMessages { quiet = quiet }
        settle()
        core.advanceFrame(); core.runAllPrograms(app.screen)
    end
    function f.entry(id) return assert(app.screen:findChild("MessageEntry" .. id, true), "Missing message " .. id) end
    function f.inside(id, name) return assert(f.entry(id):findChild(name, true), "Missing " .. name .. " in " .. id) end
    f.list = app.messageList
    f.messages = app.threadScope:member("Messages")
    return app, f
end
local function tails(entry)
    return count(entry, function(child)
        return (child.Name == "IncomingTail" or child.Name == "OutgoingTail") and shown(child)
    end)
end

-- Sender runs decorate their first and last messages; groups name senders.
for _, kind in ipairs { "group", "direct", "channel" } do
    local app, f = fixture { kind = kind }
    app.scope.store:put("ChatContact", { id = "other", display_name = "Alex" })
    f.rows = {}
    for i, sender in ipairs { "other", "other", f.me, f.me, "other" } do f.rows[i] = f.message(i, "Body " .. i, sender) end
    f.refresh()
    local expected = kind == "channel" and { 1, 0, 1, 0, 1 } or { 1, 0, 0, 1, 1 }
    for i = 1, 5 do
        local entry = f.entry(i)
        assert(tails(entry) == expected[i], kind .. " sender-run tails")
        assert(shown(f.inside(i, "SenderSection")) == (kind == "group" and (i == 1 or i == 5)),
            "Group sender names appear only at the run's start")
    end
    assert(f.inside(1, "MessageSender").Text == (kind == "channel" and "Newsroom" or "Alex"))
    local rows = app.messageList:findChild("MessageItems", true)
    assert(rows.ItemsSource and rows.ItemTemplate, "The native list binds ItemsSource and ItemTemplate")
end

-- A photo-only album has one timestamp, over its last photo; thumbnails load
-- before full photos, and edits keep the frames.
for _, kind in ipairs { "direct", "group" } do
    for _, sender in ipairs { "me", "other" } do
        local app, f = fixture { kind = kind }
        f.rows = { f.message("album", nil, sender == "me" and f.me or "other") }
        f.rows[1].edited_at = WHEN
        f.rows[1].attachments = { { id = "one", thumbnail_width = 4, thumbnail_height = 3 }, { id = "two" } }
        f.refresh()
        app.screen:UpdateLayout(390, 844)
        local entry, bubble = f.entry("album"), f.inside("album", "MessageBubble")
        local first, last = f.inside("album", "PhotoFrameone"), f.inside("album", "PhotoFrametwo")
        local time, content = last:findChild("PhotoTime"), last:findChild("PhotoContent")
        assert(time.Text == "edited 12:00" and shown(time) and not shown(first:findChild("PhotoTime")),
            "Only the album's last photo carries the time")
        assert(time.MarginRight == 6 and time.MarginBottom == 6)
        assert(not shown(f.inside("album", "Caption")) and tails(entry) == 0, "Photo-only rows have neither footer nor tail")
        assert(shown(f.inside("album", "SenderSection")) == (kind == "group" and sender == "other"))
        -- Photos round the corners they share with the bubble, not under a sender name.
        local topRadius = (kind == "group" and sender == "other") and 0 or 6
        assert(bubble.BorderRadius.TopLeftRadius == 6 and first.BorderRadius.TopLeftRadius == topRadius
            and first.BorderRadius.BottomRightRadius == 6, "Photo corners follow the bubble")
        local loading = content:findChild("PhotoLoading")
        assert(shown(loading) and loading.IgnoreHitTest and loading.Width == 32 and loading.Height == 32
            and loading.Source == "Chat/assets/icons/refresh-cw.svg", "Pending photos show a loading icon")
        near(loading.ActualX + loading.ActualWidth / 2, content.ActualWidth / 2, "The loading icon centers horizontally")
        near(loading.ActualY + loading.ActualHeight / 2, content.ActualHeight / 2, "The loading icon centers vertically")
        assert(#f.stages == 0, "Building rows does not fetch photos")
        f.paint()
        assert(table.concat(f.stages, ",") == "preview,preview,full,full",
            "Every visible thumbnail starts before any full download")
        local image = last:findChild("Photo", true)
        image:send("Node.LoadView")
        assert(shown(image) and image.Source and not shown(loading), "Loaded photos replace the loading icon")
        f.rows[1].updated_at = "message edit"
        f.refresh(true)
        f.paint()
        assert(f.entry("album") == entry and f.inside("album", "PhotoFrametwo") == last and f.photos.two == 1,
            "A message edit keeps the photo frames and their files")
        f.rows[1].body = "A caption"
        f.refresh(true)
        assert(f.entry("album") == entry and shown(f.inside("album", "Caption"))
            and f.inside("album", "MessageBody").Text == "A caption" and not shown(time), "Caption changes update the same row")
        assert(f.inside("album", "MessageTime").Text == "edited 12:00" and tails(entry) == 1)
        assert(f.inside("album", "PhotoFrametwo").BorderRadius.BottomLeftRadius == 0, "A captioned photo meets its caption squarely")
        f.rows[1].attachments = {}
        f.refresh(true)
        assert(f.entry("album") == entry and not shown(f.inside("album", "Photos")), "A text edit removes the album")
    end
end

-- A full photo that fails keeps its thumbnail and offers retry; a refresh retries.
do
    local app, f = fixture()
    f.rows = { f.message("pic", "", f.me) }
    f.rows[1].attachments = { { id = "flaky" } }
    local failures = 1
    f.photoHook = function()
        if failures > 0 then failures = failures - 1; return nil, { message = "Temporarily unavailable" } end
        return JPEG
    end
    f.refresh()
    f.paint()
    local frame = f.inside("pic", "PhotoFrameflaky")
    assert(shown(frame:findChild("RetryPhoto")) and shown(frame:findChild("Photo", true)),
        "A failed full photo keeps the thumbnail and offers retry")
    f.paint()
    assert(f.photos.flaky == 1, "Paints do not retry a failed photo by themselves")
    f.refresh(true)
    f.paint()
    assert(f.photos.flaky == 2 and not shown(frame:findChild("RetryPhoto")), "A refresh retries failed photos")
    -- Retry is also a button on the photo.
    failures = 1
    f.rows[2] = f.message("pic2", "", f.me)
    f.rows[2].attachments = { { id = "second" } }
    f.refresh(true)
    f.paint()
    local second = f.inside("pic2", "PhotoFramesecond")
    assert(f.photos.second == 1 and shown(second:findChild("RetryPhoto")))
    second:findChild("RetryPhoto"):send("Node.LeftButtonUp")
    settle()
    f.paint()
    assert(f.photos.second == 2 and not shown(second:findChild("RetryPhoto")), "Retry downloads the photo again")
end

-- The list keeps the reader's place: at the end it follows new messages;
-- otherwise the first visible message stays put.
do
    local app, f = fixture()
    app:notice("Could not refresh messages.", true)
    f.rows = {}
    for i = 1, 20 do f.rows[i] = f.message(i, "Message " .. i .. " occupies a native measured row.") end
    f.refresh()
    assert(app.status:getTextContent() == "", "A refresh the reader asked for clears old feedback")
    local list = f.list
    local bottom = function() return math.min(0, list.ActualHeight - list.ScrollHeight) end
    near(list.ContentOffset.Y, bottom(), "The conversation opens at its newest message")
    local first = f.entry(1)
    list.ContentOffset = { X = 0, Y = bottom() + 120 }
    local oldY = list.ContentOffset.Y
    f.rows[#f.rows + 1] = f.message(21, "Incoming", "other")
    f.refresh(true)
    near(list.ContentOffset.Y, oldY, "New messages below do not move a reader who scrolled up")
    assert(f.entry(1) == first, "Rows keep their identity across refreshes")
    list.ContentOffset = { X = 0, Y = bottom() }
    f.rows[#f.rows + 1] = f.message(22, "Another", "other")
    f.refresh(true)
    near(list.ContentOffset.Y, bottom(), "A reader at the newest message follows new ones")
    list.ContentOffset = { X = 0, Y = bottom() + 175 }
    local rows = list:findChild("MessageItems", true)
    local anchor, anchorY
    for _, entry in ipairs(children(rows)) do
        if rows.ActualY + entry.ActualY + entry.ActualHeight + list.ContentOffset.Y > list.AdjustedContentInsetTop then
            anchor, anchorY = entry, rows.ActualY + entry.ActualY + list.ContentOffset.Y
            break
        end
    end
    table.remove(f.rows, 1)
    f.refresh(true)
    near(rows.ActualY + anchor.ActualY + list.ContentOffset.Y, anchorY, "A moving result window keeps the first visible message")
end

-- Navigation, sign-out and the caller's lifetime reject late results.
for _, invalidate in ipairs {
    function(app) app:show("/") end,
    function(app) app.client:signOut() end,
} do
    local app, f = fixture()
    local retained = f.entry(1)
    f.rows[1].body = "Stale edit"
    f.readHook = function() invalidate(app) end
    app:refreshMessages()
    settle()
    assert(retained:findChild("MessageBody", true).Text == "Message 1", "A late page cannot change a closed conversation")
end

-- A refresh asked for during a suspended poll runs once after it.
do
    local app, f = fixture()
    local reads = #f.reads
    f.readHook = function() if #f.reads == reads + 1 then coroutine.yield() end end
    app:refreshMessages { quiet = true }
    local pending = table.remove(queue, 1)
    resume(pending)
    assert(#f.reads == reads + 1 and f.messages.mode ~= nil)
    app:refreshMessages()
    settle()
    assert(#f.reads == reads + 1, "Refreshes do not overlap")
    resume(pending)
    settle()
    assert(#f.reads == reads + 2 and f.messages.mode == nil, "The queued refresh runs after the active one")
    f.readHook = function() error("unexpected transport failure") end
    app:refreshMessages { quiet = true }
    settle()
    assert(f.messages.mode == nil and f.messages.error, "Exceptions end the request with an error state")
end

-- History: 30 at a time, older pages above without moving the visible
-- message, the newest page polled without dropping history, and an explicit
-- way back when the newest page no longer overlaps.
do
    local app, f = fixture()
    f.hasMore, f.nextCursor = true, "1"
    for i = 7, 20 do f.rows[i] = f.message(i, "Message " .. i .. " occupies a native measured row.") end
    f.older = { { id = "old1", room_id = "room1", sender_id = f.me, created_at = "2025-10-24T12:00:00Z", body = "Older" },
        { id = "1", room_id = "room1", sender_id = f.me, created_at = WHEN, body = "Message 1" } }
    f.refresh()
    assert(f.reads[#f.reads].limit == 30 and f.reads[#f.reads].before == nil)
    local list, rows = f.list, f.list:findChild("MessageItems", true)
    assert(shown(app.screen:findChild("EarlierMessages", true)), "More history offers itself above the messages")
    list.ContentOffset = { X = 0, Y = -120 }
    local visible = f.entry(1)
    local beforeY = rows.ActualY + visible.ActualY + list.ContentOffset.Y
    local reads = #f.reads
    f.paint()
    assert(#f.reads == reads + 1 and f.reads[#f.reads].before == "1" and f.reads[#f.reads].limit == 30,
        "Scrolling near the top reads one older page")
    assert(f.messages.items:count() == 21 and f.messages.items:keys()[1] == "old1", "Overlapping pages dedupe")
    near(rows.ActualY + visible.ActualY + list.ContentOffset.Y, beforeY, "History above keeps the visible message in place")
    reads = #f.reads
    f.paint()
    assert(#f.reads == reads, "The last history page stops automatic reads")
    f.rows = { f.message(20, "Latest edited") }
    f.refresh(true)
    assert(f.messages.items:count() == 21 and f.inside(20, "MessageBody").Text == "Latest edited",
        "Polling the newest page keeps loaded history")
    f.rows = { f.message("new", "Burst after history") }
    f.rows[1].created_at = "2025-10-26T12:00:00Z"
    f.refresh(true)
    assert(f.messages.gap and f.messages.items:count() == 21, "A page that no longer overlaps reports a gap, not a hole")
    assert(app.screen:findChild("HistoryText", true).Text == "New messages · show latest")
    app.screen:findChild("EarlierMessages", true):send("Node.LeftButtonUp")
    settle()
    assert(f.messages.items:count() == 1 and not f.messages.gap, "Showing the latest drops the old history")
end

-- Older pages near the top: failures wait for retry, polling keeps that
-- state, and a cursor the service no longer knows reloads the newest page.
do
    local app, f = fixture()
    f.hasMore, f.nextCursor, f.olderMore = true, "1", true
    f.rows = {}
    for i = 1, 20 do f.rows[i] = f.message(i, "Message " .. i .. " occupies a native measured row.") end
    f.refresh()
    local list = f.list
    list.ContentOffset = { X = 0, Y = -300 }
    local reads = #f.reads
    f.paint()
    assert(#f.reads == reads, "History does not load far from the top")
    list.ContentOffset = { X = 0, Y = -150 }
    f.error = { code = "network", message = "Offline" }
    f.paint()
    assert(#f.reads == reads + 1 and f.messages.moreError.code == "network")
    assert(app.screen:findChild("HistoryText", true).Text == "Could not load earlier messages · retry")
    list.ContentOffset = { X = 0, Y = -140 }
    f.paint()
    assert(#f.reads == reads + 1, "A failed older page waits for an explicit retry")
    f.error = nil
    f.refresh(true)
    assert(f.messages.moreError and f.messages.moreError.code == "network", "Polling keeps the history retry state")
    f.messages.moreError = { code = "stale_cursor", message = "History changed" }
    app.screen:findChild("EarlierMessages", true):send("Node.LeftButtonUp")
    settle()
    assert(f.reads[#f.reads].before == nil and not f.messages.moreError, "A forgotten cursor reloads the newest page")
    -- The control is disabled while an older page is in flight, and leaving discards it.
    f.readHook = function() coroutine.yield() end
    f.messages:loadMore()
    local pending = table.remove(queue, 1)
    resume(pending)
    core.advanceFrame(); core.runAllPrograms(app.screen)
    assert(app.screen:findChild("EarlierMessages", true).IgnoreHitTest, "History controls disable while loading")
    local messages = f.messages
    app:show("/")
    resume(pending)
    settle()
    assert(messages.disposed and messages.items:count() == 0, "Leaving discards an older page in flight")
end

-- Polling: every three seconds while visible and in the foreground, quietly,
-- never during a send, and a revoked session leaves the conversation.
do
    local app, f = fixture()
    app.composer.Text = "draft"
    app:notice("Keep me")
    local reads = #f.reads
    assert(poll(), "An open conversation polls every three seconds")
    settle()
    assert(#f.reads == reads + 1 and app.composer.Text == "draft" and app.status:getTextContent() == "Keep me",
        "Polling keeps the draft and the status line")
    app.threadScope:setForeground(false)
    assert(not poll(), "A background app does not poll")
    app.threadScope:setForeground(true)
    settle()
    assert(#f.reads == reads + 2, "Returning to the foreground refreshes at once")
    app.threadScope:member("Composer").submitting = true
    poll()
    settle()
    assert(#f.reads == reads + 2, "Polling waits while a message is sending")
    app.threadScope:member("Composer").submitting = false
    poll()
    app.client:signOut()
    settle()
    assert(not app.threadScope and app.currentRoute ~= "/thread", "A revoked session leaves the conversation")
end

orca.async, data.Scope.defaultHost = savedAsync, savedHost
print("PASS: Chat conversation runs, photo albums, retry, scroll policy, stale results, history paging and polling")
