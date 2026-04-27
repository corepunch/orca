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
-- Run all tests
-- ---------------------------------------------------------------------------
orca.async = function (callback) callback() end

test_button_interaction()
test_input_interaction()
test_input_checkbox()
test_form_populate_inputs()

print("All interaction tests passed.")
