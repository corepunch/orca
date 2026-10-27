-- Two-way bindings declared and serialized in Lisp; tests/test_two_way_bindings.lua
-- covers the format-independent binding cases.
local orca = require "orca"
orca.init()
local core = require "orca.core"
local ui = require "orca.UIKit"
local fs = require "orca.filesystem"
local checks, cases = 0, 0
local function equal(actual, expected, context)
    checks = checks + 1
    assert(actual == expected, (context or "value") .. ": expected " .. tostring(expected) .. ", got " .. tostring(actual))
end
local function check(value, context)
    checks = checks + 1
    assert(value, context)
end
local function collect()
    for _ = 1, 4 do core.flushQueue(); collectgarbage("collect") end
end
local function evaluate(root)
    core.advanceFrame()
    core.runAllPrograms(root)
    core.flushQueue()
end
local function bind(node, destination, path, mode)
    check(node:attachPropertyProgram(destination, path, mode or "TwoWay", true), "Binding must compile: " .. path)
end
local function scenario(name, fn)
    fn()
    collect()
    cases = cases + 1
    print("PASS: MVVM bindings " .. name)
end

-- The schema is the data layout, separate from the controls that edit it.
local schema = [[<Schema><Entity Name="MvvmFormDraft">
    <Column Name="Email" Type="string"/>
    <Column Name="Title" Type="string"/>
    <Column Name="Secret" Type="string"/>
    <Column Name="OptIn" Type="bool"/>
</Entity></Schema>]]
local seed = assert(fs.loadXmlDataStringWithSchema(
    '<MvvmFormDraft Email="reader@example.invalid" Title="XML draft" Secret="initial secret" OptIn="true"/>', schema))
local function draft(email, title, secret, optIn)
    local value = ui.Node2D {}
    value:addComponentByName("MvvmFormDraft")
    value.Email, value.Title, value.Secret = email or "", title or "", secret or ""
    value.OptIn = not not optIn
    return value
end

scenario("Lisp form initialization, editor writeback and one-way display", function()
    local form = assert(fs.loadObjectFromLispString([[(StackView
        (Input Name="EmailEditor" Text="control default"
          (Binding Target="TextRun.Text" Mode="TwoWay" Expression="DataContext/Email"))
        (Input Name="SecretEditor" Type="Password" (Binding Target="TextRun.Text" Mode="TwoWay" "DataContext/Secret"))
        (Input Name="OptInEditor" Type="Checkbox" (Binding Target="Input.Checked" Mode="TwoWay" Expression="DataContext/OptIn"))
        (TextBlock Name="EmailDisplay" (Binding Target="TextRun.Text" Mode="OneWay" "DataContext/Email"))
        (Input Name="DisabledEditor" Text="disabled default"
          (Binding Target="TextRun.Text" Mode="TwoWay" Enabled="false" "DataContext/Title")))]]))
    form.DataContext = seed
    local email, secret = form:findChild("EmailEditor"), form:findChild("SecretEditor")
    local optIn, display = form:findChild("OptInEditor"), form:findChild("EmailDisplay")
    evaluate(form)
    equal(email.Text, "reader@example.invalid", "TwoWay initializes from the model")
    equal(secret.Text, "initial secret", "Password inputs bind their real value")
    equal(optIn.Checked, true, "Typed bool data initializes a checkbox")
    equal(display.Text, seed.Email, "Read-only display shares the same draft")
    local disabled = form:findChild("DisabledEditor")
    equal(disabled.Text, "disabled default", "Enabled=false prevents initial synchronization")
    disabled.Text = "disabled edit"
    core.flushQueue()
    equal(seed.Title, "XML draft", "Disabled bindings cannot write back")
    email.Text, secret.Text = "edited@example.invalid", "typed secret 密碼🙂"
    core.flushQueue()
    equal(seed.Email, "edited@example.invalid", "Text edits write back without controller copying")
    equal(seed.Secret, "typed secret 密碼🙂", "Password writeback preserves the authentication value")
    optIn:send("Node.LeftButtonUp")
    core.flushQueue()
    equal(seed.OptIn, false, "A real checkbox click writes through the property system")
    evaluate(form)
    equal(display.Text, seed.Email, "A second view sees the edited model")
    display.Text = "view-only override"
    core.flushQueue()
    equal(seed.Email, "edited@example.invalid", "OneWay display never writes back")
    seed.Email, seed.Secret, seed.OptIn = "server@example.invalid", "reloaded", true
    evaluate(form)
    equal(email.Text, seed.Email, "Programmatic draft changes update the editor")
    equal(secret.Text, "reloaded", "Programmatic password replacement remains bound")
    equal(optIn.Checked, true, "Model bool changes update the checkbox")
end)

