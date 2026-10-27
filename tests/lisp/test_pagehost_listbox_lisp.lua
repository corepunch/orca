local test = require "orca.test"
-- PageHost navigation declared in Lisp; tests/test_pagehost_listbox.lua covers
-- the format-independent PageHost and ListBox behavior.
local orca = require "orca"
require "orca.UIKit"
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
-- PageHost: NavigateToPage passes a DataContext to the destination
-- ---------------------------------------------------------------------------
local function test_pagehost_navigation_context()
    local screen = filesystem.loadObjectFromLispString([[
(Screen Name="Test" Width=400 Height=800 ResizeMode="NoResize"
  (PageHost Name="Host" Width=400 Height=800 ActivePage="alpha"
    (Page Name="alpha" Path="/alpha" Width=400 Height=800
      (TextBlock Name="Card" Width=400 Height=100 Text="Game" DataContext=(DataObject Name="zork1") LeftButtonUp=(PageHost.NavigateToPage URL="/beta")))
    (Page Name="beta" Path="/beta" Width=400 Height=800)))]])
    local card = screen:findChild("Card", true)
    local page_b = screen:findChild("beta", true)

    test.expect(card.DataContext ~= nil, "card should have a data context")
    screen:findChild("Host", true):send("Node.ViewDidLoad")
    card:send("Node.LeftButtonUp")
    pump_messages(screen)

    test.expect(page_b.Visible, "beta should be visible")
    test.expect(page_b.DataContext ~= nil, "destination should receive the navigation context")

    screen:clear()
    print("PASS: test_pagehost_navigation_context")
end

-- Run all tests
test_pagehost_navigation_context()

print("\nAll Lisp PageHost tests passed.")
