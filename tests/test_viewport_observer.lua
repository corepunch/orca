local orca = require "orca"
orca.init()
local ui = require "orca.UIKit"
local Observer = require "orca.UIKit.ViewportObserver"
local root = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
local list = ui.StackView { Overflow = "hidden scroll" }
root:addChild(list)
local frames, events = {}, {}
local observer = Observer.new { root = function() return root end }
for i = 1, 5 do
    local frame = ui.Grid { Height = 100 }
    list:addChild(frame)
    frames[i], events[i] = frame, {}
    observer:observe(frame, function(event) events[i][#events[i] + 1] = event end)
end
observer:update()
assert(not events[1][1].isVisible, "Unlaid-out nodes cannot be visible")
root:UpdateLayout(200, 200)
observer:update()
assert(events[1][2].isVisible and events[2][2].isVisible and #events[3] == 1)
observer:update()
assert(#events[1] == 2, "Unchanged paints must not repeat enter notifications")
local near = Observer.new { root = function() return root end, margin = 64 }
assert(near:isVisible(frames[3]) and not near:isVisible(frames[4]), "Only a small near-viewport band is prefetched")
list.ContentOffset = { X = 0, Y = -100 }
root:UpdateLayout(200, 200)
observer:update()
assert(not events[1][3].isVisible and events[3][2].isVisible, "Scroll transforms produce enter and exit events")
list.Visible = false
observer:update()
assert(not events[3][3].isVisible, "Hidden ancestors suppress visibility")
list.Visible = true
frames[3]:removeFromParent()
observer:update()
assert(events[3][4].detached and observer.entries[frames[3]] == nil)
local retained = root
root = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
local hiddenShell = Observer.new { root = function() return root end, retainedRoot = function() return retained end }
assert(hiddenShell:attached(frames[2]) and not hiddenShell:isVisible(frames[2]), "Retained shells stay registered without loading")
local collected
do
    local frame = ui.Grid { Height = 100 }
    root:addChild(frame)
    observer:observe(frame, function(event) collected = event.isVisible end)
end
collectgarbage("collect")
collectgarbage("collect")
root:UpdateLayout(200, 200)
observer:update()
assert(collected, "Native-owned views must keep their registrations through Lua GC")
observer:clear()
assert(next(observer.entries) == nil)
print("PASS: viewport enter/exit, prefetch band, clipping, hidden/retained views, detach and GC")
