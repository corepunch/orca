import StackView, TextBlock from require "orca.UIKit"
Widget = require "orca.core.widget"

class ForecastScreen extends Widget
	title: "Forecast"

	content: =>
		StackView class: "bg-background p-6 gap-2", =>
			TextBlock class: "text-xl text-foreground", "7-Day Forecast"
			TextBlock class: "text-base text-foreground", "Mon: 23 C / 14 C, Clear"
			TextBlock class: "text-base text-foreground", "Tue: 21 C / 13 C, Partly Cloudy"
			TextBlock class: "text-base text-foreground", "Wed: 19 C / 11 C, Showers"
			TextBlock class: "text-base text-foreground", "Thu: 20 C / 12 C, Windy"
