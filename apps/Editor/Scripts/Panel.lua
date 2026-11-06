-- Docked panel: a one-line title bar with tool buttons above a body view.
local ui = require "orca.UIKit"
local config = require "Editor.Scripts.config"
local Terminal = require "Editor.Scripts.Terminal"

local Header = Terminal:extend {}

function Header:init(panel)
	self.panel = panel
	Terminal.init(self, { Height = config.char_height })
end

function Header:draw()
	local panel = self.panel
	local tools = panel.tools and panel:tools() or {}
	local title = ' ' .. panel.title:upper() .. (panel.subtitle and ('  ' .. panel.subtitle) or '')
	local cols = self:columns()
	local right = {}
	for _, t in ipairs(tools) do right[#right + 1] = '[' .. t.label .. ']' end
	local tail = table.concat(right, ' ') .. ' '
	local color = panel.active and config.colors.header or '![9;1m'
	self:println(nil, color, Terminal.fit(Terminal.escape(title), math.max(0, cols - #tail)), '![11;1m', tail)
	-- Tool buttons are matched by column on click.
	self.toolSpans = {}
	local col = math.max(0, cols - #tail)
	for i, t in ipairs(tools) do
		local w = #right[i]
		self.toolSpans[#self.toolSpans + 1] = { from = col, to = col + w - 1, tool = t }
		col = col + w + 1
	end
end

function Header:onClick(_, _, _, args)
	local col = math.floor((args.x - (self:origin())) / config.char_width)
	for _, span in ipairs(self.toolSpans or {}) do
		if col >= span.from and col <= span.to then span.tool.action() return end
	end
	self.panel:focus()
end

-- Header lines carry no items, so measure from the view's screen origin.
function Header:origin()
	local x, o = 0, self.view
	while o do x = x + (o.ActualX or 0); o = o.parent end
	return x
end

local Panel = {}

-- Wrap a body Terminal: body.title, body.tools() and body.subtitle feed the header.
function Panel.wrap(body, bodyView)
	body.header = Header(body)
	local grid = ui.Grid { Rows = "16px 1fr" }
	grid:addChild(body.header.view)
	grid:addChild(bodyView or body.view)
	body.container = grid
	return grid
end

return Panel
