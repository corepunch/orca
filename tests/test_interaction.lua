local test = require "orca.test"
-- Tests for user interaction: Button click events, keyboard input into text
-- fields, Checkbox checked state, and Form.populateInputs().
-- Part of the full `test` target.

local orca = require "orca"
local core = require "orca.core"
local renderer = require "orca.renderer"
local ui = require "orca.UIKit"
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

-- ---------------------------------------------------------------------------
-- Button: LeftButtonDown event handler fires and button receives focus
-- ---------------------------------------------------------------------------
local function test_button_interaction()
	local clicked = false
	local button = screen + ui.Button { Width = 100, Height = 100, LeftButtonDown = function () clicked = true end }
	screen:UpdateLayout(screen.Width, screen.Height)
	-- Simulate a left mouse down event on the button
	orca.system.dispatchMessage {
		target = screen,
		message = "LeftButtonDown",
		x = 50,
		y = 25,
	}
	-- Verify that the button's LeftButtonDown handler was triggered and that the button is focused
	test.expect(clicked, "Button LeftButtonDown handler should be triggered by the event")
	test.expect(button:isFocused(), "Button should be focused after receiving a mouse down event")
	button:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Input: typed characters via KeyDown events are accumulated into Text
-- ---------------------------------------------------------------------------
local function test_input_interaction()
	local config = { text = "Hello World!" }
	local input = screen + ui.Input { Width = 100, Height = 100 }
	input:setFocus()
	screen:UpdateLayout(screen.Width, screen.Height)
	-- Simulate a key down event on the input
	for i = 1, #config.text do
		orca.system.dispatchMessage {
			target = screen,
			message = "KeyDown",
			key = string.byte(config.text:sub(i, i)),
			text = config.text:sub(i, i),
		}
	end
	-- Verify that the input's Text property has been updated to match the simulated key presses
	test.expect_eq(input.Text, config.text, "Input text should match the simulated key presses")
	input:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Input (Checkbox): Checked property can be set and cleared
