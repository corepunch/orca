-- Full-size encrypted photos are fetched only after their frame is visible.
local ui = require "orca.UIKit"
local C = require "Chat/views/components"
local Observer = require "orca.UIKit.ViewportObserver"
local Loader = {}
Loader.__index = Loader
local MAX_ACTIVE = 2
local function key(attachment)
    return attachment.id .. ":" .. tostring(attachment.updated_at or "")
end
local function clear(node)
    while node:getFirstChild() do node:getFirstChild():removeFromParent() end
    return node
end

function Loader.new(app)
    local self = setmetatable({ app = app, active = 0, pending = {}, requests = {},
        targets = {}, cache = {}, generation = 0 }, Loader)
    self.observer = Observer.new {
        root = function() return app.screen end,
        retainedRoot = function() return app.shell and app.shell.screen end,
    }
    return self
end

function Loader:clear()
    self.generation = self.generation + 1
    self.observer:clear()
    self.targets, self.pending, self.requests, self.cache = {}, {}, {}, {}
end

function Loader:cached(attachment)
    return self.cache[key(attachment)]
end

function Loader:valid(target)
    return target.generation == self.generation and target.epoch == self.app.sessionEpoch
        and self.targets[target.frame] == target and self.observer:attached(target.frame)
end

function Loader:paint(target, path, err)
    if not self:valid(target) or not self.observer:isVisible(target.frame) then return end
    clear(target.frame)
    if path then
        target.frame:addChild(ui.ImageView { Name = "Photo" .. target.id, Src = path,
            Foreground = "#ffffff", Stretch = "UniformToFill", IgnoreHitTest = true })
        target.unsubscribe()
        self.targets[target.frame] = nil
    else
        target.failed = true
        target.frame:addChild(C.button("photo unavailable · retry", function()
            if not self:valid(target) then return end
            target.failed = false
            clear(target.frame):addChild(C.text("Loading photo…", { FontSize = 12, Foreground = C.colors.muted }))
            self:loadVisible()
        end, { Name = "RetryPhoto" .. target.id, Height = 44, FontSize = 12 }))
    end
    target.done(path, err)
    if self.app.repaint then self.app:repaint()
    elseif self.app.screen then
        self.app.screen:post("Window.Paint", require("orca.renderer").getSize())
    end
end

function Loader:pump()
    while self.active < MAX_ACTIVE and #self.pending > 0 do
        local request = table.remove(self.pending, 1)
        local wanted = false
        for _, target in ipairs(request.targets) do
            if self:valid(target) and self.observer:isVisible(target.frame) then wanted = true; break end
        end
        if not wanted then
            self.requests[request.identity] = nil
            for _, target in ipairs(request.targets) do target.requested = false end
        else
            self.active = self.active + 1
            local function work()
                local path, err = self.cache[request.key]
                local ok = pcall(function()
                    if path then return end
                    local bytes
                    bytes, err = self.app.client:getPhoto(request.id, false)
                    if request.generation ~= self.generation or request.epoch ~= self.app.sessionEpoch then return end
                    local media = self.app:getMedia()
                    if bytes and media then path, err = media:storePreview(bytes, "message-photo-" .. request.key) end
                    if not bytes and not err then err = { message = "Photo preview unavailable" } end
                    if path then self.cache[request.key] = path end
                end)
                if not ok then err = { message = "Photo preview unavailable" } end
                self.active = self.active - 1
                if self.requests[request.identity] == request then self.requests[request.identity] = nil end
                if request.generation == self.generation and request.epoch == self.app.sessionEpoch then
                    for _, target in ipairs(request.targets) do
                        target.requested = false
                        self:paint(target, path, err)
                    end
                end
                self:pump()
            end
            if self.app.task then self.app:task(work) else require("orca").async(work) end
        end
    end
end

function Loader:loadVisible()
    self.observer:update()
    for _, target in pairs(self.targets) do
        if target.visible and not target.requested and not target.failed and self:valid(target) then
            local path = self.cache[target.key]
            if path then self:paint(target, path)
            else
                target.requested = true
                local identity = target.generation .. ":" .. target.key
                local request = self.requests[identity]
                if not request then
                    request = { id = target.id, key = target.key, identity = identity, epoch = target.epoch,
                        generation = target.generation, targets = {} }
                    self.requests[identity] = request
                    self.pending[#self.pending + 1] = request
                end
                request.targets[#request.targets + 1] = target
            end
        end
    end
    self:pump()
end

function Loader:watch(frame, attachment, done)
    local target = { frame = frame, id = attachment.id, key = key(attachment),
        epoch = self.app.sessionEpoch, generation = self.generation, done = done or function() end }
    self.targets[frame] = target
    target.unsubscribe = self.observer:observe(frame, function(event)
        target.visible = event.isVisible
        if event.detached then self.targets[frame] = nil end
    end)
end

return Loader
