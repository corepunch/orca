-- Visibility notifications after layout/scroll transforms settle. Native scene
-- ownership does not retain Lua userdata, so registrations stay strong until
-- unsubscribed or detached. Geometry follows ORCA's axis-aligned app layouts.
local Observer = {}
Observer.__index = Observer

local function rect(node)
    return { x = node.ActualX or 0, y = node.ActualY or 0,
        width = node.ActualWidth or node.Width or 0, height = node.ActualHeight or node.Height or 0 }
end

function Observer.new(options)
    return setmetatable({ root = assert(options.root), retainedRoot = options.retainedRoot,
        margin = options.margin or 0, entries = {}, sequence = 0 }, Observer)
end

function Observer:attached(node)
    local root = self.root()
    local retained = self.retainedRoot and self.retainedRoot()
    while node do
        if node == root or node == retained then return true end
        node = node:getParent()
    end
    return false
end

function Observer:isVisible(frame, margin)
    local root = self.root()
    margin = margin == nil and self.margin or margin
    if not root then return false end
    local size = rect(frame)
    if size.width <= 0 or size.height <= 0 then return false end
    local x, y, right, bottom = 0, 0, size.width, size.height
    local node = frame
    while node do
        if node.Visible == false then return false end
        local r, transform = rect(node), node.RenderTransformTranslation
        x, right = x + r.x + (transform and transform.X or 0), right + r.x + (transform and transform.X or 0)
        y, bottom = y + r.y + (transform and transform.Y or 0), bottom + r.y + (transform and transform.Y or 0)
        if node == root then return true end
        local parent = node:getParent()
        if not parent then return false end
        local offset = parent.ContentOffset
        if offset then
            x, right = x + offset.X, right + offset.X
            y, bottom = y + offset.Y, bottom + offset.Y
        end
        local bounds = rect(parent)
        if parent == root or (parent.OverflowX and parent.OverflowX ~= "Visible") then
            x, right = math.max(-margin, x), math.min(bounds.width + margin, right)
        end
        if parent == root or (parent.OverflowY and parent.OverflowY ~= "Visible") then
            y, bottom = math.max(-margin, y), math.min(bounds.height + margin, bottom)
        end
        if right <= x or bottom <= y then return false end
        node = parent
    end
    return false
end

function Observer:observe(node, callback)
    if self.entries[node] then self.entries[node].active = false end
    self.sequence = self.sequence + 1
    local entry = { node = node, callback = callback, order = self.sequence, active = true }
    self.entries[node] = entry
    return function()
        entry.active = false
        if self.entries[node] == entry then self.entries[node] = nil end
    end
end

function Observer:update()
    local ordered = {}
    for _, entry in pairs(self.entries) do ordered[#ordered + 1] = entry end
    table.sort(ordered, function(a, b) return a.order < b.order end)
    for _, entry in ipairs(ordered) do
        if entry.active then
            local attached = self:attached(entry.node)
            local visible = attached and self:isVisible(entry.node)
            if not attached then
                self.entries[entry.node], entry.active = nil, false
            end
            if entry.visible ~= visible or not attached then
                entry.visible = visible
                entry.callback { isVisible = visible, detached = not attached }
            end
        end
    end
end

function Observer:clear()
    for _, entry in pairs(self.entries) do entry.active = false end
    self.entries = {}
end

return Observer
