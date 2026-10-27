-- Scene-defined draft state and form controls share the core property binding
-- mechanism. Controllers do not copy values out of named controls.
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

scenario("object-valued property traversal follows replacement models", function()
    local root, editor = ui.StackView {}, ui.Input {}
    local first, second = draft("first object"), draft("replacement object")
    root:addChild(editor)
    root.DataContext = first
    bind(editor, "TextRun.Text", "{../DataContext/Email}")
    evaluate(root)
    equal(editor.Text, "first object", "Object property traversal reads the model")
    editor.Text = "edited object"
    equal(first.Email, "edited object", "Object property traversal writes the model")
    root.DataContext = second
    evaluate(root)
    equal(editor.Text, "replacement object", "Traversal resolves the replacement object")
    editor.Text = "edited replacement"
    equal(second.Email, "edited replacement", "Writeback follows the replacement object")
    equal(first.Email, "edited object", "Writeback leaves the old object alone")
    root.DataContext = nil
    editor.Text = "detached"
    equal(second.Email, "edited replacement", "Null object traversal does not write")
    bind(editor, "TextRun.Text", "{../Node.DataContext/Email}")
    root.DataContext = first
    evaluate(root)
    equal(editor.Text, "edited object", "Qualified object property traversal reads the model")
    editor.Text = "qualified edit"
    equal(first.Email, "qualified edit", "Qualified object property traversal writes the model")
end)

scenario("native keyboard edits update the draft", function()
    local screen = ui.Screen { Width = 390, Height = 844, ResizeMode = "NoResize" }
    local root, editor, model = ui.StackView {}, ui.Input {}, draft("")
    screen:addChild(root); root:addChild(editor); root.DataContext = model
    bind(editor, "TextRun.Text", "{DataContext/Email}")
    evaluate(root)
    screen:UpdateLayout(390, 844)
    editor:setFocus()
    for _, value in ipairs { "a", "é", "🙂" } do
        orca.system.dispatchMessage { target = screen, message = "Char", text = value }
    end
    core.flushQueue()
    equal(editor.Text, "aé🙂", "Native text input preserves complete UTF-8 characters")
    equal(model.Email, "aé🙂", "Native input takes the same property writeback path")
    orca.system.dispatchMessage { target = screen, message = "KeyDown", key = 127 }
    core.flushQueue()
    equal(editor.Text, "aé", "Backspace removes one full character")
    equal(model.Email, "aé", "Native deletion updates the draft as well")
end)

scenario("unresolved context recovers when a draft is supplied", function()
    local root = ui.StackView {}
    local editor = ui.Input { Text = "local placeholder" }
    root:addChild(editor)
    bind(editor, "TextRun.Text", "{DataContext/Email}")
    evaluate(root)
    editor.Text = "edit while no context exists"
    core.flushQueue()
    local model = draft("late model", "", "", false)
    root.DataContext = model
    evaluate(root)
    equal(editor.Text, "late model", "Missing DataContext must defer binding without detaching it")
    editor.Text = "resolved edit"
    core.flushQueue()
    equal(model.Email, "resolved edit", "Recovered binding is writable")
    root.DataContext = nil
    editor.Text = "another unresolved edit"
    core.flushQueue()
    equal(model.Email, "resolved edit", "Clearing context cannot mutate the detached model")
    root.DataContext = draft("new model", "", "", false)
    evaluate(root)
    equal(editor.Text, "new model", "A later model can reuse the same binding")
end)

scenario("nearest DataContext, replacement and reparenting", function()
    local first, second = ui.StackView {}, ui.StackView {}
    local group, editor = ui.StackView {}, ui.Input {}
    local a, b, localDraft = draft("first"), draft("second"), draft("local")
    first.DataContext, second.DataContext = a, b
    first:addChild(group); group:addChild(editor)
    bind(editor, "TextRun.Text", "{DataContext/Email}")
    evaluate(first)
    equal(editor.Text, "first", "An editor inherits its root draft")
    group.DataContext = localDraft
    evaluate(first)
    equal(editor.Text, "local", "The nearest draft wins")
    editor.Text = "local edit"
    core.flushQueue()
    equal(localDraft.Email, "local edit", "Writeback uses the nearest draft")
    equal(a.Email, "first", "The ancestor model is untouched by a local override")
    group.DataContext = nil
    evaluate(first)
    equal(editor.Text, "first", "Clearing the override restores inherited data")
    second:addChild(group)
    evaluate(second)
    equal(editor.Text, "second", "Reparenting re-resolves inherited state")
    first = nil
    collect()
    editor.Text = "second edit"
    core.flushQueue()
    equal(b.Email, "second edit", "Writeback uses the current parent after old-parent collection")
    equal(a.Email, "first", "No stale write reaches the former draft")
    local replacement = draft("replacement")
    second.DataContext = replacement
    evaluate(second)
    editor.Text = "replacement edit"
    core.flushQueue()
    equal(replacement.Email, "replacement edit", "Replacing DataContext re-resolves the writable source")
    equal(b.Email, "second edit", "The replaced model remains unchanged")
    a, b, localDraft = nil, nil, nil
    collect()
    editor.Text = "after former models collected"
    core.flushQueue()
    equal(replacement.Email, "after former models collected", "Writeback cannot retain or dereference a collected old source")
end)

