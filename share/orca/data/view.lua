-- A local view over records another query or view has already loaded:
-- filtering, sorting and selection without a request. Filtering one loaded
-- page is not a server search; use a query parameter for that.
local Collection = require "orca.data.collection"
local Params = require "orca.data.params"
local View = {}
View.__index = View

View.STATE = { "Count", "HasItems", "IsEmpty", "IsFiltered", "ShowLoading", "ShowEmpty", "ShowError",
    "HasError", "ErrorMessage", "HasSelection", "SelectedKey" }

-- def: name, source (query or view), filter(record, params, context),
-- sort(a, b, params, context), params, selection ("clear" | "keep" | "first").
function View.new(scope, def)
    local self = setmetatable({ scope = scope, def = def, name = assert(def.name, "View needs a name"),
        source = assert(def.source, "View " .. tostring(def.name) .. " needs a Source"), listeners = {},
        policy = def.selection or "clear" }, View)
    self.entity = self.source.entity
    self.state = scope:stateRecord("DataView", self.name)
    self.items = Collection.new(scope.store, { name = "Items", parent = self.state })
    self.params = Params.new(scope, self.name, def.params or {}, function() self:refresh() end)
    if self.params.object then self.state:addChild(self.params.object) end
    self.unlistenSource = self.source:listen(function() self:refresh() end)
    -- Field edits can move a record in or out of the filter or change order.
    self.unlistenStore = scope.store:listen(function(record)
        if self.member and self.member[record] then self.dirty = true end
    end)
    self.unlistenFlush = scope.store:onFlush(function()
        if self.dirty then self:refresh() end
    end)
    self:refresh()
    return self
end

View.listen = function(self, fn)
    self.listeners[#self.listeners + 1] = fn
    return function()
        for index, listener in ipairs(self.listeners) do
            if listener == fn then table.remove(self.listeners, index); return end
        end
    end
end

function View:records() return self.items:records() end
function View:set(name, value) return self.params:set(name, value) end
function View:get(name) return self.params:get(name) end

function View:refresh()
    if self.disposed then return end
    self.dirty = false
    local params, context = self.params:snapshot(), self.scope.context
    local filter, sort = self.def.filter, self.def.sort
    local all, rows, member = self.source.items.rows, {}, {}
    for _, row in ipairs(all) do
        member[row.record] = true
        if not filter or filter(row.record, params, context) then
            rows[#rows + 1] = { key = row.key, record = row.record }
        end
    end
    if sort then
        table.sort(rows, function(a, b) return sort(a.record, b.record, params, context) end)
    end
    self.member = member
    self.items:set(rows)
    self:reconcileSelection()
    self:publish()
    for _, listener in ipairs({ table.unpack(self.listeners) }) do listener(self) end
end

function View:status() return self.source:status() end

function View:publish()
    local status = self.source:status()
    local count = self.items:count()
    self.scope:write(self.state, {
        Count = count, HasItems = count > 0, IsEmpty = count == 0,
        IsFiltered = self.def.filter ~= nil and count ~= self.source.items:count(),
        ShowLoading = status.loading, ShowError = status.failed, HasError = status.error ~= nil,
        ErrorMessage = status.error and status.error.message or "",
        ShowEmpty = count == 0 and status.loaded and not status.loading and not status.failed and not status.busy,
        HasSelection = self.selected ~= nil, SelectedKey = self.selected and tostring(self.selected.key) or "",
    })
end

-- Selection follows identity across refresh and reorder. A selected record
-- that leaves the view is cleared, kept, or replaced by the first row.
function View:select(target)
    local record = target
    if type(target) ~= "table" then record = self.items:find(target) or self.source.items:find(target) end
    if record == self.selected then return record end
    if self.selection then
        self.selection.object:removeFromParent()
        self.scope.store:release(self.selection)
        self.selection = nil
    end
    self.selected = record
    if record then
        self.selection = self.scope.store:mirror(record)
        self.selection.object.Name = "Selected"
        self.state:addChild(self.selection.object)
    end
    self:publish()
    if self.def.selected then self.def.selected(record, self) end
    for _, listener in ipairs(self.selectionListeners or {}) do listener(record) end
    return record
end

function View:reconcileSelection()
    local selected = self.selected
    if selected and (selected.deleted or (not self.items:find(selected.key) and self.policy ~= "keep")) then
        self:select(self.policy == "first" and self.items.rows[1] and self.items.rows[1].record or nil)
    elseif not selected and self.policy == "first" and self.items.rows[1] then
        self:select(self.items.rows[1].record)
    end
end

function View:dispose()
    if self.disposed then return end
    self:select(nil)
    self.disposed = true
    self.unlistenSource(); self.unlistenStore(); self.unlistenFlush()
    self.items:dispose()
    self.listeners = {}
end

return View
