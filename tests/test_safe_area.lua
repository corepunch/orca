local test = require "orca.test"
-- Headless tests for edge-to-edge layout: docked bars, safe-area insets of
-- scrolling content, Pivot headers that the body extends behind, and the CSS
-- backdrop-filter declaration that frosts the bars.

local core = require "orca.core"
local ui = require "orca.UIKit"
local Footer = require "orca.UIKit.Footer"

local W, H = 390, 844

local function layout(screen)
    core.advanceFrame()
    core.runAllPrograms(screen)
    screen:UpdateLayout(screen.Width, screen.Height)
end

local function edgeToEdge(props)
    props = props or {}
    props.Width, props.Height, props.ResizeMode = W, H, "NoResize"
    props.RespectSafeArea = false
    return ui.Screen(props)
end

-- A scrolling list of `count` rows, each `size` points high.
local function list(count, size, props)
    props = props or {}
    props.Overflow = props.Overflow or "hidden scroll"
    local stack = ui.StackView(props)
    for i = 1, count do stack:addChild(ui.Node2D { Name = "Row" .. i, Height = size }) end
    return stack
end

local function bottomOffset(view) return math.min(0, view.ActualHeight - view.ScrollHeight) end

local function test_docked_bars_inset_scrolling_content()
    local screen = edgeToEdge()
    local host = ui.Node2D {}
    local content = list(40, 50)
    local header = ui.Node2D { Name = "Header", Height = 44, SafeAreaEdge = "Top" }
    local footer = ui.Node2D { Name = "Footer", Height = 58, SafeAreaEdge = "Bottom" }
    host:addChild(content); host:addChild(header); host:addChild(footer)
    screen:addChild(host)
    layout(screen)

    test.expect_near(header.ActualY, 0, 0.01, "top bar docks to the top edge")
    test.expect_near(footer.ActualY, H - 58, 0.01, "bottom bar docks to the bottom edge")
    test.expect_near(content.ActualY, 0, 0.01, "scrolling content starts under the top bar")
    test.expect_near(content.ActualHeight, H, 0.01, "scrolling content runs under both bars")
    local first = content:findChild("Row1")
    test.expect_near(first.ActualY, 44, 0.01, "first row rests below the top bar")
    test.expect_near(content.ScrollHeight, 40 * 50 + 44 + 58, 0.01, "scroll range extends past the bottom bar")
    test.expect_near(content.AdjustedContentInsetTop, 44, 0.01, "the top inset is published")
    test.expect_near(content.AdjustedContentInsetBottom, 58, 0.01, "the bottom inset is published")
    local last = content:findChild("Row40")
    test.expect_near(last.ActualY + last.ActualHeight + bottomOffset(content), H - 58, 0.01,
        "last row scrolls clear of the bottom bar")
    screen:clear()
    print("PASS: test_docked_bars_inset_scrolling_content")
end

local function test_system_insets_reach_scrolling_content()
    local screen = edgeToEdge { SafeAreaTop = 59, SafeAreaBottom = 34 }
    local content = list(40, 50)
    local static = ui.Node2D { Name = "Static" }
    local still = ui.Node2D { Name = "Still", Height = 10 }
    static:addChild(still)
    local host = ui.Node2D {}
    host:addChild(content); host:addChild(static)
    screen:addChild(host)
    layout(screen)
    test.expect_near(content:findChild("Row1").ActualY, 59, 0.01, "status bar insets scrolling content")
    test.expect_near(content.ScrollHeight, 40 * 50 + 59 + 34, 0.01, "home indicator insets scrolling content")
    test.expect_near(still.ActualY, 0, 0.01, "non-scrolling content keeps its layout under the insets")

    screen.KeyboardInsetBottom = 300
    layout(screen)
    test.expect_near(content.ScrollHeight, 40 * 50 + 59 + 300, 0.01, "keyboard insets scrolling content")
    screen:clear()
    print("PASS: test_system_insets_reach_scrolling_content")
end

