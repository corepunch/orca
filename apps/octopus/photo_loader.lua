-- View-lifetime-aware photo requests. The post-paint checkpoint only enqueues work; network
-- fetches and private-file writes run in the application's async coroutines.
local ui = require "orca.UIKit"
local C = require "Octopus/views/components"
local Observer = require "orca.UIKit.ViewportObserver"
local Loader = {}
Loader.__index = Loader
local MAX_ACTIVE = 2
-- Frames that follow their photo keep its shape between Instagram's limits:
-- 4:5 portrait to 1.91:1 landscape. Beyond them the photo is centre-cropped.
local TALLEST, WIDEST, GUESS = 4 / 5, 1.91, 4 / 3

function Loader.new(app)
    return setmetatable({ app = app, active = 0, pending = {}, requests = {}, watching = {}, nextOrder = 0, ratios = {},
        observer = Observer.new { root = function() return app.screen end,
            retainedRoot = function() return app.shell and app.shell.screen end } }, Loader)
end

-- Visibility events settle after paint. Flush requests together in registration
-- order; keep pending visible targets eligible when a cancelled worker finishes.
function Loader:loadVisible()
    for frame, target in pairs(self.watching) do
        if self.app.sessionEpoch ~= target.epoch then
            target.cancel()
            target.stop()
            self.watching[frame] = nil
        end
    end
    self.observer:update()
    local ordered = {}
    for _, target in pairs(self.watching) do
        if target.inRange then ordered[#ordered + 1] = target end
    end
    table.sort(ordered, function(a, b) return a.order < b.order end)
    for _, target in ipairs(ordered) do target.load() end
end

-- The aspect ratio to reserve for a photo: its own once it has been decoded.
function Loader:aspect(key)
    return self.ratios[key] or GUESS
end

function Loader:pump()
    while self.active < MAX_ACTIVE and #self.pending > 0 do
        local request = table.remove(self.pending, 1)
        self.active = self.active + 1
        self.app:task(function()
            local path, err
            local ok = pcall(function()
                local wanted = false
                for _, target in ipairs(request.targets) do
                    if target.valid() then wanted = true; break end
                end
                if not wanted then return end
                path = self.app.media:path(request.key)
                if not path then
                    local bytes
                    bytes, err = self.app.client:getImage(request.key)
                    if bytes and self.app.sessionEpoch == request.epoch then
                        path, err = self.app.media:store(bytes, request.key)
                    end
                end
            end)
            if not ok then err = { message = "This photo could not be loaded." } end
            self.active = self.active - 1
            self.requests[request.identity] = nil
            for _, target in ipairs(request.targets) do
                if target.valid() then target.done(path, err) else target.cancel() end
            end
            self:pump()
        end)
    end
end

function Loader:watch(frame, key, name)
    local app, epoch = self.app, self.app.sessionEpoch
    local requested, loaded, failed = false, false, false
    local stop, indicator
    local function loading(active)
        if active and not indicator then
            indicator = ui.ActivityIndicator { Name = "Loading" .. name, Foreground = C.colors.dim }
            C.clear(frame):addChild(indicator)
        elseif not active and indicator then
            indicator.IsActive = false
            indicator:removeFromParent()
            indicator = nil
        end
    end
    local function cancel()
        requested = false
        loading(false)
    end
    local function valid() return app.sessionEpoch == epoch and self.observer:isVisible(frame) end
    local function finish(path, err)
        requested = false
        loading(false)
        if path then
            -- A frame with an aspect ratio takes the photo's shape and is filled
            -- edge to edge; a fixed frame fits the photo in its top-left corner.
            local shaped = frame.AspectRatio > 0
            local image = ui.ImageView { Name = name, Src = path, Foreground = "#ffffff", IgnoreHitTest = true,
                Stretch = shaped and "UniformToFill" or "Uniform" }
            if not shaped then image.MarginLeft, image.MarginRight, image.MarginTop, image.MarginBottom = 0, 0/0, 0, 0/0 end
            image:send("Node.LoadView")
            if not image.Source then
                app.media:discard(key)
                path, err = nil, { message = "This photo could not be decoded." }
            else
                local width, height = image.Source.Width, image.Source.Height
                if shaped and width > 0 and height > 0 then
                    self.ratios[key] = math.max(TALLEST, math.min(WIDEST, width / height))
                    frame.AspectRatio = self.ratios[key]
                end
                C.clear(frame):addChild(image)
                frame.Background = "#00000000"
                frame.IgnoreHitTest = true
            end
        end
        if path then
            loaded = true
            if stop then stop() end
            self.watching[frame] = nil
        elseif err then
            failed = true
            frame.Background = C.colors.raised
            frame.IgnoreHitTest = false
            C.clear(frame)
            frame:addChild(C.area(C.icon("image-off", 28, C.colors.dim, { Name = "Unavailable" .. name }), function()
                failed = false
                C.clear(frame):addChild(C.icon("image", 28, C.colors.dim))
                app:repaint()
            end, { Name = "Retry" .. name }))
        end
        app:repaint()
    end
    self.nextOrder = self.nextOrder + 1
    local target = { order = self.nextOrder, epoch = epoch, cancel = cancel, load = function()
        if loaded or requested or failed or not valid() then return end
        local path = app.media:path(key)
        if path then finish(path); return end
        requested = true
        loading(true)
        local target = { valid = valid, done = finish, cancel = cancel }
        local identity = epoch .. ":" .. key
        local request = self.requests[identity]
        if not request then
            request = { key = key, identity = identity, epoch = epoch, targets = {} }
            self.requests[identity] = request
            self.pending[#self.pending + 1] = request
        end
        request.targets[#request.targets + 1] = target
        self:pump()
    end }
    self.watching[frame] = target
    stop = self.observer:observe(frame, function(event)
        target.inRange = event.isVisible
        loading(event.isVisible and requested)
        if event.detached then self.watching[frame] = nil end
    end)
    target.stop = stop
end

return Loader
