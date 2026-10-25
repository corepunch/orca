local ui = require "orca.UIKit"
local C = require "Chat/views/components"
local colors, auto = C.colors, 0/0
local Timeline = {}

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

local function snapshot(self, room, messages)
    local rows = { kind = room.kind, name = room.name, can_post = room.can_post }
    for i, message in ipairs(messages) do
        local attachments = {}
        for j, item in ipairs(message.attachments or {}) do
            attachments[j] = { id = item.id, updated_at = item.updated_at }
        end
        rows[i] = { id = message.id, body = message.body, sender_id = message.sender_id,
            created_at = message.created_at, updated_at = message.updated_at,
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
    -- Build detached rows so a yielding photo request never exposes a partial list.
    local children, entries, photos, retryPhotos = {}, {}, {}, false
    children[#children + 1] = C.text(#messages > 0 and C.date(messages[1].created_at) or "A FRESH START", {
        FontSize = 12, Foreground = colors.dim, TextHorizontalAlignment = "Center", MarginTop = 4, MarginBottom = 6,
    })
    for _, message in ipairs(messages) do
        local mine = message.sender_id == me and room.kind ~= "channel"
        local bubble = C.stack({
            Name = "MessageBubble" .. message.id, Width = 272, Padding = "12 14", Spacing = 6,
            Background = mine and colors.sent or colors.raised,
            MarginLeft = mine and auto or 0, MarginRight = mine and 0 or auto,
        })
        if not mine then
            bubble:addChild(C.text(sender(self, room, message), { FontSize = 12, FontWeight = "Bold", Foreground = colors.accentText }))
        end
        for _, attachment in ipairs(message.attachments or {}) do
            local cached = previous and previous.node == node and previous.photos[attachment.id]
            local path = cached and cached.updated_at == attachment.updated_at and cached.path
            local photoError
            if not path then
                local bytes
                bytes, photoError = self.client:getPhoto(attachment.id, true)
                if not current() then return true end
                local media = self:getMedia()
                path = bytes and media and media:storePreview(bytes, attachment.id)
            end
            if path then
                photos[attachment.id] = { updated_at = attachment.updated_at, path = path }
                bubble:addChild(ui.ImageView { Name = "Photo" .. attachment.id, Src = path,
                    Width = 250, Height = 180, Stretch = "Uniform", IgnoreHitTest = true })
            else
                retryPhotos = true
                bubble:addChild(C.text(photoError and photoError.message or "Photo preview unavailable", {
                    FontSize = 12, Foreground = colors.muted, WordWrap = true }))
            end
        end
        if message.body and message.body ~= "" then
            bubble:addChild(C.text(message.body, { WordWrap = true, FontSize = 16, Foreground = mine and colors.sentText or colors.ink }))
        end
        bubble:addChild(C.text((mine and "Sent · " or "") .. (C.time(message.created_at) or ""), {
            FontSize = 11, Foreground = mine and colors.sentMeta or colors.dim, TextHorizontalAlignment = "Right",
        }))
        local entry, tone = C.stack({ Name = "MessageEntry" .. message.id, Spacing = 0 }), mine and colors.sent or colors.raised
        if not mine then entry:addChild(C.tail(true, tone, false)) end
        entry:addChild(bubble)
        if mine then entry:addChild(C.tail(false, tone, true)) end
        children[#children + 1], entries[#entries + 1] = entry, { id = message.id, node = entry }
    end
    if #messages == 0 then
        local channel, readOnly = room.kind == "channel", room.can_post == false
        children[#children + 1] = C.stack({ Spacing = 8, Padding = "0 8" }, {
            C.text(channel and (readOnly and "no posts yet" or "your channel starts here") or "every good chat starts with a hello", {
                Name = "EmptyTimeline", FontSize = 22, Foreground = colors.muted, WordWrap = true }),
            C.text(channel and (readOnly and "New posts will appear here when the channel publishes."
                or "Publish your first update below.") or "Send the first message below.", {
                Foreground = colors.dim, FontSize = 14, WordWrap = true }),
        })
    end
    if not current() then return true end
    -- Read scrolling now: the user may have moved while the request was in flight.
    local offset = node.ContentOffset
    local oldX, oldY = offset.X, offset.Y
    local bottom = math.min(0, node.ActualHeight - node.ScrollHeight)
    local follow = not options.preserveScroll or math.abs(oldY - bottom) <= 8
    local anchor, anchorY
    if not follow and previous and previous.node == node then
        for _, entry in ipairs(previous.entries) do
            if entry.node.ActualY + entry.node.ActualHeight + oldY > 0 then
                anchor, anchorY = entry.id, entry.node.ActualY
                break
            end
        end
    end
    while node:getFirstChild() do node:getFirstChild():removeFromParent() end
    for _, child in ipairs(children) do node:addChild(child) end
    self.timelineSnapshot = { node = node, signature = signature, entries = entries, photos = photos, retryPhotos = retryPhotos }
    local root = node:getRoot()
    root:UpdateLayout(root.Width, root.Height)
    bottom = math.min(0, node.ActualHeight - node.ScrollHeight)
    if anchor then
        for _, entry in ipairs(entries) do
            if entry.id == anchor then oldY = oldY + anchorY - entry.node.ActualY; break end
        end
    end
    node.ContentOffset = { X = oldX, Y = follow and bottom or math.max(bottom, math.min(0, oldY)) }
    if not options.quiet then self:notice(self.remote and "Synced · updates automatically" or "Offline mock · messages stay in this session") end
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
    local ok, result, err = pcall(refresh, self, options, room, node, current)
    self.messageRefreshBusy = false
    if not ok then result, err = nil, { code = "refresh_failed", message = "Could not refresh messages. Please try again." } end
    if not result and current() and not options.quiet then self:notice(err.message, true) end
    local queued = self.messageRefreshQueued
    self.messageRefreshQueued = nil
    if queued and queued.current() then return Timeline.refresh(self, queued.options) end
    return result, err
end

return Timeline
