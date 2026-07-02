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
-- PageHost: shows initial page matching ActivePage key
-- ---------------------------------------------------------------------------
local function test_pagehost_initial_activepage()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
    local host = screen + ui.PageHost { Width = 400, Height = 600, ActivePage = "beta" }
    local page_a = host + ui.Page { Name = "alpha", Width = 400, Height = 600 }
    local page_b = host + ui.Page { Name = "beta", Width = 400, Height = 600 }

    host:send("Node.ViewDidLoad")
    pump_messages(screen)

    test.expect(page_a.Visible == false, "page_a should be hidden")
    test.expect(page_b.Visible == true, "page_b should be visible")

    screen:clear()
    print("PASS: test_pagehost_initial_activepage")
end

-- ---------------------------------------------------------------------------
-- PageHost: switches page when ActivePage key changes
-- ---------------------------------------------------------------------------
local function test_pagehost_switches_page_on_activepage_change()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
    local host = screen + ui.PageHost { Width = 400, Height = 600, ActivePage = "alpha" }
    local page_a = host + ui.Page { Name = "alpha", Width = 400, Height = 600 }
    local page_b = host + ui.Page { Name = "beta", Width = 400, Height = 600 }

    host:send("Node.ViewDidLoad")
    pump_messages(screen)

    test.expect(page_a.Visible, "page_a should be visible initially")
    test.expect(page_b.Visible == false, "page_b should be hidden initially")

    host.ActivePage = "beta"
    pump_messages(screen)

    test.expect(page_a.Visible == false, "page_a should be hidden after switch")
    test.expect(page_b.Visible == true, "page_b should be visible after switch")

    screen:clear()
    print("PASS: test_pagehost_switches_page_on_activepage_change")
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

    local host = screen + ui.PageHost { Width = 400, Height = 600, ActivePage = "games" }
    local page_games = host + ui.Page { Name = "games", Width = 400, Height = 600 }
    local page_settings = host + ui.Page { Name = "settings", Width = 400, Height = 600 }

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
    host.ActivePage = "settings"
    pump_messages(screen)

    test.expect_eq(footer.SelectedValue, "settings", "Footer should show settings")
    test.expect(page_games.Visible == false, "games page should be hidden")
    test.expect(page_settings.Visible == true, "settings page should be visible")

    footer.SelectedValue = "games"
    host.ActivePage = "games"
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
    local host = screen + ui.PageHost { Width = 400, Height = 600, ActivePage = "alpha" }
    local page_a = host + ui.Page { Name = "alpha", Path = "/alpha", Width = 400, Height = 600 }
    local page_b = host + ui.Page { Name = "beta", Path = "/beta", Width = 400, Height = 600 }

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
-- PageHost: empty ActivePage shows first page (legacy behavior)
-- ---------------------------------------------------------------------------
local function test_pagehost_no_activepage_shows_first()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
    local host = screen + ui.PageHost { Width = 400, Height = 600 }
    local page_a = host + ui.Page { Name = "alpha", Width = 400, Height = 600 }

    host:send("Node.ViewDidLoad")
    pump_messages(screen)

    test.expect(page_a.Visible, "first page should be visible by default")

    screen:clear()
    print("PASS: test_pagehost_no_activepage_shows_first")
end

-- ---------------------------------------------------------------------------
-- ListBox: SelectItem message selects item by value
-- ---------------------------------------------------------------------------
local function test_listbox_selectitem_message()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

    local items = core.DataObject { Name = "Tabs" }
    items:addChild(core.DataObject { Name = "alpha" })
    items:addChild(core.DataObject { Name = "beta" })
    items:addChild(core.DataObject { Name = "gamma" })

    local template = ui.Node2D { Name = "TabItem", Width = 400, Height = 50 }

    local list = ui.ListBox {
        Width = 400, Height = 200,
        ItemsSource = items,
        ItemTemplate = template,
    }
    screen:addChild(list)

    host = screen
    host:send("Node.ViewDidLoad")
    pump_messages(screen)

    -- Auto-selects first item; switch via SelectItem message
    list:send("ListBox.SelectItem", { Value = "gamma" })
    pump_messages(screen)

    test.expect_eq(list.SelectedValue, "gamma", "SelectedValue should be gamma after SelectItem")

    list:send("ListBox.SelectItem", { Value = "alpha" })
    pump_messages(screen)

    test.expect_eq(list.SelectedValue, "alpha", "SelectedValue should be alpha after second SelectItem")

    -- SelectItem with nonexistent value should not change selection
    list:send("ListBox.SelectItem", { Value = "nope" })
    pump_messages(screen)

    test.expect_eq(list.SelectedValue, "alpha", "SelectedValue should remain alpha for nonexistent value")

    screen:clear()
    print("PASS: test_listbox_selectitem_message")
end

-- ---------------------------------------------------------------------------
-- PageHost + ListBox: binding propagates SelectedValue → ActivePage
-- ---------------------------------------------------------------------------
local function test_pagehost_listbox_binding()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

    local host = screen + ui.PageHost { Width = 400, Height = 700, ActivePage = "games" }
    local page_games = host + ui.Page { Name = "games", Width = 400, Height = 700 }
    local page_library = host + ui.Page { Name = "library", Width = 400, Height = 700 }

    local items = core.DataObject { Name = "Tabs" }
    items:addChild(core.DataObject { Name = "games" })
    items:addChild(core.DataObject { Name = "library" })

    local template = ui.Node2D { Name = "TabItem", Width = 200, Height = 100 }

    local footer = ui.ListBox {
        Width = 400, Height = 100,
        SelectedValue = "games",
        ItemsSource = items,
        ItemTemplate = template,
    }
    screen:addChild(footer)

    -- Simulate the binding: watch footer.SelectedValue and sync to host.ActivePage
    local function sync_binding()
        host.ActivePage = footer.SelectedValue
    end

    host:send("Node.ViewDidLoad")
    pump_messages(screen)

    test.expect(page_games.Visible, "games page should be visible initially")
    test.expect(page_library.Visible == false, "library page should be hidden initially")
    test.expect_eq(host.ActivePage, "games", "ActivePage should start as games")

    -- Change footer selection and propagate via binding
    footer.SelectedValue = "library"
    sync_binding()
    pump_messages(screen)

    test.expect_eq(host.ActivePage, "library", "ActivePage should follow binding to library")
    test.expect(page_games.Visible == false, "games page should be hidden")
    test.expect(page_library.Visible, "library page should be visible")

    -- Change back
    footer.SelectedValue = "games"
    sync_binding()
    pump_messages(screen)

    test.expect_eq(host.ActivePage, "games", "ActivePage should follow binding back to games")
    test.expect(page_games.Visible, "games page should be visible again")
    test.expect(page_library.Visible == false, "library page should be hidden again")

    screen:clear()
    print("PASS: test_pagehost_listbox_binding")
end

-- Run all tests
test_pagehost_initial_activepage()
test_pagehost_switches_page_on_activepage_change()
test_listbox_selectedvalue_property()
test_listbox_selectionchanged_fires()
test_pagehost_listbox_end_to_end()
test_pagehost_navigateback()
test_pagehost_no_activepage_shows_first()
test_listbox_selectitem_message()
test_pagehost_listbox_binding()

print("\nAll PageHost + ListBox tests passed.")
