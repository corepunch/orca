-- String ownership of binding expressions authored in Lisp; tests/test_string_bindings.lua
-- covers the format-independent string binding cases.
local orca = require "orca"
orca.init()
local core = require "orca.core"
require "orca.UIKit"
local fs = require "orca.filesystem"
local checks, cases = 0, 0
local function equal(actual, expected, context)
    checks = checks + 1
    assert(actual == expected, (context or "string value") .. ": expected " .. tostring(expected) .. ", got " .. tostring(actual))
end
local function collect()
    for _ = 1, 4 do core.flushQueue(); collectgarbage("collect") end
end
local function evaluate(root)
    core.advanceFrame()
    core.runAllPrograms(root)
    core.flushQueue()
end
local function scenario(name, fn)
    fn()
    collect()
    cases = cases + 1
    print("PASS: string bindings " .. name)
end

scenario("Lisp-authored expression lifetime", function()
    local lisp = [[(StackView (TextBlock Name="Source" Text="escaped & quoted \"text\"")
        (TextBlock Name="Sink" (Binding Target="TextRun.Text" "\"<\" + {../Source/Text} + \">\"")))]]
    local root = assert(fs.loadObjectFromLispString(lisp))
    lisp = nil
    collectgarbage("collect")
    local source, sink = root:findChild("Source"), root:findChild("Sink")
    evaluate(root)
    equal(sink.Text, '<escaped & quoted "text">', "Scene source storage must outlive binding compilation")
    for i = 1, 150 do
        source.Text = "lisp-update-" .. i
        evaluate(root)
        equal(sink.Text, "<lisp-update-" .. i .. ">", "Lisp expression survives later source changes")
    end
end)

print(("PASS: %d string binding scenarios, %d assertions"):format(cases, checks))
