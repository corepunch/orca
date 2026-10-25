-- Lapis-inspired row models over the application's API, scoped to one client.
local Model = {}

function Model:extend(name, fields)
    assert(type(name) == "string" and name ~= "", "Model needs a name")
    fields = fields or {}
    local rows = {}
    for key, value in pairs(fields) do
        if key ~= "fetch" and key ~= "insert" then rows[key] = value end
    end
    rows.__index = rows
    local class = setmetatable({ name = name, rows = rows, fetch = fields.fetch, insert = fields.insert }, { __index = self })
    return class, rows
end

function Model:using(client)
    assert(client, "Model needs an API client")
    return setmetatable({ client = client }, { __index = self })
end

function Model:table_name()
    return self.name
end

function Model:load(values)
    local row = {}
    for key, value in pairs(values) do row[key] = value end
    row._model = self
    return setmetatable(row, self.rows)
end

function Model:select(options)
    assert(self.client and self.fetch, "Use a client-bound model with a fetch operation")
    local values, err = self.fetch(self.client, options or {})
    if not values then return nil, err end
    local rows = {}
    for i, value in ipairs(values) do rows[i] = self:load(value) end
    return rows
end

-- Conditions are exact field matches; transport-specific query syntax stays private.
function Model:find(condition, options)
    if condition == nil then return nil end
    if type(condition) ~= "table" then condition = { id = condition } end
    local rows, err = self:select(options)
    if not rows then return nil, err end
    for _, row in ipairs(rows) do
        local matches = true
        for key, value in pairs(condition) do
            if row[key] ~= value then matches = false; break end
        end
        if matches then return row end
    end
end

-- Select once and match a set of exact keys within the provider's result window.
-- This preserves provider ordering and never invents transport query syntax.
function Model:find_all(keys, options)
    assert(type(keys) == "table", "find_all needs a list of keys")
    options = options or {}
    local key, wanted = options.key or "id", {}
    for _, value in ipairs(keys) do wanted[value] = true end
    if next(wanted) == nil then return {} end
    local query = {}
    for field, value in pairs(options) do if field ~= "key" then query[field] = value end end
    local rows, err = self:select(query)
    if not rows then return nil, err end
    local matches = {}
    for _, row in ipairs(rows) do
        if row[key] ~= nil and wanted[row[key]] then matches[#matches + 1] = row end
    end
    return matches
end

function Model:create(values)
    assert(self.client and self.insert, "This model does not support create")
    local value, err = self.insert(self.client, values)
    if not value then return nil, err end
    return self:load(value)
end

return Model
