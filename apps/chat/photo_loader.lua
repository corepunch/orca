-- Visible photos show their encrypted thumbnail while the full JPEG loads.
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
        targets = {}, cache = {}, previews = {}, generation = 0 }, Loader)
    self.observer = Observer.new {
        root = function() return app.screen end,
        retainedRoot = function() return app.shell and app.shell.screen end,
    }
    return self
end

function Loader:clear()
    self.generation = self.generation + 1
    self.observer:clear()
    self.targets, self.pending, self.requests, self.cache, self.previews = {}, {}, {}, {}, {}
end

function Loader:cached(attachment)
    return self.cache[key(attachment)]
end

function Loader:preview(attachment)
    return self.previews[key(attachment)]
end

function Loader:valid(target)
    return target.generation == self.generation and target.epoch == self.app.sessionEpoch
        and self.targets[target.frame] == target and self.observer:attached(target.frame)
end

function Loader:paint(target, path, err, preview)
    if not self:valid(target) or not self.observer:isVisible(target.frame) then return end
    if path then
        local image = ui.ImageView { Name = "Photo" .. target.id, Src = path,
            Foreground = "#ffffff", Stretch = "UniformToFill", IgnoreHitTest = true }
        image:send("Node.LoadView")
        if not image.Source then
            if preview then return end
            self.cache[target.key] = nil
            path, err = nil, { message = "This photo could not be decoded." }
        else
            if preview then target.preview = path
            else
                target.unsubscribe()
                self.targets[target.frame] = nil
            end
            clear(target.frame):addChild(image)
        end
    end
    if not path then
        target.failed = true
        if not target.preview then clear(target.frame) end
        target.frame:addChild(C.button("photo unavailable · retry", function()
            if not self:valid(target) then return end
            target.failed = false
            if target.preview then
                target.frame:findChild("RetryPhoto" .. target.id):removeFromParent()
            else
                local loading = C.icon("refresh-cw", 32, C.colors.muted)
                loading.Name = "PhotoLoading" .. target.id
                clear(target.frame):addChild(loading)
            end
            self:loadVisible()
        end, { Name = "RetryPhoto" .. target.id, Height = 44, FontSize = 12 }))
    end
    -- Timeline snapshots and the full-screen viewer must only cache the final JPEG.
    if not preview then target.done(path, err) end
    if self.app.repaint then self.app:repaint()
    elseif self.app.screen then
        self.app.screen:post("Window.Paint", require("orca.renderer").getSize())
    end
end

function Loader:pump()
    while self.active < MAX_ACTIVE and #self.pending > 0 do
        -- Finish the visible thumbnails before spending slots on full images.
        local nextRequest = 1
        for i, queued in ipairs(self.pending) do
            if queued.stage == "preview" then nextRequest = i; break end
        end
        local request = table.remove(self.pending, nextRequest)
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
                local current = false
                for _, target in ipairs(request.targets) do
                    if self:valid(target) and self.observer:isVisible(target.frame) then current = true; break end
                end
                if not current then
                    self.active = self.active - 1
                    if self.requests[request.identity] == request then self.requests[request.identity] = nil end
                    for _, target in ipairs(request.targets) do target.requested = false end
                    self:pump()
                    return
                end
                if request.stage == "preview" then
                    local ok, bytes = pcall(function() return self.app.client:getPhoto(request.attachment, true) end)
                    if request.generation == self.generation and request.epoch == self.app.sessionEpoch then
                        local visible = false
                        for _, target in ipairs(request.targets) do
                            if self:valid(target) and self.observer:isVisible(target.frame) then visible = true; break end
                        end
                        if ok and bytes and visible then
                            pcall(function()
                                local media = self.app:getMedia()
                                local preview = media and media:storePreview(bytes, "message-photo-preview-" .. request.key)
                                if preview then
                                    self.previews[request.key] = preview
                                    for _, target in ipairs(request.targets) do self:paint(target, preview, nil, true) end
                                end
                            end)
                        end
                        request.stage = "full"
                        self.pending[#self.pending + 1] = request
                    end
                    self.active = self.active - 1
                    self:pump()
                    return
                end
                local path, err = self.cache[request.key]
                local ok = pcall(function()
                    if path then return end
                    local bytes
                    bytes, err = self.app.client:getPhoto(request.attachment, false)
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
        if target.visible and not target.preview and self.previews[target.key] then
            self:paint(target, self.previews[target.key], nil, true)
        end
        if target.visible and not target.requested and not target.failed and self:valid(target) then
            local path = self.cache[target.key]
            if path then self:paint(target, path)
            else
                target.requested = true
                local identity = target.generation .. ":" .. target.key
                local request = self.requests[identity]
                if not request then
                    request = { attachment = target.attachment, key = target.key, identity = identity, epoch = target.epoch,
                        generation = target.generation, targets = {}, stage = self.previews[target.key] and "full" or "preview" }
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
    local target = { frame = frame, id = attachment.id, attachment = attachment, key = key(attachment),
        epoch = self.app.sessionEpoch, generation = self.generation, done = done or function() end }
    self.targets[frame] = target
    target.unsubscribe = self.observer:observe(frame, function(event)
        target.visible = event.isVisible
        if event.detached then self.targets[frame] = nil end
    end)
end

return Loader
