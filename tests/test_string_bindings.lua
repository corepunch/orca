-- String ownership through the public Lua, binding VM, XML and DataContext APIs.
-- Native allocation and sanitizer assertions live in test_string_properties.c.
local orca = require "orca"
orca.init()
local core = require "orca.core"
local ui = require "orca.UIKit"
local fs = require "orca.filesystem"
local checks, cases = 0, 0
local function equal(actual, expected, context)
    checks = checks + 1
    assert(actual == expected, (context or "string value") .. ": expected " .. tostring(expected) .. ", got " .. tostring(actual))
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
local function bind(node, expression)
    check(node:attachPropertyProgram("TextRun.Text", expression, "Expression", true), "Binding must compile: " .. expression)
end
local function scenario(name, fn)
    fn()
    collect()
    cases = cases + 1
    print("PASS: string bindings " .. name)
end

scenario("Lua string copies, replacement, clearing and retained snapshots", function()
    local text = ui.TextBlock {}
    local snapshots = {}
    for i = 1, 400 do
        local value = ("row-%d·漢字🙂:"):format(i) .. string.rep(string.char(65 + i % 26), i % 4096)
        text.Text = value
        equal(text.Text, value, "Assigning a Lua string must copy the full C string")
        snapshots[i] = text.Text
        value = nil
        if i % 20 == 0 then collectgarbage("collect") end
    end
    text.Text = "replacement"
    for i, value in ipairs(snapshots) do
        equal(value, ("row-%d·漢字🙂:"):format(i) .. string.rep(string.char(65 + i % 26), i % 4096), "Lua snapshots must survive native replacement")
    end
    text.Text = nil
    equal(text.Text, nil, "Clearing a pointer string must release its value")
    text.Text = ""
    equal(text.Text, "", "Empty and null strings have distinct property state")
    text.Text = string.rep("long", 2048)
    equal(#text.Text, 8192, "Pointer strings must not inherit fixed-array truncation")
end)

scenario("chained sibling paths, nested concatenation, conditionals and conversion", function()
    local root = ui.StackView {}
    local source = ui.TextBlock { Name = "Source", Text = "seed", Width = 1 }
    local alternate = ui.TextBlock { Name = "Alternate", Text = "fallback" }
    local first = ui.TextBlock { Name = "First" }
    local second = ui.TextBlock { Name = "Second" }
    local third = ui.TextBlock { Name = "Third" }
    for _, node in ipairs { source, alternate, first, second, third } do root:addChild(node) end
    bind(first, '"[" + {../Source/Text} + "]"')
    bind(second, 'IF({../Source/Width}, {../First/Text} + ":" + STRING({../Source/Width}), {../Alternate/Text})')
    bind(third, '"result=" + ({../Second/Text} + ("/" + {../Source/Text}))')
    local snapshots = {}
    for i = 1, 700 do
        local value = ("entry%d:"):format(i) .. string.rep("ü", i % 38)
        source.Text, source.Width = value, i % 2
        alternate.Text = "alternate" .. i
        evaluate(root)
        local expected = i % 2 == 1 and "[" .. value .. "]:1" or "alternate" .. i
        equal(first.Text, "[" .. value .. "]", "First link owns its output")
        equal(second.Text, expected, "Conditional preserves the selected string register")
        equal(third.Text, "result=" .. expected .. "/" .. value, "Nested binding owns all temporary results")
        snapshots[#snapshots + 1] = third.Text
        if i % 25 == 0 then collectgarbage("collect") end
    end
    equal(snapshots[1], "result=[entry1:ü]:1/entry1:ü", "Early output survives VM pool rollover and later updates")
    equal(snapshots[2], "result=alternate2/entry2:üü", "False branch snapshot remains independent")
    source.Text, alternate.Text = nil, nil
    evaluate(root)
    equal(first.Text, "[]", "Null source exports an owned empty string")
    equal(second.Text, "", "Null conditional source exports empty string")
    equal(third.Text, "result=/", "Nested concatenation handles null dependencies")
end)

scenario("large string fan-out across repeated VM temporary-pool rollover", function()
    local root = ui.StackView {}
    local source = ui.TextBlock { Name = "Source" }
    local alternate = ui.TextBlock { Name = "Alternate", Text = "fallback" }
    local gate = ui.Node2D { Name = "Gate", Width = 1 }
    root:addChild(source); root:addChild(alternate); root:addChild(gate)
    local sinks, snapshots = {}, {}
    for i = 1, 48 do
        local sink = ui.TextBlock {}
        root:addChild(sink)
        bind(sink, 'IF({../Gate/Width}, STRING({../Source/Text}), {../Alternate/Text})')
        sinks[i] = sink
    end
    for iteration = 1, 24 do
        local value = ("large%d:"):format(iteration) .. string.rep("漢🙂", 1100)
        source.Text, gate.Width = value, iteration % 3
        alternate.Text = "fallback" .. iteration
        evaluate(root)
        local expected = iteration % 3 ~= 0 and value or "fallback" .. iteration
        for i, sink in ipairs(sinks) do
            equal(sink.Text, expected, "Every fan-out target must own its result after all siblings run")
            if iteration == 1 then snapshots[i] = sink.Text end
        end
        if iteration % 4 == 0 then collectgarbage("collect") end
    end
    source.Text, alternate.Text = "replaced source", nil
    source = nil
    collectgarbage("collect")
    for _, value in ipairs(snapshots) do
        equal(value, "large1:" .. string.rep("漢🙂", 1100), "Large snapshots survive replacement, GC and many VM pool wraps")
    end
    evaluate(root)
    for _, sink in ipairs(sinks) do equal(sink.Text, "", "Null fallback owns an empty result in every target") end
end)

scenario("binding replacement and expression string ownership", function()
    local root = ui.StackView {}
    local a, b, sink = ui.TextBlock { Name = "A", Text = "alpha" }, ui.TextBlock { Name = "B", Text = "beta" }, ui.TextBlock {}
    root:addChild(a); root:addChild(b); root:addChild(sink)
    for i = 1, 300 do
        local name = i % 2 == 0 and "A" or "B"
        local prefix = "binding" .. i .. ":"
        local expression = ('"%s" + {../%s/Text}'):format(prefix, name)
        bind(sink, expression)
        expression = nil
        collectgarbage("collect")
        evaluate(root)
        equal(sink.Text, prefix .. (name == "A" and "alpha" or "beta"), "Replaced expression must own its source text and token tree")
    end
    sink.Text = nil
    a.Text, b.Text = "changed-a", "changed-b"
    evaluate(root)
    equal(sink.Text, "binding300:changed-a", "Clearing the resolved value preserves and safely reevaluates the binding")
    check(sink:attachPropertyProgram("TextRun.Text", "", "Expression", false), "Binding can be explicitly detached")
    sink.Text = "manual"
    evaluate(root)
    equal(sink.Text, "manual", "An explicitly detached binding must not resume writing")
end)

scenario("inherited DataContext replacement and source collection", function()
    local schema = [[<Schema><Entity Name="StringBindingRecord">
        <Column Name="Title" Type="string"/><Column Name="Detail" Type="string"/>
        <Column Name="Choice" Type="string"/></Entity></Schema>]]
    local seed = assert(fs.loadXmlDataStringWithSchema(
        '<StringBindingRecord Title="first" Detail="fallback" Choice="yes"/>', schema))
    local function record(title, detail, choice)
        local value = ui.Node2D {}
        value:addComponentByName("StringBindingRecord")
        value.Title, value.Detail, value.Choice = title, detail, choice
        return value
    end
    local root, group, sink = ui.StackView {}, ui.StackView {}, ui.TextBlock {}
    root:addChild(group); group:addChild(sink)
    root.DataContext = seed
    bind(sink, '"record:" + IF({DataContext/Choice}, {DataContext/Title}, {DataContext/Detail}) + "|" + {DataContext/Title}')
    seed = nil
    collect()
    evaluate(root)
    equal(sink.Text, "record:first|first", "DataContext must keep a source alive after its Lua handle is collected")
    local snapshots = {}
    for i = 1, 200 do
        local title, detail = "title" .. i, "detail" .. i
        root.DataContext = record(title, detail, i % 2 == 0 and "" or "selected")
        collectgarbage("collect")
        evaluate(root)
        local expected = "record:" .. (i % 2 == 0 and detail or title) .. "|" .. title
        equal(sink.Text, expected, "Replacing DataContext must release old source and read the new source")
        snapshots[i] = sink.Text
    end
    group.DataContext = record("override", "unused", "selected")
    collect()
    evaluate(root)
    equal(sink.Text, "record:override|override", "Nearest inherited DataContext owns its strings")
    group.DataContext = nil
    evaluate(root)
    equal(sink.Text, "record:detail200|title200", "Clearing local DataContext restores the ancestor source")
    equal(snapshots[1], "record:title1|title1", "Old DataContext snapshots survive collection and replacement")
end)

scenario("reparented bindings and complete graph reclamation", function()
    local function graph()
        local first = ui.TextBlock { Text = "first parent" }
        local second = ui.TextBlock { Text = "second parent" }
        local child = ui.TextBlock {}
        first:addChild(child)
        bind(child, '"parent=" + {../Text}')
        evaluate(first)
        equal(child.Text, "parent=first parent", "Relative path reads the first parent")
        second:addChild(child)
        first.Text = "discarded first parent"
        evaluate(second)
        equal(child.Text, "parent=second parent", "Reparented binding must not retain a cached old source")
        first = nil
        collect()
        second.Text = "updated second parent"
        evaluate(second)
        equal(child.Text, "parent=updated second parent", "Binding remains valid after old parent collection")
        local snapshot = child.Text
        child:removeFromParent()
        child = nil
        second = nil
        collect()
        equal(snapshot, "parent=updated second parent", "Lua result outlives destruction of its entire bound graph")
    end
    graph()
    collect()
    local baseline = core.objectCount()
    for _ = 1, 40 do graph() end
    collect()
    equal(core.objectCount(), baseline, "Repeated bound graph creation/destruction must not leak native objects")
end)

scenario("resolved parent paths invalidate before source destruction", function()
    local root = ui.StackView {}
    local source = ui.TextBlock { Text = "first" }
    local target = ui.TextBlock {}
    root:addChild(source); source:addChild(target)
    bind(target, "{../Text}")
    evaluate(root)
    evaluate(root) -- populate and reuse the resolved-path cache
    equal(target.Text, "first", "Cached parent source")
    target:removeFromParent()
    source:removeFromParent()
    source = nil
    collect()
    local replacement = ui.TextBlock { Text = "second" }
    root:addChild(replacement); replacement:addChild(target)
    evaluate(root)
    equal(target.Text, "second", "Destroyed source must not survive in a path cache")
    replacement.Text = "updated"
    evaluate(root)
    equal(target.Text, "updated", "Cached property still exports the current scalar value")
end)

print(("PASS: %d string binding scenarios, %d assertions"):format(cases, checks))
