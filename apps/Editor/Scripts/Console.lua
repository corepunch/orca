-- Log of editor actions and server errors, newest first.
local Terminal = require "Editor.Scripts.Terminal"
local Workspace = require "Editor.Scripts.Workspace"

local Console = Terminal:extend { title = "Console" }

local STYLE = {
	info = { '![9;0m', '\xf9' },
	ok = { '![10;0m', '\xfb' },
	warning = { '![14;0m', '!' },
	error = { '![12;0m', 'x' },
}

function Console:init()
	self.entries = {}
	self.counts = { info = 0, ok = 0, warning = 0, error = 0 }
	Terminal.init(self)
	Workspace.on('log', function(level, text) self:add(level, text) end)
end

function Console:add(level, text)
	table.insert(self.entries, 1, { level = level, text = text, time = os.date('%H:%M:%S') })
	if #self.entries > 500 then table.remove(self.entries) end
	self.counts[level] = (self.counts[level] or 0) + 1
	self:invalidate()
	if self.header then self.header:invalidate() end
end

function Console:tools()
	return {
		{ label = 'Clear', action = function()
			self.entries = {}
			self.counts = { info = 0, ok = 0, warning = 0, error = 0 }
			self:invalidate()
			self.header:invalidate()
		end },
	}
end

function Console:draw()
	self.subtitle = string.format('%d errors  %d warnings', self.counts.error, self.counts.warning)
	if #self.entries == 0 then
		self:println(nil, '![8;0m  No messages')
	end
	for _, e in ipairs(self.entries) do
		local s = STYLE[e.level] or STYLE.info
		self:println(e, '![8;0m ', e.time, ' ', s[1], s[2], ' ', e.level == 'error' and '![12;0m' or '![7;0m', Terminal.escape(e.text))
	end
end

function Console:onMouseDown(item, index)
	if item then self:select(index) end
end

return Console