local function test_content_inset_adjustment_modes()
    local screen = edgeToEdge()
    local host = ui.Node2D {}
    local never = list(10, 50, { ContentInsetAdjustment = "Never" })
    local always = ui.Node2D { ContentInsetAdjustment = "Always" }
    local child = ui.Node2D { Name = "Child" }
    always:addChild(child)
    host:addChild(never); host:addChild(always)
    host:addChild(ui.Node2D { Height = 44, SafeAreaEdge = "Top" })
    host:addChild(ui.Node2D { Height = 58, SafeAreaEdge = "Bottom" })
    screen:addChild(host)
    layout(screen)
    test.expect_near(never:findChild("Row1").ActualY, 0, 0.01, "Never leaves the content under the bars")
    test.expect_near(never.ScrollHeight, 500, 0.01, "Never keeps the scroll range")
    test.expect_near(never.AdjustedContentInsetTop, 0, 0.01, "Never publishes no inset")
    test.expect_near(child.ActualY, 44, 0.01, "Always insets non-scrolling content from the top")
    test.expect_near(child.ActualHeight, H - 44 - 58, 0.01, "Always insets non-scrolling content from the bottom")
    screen:clear()
    print("PASS: test_content_inset_adjustment_modes")
end

local function test_bars_stack_below_ancestor_bars()
    local screen = edgeToEdge { SafeAreaTop = 59 }
    local outer = ui.Node2D { Name = "Outer" }
    local inner = ui.Node2D { Name = "Inner" }
    local content = list(40, 50)
    local commands = ui.Node2D { Name = "Commands", Height = 44, SafeAreaEdge = "Top" }
    inner:addChild(content); inner:addChild(commands)
    local title = ui.Node2D { Name = "Title", Height = 100, SafeAreaEdge = "Top" }
    local second = ui.Node2D { Name = "Second", Height = 20, SafeAreaEdge = "Top" }
    outer:addChild(inner); outer:addChild(title); outer:addChild(second)
    screen:addChild(outer)
    layout(screen)
    test.expect_near(title.ActualY, 0, 0.01, "an outermost bar extends under the status bar")
    test.expect_near(second.ActualY, 100, 0.01, "bars at one edge stack in child order")
    test.expect_near(commands.ActualY, 120, 0.01, "a nested bar docks below the bars of its ancestors")
    test.expect_near(content:findChild("Row1").ActualY, 164, 0.01, "content is inset by every bar above it")
    screen:clear()
    print("PASS: test_bars_stack_below_ancestor_bars")
end

local function test_adjusting_bars_extend_through_system_insets()
    local screen = edgeToEdge { SafeAreaTop = 59, SafeAreaBottom = 34 }
    local host = ui.Node2D {}
    local content = list(40, 50)
    -- Bars sized by their content, not Height, grow through the system insets.
    local header = ui.Node2D { SafeAreaEdge = "Top", ContentInsetAdjustment = "Always" }
    local title = ui.Node2D { Name = "Title", Height = 44 }
    header:addChild(title)
    local toolbar = ui.Grid { Rows = "58px", SafeAreaEdge = "Bottom", ContentInsetAdjustment = "Always" }
    local buttons = ui.Node2D { Name = "Buttons" }
    toolbar:addChild(buttons)
    host:addChild(content); host:addChild(header); host:addChild(toolbar)
    screen:addChild(host)
    layout(screen)
    test.expect_near(header.ActualY, 0, 0.01, "the header background starts under the status bar")
    test.expect_near(header.ActualHeight, 44 + 59, 0.01, "the header grows by the status bar")
    test.expect_near(title.ActualY, 59, 0.01, "the header content stays below the status bar")
    test.expect_near(title.ActualHeight, 44, 0.01, "the header content keeps its height")
    test.expect_near(toolbar.ActualY + toolbar.ActualHeight, H, 0.01, "the toolbar reaches the bottom")
    test.expect_near(toolbar.ActualHeight, 58 + 34, 0.01, "the toolbar grows by the home indicator")
    test.expect_near(buttons.ActualY, 0, 0.01, "the toolbar content stays at its top")
    test.expect_near(buttons.ActualHeight, 58, 0.01, "the toolbar content stays above the home indicator")
    test.expect_near(content:findChild("Row1").ActualY, 44 + 59, 0.01, "content rests below the header")
    test.expect_near(content.ScrollHeight, 40 * 50 + 44 + 59 + 58 + 34, 0.01, "content scrolls clear of the toolbar")
    screen:clear()
    print("PASS: test_adjusting_bars_extend_through_system_insets")
end

