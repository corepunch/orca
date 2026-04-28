import StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core.widget"

class AlertsScreen extends Widget
	title: "Alerts"

	content: =>
		StackView class: "bg-background p-6 gap-3", =>
			TextBlock class: "text-xl text-foreground", "Alerts"
			TextBlock class: "text-base text-alert-clear", "No active severe alerts."
			TextBlock class: "text-sm text-muted-foreground", "Rain advisory expected after 18:00 in coastal areas."
