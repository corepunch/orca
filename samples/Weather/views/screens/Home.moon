import StackView, TextBlock from require "orca.UIKit"

class HomeScreen extends StackView
	title: "Today"
	body: =>
		@class = "p-6 gap-3"
		TextBlock class: "text-xl", "Today"
		TextBlock class: "text-base", "Sunny, 24 C"
		TextBlock class: "text-sm text-muted-foreground", "Feels like 26 C. Light breeze from the west."

=> HomeScreen!
