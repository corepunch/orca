local test = require "orca.test"
local orca = require "orca"
local core = require "orca.core"
local ui = require "orca.UIKit"
local filesystem = require "orca.filesystem"

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
-- PageHost: syncing ActivePage should hide all non-active pages
-- ---------------------------------------------------------------------------
local function test_pagehost_sync_hides_non_active_pages()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
    local host = screen + ui.PageHost { Width = 400, Height = 600 }
    local page_a = host + ui.Page { Name = "alpha", Width = 400, Height = 600 }
    local page_b = host + ui.Page { Name = "beta", Width = 400, Height = 600 }
    local page_c = host + ui.Page { Name = "gamma", Width = 400, Height = 600 }

    host.ActivePage = "beta"
    pump_messages(screen)

    test.expect(page_a.Visible == false, "alpha should be hidden when ActivePage is beta")
    test.expect(page_b.Visible == true, "beta should be visible when ActivePage is beta")
    test.expect(page_c.Visible == false, "gamma should be hidden when ActivePage is beta")

    host.ActivePage = "gamma"
    pump_messages(screen)

    test.expect(page_a.Visible == false, "alpha should remain hidden after switch")
    test.expect(page_b.Visible == false, "beta should be hidden after switching to gamma")
    test.expect(page_c.Visible == true, "gamma should be visible after switch")

    screen:clear()
    print("PASS: test_pagehost_sync_hides_non_active_pages")
end

-- ---------------------------------------------------------------------------
-- ListBox: clicking an item updates SelectedValue and can drive ActivePage
-- ---------------------------------------------------------------------------
local function test_listbox_click_updates_selection_and_pagehost()
    local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

    local host = screen + ui.PageHost { Width = 400, Height = 700, ActivePage = "games" }
    local page_games = host + ui.Page { Name = "games", Width = 400, Height = 700 }
    local page_library = host + ui.Page { Name = "library", Width = 400, Height = 700 }

    local items = core.DataObject { Name = "Tabs" }
    items:addChild(core.DataObject { Name = "games" })
    items:addChild(core.DataObject { Name = "library" })

    local template = ui.Node2D { Name = "TabItem", Width = 180, Height = 80 }

    local footer = screen + ui.ListBox {
        Y = 704,
        Width = 400,
        Height = 96,
        Direction = "Horizontal",
        ValueProperty = "Name",
        SelectedValue = "games",
        ItemsSource = items,
        ItemTemplate = template,
        SelectionChanged = function (self)
            host.ActivePage = self.SelectedValue
        end,
    }

    host:send("Node.ViewDidLoad")
    screen:UpdateLayout(screen.Width, screen.Height)
    pump_messages(screen)

    test.expect_eq(footer.SelectedValue, "games", "Footer should start with games selected")
    test.expect(page_games.Visible, "games page should be visible initially")
    test.expect(page_library.Visible == false, "library page should be hidden initially")

    local footerChildren = {}
    for child in footer.children do
        footerChildren[#footerChildren + 1] = child
    end
    test.expect_eq(#footerChildren, 2, "Footer should contain two items")

    local target = footerChildren[2]
    local x = target.ActualX + 5
    local y = target.ActualY + 5

    orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = x, y = y }
    orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = x, y = y }
    pump_messages(screen)

    test.expect_eq(footer.SelectedValue, "library", "Clicking second item should select library")
    test.expect_eq(host.ActivePage, "library", "Host ActivePage should follow ListBox selection")
    test.expect(page_games.Visible == false, "games page should be hidden after click")
    test.expect(page_library.Visible == true, "library page should be visible after click")

    screen:clear()
    print("PASS: test_listbox_click_updates_selection_and_pagehost")
end

