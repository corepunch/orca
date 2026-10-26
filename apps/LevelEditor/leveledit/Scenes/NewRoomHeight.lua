local controls = import "Scenes/Controls"

return controls.PopUpWindow {
	width = 380,
	height = 180,
	title = "New Room Height",
	ui.HStackView {
		spacing = 5,
		ui.Label {
			text = "Select new room height:",
			color = cg.Color.new "#ffffff",
		},
		controls.TextField {
			name = "textField",
			text = "12",
		},
	},
	getRoomCeiling = function (self)
		local textField = self:findChild("textField")
		return tonumber(textField.text)
	end,
}