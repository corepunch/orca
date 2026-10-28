local core = require "orca.core"
local ui = require "orca.UIKit"

local unicode = string.rep("東京 — hello & <world>\n", 300)
local strings = ui.ListBox_SelectionChangedEventArgs {
  SelectedValue = unicode,
  OldValue = "previous",
}
assert(type(strings) == "userdata", "event arguments retain their typed userdata API")
collectgarbage("collect")
assert(strings.SelectedValue == unicode and strings.OldValue == "previous", "constructor owns string fields")
for index = 1, 30 do strings.OldValue = "replacement " .. index end
assert(strings.OldValue == "replacement 30", "owned string fields remain mutable")
strings.OldValue = nil
assert(strings.OldValue == nil, "nil clears an owned string field")
assert(not pcall(function() return strings.UnknownField end), "typed event args reject unknown fields")

local unsigned = core.Node_LeftButtonUpEventArgs { timestamp = 0xffffffff, x = 1.5, scrollPhase = "End" }
assert(unsigned.timestamp == 0xffffffff and unsigned.scrollPhase == "End", "inherited event descriptors preserve unsigned values and enums")
assert(not pcall(function() unsigned.timestamp = -1 end), "unsigned fields reject negative values")
assert(not pcall(function() unsigned.timestamp = 0x100000000 end), "unsigned fields reject overflowing values")
assert(unsigned.timestamp == 0xffffffff, "invalid unsigned assignments preserve the previous value")
assert(not pcall(function() unsigned.scrollPhase = "InvalidPhase" end), "enum fields reject unknown options")
assert(unsigned.scrollPhase == "End", "invalid enum assignments preserve the previous value")

local context = core.DataObject { Name = "Context" }
local componentArgs = ui.NavigationHost_PushEventArgs { Path = "page.xml", DataContext = context }
assert(componentArgs.DataContext == context, "component fields store the declared component ABI and expose their object")
local replacement = core.DataObject { Name = "Replacement" }
componentArgs.DataContext = replacement
assert(componentArgs.DataContext == replacement, "component fields can replace their owned object")
assert(not pcall(function() componentArgs.DataContext = ui.TextBlock {} end), "component fields reject objects without the declared component")
assert(componentArgs.DataContext == replacement, "invalid component assignment preserves the previous owned object")
componentArgs.DataContext = replacement
assert(componentArgs.DataContext == replacement, "assigning the same owned object keeps it alive")
componentArgs.DataContext = nil
assert(componentArgs.DataContext == nil, "component fields can release their owned object")

local frame = core.Keyframe { Value = { X = 1, Y = 2, Z = 3, W = 4 } }
local value = frame.Value
frame.Value = { X = 5, Y = 6, Z = 7, W = 8 }
assert(value.X == 1 and frame.Value.X == 5, "nested reflected structs are independent snapshots")

core.flushQueue()
collectgarbage("collect")
collectgarbage("collect")
local baseline = core.objectCount()
local function ownTemporaryObject()
  return core.Object_AttachedEventArgs { Sender = core.DataObject { Name = "HeldByArgs" } }
end
local owner = ownTemporaryObject()
collectgarbage("collect")
assert(core.objectCount() == baseline + 1 and owner.Sender.Name == "HeldByArgs", "event args retain object fields after the original Lua reference is collected")
owner.Sender = nil
collectgarbage("collect")
assert(core.objectCount() == baseline, "clearing an event field releases the held object")
owner = ownTemporaryObject()
owner = nil
collectgarbage("collect")
collectgarbage("collect")
assert(core.objectCount() == baseline, "collecting event args releases their held objects")

local captured
local target = ui.Node2D {
  LeftButtonUp = function(_, args) captured = args end,
}
target:post("Node.LeftButtonUp", { x = 12, y = 34, timestamp = 0xffffffff })
core.flushQueue()
collectgarbage("collect")
assert(type(captured) == "userdata" and captured.x == 12 and captured.y == 34 and captured.timestamp == 0xffffffff,
  "dispatched arguments remain typed snapshots after queue disposal")

core.flushQueue()
collectgarbage("collect")
collectgarbage("collect")
local beforeMessage = core.objectCount()
assert(not pcall(function()
  target:post("NavigationHost.Push", { Path = unicode, DataContext = ui.TextBlock {} })
end), "message population rejects objects without the required component")
collectgarbage("collect")
collectgarbage("collect")
assert(core.objectCount() == beforeMessage, "invalid message arguments release their temporary action and objects")
local queuedContext = core.DataObject { Name = "Queued component context" }
target:post("NavigationHost.Push", { Path = unicode, DataContext = queuedContext })
queuedContext = nil
collectgarbage("collect")
assert(core.objectCount() == beforeMessage + 1, "queued component fields keep their underlying object alive")
core.flushQueue()
collectgarbage("collect")
collectgarbage("collect")
assert(core.objectCount() == beforeMessage, "dispatch disposes the queued component object reference")

print("All event argument ownership tests passed.")
