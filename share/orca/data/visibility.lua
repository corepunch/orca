-- Presentation-side bridge from the viewport to media sources: after paint,
-- it walks the visible part of a page, maps each row's DataContext to its
-- record and offers it to the scope's media. Rows need no registration and
-- the data layer never holds nodes. Geometry follows ViewportObserver.
local Visibility = {}
Visibility.__index = Visibility

local function size(node)
    return node.ActualWidth or node.Width or 0, node.ActualHeight or node.Height or 0
end

-- options: margin (points beyond the viewport that count as near).
function Visibility.new(options)
    options = options or {}
    return setmetatable({ pages = {}, margin = options.margin or 160 }, Visibility)
end

-- Watches `node` for records of `scope`; `sources` defaults to every media
-- source in the scope. Returns a function that stops watching.
function Visibility:watch(node, scope, sources)
    local entry = { node = node, scope = scope, sources = sources }
    self.pages[#self.pages + 1] = entry
    return function()
        for index, page in ipairs(self.pages) do
            if page == entry then table.remove(self.pages, index); return end
        end
    end
end

local function collect(self, node, x, y, clip, scope, found, seen)
    if node.Visible == false then return end
    local w, h = size(node)
    local t = node.RenderTransformTranslation
    local left, top = x + (node.ActualX or 0) + (t and t.X or 0), y + (node.ActualY or 0) + (t and t.Y or 0)
    local right, bottom = left + w, top + h
    if right < clip[1] or bottom < clip[2] or left > clip[3] or top > clip[4] then return end
    local context = node.DataContext
    if context then
        local record = scope.store:recordFor(context)
        if record and not seen[record] then seen[record] = true; found[#found + 1] = record end
    end
    local child = node:getFirstChild()
    if not child then return end
    local inner = clip
    if (node.OverflowX and node.OverflowX ~= "Visible") or (node.OverflowY and node.OverflowY ~= "Visible") then
        local m = self.margin
        inner = { math.max(clip[1], left - m), math.max(clip[2], top - m),
            math.min(clip[3], right + m), math.min(clip[4], bottom + m) }
    end
    local offset = node.ContentOffset
    local ox, oy = left + (offset and offset.X or 0), top + (offset and offset.Y or 0)
    while child do
        collect(self, child, ox, oy, inner, scope, found, seen)
        child = child:getNext()
    end
end

-- The records visible in a watched page, in tree order.
function Visibility:records(entry)
    local w, h = size(entry.node)
    local found = {}
    if entry.node:getParent() == nil and entry.node:getRoot() ~= entry.node then return found end
    local m = self.margin
    collect(self, entry.node, -(entry.node.ActualX or 0), -(entry.node.ActualY or 0), { -m, -m, w + m, h + m },
        entry.scope, found, {})
    return found
end

function Visibility:update()
    for _, entry in ipairs({ table.unpack(self.pages) }) do
        if not entry.scope.disposed then
            local records = self:records(entry)
            for _, member in ipairs(entry.sources or entry.scope.order) do
                if member.lookup and member.want then member:lookup(records) end
            end
        end
    end
end

return Visibility
