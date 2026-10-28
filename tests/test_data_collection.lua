local orca = require "orca"
orca.init()
local core = require "orca.core"
local ui = require "orca.UIKit"
local fs = require "orca.filesystem"

local function evaluate(root)
    core.advanceFrame()
    core.runAllPrograms(root)
    core.flushQueue()
end

local function bind(view, expression)
    assert(view:attachPropertyProgram("ItemsControl.ItemsSource", expression, "OneWay", true))
end

local function collect()
    for _ = 1, 4 do core.flushQueue(); collectgarbage("collect") end
end

local function hierarchy()
    local source, other = core.DataObject {}, core.DataObject {}
    local a, b, c = core.DataObject { Name = "A" }, core.DataObject { Name = "B" }, core.DataObject { Name = "C" }
    source:addChild(a); source:addChild(b); source:addChild(c)
    a:moveBefore(c)
    assert(source:getFirstChild() == b and b:getNext() == a and a:getNext() == c)
    c:moveBefore(b)
    assert(source:getFirstChild() == c and c:getNext() == b)
    c:moveBefore(nil)
    assert(a:getNext() == c and c:getNext() == nil)
    for _ = 1, 100 do source:addChild(b) end
    assert(source:getFirstChild() == a and c:getNext() == b and b:getNext() == nil)
    other:addChild(a)
    assert(a:getParent() == other and source:getFirstChild() == c)
    b:moveBefore(a)
    assert(source:getFirstChild() == c and c:getNext() == b, "A foreign sibling cannot move a row")
    assert(a:addChild(other) == nil, "Hierarchy cycles must be rejected")
    assert(not pcall(function() b:moveBefore("invalid") end), "MoveBefore rejects non-object siblings")
    a:removeFromParent()
    assert(other:getFirstChild() == nil and a:getParent() == nil)
    source:clear()
    assert(source:getFirstChild() == nil and b:getParent() == nil and c:getParent() == nil)
end

collect()
local baseline = core.objectCount()
hierarchy()
collect()
assert(core.objectCount() == baseline, "Same-parent reorder must not leak hierarchy references")
print("PASS: DataObject collection hierarchy, ordering and reference ownership")

do
    local source = core.DataObject {}
    local list = ui.ListBox { ItemsSource = source }
    local a, b = core.DataObject { Name = "Batch A" }, core.DataObject { Name = "Batch B" }
    source:beginUpdate()
    source:addChild(a)
    source:beginUpdate()
    source:addChild(b)
    assert(list:getFirstChild() == nil, "An open collection batch defers observer notifications")
    source:endUpdate()
    assert(list:getFirstChild() == nil, "An inner EndUpdate must leave the outer batch pending")
    source:endUpdate()
    assert(list:getFirstChild().DataContext == a and list:getFirstChild():getNext().DataContext == b,
        "The outer EndUpdate publishes the complete collection")
    local first = list:getFirstChild()
    source:beginUpdate(); source:endUpdate(); source:endUpdate()
    assert(list:getFirstChild() == first, "Empty and unmatched batches preserve retained rows")
    source:beginUpdate()
    a:moveBefore(nil)
    b:removeFromParent()
    local c = source + core.DataObject { Name = "Batch C" }
    assert(list:getFirstChild() == first, "Retained views wait for the final batched hierarchy")
    source:endUpdate()
    assert(list:getFirstChild() == first and first:getNext().DataContext == c,
        "Batched remove, reorder and add reconcile while preserving surviving rows")
    list.ItemsSource = nil
    source:clear()
end
print("PASS: Nested DataObject collection update batches")

