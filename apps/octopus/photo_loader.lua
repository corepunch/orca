-- View-lifetime-aware photo requests. The post-paint checkpoint only enqueues work; network
-- fetches and private-file writes run in the application's async coroutines.
local ui = require "orca.UIKit"
local C = require "Octopus/views/components"
local Loader = {}
Loader.__index = Loader
local MAX_ACTIVE = 2
local function watchers() return setmetatable({}, { __mode = "k" }) end

function Loader.new(app)
    return setmetatable({ app = app, active = 0, pending = {}, requests = {}, watching = watchers(), nextOrder = 0 }, Loader)
end

-- Intersect a photo with each scrolling/clipping ancestor. Octopus uses
-- translated, axis-aligned pages; include Pivot's translation as well as scroll.
local function rect(node)
    return { x = node.ActualX or 0, y = node.ActualY or 0, width = node.ActualWidth or node.Width or 0, height = node.ActualHeight or node.Height or 0 }
end

local function visible(app, frame, epoch)
    if app.sessionEpoch ~= epoch then return false end
    local size = rect(frame)
    if size.width <= 0 or size.height <= 0 then return false end
    local x, y, right, bottom = 0, 0, size.width, size.height
    local node = frame
    while node do
        if node.Visible == false then return false end
        local r, transform = rect(node), node.RenderTransformTranslation
        x, right = x + r.x + (transform and transform.X or 0), right + r.x + (transform and transform.X or 0)
        y, bottom = y + r.y + (transform and transform.Y or 0), bottom + r.y + (transform and transform.Y or 0)
        local parent = node:getParent()
        if not parent then return node == app.screen end
        local offset = parent.ContentOffset
        if offset then
            x, right = x + offset.X, right + offset.X
            y, bottom = y + offset.Y, bottom + offset.Y
        end
        local bounds = rect(parent)
        if parent == app.screen or (parent.OverflowX and parent.OverflowX ~= "Visible") then
            x, right = math.max(0, x), math.min(bounds.width, right)
        end
        if parent == app.screen or (parent.OverflowY and parent.OverflowY ~= "Visible") then
            y, bottom = math.max(0, y), math.min(bounds.height, bottom)
        end
        if right <= x or bottom <= y then return false end
        node = parent
    end
    return false
end

-- Called after Paint, once layout and scroll transforms have settled. Only
-- unloaded photo frames participate; completed frames leave this weak registry.
function Loader:loadVisible()
    local ordered = {}
    for _, target in pairs(self.watching) do ordered[#ordered + 1] = target end
    table.sort(ordered, function(a, b) return a.order < b.order end)
    for _, target in ipairs(ordered) do target.load() end
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
    local function valid() return visible(app, frame, epoch) end
    local function finish(path, err)
        requested = false
        if path then
            C.clear(frame)
            frame:addChild(ui.ImageView { Name = name, Src = path, Stretch = "Uniform",
                Foreground = "#ffffff", IgnoreHitTest = true })
            loaded = true
            self.watching[frame] = nil
        elseif err then
            failed = true
            frame.IgnoreHitTest = false
            C.clear(frame)
            frame:addChild(C.button("photo unavailable · retry", function()
                failed = false
                C.clear(frame):addChild(C.icon("image", 28, C.colors.dim))
                app:repaint()
            end, { Name = "Retry" .. name, FontSize = 13, Height = 48 }))
        end
        app:repaint()
    end
    self.nextOrder = self.nextOrder + 1
    self.watching[frame] = { order = self.nextOrder, load = function()
        if loaded or requested or failed or not valid() then return end
        local path = app.media:path(key)
        if path then finish(path); return end
        requested = true
        local target = { valid = valid, done = finish, cancel = function() requested = false end }
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
end

return Loader
