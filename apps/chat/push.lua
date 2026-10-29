-- Native events contain hints only; room access is always checked by the API.
local Polling = require "Chat/polling"
local Push = {}
Push.__index = Push

local function validId(value)
    return type(value) == "string" and #value <= 36 and value:match("^[%w][%w%._%-]*$") ~= nil
end

function Push.new(app, config, options)
    options = options or {}
    if type(config.pushProviderId) ~= "string" or config.pushProviderId == "" then return nil end
    local native = options.native
    if not native then
        local ok, module = pcall(require, "orca.notifications")
        if not ok then return nil end
        native = module
    end
    if not native.supported() then return nil end
    local self = setmetatable({ app = app, native = native, clock = options.clock or os.time }, Push)
    if not options.createTimer then
        local core = require "orca.core"
        self.owner = core.Timer { Name = "ChatPush" }
        options.createTimer = function(callback, duration)
            local owner, id, canceled = self.owner
            local handle = {}
            function handle:cancel()
                if canceled then return end
                canceled = true
                owner:cancelTimer(id)
                owner.Tick = nil
            end
            owner.Tick = function(_, args)
                if canceled or args.timerId ~= id then return end
                handle:cancel()
                callback()
            end
            id = owner:setTimer(duration)
            assert(id and id > 0, "Could not schedule notifications")
            return handle
        end
    end
    self.scheduler = Polling.new(options)
    return self
end

function Push:tick(current)
    local app, native = self.app, self.native
    local session = app.client:getSession()
    local user = session and session.user.id
    if self.user ~= user then
        if self.user then self.pending = nil end
        self.user, self.registered = user, nil
    end
    if user and not self.requested and (not self.retryAt or self.clock() >= self.retryAt) then
        self.requested = native.requestAuthorization() == true
    end
    -- A bounded drain prevents an event burst from monopolizing the UI.
    for _ = 1, 64 do
        local event = native.poll()
        if not event then break end
        if event.type == "token" and type(event.token) == "string" then
            if self.token ~= event.token then self.token, self.registered = event.token, nil end
            self.nativeFailures, self.retryAt, self.nativeError = 0, nil, nil
        elseif event.type == "open" and validId(event.conversationId) and validId(event.messageId)
            and validId(event.userId) then
            if not user or event.userId == user then self.pending = event end
        elseif event.type == "received" and user and event.userId == user then
            -- A message arrived: the open conversation reads its newest page.
            local thread = app.threadScope
            if thread and not thread.disposed then thread:member("Messages"):refresh { quiet = true } end
        elseif event.type == "error" then
            self.nativeError = event.code or "notifications"
            if self.lastError ~= self.nativeError and app.notice then
                app:notice("Notifications: " .. (event.message or "Could not enable notifications."), true)
            end
            self.lastError = self.nativeError
            if event.code == "registration_failed" or event.code == "authorization_failed" or event.code == "invalid_token" then
                self.nativeFailures = math.min((self.nativeFailures or 0) + 1, 6)
                self.requested, self.retryAt = false, self.clock() + math.min(60, 2 ^ self.nativeFailures)
            end
        end
    end
    local function sameAccount()
        local active = app.client:getSession()
        return current() and active and active.user.id == user and not app.logoutPending
    end
    local success = true
    if user and self.token and self.registered ~= self.token then
        local token = self.token
        local registered = app.client:registerPushTarget(token)
        if not sameAccount() then return true end
        if registered then self.registered = token else success = false end
    end
    local pending = self.pending
    if pending and user then
        if pending.userId ~= user then self.pending = nil; return success end
        local epoch, navigation = app.sessionEpoch, app.navigationEpoch
        local rooms, err = app.models.Rooms:select()
        if not sameAccount() or app.sessionEpoch ~= epoch or app.navigationEpoch ~= navigation
            or self.pending ~= pending then return true end
        if not rooms then
            if err and (err.status == 401 or err.status == 403) then self.pending = nil end
            return false
        end
        self.pending = nil
        for _, room in ipairs(rooms) do
            if room.id == pending.conversationId then app:openRoom(room); break end
        end
    end
    return success
end

function Push:activate()
    if self.app.logoutPending then return end
    if self.scheduler.job then self.scheduler:refresh(); return end
    self.scheduler:start({ interval = 1000, run = function(current) return self:tick(current) end }, true)
end

function Push:suspend()
    self.pending, self.registered = nil, nil
    self.scheduler:stop()
end

function Push:setForeground(active)
    if active and self.nativeError == "permission_denied" then self.requested = false end
    self.scheduler:setForeground(active)
end

return Push
