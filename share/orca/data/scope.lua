-- A data scope groups the queries, views, commands and drafts one owner
-- (an app session, a page) uses. Its root DataObject is the DataContext of
-- the views that present it; bindings name members by path, such as
-- {Binding DataContext/Rooms/Items} or {Binding DataContext/Send/CanExecute}.
-- The scope never holds UI nodes; views observe its records.
local core = require "orca.core"
local Schema = require "orca.data.schema"
local Store = require "orca.data.store"
local Query = require "orca.data.query"
local View = require "orca.data.view"
local Command = require "orca.data.command"
local Edit = require "orca.data.edit"
local Dispatch = require "orca.data.dispatch"
local Scope = {}
Scope.__index = Scope

local STATE_TYPES = {
    DataQuery = Query.STATE, DataView = View.STATE, DataCommand = Command.STATE, DataEdit = Edit.STATE,
}
local STRING_FIELDS = { ErrorMessage = true, ErrorCode = true, SelectedKey = true, Reason = true,
    MoreErrorMessage = true, MoreErrorCode = true }
local INT_FIELDS = { Count = true }
local stateEntities = {}

local function stateEntity(kind)
    if not stateEntities[kind] then
        local columns = {}
        for _, name in ipairs(assert(STATE_TYPES[kind], "Unknown state kind " .. tostring(kind))) do
            columns[#columns + 1] = { name, STRING_FIELDS[name] and "string" or INT_FIELDS[name] and "int" or "bool" }
        end
        stateEntities[kind] = Schema.define(kind, columns)
    end
    return stateEntities[kind]
end

-- The default host runs operations in ORCA coroutines and schedules with one
-- native Timer per pending callback. Tests inject synchronous hosts.
local function defaultHost()
    local orca = require "orca"
    local host = {}
    function host.run(fn) orca.async(fn) end
    function host.after(ms, fn)
        local owner = core.Timer { Name = "DataScopeTimer" }
        local handle, id, canceled = {}, nil, false
        function handle:cancel()
            if canceled then return end
            canceled = true
            if id then owner:cancelTimer(id) end
            owner.Tick = nil
        end
        owner.Tick = function(_, args)
            if canceled or args.timerId ~= id then return end
            handle:cancel()
            fn()
        end
        id = owner:setTimer(ms)
        handle.owner = owner
        return handle
    end
    function host.report(err, stage)
        local ok, async = pcall(require, "orca.core.async")
        if ok then async.report_error((stage and stage .. ": " or "") .. tostring(err))
        else io.stderr:write(tostring(err) .. "\n") end
    end
    return host
end

Scope.defaultHost = defaultHost

local serial = 0

-- options: name, schema, store (or scope/computed/context to create one),
-- provider (operations table), context, host { run, after, report },
-- valid() for session checks, resource (for diagnostics).
function Scope.new(options)
    serial = serial + 1
    local self = setmetatable({ name = options.name or "Data", resource = options.resource,
        members = {}, order = {}, epoch = 0, handlers = options.handlers or {}, serial = serial }, Scope)
    self.host = options.host or defaultHost()
    self.context = options.context or {}
    self.store = options.store or Store.new { schema = assert(options.schema, "Scope.new needs a schema"),
        scope = options.identity, computed = options.computed, context = self.context, capacity = options.capacity }
    self.context = self.store.context
    self.entityPrefix = options.entityPrefix or self.name
    self.validator = options.valid
    self.root = core.DataObject { Name = "DataScope" .. serial }
    self.provider = options.provider or {}
    Dispatch.register(self)
    return self
end

-- Presents the scope in `node`: its DataContext becomes the scope root and
-- named event handlers in the subtree resolve to scope members. `handlers`
-- adds app functions(scope, params) for behavior no declaration covers.
function Scope:attach(node, handlers)
    for name, fn in pairs(handlers or {}) do self.handlers[name] = fn end
    node.DataContext = self.root
    Dispatch.pending = self
    local ok, err = pcall(node.loadController, node, "orca.data.controller")
    Dispatch.pending = nil
    if not ok then error(err, 2) end
    -- Resolve bound collections now, so the page is complete before its first paint.
    core.runAllPrograms(node)
    return node
end

-- A controller table for an app's own controller module: explicit
-- functions first, then scope members by name.
function Scope:controller(functions) return Dispatch.controller(self, functions) end

function Scope:stateRecord(kind, name)
    if self.members[name] then
        error(("%s: '%s' is declared twice in scope %s"):format(self.resource or "data", name, self.name), 0)
    end
    local object = stateEntity(kind):instantiate()
    object.Name = name
    self.root:addChild(object)
    return object
end

-- Sets fields that changed; bindings only re-run for real changes.
function Scope:write(object, values)
    for name, value in pairs(values) do
        if object[name] ~= value then object[name] = value end
    end
end

local function add(self, member)
    if self.members[member.name] then
        error(("%s: '%s' is declared twice in scope %s"):format(self.resource or "data", member.name, self.name), 0)
    end
    self.members[member.name] = member
    self.order[#self.order + 1] = member
    return member
end

function Scope:query(def) return add(self, Query.new(self, def)) end
function Scope:view(def)
    if type(def.source) == "string" then def.source = self:member(def.source, "source of " .. def.name) end
    return add(self, View.new(self, def))
end
function Scope:command(def) return add(self, Command.new(self, def)) end
function Scope:edit(def) return add(self, Edit.new(self, def)) end
function Scope:media(def) return add(self, require("orca.data.media").new(self, def)) end

function Scope:member(name, purpose)
    local member = self.members[name]
    if not member and purpose then
        error(("%s: unknown %s '%s' in scope %s"):format(self.resource or "data", purpose, tostring(name), self.name), 2)
    end
    return member
end

-- Exposes one record (a route parameter, an open post) under `name` so a
-- detail page binds {Binding DataContext/Post/Title}. Replacing it keeps
-- bindings resolving through the same path.
function Scope:expose(name, record)
    local current = self.exposed and self.exposed[name]
    if current and current.record == record then return current.object end
    self.exposed = self.exposed or {}
    if current then
        current.object:removeFromParent()
        self.store:release(current)
        self.exposed[name] = nil
    end
    if record then
        local mirror = self.store:mirror(record)
        mirror.object.Name = name
        self.root:addChild(mirror.object)
        self.exposed[name] = mirror
        return mirror.object
    end
end

-- A subtree that presents one exposed record, as a list row presents its
-- item: its bindings read the record's fields (DataContext/Title) and its
-- handlers receive the record. Expose the record first; the scope keeps no
-- reference to the node, which may be destroyed with its page at any time.
function Scope:present(node, name)
    local mirror = assert(self.exposed and self.exposed[name], "present: nothing is exposed as " .. tostring(name))
    node.DataContext = mirror.object
    return node
end

function Scope:exposedRecord(name)
    local mirror = self.exposed and self.exposed[name]
    return mirror and mirror.record
end

function Scope:invalidate(name)
    local member = self:member(name, "query to invalidate")
    if member.invalidate then return member:invalidate() end
end

-- Commands re-check availability when scope state changes.
function Scope:changed()
    for _, member in ipairs(self.order) do
        if member.refresh and getmetatable(member) == Command then member:refresh() end
    end
end

-- A token identifies the scope generation a request started in; results for
-- an old token (sign-out, account switch, disposal) are discarded.
function Scope:token() return self.epoch end
function Scope:valid(token)
    return not self.disposed and token == self.epoch and (not self.validator or self.validator(self))
end

-- Drops in-flight results without disposing members (for example when the
-- active account changes and the scope is rebuilt for the new one).
function Scope:invalidateRequests() self.epoch = self.epoch + 1 end

function Scope:run(fn) return self.host.run(fn) end
function Scope:after(ms, fn) return self.host.after(ms, fn) end
function Scope:report(err, stage) if self.host.report then self.host.report(err, stage) end end

-- Activates polling/refresh for the scope's queries (a page became visible).
function Scope:activate()
    self.active = true
    for _, member in ipairs(self.order) do if member.activate then member:activate() end end
end

function Scope:deactivate()
    self.active = false
    for _, member in ipairs(self.order) do if member.deactivate then member:deactivate() end end
end

function Scope:setForeground(active)
    self.foreground = active
    for _, member in ipairs(self.order) do if member.setForeground then member:setForeground(active) end end
end

-- Maps an event sender to the record it presents: the nearest DataContext
-- that is a row, selection or exposed record of this scope.
function Scope:resolve(node)
    while node do
        local context = node.DataContext
        if context then
            local record = self.store:recordFor(context)
            if record then return record, context end
        end
        node = node:getParent()
    end
end

function Scope:dispose()
    if self.disposed then return end
    self.disposed = true
    self.epoch = self.epoch + 1
    for index = #self.order, 1, -1 do
        local member = self.order[index]
        if member.dispose then member:dispose() end
    end
    if self.exposed then
        for name in pairs(self.exposed) do self:expose(name, nil) end
    end
    self.root:removeFromParent()
end

return Scope
