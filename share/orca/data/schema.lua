-- Application schemas: entity identity, typed columns, service-field mapping,
-- relationships and editor metadata. Native typed DataObject classes are
-- registered from the same file; the native parser ignores the metadata
-- attributes this module reads.
local Xml = require "orca.data.xml"
local Schema = {}
Schema.__index = Schema
local Entity = {}
Entity.__index = Entity

local TYPES = { string = true, int = true, float = true, bool = true }
-- Native object properties a record column cannot shadow.
local RESERVED = { Name = true, id = true, class = true, StyleClass = true, DataContext = true }
local DEFAULTS = { string = "", int = 0, float = 0, bool = false }
-- An explicit null in a partial update; a missing field leaves the value alone.
Schema.null = setmetatable({}, { __tostring = function() return "null" end })
local registered = {}   -- entity name -> { signature, prototype }
local loaded = {}       -- resource path -> schema

local function flag(value) return value == "true" or value == "1" or value == "yes" end
local function words(value)
    local list = {}
    for word in (value or ""):gmatch("[^%s,]+") do list[#list + 1] = word end
    return list
end

function Schema.convert(kind, value)
    if value == nil then return nil end
    if kind == "string" then return type(value) == "string" and value or tostring(value) end
    if kind == "bool" then return value == true or value == "true" or value == 1 end
    local number = tonumber(value)
    if number == nil then return nil end
    if kind == "int" then return number >= 0 and math.floor(number) or math.ceil(number) end
    return number
end

function Schema.default(kind) return DEFAULTS[kind] end

local function readPath(values, path)
    local value = values
    for part in path:gmatch("[^%.]+") do
        if type(value) ~= "table" then return nil end
        value = value[part]
    end
    return value
end
Schema.readPath = readPath

local function parseEntity(node, label)
    local entity = setmetatable({
        name = assert(node.attrs.Name, label .. ": <Entity> needs Name"),
        description = node.attrs.Description, columns = {}, byName = {}, relations = {}, relationsByName = {},
        line = node.line, resource = label,
    }, Entity)
    for _, child in ipairs(node.children) do
        local a = child.attrs
        if child.tag == "Column" then
            local name = assert(a.Name, label .. ":" .. child.line .. ": <Column> needs Name")
            local kind = (a.Type or "string"):lower()
            if kind == "texture" then kind = "string" end
            if not TYPES[kind] then
                error(("%s:%d: column %s.%s has unsupported type '%s'"):format(label, child.line, entity.name, name, a.Type), 0)
            end
            if entity.byName[name] then
                error(("%s:%d: duplicate column %s.%s"):format(label, child.line, entity.name, name), 0)
            end
            if RESERVED[name] then
                error(("%s:%d: column %s.%s uses a reserved object property name; choose another (for example Title)")
                    :format(label, child.line, entity.name, name), 0)
            end
            local column = {
                name = name, type = kind, source = a.Source, nullable = flag(a.Nullable),
                readOnly = flag(a.ReadOnly) or a.Computed ~= nil or a.From ~= nil,
                computed = a.Computed, from = a.From, dependsOn = words(a.DependsOn),
                isLocal = flag(a.Local), key = flag(a.Key), description = a.Description, line = child.line,
            }
            column.default = a.Default ~= nil and Schema.convert(kind, a.Default) or DEFAULTS[kind]
            if column.key then entity.key = name end
            entity.columns[#entity.columns + 1] = column
            entity.byName[name] = column
        elseif child.tag == "Relation" then
            local relation = {
                name = assert(a.Name, label .. ":" .. child.line .. ": <Relation> needs Name"),
                entity = assert(a.Entity, label .. ":" .. child.line .. ": <Relation> needs Entity"),
                key = a.Key, source = a.Source, many = flag(a.Many), embed = flag(a.Embed), line = child.line,
                order = a.Order, description = a.Description,
            }
            entity.relations[#entity.relations + 1] = relation
            entity.relationsByName[relation.name] = relation
        end
    end
    entity.key = node.attrs.Key or entity.key
    -- Mirrors are named by this column (default: the key); ItemNamePrefix and
    -- child paths use the name.
    entity.objectName = node.attrs.ObjectName
    if #entity.columns == 0 then error(("%s:%d: entity %s has no columns"):format(label, node.line, entity.name), 0) end
    if entity.objectName and not entity.byName[entity.objectName] then
        error(("%s:%d: entity %s ObjectName '%s' is not a column"):format(label, node.line, entity.name, entity.objectName), 0)
    end
    if entity.key and not entity.byName[entity.key] then
        error(("%s:%d: entity %s key '%s' is not a column"):format(label, node.line, entity.name, entity.key), 0)
    end
    return entity
end

-- Each referenced relation, projection and key must exist. Errors name the
-- resource and line so an editor can point at the declaration.
function Schema:validate()
    for _, entity in ipairs(self.list) do
        for _, relation in ipairs(entity.relations) do
            local target = self.entities[relation.entity]
            if not target then
                error(("%s:%d: relation %s.%s names unknown entity '%s'"):format(entity.resource, relation.line,
                    entity.name, relation.name, relation.entity), 0)
            end
            if relation.key and not entity.byName[relation.key] then
                error(("%s:%d: relation %s.%s key '%s' is not a column"):format(entity.resource, relation.line,
                    entity.name, relation.name, relation.key), 0)
            end
            if not relation.many and not relation.key then
                error(("%s:%d: to-one relation %s.%s needs Key"):format(entity.resource, relation.line, entity.name, relation.name), 0)
            end
            if relation.many and not relation.source then
                error(("%s:%d: to-many relation %s.%s needs Source"):format(entity.resource, relation.line, entity.name, relation.name), 0)
            end
            if relation.embed and relation.many then
                error(("%s:%d: only to-one relations embed; %s.%s is to-many"):format(entity.resource, relation.line,
                    entity.name, relation.name), 0)
            end
            if relation.many and not target.key then
                error(("%s:%d: relation %s.%s needs a keyed entity"):format(entity.resource, relation.line, entity.name, relation.name), 0)
            end
        end
        for _, column in ipairs(entity.columns) do
            if column.from then
                local relationName, field = column.from:match("^([%w_]+)%.([%w_]+)$")
                local relation = relationName and entity.relationsByName[relationName]
                local target = relation and self.entities[relation.entity]
                if not relation or relation.many or not target or not target.byName[field] then
                    error(("%s:%d: column %s.%s projects unknown field '%s'"):format(entity.resource, column.line,
                        entity.name, column.name, column.from), 0)
                end
                column.relation, column.field = relationName, field
            end
        end
    end
    return self
end

local function signature(entity)
    local parts = {}
    for _, column in ipairs(entity.columns) do parts[#parts + 1] = column.name .. ":" .. column.type end
    return table.concat(parts, ",")
end

-- Registers one native class per entity and keeps a prototype to instantiate.
local function register(entities)
    local fs = require "orca.filesystem"
    local pending = {}
    for _, entity in ipairs(entities) do
        local known = registered[entity.name]
        if known then
            if known.signature ~= signature(entity) then
                error(("Entity %s is already registered with different columns"):format(entity.name), 0)
            end
            entity.prototype = known.prototype
        else
            pending[#pending + 1] = entity
        end
    end
    -- The native schema parser registers at most 32 entities per document.
    for first = 1, #pending, 32 do
        local schema, data = { "<Schema>" }, { "<Prototypes>" }
        for index = first, math.min(#pending, first + 31) do
            local entity = pending[index]
            schema[#schema + 1] = '<Entity Name="' .. entity.name .. '">'
            for _, column in ipairs(entity.columns) do
                schema[#schema + 1] = '<Column Name="' .. column.name .. '" Type="' .. column.type .. '"/>'
            end
            schema[#schema + 1] = "</Entity>"
            data[#data + 1] = "<" .. entity.name .. ' name="' .. entity.name .. '"/>'
        end
        schema[#schema + 1], data[#data + 1] = "</Schema>", "</Prototypes>"
        local root = assert(fs.loadXmlDataStringWithSchema(table.concat(data), table.concat(schema)),
            "Could not register data entities")
        for index = first, math.min(#pending, first + 31) do
            local entity = pending[index]
            entity.prototype = assert(root:findChild(entity.name), "Could not register entity " .. entity.name)
            registered[entity.name] = { signature = signature(entity), prototype = entity.prototype }
        end
    end
end

function Schema.parse(text, label)
    label = label or "schema"
    local root = Xml.parse(text, label)
    if root.tag ~= "Schema" then error(label .. ": the root element must be <Schema>", 0) end
    local self = setmetatable({ entities = {}, list = {}, resource = label }, Schema)
    local function add(node, resource)
        for _, child in ipairs(node.children) do
            if child.tag == "Entity" then
                local entity = parseEntity(child, resource)
                if self.entities[entity.name] then
                    error(("%s:%d: duplicate entity %s"):format(resource, child.line, entity.name), 0)
                end
                entity.schema = self
                self.entities[entity.name] = entity
                self.list[#self.list + 1] = entity
            elseif child.tag == "Include" then
                local path = child.attrs.Path or child.attrs.File
                local dir = resource:match("^(.*)/[^/]*$")
                local included = path:sub(1, 1) == "/" and path or (dir and dir .. "/" .. path or path)
                local nested = Xml.load(included)
                add(nested, included)
            end
        end
    end
    add(root, label)
    self:validate()
    register(self.list)
    return self
end

-- Registers a runtime entity (request state, parameters, form errors).
-- columns: ordered { name, type } pairs.
function Schema.define(name, columns)
    local parts = { '<Schema><Entity Name="', Xml.escape(name), '">' }
    for _, column in ipairs(columns) do
        parts[#parts + 1] = ('<Column Name="%s" Type="%s"/>'):format(column[1], column[2])
    end
    parts[#parts + 1] = "</Entity></Schema>"
    return Schema.parse(table.concat(parts), "runtime:" .. name):entity(name)
end

function Schema.load(path)
    if not loaded[path] then loaded[path] = Schema.parse(Xml.read(path), path) end
    return loaded[path]
end

function Schema:entity(name)
    local entity = self.entities[name]
    if not entity then error(("%s: unknown entity '%s'"):format(self.resource, tostring(name)), 2) end
    return entity
end

-- Adds entities from another schema (for example the runtime's own state
-- entities) so declarations can name them.
function Schema:merge(other)
    for _, entity in ipairs(other.list) do
        if not self.entities[entity.name] then
            self.entities[entity.name] = entity
            self.list[#self.list + 1] = entity
        end
    end
    return self
end

-- Field list, nullability, relationships and writability for tools such as a
-- visual editor. Plain tables only; no native objects.
function Schema:describe()
    local result = { resource = self.resource, entities = {} }
    for _, entity in ipairs(self.list) do
        local item = { name = entity.name, key = entity.key, description = entity.description, columns = {}, relations = {} }
        for _, column in ipairs(entity.columns) do
            item.columns[#item.columns + 1] = { name = column.name, type = column.type, nullable = column.nullable,
                writable = not column.readOnly, key = column.name == entity.key, source = column.source,
                computed = column.computed, from = column.from, dependsOn = column.dependsOn,
                isLocal = column.isLocal, description = column.description }
        end
        for _, relation in ipairs(entity.relations) do
            item.relations[#item.relations + 1] = { name = relation.name, entity = relation.entity,
                cardinality = relation.many and "many" or "one", key = relation.key, source = relation.source,
                embedded = relation.embed }
        end
        result.entities[#result.entities + 1] = item
    end
    return result
end

function Entity:instantiate()
    return self.prototype:instantiate()
end

function Entity:column(name) return self.byName[name] end

-- Converts a service record into column values. `partial` leaves missing
-- fields untouched (nil in the result) rather than resetting them.
function Entity:map(values, partial)
    local result, present = {}, {}
    for _, column in ipairs(self.columns) do
        if not column.computed and not column.from and not column.isLocal then
            local raw = readPath(values, column.source or column.name)
            if raw ~= nil or not partial then
                if raw ~= Schema.null then result[column.name] = Schema.convert(column.type, raw) end
                present[column.name] = true
            end
        end
    end
    return result, present
end

return Schema
