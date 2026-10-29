-- A declared operation a view can invoke: bindable availability, execution
-- and structured error state, a re-entry policy, and the queries it
-- invalidates on success. Failure never leaves a permanent busy state.
local Query = require "orca.data.query"
local Command = {}
Command.__index = Command

Command.STATE = { "CanExecute", "IsExecuting", "IsAvailable", "HasError", "ErrorMessage", "ErrorCode",
    "Succeeded", "Reason" }

local POLICIES = { block = true, queue = true, parallel = true, latest = true }

-- def: name, operation(params, context, command) -> result | nil, err;
-- canExecute(params, context) -> bool, reason; concurrency ("block" by
-- default); invalidates (query names); succeeded/failed callbacks; available.
function Command.new(scope, def)
    local self = setmetatable({ scope = scope, def = def, name = assert(def.name, "Command needs a name"),
        running = 0, generation = 0, queue = {} }, Command)
    self.policy = def.concurrency or "block"
    if not POLICIES[self.policy] then
        error(("%s: command %s has unknown concurrency '%s'"):format(scope.resource or "data", self.name, self.policy), 0)
    end
    self.available = def.available ~= false and type(def.operation) == "function"
    self.state = scope:stateRecord("DataCommand", self.name)
    self:publish()
    return self
end

function Command:check(params)
    if not self.available then return false, "unsupported" end
    if self.running > 0 and self.policy == "block" then return false, "busy" end
    if self.def.canExecute then
        local ok, reason = self.def.canExecute(params or {}, self.scope.context, self)
        if not ok then return false, reason or "unavailable" end
    end
    return true
end

function Command:publish()
    local ok, reason = self:check(self.lastParams)
    self.scope:write(self.state, {
        CanExecute = ok, IsExecuting = self.running > 0, IsAvailable = self.available,
        HasError = self.error ~= nil, ErrorMessage = self.error and self.error.message or "",
        ErrorCode = self.error and self.error.code or "", Succeeded = self.succeeded == true,
        Reason = ok and "" or tostring(reason or ""),
    })
end

-- Starts the operation. `params` carries the resolved item, selection, draft
-- snapshot, event sender and declared arguments. `done(ok, result, err)`
-- runs after the outcome is applied.
function Command:execute(params, done)
    params = params or {}
    local ok, reason = self:check(params)
    if not ok then
        if reason == "unsupported" then
            self.error = { code = "unsupported", message = "This action isn't available." }
            self:publish()
        end
        if done then done(false, nil, { code = reason }) end
        return false, reason
    end
    if self.policy == "queue" and self.running > 0 then
        self.queue[#self.queue + 1] = { params = params, done = done }
        return true
    end
    self.generation = self.generation + 1
    local generation, token = self.generation, self.scope:token()
    self.running, self.error, self.succeeded = self.running + 1, nil, false
    self:publish()
    local function finish(result, err, failure)
        self.running = self.running - 1
        local current = self.scope:valid(token) and not self.disposed
            and (self.policy ~= "latest" or generation == self.generation)
        if failure and current then self.scope:report(failure, self.name) end
        if current then
            if result == nil then
                self.error = Query.structured(err)
                if self.def.failed then self.def.failed(self.error, params, self) end
            else
                self.succeeded = true
                for _, name in ipairs(self.def.invalidates or {}) do self.scope:invalidate(name) end
                if self.def.succeeded then self.def.succeeded(result, params, self) end
            end
        end
        self:publish()
        if done then done(current and result ~= nil, result, current and err or { code = "stale" }) end
        local nextItem = table.remove(self.queue, 1)
        if nextItem and not self.disposed then self:execute(nextItem.params, nextItem.done) end
    end
    local function run()
        -- Work queued for a replaced reader or a disposed page never starts.
        if self.disposed or not self.scope:valid(token) then return finish(nil, { code = "stale" }) end
        local called, result, err = xpcall(self.def.operation, debug.traceback, params, self.scope.context, self)
        if not called then
            finish(nil, { code = "error", message = "Something went wrong. Please try again." }, result)
        else
            finish(result, err)
        end
    end
    if self.def.sync then run() else self.scope:run(run) end
    return true
end

-- Re-evaluates CanExecute for the parameters a view last offered.
function Command:refresh(params)
    if params then self.lastParams = params end
    self:publish()
end

function Command:clearError() self.error = nil; self:publish() end

function Command:dispose()
    self.disposed = true
    self.queue = {}
end

return Command
