-- The Chat provider: operations, computed fields, filters and media behind
-- the names Data/*.xml declares. It reuses the client-bound models and the
-- service API; views never call them directly.
--
-- context: client, models, media (preview files), avatarPaths (shared with
-- Lua-built avatars), read/pinned (local room sets), openRoom(raw),
-- sessionLost(), and colors/time helpers are not needed here.
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
        openRoom = function(params)
            context.read[params.item.key] = true
            params.item.store:refresh(params.item)
            context.openRoom(params.item.raw)
            return true
        end,
    }
    local hooks = {
        roomsLoaded = function(query, rows)
            local contacts = query.scope:member("Contacts")
            if #rows == 0 then contacts:assign {} else contacts:refresh { quiet = contacts.loaded } end
        end,
    }
    local filters = {
        search = function(record, params)
            local query = (params.Search or ""):lower()
            return query == "" or record.values.SearchText:find(query, 1, true) ~= nil
        end,
        unread = function(record) return record.values.HasUnread end,
        pinned = function(record) return record.values.IsPinned end,
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
    local handlers = {
        -- Clearing the search returns focus to the field, as typing continues there.
        ClearSearch = function(scope, params)
            scope:member("Matching"):set("Search", "")
            local field = params.sender and params.sender:getParent():findChild("Search")
            if field then field:setFocus() end
        end,
    }
    return { computed = computed, operations = operations, hooks = hooks, filters = filters, media = media,
        handlers = handlers }
end

return Provider
