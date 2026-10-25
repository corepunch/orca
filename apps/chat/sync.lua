local core = require "orca.core"
local Polling = require "Chat/polling"
local Sync = {}
Sync.__index = Sync

local function equal(left, right)
    if type(left) ~= type(right) then return false end
    if type(left) ~= "table" then return left == right end
    for key, value in pairs(left) do
        if key ~= "_model" and not equal(value, right[key]) then return false end
    end
    for key in pairs(right) do
        if key ~= "_model" and left[key] == nil then return false end
    end
    return true
end

-- All inbox reads share a lock, including manual refresh/navigation.
function Sync.refreshInbox(app, current, quiet)
    if app.inboxRefreshBusy then return true end
    app.inboxRefreshBusy = true
    local ok, result, err = xpcall(function()
        local rooms, roomError = app.models.Rooms:select()
        if not current() then return true end
        if not rooms then return nil, roomError end
        local contacts, contactError = app.models.Contacts:select()
        if not current() then return true end
        if not contacts then return nil, contactError end
        local changed = not equal(rooms, app.rooms) or not equal(contacts, app.contacts)
        app.rooms, app.contacts, app.contactsById = rooms, contacts, {}
        for _, contact in ipairs(contacts) do app.contactsById[contact.id] = contact end
        if changed and app.roomLists then
            local offsets = {}
            for key, list in pairs(app.roomLists) do
                offsets[key] = { X = list.ContentOffset.X, Y = list.ContentOffset.Y }
            end
            app:renderRoomList()
            for key, list in pairs(app.roomLists) do list.ContentOffset = offsets[key] end
        end
        return true
    end, debug.traceback)
    app.inboxRefreshBusy = false
    if not ok then
        if current() and app.reportError then app:reportError(result, "Syncing conversations") end
        result, err = nil, { code = "network", message = "Could not sync conversations. Please try again." }
    end
    if not result and not quiet and current() then app:notice(err.message, true) end
    return result, err
end

function Sync.new(app, options)
    options = options or {}
    local self = setmetatable({ app = app }, Sync)
    options.onError = options.onError or function(err)
        if app.reportError then app:reportError(err, "Polling") end
    end
    if not options.createTimer then
        -- One retained Timer owns the timer. Timer.Tick is dispatched through
        -- ORCA's yieldable event coroutine, just like button handlers.
        self.owner = core.Timer { Name = "ChatPolling" }
        options.createTimer = function(callback, duration)
            local id, canceled
            local owner = self.owner
            local handle = {}
            function handle:cancel()
                if canceled then return end
                canceled = true
                owner:cancelTimer(id)
                owner.Tick = nil
            end
            owner.Tick = function(_, args)
                if canceled or args.timerId ~= id then return end
                -- Native timers repeat; turn this tick into a one-shot before
                -- the HTTP request can yield or another tick can be dispatched.
                handle:cancel()
                callback()
            end
            id = owner:setTimer(duration)
            if not id or id <= 0 then
                owner.Tick = nil
                error("Could not schedule Chat polling", 0)
            end
            return handle
        end
    end
    self.scheduler = Polling.new(options)
    return self
end

function Sync:stop()
    self.scheduler:stop()
end

function Sync:setForeground(active)
    self.scheduler:setForeground(active)
end

function Sync:activate()
    local app, screen = self.app, self.app.screen
    local session = app.client and app.client:getSession()
    if not session then self:stop(); return end
    local navigation, epoch, user = app.navigationEpoch, app.sessionEpoch, session.user.id
    local thread = app.room and app.messageList and app.messageList:getRoot() == screen
    local inbox = app.shell and app.shell.screen == screen and app.shell.active == "Chats"
    if not thread and not inbox then self:stop(); return end
    local room, node = app.room, app.messageList
    local function valid()
        local active = app.client:getSession()
        return app.screen == screen and app.navigationEpoch == navigation and app.sessionEpoch == epoch
            and active ~= nil and active.user.id == user
            and (not thread or (app.room == room and app.messageList == node))
    end
    self.scheduler:start {
        interval = thread and 3000 or 15000,
        valid = valid,
        run = function(current)
            if app.sending then return true end
            local result, err
            if thread then
                result, err = app:renderMessages { current = current, preserveScroll = true, quiet = true }
            else
                result, err = Sync.refreshInbox(app, current, true)
            end
            if not result and err and current() then app:notice(err.message, true) end
            -- A revoked session stops the job's valid predicate, but still
            -- needs to remove authenticated views from the active screen.
            if app.screen == screen and app.navigationEpoch == navigation and app.sessionEpoch == epoch
                and not app.client:getSession() then
                app:resetSessionViews()
                app:show("/")
            end
            return result, err
        end,
    }
end

return Sync
