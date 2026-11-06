-- Read-only message dialog (shortcut reference, about box).
local config = require "Editor.Scripts.config"
local Terminal = require "Editor.Scripts.Terminal"
local Popups = require "Editor.Scripts.Popups"

local Dialog = Terminal:extend {}

function Dialog:init(title, lines)
	self.title, self.lines = title, lines
	local width = #title + 8
	for _, l in ipairs(lines) do
		width = math.max(width, l[2] and (2 + 18 + #l[2] + 2) or (#(l[1] or '') + 4))
	end
	self.cols = width
	Terminal.init(self, {
		Width = width * config.char_width,
		Height = (#lines + 3) * config.char_height,
		DropShadow = true,
	})
	self:invalidate()
end

function Dialog:draw()
	self:println(nil, '![15;1m ', Terminal.fit(self.title, self.cols - 12), '![11;1m[Esc] Close ')
	self:println(nil, '')
	for _, l in ipairs(self.lines) do
		if l[2] then
			self:println(nil, '  ![14;0m', Terminal.fit(l[1], 18), '![7;0m', l[2])
		else
			self:println(nil, '  ', l[1] and ('![7;0m' .. l[1]) or '')
		end
	end
end

function Dialog:onKey(chord)
	if chord == 'escape' or chord == 'enter' then Popups.close(self) return true end
	return false
end

function Dialog:onClick() Popups.close(self) end

function Dialog.show(title, lines)
	local d = Dialog(title, lines)
	local sw, sh = Popups.size()
	return Popups.push(d, (sw - d.view.Width) / 2, math.max(16, (sh - d.view.Height) / 3), true)
end

return Dialog
