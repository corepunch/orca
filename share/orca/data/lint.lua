-- Binding diagnostics without running a page: checks the {Binding
-- DataContext/...} paths and event handler names of an XML page against its
-- scope declaration and schema, and follows ItemTemplates into the entity
-- their rows present. Each problem names the resource, line, node,
-- property, path and reason, for tools and tests.
--
--   local problems = Lint.page("Chat/Pages/Inbox.xml", "Chat/Data/Inbox.xml")
--   local problems = Lint.page("Octopus/Pages/Post.xml", "Octopus/Data/Post.xml",
--       { present = { PostDetail = "Post" } })
local Xml = require "orca.data.xml"
local Schema = require "orca.data.schema"
local Declarations = require "orca.data.declarations"
local Lint = {}

local STATE = {
    Query = require("orca.data.query").STATE, View = require("orca.data.view").STATE,
    Command = require("orca.data.command").STATE, Edit = require("orca.data.edit").STATE,
}
local ACTIONS = { Load = "Query", Refresh = "Query", Retry = true, LoadMore = "Query", Invalidate = "Query",
    Submit = "Edit", Cancel = "Edit", Clear = "Edit", Select = "View", Begin = "Edit", Reset = "Query" }
-- Names a binding may use on any node, whatever its data.
local OBJECT = { Name = true, id = true }

local function set(list)
    local result = {}
    for _, name in ipairs(list or {}) do result[name] = true end
    return result
end

local function file(path)
    if path:match("%.xml$") then return path end
    return path .. ".xml"
end

-- The shape a scope presents: members by name, with their entities.
local function shape(spec, schema)
    local members = {}
    for _, member in ipairs(spec.members) do
        local a = member.attrs
        local entry = { kind = member.kind, name = member.name, attrs = a, params = {} }
        for _, param in ipairs(member.params) do entry.params[param.name] = true end
        if member.kind == "Query" then entry.entity = a.Entity
        elseif member.kind == "Edit" or member.kind == "Expose" then entry.entity = a.Entity end
        members[member.name] = entry
    end
    -- Views take their source's entity.
    for _, entry in pairs(members) do
        local source, guard = entry, 0
        while source and source.kind == "View" and guard < 16 do
            source, guard = members[source.attrs.Source], guard + 1
        end
        if entry.kind == "View" then entry.entity = source and source.entity end
    end
    return { kind = "scope", members = members, schema = schema }
end

local function entityContext(schema, name)
    local entity = name and schema.entities[name]
    return entity and { kind = "entity", entity = entity, schema = schema } or nil
end