local function bindings()
    local root, inherited, direct, nested, explicit = ui.Node2D {}, ui.Node2D {}, ui.ItemsControl {}, ui.ItemsControl {}, ui.ItemsControl {}
    root:addChild(inherited); inherited:addChild(direct); inherited:addChild(nested); inherited:addChild(explicit)
    local model = core.DataObject {}
    local items = model + core.DataObject { Name = "Items" }
    local first = items + core.DataObject { Name = "First" }
    root.DataContext = model
    bind(direct, "{DataContext}")
    bind(nested, "{DataContext/Items}")
    bind(explicit, "{DataContext/Items/First}")
    evaluate(root)
    assert(direct.ItemsSource == model, "Root DataContext bindings must export objects")
    assert(nested.ItemsSource == items, "A final named collection child must bind as an object")
    assert(explicit.ItemsSource == first, "Nested named children must bind as objects")
    local replacement = core.DataObject {}
    local newItems = replacement + core.DataObject { Name = "Items" }
    local newFirst = newItems + core.DataObject { Name = "First" }
    root.DataContext = replacement
    evaluate(root)
    assert(direct.ItemsSource == replacement and nested.ItemsSource == newItems and explicit.ItemsSource == newFirst,
        "Inherited object bindings must follow replacement DataContext")
    local override = core.DataObject {}
    local overrideItems = override + core.DataObject { Name = "Items" }
    overrideItems:addChild(core.DataObject { Name = "First" })
    inherited.DataContext = override
    evaluate(root)
    assert(direct.ItemsSource == override and nested.ItemsSource == overrideItems,
        "Object collection bindings must use the nearest context")
    inherited.DataContext = nil
    evaluate(root)
    assert(nested.ItemsSource == newItems, "Clearing a local context restores inherited collection bindings")
    local secondRoot = ui.Node2D { DataContext = model }
    secondRoot:addChild(inherited)
    evaluate(secondRoot)
    assert(nested.ItemsSource == items, "Reparenting must resolve the current collection")
    secondRoot.DataContext = nil
    evaluate(secondRoot)
    assert(direct.ItemsSource == nil and nested.ItemsSource == nil and explicit.ItemsSource == nil,
        "Clearing inherited context must clear object-valued binding results")

    local bridge = ui.Node2D {}
    local text = bridge + ui.TextBlock {}
    local host = ui.Node2D { DataContext = model }
    host:addChild(bridge)
    assert(bridge:attachPropertyProgram("Node.DataContext", "{DataContext/Items}", "OneWay", true))
    assert(text:attachPropertyProgram("TextRun.Text", "{DataContext/First/Name}", "OneWay", true))
    evaluate(host)
    assert(bridge.DataContext == items and text.Text == "First", "Nested context bindings use the ancestor model")
    host.DataContext = replacement
    evaluate(host)
    assert(bridge.DataContext == newItems and text.Text == "First",
        "A previously bound DataContext must follow the replaced inherited model")
    host.DataContext = override
    evaluate(host)
    assert(bridge.DataContext == overrideItems, "Inherited DataContext rebinding must remain repeatable")
end
bindings()
print("PASS: Object-valued root, nested, inherited and replacement DataContext bindings")

local schema = [[<Schema>
    <Entity Name="CollectionBindingRecord"><Column Name="Title" Type="string"/></Entity>
</Schema>]]
local model = assert(fs.loadXmlDataStringWithSchema('<CollectionBindingRecord Title="initial"/>', schema))
local view = ui.TextBlock { DataContext = model }
assert(view:attachPropertyProgram("TextRun.Text", "{DataContext/Title}", "OneWay", true))
evaluate(view)
assert(view.Text == "initial")
model.Title = "changed"
evaluate(view)
assert(view.Text == "changed", "Schema record properties continue to propagate through native bindings")
print("PASS: Schema DataObject property changes")

do
    local items = model + core.DataObject { Name = "Items" }
    items:addChild(core.DataObject { Name = "Row" })
    local list = ui.ListBox { DataContext = model, ItemsSource = items }
    assert(model:attachPropertyProgram("CollectionBindingRecord.Title", '""', "OneWay", true))
    assert(list:attachPropertyProgram("ItemsControl.ItemsSource",
        "IF({DataContext/Title}, {DataContext/Items}, {DataContext/Missing})", "OneWay", true))
    evaluate(list)
    assert(list.ItemsSource == nil, "Nested source binding evaluation must restore the outer object target")
    assert(model:attachPropertyProgram("CollectionBindingRecord.Title", '"present"', "OneWay", true))
    evaluate(list)
    assert(list.ItemsSource == items, "Nested evaluation must preserve object-valued branches")
end
print("PASS: Reentrant binding evaluation restores the outer target context")
