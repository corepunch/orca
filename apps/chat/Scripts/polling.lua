-- Scheduling only: the app supplies an object-owned, one-shot ORCA timer.
-- Timer callbacks must run in a yieldable event coroutine, like other UI work.
local Polling = {}
Polling.__index = Polling

function Polling.new(options)
    assert(type(options.createTimer) == "function", "Polling requires a timer factory")
    return setmetatable({ createTimer = options.createTimer, onError = options.onError, foreground = true,
        generation = 0, failures = 0, maxDelay = options.maxDelay or 60000 }, Polling)
end

function Polling:_active()
    return self.foreground and self.job and (not self.job.valid or self.job.valid())
end

function Polling:_cancelTimer()
    local timer = self.timer
    self.timer, self.ticket = nil, nil
    if timer then timer:cancel() end
end

function Polling:_arm(delay)
    if not self:_active() or self.running then return end
    self:_cancelTimer()
    local ticket = {}
    self.ticket = ticket
    self.timer = self.createTimer(function()
        -- A canceled timer may already have queued an event. Never run it.
        if self.ticket ~= ticket then return end
        self:_cancelTimer()
        self:_tick()
    end, math.max(1, delay))
end

function Polling:_tick()
    if self.running or not self:_active() then return end
    local job, generation = self.job, self.generation
    local function isCurrent()
        return self.job == job and self.generation == generation and self:_active() == true
    end
    self.running, self.requested = true, false
    -- Lua's protected call is yieldable. The lock covers the whole request,
    -- including network yields, rather than only the timer event dispatch.
    local ok, success = xpcall(job.run, debug.traceback, isCurrent)
    self.running = false
    if not ok and isCurrent() and self.onError then self.onError(success) end
    if not self:_active() then return end
    if not isCurrent() or self.requested then
        self.requested = false
        self:_arm(1)
    else
        self.failures = ok and success and 0 or math.min(self.failures + 1, 8)
        self:_arm(math.min(self.maxDelay, job.interval * 2 ^ self.failures))
    end
end

function Polling:start(job, immediate)
    assert(type(job) == "table" and type(job.run) == "function"
        and type(job.interval) == "number" and job.interval > 0, "Invalid polling job")
    self:_cancelTimer()
    self.generation = self.generation + 1
    self.job, self.failures, self.requested = job, 0, immediate == true
    self:_arm(immediate and 1 or job.interval)
end

function Polling:stop()
    self:_cancelTimer()
    self.generation = self.generation + 1
    self.job, self.failures, self.requested = nil, 0, false
    -- Do not release a yielding request's lock. A new job must wait for it.
end

function Polling:setForeground(foreground)
    foreground = foreground == true
    if self.foreground == foreground then return end
    self:_cancelTimer()
    self.foreground = foreground
    self.generation = self.generation + 1
    self.failures, self.requested = 0, foreground
    if foreground then self:_arm(1) end
end

function Polling:refresh()
    if not self:_active() then return end
    self.requested = true
    self:_arm(1)
end

return Polling
