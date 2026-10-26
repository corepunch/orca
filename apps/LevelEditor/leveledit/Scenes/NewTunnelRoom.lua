local controls = import "Scenes/Controls"

return controls.PopUpWindow {
	width = 380,
	height = 180,
	title = "New Tunnel Room",
	ui.HStackView {
		spacing = 5,
		ui.Label {
			text = "Select new room depth and height:",
			color = cg.Color.new "#ffffff",
		},
		controls.TextField {
			name = "textField",
			text = "3, 8",
		},
	},
	getTokens = function (self)
		local textField = self:findChild("textField")
		return textField.text:match("([^,]+),([^,]+)")
	end,
	getRoomCeiling = function (self)
		local _, ceiling = self:getTokens()
		return tonumber(ceiling)
	end,
	getRoomDepth = function (self)
		local depth, _ = self:getTokens()
		return tonumber(depth)
	end,
}