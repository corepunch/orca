import StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core.widget"

class AboutScreen extends Widget
	title: "About"

	content: =>
		StackView class: "bg-background p-6 gap-3", =>
			TextBlock class: "text-xl text-foreground", "About"
			TextBlock class: "text-base text-muted-foreground", "Weather demo with bottom navigation and route-driven content."
