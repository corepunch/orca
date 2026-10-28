local C = require "Chat/views/components"
local colors = C.colors
local Timeline = {}
local PHOTO_WIDTH = 260
local MessageItems = require "Chat/views/message_items"

-- Keep the photo's aspect ratio within a band that never dominates the thread.
local function photoHeight(attachment)
    local w = attachment.thumbnail_width or attachment.width
    local h = attachment.thumbnail_height or attachment.height
    local height = (w and h and w > 0) and PHOTO_WIDTH * h / w or PHOTO_WIDTH * 3 / 4
    return math.floor(math.max(140, math.min(340, height)))
end

local function equal(a, b)
    if type(a) ~= type(b) then return false end
    if type(a) ~= "table" then return a == b end
    for key, value in pairs(a) do if not equal(value, b[key]) then return false end end
    for key in pairs(b) do if a[key] == nil then return false end end
    return true
end

local function sender(self, room, message)
    local person = self.contactsById and self.contactsById[message.sender_id]
    return room.kind == "channel" and room.name
        or (person and person:get_display_name() or "Member " .. tostring(message.sender_id or ""):sub(1, 8))
end

-- What the reader may do with a message; the server enforces the same rules.
local function permissions(room, message, me)
    local mine = message.sender_id == me
    local staff = room.role == "owner" or room.role == "admin"
    return mine and room.can_post ~= false, mine or (room.kind ~= "direct" and staff)
end

local function snapshot(self, room, messages)
    local rows = { kind = room.kind, name = room.name, can_post = room.can_post, role = room.role }
    for i, message in ipairs(messages) do
        local attachments = {}
        for j, item in ipairs(message.attachments or {}) do
            attachments[j] = { id = item.id, updated_at = item.updated_at }
        end
        rows[i] = { id = message.id, body = message.body, sender_id = message.sender_id,
            created_at = message.created_at, updated_at = message.updated_at, edited_at = message.edited_at,
            sender = sender(self, room, message), attachments = attachments }
    end
    return rows
end

