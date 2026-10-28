require("orca").init()
require "orca.filesystem"
local core = require "orca.core"
local ui = require "orca.UIKit"
local ID_Input_Submit = 0xbbc22157 -- generated Input.Submit identifier
local controller = "tests/fixtures/controllers/test_ctrl"

local function flush() core.flushQueue() end
local function input(name)
  return ui.Input { Name = name or "Editor", Text = "unchanged" }
end
local function submit(view, text)
  _G.__controllerProbe = nil
  view:post("Input.Submit", { Text = text or "payload" })
  flush()
  return _G.__controllerProbe
end
local function bind(view, name)
  view:post("Object.BindHandler", { PropertyId = ID_Input_Submit, FunctionName = name })
  flush()
end
local function expect_probe(probe, view, sender, version, text)
  assert(probe, "Controller callback must run")
  assert(probe.view == view and probe.controller.view == view,
    "Controller self.view must identify the object that loaded it")
  assert(probe.sender == sender and probe.text == text,
    "Callback must receive the original sender and typed event payload")
  assert(probe.version == version, "Expected controller implementation must run")
end

local direct = input()
direct:loadController(controller .. "1")
flush()
expect_probe(submit(direct, "direct"), direct, direct, 1, "direct")
assert(direct.Text == "one:direct", "Direct loading wires the root event convention")
print("PASS: direct controller loading invokes a root convention handler")

local posted = input()
posted:post("Object.LoadController", { Path = controller .. "1" })
flush()
expect_probe(submit(posted, "posted"), posted, posted, 1, "posted")
assert(posted.Text == "one:posted")
print("PASS: queued LoadController loads and wires the controller")

local named = input()
named:loadController(controller .. "1")
flush()
named.Submit = nil
bind(named, "on_submit")
expect_probe(submit(named, "named"), named, named, 1, "named")
assert(named.Text == "one:named")
print("PASS: queued BindHandler resolves an explicit function")

local parent = core.Node { Name = "Parent" }
local child = input("MyInput")
parent:addChild(child)
parent:loadController(controller .. "1")
flush()
bind(child, "on_submit")
expect_probe(submit(child, "ancestor"), parent, child, 1, "ancestor")
assert(child.Text == "one:ancestor")
print("PASS: named handlers find an ancestor controller and preserve sender/view identity")

local wired = core.Node { Name = "Target" }
local wiredInput = input("MyInput")
wired:addChild(wiredInput)
wired:loadController(controller .. "_wired")
flush()
local first = submit(wiredInput, "first")
expect_probe(first, wired, wiredInput, "wired", "first")
local second = submit(wiredInput, "second")
expect_probe(second, wired, wiredInput, "wired", "second")
assert(first.controller == second.controller and second.calls == first.calls + 1,
  "Convention callbacks share their bound controller instance across invocations")
assert(wiredInput.Text == "wired:second")
print("PASS: descendant convention wiring binds the controller closure")

local reloaded = input()
reloaded:loadController(controller .. "1")
flush()
local previous = submit(reloaded, "before")
expect_probe(previous, reloaded, reloaded, 1, "before")
reloaded:loadController(controller .. "2")
flush()
local replacement = submit(reloaded, "after")
expect_probe(replacement, reloaded, reloaded, 2, "after")
assert(replacement.controller ~= previous.controller and replacement.calls == 1,
  "Reloading replaces the old controller and handler closure")
assert(reloaded.Text == "two:after")
reloaded:loadController("")
flush()
expect_probe(submit(reloaded, "kept"), reloaded, reloaded, 2, "kept")
assert(reloaded.Text == "two:kept", "An empty controller path leaves the current controller usable")
print("PASS: controller reload replaces callbacks; an empty load preserves the current controller")

local explicit = input()
explicit:loadController(controller .. "1")
flush()
bind(explicit, "on_submit")
local oldNamed = submit(explicit, "old named")
expect_probe(oldNamed, explicit, explicit, 1, "old named")
explicit:loadController(controller .. "2")
flush()
local newNamed = submit(explicit, "new named")
expect_probe(newNamed, explicit, explicit, 2, "new named")
assert(newNamed.controller ~= oldNamed.controller and newNamed.calls == 1,
  "Reloading must replace an explicit named closure bound to the previous controller")
assert(explicit.Text == "two:new named")
print("PASS: explicit named handlers resolve the replacement controller on reload")

local lateParent = core.Node { Name = "LateParent" }
local lateChild = input("LateChild")
lateParent:addChild(lateChild)
lateChild.Submit = "on_submit"
flush()
assert(not submit(lateChild) and lateChild.Text == "unchanged",
  "A named declaration waits until a controller can resolve it")
lateParent:loadController(controller .. "1")
flush()
expect_probe(submit(lateChild, "late ancestor"), lateParent, lateChild, 1, "late ancestor")
assert(lateChild.Text == "one:late ancestor")
print("PASS: pending child declarations resolve when an ancestor controller loads")

for _, missing in ipairs { "nonexistent_function", "" } do
  bind(named, "on_submit")
  expect_probe(submit(named, "active"), named, named, 1, "active")
  bind(named, missing)
  named.Text = "unchanged"
  assert(not submit(named), "Missing or empty named replacements must remove the old callback")
  assert(named.Text == "unchanged")
end
bind(named, "on_submit")
assert(submit(named), "Cleared handlers can be rebound")
named.Submit = nil
named.Text = "unchanged"
assert(not submit(named) and named.Text == "unchanged", "Explicit nil clears the callback")
local unbound = input()
bind(unbound, "on_submit")
assert(not submit(unbound) and unbound.Text == "unchanged", "Binding without a controller invokes no callback")
print("PASS: missing, empty and cleared handlers stop callbacks; absent controllers remain unbound")

_G.__controllerProbe = nil
print("All controller behavior tests passed.")
