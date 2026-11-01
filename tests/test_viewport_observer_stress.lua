-- Offline geometry doubles let the 10,000-registration benchmark run without
-- creating windows or downloading avatars. Native geometry is covered by
-- test_viewport_observer.lua.
package.path = "./share/?.lua;./share/?/init.lua;" .. package.path
local Observer = require "orca.UIKit.ViewportObserver"

local root = { ActualWidth = 360, ActualHeight = 720 }
function root:getParent() return nil end
local list = { ActualWidth = 360, ActualHeight = 720, OverflowY = "Scroll",
    ContentOffset = { X = 0, Y = 0 } }
function list:getParent() return root end
local observer = Observer.new { root = function() return root end }
local frames, visible, calls, last = {}, {}, 0, 0
local count = tonumber(os.getenv("ORCA_STRESS_ROWS")) or 10000
assert(count >= 100, "Stress fixture needs at least 100 rows")
for i = 1, count do
    local frame = { ActualWidth = 360, ActualHeight = 72, ActualY = (i - 1) * 72 }
    function frame:getParent() return self.parent end
    frame.parent, frames[i] = list, frame
    observer:observe(frame, function(event)
        calls = calls + 1
        if calls <= count then
            assert(i > last, "Initial notifications follow registration order")
            last = i
        end
        visible[i] = event.isVisible
    end)
end
observer:update()
assert(calls == count and visible[1] and visible[10] and not visible[11])
local started = os.clock()
collectgarbage("collect")
collectgarbage("stop")
local memory = collectgarbage("count")
for page = 1, 30 do
    list.ContentOffset.Y = -((page * 31) % (count - 10)) * 72
    observer:update()
end
local allocated = collectgarbage("count") - memory
collectgarbage("restart")
assert(allocated < 2048, "Scroll observation must not allocate geometry tables for every retained row")
print(string.format("viewport stress: %d rows, 30 scroll updates %.3fs, %.0f KiB allocated",
    count, os.clock() - started, allocated))
local first = ((30 * 31) % (count - 10)) + 1
for i = 1, count do assert(visible[i] == (i >= first and i < first + 10)) end
local stable = calls
observer:update()
assert(calls == stable, "Stationary frames must not restart completed work")
for i = 1, count do frames[i].parent = nil end
observer:update()
assert(next(observer.entries) == nil, "Detached history must release registrations")

-- Callbacks can cancel, replace, clear and register observers while dispatching.
local changing = Observer.new { root = function() return root end }
local a, b, c = frames[1], frames[2], frames[3]
a.parent, b.parent, c.parent = root, root, root
local events, cancel = {}, nil
changing:observe(a, function()
    events[#events + 1] = "a"
    cancel()
    changing:observe(c, function() events[#events + 1] = "c" end)
end)
cancel = changing:observe(b, function() error("Cancelled callbacks cannot run") end)
changing:update()
assert(table.concat(events) == "a", "Registrations made during dispatch wait for the next frame")
changing:update()
assert(table.concat(events) == "ac")
changing:observe(a, function()
    changing:clear()
    changing:observe(b, function() events[#events + 1] = "b" end)
end)
changing:observe(c, function() error("Cleared callbacks cannot run") end)
changing:update()
changing:update()
assert(table.concat(events) == "acb")
changing:clear()
local weak = setmetatable({}, { __mode = "v" })
do
    local frame = { ActualWidth = 10, ActualHeight = 10 }
    weak[1] = frame
    local cancel = changing:observe(frame, function() end)
    cancel()
end
collectgarbage("collect")
assert(weak[1] == nil, "Unsubscribing releases nodes without waiting for another paint")
print("PASS: large observer scroll, stable frames, detach and callback mutation")
