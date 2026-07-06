local test = require "orca.test"
local orca = require "orca"
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
-- NavigationHost: pushed pages replace the tab shell and inherit row context
-- ---------------------------------------------------------------------------
local function test_navigationhost_push_pop()
    local screen = filesystem.loadObjectFromXmlString([[
      <Screen Name="Test" Width="400" Height="800" ResizeMode="NoResize">
        <NavigationHost Name="Navigation" Width="400" Height="800">
          <Page Name="RootPage" Width="400" Height="800">
            <TextBlock Name="Card" Text="Game" DataContext="{DataObject Name=zork1}"
              LeftButtonUp="{NavigationHost.Push 'Adventure/Pages/Game.xml'}"/>
          </Page>
        </NavigationHost>
      </Screen>
    ]])
    local navigation = screen:findChild("Navigation", true)
    local root = screen:findChild("RootPage", true)
    local card = screen:findChild("Card", true)

    navigation:send("Node.ViewDidLoad")
    card:send("Node.LeftButtonUp")
    pump_messages(screen)

    local detail = screen:findChild("GamePage", true)
    test.expect(detail ~= nil, "push should load the game page")
    test.expect(detail.Visible, "pushed page should be visible")
    test.expect(root.Visible == false, "root page should hide while detail is pushed")
    test.expect(navigation.CanGoBack, "navigation host should report that it can go back")
    test.expect(detail.DataContext ~= nil, "pushed page should inherit the card data context")

    screen:findChild("GameBack", true):send("Node.LeftButtonUp")
    pump_messages(screen)

    test.expect(root.Visible, "root page should be restored after pop")
    test.expect(navigation.CanGoBack == false, "root page should not allow another pop")
    test.expect(screen:findChild("GamePage", true) == nil, "popped page should be removed")

    screen:clear()
    print("PASS: test_navigationhost_push_pop")
end

-- ---------------------------------------------------------------------------
-- NavigationHost: CanGoBack is false on root, true after push
-- ---------------------------------------------------------------------------
local function test_navigationhost_cangoback()
    local screen = filesystem.loadObjectFromXmlString([[
      <Screen Name="Test" Width="400" Height="800" ResizeMode="NoResize">
        <NavigationHost Name="Navigation" Width="400" Height="800">
          <Page Name="RootPage" Width="400" Height="800"/>
        </NavigationHost>
      </Screen>
    ]])
    local navigation = screen:findChild("Navigation", true)

    test.expect(navigation.CanGoBack == false, "CanGoBack should be false on root")
    navigation:send("Node.ViewDidLoad")
    pump_messages(screen)

    test.expect(navigation.CanGoBack == false, "CanGoBack should remain false with only root")

    navigation:send("NavigationHost.Push", { Path = "Adventure/Pages/Game.xml" })
    pump_messages(screen)

    test.expect(navigation.CanGoBack, "CanGoBack should be true after push")

    navigation:send("NavigationHost.Pop")
    pump_messages(screen)

    test.expect(navigation.CanGoBack == false, "CanGoBack should be false after pop to root")

    screen:clear()
    print("PASS: test_navigationhost_cangoback")
end

