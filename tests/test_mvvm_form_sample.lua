local orca = require "orca"
orca.init()
local core = require "orca.core"
local fs = require "orca.filesystem"
require "orca.UIKit"
local renderer = require "orca.renderer"
local native = os.getenv("ORCA_MVVM_NATIVE") == "1"
if native then renderer.init(390, 844, true) end
fs.init("apps/MvvmForms")
local Application = require "orca.core.application"
local App = require "MvvmForms/App"
local app = App()
Application.app = app
local function drain()
    core.flushQueue()
    while true do
        local event = orca.system.peekMessage()
        if not event then break end
        if not event:is("Window.Paint") then orca.system.dispatchMessage(app.form, event) end
    end
    core.advanceFrame()
    core.runAllPrograms(app.form)
    core.flushQueue()
end
local function control(name) return assert(app.form:findChild(name, true), name) end
local function click(name) control(name):send("Node.LeftButtonUp"); drain() end
drain()
app.form:UpdateLayout(390, 844)
assert(control("Email").Text == "sam@example.invalid")
assert(control("DisplayName").Text == app.committed.DisplayName)
assert(control("Email").ActualWidth > 250 and control("Email").ActualHeight >= 48)
control("DisplayName").Text = "A new draft name"
control("Email").Text = "new@example.invalid"
drain()
assert(app.draft.Email == "new@example.invalid" and app.committed.Email == "sam@example.invalid")
assert(control("Preview").Text == "A new draft name")
click("Cancel")
assert(control("Email").Text == "sam@example.invalid")
assert(control("DisplayName").Text == "Sam Rivera")
control("Email").Text = "invalid address"
click("Save")
assert(app.committed.Email == "sam@example.invalid")
assert(control("Status").Text == "Enter a valid email address before saving.")
control("Email").Text = "saved@example.invalid"
control("DisplayName").Text = "Saved name"
click("OptIn")
local optIn = app.draft.OptIn
click("Save")
assert(app.committed.Email == "saved@example.invalid" and app.committed.OptIn == optIn)
assert(control("Status").Text == "Saved for this session.")
control("DisplayName"):send("Input.Clear")
drain()
assert(app.draft.DisplayName == "", "Input.Clear must update the bound draft")
click("Save")
assert(app.committed.DisplayName == "Saved name", "Invalid drafts cannot overwrite committed data")
click("Cancel")
assert(control("DisplayName").Text == "Saved name")
app.form:UpdateLayout(390, 844)
if native then app.form:send("Screen.RenderScreen", { width = 390, height = 844 }) end
print("PASS: XML MVVM sample loads typed data, edits only its draft, validates Save, and discards changes through real buttons")
