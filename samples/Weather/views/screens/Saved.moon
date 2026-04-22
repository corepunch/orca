import StackView, TextBlock from require "orca.UIKit"

=>
	view = StackView class: "p-6 gap-2", =>
		TextBlock class: "text-xl", "Saved Locations"
		TextBlock class: "text-base", "San Francisco"
		TextBlock class: "text-base", "New York"
		TextBlock class: "text-base", "Berlin"
	view.title = "Saved"
	view
