import Screen, StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core2.widget"

class Default extends Widget
	content: =>
		inner = @content_for "inner"
		title = if @app_title then @app_title! else "App"

		Screen ->
			StackView class: "p-6 gap-3", =>
				TextBlock class: "text-3xl", title
				TextBlock class: "text-sm", "Layout chrome above route content"
				if inner
					@addChild inner
				else
					TextBlock class: "text-sm", "No inner content"
		