-- Chart, LineSeries and ColumnSeries (plugins/UIKit/Chart.c): the value axis
-- fits the visible series with round intervals, columns include zero, fixed
-- ranges win, category labels thin out instead of overlapping, the legend
-- follows titled series and membership changes relayout the chart. The
-- generated labels are what a reader sees, so the checks read them.
local orca = require "orca"
orca.init()
local ui = require "orca.UIKit"
require("orca.renderer").init(390, 844, true)
local data = require "orca.data"
local checks = 0
local function check(value, message) assert(value, message); checks = checks + 1 end

local Point = data.Schema.define("TestChartPoint", { { "Label", "string" }, { "Value", "float" }, { "Other", "int" } })
local Points = data.Schema.define("TestChartPoints", { { "Title", "string" } })

local function collection(values, others)
    local source = Points:instantiate()
    for index, value in ipairs(values) do
        local point = Point:instantiate()
        point.Label = ("%02d"):format(index - 1)
        point.Value = value
        point.Other = others and others[index] or 0
        source:addChild(point)
    end
    return source
end

local screen = ui.Screen { Width = 390, Height = 844 }
local chart = ui.Chart { Name = "Chart", Width = 300, Height = 200, CategoryPath = "Label" }
local line = ui.LineSeries { Name = "Line", ValuePath = "Value", Title = "value" }
local columns = ui.ColumnSeries { Name = "Columns", ValuePath = "Other", Title = "other", Visible = false }
chart:addChild(line)
chart:addChild(columns)
screen:addChild(chart)

local function layout() screen:UpdateLayout(390, 844); screen:UpdateLayout(390, 844) end

-- The visible generated labels, as { text, x, y }, sorted top-left first.
local function labels()
    local found = {}
    local child = chart:getFirstChild()
    while child do
        if child.className == "TextBlock" and child.Visible then
            found[#found + 1] = { text = child.Text, x = child.ActualX or 0, y = child.ActualY or 0, width = child.ActualWidth or 0 }
        end
        child = child:getNext()
    end
    return found
end
local function texts()
    local all = {}
    for _, label in ipairs(labels()) do all[label.text] = (all[label.text] or 0) + 1 end
    return all
end

-- A line from 1 to 9: gridlines every 2, from 0 to 10.
chart.ItemsSource = collection { 1, 3, 5, 9, 7 }
layout()
local shown = texts()
for _, value in ipairs { "0", "2", "4", "6", "8", "10" } do check(shown[value], "Value label " .. value) end
check(not shown["12"] and not shown["-2"], "The axis stops at round values around the data")
check(shown["value"] and not shown["other"], "The legend names the visible titled series only")
check(shown["00"] and shown["04"], "Categories are labelled from CategoryPath")

-- A hidden column series does not stretch the axis; shown, it adds zero and its range.
chart.ItemsSource = collection({ 4, 5, 6 }, { -3, 1, 2 })
layout()
shown = texts()
check(shown["4"] and shown["6"] and not shown["0"], "Hidden columns leave the axis to the line: 4 to 6")
columns.Visible = true
layout()
shown = texts()
-- -3 to 6 in steps of 5 (1, 2 or 5 times a power of ten): -5 to 10.
check(shown["-5"] and shown["0"] and shown["10"] and shown["other"], "Columns include zero and their negative values")

-- A fixed range and interval win over the data.
chart.Minimum, chart.Maximum, chart.Interval = 0, 100, 25
layout()
shown = texts()
for _, value in ipairs { "0", "25", "50", "75", "100" } do check(shown[value], "Fixed label " .. value) end
check(not shown["6"], "The data no longer sets the axis")
chart.Minimum, chart.Maximum, chart.Interval = 0 / 0, 0 / 0, 0 / 0

-- 24 categories on a narrow chart thin out so labels never overlap.
local values = {}
for i = 1, 24 do values[i] = i end
chart.ItemsSource = collection(values)
chart.Width = 200
layout()
local bottom = {}
for _, label in ipairs(labels()) do
    if label.text:match("^%d%d$") and tonumber(label.text) and label.y > 175 then bottom[#bottom + 1] = label end
end
table.sort(bottom, function(a, b) return a.x < b.x end)
check(#bottom > 1 and #bottom < 24, "Only some of 24 categories are labelled: " .. #bottom)
for i = 2, #bottom do
    check(bottom[i].x >= bottom[i - 1].x + bottom[i - 1].width, "Category labels do not overlap")
end
chart.LabelInterval = 6
layout()
local every = 0
for _, label in ipairs(labels()) do if label.text:match("^%d%d$") and label.y > 175 then every = every + 1 end end
check(every == 4, "LabelInterval 6 labels 4 of 24 categories: " .. every)

-- Adding a record relayouts the chart.
chart.LabelInterval = 1
chart.Width = 300
local source = collection { 1, 2 }
chart.ItemsSource = source
layout()
check(not texts()["02"], "Two categories")
local point = Point:instantiate()
point.Label, point.Value = "02", 20
source:addChild(point)
require("orca.core").flushQueue()
layout()
shown = texts()
check(shown["02"] and shown["20"], "A new record adds its category and stretches the axis")

print(("PASS: chart (%d checks)"):format(checks))
