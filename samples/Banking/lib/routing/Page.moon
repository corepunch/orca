ui = require "orca.ui"

class Page extends ui.Node2D
	new: (@view, ...) => 
		@args = {...}
		super!
		
	content: => 
		if @view
			@contentView = self.view (table.unpack @args)
		else
			ui.TextBlock class: "align-middle-center", "No content"