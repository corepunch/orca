import StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core.widget"
import LOCATIONS from require "model"

class SavedScreen extends Widget
	title: "Saved"

	content: =>
		StackView class: "bg-background p-6 gap-2", =>
			TextBlock class: "text-xl text-foreground", "Saved Locations"
			for loc in *LOCATIONS
				TextBlock class: "text-base text-foreground", loc.name
