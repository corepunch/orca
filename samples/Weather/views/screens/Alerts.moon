import StackView, TextBlock from require "orca.UIKit"

=>
	view = StackView class: "p-6 gap-3", =>
		TextBlock class: "text-xl", "Alerts"
		TextBlock class: "text-base", "No active severe alerts."
		TextBlock class: "text-sm text-muted-foreground", "Rain advisory expected after 18:00 in coastal areas."
	view.title = "Alerts"
	view