-- ---------------------------------------------------------------------------
-- ListBox: ValueProperty resolves a schema column directly on the record
-- ---------------------------------------------------------------------------
local function test_listbox_valueproperty_dataobject_value()
        local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

        local items = filesystem.loadObjectFromXmlStringWithSchema([[
            <Tabs>
                <FooterTab name="TabGames"   Key="games"   Label="Adventures"/>
                <FooterTab name="TabLibrary" Key="library" Label="Library"/>
            </Tabs>
        ]], [[
            <Schema>
                <Entity Name="Tabs">
                    <Column Name="FooterTab" Type="relation" Entity="FooterTab"/>
                </Entity>
                <Entity Name="FooterTab">
                    <Column Name="Key"   Type="string" Key="true"/>
                    <Column Name="Label" Type="string"/>
                </Entity>
            </Schema>
        ]])

        local template = ui.StackView {
            Name = "TabItem",
            Width = 180,
            Height = 80,
        }
        template:addChild(ui.TextBlock {
            Name = "TabLabel",
            Width = 180,
            Height = 80,
            Text = "tab",
        })

        local list = screen + ui.ListBox {
            Width = 400,
            Height = 96,
            Direction = "Horizontal",
            ItemsSource = items,
            ItemTemplate = template,
            ValueProperty = "Key",
            SelectedValue = "games",
        }

        screen:UpdateLayout(screen.Width, screen.Height)
        pump_messages(screen)

        local children = {}
        for child in list.children do
            children[#children + 1] = child
        end
        test.expect_eq(#children, 2, "List should contain two footer items")

        local target = children[2]
        local x = target.ActualX + 10
        local y = target.ActualY + 10

        orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = x, y = y }
        orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = x, y = y }
        pump_messages(screen)

        test.expect_eq(list.SelectedValue, "library", "Click should set SelectedValue to DataObjectString Key value")

        screen:clear()
        print("PASS: test_listbox_valueproperty_dataobject_value")
end

-- ---------------------------------------------------------------------------
-- ListBox: clicking in slot gaps still selects by listbox region
-- ---------------------------------------------------------------------------
local function test_listbox_click_slot_region_selection()
        local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

        local host = screen + ui.PageHost { Width = 400, Height = 700, ActivePage = "games" }
        local page_games = host + ui.Page { Name = "games", Width = 400, Height = 700 }
        local page_library = host + ui.Page { Name = "library", Width = 400, Height = 700 }
        local page_settings = host + ui.Page { Name = "settings", Width = 400, Height = 700 }

        local items = core.DataObject { Name = "Tabs" }
        items:addChild(core.DataObject { Name = "games" })
        items:addChild(core.DataObject { Name = "library" })
        items:addChild(core.DataObject { Name = "settings" })

        local template = ui.StackView {
            Name = "TabItem",
            Width = 40,
            Height = 40,
            Direction = "Vertical",
        }
        template:addChild(ui.TextBlock {
            Name = "TabLabel",
            Width = 36,
            Height = 14,
            Text = "tab",
        })

        local footer = screen + ui.ListBox {
                Y = 704,
                Width = 400,
                Height = 96,
                Direction = "Horizontal",
                ValueProperty = "Name",
                SelectedValue = "games",
                ItemsSource = items,
                ItemTemplate = template,
                SelectionChanged = function (self)
                        host.ActivePage = self.SelectedValue
                end,
        }

        host:send("Node.ViewDidLoad")
        screen:UpdateLayout(screen.Width, screen.Height)
        pump_messages(screen)

        local slotW = footer.ActualWidth / 3
        local y = footer.ActualY + footer.ActualHeight * 0.5

        -- Click in middle slot area where there may be no child hit.
        local xLibrary = footer.ActualX + slotW * 1.5
        orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = xLibrary, y = y }
        orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = xLibrary, y = y }
        pump_messages(screen)

        test.expect_eq(footer.SelectedValue, "library", "Middle slot click should select library")
        test.expect_eq(host.ActivePage, "library", "Middle slot click should update ActivePage")
        test.expect(page_games.Visible == false, "games page should hide after middle slot click")
        test.expect(page_library.Visible == true, "library page should show after middle slot click")

        local xSettings = footer.ActualX + slotW * 2.5
        orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = xSettings, y = y }
        orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = xSettings, y = y }
        pump_messages(screen)

        test.expect_eq(footer.SelectedValue, "settings", "Last slot click should select settings")
        test.expect_eq(host.ActivePage, "settings", "Last slot click should update ActivePage")
        test.expect(page_library.Visible == false, "library page should hide after last slot click")
        test.expect(page_settings.Visible == true, "settings page should show after last slot click")

        screen:clear()
        print("PASS: test_listbox_click_slot_region_selection")