scenario("bind shorthand with modes on property values", function()
    local form = assert(fs.loadObjectFromLispString([[(StackView
        (Input Name="Positional" Text=(bind "DataContext/Email" Mode=TwoWay))
        (Input Name="NamedPath" Text=(bind "DataContext/Title" Mode="TwoWay"))
        (Input Name="WriteOnly" Text=(bind "DataContext/Secret" Mode=OneWayToSource))
        (TextBlock Name="DefaultDisplay" Text=(bind "DataContext/Email")))]]))
    local model = draft("email from draft", "title from draft", "keep secret")
    form.DataContext = model
    evaluate(form)
    local positional, named = form:findChild("Positional"), form:findChild("NamedPath")
    local writer, display = form:findChild("WriteOnly"), form:findChild("DefaultDisplay")
    equal(positional.Text, "email from draft", "bind with a mode resolves its path")
    equal(named.Text, "title from draft", "A quoted mode name resolves the same source mechanism")
    equal(display.Text, model.Email, "Unspecified mode remains one-way display binding")
    equal(model.Secret, "keep secret", "OneWayToSource shorthand does not overwrite model initialization")
    positional.Text, named.Text, writer.Text = "email edited", "title edited", "secret edited"
    core.flushQueue()
    equal(model.Email, "email edited", "bind Mode=TwoWay enables writeback")
    equal(model.Title, "title edited", "A quoted TwoWay mode enables writeback")
    equal(model.Secret, "secret edited", "OneWayToSource shorthand writes actual edits")
    display.Text = "display override"
    core.flushQueue()
    equal(model.Email, "email edited", "Default shorthand cannot write back")
end)

scenario("Lisp save and reload preserve binding modes and enabled state", function()
    local original = ui.StackView {}
    local editor = ui.Input { Name = "EditRoundtrip" }
    local writer = ui.Input { Name = "WriteRoundtrip", Text = "writer initial" }
    local disabled = ui.Input { Name = "DisabledRoundtrip", Text = "disabled initial" }
    original:addChild(editor); original:addChild(writer); original:addChild(disabled)
    bind(editor, "TextRun.Text", "{DataContext/Email}")
    bind(writer, "TextRun.Text", "{DataContext/Secret}", "OneWayToSource")
    check(disabled:attachPropertyProgram("TextRun.Text", "{DataContext/Title}", "TwoWay", false),
        "Disabled TwoWay binding can retain its expression for serialization")
    local lisp = assert(fs.serializeObjectToLispString(original))
    check(lisp:find('Mode="TwoWay"', 1, true), "Serialized binding includes TwoWay mode")
    check(lisp:find('Mode="OneWayToSource"', 1, true), "Serialized binding includes write-only mode")
    check(lisp:find('Enabled="false"', 1, true), "Serialized binding includes the disabled state")
    local restored = assert(fs.loadObjectFromLispString(lisp))
    original, lisp = nil, nil
    collect()
    local model = draft("restored email", "restored title", "restored secret")
    restored.DataContext = model
    evaluate(restored)
    local edit = assert(restored:findChild("EditRoundtrip"))
    local write = assert(restored:findChild("WriteRoundtrip"))
    local off = assert(restored:findChild("DisabledRoundtrip"))
    equal(edit.Text, "restored email", "Reloaded TwoWay binding initializes from the draft")
    equal(write.Text, "writer initial", "Reloaded OneWayToSource preserves its control state")
    equal(model.Secret, "restored secret", "Reloaded write-only binding preserves model initialization")
    equal(off.Text, "disabled initial", "Reloaded disabled binding does not pull")
    edit.Text, write.Text, off.Text = "restored edit", "restored write", "must not write"
    core.flushQueue()
    equal(model.Email, "restored edit", "Reloaded TwoWay retains writeback")
    equal(model.Secret, "restored write", "Reloaded OneWayToSource retains writeback")
    equal(model.Title, "restored title", "Reloaded disabled binding does not write back")
end)

scenario("OneWayToSource does not pull or erase model initialization", function()
    local form = assert(fs.loadObjectFromLispString([[(StackView
        (Input Name="Writer" Text="authored input" (Binding Target="TextRun.Text" Mode="OneWayToSource" "DataContext/Email")))]]))
    local model = draft("keep model", "", "", false)
    form.DataContext = model
    local writer = form:findChild("Writer")
    evaluate(form)
    equal(writer.Text, "authored input", "OneWayToSource never pulls from the model")
    equal(model.Email, "keep model", "Binding activation must not send an unset/default control into the draft")
    writer.Text = "explicit edit"
    core.flushQueue()
    equal(model.Email, "explicit edit", "An actual editor edit writes to the source")
    model.Email = "model replacement"
    evaluate(form)
    equal(writer.Text, "explicit edit", "Later model changes also do not pull")
    writer.Text = "next edit"
    core.flushQueue()
    equal(model.Email, "next edit", "Writeback remains active after model updates")
end)

print(("PASS: %d MVVM binding scenarios, %d assertions"):format(cases, checks))
