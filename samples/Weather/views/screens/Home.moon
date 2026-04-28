import StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core.widget"

class HomeScreen extends Widget
	title: "Today"

	content: =>
		StackView class: "bg-background p-6 gap-3", =>
			TextBlock class: "text-xl text-foreground", "Today"
			TextBlock class: "text-base text-foreground", "Sunny, 24 C"
			TextBlock class: "text-sm text-muted-foreground", "Feels like 26 C. Light breeze from the west."