-- ---------------------------------------------------------------------------
local function test_input_checkbox()
	local checkbox = screen + ui.Input {
		Type = "Checkbox",
		Width = 24,
		Height = 24,
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	-- The input type should be "Checkbox"
	test.expect_eq(checkbox.Type, "Checkbox", "Input type should be 'Checkbox'")

	-- Set and verify checked state
	checkbox.Checked = true
	test.expect(checkbox.Checked, "Checkbox should be checked after setting Checked = true")

	-- Unset and verify checked state
	checkbox.Checked = false
	test.expect(not checkbox.Checked, "Checkbox should be unchecked after setting Checked = false")

	checkbox:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Form: populateInputs collects all named child inputs into a table
-- ---------------------------------------------------------------------------
local function test_form_populate_inputs()
	local config = { username = "alice", password = "test_password_123" }
	local form = screen + ui.Form {
		Direction = "Vertical",
	}
	local username_input = form + ui.Input {
		Name = "username",
		Width = 200,
		Height = 30,
	}
	local password_input = form + ui.Input {
		Name = "password",
		Width = 200,
		Height = 30,
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	-- Type text into the username input
	username_input:setFocus()
	for i = 1, #config.username do
		orca.system.dispatchMessage {
			target = screen,
			message = "KeyDown",
			key = string.byte(config.username:sub(i, i)),
			text = config.username:sub(i, i),
		}
	end

	-- Type text into the password input
	password_input:setFocus()
	for i = 1, #config.password do
		orca.system.dispatchMessage {
			target = screen,
			message = "KeyDown",
			key = string.byte(config.password:sub(i, i)),
			text = config.password:sub(i, i),
		}
	end

	-- Verify individual input texts
	test.expect_eq(username_input.Text, config.username, "Username input text should match typed keys")
	test.expect_eq(password_input.Text, config.password, "Password input text should match typed keys")

	-- Verify that populateInputs collects all named inputs as a table
	local data = form:populateInputs()
	test.expect_eq(type(data), "table", "populateInputs should return a table")
	test.expect_eq(data.username, config.username, "populateInputs should collect the username input value")
	test.expect_eq(data.password, config.password, "populateInputs should collect the password input value")

	form:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- RadioGroup: click selects the correct option, fires SelectionChanged
-- ---------------------------------------------------------------------------
local function test_radio_group_interaction()
	local AX_KEY_SPACE = 32
	local AX_KEY_LEFTARROW = 130
	local selected_value = nil
	local old_value_seen = nil

	local group = screen + ui.RadioGroup {
		Width = 300,
		Height = 50,
		Direction = "Horizontal",
		SelectionChanged = function (self, e)
			selected_value = self.SelectedValue
			if e then old_value_seen = e.OldValue end
		end,
	}
	local btn_c = group + ui.RadioButton { Value = "celsius",    Width = 100, Height = 30 }
	local btn_f = group + ui.RadioButton { Value = "fahrenheit", Width = 100, Height = 30 }

	screen:UpdateLayout(screen.Width, screen.Height)

	-- Click "fahrenheit" button using its computed screen position
	local fx = btn_f.ActualX + 5
	local fy = btn_f.ActualY + 5
	orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = fx, y = fy }
	orca.system.dispatchMessage { target = screen, message = "LeftButtonUp",   x = fx, y = fy }

	test.expect_eq(group.SelectedValue, "fahrenheit", "RadioGroup.SelectedValue should be 'fahrenheit' after click")
	test.expect(btn_f.IsChecked,     "RadioButton fahrenheit should be IsChecked after click")
	test.expect(btn_f.selected,      "RadioButton fahrenheit should expose selected state for active styles")
	test.expect(not btn_c.IsChecked, "RadioButton celsius should NOT be IsChecked after sibling click")
	test.expect(not btn_c.selected,  "RadioButton celsius selected state should be cleared after sibling click")
	test.expect_eq(selected_value, "fahrenheit", "SelectionChanged handler should receive the new SelectedValue")
	test.expect(old_value_seen == nil, "OldValue should be nil when nothing was previously selected")

	-- Click "celsius" — verify mutual exclusion and that OldValue is passed correctly
	local cx = btn_c.ActualX + 5
	local cy = btn_c.ActualY + 5
	orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = cx, y = cy }
	orca.system.dispatchMessage { target = screen, message = "LeftButtonUp",   x = cx, y = cy }

	test.expect_eq(group.SelectedValue, "celsius",    "RadioGroup.SelectedValue should switch to 'celsius'")
	test.expect(btn_c.IsChecked,     "RadioButton celsius should be IsChecked after click")
	test.expect(btn_c.selected,      "RadioButton celsius should expose selected state for active styles")
	test.expect(not btn_f.IsChecked, "RadioButton fahrenheit should NOT be IsChecked after sibling click")
	test.expect(not btn_f.selected,  "RadioButton fahrenheit selected state should be cleared after sibling click")
	test.expect_eq(selected_value, "celsius",     "SelectionChanged should report new SelectedValue")
	test.expect_eq(old_value_seen, "fahrenheit",  "SelectionChanged should report OldValue = 'fahrenheit'")

	-- Clicking the already-selected button should NOT fire SelectionChanged again
	selected_value = nil
	orca.system.dispatchMessage { target = screen, message = "LeftButtonDown", x = cx, y = cy }
	orca.system.dispatchMessage { target = screen, message = "LeftButtonUp",   x = cx, y = cy }
	test.expect(selected_value == nil, "SelectionChanged should NOT fire when clicking already-selected button")

	-- Space selects the focused radio button.
	selected_value = nil
	old_value_seen = nil
	btn_f:setFocus()
	orca.system.dispatchMessage { target = screen, message = "KeyDown", key = AX_KEY_SPACE }
	test.expect_eq(group.SelectedValue, "fahrenheit", "Space should select the focused radio button")
	test.expect(btn_f.IsChecked, "Focused RadioButton should be IsChecked after Space")
	test.expect_eq(selected_value, "fahrenheit", "Space selection should fire SelectionChanged")
	test.expect_eq(old_value_seen, "celsius", "Space selection should report OldValue")

	-- Arrow keys move selection and focus within the group.
	selected_value = nil
	old_value_seen = nil
	orca.system.dispatchMessage { target = screen, message = "KeyDown", key = AX_KEY_LEFTARROW }
	test.expect_eq(group.SelectedValue, "celsius", "Left arrow should select the previous radio button")
	test.expect(btn_c.IsChecked, "Previous RadioButton should be IsChecked after Left arrow")
	test.expect(btn_c:isFocused(), "Previous RadioButton should receive focus after Left arrow")
	test.expect_eq(selected_value, "celsius", "Arrow selection should fire SelectionChanged")
	test.expect_eq(old_value_seen, "fahrenheit", "Arrow selection should report OldValue")

	group:removeFromParent()
