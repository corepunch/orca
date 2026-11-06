-- Syntax-colored code surface. Character cells are confined to authored code;
-- the surrounding editor and its data-driven controls remain native UIKit.
local Class = require "Editor.Scripts.Class"
local Native = require "Editor.Scripts.Native"
local Syntax = require "Editor.Scripts.Syntax"
local CodeInput = Class:extend {}
local CELL_WIDTH, CELL_HEIGHT, GUTTER = 8, 16, 5

function CodeInput:init(view)
	self.view = view or Native.load('Editor/Views/CodeInput')
	self.Text, self.Cursor, self.top, self.left = '', 0, 1, 0
	self.functions = {}
	self.view.KeyDown = function(_, args)
		return self.KeyDown and self.KeyDown(self, args) or false
	end
	self.view.TextInput = function(_, args)
		return self.TextInput and self.TextInput(self, args) or false
	end
	self.view.LeftButtonDown = function(_, args)
		self:setFocus()
		self:pick(args.x, args.y)
		return true
	end
	self.view.ScrollWheel = function(_, args)
		local delta = args.deltaY or args.scrollY or 0
		if delta ~= 0 then
			self.top = math.max(1, self.top + (delta > 0 and -3 or 3))
			self:render(true)
		end
		return true
	end
end

function CodeInput:setFocus() self.view:setFocus() end
function CodeInput:post(message, args) return self.view:post(message, args) end
function CodeInput:send(message, args) return self.view:send(message, args) end

function CodeInput:lines()
	local lines = {}
	for line in (self.Text .. '\n'):gmatch('([^\n]*)\n') do lines[#lines + 1] = line end
	return lines
end

function CodeInput:position()
	local cursor, row, start = math.max(0, math.min(#self.Text, self.Cursor)), 1, 1
	for pos in self.Text:sub(1, cursor):gmatch('()\n') do row, start = row + 1, pos + 1 end
	return row, cursor - start + 1
end

function CodeInput:render(keepScroll)
	local view, lines = self.view, self:lines()
	local rows = math.max(1, math.floor((view.ActualHeight or 0) / CELL_HEIGHT))
	local columns = math.max(1, math.min(250, math.floor((view.ActualWidth or 0) / CELL_WIDTH) - GUTTER))
	local caretRow, caretColumn = self:position()
	if not keepScroll then
		if caretRow < self.top then self.top = caretRow end
		if caretRow >= self.top + rows then self.top = caretRow - rows + 1 end
		if caretColumn < self.left then self.left = caretColumn end
		if caretColumn >= self.left + columns then self.left = caretColumn - columns + 1 end
	end
	self.top = math.max(1, math.min(self.top, #lines))
	local bad = Syntax.unbalanced(self.Text)
	local a, b = Syntax.matchBracket(self.Text, self.Cursor + 1)
	local offset = 0
	for i = 1, self.top - 1 do offset = offset + #lines[i] + 1 end
	view:send('ConsoleView.Erase')
	self.rendered = {}
	for row = self.top, self.top + rows - 1 do
		local line = lines[row]
		local text = '![8;0m    \xb3'
		if line then
			local errors, matched = {}, {}
			for pos in pairs(bad) do
				local column = pos - offset - self.left
				if column > 0 and column <= columns then errors[column] = true end
			end
			if b then
				for _, pos in ipairs {a, b} do
					local column = pos - offset - self.left
					if column > 0 and column <= columns then matched[column] = true end
				end
			end
			local visible = line:sub(self.left + 1, self.left + columns)
			local number = row == caretRow and '![15;0m' or '![8;0m'
			text = number .. string.format('%3d', row) .. ' ![8;0m\xb3'
				.. Syntax.highlight(visible, {functions = self.functions, highlight = matched, bad = errors})
			offset = offset + #line + 1
		end
		self.rendered[#self.rendered + 1] = text
		view:send('ConsoleView.Println', {Index = row, Text = text .. '![7;0m'})
	end
	local visibleCaret = caretRow >= self.top and caretRow < self.top + rows
	view.Cursor = visibleCaret and ((caretRow - self.top) * view.BufferWidth + GUTTER + caretColumn - self.left) or -1
	view:send('ConsoleView.Invalidate')
	self.lastWidth, self.lastHeight = view.ActualWidth, view.ActualHeight
end

function CodeInput:checkResize()
	if self.lastWidth ~= self.view.ActualWidth or self.lastHeight ~= self.view.ActualHeight then self:render() end
end

function CodeInput:pick(x, y)
	local originX, originY = Native.origin(self.view)
	local lines = self:lines()
	local row = math.max(1, math.min(#lines, self.top + math.floor((y - originY) / CELL_HEIGHT)))
	local column = math.max(0, math.min(#lines[row], self.left + math.floor((x - originX) / CELL_WIDTH) - GUTTER))
	local cursor = column
	for i = 1, row - 1 do cursor = cursor + #lines[i] + 1 end
	self.Cursor = cursor
	if self.CaretChanged then self:CaretChanged() else self:render() end
end

return CodeInput
