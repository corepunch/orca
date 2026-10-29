-- Declared, typed parameters of a query or view. They live in a native
-- "Params" record so inputs bind to them with Mode=TwoWay; an edit, a
-- route parameter or a selection change all arrive through `set`.
local Schema = require "orca.data.schema"
local Params = {}
Params.__index = Params

-- decls: list of { name, type, default, debounce, description }.
function Params.new(scope, owner, decls, changed)
    local self = setmetatable({ values = {}, decls = {}, list = decls, changed = changed }, Params)
    local columns = {}
    for _, decl in ipairs(decls) do
        decl.type = decl.type or "string"
        self.decls[decl.name] = decl
        self.values[decl.name] = Schema.convert(decl.type, decl.default) or Schema.default(decl.type)
        columns[#columns + 1] = { decl.name, decl.type }
    end
    if #columns > 0 then
        local entity = Schema.define((scope.entityPrefix or "Data") .. owner .. "Params", columns)
        self.object = entity:instantiate()
        self.object.Name = "Params"
        for _, decl in ipairs(decls) do
            self.object[decl.name] = self.values[decl.name]
            -- TwoWay edits reach the parameter after the property notification.
            self.object["on" .. decl.name .. "Changed"] = function(_, value)
                self:set(decl.name, value)
            end
        end
    end
    return self
end

function Params:get(name) return self.values[name] end

function Params:set(name, value)
    local decl = self.decls[name]
    if not decl then error("Unknown parameter '" .. tostring(name) .. "'", 2) end
    value = Schema.convert(decl.type, value)
    if value == nil then value = Schema.default(decl.type) end
    if self.values[name] == value then return false end
    self.values[name] = value
    if self.object and self.object[name] ~= value then self.object[name] = value end
    if self.changed then self.changed(name, value) end
    return true
end

function Params:debounce(name)
    local decl = self.decls[name]
    return decl and decl.debounce
end

function Params:snapshot()
    local copy = {}
    for name, value in pairs(self.values) do copy[name] = value end
    return copy
end

return Params
