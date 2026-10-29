-- Builds a data scope from an XML declaration:
--
--   <DataScope Name="Inbox" Schema="Chat/Data/Chat.Schema.xml">
--     <Query Name="Rooms" Entity="ChatRoom" Operation="listRooms" Poll="15000"/>
--     <View Name="Unread" Source="Rooms" Filter="unread"/>
--     <Command Name="OpenRoom" Operation="openRoom" Parameter="Item"/>
--   </DataScope>
--
-- Names refer to a provider table (operations, filters, sorts, computed,
-- validators, conditions, handlers). Unknown names fail at load with the
-- resource and line, before any request runs.
local Xml = require "orca.data.xml"
local Schema = require "orca.data.schema"
local Scope = require "orca.data.scope"
local Declarations = {}

local function words(value)
    local list = {}
    for word in (value or ""):gmatch("[^%s,]+") do list[#list + 1] = word end
    return list
end

local function number(value) return value and tonumber(value) or nil end
local function flag(value) return value == "true" or value == "1" end

local function fail(resource, node, message)
    error(("%s:%d: %s"):format(resource, node.line or 0, message), 0)
end

local function lookup(resource, node, provider, kind, name)
    if not name then return nil end
    local group = provider[kind] or {}
    local fn = group[name]
    if type(fn) ~= "function" then
        fail(resource, node, ("<%s Name=\"%s\"> names unknown %s '%s'"):format(node.tag, node.attrs.Name or "?",
            kind:gsub("s$", ""), name))
    end
    return fn
end

local function params(node)
    local list = {}
    for _, child in ipairs(node.children) do
        if child.tag == "Param" then
            list[#list + 1] = { name = child.attrs.Name, type = (child.attrs.Type or "string"):lower(),
                default = child.attrs.Default, debounce = number(child.attrs.Debounce), bind = child.attrs.Bind,
                route = child.attrs.Route, description = child.attrs.Description, line = child.line }
        end
    end
    return list
end

-- Parses a declaration into plain tables (also used for editor metadata).
function Declarations.parse(text, resource)
    local root = Xml.parse(text, resource)
    if root.tag ~= "DataScope" then fail(resource, root, "the root element must be <DataScope>") end
    local spec = { name = root.attrs.Name or "Data", schema = root.attrs.Schema, provider = root.attrs.Provider,
        resource = resource, members = {} }
    for _, node in ipairs(root.children) do
        local a = node.attrs
        if node.tag == "Include" then
            -- Shared declarations (for example the actions every post list offers).
            local included = Declarations.load(assert(a.Path, resource .. ":" .. node.line .. ": <Include> needs Path"))
            for _, member in ipairs(included.members) do spec.members[#spec.members + 1] = member end
            goto continue
        end
        local member = { kind = node.tag, name = a.Name, line = node.line, node = node, attrs = a, params = params(node),
            resource = resource }
        if not member.name then fail(resource, node, "<" .. node.tag .. "> needs Name") end
        if node.tag == "Edit" then
            member.copy = {}
            for _, child in ipairs(node.children) do
                if child.tag == "Copy" then member.copy[child.attrs.Field] = child.attrs.From or child.attrs.Field end
            end
        end
        if node.tag == "Command" then
            member.arguments = {}
            for _, child in ipairs(node.children) do
                if child.tag == "Argument" then member.arguments[child.attrs.Name] = child.attrs.Value end
            end
        end
        if node.tag == "Media" then
            member.targets = {}
            for _, child in ipairs(node.children) do
                if child.tag == "Target" then
                    member.targets[#member.targets + 1] = { entity = child.attrs.Entity, id = child.attrs.Id,
                        path = child.attrs.Path, state = child.attrs.State, revision = child.attrs.Revision,
                        ratio = child.attrs.Ratio, line = child.line }
                end
            end
        end
        if not ({ Query = true, View = true, Command = true, Edit = true, Expose = true, Handler = true,
            Media = true })[node.tag] then
            fail(resource, node, "unknown declaration <" .. node.tag .. ">")
        end
        spec.members[#spec.members + 1] = member
        ::continue::
    end
    return spec
end

local cache = {}
function Declarations.load(path)
    if not cache[path] then cache[path] = Declarations.parse(Xml.read(path), path) end
    return cache[path]
end

-- Metadata for tools: parameters, results, availability and state names.
function Declarations.describe(spec)
    local Query, View, Command, Edit = require "orca.data.query", require "orca.data.view",
        require "orca.data.command", require "orca.data.edit"
    local states = { Query = Query.STATE, View = View.STATE, Command = Command.STATE, Edit = Edit.STATE }
    local result = { name = spec.name, schema = spec.schema, members = {} }
    for _, member in ipairs(spec.members) do
        local a = member.attrs
        local params = {}
        for _, param in ipairs(member.params) do
            params[#params + 1] = { name = param.name, type = param.type, default = param.default,
                debounce = param.debounce, bind = param.bind }
        end
        result.members[#result.members + 1] = { kind = member.kind, name = member.name, entity = a.Entity,
            source = a.Source, operation = a.Operation, params = params, state = states[member.kind],
            parameter = a.Parameter, invalidates = words(a.Invalidates), poll = number(a.Poll),
            submit = a.Submit, description = a.Description }
    end
    return result
end

-- options: provider (table), schema (Schema, overrides the declaration),
-- store, host, context, valid, identity, name.
function Declarations.build(spec, options)
    options = options or {}
    if type(spec) == "string" then spec = Declarations.load(spec) end
    local resource = spec.resource
    local provider = options.provider
    if not provider and spec.provider then provider = require(spec.provider) end
    if type(provider) == "function" then provider = provider(options.context or {}) end
    provider = provider or {}
    local schema = options.schema or (options.store and options.store.schema)
        or Schema.load(assert(spec.schema, resource .. ": <DataScope> needs Schema"))
    local scope = Scope.new { name = options.name or spec.name, schema = schema, store = options.store,
        provider = provider, computed = provider.computed, context = options.context, host = options.host,
        valid = options.valid, identity = options.identity, resource = resource, handlers = {} }
    scope.spec = spec
    local binds = {}
    for _, member in ipairs(spec.members) do
        local a, node = member.attrs, member.node
        local resource = member.resource or resource
        if member.kind == "Query" then
            if not a.Entity or not schema.entities[a.Entity] then
                fail(resource, node, ("query %s names unknown entity '%s'"):format(member.name, tostring(a.Entity)))
            end
            scope:query { name = member.name, entity = a.Entity, params = member.params,
                operation = lookup(resource, node, provider, "operations", a.Operation or member.name),
                pageSize = number(a.PageSize), poll = number(a.Poll), require = words(a.Require),
                pollWhen = lookup(resource, node, provider, "conditions", a.PollWhen),
                autoLoad = a.AutoLoad ~= "false", prefetch = flag(a.Prefetch),
                order = a.Order, window = a.Window, prefetchEdge = a.PrefetchEdge,
                cached = lookup(resource, node, provider, "caches", a.Cached),
                loaded = lookup(resource, node, provider, "hooks", a.Loaded),
                arrange = lookup(resource, node, provider, "hooks", a.Arrange) }
        elseif member.kind == "View" then
            if not scope:member(a.Source or "") then
                fail(resource, node, ("view %s names unknown source '%s'"):format(member.name, tostring(a.Source)))
            end
            scope:view { name = member.name, source = a.Source, params = member.params,
                filter = lookup(resource, node, provider, "filters", a.Filter),
                sort = lookup(resource, node, provider, "sorts", a.Sort), selection = a.Selection }
        elseif member.kind == "Command" then
            local arguments = member.arguments
            local operation = a.Operation and lookup(resource, node, provider, "operations", a.Operation)
            local target = a.Target and a.Action
            if target then
                local query = scope:member(a.Target)
                if not query or not query[a.Action:sub(1, 1):lower() .. a.Action:sub(2)] then
                    fail(resource, node, ("command %s targets unknown action %s.%s"):format(member.name, a.Target, a.Action))
                end
                local method = a.Action:sub(1, 1):lower() .. a.Action:sub(2)
                operation = function() return query[method](query) or true end
            end
            local parameter = a.Parameter
            local base = operation
            if base and (next(arguments) or parameter == "Item") then
                operation = function(params, context, command)
                    if parameter == "Item" and not params.item then
                        return nil, { code = "missing_item", message = "Nothing is selected." }
                    end
                    for key, value in pairs(arguments) do if params[key] == nil then params[key] = value end end
                    return base(params, context, command)
                end
            end
            scope:command { name = member.name, operation = operation, available = a.Available ~= "false",
                canExecute = lookup(resource, node, provider, "conditions", a.CanExecute),
                concurrency = a.Concurrency, invalidates = words(a.Invalidates), sync = flag(a.Sync) or target ~= nil,
                succeeded = lookup(resource, node, provider, "hooks", a.Succeeded),
                failed = lookup(resource, node, provider, "hooks", a.Failed) }
        elseif member.kind == "Edit" then
            if not a.Entity or not schema.entities[a.Entity] then
                fail(resource, node, ("edit %s names unknown entity '%s'"):format(member.name, tostring(a.Entity)))
            end
            scope:edit { name = member.name, entity = a.Entity, submit = a.Submit, copy = member.copy,
                validate = lookup(resource, node, provider, "validators", a.Validate),
                canSubmit = lookup(resource, node, provider, "conditions", a.CanSubmit),
                submitted = lookup(resource, node, provider, "hooks", a.Submitted),
                clearOnSuccess = a.ClearOnSuccess ~= "false" }
        elseif member.kind == "Media" then
            for _, target in ipairs(member.targets) do
                local entity = schema.entities[target.entity or ""]
                if not entity or not entity.byName[target.id or ""] then
                    fail(resource, node, ("media %s target names unknown column %s.%s"):format(member.name,
                        tostring(target.entity), tostring(target.id)))
                end
            end
            scope:media { name = member.name, targets = member.targets, maxActive = number(a.MaxActive),
                stages = a.Stages and words(a.Stages) or nil,
                lookup = lookup(resource, node, provider, "media", a.Lookup),
                fetch = lookup(resource, node, provider, "media", a.Fetch or "fetch"),
                store = lookup(resource, node, provider, "media", a.Store or "store"),
                cached = lookup(resource, node, provider, "media", a.Cached),
                validate = lookup(resource, node, provider, "media", a.Validate),
                missing = lookup(resource, node, provider, "media", a.Missing),
                preview = lookup(resource, node, provider, "media", a.Preview),
                discard = lookup(resource, node, provider, "media", a.Discard),
                repaint = provider.repaint }
        elseif member.kind == "Handler" then
            scope.handlers[member.name] = lookup(resource, node, provider, "handlers", a.Function or member.name)
        elseif member.kind == "Expose" then
            if a.Entity and not schema.entities[a.Entity] then
                fail(resource, node, ("expose %s names unknown entity '%s'"):format(member.name, a.Entity))
            end
            scope.declaredExposures = scope.declaredExposures or {}
            scope.declaredExposures[member.name] = a.Entity or true
            -- A route parameter names the record (Key="RoomId").
            if a.Key then
                if not a.Entity then fail(resource, node, ("expose %s with Key needs Entity"):format(member.name)) end
                scope.routeExposures = scope.routeExposures or {}
                scope.routeExposures[#scope.routeExposures + 1] = { name = member.name, entity = a.Entity, key = a.Key }
            end
        end
        for _, param in ipairs(member.params) do
            if param.bind then binds[#binds + 1] = { member = member, param = param } end
            if param.route then
                scope.routeParams = scope.routeParams or {}
                scope.routeParams[#scope.routeParams + 1] = { member = member.name, param = param.name, route = param.route }
            end
        end
        -- An edit copy of a record the route exposes (Begin="Room").
        if member.kind == "Edit" and a.Begin then
            scope.beginEdits = scope.beginEdits or {}
            scope.beginEdits[#scope.beginEdits + 1] = { edit = member.name, record = a.Begin }
        end
    end
    -- Edits name the command they submit; validate once everything exists.
    for _, member in ipairs(spec.members) do
        local resource = member.resource or resource
        if member.kind == "Edit" and member.attrs.Submit and not scope:member(member.attrs.Submit) then
            fail(resource, member.node, ("edit %s submits unknown command '%s'"):format(member.name, member.attrs.Submit))
        end
        for _, name in ipairs(words(member.attrs.Invalidates)) do
            if not scope:member(name) then
                fail(resource, member.node, ("command %s invalidates unknown query '%s'"):format(member.name, name))
            end
        end
    end
    -- Master-detail: Bind="Rooms.Selected.Id" feeds a parameter from a view's
    -- selection, so the dependent query follows it without app code.
    for _, item in ipairs(binds) do
        local resource = item.member.resource or resource
        local source, column = item.param.bind:match("^([%w_]+)%.Selected%.([%w_]+)$")
        local view = source and scope:member(source)
        if not view or not view.select then
            fail(resource, item.member.node, ("parameter %s binds unknown selection '%s'"):format(item.param.name, item.param.bind))
        end
        local target = scope:member(item.member.name)
        local function apply()
            local selected = view.selected
            target:set(item.param.name, selected and selected.values[column] or nil)
        end
        view.selectionListeners = view.selectionListeners or {}
        view.selectionListeners[#view.selectionListeners + 1] = apply
        apply()
    end
    return scope
end

return Declarations
