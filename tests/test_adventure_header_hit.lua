local test = require "orca.test"
local orca = require "orca"
local ui = require "orca.UIKit"

orca.async = function(fn, ...) fn(...) end

local function pump_coroutines(root)
	while true do
		local msg = orca.system.peekMessage()
		if not msg then return end
		if msg.message == 0x3cc3febc or msg.message == 0xf13ac065 then
			orca.system.dispatchMessage(root, msg)
		end
	end
end

dofile(SHAREDIR .. "/plugins/tailwind.lua")

local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }
local clicked = false
local receiver = nil
local header = nil
local back = nil
local console_view = nil

local function AdventureHeader(title, on_back)
	return ui.Grid({
		class = "bg-header-background items-center overflow-x-hidden",
		Columns = "48px 1fr",
		LeftButtonUp = function()
			receiver = "header"
			return true
		end,
	}, function()
		back = ui.Node2D {
			class = "align-middle-center text-accent-foreground",
			BackgroundColor = "#ff00ffff",
			Width = 48,
			Height = 48,
			LeftButtonUp = function()
				clicked = true
				receiver = "back"
				if on_back then on_back() end
				return true
			end,
		}
	end)
end

local header_slot = AdventureHeader("Adventure")
header = header_slot

local chrome = screen + ui.Grid({ Rows = "32px 52px 1fr 72px 24px" }, function(self)
	ui.Node2D {
		class = "bg-header-background",
		LeftButtonUp = function()
			receiver = "top"
			return true
		end,
	}
	self:addChild(header_slot)
	ui.Node2D({
		class = "bg-background h-full",
		LeftButtonUp = function()
			receiver = "content"
			return true
		end,
	}, function()
		console_view = ui.StackView({
			class = "flex-col overflow-y-scroll h-full py-4",
			LeftButtonUp = function()
				receiver = "console"
				return true
			end,
		}, function()
			for i = 1, 80 do
				ui.TextBlock {
					class = "p-2 text-base text-foreground",
					Text = "Transcript line " .. tostring(i),
				}
			end
		end)
	end)
	ui.StackView({
		class = "bg-footer-background px-4 py-2 items-center",
		LeftButtonUp = function()
			receiver = "commandbar"
			return true
		end,
	}, function()
		ui.Input {
			class = "bg-surface w-full h-12 px-4 py-2 rounded text-base text-foreground text-nowrap text-clip overflow-x-hidden",
			PlaceholderText = "Enter command...",
			Name = "command",
		}
	end)
	ui.Node2D {
		class = "bg-footer-background",
		LeftButtonUp = function()
			receiver = "bottom"
			return true
		end,
	}
end)

pump_coroutines(screen)
test.expect(header ~= nil, "Adventure header Grid should be built by constructor context")
test.expect(back ~= nil, "Adventure header back Node2D should be built by constructor context")
test.expect(console_view ~= nil, "Adventure transcript StackView should be built by constructor context")

screen:UpdateLayout(screen.Width, screen.Height)
console_view:SetScrollTop(console_view.ScrollHeight)
screen:UpdateLayout(screen.Width, screen.Height)

test.expect_near(header.ActualY, 32, 0.5, "Adventure header row Y")
test.expect_near(header.ActualHeight, 52, 0.5, "Adventure header row height")
test.expect_near(back.ActualX, 0, 0.5, "Back button visual X within header")
test.expect_near(back.ActualY, 2, 0.5, "Back button visual Y within header")
test.expect_near(back.ActualWidth, 48, 0.5, "Back button visual width")
test.expect_near(back.ActualHeight, 48, 0.5, "Back button visual height")

local hit_x = header.ActualX + back.ActualX + back.ActualWidth / 2
local hit_y = header.ActualY + back.ActualY + back.ActualHeight / 2
local cases = {
	{ hit_x, hit_y, true, "center" },
	{ header.ActualX + back.ActualX - 1, hit_y, false, "left edge outside" },
	{ header.ActualX + back.ActualX + back.ActualWidth, hit_y, false, "right edge outside" },
	{ hit_x, header.ActualY + back.ActualY - 1, false, "top edge outside" },
	{ hit_x, header.ActualY + back.ActualY + back.ActualHeight, false, "bottom edge outside" },
}

for _, point in ipairs(cases) do
	clicked = false
	receiver = nil
	orca.system.dispatchMessage {
		target = screen,
		message = "LeftButtonUp",
		x = point[1],
		y = point[2],
	}
	test.expect_eq(clicked, point[3],
		"Adventure header back Node2D hit area should match actual frame at " .. point[4] .. " (receiver: " .. tostring(receiver) .. ")")
end

print("Adventure header hit test passed.")
