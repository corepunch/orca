local controls = import "Scenes/Controls"

return ui.View {
	height = 25,
	horizontalPadding = cg.Vector2.new(5, 5),
	ui.Label {
		name = "title",
		horizontalAlignment = "left",
		text = "Hello",
		color = "#ffffff",
	},
  stateManger = {
    selected = function (self, value)
      self.background = renderer.Brush.color(value and "#373839" or "#0072F3")
    end,
  },
	roomName = ref "title.text",
}