-- Resolves `path` (after DataContext/) against a context. Returns what it
-- names ({ kind = "value" | "collection" | "record", entity }) or nil, reason.
local function resolve(context, path)
    local parts = {}
    for part in path:gmatch("[^/]+") do parts[#parts + 1] = part end
    local current = context
    local index = 1
    while index <= #parts do
        local part = parts[index]
        if not current then return nil, "nothing to resolve " .. part .. " against" end
        if current.kind == "scope" then
            local member = current.members[part]
            if not member then return nil, "the scope has no member " .. part end
            local nextPart = parts[index + 1]
            if member.kind == "Expose" then
                current = entityContext(current.schema, member.entity) or { kind = "any" }
            elseif member.kind == "Edit" then
                if nextPart == "Status" then
                    if not parts[index + 2] or set(STATE.Edit)[parts[index + 2]] then return { kind = "value" } end
                    return nil, "Edit state has no field " .. parts[index + 2]
                elseif nextPart == "Errors" then
                    current = entityContext(current.schema, member.entity)
                    index = index + 1
                else
                    current = entityContext(current.schema, member.entity)
                end
            else
                if not nextPart then return { kind = "record" } end
                if nextPart == "Items" and (member.kind == "Query" or member.kind == "View") then
                    if parts[index + 2] then return nil, "Items is a collection; bind a template to its rows" end
                    return { kind = "collection", entity = member.entity }
                end
                if nextPart == "Params" then
                    local param = parts[index + 2]
                    if param and not member.params[param] then return nil, member.name .. " has no parameter " .. param end
                    return { kind = "value" }
                end
                if nextPart == "Selected" and member.kind == "View" then
                    current = entityContext(current.schema, member.entity) or { kind = "any" }
                    index = index + 1
                elseif set(STATE[member.kind])[nextPart] then
                    if parts[index + 2] then return nil, nextPart .. " is a value" end
                    return { kind = "value" }
                else
                    return nil, ("%s %s has no state %s"):format(member.kind, member.name, nextPart)
                end
            end
        elseif current.kind == "entity" then
            local entity = current.entity
            if entity.byName[part] or OBJECT[part] then
                if parts[index + 1] then return nil, part .. " is a value" end
                return { kind = "value" }
            end
            local relation = entity.relationsByName[part]
            if not relation then return nil, ("%s has no column or relation %s"):format(entity.name, part) end
            if relation.many then
                if parts[index + 1] then return nil, part .. " is a collection" end
                return { kind = "collection", entity = relation.entity }
            end
            current = entityContext(current.schema, relation.entity)
        else
            return { kind = "any" }
        end
        index = index + 1
    end
    if current and current.kind == "entity" then return { kind = "record", entity = current.entity.name } end
    return { kind = "record" }
end

local function bindings(value)
    local paths = {}
    for path in value:gmatch("{Binding%s+DataContext/([^,}%s]+)") do paths[#paths + 1] = path end
    for path in value:gmatch("{DataContext/([^,}%s]+)") do paths[#paths + 1] = path end
    return paths
end

local EVENTS = { Click = true, Submit = true, SelectionChanged = true, BackRequested = true, HeaderClick = true }

local function handlerKnown(spec, members, name)
    for _, member in ipairs(spec.members) do
        if member.kind == "Handler" and member.name == name then return true end
    end
    local target, action = name:match("^([%w_]+)%.([%w_]+)$")
    if target then
        local member = members[target]
        if not member then return false, "no member " .. target end
        local kind = ACTIONS[action]
        if not kind then return false, "unknown action " .. action end
        if kind ~= true and member.kind ~= kind and not (action == "Retry" and member.kind == "Media") then
            return false, ("%s is a %s; %s applies to %s members"):format(target, member.kind, action, kind)
        end
        return true
    end
    local member = members[name]
    if member and (member.kind == "Command" or member.kind == "Edit") then return true end
    return false, "no handler, command or edit named " .. name
end

local checkTemplate

local function walk(node, context, state)
    local presented = state.present[node.attrs.Name or ""]
    if presented then
        local member = state.scope.members[presented]
        context = member and entityContext(state.schema, member.entity) or context
    end
    local collections = {}
    for _, name in ipairs(node.order or {}) do
        local value = node.attrs[name]
        for _, path in ipairs(bindings(value)) do
            local result, reason = resolve(context, path)
            if not result then
                state.problems[#state.problems + 1] = { resource = state.resource, line = node.line, node = node.attrs.Name or node.tag,
                    property = name, path = "DataContext/" .. path, reason = reason }
            elseif name == "ItemsSource" then
                collections[#collections + 1] = result
            end
        end
        if EVENTS[name] and not value:match("^{") then
            local ok, reason = handlerKnown(state.spec, state.scope.members, value)
            if not ok then
                state.problems[#state.problems + 1] = { resource = state.resource, line = node.line, node = node.attrs.Name or node.tag,
                    property = name, path = value, reason = reason }
            end
        end
    end
    if node.tag == "BindingExpression" and node.text then
        for _, path in ipairs(bindings(node.text)) do
            local result, reason = resolve(context, path)
            if not result then
                state.problems[#state.problems + 1] = { resource = state.resource, line = node.line, node = node.tag,
                    property = node.attrs.Target, path = "DataContext/" .. path, reason = reason }
            end
        end
    end
    -- Rows of a bound collection present its entity.
    local template = node.attrs.ItemTemplate
    if template and #collections > 0 and collections[1].entity then
        checkTemplate(template, entityContext(state.schema, collections[1].entity), state)
    end
    for _, child in ipairs(node.children) do walk(child, context, state) end
end

checkTemplate = function(path, context, state)
    if not context then return end
    local key = path .. "@" .. context.entity.name
    if state.seen[key] then return end
    state.seen[key] = true
    local resource = file(path)
    local root = Xml.parse(Xml.read(resource), resource)
    walk(root, context, { resource = resource, spec = state.spec, scope = state.scope, schema = state.schema,
        present = {}, problems = state.problems, seen = state.seen })
end

-- Checks a page against a scope declaration (a path or a parsed spec).
-- options.present maps node names to exposed records they present (as
-- Scope:present does at run time).
function Lint.page(page, declaration, options)
    local resource = file(page)
    return Lint.source(Xml.read(resource), resource, declaration, options)
end

-- The same check over page text (an editor's unsaved page).
function Lint.source(text, resource, declaration, options)
    options = options or {}
    local spec = type(declaration) == "string" and Declarations.load(declaration) or declaration
    local schema = Schema.load(spec.schema)
    local root = Xml.parse(text, resource)
    local scope = shape(spec, schema)
    local state = { resource = resource, spec = spec, scope = scope, schema = schema, present = options.present or {},
        problems = {}, seen = {} }
    walk(root, scope, state)
    return state.problems
end

-- One line per problem: resource:line node.property path: reason.
function Lint.format(problems)
    local lines = {}
    for _, problem in ipairs(problems) do
        lines[#lines + 1] = ("%s:%d: %s.%s %s: %s"):format(problem.resource, problem.line or 0, problem.node,
            problem.property or "?", problem.path, problem.reason)
    end
    return table.concat(lines, "\n")
end

return Lint