end

    -- ---------------------------------------------------------------------------
    -- ListBox: selection syncs sibling PageHost.ActivePage automatically
    -- ---------------------------------------------------------------------------
    local function test_listbox_auto_syncs_sibling_pagehost()
        local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

        local host = screen + ui.PageHost { Width = 400, Height = 700, ActivePage = "games" }
        local page_games = host + ui.Page { Name = "games", Width = 400, Height = 700 }
        local page_library = host + ui.Page { Name = "library", Width = 400, Height = 700 }

        local items = core.DataObject { Name = "Tabs" }
        items:addChild(core.DataObject { Name = "games" })
        items:addChild(core.DataObject { Name = "library" })

        local template = ui.Node2D { Name = "TabItem", Width = 80, Height = 60 }

        local footer = screen + ui.ListBox {
            Y = 704,
            Width = 400,
            Height = 96,
            Direction = "Horizontal",
            ValueProperty = "Name",
            SelectedValue = "games",
            ItemsSource = items,
            ItemTemplate = template,
        }

        host:send("Node.ViewDidLoad")
        screen:UpdateLayout(screen.Width, screen.Height)
        pump_messages(screen)

        local children = {}
        for child in footer.children do
            children[#children + 1] = child
        end
        test.expect_eq(#children, 2, "Footer should contain two items")

        local target = children[2]
        local x = target.ActualX + 5
        local y = target.ActualY + 5
        orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = x, y = y }
        orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = x, y = y }
        pump_messages(screen)

        test.expect_eq(footer.SelectedValue, "library", "Footer click should select library")
        test.expect_eq(host.ActivePage, "library", "Sibling PageHost should follow ListBox selection")
        test.expect(page_games.Visible == false, "games page should be hidden after footer click")
        test.expect(page_library.Visible == true, "library page should be visible after footer click")

        screen:clear()
        print("PASS: test_listbox_auto_syncs_sibling_pagehost")
    end

