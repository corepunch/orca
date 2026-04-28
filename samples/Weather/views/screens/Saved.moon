import StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core.widget"

class SavedScreen extends Widget
	title: "Saved"

	content: =>
		StackView class: "bg-background p-6 gap-2", =>
			TextBlock class: "text-xl text-foreground", "Saved Locations"
			TextBlock class: "text-base text-foreground", "San Francisco"
			TextBlock class: "text-base text-foreground", "New York"
			TextBlock class: "text-base text-foreground", "Berlin"