local function pivotPage(extend)
    local pivot = ui.Pivot { Title = "Messenger", IsLocked = true, ExtendBodyBehindHeader = extend }
    local item = ui.PivotItem { Header = "room" }
    local content = list(40, 50)
    item:addChild(content)
    pivot:addChild(item)
    return pivot, item, content
end

local function test_pivot_body_extends_behind_header()
    local screen = edgeToEdge { SafeAreaTop = 59, SafeAreaBottom = 34 }
    local pivot, item, content = pivotPage(true)
    local footer = Footer { Rows = "58px" }
    Footer.layout(screen, pivot, footer)
    layout(screen)

    local bar, child = nil, pivot:getFirstChild()
    while child do
        if child:getClassName() == "Node2D" then bar = child end
        child = child:getNext()
    end
    test.expect(bar ~= nil and bar.Visible, "an extended Pivot shows its bar")
    test.expect_near(bar.ActualY, 0, 0.01, "the Pivot bar extends under the status bar")
    test.expect_near(item.ActualY, 0, 0.01, "the body starts at the top of the Pivot")
    local first = content:findChild("Row1")
    test.expect_near(first.ActualY, bar.ActualHeight, 1, "content rests below the Pivot header")
    test.expect(bar.ActualHeight > 59 + 44, "the header keeps clear of the status bar")
    test.expect_near(footer.ActualY + footer.ActualHeight, H, 0.01, "the footer reaches the bottom")
    local last = content:findChild("Row40")
    test.expect_near(item.ActualY + content.ActualY + last.ActualY + last.ActualHeight + bottomOffset(content),
        footer.ActualY, 0.5, "the last row scrolls clear of the footer")
    screen:clear()
    print("PASS: test_pivot_body_extends_behind_header")
end

local function test_pivot_header_consumes_top_inset()
    local screen = edgeToEdge { SafeAreaTop = 59 }
    local pivot, item, content = pivotPage(false)
    screen:addChild(pivot)
    layout(screen)
    test.expect(item.ActualY > 59 + 44, "the body starts below the header and the status bar")
    test.expect_near(content:findChild("Row1").ActualY, 0, 0.01,
        "content below a header that does not extend gets no top inset")
    screen:clear()
    print("PASS: test_pivot_header_consumes_top_inset")
end

local function test_backdrop_filter_css()
    local screen = ui.Screen {
        Width = W, Height = H, ResizeMode = "NoResize",
        StyleSheet = ui.loadObjectFromCssString [[
            .frosted { backdrop-filter: blur(20px) saturate(180%); }
            .plain { backdrop-filter: none; }
            .webkit { -webkit-backdrop-filter: saturate(2) blur(8px); }
        ]],
    }
    local frosted, plain = ui.Node2D { class = "frosted" }, ui.Node2D { class = "plain" }
    local webkit, default = ui.Node2D { class = "webkit" }, ui.Node2D {}
    for _, node in ipairs { frosted, plain, webkit, default } do screen:addChild(node) end
    layout(screen)
    test.expect_near(frosted.BackdropFilterBlurRadius, 20, 0.001, "blur() sets the blur radius")
    test.expect_near(frosted.BackdropFilterSaturation, 1.8, 0.001, "saturate() accepts a percentage")
    test.expect_near(plain.BackdropFilterBlurRadius, 0, 0.001, "none clears the blur")
    test.expect_near(plain.BackdropFilterSaturation, 1, 0.001, "none keeps colors unchanged")
    test.expect_near(webkit.BackdropFilterBlurRadius, 8, 0.001, "the -webkit- alias applies")
    test.expect_near(webkit.BackdropFilterSaturation, 2, 0.001, "saturate() accepts a number")
    test.expect_near(default.BackdropFilterSaturation, 1, 0.001, "saturation defaults to 1")
    screen:clear()
    print("PASS: test_backdrop_filter_css")
end

test_docked_bars_inset_scrolling_content()
test_system_insets_reach_scrolling_content()
test_content_inset_adjustment_modes()
test_bars_stack_below_ancestor_bars()
test_adjusting_bars_extend_through_system_insets()
test_pivot_body_extends_behind_header()
test_pivot_header_consumes_top_inset()
test_backdrop_filter_css()
print("PASS: safe area and backdrop filter")