local function refresh(self, options, room, node, current)
    local messages, err = self.models.Messages:select { room_id = room.id, limit = 100 }
    if not current() then return true end
    if not messages then return nil, err end
    local session = self.client:getSession()
    if not session then return true end
    local me = session.user.id
    local previous = self.timelineSnapshot
    local signature = snapshot(self, room, messages)
    if previous and previous.node == node and not previous.retryPhotos and equal(previous.signature, signature) then
        if not options.preserveScroll then
            node.ContentOffset = { X = 0, Y = math.min(0, node.ActualHeight - node.ScrollHeight) }
        end
        return true
    end
    local rows, photos = {}, {}
    local previousPhotos = previous and previous.node == node and previous.photos or {}
    local group = room.kind == "group"
    for i, message in ipairs(messages) do
        local mine = message.sender_id == me and room.kind ~= "channel"
        local prev, nextMessage = messages[i - 1], messages[i + 1]
        local first = not prev or prev.sender_id ~= message.sender_id
        local last = not nextMessage or nextMessage.sender_id ~= message.sender_id
        local attachments = message.attachments or {}
        local photo = #attachments > 0
        local photoOnly = photo and (not message.body or message.body == "")
        local time = C.time(message.created_at) or ""
        local timestamp = message.edited_at and "edited " .. time or time
        local canEdit, canDelete = permissions(room, message, me)
        local row = { key = message.id, message = message, canEdit = canEdit, canDelete = canDelete, photos = {}, fields = {
            EntryName = "MessageEntry" .. message.id, HolderName = "MessageHolder" .. message.id,
            BubbleName = "MessageBubble" .. message.id, ActionsName = (canEdit or canDelete) and "MessageActions" .. message.id or "",
            TimeName = not photoOnly and "MessageTime" .. message.id or "",
            ShowDate = i == 1, Date = C.date(messages[1].created_at), Empty = false, HasMessage = true,
            EmptyTitle = "", EmptyHint = "", Sender = sender(self, room, message), Body = message.body or "", Timestamp = timestamp,
            ShowSender = group and first and not mine, HasPhotos = photo, ShowCaption = not photoOnly,
            HasBody = message.body ~= nil and message.body ~= "", CanAct = canEdit or canDelete,
            IncomingTail = first and not mine and not photoOnly, OutgoingTail = last and mine and not photoOnly,
            Alignment = mine and "End" or "Start", Gap = i == 1 and 0 or first and 8 or 2,
            LeftInset = mine and 56 or 0, RightInset = mine and 0 or 56,
            BubbleWidth = photo and PHOTO_WIDTH or 0/0, Inset = photo and 0 or 10,
            VerticalInset = photo and 0 or 6, SectionGap = photo and 0 or 2,
            PhotoInset = photo and 10 or 0, PhotoVerticalInset = photo and 6 or 0,
            Tone = mine and colors.sent or colors.raised, Ink = mine and colors.sentText or colors.ink,
            Meta = mine and colors.sentMeta or colors.dim, Dim = colors.dim, Muted = colors.muted, Accent = colors.accentText,
        } }
        for index, attachment in ipairs(attachments) do
            local cached = previousPhotos[attachment.id]
            if cached and cached.updated_at == attachment.updated_at then photos[attachment.id] = cached end
            row.photos[index] = { attachment = attachment,
                path = cached and cached.updated_at == attachment.updated_at and cached.path,
                fields = { FrameName = "PhotoFrame" .. attachment.id, ContentName = "PhotoContent" .. attachment.id,
                    Height = photoHeight(attachment), Soft = colors.soft, Muted = colors.muted,
                    ShowTime = photoOnly and index == #attachments,
                    TimeName = photoOnly and index == #attachments and "MessageTime" .. message.id or "", Timestamp = timestamp } }
        end
        rows[i] = row
    end
    if #messages == 0 then
        local channel, readOnly = room.kind == "channel", room.can_post == false
        rows[1] = { key = "__empty", photos = {}, fields = {
            EntryName = "EmptyTimelineEntry", HolderName = "", BubbleName = "", ActionsName = "", TimeName = "",
            ShowDate = true, Date = "A FRESH START", Empty = true, HasMessage = false,
            EmptyTitle = channel and (readOnly and "no posts yet" or "your channel starts here") or "every good chat starts with a hello",
            EmptyHint = channel and (readOnly and "New posts will appear here when the channel publishes."
                or "Publish your first update below.") or "Send the first message below.",
            Sender = "", Body = "", Timestamp = "", Alignment = "Start", Tone = colors.raised, Ink = colors.ink,
            Meta = colors.dim, Dim = colors.dim, Muted = colors.muted, Accent = colors.accentText,
            ShowSender = false, HasPhotos = false, ShowCaption = false, HasBody = false, CanAct = false,
            IncomingTail = false, OutgoingTail = false, Gap = 0, LeftInset = 0, RightInset = 0, BubbleWidth = 0/0,
            Inset = 0, VerticalInset = 0, PhotoInset = 0, PhotoVerticalInset = 0, SectionGap = 0,
        } }
    end
    if not current() then return true end
    -- Read scrolling now: the user may have moved while the request was in flight.
    local offset = node.ContentOffset
    local oldX, oldY = offset.X, offset.Y
    local bottom = math.min(0, node.ActualHeight - node.ScrollHeight)
    local follow = not options.preserveScroll or math.abs(oldY - bottom) <= 8
    local anchor, anchorY
    if not follow and previous and previous.node == node then
        -- Messages scroll under the header: the first visible one is below its inset.
        local visibleTop = node.AdjustedContentInsetTop
        for _, entry in ipairs(previous.entries) do
            if entry.node.ActualY + entry.node.ActualHeight + oldY > visibleTop then
                anchor, anchorY = entry.id, entry.node.ActualY
                break
            end
        end
    end
    local items = previous and previous.node == node and previous.items or MessageItems.new(self, node)
    local bound = items:refresh(rows)
    local entries = {}
    for i, entry in ipairs(bound) do entries[i] = { id = rows[i].key, node = entry.node } end
    self.timelineSnapshot = { node = node, signature = signature, entries = entries, items = items,
        photos = photos, failedPhotos = {}, retryPhotos = false }
    -- Clear request feedback before measuring the composer's final viewport.
    if not options.quiet then self:notice("") end
    local root = node:getRoot()
    root:UpdateLayout(root.Width, root.Height)
    bottom = math.min(0, node.ActualHeight - node.ScrollHeight)
    if anchor then
        for _, entry in ipairs(entries) do
            if entry.id == anchor then oldY = oldY + anchorY - entry.node.ActualY; break end
        end
    end
    node.ContentOffset = { X = oldX, Y = follow and bottom or math.max(bottom, math.min(0, oldY)) }
    return true
end

function Timeline.refresh(self, options)
    options = options or {}
    local room, node = self.room, self.messageList
    local epoch, navigation = self.sessionEpoch, self.navigationEpoch
    local session = self.client and self.client:getSession()
    if not room or not node or not session then return true end
    local user = session.user.id
    local function current()
        local active = self.client:getSession()
        return self.room == room and self.messageList == node and self.sessionEpoch == epoch
            and self.navigationEpoch == navigation and active and active.user.id == user
            and (not options.current or options.current())
    end
    if self.messageRefreshBusy then
        if not options.preserveScroll then self.messageRefreshQueued = { current = current, options = options } end
        return true
    end
    self.messageRefreshBusy = true
    local ok, result, err = xpcall(refresh, debug.traceback, self, options, room, node, current)
    self.messageRefreshBusy = false
    if not ok then
        if current() and self.reportError then self:reportError(result, "Refreshing messages") end
        result, err = nil, { code = "refresh_failed", message = "Could not refresh messages. Please try again." }
    end
    if not result and current() and not options.quiet then self:notice(err.message, true) end
    local queued = self.messageRefreshQueued
    self.messageRefreshQueued = nil
    if queued and queued.current() then return Timeline.refresh(self, queued.options) end
    return result, err
end

return Timeline
