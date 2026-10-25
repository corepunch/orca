package.path = "apps/chat/?.lua;" .. package.path
local Polling = require "polling"
local checks = 0
local function check(value, message)
    assert(value, message)
    checks = checks + 1
end
local function fixture()
    local timers = {}
    local poll = Polling.new { createTimer = function(callback, delay)
        local timer = { callback = callback, delay = delay }
        function timer:cancel() self.canceled = true end
        timers[#timers + 1] = timer
        return timer
    end }
    local function latest() return timers[#timers] end
    local function fire(timer)
        local co = coroutine.create(timer.callback)
        local ok, err = coroutine.resume(co)
        assert(ok, err)
        return co
    end
    return poll, timers, latest, fire
end

local poll, timers, latest, fire = fixture()
local calls = 0
local job = { interval = 3000, valid = function() return true end, run = function(current)
    check(current(), "An active request must be current")
    calls = calls + 1
    return true
end }
poll:start(job)
check(latest().delay == 3000 and calls == 0, "Thread polling waits three seconds after initial fetch")
local initial = latest()
fire(initial)
check(initial.canceled and calls == 1 and latest().delay == 3000, "Completion schedules one next timer")
fire(initial)
check(calls == 1 and #timers == 2, "A queued duplicate event must not refetch")
poll:stop()
check(latest().canceled, "Stop cancels the object-owned timer")
fire(latest())
check(calls == 1, "Canceled work must not run")
poll:start({ interval = 15000, run = function() calls = calls + 1; return true end })
check(latest().delay == 15000, "Inbox polling uses fifteen seconds")

local success = false
poll:start({ interval = 3000, run = function() return success end })
for _, delay in ipairs { 6000, 12000, 24000, 48000, 60000, 60000 } do
    fire(latest())
    check(latest().delay == delay, "Failure delay must back off and remain bounded")
end
success = true
fire(latest())
check(latest().delay == 3000, "Successful polling restores normal cadence")
poll:start({ interval = 3000, run = function() error("request failed") end })
fire(latest())
check(not poll.running and latest().delay == 6000, "Thrown requests must release the lock and back off")

poll, timers, latest, fire = fixture()
local resumedCurrent, active = nil, true
poll:start({ interval = 3000, valid = function() return active end, run = function(current)
    coroutine.yield()
    resumedCurrent = current()
    return true
end })
local inflight = fire(latest())
check(poll.running and latest().canceled and #timers == 1, "A yielding request must have no next timer yet")
poll:refresh()
poll:refresh()
check(#timers == 1, "Refresh during a request must not overlap it")
assert(coroutine.resume(inflight))
check(resumedCurrent and not poll.running and latest().delay == 1 and #timers == 2,
    "Pending refreshes coalesce into one immediate request after completion")

inflight = fire(latest())
poll:setForeground(false)
check(poll.running and latest().canceled, "Backgrounding keeps the request lock while canceling future work")
assert(coroutine.resume(inflight))
check(not resumedCurrent and #timers == 2, "A backgrounded completion is stale and cannot schedule more work")
poll:setForeground(true)
check(latest().delay == 1 and #timers == 3, "Foreground resumes with an immediate refresh")
poll:setForeground(true)
check(#timers == 3, "Duplicate foreground notifications do not add timers")

inflight = fire(latest())
local secondCalls = 0
poll:stop()
poll:start({ interval = 15000, run = function(current)
    check(current(), "The replacement job must become current")
    secondCalls = secondCalls + 1
    return true
end })
check(#timers == 3 and secondCalls == 0, "Navigating must wait for the old in-flight request")
assert(coroutine.resume(inflight))
check(not resumedCurrent and latest().delay == 1, "Old route results must be stale before the new route refresh")
fire(latest())
check(secondCalls == 1 and latest().delay == 15000, "Replacement job keeps its own normal interval")

poll:start({ interval = 3000, valid = function() return active end, run = function() error("must not run") end })
active = false
local count = #timers
fire(latest())
check(#timers == count and not poll.running, "Lost session/context stops polling without requests")
poll:refresh()
check(#timers == count, "Manual refresh also requires a valid context")

poll, timers, latest, fire = fixture()
poll:setForeground(false)
poll:start(job, true)
check(#timers == 0, "A new route cannot start polling while backgrounded")
poll:setForeground(true)
check(latest().delay == 1, "A route selected in the background refreshes on resume")
poll:stop()
poll:setForeground(false)
poll:setForeground(true)
check(#timers == 1, "Resume after sign-out cannot restart a stopped job")

poll, timers, latest, fire = fixture()
local oldCurrent
poll:start({ interval = 3000, run = function(current)
    coroutine.yield()
    oldCurrent = current()
    return nil, { code = "network" }
end })
inflight = fire(latest())
poll:setForeground(false)
poll:setForeground(true)
check(#timers == 1 and poll.running, "Resume must wait for a request suspended before backgrounding")
assert(coroutine.resume(inflight))
check(not oldCurrent and #timers == 2 and latest().delay == 1,
    "A pre-background response stays stale after resume; schedule only one catch-up")
inflight = fire(latest())
poll:stop()
assert(coroutine.resume(inflight))
check(not oldCurrent and #timers == 2 and not poll.running,
    "A stopped yielding request cannot resurrect a timer on completion")

print("PASS: Chat polling (" .. checks .. " checks)")