end

-- RadioGroup: SelectedValue set at construction auto-checks the matching child
-- ---------------------------------------------------------------------------
local function test_radio_group_selected_value_sync()
	local group = screen + ui.RadioGroup {
		Width = 300, Height = 50,
		SelectedValue = "fahrenheit",
	}
	local btn_c = group + ui.RadioButton { Value = "celsius",    Width = 100, Height = 30 }
	local btn_f = group + ui.RadioButton { Value = "fahrenheit", Width = 100, Height = 30 }

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect(btn_f.IsChecked,     "RadioButton matching SelectedValue should be checked after Start")
	test.expect(not btn_c.IsChecked, "RadioButton not matching SelectedValue should be unchecked")

	group:removeFromParent()
end

-- RadioGroup: changing SelectedValue at runtime syncs child buttons
-- ---------------------------------------------------------------------------
local function test_radio_group_selected_value_runtime_change()
	local group = screen + ui.RadioGroup { Width = 300, Height = 50 }
	local btn_c = group + ui.RadioButton { Value = "celsius",    Width = 100, Height = 30 }
	local btn_f = group + ui.RadioButton { Value = "fahrenheit", Width = 100, Height = 30 }

	screen:UpdateLayout(screen.Width, screen.Height)

	group.SelectedValue = "celsius"
	test.expect(btn_c.IsChecked,     "Setting SelectedValue = 'celsius' should check celsius button")
	test.expect(not btn_f.IsChecked, "Setting SelectedValue = 'celsius' should uncheck fahrenheit button")

	group.SelectedValue = "fahrenheit"
	test.expect(btn_f.IsChecked,     "Setting SelectedValue = 'fahrenheit' should check fahrenheit button")
	test.expect(not btn_c.IsChecked, "Setting SelectedValue = 'fahrenheit' should uncheck celsius button")

	group:removeFromParent()
end

-- RadioButton: setting IsChecked = true programmatically syncs the group
-- ---------------------------------------------------------------------------
local function test_radio_button_programmatic_check()
	local selected_value = nil
	local group = screen + ui.RadioGroup {
		Width = 300, Height = 50,
		SelectionChanged = function(self) selected_value = self.SelectedValue end,
	}
	local btn_c = group + ui.RadioButton { Value = "celsius",    Width = 100, Height = 30 }
	local btn_f = group + ui.RadioButton { Value = "fahrenheit", Width = 100, Height = 30 }

	screen:UpdateLayout(screen.Width, screen.Height)

	btn_f.IsChecked = true
	test.expect(btn_f.IsChecked,                  "Programmatic IsChecked = true should check button")
	test.expect(not btn_c.IsChecked,              "Programmatic check should uncheck sibling")
	test.expect_eq(group.SelectedValue, "fahrenheit", "Programmatic check should update group SelectedValue")
	test.expect_eq(selected_value, "fahrenheit",  "Programmatic check should fire SelectionChanged")

	group:removeFromParent()
end

orca.async = function (fn, ...) fn(...) end

test_button_interaction()
test_input_interaction()
test_input_checkbox()
test_form_populate_inputs()
test_radio_group_interaction()
test_radio_group_selected_value_sync()
test_radio_group_selected_value_runtime_change()
test_radio_button_programmatic_check()

print("All interaction tests passed.")
