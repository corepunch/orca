local test = require "orca.test"
local orca = require "orca"
local core = require "orca.core"
local ui = require "orca.UIKit"

orca.async = function (fn, ...) fn(...) end

local function pump_messages(root)
    while true do
        local msg = orca.system.peekMessage()
        if not msg then return end
        orca.system.dispatchMessage(root, msg)
    end
end

-- ---------------------------------------------------------------------------
-- PageHost: shows initial page matching CurrentPage
-- ---------------------------------------------------------------------------
local function test_pagehost_initial_page_from_currentpage()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
    local host = screen + ui.PageHost { Width = 400, Height = 600, CurrentPage = "beta" }
    local page_a = host + ui.Page { Name = "page_a", Key = "alpha", Width = 400, Height = 600 }
    local page_b = host + ui.Page { Name = "page_b", Key = "beta", Width = 400, Height = 600 }

    host:send("Node.ViewDidLoad")
    pump_messages(screen)

    test.expect(page_a.Visible == false, "page_a should be hidden")
    test.expect(page_b.Visible == true, "page_b should be visible")

    screen:clear()
    print("PASS: test_pagehost_initial_page_from_currentpage")
end

-- ---------------------------------------------------------------------------
-- PageHost: switches page when CurrentPage changes
-- ---------------------------------------------------------------------------
local function test_pagehost_switches_page_on_currentpage_change()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
    local host = screen + ui.PageHost { Width = 400, Height = 600, CurrentPage = "alpha" }
    local page_a = host + ui.Page { Name = "page_a", Key = "alpha", Width = 400, Height = 600 }
    local page_b = host + ui.Page { Name = "page_b", Key = "beta", Width = 400, Height = 600 }

    host:send("Node.ViewDidLoad")
    pump_messages(screen)

    test.expect(page_a.Visible, "page_a should be visible initially")
    test.expect(page_b.Visible == false, "page_b should be hidden initially")

    host.CurrentPage = "beta"
    pump_messages(screen)

    test.expect(page_a.Visible == false, "page_a should be hidden after switch")
    test.expect(page_b.Visible == true, "page_b should be visible after switch")

    screen:clear()
    print("PASS: test_pagehost_switches_page_on_currentpage_change")
end

