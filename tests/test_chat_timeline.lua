-- Protocol/state tests use small node doubles; native layout remains covered by test_chat_app.
package.path = "apps/chat/?.lua;" .. package.path
local savedUI, savedComponents = package.loaded["orca.UIKit"], package.loaded["Chat/views/components"]
local function node(props, children)
    local n = props or {}
    n.children = {}
    function n:addChild(child) child.parent = self; self.children[#self.children + 1] = child end
    function n:getFirstChild() return self.children[1] end
    function n:removeFromParent()
        for i, child in ipairs(self.parent.children) do
            if child == self then table.remove(self.parent.children, i); break end
        end
        self.parent = nil
    end
    for _, child in ipairs(children or {}) do n:addChild(child) end
    return n
end
local C = { colors = setmetatable({}, { __index = function(_, key) return key end }) }
C.stack = node
C.tail = function(incoming) return node { Tail = true, Incoming = incoming } end
C.text = function(value, props) local n = node(props); n.Text = value; return n end
C.date, C.time = tostring, tostring
package.loaded["orca.UIKit"] = { ImageView = node, Grid = node }
package.loaded["Chat/views/components"] = C
local Timeline = require "views.timeline"

local function fixture()
    local messages = {}
    for i = 1, 6 do messages[i] = { id = tostring(i), body = "Message " .. i, sender_id = "me", created_at = "time" } end
    messages[1].attachments = { { id = "photo1" } }
    local list = node { ActualHeight = 200, ScrollHeight = 0, ContentOffset = { X = 0, Y = 0 } }
    local root = { Width = 390, Height = 844 }
    function root:UpdateLayout()
        for i, child in ipairs(list.children) do child.ActualY, child.ActualHeight = (i - 1) * 100, 100 end
        list.ScrollHeight = #list.children * 100
    end
    function list:getRoot() return root end
    local app = { room = { id = "room1" }, messageList = list, sessionEpoch = 1, navigationEpoch = 1,
        rows = messages, reads = 0, photos = 0, composer = { Text = "draft remains" }, photoDrafts = { room1 = "draft-photo" } }
    local session = { user = { id = "me" } }
    app.client = { getSession = function() return session end }
    function app.client:getPhoto()
        app.photos = app.photos + 1
        if app.photoHook then app.photoHook() end
        return "jpeg"
    end
    app.models = { Messages = {} }
    function app.models.Messages:select()
        app.reads = app.reads + 1
        if app.readHook then app.readHook() end
        return app.rows, app.error
    end
    app.messagePhotos = { pending = {} }
    function app.messagePhotos:watch(frame, attachment, done)
        self.pending[#self.pending + 1] = { frame = frame, attachment = attachment, done = done }
    end
    -- Protocol tests explicitly demand registered previews after committed layout.
    -- Native viewport/concurrency behavior is covered by test_chat_photo_loader.
    function app:loadPhotos()
        local pending = self.messagePhotos.pending
        self.messagePhotos.pending = {}
        for _, target in ipairs(pending) do
            local ancestor = target.frame
            while ancestor and ancestor ~= list do ancestor = ancestor.parent end
            if ancestor == list then
                local bytes, err = self.client:getPhoto(target.attachment.id, true)
                target.done(bytes and "preview.jpg", err)
            end
        end
    end
    function app:getMedia() return { storePreview = function() return "preview.jpg" end } end
    function app:notice(message) self.noticeText = message end
    function app:logout() session = nil end
    return app, list
end

local function named(parent, name)
    for _, child in ipairs(parent.children) do
        if child.Name == name then return child end
    end
end
local function count(parent, match)
    local total = match(parent) and 1 or 0
    for _, child in ipairs(parent.children) do total = total + count(child, match) end
    return total
end

-- Grouping changes decoration, not the independently addressable message rows.
for _, kind in ipairs { "group", "direct", "channel" } do
    local grouped, groupedList = fixture()
    grouped.room.kind, grouped.room.name = kind, "Newsroom"
    grouped.contactsById = { other = { get_display_name = function() return "Alex" end } }
    grouped.rows = {}
    for i, senderId in ipairs { "other", "other", "me", "me", "other" } do
        grouped.rows[i] = { id = tostring(i), body = "Body " .. i, sender_id = senderId, created_at = "time" }
    end
    assert(Timeline.refresh(grouped))
    local expectedTails = kind == "channel" and { 1, 0, 1, 0, 1 } or { 1, 0, 0, 1, 1 }
    for i = 1, 5 do
        local entry = assert(named(groupedList, "MessageEntry" .. i), "Every grouped message needs its own row")
        assert(named(entry, "MessageBubble" .. i), "Every grouped message needs its own bubble")
        assert(count(entry, function(child) return child.Tail end) == expectedTails[i],
            kind .. " must decorate only the appropriate edge of each sender run")
        local expectedNames = kind == "group" and (i == 1 or i == 5) and 1 or 0
        assert(count(entry, function(child) return child.Text == "Alex" end) == expectedNames,
            "Sender names belong only on the first incoming message in each group-chat run")
    end
    assert(count(groupedList, function(child) return child.Text == "Newsroom" end) == 0,
        "Channel messages must not repeat the channel title")
    assert(count(groupedList, function(child) return child.Tail and not child.Incoming end)
        == (kind == "channel" and 0 or 1), "Channel posts must never use outgoing decoration")
end

-- A notice can consume footer space; bottom alignment must use its final viewport.
local noticeApp, noticeList = fixture()
local noticeRoot = noticeList:getRoot()
local originalLayout = noticeRoot.UpdateLayout
function noticeRoot:UpdateLayout(...)
    noticeList.ActualHeight = noticeApp.noticeText and 160 or 200
    originalLayout(self, ...)
end
assert(Timeline.refresh(noticeApp))
assert(noticeApp.noticeText and noticeList.ActualHeight == 160 and noticeList.ContentOffset.Y == -540,
    "Refresh must publish the footer notice before layout and bottom alignment")

local app, list = fixture()
assert(Timeline.refresh(app))
assert(list.ContentOffset.Y == -500 and app.photos == 0, "Building rows must not fetch previews")
app:loadPhotos()
assert(app.photos == 1)
local first = list.children[1]
assert(Timeline.refresh(app, { preserveScroll = true, quiet = true }))
assert(list.children[1] == first and app.photos == 1, "Unchanged polls must retain rows and skip photo requests")
list.ContentOffset = { X = 0, Y = -175 }
app.rows[#app.rows + 1] = { id = "7", body = "New", sender_id = "other", created_at = "time" }
assert(Timeline.refresh(app, { preserveScroll = true }))
assert(list.ContentOffset.Y == -175, "Incoming messages must preserve reading position")
assert(app.photos == 1, "New messages must reuse successful photo previews already in this timeline")
assert(app.composer.Text == "draft remains" and app.photoDrafts.room1 == "draft-photo")
list.ContentOffset = { X = 0, Y = -600 }
app.rows[#app.rows + 1] = { id = "8", body = "Another", sender_id = "other", created_at = "time" }
assert(Timeline.refresh(app, { preserveScroll = true }))
assert(list.ContentOffset.Y == -700, "A reader at the bottom must follow new messages")

-- Deferred photo work cannot overwrite scrolling after rows are committed.
app.rows[1].body = "Edited"
app.rows[1].attachments[1].updated_at = "changed"
app.photoHook = function() list.ContentOffset = { X = 0, Y = -200 } end
assert(Timeline.refresh(app, { preserveScroll = true }))
app:loadPhotos()
assert(list.ContentOffset.Y == -200)
app.photoHook = nil

-- A moving 100-row result window must retain the first still-visible message anchor.
list.ContentOffset = { X = 0, Y = -250 }
table.remove(app.rows, 1)
assert(Timeline.refresh(app, { preserveScroll = true }))
assert(list.ContentOffset.Y == -150, "Removing an earlier row must preserve the visible message anchor")

local oldFirst = list.children[1]
app.rows[1].body = "Stale"
app.readHook = function() app.navigationEpoch = app.navigationEpoch + 1 end
assert(Timeline.refresh(app, { preserveScroll = true }))
assert(list.children[1] == oldFirst, "Navigation during a fetch must discard its result")
app.readHook = nil

-- Row construction commits atomically before yielding preview downloads start.
app, list = fixture()
assert(Timeline.refresh(app))
oldFirst = list.children[1]
app.rows[1].body = "Pending photo"
app.rows[1].attachments[1].updated_at = "changed"
app.photoHook = function() assert(list.children[1] ~= oldFirst); app:logout() end
assert(Timeline.refresh(app, { preserveScroll = true }))
local committed = list.children[1]
assert(committed ~= oldFirst and #list.children == 7, "Rows commit without waiting for photo downloads")
app:loadPhotos()
assert(list.children[1] == committed and not app.timelineSnapshot.photos.photo1,
    "Logout during deferred preview work must not apply stale photo results")

app, list = fixture()
local photoCalls = 0
app.client.getPhoto = function()
    photoCalls = photoCalls + 1
    if photoCalls == 1 then return nil, { message = "Temporarily unavailable" } end
    return "jpeg"
end
assert(Timeline.refresh(app))
app:loadPhotos()
assert(app.timelineSnapshot.retryPhotos)
assert(Timeline.refresh(app, { preserveScroll = true }))
app:loadPhotos()
assert(photoCalls == 2 and not app.timelineSnapshot.retryPhotos,
    "A failed photo preview must retry even if the message snapshot is unchanged")
assert(Timeline.refresh(app, { preserveScroll = true }) and photoCalls == 2)

-- A manual refresh during a yielding poll is queued, never overlapped or dropped.
app, list = fixture()
local calls = 0
app.readHook = function() calls = calls + 1; if calls == 1 then coroutine.yield() end end
local pending = coroutine.create(function() assert(Timeline.refresh(app, { preserveScroll = true })) end)
assert(coroutine.resume(pending))
assert(app.messageRefreshBusy and app.reads == 1)
assert(Timeline.refresh(app))
assert(app.reads == 1, "Manual and timer reads must not overlap")
assert(coroutine.resume(pending))
assert(not app.messageRefreshBusy and app.reads == 2, "Queued manual refresh must run after the poll completes")

app.readHook = function() error("unexpected transport failure") end
local ok, err = Timeline.refresh(app, { preserveScroll = true, quiet = true })
assert(not ok and err.code == "refresh_failed" and not app.messageRefreshBusy, "Exceptions must release the read lock")
package.loaded["orca.UIKit"], package.loaded["Chat/views/components"] = savedUI, savedComponents
package.loaded["views.timeline"] = nil
print("PASS: Chat timeline grouping, footer layout, polling snapshots, scrolling, stale results and serialization")
