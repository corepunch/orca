local Observer = require "orca.UIKit.ViewportObserver"
local Loader = {}
Loader.__index = Loader
local MAX_ACTIVE = 2

function Loader.new(app, adapter)
    local self = setmetatable({ app = app, adapter = adapter, epoch = app.sessionEpoch,
        targets = {}, pending = {}, files = {}, generations = {}, active = 0, sequence = 0 }, Loader)
    self.observer = Observer.new {
        root = function() return app.screen end,
        retainedRoot = function() return app.shell and app.shell.screen end,
        margin = 128,
    }
    return self
end

function Loader:current()
    return not self.closed and self.app.sessionEpoch == self.epoch
end

function Loader:valid(target)
    return self:current() and self.targets[target]
        and target.generation == (self.generations[target.userId] or 0)
end

function Loader:forget(target)
    self.targets[target] = nil
    if target.unsubscribe then target.unsubscribe(); target.unsubscribe = nil end
end

function Loader:invalidate(userId)
    self.generations[userId] = (self.generations[userId] or 0) + 1
    self.app.avatarPaths[userId] = nil
    for target in pairs(self.targets) do
        if target.userId == userId then self:forget(target) end
    end
end

function Loader:clear()
    self.closed = true
    self.observer:clear()
    self.targets, self.pending, self.files = {}, {}, {}
end

function Loader:apply(target, path)
    if not self:valid(target) then return end
    if self.observer:isVisible(target.node) then
        self.adapter.apply(target.node, path)
        self:forget(target)
    else target.state = nil end
end

function Loader:watch(node, userId)
    local path = self.app.avatarPaths[userId]
    if path then return self.adapter.apply(node, path) end
    if path == false or not self:current() then return end
    self.sequence = self.sequence + 1
    local target = { node = node, userId = userId, generation = self.generations[userId] or 0, order = self.sequence }
    self.targets[target] = true
    target.unsubscribe = self.observer:observe(node, function(event)
        target.near = event.isVisible
        if event.detached then self:forget(target) end
    end)
end

function Loader:pump()
    while self:current() and self.active < MAX_ACTIVE and #self.pending > 0 do
        local request = table.remove(self.pending, 1)
        local wanted = false
        for _, target in ipairs(request.targets) do
            if self:valid(target) and self.observer:isVisible(target.node) then wanted = true; break end
        end
        if not wanted then
            self.files[request.id] = nil
            for _, target in ipairs(request.targets) do if self:valid(target) then target.state = nil end end
        else
            self.active = self.active + 1
            self.adapter.schedule(function()
                local path
                local ok = pcall(function()
                    path = self.adapter.cached(request.id)
                    if not path then
                        local bytes = self.adapter.fetch(request.id)
                        if bytes and self:current() then path = self.adapter.store(bytes, request.id) end
                    end
                end)
                self.active = self.active - 1
                self.files[request.id] = nil
                if self:current() then
                    for _, target in ipairs(request.targets) do
                        if self:valid(target) then
                            if ok and path then
                                self.app.avatarPaths[target.userId] = path
                                self:apply(target, path)
                            else
                                -- Keep initials on failure. New views can retry; paints cannot spin.
                                self:forget(target)
                            end
                        end
                    end
                    if ok and path then self.adapter.repaint() end
                    self:pump()
                end
            end)
        end
    end
end

function Loader:loadVisible()
    if not self:current() then return end
    self.observer:update()
    local batch, users = {}, {}
    for target in pairs(self.targets) do
        if target.near and not target.state and self:valid(target) then
            local path = self.app.avatarPaths[target.userId]
            if path then self:apply(target, path)
            elseif path == false then self:forget(target)
            else
                batch[#batch + 1] = target
                users[target.userId] = true
            end
        end
    end
    if #batch == 0 or self.lookupRunning then return self:pump() end
    table.sort(batch, function(a, b)
        local av, bv = self.observer:isVisible(a.node, 0), self.observer:isVisible(b.node, 0)
        if av ~= bv then return av end
        return a.order < b.order
    end)
    local ids = {}
    for id in pairs(users) do ids[#ids + 1] = id end
    table.sort(ids)
    for _, target in ipairs(batch) do target.state = "lookup" end
    self.lookupRunning = true
    self.adapter.schedule(function()
        local ok, avatars = pcall(self.adapter.lookup, ids)
        self.lookupRunning = false
        if not self:current() then return end
        local changed = false
        for _, target in ipairs(batch) do
            if self:valid(target) then
                local fileId = ok and avatars and avatars[target.userId]
                if not fileId then
                    if fileId == false then self.app.avatarPaths[target.userId] = false end
                    self:forget(target)
                else
                    local cached = self.adapter.cached(fileId)
                    if cached then
                        self.app.avatarPaths[target.userId] = cached
                        self:apply(target, cached)
                        changed = true
                    else
                        target.state = "file"
                        local request = self.files[fileId]
                        if not request then
                            request = { id = fileId, targets = {} }
                            self.files[fileId] = request
                            self.pending[#self.pending + 1] = request
                        end
                        request.targets[#request.targets + 1] = target
                    end
                end
            end
        end
        if changed then self.adapter.repaint() end
        self:pump()
    end)
end

return Loader
