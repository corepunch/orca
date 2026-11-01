-- Visibility geometry and native-property work on long feeds. The node
-- doubles count bridge reads; timing assertions would be machine-dependent.
package.path = "share/?.lua;share/?/init.lua;" .. package.path
local Visibility = require "orca.data.visibility"
local horizontal = 0
local function node(values, children)
    values = values or {}
    values.Visible = values.Visible ~= false
    values.ActualX, values.ActualY = values.ActualX or 0, values.ActualY or 0
    values.ActualWidth, values.ActualHeight = values.ActualWidth or 390, values.ActualHeight or 100
    local result = setmetatable({}, { __newindex = function(_, key, value) values[key] = value end, __index = function(_, key)
        if key == "ActualX" or key == "ActualWidth" then horizontal = horizontal + 1 end
        return values[key]
    end })
    values.getParent = function() return values.parent end
    values.getClassName = function() return "Screen" end
    values.getFirstChild = function() return children and children[1] end
    values.getNext = function() return values.next end
    for index, child in ipairs(children or {}) do child.parent = result; child.next = children[index + 1] end
    return result
end
local records = {}
local scope = { store = { recordFor = function(_, context) return records[context] end } }
local function row(id, values)
    values = values or {}
    values.DataContext = id
    records[id] = { key = id }
    return node(values)
end
local children = { row("visible", { ActualY = 0 }),
    row("translated", { ActualY = 2000, RenderTransformTranslation = { X = 0, Y = -1900 } }),
    row("horizontal", { ActualX = 2000, ActualY = 0 }),
    row("hidden", { Visible = false }),
    row("empty", { ActualWidth = 0, ActualHeight = 0 }) }
for index = 1, 10000 do children[#children + 1] = row("offscreen" .. index, { ActualY = 2000 + index * 100 }) end
local screen = node({ ActualHeight = 844 }, children)
local visibility = Visibility.new { margin = 0 }
visibility:watch(screen, scope)
horizontal = 0
local found = visibility:records(visibility.pages[1])
assert(#found == 2 and found[1].key == "visible" and found[2].key == "translated",
    "Culling must respect visibility, zero size, horizontal bounds and translated rows")
assert(horizontal < 30, "Offscreen feed rows must not read horizontal native geometry")

-- Parent scrolling and clipping are applied before collecting descendants.
local scroll = node({ ActualHeight = 100, OverflowY = "Scroll", ContentOffset = { X = 0, Y = -2000 } },
    { row("above", { ActualY = 0 }), row("scrolled", { ActualY = 2000 }) })
screen = node({ ActualHeight = 844 }, { scroll })
visibility = Visibility.new { margin = 0 }
visibility:watch(screen, scope)
found = visibility:records(visibility.pages[1])
assert(#found == 1 and found[1].key == "scrolled", "The scroller offset and clip must reveal the correct record")
print("Data visibility: 10000 offscreen rows culled without horizontal bridge reads")
