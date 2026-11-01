-- Visibility notifications after layout/scroll transforms settle. Native scene
-- ownership does not retain Lua userdata, so registrations stay strong until
-- unsubscribed or detached. Geometry follows ORCA's axis-aligned app layouts.
local Observer = {}
Observer.__index = Observer

local function deactivate(entry)
    entry.active, entry.node, entry.callback = false, nil, nil
end

function Observer.new(options)
    return setmetatable({ root = assert(options.root), retainedRoot = options.retainedRoot,
        margin = options.margin or 0, entries = {}, ordered = {} }, Observer)
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
    local width = frame.ActualWidth or frame.Width or 0
    local height = frame.ActualHeight or frame.Height or 0
    if width <= 0 or height <= 0 then return false end
    local x, y, right, bottom = 0, 0, width, height
    local node = frame
    while node do
        if node.Visible == false then return false end
        local transform = node.RenderTransformTranslation
        local dx = (node.ActualX or 0) + (transform and transform.X or 0)
        local dy = (node.ActualY or 0) + (transform and transform.Y or 0)
        x, right = x + dx, right + dx
        y, bottom = y + dy, bottom + dy
        if node == root then return true end
        local parent = node:getParent()
        if not parent then return false end
        local offset = parent.ContentOffset
        if offset then
            x, right = x + offset.X, right + offset.X
            y, bottom = y + offset.Y, bottom + offset.Y
        end
        if parent == root or (parent.OverflowX and parent.OverflowX ~= "Visible") then
            x, right = math.max(-margin, x), math.min((parent.ActualWidth or parent.Width or 0) + margin, right)
        end
        if parent == root or (parent.OverflowY and parent.OverflowY ~= "Visible") then
            y, bottom = math.max(-margin, y), math.min((parent.ActualHeight or parent.Height or 0) + margin, bottom)
        end
        if right <= x or bottom <= y then return false end
        node = parent
    end
    return false
end

function Observer:observe(node, callback)
    if self.entries[node] then deactivate(self.entries[node]) end
    local entry = { node = node, callback = callback, active = true }
    self.entries[node] = entry
    self.ordered[#self.ordered + 1] = entry
    self.dirty = true
    return function()
        deactivate(entry)
        if self.entries[node] == entry then self.entries[node] = nil end
        self.dirty = true
    end
end

function Observer:update()
    -- Keep registration order between paints. Compact only after membership
    -- changes; a separate snapshot preserves callback mutation semantics.
    if self.dirty then
        local ordered = {}
        for _, entry in ipairs(self.ordered) do
            if entry.active then ordered[#ordered + 1] = entry end
        end
        self.ordered, self.dirty = ordered, false
    end
    local ordered = self.ordered
    for i = 1, #ordered do
        local entry = ordered[i]
        if entry.active then
            local callback = entry.callback
            local attached = self:attached(entry.node)
            local visible = attached and self:isVisible(entry.node)
            if not attached then
                self.entries[entry.node], entry.active = nil, false
                self.dirty = true
            end
            if entry.visible ~= visible or not attached then
                entry.visible = visible
                callback { isVisible = visible, detached = not attached }
            end
            if not attached then deactivate(entry) end
        end
    end
end

function Observer:clear()
    for _, entry in pairs(self.entries) do deactivate(entry) end
    self.entries, self.ordered, self.dirty = {}, {}, false
end

return Observer
