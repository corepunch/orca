-- Application menu bar; each title opens a dropdown Menu.
local config = require "Editor.Scripts.config"
local Terminal = require "Editor.Scripts.Terminal"
local Menu = require "Editor.Scripts.Menu"
local Popups = require "Editor.Scripts.Popups"

local MenuBar = Terminal:extend {}

function MenuBar:init(menus, status)
	self.menus = menus     -- { { title = "File", items = function() ... end }, ... }
	self.status = status   -- function() -> right-aligned text
	Terminal.init(self, { Height = config.char_height })
end

function MenuBar:draw()
	local parts, col = { '![1;15m ' }, 1
	self.spans = {}
	for i, m in ipairs(self.menus) do
		local label = ' ' .. m.title .. ' '
		local color = self.openIndex == i and '![15;1m' or '![1;15m'
		parts[#parts + 1] = color .. label .. '![1;15m'
		self.spans[i] = { from = col, to = col + #label - 1 }
		col = col + #label
	end
	local status = self.status and self.status() or ''
	local cols = self:columns()
	local pad = math.max(1, cols - col - #status - 1)
	self:println(nil, table.concat(parts), string.rep(' ', pad), '![8;15m', Terminal.escape(status), ' ')
end

function MenuBar:indexAt(x)
	local col = math.floor(x / config.char_width)
	for i, s in ipairs(self.spans or {}) do
		if col >= s.from and col <= s.to then return i end
	end
end

function MenuBar:open(index)
	Popups.closeAll()
	local m = self.menus[index]
	if not m then return end
	self.openIndex = index
	self:invalidate()
	local menu = Menu(m.items(), {
		onNavigate = function(dir)
			self:open((index - 1 + dir) % #self.menus + 1)
		end,
	})
	menu.onClose = function()
		if self.openIndex == index then
			self.openIndex = nil
			self:invalidate()
		end
	end
	Popups.push(menu, self.spans[index].from * config.char_width, config.char_height)
end

function MenuBar:onMouseDown(_, _, _, args)
	local i = self:indexAt(args.x)
	if i then self:open(i) end
end

-- Alt+letter opens the matching menu.
function MenuBar:accelerator(letter)
	for i, m in ipairs(self.menus) do
		if m.title:sub(1, 1):lower() == letter then self:open(i) return true end
	end
	return false
end

return MenuBar