scenario("single-path peer binding and multiple views share a model", function()
    local root = ui.StackView {}
    local model = draft("seed")
    root.DataContext = model
    local a, b, mirrored = ui.Input { Name = "A" }, ui.Input { Name = "B" }, ui.Input { Name = "Mirrored" }
    root:addChild(a); root:addChild(b); root:addChild(mirrored)
    bind(a, "TextRun.Text", "{DataContext/Email}")
    bind(b, "TextRun.Text", "{DataContext/Email}")
    bind(mirrored, "TextRun.Text", "{../A/Text}")
    evaluate(root)
    equal(a.Text, "seed"); equal(b.Text, "seed"); equal(mirrored.Text, "seed")
    mirrored.Text = "peer edit"
    core.flushQueue()
    equal(a.Text, "peer edit", "A writable relative path updates its peer property")
    equal(model.Email, "peer edit", "Writeback can travel through another writable binding")
    evaluate(root)
    equal(b.Text, "peer edit", "Other controls observe the model without a feedback loop")
    for i = 1, 250 do
        local value = "round-trip " .. i .. " 漢🙂"
        a.Text = value
        core.flushQueue()
        equal(model.Email, value)
        evaluate(root)
        equal(a.Text, value); equal(b.Text, value); equal(mirrored.Text, value)
        model.Email = "from model " .. i
        evaluate(root)
        equal(a.Text, model.Email); equal(b.Text, model.Email); equal(mirrored.Text, model.Email)
    end
end)

scenario("rebinding, explicit detachment and rejection of non-writable expressions", function()
    local root, editor = ui.StackView {}, ui.Input {}
    local model = draft("email", "title")
    root.DataContext = model; root:addChild(editor)
    bind(editor, "TextRun.Text", "{DataContext/Email}")
    evaluate(root)
    check(not editor:attachPropertyProgram("TextRun.Text", '"prefix:" + {DataContext/Email}', "TwoWay", true),
        "TwoWay must reject reverse evaluation of concatenation")
    check(not editor:attachPropertyProgram("TextRun.Text", 'STRING({DataContext/Email})', "OneWayToSource", true),
        "OneWayToSource requires a writable path too")
    local modeOk, modeResult = pcall(function()
        return editor:attachPropertyProgram("TextRun.Text", "{DataContext/Title}", "UnsupportedBindingMode", true)
    end)
    check(not modeOk or not modeResult, "Invalid binding modes must be rejected")
    check(not editor:attachPropertyProgram("TextRun.Text", "1 + 2", "TwoWay", true),
        "A constant arithmetic expression is not a writable source")
    model.Email = "still email"
    evaluate(root)
    equal(editor.Text, "still email", "Rejected rebinding preserves the previous working binding")
    bind(editor, "TextRun.Text", "{DataContext/Title}")
    evaluate(root)
    equal(editor.Text, "title", "Rebinding selects a new source field")
    editor.Text = "edited title"
    core.flushQueue()
    equal(model.Title, "edited title"); equal(model.Email, "still email")
    check(editor:attachPropertyProgram("TextRun.Text", "", "Expression", false), "A writable binding can be detached")
    editor.Text = "detached editor"
    core.flushQueue()
    equal(model.Title, "edited title", "Detaching removes writeback as well as forward evaluation")
    model.Title = "after detach"
    evaluate(root)
    equal(editor.Text, "detached editor", "Detaching also removes forward evaluation")
end)

scenario("long strings, null clearing and snapshots remain independent", function()
    local root, editor = ui.StackView {}, ui.Input {}
    local model = draft("seed")
    root.DataContext = model; root:addChild(editor)
    bind(editor, "TextRun.Text", "{DataContext/Email}")
    evaluate(root)
    local snapshots = {}
    for i = 1, 120 do
        local value = "entry" .. i .. ":" .. string.rep("密碼🙂", 600)
        editor.Text = value
        core.flushQueue()
        equal(model.Email, value, "Writeback owns all string bytes")
        snapshots[i] = editor.Text
        model.Email = "replacement " .. i
        evaluate(root)
        equal(editor.Text, model.Email, "Forward update releases the previous editor value safely")
        if i % 10 == 0 then collect() end
    end
    equal(snapshots[1], "entry1:" .. string.rep("密碼🙂", 600), "Old Lua snapshots outlive both native values")
    editor.Text = ""
    core.flushQueue()
    equal(model.Email, "", "Empty input is an explicit value")
    model.Email = nil
    evaluate(root)
    equal(editor.Text, "", "An unset model string displays an empty input")
    equal(model.Email, nil, "Forward empty-string display must not write back into an unset source")
    editor.Text = "after null"
    core.flushQueue()
    equal(model.Email, "after null", "The binding remains editable after a null model value")
end)

scenario("bound draft graph reclamation", function()
    local function graph()
        local root = ui.StackView {}
        local model, editor = draft("lifetime"), ui.Input {}
        root.DataContext = model
        root:addChild(editor)
        bind(editor, "TextRun.Text", "{DataContext/Email}")
        evaluate(root)
        editor.Text = "owned after model handle collection"
        core.flushQueue()
        model = nil
        collect()
        equal(editor.Text, "owned after model handle collection")
        root.DataContext = nil
        root = nil
        editor = nil
        collect()
    end
    graph()
    local baseline = core.objectCount()
    for _ = 1, 40 do graph() end
    collect()
    equal(core.objectCount(), baseline, "Bidirectional bindings must not retain discarded forms or models")
end)

print(("PASS: %d MVVM binding scenarios, %d assertions"):format(cases, checks))
