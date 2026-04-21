import Screen, StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core2.widget"

class Default extends Widget
	content: =>
		inner = @content_for "inner"
		title = if @app_title then @app_title! else "App"

		Screen ->
			stack = StackView class: "p-6 gap-3", ->
				TextBlock Text: title, class: "text-3xl"
				TextBlock Text: "Layout chrome above route content", class: "text-sm"
			if inner
				stack:addChild inner
			else
				stack:addChild TextBlock Text: "No inner content", class: "text-sm"