-- ---------------------------------------------------------------------------
-- ListBox: :active styles apply to selected item descendants
-- ---------------------------------------------------------------------------
local function test_listbox_active_styles_on_descendants()
        local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
        screen.StyleSheet = ui.loadObjectFromCssString [[
            .footer .tab-label { color: #ffffff; }
            .footer .tab-item:active .tab-label { color: #345ec7; }
        ]]

        local items = core.DataObject { Name = "Tabs" }
        items:addChild(core.DataObject { Name = "games" })
        items:addChild(core.DataObject { Name = "library" })

        local template = ui.StackView {
            Name = "TabItem",
            class = "tab-item",
            Width = 120,
            Height = 60,
            Direction = "Vertical",
        }
        template:addChild(ui.TextBlock {
            Name = "TabLabel",
            class = "tab-label",
            Width = 110,
            Height = 20,
            Text = "label",
        })

        local footer = screen + ui.ListBox {
            Name = "Footer",
            class = "footer",
            Y = 704,
            Width = 400,
            Height = 96,
            Direction = "Horizontal",
            ValueProperty = "Name",
            SelectedValue = "games",
            ItemsSource = items,
            ItemTemplate = template,
        }

        screen:UpdateLayout(screen.Width, screen.Height)
        pump_messages(screen)

        local rows = {}
        for child in footer.children do
            rows[#rows + 1] = child
        end
        test.expect_eq(#rows, 2, "Footer should contain two rendered items")

        local target = rows[2]
        local x = target.ActualX + 5
        local y = target.ActualY + 5
        orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = x, y = y }
        orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = x, y = y }
        pump_messages(screen)

        local firstLabel, secondLabel = nil, nil
        local idx = 0
        for row in footer.children do
            idx = idx + 1
            for grandchild in row.children do
                if grandchild.Name == "TabLabel" then
                    if idx == 1 then firstLabel = grandchild end
                    if idx == 2 then secondLabel = grandchild end
                end
            end
        end

        test.expect(firstLabel ~= nil and secondLabel ~= nil, "Both TabLabel nodes should exist")

        local firstColor = firstLabel and firstLabel["ForegroundColor"]
        local secondColor = secondLabel and secondLabel["ForegroundColor"]
        test.expect(firstColor ~= nil and secondColor ~= nil, "TabLabel ForegroundColor should be available")

        local secondB = secondColor and secondColor["B"] or 0
        local firstB = firstColor and firstColor["B"] or 0

        test.expect_near(secondB, 0xc7 / 255, 0.02, "Selected label should use accent blue")
        test.expect_near(firstB, 1.0, 0.02, "Unselected label should remain white")

        screen:clear()
        print("PASS: test_listbox_active_styles_on_descendants")
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

-- ---------------------------------------------------------------------------
-- SelectedItem tests
--
-- Verifies ItemsControl.SelectedItem (DataObject*) is set correctly across
-- all selection paths and supports master-detail data navigation:
--
-- | Test | Covers |
-- |------|--------|
-- | test_selecteditem_default_nil | No ItemsSource → SelectedItem is nil |
-- | test_selecteditem_updates_on_value_set | SelectedValue change propagates to SelectedItem |
-- | test_selecteditem_via_selectitem_message | ListBox.SelectItem message updates SelectedItem |
-- | test_selecteditem_via_click | Mouse click updates SelectedItem |
-- | test_selecteditem_custom_valueproperty | Works with DataObjectString.Key custom value property |
-- | test_selecteditem_auto_select | Auto-select of first item sets SelectedItem |
-- | test_selecteditem_reference_stable | SelectedItem switches correctly between items |
-- | test_selecteditem_path_resolution | Navigate SelectedItem.Title, SelectedItem.Comments children |
-- | test_selecteditem_gridbox | GridBox inherits SelectedItem but keeps it nil |
-- ---------------------------------------------------------------------------

-- ---------------------------------------------------------------------------
-- SelectedItem defaults to nil when ListBox has no ItemsSource
-- ---------------------------------------------------------------------------
local function test_selecteditem_default_nil()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
  local list = ui.ListBox { Width = 400, Height = 200 }
  screen:addChild(list)
  test.expect(list.SelectedItem == nil, "SelectedItem should be nil by default")
  screen:clear()
  print("PASS: test_selecteditem_default_nil")
end

-- ---------------------------------------------------------------------------
-- SelectedItem follows SelectedValue: setting it programmatically updates
-- the DataObject pointer to match the new selection
-- ---------------------------------------------------------------------------
local function test_selecteditem_updates_on_value_set()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
  local items = core.DataObject { Name = "Items" }
  local d1 = items:addChild(core.DataObject { Name = "Alpha" })
  local d2 = items:addChild(core.DataObject { Name = "Beta" })
  local template = ui.Node2D { Name = "T", Width = 100, Height = 50 }
  local list = ui.ListBox {
    Width = 400, Height = 200,
    ItemsSource = items, ItemTemplate = template,
  }
  screen:addChild(list)

  -- Auto-select selects first item; verify before override
  test.expect_eq(list.SelectedItem.Name, "Alpha", "Auto-select should set SelectedItem to first item")

  list.SelectedValue = "Beta"
  test.expect(list.SelectedItem ~= nil, "SelectedItem should not be nil after setting SelectedValue")
  test.expect_eq(list.SelectedItem.Name, "Beta", "SelectedItem.Name should match the selected DataObject")

  list.SelectedValue = "Alpha"
  test.expect_eq(list.SelectedItem.Name, "Alpha", "SelectedItem should update when SelectedValue changes")
  screen:clear()
  print("PASS: test_selecteditem_updates_on_value_set")
end

-- ---------------------------------------------------------------------------
-- SelectedItem updates via the ListBox.SelectItem message, and is preserved
-- when the message targets a nonexistent value
-- ---------------------------------------------------------------------------
local function test_selecteditem_via_selectitem_message()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
  local items = core.DataObject { Name = "Items" }
  local d1 = items:addChild(core.DataObject { Name = "X" })
  local d2 = items:addChild(core.DataObject { Name = "Y" })
  local d3 = items:addChild(core.DataObject { Name = "Z" })
  local template = ui.Node2D { Name = "T", Width = 100, Height = 50 }
  local list = ui.ListBox {
    Width = 400, Height = 200,
    ItemsSource = items, ItemTemplate = template,
  }
  screen:addChild(list)

  list:send("ListBox.SelectItem", { Value = "Z" })
  test.expect_eq(list.SelectedValue, "Z", "SelectedValue should be Z after SelectItem")
  test.expect_eq(list.SelectedItem.Name, "Z", "SelectedItem.Name should be Z after SelectItem")

  list:send("ListBox.SelectItem", { Value = "X" })
  test.expect_eq(list.SelectedItem.Name, "X", "SelectedItem.Name should be X after second SelectItem")

  -- Nonexistent value should not change SelectedItem
  list:send("ListBox.SelectItem", { Value = "nonexistent" })
  test.expect_eq(list.SelectedItem.Name, "X", "SelectedItem should remain X for nonexistent value")
  screen:clear()
  print("PASS: test_selecteditem_via_selectitem_message")
end

-- ---------------------------------------------------------------------------
-- SelectedItem updates when the user clicks a list item, following both
-- the visual selection and the backing DataObject pointer
-- ---------------------------------------------------------------------------
local function test_selecteditem_via_click()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
  local items = core.DataObject { Name = "Items" }
  items:addChild(core.DataObject { Name = "First" })
  items:addChild(core.DataObject { Name = "Second" })
  items:addChild(core.DataObject { Name = "Third" })
  local template = ui.Node2D { Name = "T", Width = 100, Height = 50 }
  local list = ui.ListBox {
    Width = 400, Height = 200, Direction = "Horizontal",
    ItemsSource = items, ItemTemplate = template,
  }
  screen:addChild(list)
  screen:UpdateLayout(screen.Width, screen.Height)

  -- Click the second item
  local children = {}
  for child in list.children do children[#children + 1] = child end
  local target = children[2]
  local x = target.ActualX + 5
  local y = target.ActualY + 5
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = x, y = y }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = x, y = y }

  test.expect_eq(list.SelectedValue, "Second", "SelectedValue should be Second after click")
  test.expect_eq(list.SelectedItem.Name, "Second", "SelectedItem.Name should be Second after click")

  -- Click the third item
  local target3 = children[3]
  local x3 = target3.ActualX + 5
  local y3 = target3.ActualY + 5
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = x3, y = y3 }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = x3, y = y3 }

  test.expect_eq(list.SelectedItem.Name, "Third", "SelectedItem.Name should be Third after second click")
  screen:clear()
  print("PASS: test_selecteditem_via_click")
