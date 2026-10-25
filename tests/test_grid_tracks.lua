local test = require "orca.test"
-- Grid track sizing, after WPF: fixed tracks keep their size, Auto tracks fit
-- their content, and star (fr) tracks share what remains. Children use
-- explicit sizes so the checks do not depend on font metrics.

local orca = require "orca"
local ui = require "orca.UIKit"

local function screen(width, height)
	return ui.Screen { Width = width or 400, Height = height or 600, ResizeMode = "NoResize" }
end

local function layout(s)
	s:UpdateLayout(s.Width, s.Height)
	s:UpdateLayout(s.Width, s.Height)
end

-- Builds a grid with one Node2D per entry of `children` and lays it out.
local function grid(s, props, children)
	local g = s + ui.Grid(props)
	local nodes = {}
	for i, child in ipairs(children) do nodes[i] = g + ui.Node2D(child) end
	layout(s)
	return g, nodes
end

local function rows(nodes)
	local out = {}
	for i, n in ipairs(nodes) do out[i] = string.format("%g+%g", n.ActualY, n.ActualHeight) end
	return table.concat(out, " ")
end

local function columns(nodes)
	local out = {}
	for i, n in ipairs(nodes) do out[i] = string.format("%g+%g", n.ActualX, n.ActualWidth) end
	return table.concat(out, " ")
end

local function test_fixed_and_star_rows()
	local _, n = grid(screen(), { Rows = "60px 1fr 80px" }, { {}, {}, {} })
	test.expect_eq(rows(n), "0+60 60+460 520+80", "Fixed rows keep their size and a star row takes the rest")
	_, n = grid(screen(), { Rows = "60 1fr 80" }, { {}, {}, {} })
	test.expect_eq(rows(n), "0+60 60+460 520+80", "Unitless track sizes are pixels, as in WPF")
	print("PASS: test_fixed_and_star_rows")
end

local function test_weighted_stars()
	local _, n = grid(screen(), { Rows = "1fr 3fr" }, { {}, {} })
	test.expect_eq(rows(n), "0+150 150+450", "fr weights divide the remaining space")
	_, n = grid(screen(), { Rows = "* 2*" }, { {}, {} })
	test.expect_eq(rows(n), "0+200 200+400", "WPF star syntax matches fr weights")
	_, n = grid(screen(), { Rows = "1* *" }, { {}, {} })
	test.expect_eq(rows(n), "0+300 300+300", "A bare star has weight one")
	print("PASS: test_weighted_stars")
end

local function test_auto_rows_fit_content()
	local _, n = grid(screen(), { Rows = "auto 1fr auto" }, { { Height = 50 }, {}, { Height = 70 } })
	test.expect_eq(rows(n), "0+50 50+480 530+70", "Auto rows fit their content; the star row takes the rest")
	_, n = grid(screen(), { Rows = "Auto *" }, { { Height = 50 }, {} })
	test.expect_eq(rows(n), "0+50 50+550", "WPF spellings Auto and * are accepted")
	_, n = grid(screen(), { Rows = "auto 1fr" }, { { Height = 500 }, {} })
	test.expect_eq(rows(n), "0+500 500+100", "An Auto row grows with its content instead of overlapping the next row")
	_, n = grid(screen(), { Rows = "auto 1fr" }, { { Height = 40, MarginTop = 6, MarginBottom = 4 }, {} })
	test.expect_eq(rows(n), "6+40 50+550", "An Auto row includes its child's margins")
	print("PASS: test_auto_rows_fit_content")
end

local function test_auto_columns_fit_content()
	local _, n = grid(screen(), { Columns = "auto 1fr auto" }, { { Width = 40 }, {}, { Width = 90 } })
	test.expect_eq(columns(n), "0+40 40+270 310+90", "Auto columns fit their content; the star column takes the rest")
	_, n = grid(screen(), { Columns = "auto auto" }, { { Width = 50 }, { Width = 70 } })
	test.expect_eq(columns(n), "0+50 50+70", "Auto columns do not share the free space")
	_, n = grid(screen(), { Columns = "auto 1fr" }, { { Width = 30 }, {}, { Width = 80 }, {} })
	test.expect_eq(columns(n), "0+30 80+320 0+80 80+320", "An Auto column is as wide as its widest child")
	print("PASS: test_auto_columns_fit_content")
end

local function test_spacing()
	local _, n = grid(screen(), { Columns = "100px 1fr 1fr", Spacing = 10 }, { {}, {}, {} })
	test.expect_eq(columns(n), "0+100 110+140 260+140", "Spacing is removed before stars share the rest")
	_, n = grid(screen(), { Rows = "auto auto 1fr", Spacing = 8 }, { { Height = 20 }, { Height = 30 }, {} })
	test.expect_eq(rows(n), "0+20 28+30 66+534", "Auto rows and spacing stack without gaps or overlap")
	print("PASS: test_spacing")
end

local function test_star_clamps_to_zero()
	local _, n = grid(screen(400, 300), { Rows = "400px 1fr" }, { {}, {} })
	test.expect_eq(n[2].ActualHeight, 0, "A star track never becomes negative")
	test.expect_eq(n[2].ActualY, 400, "Tracks after an overflowing fixed track keep their order")
	print("PASS: test_star_clamps_to_zero")
