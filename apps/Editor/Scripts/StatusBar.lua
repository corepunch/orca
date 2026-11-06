-- One-line status: selection, undo depth, document state and hints.
local config = require "Editor.Scripts.config"
local Terminal = require "Editor.Scripts.Terminal"
local Workspace = require "Editor.Scripts.Workspace"

local StatusBar = Terminal:extend {}

function StatusBar:init()
	Terminal.init(self, { Height = config.char_height })
	self.hint = "F1 Shortcuts  Ctrl+S Save  Ctrl+Z Undo  Ctrl+B Binding  F5 Refresh"
end

function StatusBar:draw()
	local doc = Workspace.document and Workspace.leaf(Workspace.document) or 'No document'
	local state = Workspace.dirty and '![14;1m \x07 Modified ' or '![10;1m \xfb Saved '
	local left = string.format(' %s  ![9;1m%s', doc, Terminal.escape(Workspace.selection or ''))
	local undo = string.format('Undo %d  Redo %d', #Workspace.undoStack, #Workspace.redoStack)
	local cols = self:columns()
	local used = #doc + #(Workspace.selection or '') + 3
	local rightText = undo .. '  ' .. self.hint .. ' '
	local pad = math.max(1, cols - used - #rightText - 11)
	self:println(nil, '![15;1m', left, string.rep(' ', pad), '![9;1m', rightText, state)
end

return StatusBar
