-- Native collection/template tests. The injected service has no network or
-- authentication dependency; XML bindings, layout and photo loading are real.
local orca = require "orca"
orca.init()
local fs = require "orca.filesystem"
local core = require "orca.core"
local ui = require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
fs.init("apps/chat")
require("orca.core.startup").load_plugins()
local Timeline = require "Chat/views/timeline"
local Loader = require "Chat/photo_loader"
local C = require "Chat/views/components"
local WHEN = "2025-10-25T12:00:00Z"
local IMAGE = assert(os.getenv("PWD")) .. "/tests/fixtures/chat/photo.jpg"

local function children(node)
    local result, child = {}, node:getFirstChild()
    while child do result[#result + 1] = child; child = child:getNext() end
    return result
end
local function named(node, name) return assert(node:findChild(name, true), "Missing " .. name) end
local function shown(node)
    while node do
        if not node.Visible then return false end
        node = node:getParent()
    end
    return true
end
local function count(node, predicate)
    local total = predicate(node) and 1 or 0
    for _, child in ipairs(children(node)) do total = total + count(child, predicate) end
    return total
end
local function visibleCount(node, predicate)
    return count(node, function(child) return shown(child) and predicate(child) end)
end
local function bottom(list) return math.min(0, list.ActualHeight - list.ScrollHeight) end
local function near(a, b, message) assert(math.abs(a - b) < 1, message .. ": " .. a .. " vs " .. b) end
local function message(id, body, sender)
    return { id = tostring(id), body = body, sender_id = sender or "me", created_at = WHEN }
end

local function fixture(height)
    local screen = ui.Screen { Width = 390, Height = height or 300, ResizeMode = "NoResize" }
    local layout = ui.Grid { Rows = "1fr auto" }
    local list = ui.StackView { Overflow = "hidden scroll", Spacing = 0 }
    local status = ui.TextBlock { Height = 40, Visible = false }
    layout:addChild(list); layout:addChild(status); screen:addChild(layout)
    local app = { room = { id = "room1", kind = "direct" }, messageList = list, screen = screen,
        sessionEpoch = 1, navigationEpoch = 1, rows = {}, reads = 0, photos = 0, photoStages = {},
        composer = ui.Input { Text = "draft remains" }, photoDrafts = { room1 = "draft-photo" } }
    for i = 1, 6 do app.rows[i] = message(i, "Message " .. i) end
    app.rows[1].attachments = { { id = "photo1" } }
    local session = { user = { id = "me" } }
    app.client = { getSession = function() return session end }
    function app.client:getPhoto(attachment, thumbnail)
        assert(type(attachment) == "table" and attachment.id, "The loader reuses the known attachment metadata")
        app.photoStages[#app.photoStages + 1] = thumbnail and "preview" or "full"
        if thumbnail then return "local fixture JPEG" end
        app.photos = app.photos + 1
        if app.photoHook then app.photoHook() end
        return "local fixture JPEG", nil
    end
    app.models = { Messages = {} }
    function app.models.Messages:page(options)
        assert(options.limit == 30, "Every latest/history request must stay bounded to 30 messages")
        app.lastRequest = options
        app.reads = app.reads + 1
        if app.readHook then app.readHook() end
        if app.error then return nil, app.error end
        return { messages = app.rows, has_more = app.hasMore or false, next_cursor = app.nextCursor }
    end
    function app:getMedia() return { storePreview = function() return IMAGE end } end
    function app:notice(value)
        self.noticeText = value
        status.Visible, status.Text = value ~= "", value
    end
    function app:logout() session = nil; self.sessionEpoch = self.sessionEpoch + 1; self.messagePhotos:clear() end
    function app:task(fn) fn() end
    function app:load(work) work() end
    function app:repaint() end
    function app:reportError(value) self.failure = value end
    function app:openMessageActions(value) self.actionsFor = value.id end
    app.messagePhotos = Loader.new(app)
    function app:loadPhotos()
        screen:UpdateLayout(screen.Width, screen.Height)
        self.messagePhotos:loadVisible()
        core.flushQueue()
    end
    return app, list, screen
end
local function refresh(app, options)
    local result, err = Timeline.refresh(app, options)
    assert(result, app.failure or (err and err.message))
    core.flushQueue()
    app.screen:UpdateLayout(app.screen.Width, app.screen.Height)
end
local function tails(entry)
    return visibleCount(entry, function(child) return child.Name == "IncomingTail" or child.Name == "OutgoingTail" end)
end

-- Every message has its own retained row; adjacency changes only decoration.
for _, kind in ipairs { "group", "direct", "channel" } do
    local app, list = fixture()
    app.room.kind, app.room.name = kind, "Newsroom"
    app.contactsById = { other = { get_display_name = function() return "Alex" end } }
    app.rows = {}
    for i, sender in ipairs { "other", "other", "me", "me", "other" } do app.rows[i] = message(i, "Body " .. i, sender) end
    refresh(app)
    local expected = kind == "channel" and { 1, 0, 1, 0, 1 } or { 1, 0, 0, 1, 1 }
    for i = 1, 5 do
        local entry = named(list, "MessageEntry" .. i)
        assert(named(named(entry, "MessageHolder" .. i), "MessageBubble" .. i))
        assert(tails(entry) == expected[i], kind .. " sender-run tails")
        assert(visibleCount(entry, function(child) return child.Text == "Alex" end)
            == (kind == "group" and (i == 1 or i == 5) and 1 or 0), "Group sender names appear only at the run's start")
    end
    assert(visibleCount(list, function(child) return child.Text == "Newsroom" end) == 0)
    assert(visibleCount(list, function(child) return child.Name == "OutgoingTail" end) == (kind == "channel" and 0 or 1))
    assert(list.ItemsSource and list.ItemTemplate, "The native list binds ItemsSource and ItemTemplate")
end

-- A photo-only album has one overlay timestamp; the loader replaces only content.
for _, kind in ipairs { "direct", "group", "channel" } do
    for _, sender in ipairs { "me", "other" } do
        for _, empty in ipairs { false, "" } do
            local app, list = fixture(700)
            app.room.kind, app.room.name = kind, "Newsroom"
            app.contactsById = { other = { get_display_name = function() return "Alex" end } }
            app.rows = { message("album", empty or nil, sender) }
            app.rows[1].edited_at = WHEN
            app.rows[1].attachments = { { id = "one" }, { id = "two" } }
            refresh(app)
            local entry = named(list, "MessageEntryalbum")
            local bubble, last = named(entry, "MessageBubblealbum"), named(entry, "PhotoFrametwo")
            local time, content = named(last, "MessageTimealbum"), named(last, "PhotoContenttwo")
            assert(time.Text == "edited " .. C.time(WHEN) and time.ForegroundColor.A == 1 and time.BackgroundColor.A > 0)
            assert(time.MarginLeft ~= time.MarginLeft and time.MarginTop ~= time.MarginTop and time.MarginRight == 6 and time.MarginBottom == 6)
            assert(visibleCount(entry, function(child) return child.Name == "MessageTimealbum" end) == 1)
            assert(not shown(named(bubble, "Caption")) and tails(entry) == 0, "Photo-only rows have neither footer nor tail")
            assert(shown(named(bubble, "SenderSection")) == (kind == "group" and sender == "other"))
            local loading = named(content, "PhotoLoadingtwo")
            assert(shown(loading) and loading.IgnoreHitTest and loading.Width == 32 and loading.Height == 32,
                "Pending photos show a centered loading icon")
            assert(loading.Source == "Chat/assets/icons/refresh-cw.svg", "The placeholder reuses the packaged SVG icon")
            near(loading.ActualX + loading.ActualWidth / 2, content.ActualWidth / 2, "The loading icon centers horizontally")
            near(loading.ActualY + loading.ActualHeight / 2, content.ActualHeight / 2, "The loading icon centers vertically")
            assert(visibleCount(content, function(child) return child:getTextContent() == "Loading photo…" end) == 0,
                "The photo placeholder has no loading caption")
            app:loadPhotos()
            assert(app.photos == 2 and named(content, "Phototwo").Source, "Visible local previews resolve native resources")
            assert(table.concat(app.photoStages, ",") == "preview,preview,full,full",
                "Every visible album preview starts before any full download")
            assert(not content:findChild("PhotoLoadingtwo", true), "Loaded photos remove the loading icon")
            app.rows[1].updated_at = "message edit"
            refresh(app, { preserveScroll = true })
            app:loadPhotos()
            assert(named(list, "MessageEntryalbum") == entry and named(last, "MessageTimealbum") == time and app.photos == 2,
                "A message edit retains photo frames, cached previews and overlay identity")
            app.rows[1].body = "A caption"
            refresh(app, { preserveScroll = true })
            assert(named(list, "MessageEntryalbum") == entry and shown(named(bubble, "Caption"))
                and named(bubble, "MessageBody").Text == "A caption" and not shown(time), "Caption changes update the same row")
            assert(named(bubble, "MessageTimealbum").Text == "edited " .. C.time(WHEN) and tails(entry) == 1)
            app.rows[1].attachments = {}
            refresh(app, { preserveScroll = true })
            assert(named(list, "MessageEntryalbum") == entry and not shown(named(bubble, "Photos")), "A text edit removes the album from the retained row")
        end
    end
end

-- Clearing feedback changes the viewport before refresh chooses its bottom.
local app, list = fixture()
app:notice("Could not refresh messages.")
app.screen:UpdateLayout(390, 300)
local reduced = list.ActualHeight
refresh(app)
assert(app.noticeText == "" and list.ActualHeight > reduced)
near(list.ContentOffset.Y, bottom(list), "Initial refresh aligns to the reclaimed viewport")
local first, data = named(list, "MessageEntry1"), named(list, "MessageEntry1").DataContext
assert(app.photos == 0, "Building bound rows must not fetch photos")
list.ContentOffset = { X = 0, Y = 0 }
app:loadPhotos()
assert(app.photos == 1)
refresh(app, { preserveScroll = true, quiet = true })
assert(named(list, "MessageEntry1") == first and first.DataContext == data and app.photos == 1)

-- Use actual measured geometry for scrolling, rather than fixed-height doubles.
app.rows = {}
for i = 1, 20 do app.rows[i] = message(i, "Message " .. i .. " occupies a native measured row.") end
refresh(app)
list.ContentOffset = { X = 0, Y = math.max(bottom(list) + 50, -175) }
local oldY = list.ContentOffset.Y
app.rows[#app.rows + 1] = message(21, "Incoming", "other")
refresh(app, { preserveScroll = true })
near(list.ContentOffset.Y, oldY, "Appending preserves a reader's position")
assert(named(list, "MessageEntry1") == first and app.composer.Text == "draft remains" and app.photoDrafts.room1 == "draft-photo")
list.ContentOffset = { X = 0, Y = bottom(list) }
app.rows[#app.rows + 1] = message(22, "Another", "other")
refresh(app, { preserveScroll = true })
near(list.ContentOffset.Y, bottom(list), "A reader at the bottom follows new messages")
list.ContentOffset = { X = 0, Y = -175 }
local anchor, anchorY
for _, entry in ipairs(app.timelineSnapshot.entries) do
    if entry.node.ActualY + entry.node.ActualHeight + list.ContentOffset.Y > list.AdjustedContentInsetTop then
        anchor, anchorY = entry.node, entry.node.ActualY + list.ContentOffset.Y
        break
    end
end
table.remove(app.rows, 1)
refresh(app, { preserveScroll = true })
near(anchor.ActualY + list.ContentOffset.Y, anchorY, "A moving result window preserves the first visible message")

-- Navigation, account replacement and caller lifetimes reject stale fetches.
for _, invalidate in ipairs {
    function(value) value.navigationEpoch = value.navigationEpoch + 1 end,
    function(value) value:logout() end,
    function(value) value.room = { id = "another" } end,
} do
    local stale, staleList = fixture()
    refresh(stale)
    local retained = named(staleList, "MessageEntry1")
    stale.rows[1].body = "Stale edit"
    stale.readHook = function() invalidate(stale) end
    refresh(stale, { preserveScroll = true })
    assert(named(staleList, "MessageEntry1") == retained and named(retained, "MessageBody").Text == "Message 1")
end
local guarded, guardedList = fixture()
refresh(guarded)
guarded.rows[1].body = "Caller is gone"
refresh(guarded, { preserveScroll = true, current = function() return false end })
assert(named(named(guardedList, "MessageEntry1"), "MessageBody").Text == "Message 1")

-- Deferred previews cannot re-scroll the list or apply another session's result.
app, list = fixture()
refresh(app)
first = named(list, "MessageEntry1")
app.rows[1].body = "Edited before preview"
refresh(app, { preserveScroll = true })
assert(named(list, "MessageEntry1") == first and named(first, "MessageBody").Text == "Edited before preview")
list.ContentOffset = { X = 0, Y = 0 }
app.photoHook = function() list.ContentOffset = { X = 0, Y = -50 } end
app:loadPhotos()
near(list.ContentOffset.Y, -50, "Deferred image work does not overwrite scrolling")
app, list = fixture()
refresh(app)
list.ContentOffset = { X = 0, Y = 0 }
app.photoHook = function()
    app.previewBeforeLogout = list:findChild("Photophoto1", true)
    app:logout()
end
app:loadPhotos()
assert(not app.timelineSnapshot.photos.photo1 and list:findChild("Photophoto1", true) == app.previewBeforeLogout,
    "Logout discards the deferred full photo without replacing the earlier preview")

-- Failed full downloads retain their preview and retry on an unchanged snapshot.
app, list = fixture()
local calls = 0
app.client.getPhoto = function(_, attachment, thumbnail)
    if thumbnail then return "local fixture JPEG" end
    calls = calls + 1
    if calls == 1 then return nil, { message = "Temporarily unavailable" } end
    return "local fixture JPEG"
end
refresh(app)
list.ContentOffset = { X = 0, Y = 0 }
app:loadPhotos()
assert(app.timelineSnapshot.retryPhotos)
refresh(app, { preserveScroll = true })
app:loadPhotos()
assert(calls == 2 and not app.timelineSnapshot.retryPhotos)
refresh(app, { preserveScroll = true })
assert(calls == 2)

-- Manual refresh during a suspended poll runs once after it, without overlap.
app, list = fixture()
local calls = 0
app.readHook = function() calls = calls + 1; if calls == 1 then coroutine.yield() end end
local pending = coroutine.create(function() assert(Timeline.refresh(app, { preserveScroll = true })) end)
assert(coroutine.resume(pending))
assert(app.messageRefreshBusy and app.reads == 1)
assert(Timeline.refresh(app) and app.reads == 1)
local ok, err = coroutine.resume(pending)
assert(ok, err)
assert(not app.messageRefreshBusy and app.reads == 2, "The queued refresh runs after the active poll")
app.readHook = function() error("unexpected transport failure") end
local ok, err = Timeline.refresh(app, { preserveScroll = true, quiet = true })
assert(not ok and err.code == "refresh_failed" and not app.messageRefreshBusy, "Exceptions must release the read lock")
-- Initial and polling fetch only 30; older pages prepend without moving the visible row.
app, list = fixture()
app.hasMore, app.nextCursor = true, "1"
assert(Timeline.refresh(app))
assert(app.lastRequest.limit == 30 and app.lastRequest.before == nil)
list.ContentOffset = { X = 0, Y = -120 }
local anchorBefore = named(list, "MessageEntry1").ActualY + list.ContentOffset.Y
app.rows = {
    { id = "old1", sender_id = "me", created_at = "2025-10-24T12:00:00Z", body = "Older" },
    { id = "1", sender_id = "me", created_at = WHEN, body = "Duplicate must keep current row" },
}
app.hasMore = false
assert(Timeline.refresh(app, { older = true, preserveScroll = true }))
assert(app.lastRequest.before == "1" and app.lastRequest.limit == 30)
assert(#app.timelinePage.messages == 7 and app.timelinePage.messages[2].body == "Message 1", "Overlapping pages dedupe IDs and preserve current edits")
assert(named(list, "MessageEntry1").ActualY + list.ContentOffset.Y == anchorBefore,
    "Prepending history must preserve the first visible message's screen position")
assert(app.composer.Text == "draft remains" and app.photoDrafts.room1 == "draft-photo")
local reads = app.reads
Timeline.loadNearTop(app)
assert(app.reads == reads, "The final history page must stop automatic reads")
app.rows = { { id = "6", sender_id = "me", created_at = WHEN, body = "Latest edited" } }
assert(Timeline.refresh(app, { preserveScroll = true, quiet = true }))
assert(#app.timelinePage.messages == 7 and app.timelinePage.messages[7].body == "Latest edited",
    "Latest polling must refresh current rows without dropping loaded history")
app.rows = { { id = "new", sender_id = "me", created_at = "2025-10-26T12:00:00Z", body = "Burst after history" } }
assert(Timeline.refresh(app, { preserveScroll = true, quiet = true }))
assert(app.timelinePage.latestPending and #app.timelinePage.messages == 7, "A nonoverlapping latest page must offer explicit recovery instead of creating a history gap")
assert(named(list, "EarlierMessages"))
assert(Timeline.refresh(app, { reset = true }))
assert(#app.timelinePage.messages == 1 and not app.timelinePage.latestPending)

-- Near-top automatic demand, serialized requests, retry and stale-cursor recovery.
app, list = fixture()
app.hasMore, app.nextCursor = true, "1"
assert(Timeline.refresh(app))
list.ContentOffset = { X = 0, Y = -300 }
reads = app.reads
Timeline.loadNearTop(app)
assert(app.reads == reads, "History must not load far from the top")
list.ContentOffset = { X = 0, Y = -150 }
app.error = { code = "network", message = "Offline" }
Timeline.loadNearTop(app)
assert(app.reads == reads + 1 and app.timelinePage.error.code == "network")
Timeline.loadNearTop(app)
assert(app.reads == reads + 1, "A failed older request must wait for explicit retry")
app.error = nil
assert(Timeline.refresh(app, { preserveScroll = true, quiet = true }))
assert(app.timelinePage.error and app.timelinePage.error.code == "network", "A successful latest poll must preserve explicit history retry state")
app.error = { code = "stale_cursor", message = "History changed" }
assert(not Timeline.refresh(app, { older = true, preserveScroll = true, quiet = true }))
app.error, app.hasMore = nil, false
named(list, "EarlierMessages"):post("Button.Click")
core.flushQueue()
assert(app.lastRequest.before == nil and not app.timelinePage.error, "A deleted cursor must explicitly reload latest rather than repeat the broken cursor")

app, list = fixture()
app.hasMore, app.nextCursor = true, "1"
assert(Timeline.refresh(app))
app.readHook = function() coroutine.yield() end
pending = coroutine.create(function() Timeline.refresh(app, { older = true, preserveScroll = true, quiet = true }) end)
assert(coroutine.resume(pending))
assert(named(list, "EarlierMessages").IgnoreHitTest, "History controls disable while an older page is in flight")
reads = app.reads
Timeline.loadNearTop(app)
assert(app.reads == reads and app.timelinePage.loading)
local oldPage = app.timelinePage
app.navigationEpoch = app.navigationEpoch + 1
assert(coroutine.resume(pending))
assert(app.timelinePage == oldPage and #app.timelinePage.messages == 6 and not app.messageRefreshBusy,
    "Navigation must discard an older response and release the request lock")
app.readHook = function() error("history transport exploded") end
assert(not Timeline.refresh(app, { older = true, preserveScroll = true, quiet = true }))
assert(not app.messageRefreshBusy and not app.timelinePage.loading and app.timelinePage.error,
    "Unexpected history errors must leave a retryable state")
print("PASS: Chat timeline grouping, photo timestamps, footer layout, polling snapshots, scrolling, stale results and serialization")
