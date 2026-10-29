-- A keyed, ordered collection of store records presented as native
-- DataObject rows. Rows keep their native identity while their key remains;
-- membership edits are batched into one ItemsControl notification.
local core = require "orca.core"
local Collection = {}
Collection.__index = Collection

function Collection.new(store, options)
    options = options or {}
    local self = setmetatable({ store = store, rows = {}, byKey = {}, listeners = {}, watchers = {},
        name = options.name or "Items" }, Collection)
    self.node = options.node or core.DataObject { Name = self.name }
    if options.parent then options.parent:addChild(self.node) end
    -- Deleted records leave every collection that shows them.
    self.unlisten = store:listen(function(record, changes)
        if changes.deleted then self:remove(record) end
    end)
    return self
end

local function rowKey(item)
    if item.record then return item.key or item.record.key, item.record end
    return item.key, item
end

-- Replaces the membership with `items`: store records, or { key, record }
-- when one record appears in several rows (a post and its reposts).
-- `change` describes the edit to watchers (for example { prepended = true }).
function Collection:set(items, change)
    local ordered, seen = {}, {}
    for index, item in ipairs(items) do
        local key, record = rowKey(item)
        if key == nil then error(("Collection %s row %d has no key"):format(self.name, index), 2) end
        if seen[key] then error(("Duplicate key in collection %s: %s"):format(self.name, tostring(key)), 2) end
        seen[key] = true
        ordered[index] = { key = key, record = record }
    end
    local rows, byKey = {}, {}
    for index, item in ipairs(ordered) do
        local row = self.byKey[item.key]
        -- The same key with another record (a replaced scope) gets a new row.
        if not row or row.record ~= item.record then
            row = { key = item.key, record = item.record, mirror = self.store:mirror(item.record) }
        end
        rows[index], byKey[item.key] = row, row
    end
    change = change or {}
    for _, watcher in ipairs({ table.unpack(self.watchers) }) do
        if watcher.before then watcher.before(self, change) end
    end
    local node = self.node
    node:beginUpdate()
    local ok, err = xpcall(function()
        for key, row in pairs(self.byKey) do
            if byKey[key] ~= row then
                row.mirror.object:removeFromParent()
                self.store:release(row.mirror)
            end
        end
        for _, row in ipairs(rows) do
            if row.mirror.object:getParent() ~= node then node:addChild(row.mirror.object) end
        end
        local before
        for index = #rows, 1, -1 do
            local object = rows[index].mirror.object
            if object:getNext() ~= before then object:moveBefore(before) end
            before = object
        end
    end, debug.traceback)
    node:endUpdate()
    if not ok then error(err, 0) end
    self.rows, self.byKey = rows, byKey
    self:emit()
    for _, watcher in ipairs({ table.unpack(self.watchers) }) do
        if watcher.after then watcher.after(self, change) end
    end
    return rows
end

local function current(self)
    local list = {}
    for index, row in ipairs(self.rows) do list[index] = { key = row.key, record = row.record } end
    return list
end

function Collection:append(items)
    local combined = current(self)
    for _, item in ipairs(items) do
        local key = rowKey(item)
        if not self.byKey[key] then combined[#combined + 1] = item end
    end
    return self:set(combined, { appended = true })
end

-- Older rows before the current ones (a conversation's history). Rows
-- already present keep their place.
function Collection:prepend(items)
    local combined, seen = {}, {}
    for _, item in ipairs(items) do
        local key = rowKey(item)
        if not self.byKey[key] and not seen[key] then seen[key] = true; combined[#combined + 1] = item end
    end
    for _, item in ipairs(current(self)) do combined[#combined + 1] = item end
    return self:set(combined, { prepended = true })
end

-- `watcher.before(collection, change)` and `watcher.after(collection, change)`
-- run around every membership change, so a view can keep its place.
function Collection:watch(watcher)
    self.watchers[#self.watchers + 1] = watcher
    return function()
        for index, entry in ipairs(self.watchers) do
            if entry == watcher then table.remove(self.watchers, index); return end
        end
    end
end

function Collection:remove(record)
    if self.disposed then return end
    local keep = {}
    for _, row in ipairs(self.rows) do
        if row.record ~= record then keep[#keep + 1] = { key = row.key, record = row.record } end
    end
    if #keep ~= #self.rows then self:set(keep, { removed = true }) end
end

function Collection:clear() return self:set {} end
function Collection:count() return #self.rows end
function Collection:find(key) local row = self.byKey[key]; return row and row.record end

function Collection:records()
    local list = {}
    for index, row in ipairs(self.rows) do list[index] = row.record end
    return list
end

function Collection:keys()
    local list = {}
    for index, row in ipairs(self.rows) do list[index] = row.key end
    return list
end

-- Maps a row DataObject (for example an event sender's DataContext) back to
-- its store record.
function Collection:resolve(object)
    for _, row in ipairs(self.rows) do
        if row.mirror.object == object then return row.record, row.key end
    end
end

function Collection:listen(fn)
    self.listeners[#self.listeners + 1] = fn
    return function()
        for index, listener in ipairs(self.listeners) do
            if listener == fn then table.remove(self.listeners, index); return end
        end
    end
end

function Collection:emit()
    for _, listener in ipairs({ table.unpack(self.listeners) }) do listener(self) end
end

function Collection:dispose()
    if self.disposed then return end
    self.disposed = true
    self.unlisten()
    for _, row in ipairs(self.rows) do
        row.mirror.object:removeFromParent()
        self.store:release(row.mirror)
    end
    self.rows, self.byKey, self.listeners, self.watchers = {}, {}, {}, {}
end

return Collection
