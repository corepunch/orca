-- A network query: one provider operation, bindable parameters, observable
-- request state and a keyed result collection. Each request carries a
-- generation; a response for older parameters, a replaced session or a
-- disposed owner is discarded. Cancellation only invalidates the result: the
-- transport may still complete, and nothing is rolled back on the server.
local Collection = require "orca.data.collection"
local Params = require "orca.data.params"
local Query = {}
Query.__index = Query

local STATE = { "IsLoading", "IsRefreshing", "IsLoadingMore", "IsBusy", "HasLoaded", "IsEmpty", "HasItems",
    "HasError", "ErrorMessage", "ErrorCode", "HasMore", "CanLoadMore", "Count", "ShowLoading", "ShowEmpty",
    "ShowError", "IsStale" }
Query.STATE = STATE

local function structured(err)
    if type(err) == "table" then
        return { code = err.code or "error", message = err.message or "Something went wrong.", retryable = err.retryable }
    end
    return { code = "error", message = err ~= nil and tostring(err) or "Something went wrong." }
end
Query.structured = structured

-- def: name, entity, operation(params, page, context) -> result | nil, err;
-- params (Params declarations), pageSize, poll (ms), require (param names that
-- must be non-empty), rowKey(record, item) for rows distinct from entities,
-- keepOnRefresh (default true), invalidatedBy (command names).
function Query.new(scope, def)
    local self = setmetatable({ scope = scope, def = def, name = assert(def.name, "Query needs a name"),
        generation = 0, cursor = nil, error = nil, loaded = false, listeners = {} }, Query)
    self.entity = scope.store:entity(assert(def.entity, "Query " .. self.name .. " needs Entity"))
    if type(def.operation) ~= "function" then
        error(("%s: query %s has no operation"):format(scope.resource or "data", self.name), 0)
    end
    self.state = scope:stateRecord("DataQuery", self.name)
    self.items = Collection.new(scope.store, { name = "Items", parent = self.state })
    self.params = Params.new(scope, self.name, def.params or {}, function(name) self:paramsChanged(name) end)
    if self.params.object then self.state:addChild(self.params.object) end
    -- Membership changes (including deleted records) republish the state.
    self.items:listen(function() self:publish() end)
    self:publish()
    return self
end

