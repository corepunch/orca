local core = require "orca.core"
local Polling = require "Chat/polling"
local Sync = {}
Sync.__index = Sync

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
    -- The inbox polls through its Rooms query; this job covers the open thread.
    local thread = app.room and app.messageList and app.messageList:getRoot() == screen
    if not thread then self:stop(); return end
    local room, node = app.room, app.messageList
    local function valid()
        local active = app.client:getSession()
        return app.screen == screen and app.navigationEpoch == navigation and app.sessionEpoch == epoch
            and active ~= nil and active.user.id == user
            and app.room == room and app.messageList == node
    end
    self.scheduler:start {
        interval = 3000,
        valid = valid,
        run = function(current)
            if app.sending then return true end
            local result, err = app:renderMessages { current = current, preserveScroll = true, quiet = true }
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