-- ---------------------------------------------------------------------------
-- NavigationHost: multiple pushes build stack, pops unwind in LIFO order
-- ---------------------------------------------------------------------------
local function test_navigationhost_push_pop_stack()
    local screen = filesystem.loadObjectFromXmlString([[
      <Screen Name="Test" Width="400" Height="800" ResizeMode="NoResize">
        <NavigationHost Name="Navigation" Width="400" Height="800">
          <Page Name="RootPage" Width="400" Height="800">
            <TextBlock Name="RootLabel" Text="Root"/>
          </Page>
        </NavigationHost>
      </Screen>
    ]])
    local navigation = screen:findChild("Navigation", true)
    local root = screen:findChild("RootPage", true)

    navigation:send("Node.ViewDidLoad")
    pump_messages(screen)

    -- Push first page
    navigation:send("NavigationHost.Push", { Path = "Adventure/Pages/Game.xml" })
    pump_messages(screen)
    local page1 = screen:findChild("GamePage", true)
    test.expect(page1 ~= nil, "first push should load GamePage")
    test.expect(page1.Visible, "first pushed page should be visible")
    test.expect(root.Visible == false, "root should hide after first push")

    -- Push second page (same template, different instance)
    navigation:send("NavigationHost.Push", { Path = "Adventure/Pages/Game.xml" })
    pump_messages(screen)
    -- Both pushed pages share the template name; find the visible one
    local pages = {}
    for child in navigation.children do
        if child.Name == "GamePage" then pages[#pages + 1] = child end
    end
    test.expect(#pages == 2, "should have two GamePage instances after second push")
    test.expect(pages[2].Visible, "second pushed page should be visible")
    test.expect(pages[1].Visible == false, "first pushed page should hide after second push")
    test.expect(navigation.CanGoBack, "CanGoBack should be true with stack depth > 1")

    -- Pop back to first push
    navigation:send("NavigationHost.Pop")
    pump_messages(screen)
    test.expect(pages[1].Visible, "first pushed page should be visible after pop")
    test.expect(pages[2].Visible == false, "second pushed page should hide after pop")

    -- Pop back to root
    navigation:send("NavigationHost.Pop")
    pump_messages(screen)
    test.expect(root.Visible, "root should be visible after second pop")
    test.expect(navigation.CanGoBack == false, "CanGoBack should be false back at root")

    screen:clear()
    print("PASS: test_navigationhost_push_pop_stack")
end

-- ---------------------------------------------------------------------------
-- NavigationHost: Pop on root page is a no-op
-- ---------------------------------------------------------------------------
local function test_navigationhost_pop_root_noop()
    local screen = filesystem.loadObjectFromXmlString([[
      <Screen Name="Test" Width="400" Height="800" ResizeMode="NoResize">
        <NavigationHost Name="Navigation" Width="400" Height="800">
          <Page Name="RootPage" Width="400" Height="800"/>
        </NavigationHost>
      </Screen>
    ]])
    local navigation = screen:findChild("Navigation", true)
    local root = screen:findChild("RootPage", true)

    navigation:send("Node.ViewDidLoad")
    pump_messages(screen)

    test.expect(root.Visible, "root should be visible initially")

    -- Pop on root should do nothing
    navigation:send("NavigationHost.Pop")
    pump_messages(screen)

    test.expect(root.Visible, "root should still be visible after pop on root")
    test.expect(navigation.CanGoBack == false, "CanGoBack should remain false")

    screen:clear()
    print("PASS: test_navigationhost_pop_root_noop")
end

-- ---------------------------------------------------------------------------
-- NavigationHost: Push with DataContext passes it to the pushed page
-- ---------------------------------------------------------------------------
local function test_navigationhost_push_with_context()
    local screen = filesystem.loadObjectFromXmlString([[
      <Screen Name="Test" Width="400" Height="800" ResizeMode="NoResize">
        <NavigationHost Name="Navigation" Width="400" Height="800">
          <Page Name="RootPage" Width="400" Height="800">
            <TextBlock Name="Card" Text="Game"
              DataContext="{DataObject Name=zork1}"
              LeftButtonUp="{NavigationHost.Push 'Adventure/Pages/Game.xml'}"/>
          </Page>
        </NavigationHost>
      </Screen>
    ]])
    local navigation = screen:findChild("Navigation", true)
    local card = screen:findChild("Card", true)

    navigation:send("Node.ViewDidLoad")
    card:send("Node.LeftButtonUp")
    pump_messages(screen)

    local detail = screen:findChild("GamePage", true)
    test.expect(detail ~= nil, "pushed page should exist")
    test.expect(detail.DataContext ~= nil, "pushed page should receive DataContext")
    test.expect(detail.DataContext.Name == "zork1", "DataContext should be the card's data object")

    screen:clear()
    print("PASS: test_navigationhost_push_with_context")
end

-- ---------------------------------------------------------------------------
-- NavigationHost: Push without DataContext leaves pushed page DataContext nil
-- ---------------------------------------------------------------------------
local function test_navigationhost_push_without_context()
    local screen = filesystem.loadObjectFromXmlString([[
      <Screen Name="Test" Width="400" Height="800" ResizeMode="NoResize">
        <NavigationHost Name="Navigation" Width="400" Height="800">
          <Page Name="RootPage" Width="400" Height="800"/>
        </NavigationHost>
      </Screen>
    ]])
    local navigation = screen:findChild("Navigation", true)

    navigation:send("Node.ViewDidLoad")
    pump_messages(screen)

    navigation:send("NavigationHost.Push", { Path = "Adventure/Pages/Game.xml" })
    pump_messages(screen)

    local detail = screen:findChild("GamePage", true)
    test.expect(detail ~= nil, "pushed page should exist")
    test.expect(detail.DataContext == nil, "pushed page DataContext should be nil without explicit context")

    screen:clear()
    print("PASS: test_navigationhost_push_without_context")
end

-- ---------------------------------------------------------------------------
-- NavigationHost: popped page is removed from the hierarchy
-- ---------------------------------------------------------------------------
local function test_navigationhost_pop_removes_page()
    local screen = filesystem.loadObjectFromXmlString([[
      <Screen Name="Test" Width="400" Height="800" ResizeMode="NoResize">
        <NavigationHost Name="Navigation" Width="400" Height="800">
          <Page Name="RootPage" Width="400" Height="800"/>
        </NavigationHost>
      </Screen>
    ]])
    local navigation = screen:findChild("Navigation", true)

    navigation:send("Node.ViewDidLoad")
    pump_messages(screen)

    navigation:send("NavigationHost.Push", { Path = "Adventure/Pages/Game.xml" })
    pump_messages(screen)
    test.expect(screen:findChild("GamePage", true) ~= nil, "GamePage should exist after push")

    navigation:send("NavigationHost.Pop")
    pump_messages(screen)
    test.expect(screen:findChild("GamePage", true) == nil, "GamePage should be removed after pop")

    screen:clear()
    print("PASS: test_navigationhost_pop_removes_page")
end

-- Run all tests
test_navigationhost_push_pop()
test_navigationhost_cangoback()
test_navigationhost_push_pop_stack()
test_navigationhost_pop_root_noop()
test_navigationhost_push_with_context()
test_navigationhost_push_without_context()
test_navigationhost_pop_removes_page()

print("\nAll NavigationHost tests passed.")
