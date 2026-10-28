require("orca").init()
local core = require "orca.core"
local fs = require "orca.filesystem"
local system = require "orca.system"
local ui = require "orca.UIKit"

local function pump(root)
    while true do
        local message = system.peekMessage()
        if not message then break end
        system.dispatchMessage(root, message)
    end
    core.flushQueue()
end

local controller = "tests/fixtures/controllers/test_ctrl_roundtrip"
-- Attribute order must not decide whether the controller exists at bind time.
local source = fs.getSceneExtension() == ".xml"
    and '<Input Name="Editor" Submit="CustomSubmit" Clear="CustomClear" Controller="' .. controller .. '"/>'
    or '(Input Name="Editor" Submit="CustomSubmit" Clear="CustomClear" Controller="' .. controller .. '")'
local original = assert(fs.loadObjectFromString(source))
local serialized = fs.serializeObjectToString(original)
assert(serialized:find(controller, 1, true) and serialized:find("CustomSubmit", 1, true),
    "Pending controller/event declarations must serialize before dispatch")
local loaded = assert(fs.loadObjectFromString(serialized))
local cloned = original:instantiate()
pump(original)
for _, root in ipairs { original, loaded, cloned } do
    root:post("Input.Submit", { Text = "payload" })
    pump(root)
    assert(root.Text == "payload:Editor", "Round-tripped event uses its own controller and sender")
    local text = fs.serializeObjectToString(root)
    assert(text:find("CustomSubmit", 1, true), "Bound declarations must survive serialization")
end

local pending = assert(fs.loadObjectFromString(source))
pending.Submit = function(self) self.Text = "runtime" end
pump(pending)
pending:post("Input.Submit", { Text = "ignored" })
pump(pending)
assert(pending.Text == "runtime", "A runtime handler overrides pending named/convention wiring")
pending:post("Input.Clear")
pump(pending)
assert(pending.Text == "cleared", "Replacing one handler preserves unrelated pending declarations")
assert(not fs.serializeObjectToString(pending):find("CustomSubmit", 1, true),
    "Runtime closures must not serialize stale declarations or registry references")
pending.Submit = "CustomSubmit"
pump(pending)
pending:post("Input.Submit", { Text = "rebound" })
pump(pending)
assert(pending.Text == "rebound:Editor", "Lua can rebind a controller function by name")
local callbacks = setmetatable({}, { __mode = "v" })
for i = 1, 20 do
    local captured = {}
    callbacks[i] = captured
    pending.Submit = function() return captured end
end
pending.Submit = nil
collectgarbage("collect")
assert(next(callbacks) == nil, "Replacing/clearing handlers must release their previous Lua closures")

local cleared = assert(fs.loadObjectFromString(source))
cleared.Text = "untouched"
cleared.Submit = nil
pump(cleared)
cleared:post("Input.Submit", { Text = "ignored" })
pump(cleared)
assert(cleared.Text == "untouched", "Clearing a pending named handler must prevent later rebinding")
for _, root in ipairs { cleared:instantiate(), assert(fs.loadObjectFromString(fs.serializeObjectToString(cleared))) } do
    pump(root)
    root:post("Input.Submit", { Text = "ignored" })
    pump(root)
    assert(root.Text == "untouched", "An explicitly cleared declaration must survive cloning and reload")
end

local scripted = ui.Input { Name = "Programmatic" }
scripted:loadController(controller)
scripted.Submit = "CustomSubmit"
pump(scripted)
local scriptedClone = scripted:instantiate()
pump(scriptedClone)
scriptedClone:post("Input.Submit", { Text = "scripted" })
pump(scriptedClone)
assert(scriptedClone.Text == "scripted:Programmatic", "Programmatic controller paths must survive cloning")
scripted.Submit = function(self) self.Text = "stale" end
scripted.Submit = "MissingHandler"
pump(scripted)
scripted.Text = "unchanged"
scripted:post("Input.Submit", { Text = "ignored" })
pump(scripted)
assert(scripted.Text == "unchanged", "A missing named replacement must not invoke the previous callback")

local action = assert(fs.loadObjectFromString(fs.getSceneExtension() == ".xml"
    and '<Input.Submit/>' or '(Input.Submit)'))
local payload = ("Привет <&> \"quoted\"\n"):rep(1024)
action.Text = payload
local restored = assert(fs.loadObjectFromString(fs.serializeObjectToString(action)))
assert(restored.Text == payload, "Message action strings must round-trip without truncation or escaping loss")
action.Text = ""
restored = assert(fs.loadObjectFromString(fs.serializeObjectToString(action)))
assert(restored.Text == "", "An explicitly empty message string must survive serialization")
local mouse = assert(fs.loadObjectFromString(fs.getSceneExtension() == ".xml"
    and '<Node.RightButtonUp/>' or '(Node.RightButtonUp)'))
mouse.timestamp = 0xffffffff
local mouseClone = mouse:instantiate()
assert(mouseClone.timestamp == 0xffffffff, "Unsigned message fields must preserve their full range")

print("PASS: event/controller serialization, cloning, attribute order and handler replacement")
