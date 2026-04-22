import StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core.widget"

class AboutScreen extends Widget
	title: "About"

	content: =>
		StackView class: "p-6 gap-3", =>
			TextBlock class: "text-xl", "About"
			TextBlock class: "text-base", "Weather demo with bottom navigation and route-driven content."