function Query:listen(fn)
    self.listeners[#self.listeners + 1] = fn
    return function()
        for index, listener in ipairs(self.listeners) do
            if listener == fn then table.remove(self.listeners, index); return end
        end
    end
end

function Query:emit()
    for _, listener in ipairs({ table.unpack(self.listeners) }) do listener(self) end
end

function Query:records() return self.items:records() end

-- The bindable state is derived from the request fields in one place.
function Query:publish()
    local count = self.items:count()
    -- Quiet requests (polling, invalidation) keep the visible state steady.
    local mode = not self.quiet and self.mode or nil
    local values = {
        IsLoading = mode == "load" and not self.loaded,
        IsRefreshing = mode == "refresh" or (mode == "load" and self.loaded),
        IsLoadingMore = mode == "more", IsBusy = mode ~= nil, HasLoaded = self.loaded,
        IsEmpty = self.loaded and count == 0, HasItems = count > 0,
        HasError = self.error ~= nil, ErrorMessage = self.error and self.error.message or "",
        ErrorCode = self.error and self.error.code or "", HasMore = self.cursor ~= nil,
        CanLoadMore = self.cursor ~= nil and self.mode == nil, Count = count, IsStale = self.stale == true,
    }
    values.ShowLoading = values.IsLoading and count == 0
    values.ShowEmpty = values.IsEmpty and not values.HasError and mode == nil
    -- Rows that loaded once stay visible through a failed refresh.
    values.ShowError = values.HasError and not self.loaded and mode == nil
    self.scope:write(self.state, values)
    self:emit()
end

-- What dependent views need to know, independent of native fields.
function Query:status()
    local count = self.items:count()
    local visible = not self.quiet and self.mode or nil
    return { loading = visible == "load" and not self.loaded and count == 0, busy = visible ~= nil,
        loaded = self.loaded, error = self.error, failed = self.error ~= nil and not self.loaded and visible == nil }
end

local function normalize(result)
    if result == nil then return nil end
    if result.items then return result.items, result.cursor, result.hasMore end
    return result, nil, nil
end

-- Converts operation rows into store records (or keeps records and
-- { key, record } rows a domain operation already built).
function Query:rows(items)
    local store, rows = self.scope.store, {}
    store:batch(function()
        for index, item in ipairs(items) do
            if type(item) == "table" and item.store == store and item.entity then
                rows[index] = item
            elseif type(item) == "table" and item.record and item.key ~= nil then
                rows[index] = item
            else
                rows[index] = store:put(self.entity.name, item)
            end
        end
    end)
    return rows
end

local function blocked(self)
    for _, name in ipairs(self.def.require or {}) do
        local value = self.params:get(name)
        if value == nil or value == "" then return true end
    end
    return false
end

-- mode: "load" (first or replacing), "refresh" (keep rows), "more" (next page).
function Query:request(mode, options)
    options = options or {}
    if self.disposed then return false end
    if mode == "more" and (self.cursor == nil or self.mode ~= nil) then return false end
    if mode ~= "more" and self.mode ~= nil and not options.force then
        -- Same parameters already in flight: coalesce instead of racing.
        self.pendingRefresh = self.pendingRefresh or mode
        return false
    end
    if blocked(self) then
        self.generation = self.generation + 1
        self.mode, self.error, self.cursor, self.loaded = nil, nil, nil, false
        self.items:clear()
        self:publish()
        return false
    end
    self.generation = self.generation + 1
    local generation, token = self.generation, self.scope:token()
    local params = self.params:snapshot()
    local page = { cursor = mode == "more" and self.cursor or nil, size = self.def.pageSize }
    self.mode, self.quiet = mode, options.quiet and mode ~= "more"
    self:publish()
    self.scope:run(function()
        local ok, result, err = xpcall(self.def.operation, debug.traceback, params, page, self.scope.context, self)
        if self.disposed or self.generation ~= generation or not self.scope:valid(token) then return end
        self.mode = nil
        if not ok then
            self.scope:report(result, "Loading " .. self.name)
            result, err = nil, { code = "error", message = "Could not load. Please try again." }
        end
        local items, cursor, hasMore = normalize(result)
        if not items then
            self.error = structured(err)
            self.stale = self.loaded
            self:publish()
            self.scope:changed(self)
            return self:afterRequest()
        end
        local rows = self:rows(items)
        if hasMore == false then cursor = nil end
        self.error, self.stale, self.cursor, self.loaded = nil, false, cursor, true
        if mode == "more" then self.items:append(rows) else self.items:set(rows) end
        self.scope:changed(self)
        if self.def.loaded then self.def.loaded(self, rows, mode) end
        self:afterRequest()
    end)
    return true
end

function Query:afterRequest()
    local pending = self.pendingRefresh
    self.pendingRefresh = nil
    if pending and not self.disposed then self:request(pending, { quiet = true }) end
end

-- Initial load (or reload after parameters change); keeps nothing stale.
function Query:load() return self:request("load") end
function Query:refresh(options) return self:request(self.loaded and "refresh" or "load", options) end
function Query:retry() return self:request(self.loaded and "refresh" or "load", { force = true }) end
function Query:loadMore() return self:request("more") end

-- Marks results stale after a command; active queries reload, inactive ones
-- on activation.
function Query:invalidate()
    if self.active == false then self.stale = true; return false end
    return self:refresh { quiet = true, force = true }
end

function Query:set(name, value) return self.params:set(name, value) end
function Query:get(name) return self.params:get(name) end

-- A parameter change starts a new result: the cursor and rows of other
-- parameters do not apply. Debounced parameters wait for the pause.
function Query:paramsChanged(name)
    self.cursor = nil
    local delay = self.params:debounce(name)
    if self.debounceHandle then self.debounceHandle:cancel(); self.debounceHandle = nil end
    if delay and delay > 0 then
        self.generation = self.generation + 1
        self.mode = nil
        self.debounceHandle = self.scope:after(delay, function()
            self.debounceHandle = nil
            self:request("load", { force = true })
        end)
        self:publish()
    else
        self:request("load", { force = true })
    end
end

-- Polling and foreground refresh follow the query's owner, not each view.
function Query:activate()
    self.active = true
    if self.stale and self.loaded then self.stale = false; self:refresh { quiet = true } end
    if not self.loaded and self.mode == nil then self:load() end
    self:schedule()
end

function Query:deactivate()
    self.active = false
    if self.pollHandle then self.pollHandle:cancel(); self.pollHandle = nil end
end

function Query:schedule()
    if not self.def.poll or self.disposed or not self.active or self.pollHandle then return end
    if self.scope.foreground == false then return end
    self.pollHandle = self.scope:after(self.def.poll, function()
        self.pollHandle = nil
        if self.disposed or not self.active then return end
        if self.mode == nil and (not self.def.pollWhen or self.def.pollWhen(self)) then self:refresh { quiet = true } end
        self:schedule()
    end)
end

function Query:setForeground(active)
    if active then
        if self.active then
            if self.def.poll then self:refresh { quiet = true } end
            self:schedule()
        end
    elseif self.pollHandle then
        self.pollHandle:cancel(); self.pollHandle = nil
    end
end

function Query:dispose()
    if self.disposed then return end
    self:deactivate()
    if self.debounceHandle then self.debounceHandle:cancel() end
    self.disposed = true
    self.generation = self.generation + 1
    self.items:dispose()
    self.listeners = {}
end

return Query
