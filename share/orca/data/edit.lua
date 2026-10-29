-- A form draft: a native record that inputs bind with Mode=TwoWay, dirty and
-- validation state, and a submit that hands a snapshot to a command. The
-- committed record is never the target of keystrokes. Failure keeps the
-- input; text typed while a submit is pending survives its success.
local Schema = require "orca.data.schema"
local Edit = {}
Edit.__index = Edit

Edit.STATE = { "IsDirty", "IsValid", "HasError", "ErrorMessage", "IsSubmitting", "CanSubmit", "IsEditing" }

-- def: name, entity (draft entity), validate(values, context) -> errors | nil
-- (field -> message, `_` for the form), submit (command name), copy
-- (draft field -> record column for edit copies), keyed (per-key drafts),
-- clearOnSuccess (default true), canSubmit(values, context),
-- submitted(ok, result, err, info, edit) after each submit finishes, where
-- info is { key, visible (its draft is the current one), cleared (nothing
-- was typed while it was pending) }.
local NOKEY = {}
local function id(key) if key == nil then return NOKEY end return key end

function Edit.new(scope, def)
    local self = setmetatable({ scope = scope, def = def, name = assert(def.name, "Edit needs a name"),
        drafts = {}, base = {}, errors = {}, pending = {} }, Edit)
    self.entity = scope.store:entity(assert(def.entity, "Edit " .. self.name .. " needs Entity"))
    self.draft = self.entity:instantiate()
    self.draft.Name = self.name
    scope.root:addChild(self.draft)
    local errorColumns = {}
    for _, column in ipairs(self.entity.columns) do errorColumns[#errorColumns + 1] = { column.name, "string" } end
    self.errorObject = Schema.define(self.entity.name .. "Errors", errorColumns):instantiate()
    self.errorObject.Name = "Errors"
    self.draft:addChild(self.errorObject)
    self.state = scope:stateRecord("DataEdit", "Status")
    self.state:removeFromParent()
    self.draft:addChild(self.state)
    for _, column in ipairs(self.entity.columns) do
        self.base[column.name] = column.default
        self.draft[column.name] = column.default
        self.draft["on" .. column.name .. "Changed"] = function() self:changed() end
    end
    self:publish()
    return self
end

function Edit:values()
    local values = {}
    for _, column in ipairs(self.entity.columns) do
        values[column.name] = Schema.convert(column.type, self.draft[column.name])
    end
    return values
end
Edit.snapshot = Edit.values

function Edit:write(values)
    for _, column in ipairs(self.entity.columns) do
        local value = values[column.name]
        if value == nil then value = column.default end
        if self.draft[column.name] ~= value then self.draft[column.name] = value end
    end
end

function Edit:dirty()
    local values = self:values()
    for name, value in pairs(values) do
        if value ~= self.base[name] then return true end
    end
    return false
end

function Edit:publish()
    -- Each keyed draft has its own submission; the current key's shows.
    self.submitting = self.pending[id(self.key)] ~= nil
    local values = self:values()
    local valid = true
    if self.def.validate then valid = self.def.validate(values, self.scope.context) == nil end
    local can = not self.submitting and valid
    if can and self.def.canSubmit then can = self.def.canSubmit(values, self.scope.context, self) ~= false end
    local general = self.errors._
    self.scope:write(self.state, {
        IsDirty = self:dirty(), IsValid = valid, HasError = general ~= nil or next(self.errors) ~= nil,
        ErrorMessage = general or "", IsSubmitting = self.submitting == true, CanSubmit = can,
        IsEditing = self.target ~= nil,
    })
    for _, column in ipairs(self.entity.columns) do
        local message = self.errors[column.name] or ""
        if self.errorObject[column.name] ~= message then self.errorObject[column.name] = message end
    end
    if self.def.submit then
        local command = self.scope:member(self.def.submit)
        if command and command.refresh then command:refresh { draft = values, edit = self } end
    end
end

-- Field errors clear as the user corrects the field.
function Edit:changed()
    if next(self.errors) then
        local values = self:values()
        for name in pairs(self.errors) do
            if name ~= "_" and self.previous and values[name] ~= self.previous[name] then self.errors[name] = nil end
        end
    end
    self:publish()
end

-- Per-key drafts (one composer draft per conversation). Switching saves the
-- current draft and restores the other key's.
function Edit:switch(key)
    if self.key == key then return end
    if self.key ~= nil and not self.target then self.drafts[self.key] = self:values() end
    self.key, self.target, self.stashed, self.errors = key, nil, nil, {}
    self:write(self.drafts[key] or {})
    self:publish()
end

-- Starts an edit copy of a committed record. The unsent new draft is kept
-- and comes back after save or cancel.
function Edit:begin(record)
    if not self.target then self.stashed = self:values() end
    self.target, self.errors = record, {}
    local values = {}
    for field, column in pairs(self.def.copy or {}) do values[field] = record.values[column] end
    self:write(values)
    self.base = self:values()
    self:publish()
end

function Edit:cancel()
    if self.target then
        self.target = nil
        self:write(self.stashed or {})
        self.stashed = nil
    else
        self:write({})
    end
    self.base, self.errors = {}, {}
    for _, column in ipairs(self.entity.columns) do self.base[column.name] = column.default end
    self:publish()
end

function Edit:reset(values)
    self.errors = {}
    self:write(values or {})
    self:publish()
end

-- Clears only fields the user has not changed since the snapshot.
local function settle(self, snapshot)
    local current, cleared = self:values(), {}
    for name, value in pairs(current) do
        if value ~= snapshot[name] then cleared[name] = value end
    end
    return cleared
end

function Edit:submit(params)
    local key = self.key
    if self.pending[id(key)] then return false, "busy" end
    local snapshot = self:values()
    local errors = self.def.validate and self.def.validate(snapshot, self.scope.context)
    if errors then
        self.errors, self.previous = errors, snapshot
        self:publish()
        return false, "invalid"
    end
    local command = assert(self.scope:member(self.def.submit), "Edit " .. self.name .. " has no submit command")
    local target = self.target
    params = params or {}
    params.draft, params.edit, params.target = snapshot, self, target
    params.key = key
    self.pending[id(key)], self.errors = true, {}
    self:publish()
    local started = command:execute(params, function(ok, result, err)
        self.pending[id(key)] = nil
        local visible, cleared = self.key == key, true
        if ok and visible then
            local keep = settle(self, snapshot)
            cleared = next(keep) == nil
            if target and self.target == target then
                self.target = nil
                self:write(self.stashed or {})
                self.stashed = nil
            elseif self.def.clearOnSuccess ~= false then
                self:write(keep)
            end
            self.base = {}
            for _, column in ipairs(self.entity.columns) do self.base[column.name] = column.default end
            if self.key ~= nil then self.drafts[self.key] = nil end
        elseif ok and self.def.clearOnSuccess ~= false then
            -- Another draft is showing: settle the saved one the same way.
            local keep = {}
            for name, value in pairs(self.drafts[key] or {}) do
                if value ~= snapshot[name] then keep[name] = value end
            end
            cleared = next(keep) == nil
            self.drafts[key] = not cleared and keep or nil
        elseif visible and err and err.code ~= "stale" and err.message then
            self.errors = { _ = err.message or "Could not save. Your changes are still here." }
        end
        if not self.disposed then self:publish() end
        if self.def.submitted then
            self.def.submitted(ok, result, err, { key = key, visible = visible, cleared = cleared }, self)
        end
    end)
    if not started then
        self.pending[id(key)] = nil
        self:publish()
    end
    return started
end

-- Forget saved drafts (sign-out).
function Edit:clear()
    self.drafts, self.key, self.target, self.stashed, self.errors = {}, nil, nil, nil, {}
    self.pending = {}
    self:write({})
    self:publish()
end

function Edit:dispose() self.disposed = true end

return Edit