end

local function test_unbounded_grid_in_stack()
	local s = screen()
	local stack = s + ui.StackView { Direction = "Vertical" }
	local g = stack + ui.Grid { Rows = "auto auto" }
	local a = g + ui.Node2D { Height = 40 }
	local b = g + ui.Node2D { Height = 60 }
	local after = stack + ui.Node2D { Height = 10 }
	layout(s)
	test.expect_eq(rows { a, b }, "0+40 40+60", "Auto rows stack inside an unbounded grid")
	test.expect_eq(g.ActualHeight, 100, "An unbounded grid is as tall as its Auto rows")
	test.expect_eq(after.ActualY, 100, "The next sibling starts after the grid")

	local star = stack + ui.Grid { Rows = "1fr 2fr" }
	local c = star + ui.Node2D { Height = 30 }
	local d = star + ui.Node2D { Height = 50 }
	layout(s)
	-- WPF measures unbounded star rows by content (30 + 50), then arranges the
	-- grid at that height and shares it by weight.
	test.expect_eq(star.ActualHeight, 80, "With unbounded height, star rows request their content")
	test.expect(math.abs(d.ActualY - 80 / 3) < 0.01, "Star rows then share the arranged height by weight")
	print("PASS: test_unbounded_grid_in_stack")
end

local function test_implicit_tracks()
	local _, n = grid(screen(), { Columns = "1fr 1fr" }, { { Height = 50 }, { Height = 70 }, { Height = 40 }, { Height = 20 } })
	test.expect_eq(rows(n), "0+50 0+70 70+40 70+20", "Rows generated for overflowing children fit their content")
	_, n = grid(screen(), { Columns = "1fr 1fr" }, { {}, {} })
	test.expect_eq(rows(n), "0+600 0+600", "A single generated row fills the grid, like WPF's default star row")
	print("PASS: test_implicit_tracks")
end

local function test_nested_auto_footer()
	-- The outer auto row measures its child with both axes unbounded. Packing
	-- that infinite width must not corrupt the nested grid's finite height.
	for _, childType in ipairs { ui.Node2D, ui.ApplicationBar } do
		local s = screen(390, 844)
		local outer = s + ui.Grid { Rows = "1fr auto", PaddingTop = 59 }
		local body = outer + ui.Node2D {}
		local footer = outer + ui.Grid { Rows = "58px", PaddingBottom = 34 }
		local bar = footer + childType {}
		layout(s)
		test.expect_eq(outer.ActualHeight, 844, "An unbounded measurement must not enlarge the viewport")
		test.expect_eq(footer.ActualHeight, 92, "The auto footer fits its fixed row and safe-area padding")
		test.expect_eq(footer.ActualY, 752, "The auto footer remains at the viewport bottom")
		test.expect_eq(bar.ActualHeight, 58, "The bar keeps its fixed row height")
		test.expect_eq(footer.ActualY + bar.ActualY + bar.ActualHeight, 810,
			"The controls end above the bottom safe area")
	end
	local s = screen(390, 844)
	local outer = s + ui.Grid { Rows = "1fr auto" }
	local body = outer + ui.Node2D {}
	local bar = outer + ui.ApplicationBar {}
	layout(s)
	test.expect_eq(bar.ActualY, 786, "An unbounded application bar measurement keeps its finite height")
	test.expect_eq(bar.ActualHeight, 58, "An auto row fits the application bar")
	print("PASS: test_nested_auto_footer")
end

local function test_xml_tracks()
	local filesystem = require "orca.filesystem"
	-- A throwaway project, so the XML parser reads Rows exactly as authored.
	local root = os.tmpname()
	os.remove(root)
	os.execute("mkdir -p '" .. root .. "/Screens'")
	local function write(path, text)
		local f = assert(io.open(root .. "/" .. path, "w"))
		f:write(text)
		f:close()
	end
	write("package.lua", 'Name = "Tracks"\nScreenLibrary = { IsExternal = true }\n')
	write("Screens/Tracks.xml", [[
<?xml version="1.0" encoding="UTF-8"?>
<Screen Name="Tracks" Width="400" Height="600" ResizeMode="NoResize">
	<Grid Rows="auto * 48">
		<Node2D Name="Header" Height="50"/>
		<Node2D Name="Body"/>
		<Node2D Name="Footer"/>
	</Grid>
</Screen>]])
	filesystem.init(root)
	local s = filesystem.loadObject("Tracks/Screens/Tracks")
	layout(s)
	test.expect_eq(rows { s:findChild("Header", true), s:findChild("Body", true), s:findChild("Footer", true) },
		"0+50 50+502 552+48", "XML Rows use the same track grammar")
	os.execute("rm -rf '" .. root .. "'")
	print("PASS: test_xml_tracks")
end


test_fixed_and_star_rows()
test_weighted_stars()
test_auto_rows_fit_content()
test_auto_columns_fit_content()
test_spacing()
test_star_clamps_to_zero()
test_unbounded_grid_in_stack()
test_implicit_tracks()
test_nested_auto_footer()
test_xml_tracks()