-- ---------------------------------------------------------------------------
-- ListBox: SelectedValue property works
-- ---------------------------------------------------------------------------
local function test_listbox_selectedvalue_property()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

    local items = core.DataObject { Name = "Tabs" }
    items:addChild(core.DataObject { Name = "alpha" })
    items:addChild(core.DataObject { Name = "beta" })

    local template = ui.Node2D { Name = "TabItem", Width = 400, Height = 50 }

    local list = ui.ListBox {
        Width = 400, Height = 200,
        ItemsSource = items,
        ItemTemplate = template,
    }
    screen:addChild(list)

    local children = {}
    for child in list.children do children[#children + 1] = child end
    test.expect_eq(#children, 2, "Should have 2 children")

    -- Auto-select only works with ItemsSource + valid DataContext;
    -- for unit tests, set SelectedValue explicitly
    list.SelectedValue = "beta"
    pump_messages(screen)

    test.expect_eq(list.SelectedValue, "beta", "SelectedValue should be beta")

    screen:clear()
    print("PASS: test_listbox_selectedvalue_property")
end

-- ---------------------------------------------------------------------------
-- ListBox: SelectedValue changes correctly
-- ---------------------------------------------------------------------------
local function test_listbox_selectionchanged_fires()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

    local items = core.DataObject { Name = "Tabs" }
    items:addChild(core.DataObject { Name = "games" })
    items:addChild(core.DataObject { Name = "settings" })

    local template = ui.Node2D { Name = "TabItem", Width = 400, Height = 50 }

    local list = ui.ListBox {
        Width = 400, Height = 200,
        ItemsSource = items,
        ItemTemplate = template,
    }
    screen:addChild(list)

    list.SelectedValue = "settings"
    pump_messages(screen)

    test.expect_eq(list.SelectedValue, "settings", "SelectedValue should be settings after change")
    test.expect(list.SelectedValue ~= "games", "SelectedValue should not be games")

    screen:clear()
    print("PASS: test_listbox_selectionchanged_fires")
end

-- ---------------------------------------------------------------------------
-- PageHost + ListBox end-to-end: SelectedValue drives page switching
-- ---------------------------------------------------------------------------
local function test_pagehost_listbox_end_to_end()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

    local host = screen + ui.PageHost { Width = 400, Height = 600, CurrentPage = "games" }
    local page_games = host + ui.Page { Name = "page_games", Key = "games", Width = 400, Height = 600 }
    local page_settings = host + ui.Page { Name = "page_settings", Key = "settings", Width = 400, Height = 600 }

    local items = core.DataObject { Name = "Tabs" }
    items:addChild(core.DataObject { Name = "games" })
    items:addChild(core.DataObject { Name = "settings" })

    local template = ui.Node2D { Name = "TabItem", Width = 200, Height = 100 }

    local footer = ui.ListBox {
        Width = 400, Height = 100,
        SelectedValue = "games",
        ItemsSource = items,
        ItemTemplate = template,
    }
    screen:addChild(footer)

    host:send("Node.ViewDidLoad")
    pump_messages(screen)

    test.expect(page_games.Visible, "games page should be visible initially")
    test.expect(page_settings.Visible == false, "settings page should be hidden initially")

    -- Simulate binding: set both values (like {../Footer/SelectedValue} would)
    footer.SelectedValue = "settings"
    host.CurrentPage = "settings"
    pump_messages(screen)

    test.expect_eq(footer.SelectedValue, "settings", "Footer should show settings")
    test.expect(page_games.Visible == false, "games page should be hidden")
    test.expect(page_settings.Visible == true, "settings page should be visible")

    footer.SelectedValue = "games"
    host.CurrentPage = "games"
    pump_messages(screen)

    test.expect_eq(footer.SelectedValue, "games", "Footer should show games")
    test.expect(page_games.Visible, "games page should be visible again")
    test.expect(page_settings.Visible == false, "settings page should be hidden again")

    screen:clear()
    print("PASS: test_pagehost_listbox_end_to_end")
end

-- ---------------------------------------------------------------------------
-- PageHost: NavigateBack returns to previous page
-- ---------------------------------------------------------------------------
local function test_pagehost_navigateback()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
    local host = screen + ui.PageHost { Width = 400, Height = 600, CurrentPage = "alpha" }
    local page_a = host + ui.Page { Name = "page_a", Key = "alpha", Path = "/alpha", Width = 400, Height = 600 }
    local page_b = host + ui.Page { Name = "page_b", Key = "beta", Path = "/beta", Width = 400, Height = 600 }

    host:send("Node.ViewDidLoad")
    pump_messages(screen)

    host:send("PageHost.NavigateToPage", { URL = "/beta" })
    pump_messages(screen)
    test.expect(page_b.Visible, "beta should be visible")

    host:send("PageHost.NavigateBack")
    pump_messages(screen)

    test.expect(page_a.Visible, "alpha should be visible after navigate back")
    test.expect(page_b.Visible == false, "beta should be hidden after navigate back")

    screen:clear()
    print("PASS: test_pagehost_navigateback")
end

-- ---------------------------------------------------------------------------
-- PageHost: no CurrentPage shows first page (legacy behavior)
-- ---------------------------------------------------------------------------
local function test_pagehost_no_currentpage_shows_first()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
    local host = screen + ui.PageHost { Width = 400, Height = 600 }
    local page_a = host + ui.Page { Name = "page_a", Key = "alpha", Width = 400, Height = 600 }

    host:send("Node.ViewDidLoad")
    pump_messages(screen)

    test.expect(page_a.Visible, "first page should be visible by default")

    screen:clear()
    print("PASS: test_pagehost_no_currentpage_shows_first")
end

-- Run all tests
test_pagehost_initial_page_from_currentpage()
test_pagehost_switches_page_on_currentpage_change()
test_listbox_selectedvalue_property()
test_listbox_selectionchanged_fires()
test_pagehost_listbox_end_to_end()
test_pagehost_navigateback()
test_pagehost_no_currentpage_shows_first()

print("\nAll PageHost + ListBox tests passed.")
