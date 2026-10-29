-- One identity map per data scope (provider, account, session). A record is
-- the committed value of one entity key; native DataObject mirrors present it
-- in collections and detail contexts. Updating a record updates every mirror
-- in place, so bound rows are never recreated for a field change.
local Schema = require "orca.data.schema"
local Collection
local Store = {}
Store.__index = Store

-- `options`: schema, scope (identity label), computed (name -> fn), context,
-- capacity (unreferenced records kept for revisits).
function Store.new(options)
    assert(options and options.schema, "Store.new needs a schema")
    return setmetatable({
        schema = options.schema, scope = options.scope or "default", computed = options.computed or {},
        context = options.context or {}, capacity = options.capacity or 500,
        records = {}, dependents = {}, listeners = {}, flushers = {}, idle = {}, idleCount = 0, clock = 0,
        byObject = {},
        batchDepth = 0, pending = nil,
    }, Store)
end

local function bucket(map, name)
    local value = map[name]
    if not value then value = {}; map[name] = value end
    return value
end

function Store:get(entity, key)
    local records = self.records[entity]
    return records and records[key]
end

function Store:entity(name) return self.schema:entity(name) end

function Store:listen(fn)
    self.listeners[#self.listeners + 1] = fn
    return function()
        for index, listener in ipairs(self.listeners) do
            if listener == fn then table.remove(self.listeners, index); return end
        end
    end
end

-- Called once after each delivered group of record notifications, so views
-- re-filter once per batch instead of once per record.
function Store:onFlush(fn)
    self.flushers[#self.flushers + 1] = fn
    return function()
        for index, flusher in ipairs(self.flushers) do
            if flusher == fn then table.remove(self.flushers, index); return end
        end
    end
end

local function flush(self)
    for _, flusher in ipairs({ table.unpack(self.flushers) }) do flusher() end
end

local function notify(self, record, changes)
    if self.batchDepth > 0 then
        self.pending = self.pending or { order = {}, changes = {} }
        local known = self.pending.changes[record]
        if not known then
            known = {}
            self.pending.changes[record] = known
            self.pending.order[#self.pending.order + 1] = record
        end
        for key, value in pairs(changes) do known[key] = value end
        return
    end
    for _, listener in ipairs({ table.unpack(self.listeners) }) do listener(record, changes) end
    if self.batchDepth == 0 and not self.flushing then
        self.flushing = true
        local ok, err = pcall(flush, self)
        self.flushing = false
        if not ok then error(err, 0) end
    end
end

-- Coalesces notifications: listeners see each changed record once.
function Store:batch(fn)
    self.batchDepth = self.batchDepth + 1
    local ok, a, b, c = xpcall(fn, debug.traceback)
    self.batchDepth = self.batchDepth - 1
    if self.batchDepth == 0 and self.pending then
        local pending = self.pending
        self.pending = nil
        self.flushing = true
        local ok, err = pcall(function()
            for _, record in ipairs(pending.order) do
                for _, listener in ipairs({ table.unpack(self.listeners) }) do listener(record, pending.changes[record]) end
            end
            flush(self)
        end)
        self.flushing = false
        if not ok then error(err, 0) end
    end
    if not ok then error(a, 0) end
    return a, b, c
end

local function writeMirror(self, mirror, names)
    local record, object = mirror.record, mirror.object
    for _, column in ipairs(record.entity.columns) do
        if not names or names[column.name] then
            local value = record.values[column.name]
            if value == nil then value = column.default end
            if mirror.written[column.name] ~= value then
                object[column.name] = value
                mirror.written[column.name] = value
            end
        end
    end
    for name, children in pairs(mirror.children) do
        if not names or names[name] then children:set(record.children[name] or {}) end
    end
end

-- Recomputes projections and computed columns, then updates mirrors and
-- dependents with only the fields that changed.
local function recompute(self, record, depth)
    if record.deleted then return {} end
    depth = depth or 0
    if depth > 8 then error("Data relation cycle near " .. record.entity.name .. ":" .. tostring(record.key), 0) end
    local entity, values = record.entity, {}
    for name, value in pairs(record.base) do values[name] = value end
    for name, value in pairs(record.locals) do values[name] = value end
    -- To-one relations resolve through their key column, possibly to a record
    -- that has not been loaded yet; it fills in when that record arrives.
    for _, relation in ipairs(entity.relations) do
        if not relation.many then
            local key = values[relation.key]
            local previous = record.relatedKeys[relation.name]
            if previous ~= key then
                if previous ~= nil then
                    local set = self.dependents[relation.entity] and self.dependents[relation.entity][previous]
                    if set then set[record] = nil end
                end
                if key ~= nil then bucket(bucket(self.dependents, relation.entity), key)[record] = true end
                record.relatedKeys[relation.name] = key
            end
        end
    end
    for _, column in ipairs(entity.columns) do
        if column.from then
            local relation = entity.relationsByName[column.relation]
            local target = values[relation.key] ~= nil and self:get(relation.entity, values[relation.key])
            values[column.name] = target and not target.deleted and target.values[column.field] or nil
        end
    end
    for _, column in ipairs(entity.columns) do
        if column.computed then
            local fn = self.computed[column.computed]
            if not fn then
                error(("%s:%d: no computed function '%s' for %s.%s"):format(entity.resource, column.line,
                    column.computed, entity.name, column.name), 0)
            end
            values[column.name] = Schema.convert(column.type, fn(values, record, self.context))
        end
    end
    local changes = {}
    for _, column in ipairs(entity.columns) do
        if values[column.name] ~= record.values[column.name] then changes[column.name] = true end
    end
    for name in pairs(record.childChanges or {}) do changes[name] = true end
    record.childChanges = nil
    record.values = values
    if next(changes) then
        for mirror in pairs(record.mirrors) do writeMirror(self, mirror, changes) end
        notify(self, record, changes)
        local set = self.dependents[entity.name] and self.dependents[entity.name][record.key]
        if set then
            for dependent in pairs(set) do recompute(self, dependent, depth + 1) end
        end
    end
    return changes
end

local function forget(self, record)
    if self.idle[record] then self.idle[record], self.idleCount = nil, self.idleCount - 1 end
end

local function touch(self, record)
    if record.refs > 0 then forget(self, record); return end
    self.clock = self.clock + 1
    if not self.idle[record] then self.idleCount = self.idleCount + 1 end
    self.idle[record] = self.clock
end

-- Inserts or updates a record from a service table. Options: partial (missing
-- fields keep their value), key (when the entity's key is computed elsewhere),
-- locals (client-only column values).
function Store:put(entityName, raw, options)
    options = options or {}
    local entity = self:entity(entityName)
    local mapped, present = entity:map(raw or {}, options.partial)
    local key = options.key
    if key == nil and entity.key then
        key = mapped[entity.key]
        if key == nil and options.partial then
            local column = entity.byName[entity.key]
            key = Schema.convert(column.type, Schema.readPath(raw, column.source or column.name))
        end
    end
    if key == nil then error(("Record for %s has no key"):format(entity.name), 2) end
    local records = bucket(self.records, entity.name)
    local record = records[key]
    if not record or record.deleted then
        record = { entity = entity, key = key, store = self, base = {}, locals = {}, values = {}, raw = {},
            mirrors = {}, refs = 0, relatedKeys = {}, children = {} }
        records[key] = record
        for _, column in ipairs(entity.columns) do
            if column.isLocal then record.locals[column.name] = column.default end
        end
    end
    if options.partial then
        for field, value in pairs(raw or {}) do record.raw[field] = value end
    else
        record.raw = raw or {}
    end
    if mapped[entity.key or ""] == nil and entity.key and present[entity.key] then mapped[entity.key] = key end
    for name in pairs(present) do record.base[name] = mapped[name] end
    if entity.key and record.base[entity.key] == nil then record.base[entity.key] = key end
    for name, value in pairs(options.locals or {}) do
        local column = entity.byName[name]
        if not column or not column.isLocal then error(("%s.%s is not a local column"):format(entity.name, name), 2) end
        record.locals[name] = Schema.convert(column.type, value)
    end
    -- Embedded to-many relations: tags, attachments, a message's photos.
    for _, relation in ipairs(entity.relations) do
        if relation.many then
            local list = Schema.readPath(raw or {}, relation.source)
            if list ~= nil or not options.partial then
                local children = {}
                for index, item in ipairs(type(list) == "table" and list or {}) do
                    local child = type(item) == "table" and item or { value = item, index = index }
                    children[#children + 1] = self:put(relation.entity, child)
                end
                record.children[relation.name] = children
                record.childChanges = record.childChanges or {}
                record.childChanges[relation.name] = true
            end
        end
    end
    recompute(self, record)
    touch(self, record)
    return record
end

-- Applies a batch of service rows and returns their records in order.
function Store:putAll(entityName, rows, options)
    local records = {}
    self:batch(function()
        for index, row in ipairs(rows or {}) do records[index] = self:put(entityName, row, options) end
    end)
    return records
end

-- Changes client-only (Local) columns, such as a read marker or a pin.
function Store:update(record, locals)
    for name, value in pairs(locals) do
        local column = record.entity.byName[name]
        if not column or not column.isLocal then error(("%s.%s is not a local column"):format(record.entity.name, name), 2) end
        record.locals[name] = Schema.convert(column.type, value)
    end
    return recompute(self, record)
end

-- Context values feed computed columns (the signed-in user, a clock).
function Store:setContext(values)
    for key, value in pairs(values) do self.context[key] = value end
    self:batch(function()
        for _, records in pairs(self.records) do
            for _, record in pairs(records) do recompute(self, record) end
        end
    end)
end

function Store:remove(entityName, key)
    local record = self:get(entityName, key)
    if not record or record.deleted then return nil end
    record.deleted = true
    self.records[entityName][key] = nil
    forget(self, record)
    notify(self, record, { deleted = true })
    local set = self.dependents[entityName] and self.dependents[entityName][key]
    if set then
        for dependent in pairs(set) do recompute(self, dependent) end
    end
    return record
end

-- A native presentation of `record`, kept current until released.
function Store:mirror(record)
    Collection = Collection or require "orca.data.collection"
    local mirror = { record = record, object = record.entity:instantiate(), written = {}, children = {} }
    mirror.object.Name = tostring(record.key)
    for _, relation in ipairs(record.entity.relations) do
        if relation.many then
            mirror.children[relation.name] = Collection.new(self, { name = relation.name, parent = mirror.object })
        end
    end
    record.mirrors[mirror] = true
    mirror.id = mirror.object:getUniqueID()
    self.byObject[mirror.id] = mirror
    record.refs = record.refs + 1
    forget(self, record)
    writeMirror(self, mirror)
    return mirror
end

function Store:release(mirror)
    local record = mirror.record
    if not record.mirrors[mirror] then return end
    record.mirrors[mirror] = nil
    self.byObject[mirror.id] = nil
    record.refs = record.refs - 1
    for _, children in pairs(mirror.children) do children:dispose() end
    touch(self, record)
end

-- The record a native mirror presents (an event sender's DataContext).
function Store:recordFor(object)
    local mirror = object and object.getUniqueID and self.byObject[object:getUniqueID()]
    return mirror and mirror.record, mirror
end

-- Keeps a record (a selection or an open detail page) while unreferenced.
function Store:retain(record) record.refs = record.refs + 1; forget(self, record); return record end
function Store:unretain(record) record.refs = math.max(0, record.refs - 1); touch(self, record) end

-- Drops the oldest unreferenced records beyond capacity. Records still shown
-- or retained are never evicted.
function Store:collect(capacity)
    capacity = capacity or self.capacity
    if self.idleCount <= capacity then return 0 end
    local list = {}
    for record, stamp in pairs(self.idle) do list[#list + 1] = { record = record, stamp = stamp } end
    table.sort(list, function(a, b) return a.stamp < b.stamp end)
    local dropped = 0
    for index = 1, #list - capacity do
        local record = list[index].record
        if record.refs == 0 then
            forget(self, record)
            if self.records[record.entity.name][record.key] == record then
                self.records[record.entity.name][record.key] = nil
            end
            dropped = dropped + 1
        end
    end
    return dropped
end

function Store:count(entityName)
    local count = 0
    for _ in pairs(self.records[entityName] or {}) do count = count + 1 end
    return count
end

function Store:dispose()
    self.disposed = true
    self.records, self.dependents, self.listeners, self.flushers, self.idle, self.idleCount = {}, {}, {}, {}, {}, 0
    self.byObject = {}
end

return Store