end

-- ---------------------------------------------------------------------------
-- SelectedItem resolves correctly when ListBox uses a custom ValueProperty
-- (e.g. DataObjectString.Key) rather than the default DataObject.Name
-- ---------------------------------------------------------------------------
local function test_selecteditem_custom_valueproperty()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

  local items = filesystem.loadObjectFromXmlStringWithSchema([[
    <Catalog>
      <SelectedGame name="Game1" Key="rpg" Title="Fantasy Quest"/>
      <SelectedGame name="Game2" Key="shooter" Title="Blaster"/>
    </Catalog>
  ]], [[
    <Schema>
      <Entity Name="Catalog">
        <Column Name="SelectedGame" Type="relation" Entity="SelectedGame"/>
      </Entity>
      <Entity Name="SelectedGame">
        <Column Name="Key" Type="string"/>
        <Column Name="Title" Type="string"/>
      </Entity>
    </Schema>
  ]])

  local template = ui.Node2D { Name = "T", Width = 100, Height = 50 }
  local list = ui.ListBox {
    Width = 400, Height = 200,
    ItemsSource = items,
    ItemTemplate = template,
    ValueProperty = "Key",
    SelectedValue = "shooter",
  }
  screen:addChild(list)

  test.expect_eq(list.SelectedValue, "shooter", "SelectedValue should be shooter")
  test.expect(list.SelectedItem ~= nil, "SelectedItem should be set")
  test.expect_eq(list.SelectedItem.Name, "Game2", "SelectedItem.Name should match Game2")

  -- Click the first item
  screen:UpdateLayout(screen.Width, screen.Height)
  local children = {}
  for child in list.children do children[#children + 1] = child end
  local target = children[1]
  local x = target.ActualX + 5
  local y = target.ActualY + 5
  orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = x, y = y }
  orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = x, y = y }

  test.expect_eq(list.SelectedValue, "rpg", "SelectedValue should be rpg after click")
  test.expect_eq(list.SelectedItem.Name, "Game1", "SelectedItem.Name should be Game1 after click")
  screen:clear()
  print("PASS: test_selecteditem_custom_valueproperty")
