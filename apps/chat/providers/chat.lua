-- The Chat provider: operations, computed fields, filters and media behind
-- the names Data/*.xml declares. It reuses the client-bound models and the
-- service API; views never call them directly.
--
-- context: client, models, media (preview files), avatarPaths (shared with
-- Lua-built avatars), read/pinned (local room sets), openRoom(raw),
-- sessionLost(), show(route), navigation() (changes on every navigation);
-- for the open conversation: room(), contact(id),
-- attachment(id), photoDraft(roomId), photoSent(roomId, photo), notice(text,
-- isError), focusComposer(), openPhoto(raw), openMessageActions(record),
-- attachPhoto(scope), removePhoto(scope).
local Provider = {}

local function initials(name)
    name = name or ""
    local first = name:match("^([%w])") or "C"
    local last = name:match("%s+([%w])[%w]*$")
    return (first .. (last or "")):upper()
end

-- The same tile colour as Lua-built avatars (views/components.lua).
local function tone(key)
    local hash = 0
    key = key or ""
    for i = 1, #key do hash = hash + key:byte(i) * i end
    return "tone" .. (hash % 4 + 1)
end

local function clock(iso)
    return type(iso) == "string" and iso:match("T(%d%d:%d%d)") or ""
end

local function day(iso)
    local y, m, d = (iso or ""):match("^(%d%d%d%d)%-(%d%d)%-(%d%d)")
    if not y then return "CONVERSATION" end
    return os.date("%a, %d %b", os.time { year = tonumber(y), month = tonumber(m), day = tonumber(d), hour = 12 })
end

-- Photos keep their shape within a band that never dominates the thread.
local PHOTO_WIDTH, AUTO = 260, 0 / 0
local function photoHeight(values)
    local w, h = values.ThumbnailWidth or values.Width, values.ThumbnailHeight or values.Height
    local height = (w and h and w > 0) and PHOTO_WIDTH * h / w or PHOTO_WIDTH * 3 / 4
    return math.floor(math.max(140, math.min(340, height)))
end

-- What the history control above the messages says.
function Provider.historyText(query)
    local more = query.moreError
    if query.mode == "more" then return "Loading earlier messages…" end
    if query.gap then return "New messages · show latest" end
    if more and more.code == "stale_cursor" then return "History changed · reload latest" end
    if more then return "Could not load earlier messages · retry" end
    return "Earlier messages"
end

function Provider.new(context)
    local client, models = context.client, context.models
    local computed = {
        read = function(values) return context.read[values.Id] == true end,
        pinned = function(values) return context.pinned[values.Id] == true end,
        unreadCount = function(values) return values.IsRead and 0 or (values.Unread or 0) end,
        hasUnread = function(values) return (values.UnreadCount or 0) > 0 end,
        unreadText = function(values) return tostring(values.UnreadCount or 0) end,
        showPin = function(values) return values.IsPinned and not values.HasUnread end,
        roomState = function(values) return values.HasUnread and "unread" or "" end,
        roomTime = function(values) return clock(values.UpdatedAt or values.CreatedAt) end,
        preview = function(values) return values.Subtitle or "Start the conversation" end,
        initials = function(values) return initials(values.Title) end,
        tone = function(values) return tone(values.Id) end,
        roomAvatar = function(values) return values.Kind == "direct" and values.PeerId or "" end,
        contactAvatar = function(values) return values.Id end,
        hasAvatar = function(values) return (values.AvatarPath or "") ~= "" end,
        roomSearch = function(values) return ((values.Title or "") .. " " .. (values.Subtitle or "")):lower() end,
        publicName = function(values)
            return values.DisplayName ~= nil and values.DisplayName ~= "" and values.DisplayName or values.Username or "Member"
        end,
        handle = function(values) return (values.Username or "") ~= "" and "@" .. values.Username or "" end,
        -- Messages: a photo message is as wide as its photos and has no tail
        -- unless it has text; runs of one sender share decoration.
        hasPhotos = function(_, record) return #(record.children.Attachments or {}) > 0 end,
        hasBody = function(values) return (values.Body or "") ~= "" end,
        showCaption = function(values) return values.HasBody or not values.HasPhotos end,
        timestamp = function(values) return (values.EditedAt and "edited " or "") .. clock(values.CreatedAt) end,
        showSender = function(values) return values.Group and values.First and not values.Mine end,
        incomingTail = function(values) return values.First and not values.Mine and values.ShowCaption end,
        outgoingTail = function(values) return values.Last and values.Mine and values.ShowCaption end,
        alignment = function(values) return values.Mine and "End" or "Start" end,
        gap = function(values) return values.Leading and 0 or values.First and 8 or 2 end,
        leftInset = function(values) return values.Mine and 56 or 0 end,
        rightInset = function(values) return values.Mine and 0 or 56 end,
        bubbleWidth = function(values) return values.HasPhotos and PHOTO_WIDTH or AUTO end,
        inset = function(values) return values.HasPhotos and 0 or 10 end,
        verticalInset = function(values) return values.HasPhotos and 0 or 6 end,
        sectionGap = function(values) return values.HasPhotos and 0 or 2 end,
        photoInset = function(values) return values.HasPhotos and 10 or 0 end,
        photoVerticalInset = function(values) return values.HasPhotos and 6 or 0 end,
        bubbleTone = function(values) return values.Mine and "sent" or "received" end,
        canAct = function(values) return values.CanEdit or values.CanDelete end,
        actionsSize = function(values) return values.HasPhotos and 44 or AUTO end,
        actionsMargin = function(values) return values.HasPhotos and AUTO or 0 end,
        actionsText = function(values) return values.HasPhotos and "…" or "" end,
        actionsStyle = function(values) return values.HasPhotos and "photo-actions" or "" end,
        photoHeight = photoHeight,
        hasPhoto = function(values) return (values.PhotoPath or "") ~= "" end,
        photoPending = function(values) return (values.PhotoPath or "") == "" and values.PhotoState ~= "error" end,
        photoFailed = function(values) return values.PhotoState == "error" end,
        canPost = function(values) return not values.ReadOnly end,
        -- Conversation info.
        kindText = function(values) return (values.Kind or "group"):upper() end,
        roomAbout = function(values)
            return values.Kind == "channel" and "Only owners and admins can publish here."
                or "A space for everyone in this conversation."
        end,
        isOwner = function(values) return values.Role == "owner" end,
        canRename = function(values) return values.Kind ~= "direct" and values.Role == "owner" end,
        canInvite = function(values) return values.Kind ~= "direct" and (values.Role == "owner" or values.Role == "admin") end,
        canLeave = function(values) return values.Kind ~= "direct" and values.Role ~= "owner" end,
        leaveText = function(values) return values.Kind == "channel" and "Leave channel" or "Leave group" end,
        pinText = function(values) return values.IsPinned and "unpin" or "pin" end,
        hasInvite = function(values) return (values.InviteCode or "") ~= "" end,
        inviteText = function(values)
            if (values.InviteCode or "") ~= "" then
                return "Invitation: " .. values.InviteCode .. "\nShare this code with someone you trust."
            end
            return values.InviteRevoked and "Invitation revoked." or "Invitations expire and can be revoked."
        end,
        memberTone = function(values) return tone(values.UserId) end,
        memberAvatar = function(values) return values.UserId end,
        roleAction = function(values) return values.Role == "admin" and "Make member" or "Make admin" end,
    }
    local function signedIn()
        if client:getSession() then return true end
        if context.sessionLost then context.sessionLost() end
        return false
    end
    local operations = {
        -- Rooms first; contacts follow (Loaded) only when rooms exist.
        rooms = function()
            local rooms, err = models.Rooms:select()
            if not signedIn() then return nil, { code = "unauthenticated", message = "Signed out." } end
            if not rooms then return nil, err end
            return rooms
        end,
        contacts = function()
            local contacts, err = models.Contacts:select()
            if not signedIn() then return nil, { code = "unauthenticated", message = "Signed out." } end
            return contacts, err
        end,
        -- The portrait wall: people from the latest direct conversations,
        -- then everyone else by name. Derived from loaded records.
        recentContacts = function(_, _, _, query)
            local people, byId, direct = query.scope:member("People"):records(), {}, {}
            for _, record in ipairs(people) do byId[record.key] = record end
            for _, room in ipairs(query.scope:member("Rooms"):records()) do
                if room.values.Kind == "direct" then direct[#direct + 1] = room.values end
            end
            table.sort(direct, function(a, b) return (a.UpdatedAt or a.CreatedAt or "") > (b.UpdatedAt or b.CreatedAt or "") end)
            local ordered, seen = {}, {}
            local function add(record)
                if record and not seen[record.key] and #ordered < 32 then ordered[#ordered + 1], seen[record.key] = record, true end
            end
            for _, room in ipairs(direct) do add(byId[room.PeerId]) end
            for _, record in ipairs(people) do add(record) end
            return ordered
        end,
        -- A reply that arrives after the reader moved elsewhere does not
        -- navigate over their newer choice.
        openContact = function(params)
            local person, navigation = params.item, context.navigation()
            local room, err = models.Rooms:create { name = person.values.Title, member_ids = { person.key },
                kind = "direct", visibility = "private" }
            if not signedIn() then return nil, { code = "unauthenticated", message = "Signed out." } end
            if not room then return nil, err or { message = "Could not open this conversation. Try again." } end
            if context.navigation() == navigation then context.openRoom(room) end
            return room
        end,
        openRoom = function(params)
            context.read[params.item.key] = true
            params.item.store:refresh(params.item)
            context.openRoom(params.item.raw)
            return true
        end,
        -- Conversation info ------------------------------------------------
        members = function(params, _, _, query)
            local rows, err = client:listMembers(params.RoomId)
            if not signedIn() then return nil, { code = "unauthenticated", message = "Signed out." } end
            if not rows then return nil, err end
            local room = query.scope:exposedRecord("Room")
            local me = client:getSession().user.id
            local owner = room and room.values.Role == "owner"
            local manage = room and (owner or room.values.Role == "admin") and room.values.Kind ~= "direct"
            local list, store = {}, query.scope.store
            for _, member in ipairs(rows) do
                local id = member.user_id or member.id
                local info = member.profile or member
                local mayChange = manage and id ~= me and member.role ~= "owner"
                list[#list + 1] = store:put("ChatMember", { key = params.RoomId .. ":" .. id, user_id = id,
                    display_name = info.display_name, username = info.username, role = member.role or "member" },
                    { locals = { CanChangeRole = mayChange and owner or false, CanRemove = mayChange or false } })
            end
            return list
        end,
        renameRoom = function(params)
            local room = params.target
            local updated, err = client:renameRoom(room.key, params.draft.Title)
            if not updated then context.notice(err.message, true); return nil, err end
            room.store:put("ChatRoom", { id = room.key, name = updated.name }, { partial = true })
            if context.roomRenamed then context.roomRenamed(room.key, updated.name) end
            return true
        end,
        setRole = function(params, _, command)
            local member, room = params.item, command.scope:exposedRecord("Room")
            local ordinary = room.values.Kind == "channel" and "subscriber" or "member"
            local ok, err = client:setMemberRole(room.key, member.values.UserId,
                member.values.Role == "admin" and ordinary or "admin")
            if not ok then context.notice(err.message, true); return nil, err end
            return true
        end,
        removeMember = function(params, _, command)
            local ok, err = client:removeMember(command.scope:exposedRecord("Room").key, params.item.values.UserId)
            if not ok then context.notice(err.message, true); return nil, err end
            return true
        end,
        createInvite = function(_, _, command)
            local room = command.scope:exposedRecord("Room")
            if room.values.HasInvite then
                context.notice("Revoke the current invitation before creating a new one.")
                return true
            end
            local result, err = client:createInvite(room.key)
            if not result then context.notice(err.message, true); return nil, err end
            room.store:update(room, { InviteCode = tostring(result.code or result.token), InviteRevoked = false })
            return true
        end,
        revokeInvite = function(_, _, command)
            local room = command.scope:exposedRecord("Room")
            if not room.values.HasInvite then context.notice("Create an invitation first."); return true end
            local ok, err = client:revokeInvite(room.key, room.values.InviteCode)
            if not ok then context.notice(err.message, true); return nil, err end
            room.store:update(room, { InviteCode = "", InviteRevoked = true })
            return true
        end,
        leaveRoom = function(_, _, command)
            local room = command.scope:exposedRecord("Room")
            local ok, err = client:leaveRoom(room.key)
            if not ok then context.notice(err.message, true); return nil, err end
            context.roomLeft(room.key)
            return true
        end,
        togglePin = function(_, _, command)
            local room = command.scope:exposedRecord("Room")
            context.pinned[room.key] = not context.pinned[room.key] or nil
            room.store:refresh(room)
            return true
        end,
        -- The newest page first; `page.cursor` reads the page before it.
        messages = function(params, page)
            local result, err = models.Messages:page { room_id = params.RoomId, limit = page.size,
                before = page.cursor }
            if not signedIn() then return nil, { code = "unauthenticated", message = "Signed out." } end
            if not result then return nil, err end
            return { items = result.messages, cursor = result.next_cursor, hasMore = result.has_more }
        end,
        -- The composer: a new message (after uploading its photo) or an edit
        -- of a sent one (params.target).
        send = function(params, _, command)
            local draft, target, room = params.draft, params.target, context.room()
            local text = draft.Text or ""
            if target then
                if text == (target.values.Body or "") then return true end
                local row, err = client:editMessage(target.key, text)
                if not row then context.notice(err.message, true); return nil, err end
                command.scope.store:put("ChatMessage", row, { partial = true })
                return true
            end
            if room.can_post == false then
                return nil, { code = "forbidden", message = "Only channel admins can publish." }
            end
            local photo, attachments = context.photoDraft(room.id), nil
            if photo then
                context.notice("Uploading photo…")
                if not photo.attachment then
                    local attachment, err = client:uploadPhoto(room.id, photo)
                    if not attachment then
                        local message = err.message .. " · Your draft is safe. Try sending again."
                        context.notice(message, true)
                        return nil, { code = err.code, message = message }
                    end
                    photo.attachment = attachment
                end
                attachments = { photo.attachment.id }
            end
            local message, err = models.Messages:create { room_id = room.id, body = text, attachment_ids = attachments }
            if not message then context.notice(err.message, true); return nil, err end
            context.photoSent(room.id, photo)
            return message
        end,
        deleteMessage = function(params, _, command)
            local message = params.item
            local ok, err = client:deleteMessage(message.key)
            if not ok then context.notice(err.message, true); return nil, err end
            -- Its neighbours may now start or end a run.
            local messages, remaining = command.scope:member("Messages"), {}
            for _, record in ipairs(messages.items:records()) do
                if record ~= message then remaining[#remaining + 1] = record end
            end
            command.scope.provider.layoutThread(messages, remaining)
            message.store:remove("ChatMessage", message.key)
            context.notice("Message deleted.")
            return true
        end,
    }
    -- Sender runs, permissions and photo timestamps over the whole thread:
    -- they depend on neighbours, so a page of history changes them too.
    local function layoutThread(query, records)
        local session = client:getSession()
        local room = context.room()
        if not session or not room then return end
        local me, store = session.user.id, query.scope.store
        records = records or query.items:records()
        local staff = room.role == "owner" or room.role == "admin"
        store:batch(function()
            for index, record in ipairs(records) do
                local values = record.values
                local previous, following = records[index - 1], records[index + 1]
                local mine = values.SenderId == me and room.kind ~= "channel"
                local person = context.contact(values.SenderId)
                local sender = room.kind == "channel" and room.name
                    or person and person.Title or "Member " .. tostring(values.SenderId or ""):sub(1, 8)
                local own = values.SenderId == me
                store:update(record, { Mine = mine, Leading = index == 1, Group = room.kind == "group", Sender = sender,
                    First = not previous or previous.values.SenderId ~= values.SenderId,
                    Last = not following or following.values.SenderId ~= values.SenderId,
                    CanEdit = own and room.can_post ~= false, CanDelete = own or (room.kind ~= "direct" and staff) })
                local photos = record.children.Attachments or {}
                for position, photo in ipairs(photos) do
                    store:update(photo, { ShowTime = not values.HasBody and position == #photos,
                        Timestamp = values.Timestamp })
                end
            end
        end)
    end

    local hooks = {
        arrangeThread = function(query, records) layoutThread(query, records) end,
        -- A saved name starts a new edit of the room, showing what was stored.
        renamed = function(_, _, _, info, edit)
            if info.target and not edit.disposed then edit:begin(info.target) end
        end,
        messagesLoaded = function(query, _, mode)
            local thread = query.scope:exposedRecord("Thread")
            local first = query.items:records()[1]
            if thread then
                query.scope.store:put("ChatThread", { id = thread.key, date = first and day(first.values.CreatedAt) or "" },
                    { partial = true })
            end
            -- Photos that failed before get another try with the new rows.
            local photos = query.scope:member("Photos")
            if photos then photos:retryFailed() end
            if mode ~= "more" and not query.quiet then context.notice("") end
        end,
        -- After a send the list follows the new message and the composer keeps focus.
        sent = function(_, params, command)
            if params.target then return end
            local messages = command.scope:member("Messages")
            messages:refresh()
            context.focusComposer()
        end,
        roomsLoaded = function(query, rows)
            local contacts = query.scope:member("Contacts")
            if #rows == 0 then contacts:assign {}; query.scope:member("Recent"):assign {}
            else contacts:refresh { quiet = contacts.loaded } end
        end,
        -- The portrait wall follows contacts once the Contacts page asked for it.
        contactsLoaded = function(query)
            local recent = query.scope:member("Recent")
            if recent.loaded or recent.mode then recent:refresh { quiet = recent.loaded } end
        end,
    }
    local filters = {
        search = function(record, params)
            local query = (params.Search or ""):lower()
            return query == "" or record.values.SearchText:find(query, 1, true) ~= nil
        end,
        unread = function(record) return record.values.HasUnread end,
        pinned = function(record) return record.values.IsPinned end,
        person = function(record)
            local session = client:getSession()
            return not session or record.key ~= session.user.id
        end,
        contactSearch = function(record, params)
            local query = (params.Search or ""):lower()
            return query == "" or ((record.values.Title or "") .. " " .. (record.values.Username or "")):lower():find(query, 1, true) ~= nil
        end,
    }
    local sorts = {
        byName = function(a, b) return (a.values.Title or ""):lower() < (b.values.Title or ""):lower() end,
    }
    local media = {
        avatarLookup = function(ids) return client:getAvatars(ids) end,
        avatarFetch = function(id) return client:getAvatar(id) end,
        -- Files are keyed by photo ID; Lua-built avatars share paths by user ID.
        avatarCached = function(file, user)
            local path = context.avatarPaths[user] or (context.media and context.media.keys and context.media.keys["avatar-" .. file])
            if path then context.avatarPaths[user] = path end
            return path
        end,
        avatarMissing = function(user) context.avatarPaths[user] = false end,
        avatarStore = function(bytes, file, _, user)
            local path, err = context.media:storePreview(bytes, "avatar-" .. file)
            if path then context.avatarPaths[user] = path end
            return path, err
        end,
    }
    local conditions = {
        -- Polling waits while the composer is sending.
        idle = function(query)
            local composer = query.scope:member("Composer")
            return not (composer and composer.submitting)
        end,
    }
    local validators = {
        message = function(values)
            if not (values.Text or ""):match("%S") and not values.HasPhoto then return { Text = "" } end
        end,
    }
    media.photoFetch = function(_, record, stage) return client:getPhoto(record.raw, stage == "preview") end
    media.photoStore = function(bytes, file, stage, id)
        local record = context.attachment and context.attachment(id)
        local revision = record and tostring(record.values.UpdatedAt or "") or ""
        return context.media:storePreview(bytes, (stage == "preview" and "message-photo-preview-" or "message-photo-")
            .. file .. ":" .. revision)
    end
    media.photoCached = function(file, id)
        local record = context.attachment and context.attachment(id)
        local revision = record and tostring(record.values.UpdatedAt or "") or ""
        return context.media.keys and context.media.keys["message-photo-" .. file .. ":" .. revision]
    end
    media.photoValidate = function(path) return require("orca.data.media").decodes(path) end
    local handlers = {
        -- The control above the messages: older history, or back to the newest page.
        RefreshContacts = function(scope) scope:member("Contacts"):refresh() end,
        FindSomeone = function() context.show("/new") end,
        LoadHistory = function(scope)
            local messages = scope:member("Messages")
            local more = messages.moreError
            if messages.gap or (more and more.code == "stale_cursor") then messages:reset() else messages:loadMore() end
        end,
        MessageActions = function(_, params)
            if params.item then context.openMessageActions(params.item) end
        end,
        OpenPhoto = function(_, params)
            if params.item then context.openPhoto(params.item.raw) end
        end,
        AttachPhoto = function(scope) context.attachPhoto(scope) end,
        RemovePhoto = function(scope) context.removePhoto(scope) end,
        CancelEdit = function(scope) scope:member("Composer"):cancel() end,
        AskLeave = function(scope)
            local room = scope:exposedRecord("Room")
            room.store:update(room, { ConfirmLeave = true })
        end,
        CancelLeave = function(scope)
            local room = scope:exposedRecord("Room")
            room.store:update(room, { ConfirmLeave = false })
        end,
        -- Clearing the search returns focus to the field, as typing continues there.
        ClearSearch = function(scope, params)
            scope:member("Matching"):set("Search", "")
            local field = params.sender and params.sender:getParent():findChild("Search")
            if field then field:setFocus() end
        end,
    }
    return { computed = computed, operations = operations, hooks = hooks, filters = filters, sorts = sorts, media = media,
        handlers = handlers, conditions = conditions, validators = validators, layoutThread = layoutThread }
end

return Provider