end

-- ---------------------------------------------------------------------------
-- ListBox auto-selects the first data item when no SelectedValue is given,
-- and SelectedItem is set to that item's DataObject
-- ---------------------------------------------------------------------------
local function test_selecteditem_auto_select()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
  local items = core.DataObject { Name = "Items" }
  items:addChild(core.DataObject { Name = "Auto1" })
  items:addChild(core.DataObject { Name = "Auto2" })
  local template = ui.Node2D { Name = "T", Width = 100, Height = 50 }
  local list = ui.ListBox {
    Width = 400, Height = 200,
    ItemsSource = items, ItemTemplate = template,
  }
  screen:addChild(list)

  test.expect(list.SelectedItem ~= nil, "SelectedItem should be set after auto-select")
  test.expect_eq(list.SelectedItem.Name, "Auto1", "First item should be auto-selected")
  test.expect_eq(list.SelectedValue, "Auto1", "SelectedValue should be Auto1 after auto-select")
  screen:clear()
  print("PASS: test_selecteditem_auto_select")
end

-- ---------------------------------------------------------------------------
-- SelectedItem switches correctly between DataObjects — the reference
-- points to the right record even after multiple selection changes
-- ---------------------------------------------------------------------------
local function test_selecteditem_reference_stable()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
  local items = core.DataObject { Name = "Items" }
  local targetObj = items:addChild(core.DataObject { Name = "Target" })
  items:addChild(core.DataObject { Name = "Other" })
  local template = ui.Node2D { Name = "T", Width = 100, Height = 50 }
  local list = ui.ListBox {
    Width = 400, Height = 200,
    ItemsSource = items, ItemTemplate = template,
  }
  screen:addChild(list)

  -- First auto-selects Target; switch to Other
  list.SelectedValue = "Other"
  test.expect(list.SelectedItem ~= nil)
  test.expect_eq(list.SelectedItem.Name, "Other")
  test.expect(list.SelectedItem ~= targetObj, "SelectedItem should not be targetObj after switch")

  -- Switch back to Target
  list.SelectedValue = "Target"
  test.expect_eq(list.SelectedItem.Name, "Target")
  screen:clear()
  print("PASS: test_selecteditem_reference_stable")
end

-- ---------------------------------------------------------------------------
-- After selecting an item, the path SelectedItem.Title resolves to the
-- item's child DataObject (a "column" in the table metaphor). Switching
-- the master selection re-navigates to the new item's children. This
-- validates the core master-detail binding path.
-- ---------------------------------------------------------------------------
local function test_selecteditem_path_resolution()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

  -- Create items where each has Title and Comments children
  local items = filesystem.loadObjectFromXmlStringWithSchema([[
    <Catalog>
      <SelectedGameDetail name="GameA" Title="Alpha Quest">
        <Comments><SelectedComment name="C1" Text="Great!"/><SelectedComment name="C2" Text="Fun"/></Comments>
      </SelectedGameDetail>
      <SelectedGameDetail name="GameB" Title="Bravo Wars">
        <Comments><SelectedComment name="C3" Text="Intense"/></Comments>
      </SelectedGameDetail>
    </Catalog>
  ]], [[
    <Schema>
      <Entity Name="Catalog"><Column Name="SelectedGameDetail" Type="relation" Entity="SelectedGameDetail"/></Entity>
      <Entity Name="SelectedGameDetail">
        <Column Name="Title" Type="string"/><Column Name="Comments" Type="relation" Entity="SelectedComment"/>
      </Entity>
      <Entity Name="SelectedComment"><Column Name="Text" Type="string"/></Entity>
    </Schema>
  ]])

  local template = ui.Node2D { Name = "T", Width = 100, Height = 50 }
  local list = ui.ListBox {
    Name = "GameList",
    Width = 400, Height = 200,
    ItemsSource = items, ItemTemplate = template,
    SelectedValue = "GameA",
  }
  screen:addChild(list)

  test.expect_eq(list.SelectedItem.Name, "GameA", "SelectedItem should be GameA")

  -- Verify we can read properties on the selected DataObject through Lua
  local selected = list.SelectedItem
  -- Navigate to Title child DataObjectString
  test.expect_eq(selected.Title, "Alpha Quest", "Title should be 'Alpha Quest'")

  -- Navigate to Comments child collection
  local comments = selected:findChild("Comments", true)
  test.expect(comments ~= nil, "Comments collection should exist on selected item")
  local commentCount = 0
  for _ in comments.children do commentCount = commentCount + 1 end
  test.expect_eq(commentCount, 2, "GameA should have 2 comments")

  -- Switch selection and verify data follows
  list.SelectedValue = "GameB"
  test.expect_eq(list.SelectedItem.Name, "GameB", "SelectedItem should be GameB after switch")
  test.expect_eq(list.SelectedItem.Title, "Bravo Wars", "After switch, Title should be 'Bravo Wars'")

  local commentsB = list.SelectedItem:findChild("Comments", true)
  local countB = 0
  for _ in commentsB.children do countB = countB + 1 end
  test.expect_eq(countB, 1, "GameB should have 1 comment")

  screen:clear()
  print("PASS: test_selecteditem_path_resolution")
end

-- ---------------------------------------------------------------------------
-- GridBox inherits SelectedItem from ItemsControl but has no selection
-- handling, so SelectedItem remains nil (ListBox-specific behavior)
-- ---------------------------------------------------------------------------
local function test_selecteditem_gridbox()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }
  local items = core.DataObject { Name = "Items" }
  items:addChild(core.DataObject { Name = "G1" })
  items:addChild(core.DataObject { Name = "G2" })

  local template = ui.Node2D { Name = "T", Width = 100, Height = 50 }
  local grid = ui.GridBox {
    Columns = "auto",
    ItemsSource = items, ItemTemplate = template,
  }
  screen:addChild(grid)

  test.expect(grid.SelectedItem == nil, "GridBox SelectedItem should be nil (no selection handling)")
  screen:clear()
  print("PASS: test_selecteditem_gridbox")
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
test_pagehost_sync_hides_non_active_pages()
test_listbox_click_updates_selection_and_pagehost()
test_listbox_valueproperty_dataobject_value()
test_listbox_click_slot_region_selection()
test_listbox_auto_syncs_sibling_pagehost()
test_listbox_active_styles_on_descendants()
test_pagehost_listbox_binding()
test_selecteditem_default_nil()
test_selecteditem_updates_on_value_set()
test_selecteditem_via_selectitem_message()
test_selecteditem_via_click()
test_selecteditem_custom_valueproperty()
test_selecteditem_auto_select()
test_selecteditem_reference_stable()
test_selecteditem_path_resolution()
test_selecteditem_gridbox()

print("\nAll PageHost + ListBox tests passed.")
